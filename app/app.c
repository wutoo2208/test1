#include "app/app.h"

#include <string.h>

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
    LineSensorSample lineSample;
    const LineTrackingStatus *lineTracking;
    Req002TrackingInput req002Tracking;

    BoardSafety_service();
    MotorTest_service(nowMs);
    StartButton_poll(&gStartButton, nowMs);
    LineSensors_service(nowMs);
    nowMs = Timebase_nowMs();
    LineTracking_service(nowMs);
    lineSample = LineSensors_snapshot();
    lineTracking = LineTracking_getStatus();
    memcpy(req002Tracking.analog, lineSample.analog,
        sizeof(req002Tracking.analog));
    req002Tracking.centeredError = lineTracking->centeredError;
    req002Tracking.confidence = lineTracking->confidence;
    req002Tracking.signalSum = lineTracking->signalSum;
    req002Tracking.sampleSequence = lineSample.sequence;
    req002Tracking.trackingSequence = lineTracking->sequence;
    req002Tracking.lastSuccessMs = lineSample.lastSuccessMs;
    req002Tracking.sensorErrorCount = lineSample.errorCount;
    req002Tracking.digitalBits = lineSample.digitalBits;
    req002Tracking.i2cFailureStage = lineSample.lastFailureStage;
    req002Tracking.sensorValid = lineSample.valid;
    req002Tracking.trackingValid = lineTracking->valid;
    req002Tracking.lineLost = lineTracking->lineLost;
    Req002_service(nowMs, StartButton_takePress(&gStartButton),
        &req002Tracking);
    DiagConsole_service();
    Nrf24Ptx_service();
}
