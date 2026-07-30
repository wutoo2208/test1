#include "app/app.h"

#include "app/req002.h"
#include "bsp/board_safety.h"
#include "bsp/timebase.h"
#include "drivers/diag_console.h"
#include "drivers/encoders.h"
#include "drivers/line_sensors.h"
#include "drivers/nrf24_ptx.h"
#include "drivers/start_button.h"

static StartButton gStartButton;

void App_init(void)
{
    uint32_t nowMs;

    BoardSafety_init();
    Timebase_init();
    nowMs = Timebase_nowMs();
    StartButton_init(&gStartButton, nowMs);
    Encoders_init();
    Req002_init(nowMs);
    DiagConsole_init();
    DiagConsole_reportBoot();
    Nrf24Ptx_init();
}

void App_service(void)
{
    uint32_t nowMs = Timebase_nowMs();

    BoardSafety_service();
    StartButton_poll(&gStartButton, nowMs);
    Req002_service(nowMs, StartButton_takePress(&gStartButton),
        LineSensors_readRawBits());
    DiagConsole_service();
    Nrf24Ptx_service();
}
