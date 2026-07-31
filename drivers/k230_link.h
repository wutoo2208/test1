#ifndef DRIVERS_K230_LINK_H_
#define DRIVERS_K230_LINK_H_

#include <stdbool.h>
#include <stdint.h>

/*
 * K230 -> MSPM0 UART protocol v1.
 *
 * Wire format (14 bytes):
 *   55 AA STATUS SEQ POSL POSH VELL VELH CONF XL XH YL YH CRC8
 *
 * The protocol and PA9/UART1_RX pin ownership are documented in:
 *   docs/protocols/k230-ball-position-uart-v1.md
 *
 * CLAUDE CODE INTEGRATION NOTE
 * ----------------------------
 * This module deliberately contains no DriverLib or SysConfig identifiers.
 * Do not guess the generated UART instance/IRQ names here. After UART1 RX on
 * PA9 is configured and generated, use the exact names from
 * ti_msp_dl_config.h in the hardware adapter/ISR.
 *
 * Intended call flow:
 *   1. K230Link_init(nowMs) before enabling the UART RX interrupt.
 *   2. For every byte drained from the UART RX FIFO in the ISR, call
 *      K230Link_pushRxByteFromIsr(byte).
 *   3. Call K230Link_service(nowMs) frequently from the foreground loop.
 *   4. Use K230Link_snapshot(nowMs, &sample), and never use POS/VEL unless
 *      sample.linkFresh && sample.measurementUsable are both true.
 *
 * STATUS=PREDICTED is exposed but is not automatically reweighted because
 * the prediction confidence policy has not been approved yet.
 */

#define K230_LINK_FRAME_SIZE               (14U)
#define K230_LINK_CRC_INPUT_SIZE           (13U)
#define K230_LINK_RX_QUEUE_CAPACITY        (128U)
#define K230_LINK_TIMEOUT_MS               (100U)
#define K230_LINK_LOST_PIXEL_VALUE         (0xFFFFU)

typedef enum {
    K230_STATUS_LOST = 0,
    K230_STATUS_MEASURED = 1,
    K230_STATUS_PREDICTED = 2
} K230Status;

typedef struct {
    K230Status status;
    int16_t positionTenthMm;
    int16_t velocityTenthMmPerSecond;
    uint16_t pixelX;
    uint16_t pixelY;
    uint32_t receivedMs;
    uint32_t frameAgeMs;
    uint32_t sequenceAgeMs;
    uint8_t sequence;
    uint8_t confidence;
    bool linkFresh;
    bool measurementUsable;
    bool predicted;
    bool sequenceAdvanced;
    bool lostPayloadCanonical;
} K230LinkSample;

typedef struct {
    uint32_t rxBytes;
    uint32_t queueOverflows;
    uint32_t parserResetsAfterOverflow;
    uint32_t headerResyncs;
    uint32_t validFrames;
    uint32_t measuredFrames;
    uint32_t predictedFrames;
    uint32_t lostFrames;
    uint32_t crcErrors;
    uint32_t statusErrors;
    uint32_t lostPayloadMismatches;
    uint32_t duplicateSequences;
    uint32_t sequenceGapEvents;
    uint32_t timeoutEvents;
    uint32_t linkRecoveries;
} K230LinkStats;

void K230Link_init(uint32_t nowMs);

/*
 * ISR-safe single-producer enqueue. The foreground is the only consumer.
 * Returns false when the queue is full; the byte is then dropped and the
 * parser is reset by K230Link_service().
 */
bool K230Link_pushRxByteFromIsr(uint8_t byte);

/* Drain queued bytes, parse complete frames, and update the 100 ms timeout. */
void K230Link_service(uint32_t nowMs);

/*
 * Returns false until at least one CRC/status-valid frame has been received.
 * STATUS=LOST is a valid protocol frame but measurementUsable will be false.
 */
bool K230Link_snapshot(uint32_t nowMs, K230LinkSample *sample);

/* Diagnostic counters; rxBytes/queueOverflows may advance in the RX ISR. */
void K230Link_getStats(K230LinkStats *stats);

uint16_t K230Link_pendingBytes(void);

/* Public for host tests and K230-side compatibility checks. */
uint8_t K230Protocol_crc8Atm(const uint8_t *data, uint32_t length);

#endif /* DRIVERS_K230_LINK_H_ */
