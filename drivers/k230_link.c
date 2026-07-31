#include "drivers/k230_link.h"

#include <stddef.h>
#include <string.h>

#define K230_FRAME_HEAD_0                  (0x55U)
#define K230_FRAME_HEAD_1                  (0xAAU)
#define K230_CRC8_POLYNOMIAL               (0x07U)
#define K230_RX_QUEUE_MASK                 (K230_LINK_RX_QUEUE_CAPACITY - 1U)

#if (K230_LINK_RX_QUEUE_CAPACITY == 0U) ||     ((K230_LINK_RX_QUEUE_CAPACITY & K230_RX_QUEUE_MASK) != 0U)
#error "K230_LINK_RX_QUEUE_CAPACITY must be a power of two"
#endif

#if (K230_LINK_RX_QUEUE_CAPACITY > 256U)
#error "K230 RX queue indexes require capacity <= 256"
#endif

/*
 * Only K230Link_pushRxByteFromIsr() writes gRxHead and queue data.
 * Only K230Link_service() writes gRxTail and parser/publication state.
 */
static volatile uint8_t gRxQueue[K230_LINK_RX_QUEUE_CAPACITY];
static volatile uint16_t gRxHead;
static volatile uint16_t gRxTail;
static volatile uint32_t gRxBytesFromIsr;
static volatile uint32_t gQueueOverflowsFromIsr;

static uint8_t gFrame[K230_LINK_FRAME_SIZE];
static uint8_t gFrameIndex;
static uint8_t gLastSequence;
static uint32_t gLastSequenceAdvanceMs;
static uint32_t gLastObservedOverflow;
static bool gSequenceInitialized;
static bool gHaveFrame;
static bool gTimedOut;

static K230LinkSample gLatest;
static K230LinkStats gStats;

static uint16_t readU16Le(const uint8_t *data)
{
    return (uint16_t) data[0] | ((uint16_t) data[1] << 8U);
}

static int16_t readI16Le(const uint8_t *data)
{
    uint16_t raw = readU16Le(data);
    int32_t value = (int32_t) raw;

    if ((raw & 0x8000U) != 0U) {
        value -= 65536L;
    }
    return (int16_t) value;
}

static bool elapsedMoreThan(uint32_t nowMs, uint32_t sinceMs,
    uint32_t limitMs)
{
    return (uint32_t) (nowMs - sinceMs) > limitMs;
}

uint8_t K230Protocol_crc8Atm(const uint8_t *data, uint32_t length)
{
    uint8_t crc = 0U;
    uint32_t index;

    if ((data == NULL) && (length != 0U)) {
        return 0U;
    }

    for (index = 0U; index < length; index++) {
        uint8_t bit;
        crc ^= data[index];
        for (bit = 0U; bit < 8U; bit++) {
            if ((crc & 0x80U) != 0U) {
                crc = (uint8_t) ((crc << 1U) ^ K230_CRC8_POLYNOMIAL);
            } else {
                crc <<= 1U;
            }
        }
    }
    return crc;
}

static void resetParserUsingLastByte(uint8_t byte)
{
    if (byte == K230_FRAME_HEAD_0) {
        gFrame[0] = K230_FRAME_HEAD_0;
        gFrameIndex = 1U;
    } else {
        gFrameIndex = 0U;
    }
}

static bool lostPayloadIsCanonical(const K230LinkSample *sample)
{
    return (sample->positionTenthMm == 0) &&
        (sample->velocityTenthMmPerSecond == 0) &&
        (sample->confidence == 0U) &&
        (sample->pixelX == K230_LINK_LOST_PIXEL_VALUE) &&
        (sample->pixelY == K230_LINK_LOST_PIXEL_VALUE);
}

static void updateSequence(K230LinkSample *sample, uint32_t nowMs)
{
    if (!gSequenceInitialized) {
        gSequenceInitialized = true;
        gLastSequence = sample->sequence;
        gLastSequenceAdvanceMs = nowMs;
        sample->sequenceAdvanced = true;
        return;
    }

    if (sample->sequence == gLastSequence) {
        gStats.duplicateSequences++;
        sample->sequenceAdvanced = false;
        return;
    }

    if (sample->sequence != (uint8_t) (gLastSequence + 1U)) {
        gStats.sequenceGapEvents++;
    }
    gLastSequence = sample->sequence;
    gLastSequenceAdvanceMs = nowMs;
    sample->sequenceAdvanced = true;
}

static void publishFrame(uint32_t nowMs)
{
    K230LinkSample sample;
    uint8_t status = gFrame[2];
    uint8_t expectedCrc = K230Protocol_crc8Atm(gFrame,
        K230_LINK_CRC_INPUT_SIZE);

    if (expectedCrc != gFrame[K230_LINK_FRAME_SIZE - 1U]) {
        gStats.crcErrors++;
        return;
    }
    if (status > (uint8_t) K230_STATUS_PREDICTED) {
        gStats.statusErrors++;
        return;
    }

    memset(&sample, 0, sizeof(sample));
    sample.status = (K230Status) status;
    sample.sequence = gFrame[3];
    sample.positionTenthMm = readI16Le(&gFrame[4]);
    sample.velocityTenthMmPerSecond = readI16Le(&gFrame[6]);
    sample.confidence = gFrame[8];
    sample.pixelX = readU16Le(&gFrame[9]);
    sample.pixelY = readU16Le(&gFrame[11]);
    sample.receivedMs = nowMs;
    sample.linkFresh = true;
    sample.predicted = sample.status == K230_STATUS_PREDICTED;
    sample.measurementUsable = sample.status != K230_STATUS_LOST;
    sample.lostPayloadCanonical = true;

    if (sample.status == K230_STATUS_LOST) {
        sample.lostPayloadCanonical = lostPayloadIsCanonical(&sample);
        if (!sample.lostPayloadCanonical) {
            gStats.lostPayloadMismatches++;
        }
    }

    updateSequence(&sample, nowMs);
    sample.sequenceAgeMs = (uint32_t) (nowMs - gLastSequenceAdvanceMs);

    if (gTimedOut) {
        gStats.linkRecoveries++;
    }
    gTimedOut = false;
    gLatest = sample;
    gHaveFrame = true;
    gStats.validFrames++;

    switch (sample.status) {
        case K230_STATUS_MEASURED:
            gStats.measuredFrames++;
            break;
        case K230_STATUS_PREDICTED:
            gStats.predictedFrames++;
            break;
        case K230_STATUS_LOST:
        default:
            gStats.lostFrames++;
            break;
    }
}

static void consumeByte(uint8_t byte, uint32_t nowMs)
{
    if (gFrameIndex == 0U) {
        if (byte == K230_FRAME_HEAD_0) {
            gFrame[0] = byte;
            gFrameIndex = 1U;
        }
        return;
    }

    if (gFrameIndex == 1U) {
        if (byte == K230_FRAME_HEAD_1) {
            gFrame[1] = byte;
            gFrameIndex = 2U;
        } else if (byte == K230_FRAME_HEAD_0) {
            gFrame[0] = byte;
        } else {
            gFrameIndex = 0U;
            gStats.headerResyncs++;
        }
        return;
    }

    gFrame[gFrameIndex++] = byte;
    if (gFrameIndex == K230_LINK_FRAME_SIZE) {
        publishFrame(nowMs);
        resetParserUsingLastByte(byte);
    }
}

void K230Link_init(uint32_t nowMs)
{
    (void) nowMs;
    gRxHead = 0U;
    gRxTail = 0U;
    gRxBytesFromIsr = 0U;
    gQueueOverflowsFromIsr = 0U;
    gFrameIndex = 0U;
    gLastSequence = 0U;
    gLastSequenceAdvanceMs = 0U;
    gLastObservedOverflow = 0U;
    gSequenceInitialized = false;
    gHaveFrame = false;
    gTimedOut = false;
    memset(gFrame, 0, sizeof(gFrame));
    memset(&gLatest, 0, sizeof(gLatest));
    memset(&gStats, 0, sizeof(gStats));
}

bool K230Link_pushRxByteFromIsr(uint8_t byte)
{
    uint16_t head = gRxHead;
    uint16_t next = (uint16_t) ((head + 1U) & K230_RX_QUEUE_MASK);

    gRxBytesFromIsr++;
    if (next == gRxTail) {
        gQueueOverflowsFromIsr++;
        return false;
    }

    gRxQueue[head] = byte;
    gRxHead = next;
    return true;
}

void K230Link_service(uint32_t nowMs)
{
    uint32_t overflowCount = gQueueOverflowsFromIsr;

    if (overflowCount != gLastObservedOverflow) {
        gLastObservedOverflow = overflowCount;
        gFrameIndex = 0U;
        gStats.parserResetsAfterOverflow++;
    }

    while (gRxTail != gRxHead) {
        uint16_t tail = gRxTail;
        uint8_t byte = gRxQueue[tail];
        gRxTail = (uint16_t) ((tail + 1U) & K230_RX_QUEUE_MASK);
        consumeByte(byte, nowMs);
    }

    if (gHaveFrame && !gTimedOut &&
        elapsedMoreThan(nowMs, gLatest.receivedMs, K230_LINK_TIMEOUT_MS)) {
        gTimedOut = true;
        gStats.timeoutEvents++;
    }
}

bool K230Link_snapshot(uint32_t nowMs, K230LinkSample *sample)
{
    if ((sample == NULL) || !gHaveFrame) {
        return false;
    }

    *sample = gLatest;
    sample->frameAgeMs = (uint32_t) (nowMs - sample->receivedMs);
    sample->sequenceAgeMs = (uint32_t) (nowMs - gLastSequenceAdvanceMs);
    sample->linkFresh = !gTimedOut &&
        !elapsedMoreThan(nowMs, sample->receivedMs, K230_LINK_TIMEOUT_MS);
    sample->measurementUsable = sample->linkFresh &&
        (sample->status != K230_STATUS_LOST);
    sample->predicted = sample->status == K230_STATUS_PREDICTED;
    return true;
}

void K230Link_getStats(K230LinkStats *stats)
{
    if (stats == NULL) {
        return;
    }

    *stats = gStats;
    stats->rxBytes = gRxBytesFromIsr;
    stats->queueOverflows = gQueueOverflowsFromIsr;
}

uint16_t K230Link_pendingBytes(void)
{
    uint16_t head = gRxHead;
    uint16_t tail = gRxTail;
    return (uint16_t) ((head - tail) & K230_RX_QUEUE_MASK);
}
