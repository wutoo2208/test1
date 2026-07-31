#include "app/app.h"

#include "app/req002.h"
#include "app/motor_test.h"
#include "algorithm/line_tracking.h"
#include "bsp/board_safety.h"
#include "bsp/timebase.h"
#include "drivers/diag_console.h"
#include "drivers/encoders.h"
#include "drivers/line_sensors.h"
#include "drivers/motor_driver.h"
#include "drivers/nrf24_ptx.h"
#include "drivers/start_button.h"

static StartButton gStartButton;

void App_init(void)
{
    uint32_t nowMs;

    MotorDriver_init();
    BoardSafety_init();
    Timebase_init();
    nowMs = Timebase_nowMs();
    MotorTest_init(nowMs);
    StartButton_init(&gStartButton, nowMs);
    LineSensors_init(nowMs);
    LineTracking_init(nowMs);
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
    MotorTest_service(nowMs);
    StartButton_poll(&gStartButton, nowMs);
    LineSensors_service(nowMs);
    nowMs = Timebase_nowMs();
    LineTracking_service(nowMs);
    Req002_service(nowMs, StartButton_takePress(&gStartButton),
        LineSensors_readRawBits());
    DiagConsole_service();
    Nrf24Ptx_service();
}
