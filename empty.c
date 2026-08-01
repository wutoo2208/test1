/*
 * Copyright (c) 2021, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * * Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 *
 * * Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the distribution.
 *
 * * Neither the name of Texas Instruments Incorporated nor the names of
 *   its contributors may be used to endorse or promote products derived
 
 *   from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "ti_msp_dl_config.h"

#include "app/app.h"
#include "app/motor_test.h"
#include "bsp/timebase.h"
#include "drivers/diag_console.h"
#include "drivers/encoders.h"
#include "drivers/k230_link.h"

typedef struct {
    uint32_t serviceCount;
    uint32_t haveSample;
    uint32_t status;
    int32_t positionTenthMm;
    int32_t velocityTenthMmPerSecond;
    uint32_t confidence;
    uint32_t pixelX;
    uint32_t pixelY;
    uint32_t sequence;
    uint32_t receivedMs;
    uint32_t frameAgeMs;
    uint32_t sequenceAgeMs;
    uint32_t linkFresh;
    uint32_t measurementUsable;
    uint32_t predicted;
    uint32_t sequenceAdvanced;
    uint32_t lostPayloadCanonical;
    uint32_t pendingBytes;
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
} K230RuntimeDiag;

/* Read-only diagnostic mirror for no-actuation K230 link verification. */
volatile K230RuntimeDiag gK230RuntimeDiag;

static void K230Diag_service(void)
{
    K230LinkSample sample;
    K230LinkStats stats;
    uint32_t nowMs = Timebase_nowMs();

    K230Link_service(nowMs);
    gK230RuntimeDiag.serviceCount++;
    if (K230Link_snapshot(nowMs, &sample)) {
        gK230RuntimeDiag.haveSample = 1U;
        gK230RuntimeDiag.status = (uint32_t) sample.status;
        gK230RuntimeDiag.positionTenthMm = sample.positionTenthMm;
        gK230RuntimeDiag.velocityTenthMmPerSecond =
            sample.velocityTenthMmPerSecond;
        gK230RuntimeDiag.confidence = sample.confidence;
        gK230RuntimeDiag.pixelX = sample.pixelX;
        gK230RuntimeDiag.pixelY = sample.pixelY;
        gK230RuntimeDiag.sequence = sample.sequence;
        gK230RuntimeDiag.receivedMs = sample.receivedMs;
        gK230RuntimeDiag.frameAgeMs = sample.frameAgeMs;
        gK230RuntimeDiag.sequenceAgeMs = sample.sequenceAgeMs;
        gK230RuntimeDiag.linkFresh = sample.linkFresh ? 1U : 0U;
        gK230RuntimeDiag.measurementUsable =
            sample.measurementUsable ? 1U : 0U;
        gK230RuntimeDiag.predicted = sample.predicted ? 1U : 0U;
        gK230RuntimeDiag.sequenceAdvanced =
            sample.sequenceAdvanced ? 1U : 0U;
        gK230RuntimeDiag.lostPayloadCanonical =
            sample.lostPayloadCanonical ? 1U : 0U;
    } else {
        gK230RuntimeDiag.haveSample = 0U;
    }

    K230Link_getStats(&stats);
    gK230RuntimeDiag.pendingBytes = K230Link_pendingBytes();
    gK230RuntimeDiag.rxBytes = stats.rxBytes;
    gK230RuntimeDiag.queueOverflows = stats.queueOverflows;
    gK230RuntimeDiag.parserResetsAfterOverflow =
        stats.parserResetsAfterOverflow;
    gK230RuntimeDiag.headerResyncs = stats.headerResyncs;
    gK230RuntimeDiag.validFrames = stats.validFrames;
    gK230RuntimeDiag.measuredFrames = stats.measuredFrames;
    gK230RuntimeDiag.predictedFrames = stats.predictedFrames;
    gK230RuntimeDiag.lostFrames = stats.lostFrames;
    gK230RuntimeDiag.crcErrors = stats.crcErrors;
    gK230RuntimeDiag.statusErrors = stats.statusErrors;
    gK230RuntimeDiag.lostPayloadMismatches = stats.lostPayloadMismatches;
    gK230RuntimeDiag.duplicateSequences = stats.duplicateSequences;
    gK230RuntimeDiag.sequenceGapEvents = stats.sequenceGapEvents;
    gK230RuntimeDiag.timeoutEvents = stats.timeoutEvents;
    gK230RuntimeDiag.linkRecoveries = stats.linkRecoveries;
}

int main(void)
{
    SYSCFG_DL_init();
    NVIC_DisableIRQ(K230_UART_INST_INT_IRQN);
    App_init();
    K230Link_init(Timebase_nowMs());
    Encoders_speedShadowInit(Timebase_nowMs());
    NVIC_ClearPendingIRQ(K230_UART_INST_INT_IRQN);
    NVIC_EnableIRQ(K230_UART_INST_INT_IRQN);

    while (1) {
        K230Diag_service();
        Encoders_speedShadowService(Timebase_nowMs());
        App_service();
    }
}

void SysTick_Handler(void)
{
    Timebase_onSysTick();
    MotorTest_onTimebaseTick(Timebase_nowMs());
}

void DIAG_UART_INST_IRQHandler(void)
{
    DiagConsole_onUartInterrupt();
}

void K230_UART_INST_IRQHandler(void)
{
    if (DL_UART_Main_getPendingInterrupt(K230_UART_INST) ==
        DL_UART_MAIN_IIDX_RX) {
        while (!DL_UART_Main_isRXFIFOEmpty(K230_UART_INST)) {
            (void) K230Link_pushRxByteFromIsr(
                DL_UART_Main_receiveData(K230_UART_INST));
        }
    }
}

void GROUP1_IRQHandler(void)
{
    switch (DL_Interrupt_getPendingGroup(DL_INTERRUPT_GROUP_1)) {
        case DIAG_GPIO_INT_IIDX:
            if (DL_GPIO_getPendingInterrupt(
                    DIAG_GPIO_LEFT_ENCODER_A_PORT) ==
                DIAG_GPIO_LEFT_ENCODER_A_IIDX) {
                Encoders_onLeftEncoderAInterrupt();
            }
            break;
        default:
            break;
    }
}
