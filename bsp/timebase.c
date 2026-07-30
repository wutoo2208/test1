#include "bsp/timebase.h"

#include "ti_msp_dl_config.h"

static volatile uint32_t gMilliseconds;

void Timebase_init(void)
{
    gMilliseconds = 0U;
    DL_SYSTICK_init(CPUCLK_FREQ / 1000U);
    DL_SYSTICK_enableInterrupt();
    DL_SYSTICK_enable();
}

uint32_t Timebase_nowMs(void)
{
    return gMilliseconds;
}

bool Timebase_reached(uint32_t nowMs, uint32_t deadlineMs)
{
    return ((int32_t) (nowMs - deadlineMs)) >= 0;
}

void Timebase_onSysTick(void)
{
    gMilliseconds++;
}
