#include "drivers/line_sensors.h"

#include "ti_msp_dl_config.h"

uint8_t LineSensors_readRawBits(void)
{
    uint8_t bits = 0U;

    if (DL_GPIO_readPins(DIAG_GPIO_TCRT_OUT1_PORT,
            DIAG_GPIO_TCRT_OUT1_PIN) != 0U) bits |= (1U << 0U);
    if (DL_GPIO_readPins(DIAG_GPIO_TCRT_OUT2_PORT,
            DIAG_GPIO_TCRT_OUT2_PIN) != 0U) bits |= (1U << 1U);
    if (DL_GPIO_readPins(DIAG_GPIO_TCRT_OUT3_PORT,
            DIAG_GPIO_TCRT_OUT3_PIN) != 0U) bits |= (1U << 2U);
    if (DL_GPIO_readPins(DIAG_GPIO_TCRT_OUT4_PORT,
            DIAG_GPIO_TCRT_OUT4_PIN) != 0U) bits |= (1U << 3U);
    if (DL_GPIO_readPins(DIAG_GPIO_TCRT_OUT5_PORT,
            DIAG_GPIO_TCRT_OUT5_PIN) != 0U) bits |= (1U << 4U);
    return bits;
}
