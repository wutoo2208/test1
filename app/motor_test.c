#include "app/motor_test.h"

#include "bsp/board_safety.h"
#include "bsp/timebase.h"
#include "config/firmware_config.h"
#include "ti/devices/msp/m0p/mspm0g350x.h"

static volatile MotorTestStatus gStatus;
static volatile bool gTimedOut;

void MotorTest_init(uint32_t nowMs)
{
    gStatus.state = MOTOR_TEST_IDLE;
    gStatus.wheel = MOTOR_WHEEL_LEFT;
    gStatus.dutyPermille = 0U;
    gStatus.startedMs = nowMs;
    gStatus.deadlineMs = nowMs;
    gStatus.outputsActive = false;
    gTimedOut = false;
}

MotorTestStartResult MotorTest_start(MotorWheel wheel, uint32_t nowMs)
{
#if MOTOR_SELFTEST_BUILD
    MotorDriverResult result;

    if (gStatus.state == MOTOR_TEST_RUNNING) {
        return MOTOR_TEST_START_BUSY;
    }
    if (BoardSafety_faultLatched()) {
        return MOTOR_TEST_START_SAFETY_FAULT;
    }
    if (!MotorDriver_outputsStopped()) {
        return MOTOR_TEST_START_BUSY;
    }

    BoardSafety_stop(BOARD_SAFETY_STOP_TEST_ABORT);
    gStatus.state = MOTOR_TEST_RUNNING;
    gStatus.wheel = wheel;
    gStatus.dutyPermille = MOTOR_TEST_DUTY_PERMILLE;
    gStatus.startedMs = nowMs;
    gStatus.deadlineMs = nowMs + MOTOR_TEST_DURATION_MS;
    gStatus.outputsActive = true;
    gTimedOut = false;

    result = MotorDriver_driveSinglePrimary(wheel, gStatus.dutyPermille);
    if (result != MOTOR_DRIVER_OK) {
        BoardSafety_stop(BOARD_SAFETY_STOP_TEST_ABORT);
        gStatus.outputsActive = false;
        gStatus.state = MOTOR_TEST_REJECTED;
        return MOTOR_TEST_START_DRIVER_REJECTED;
    }
    return MOTOR_TEST_START_OK;
#else
    (void) wheel;
    (void) nowMs;
    return MOTOR_TEST_START_DISABLED;
#endif
}

void MotorTest_onTimebaseTick(uint32_t nowMs)
{
#if MOTOR_SELFTEST_BUILD
    if ((gStatus.state == MOTOR_TEST_RUNNING) &&
        Timebase_reached(nowMs, gStatus.deadlineMs)) {
        BoardSafety_stop(BOARD_SAFETY_STOP_TEST_TIMEOUT);
        gStatus.outputsActive = false;
        gTimedOut = true;
    }
#else
    (void) nowMs;
#endif
}

void MotorTest_service(uint32_t nowMs)
{
    (void) nowMs;
#if MOTOR_SELFTEST_BUILD
    if (gTimedOut) {
        gTimedOut = false;
        gStatus.state = MOTOR_TEST_COMPLETE;
    } else if ((gStatus.state == MOTOR_TEST_RUNNING) &&
               BoardSafety_faultLatched()) {
        BoardSafety_stop(BOARD_SAFETY_STOP_FAULT);
        gStatus.outputsActive = false;
        gStatus.state = MOTOR_TEST_FAULT;
    }
#endif
}

void MotorTest_abort(void)
{
    BoardSafety_stop(BOARD_SAFETY_STOP_TEST_ABORT);
    gStatus.outputsActive = false;
    if (gStatus.state == MOTOR_TEST_RUNNING) {
        gStatus.state = MOTOR_TEST_ABORTED;
    }
}

MotorTestStatus MotorTest_snapshot(void)
{
    MotorTestStatus status;
    __disable_irq();
    status = gStatus;
    __enable_irq();
    return status;
}

const char *MotorTest_stateName(MotorTestState state)
{
    switch (state) {
        case MOTOR_TEST_IDLE: return "IDLE";
        case MOTOR_TEST_RUNNING: return "RUNNING";
        case MOTOR_TEST_COMPLETE: return "COMPLETE";
        case MOTOR_TEST_ABORTED: return "ABORTED";
        case MOTOR_TEST_REJECTED: return "REJECTED";
        case MOTOR_TEST_FAULT:
        default: return "FAULT";
    }
}
