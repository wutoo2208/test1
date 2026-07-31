#ifndef APP_MOTOR_TEST_H_
#define APP_MOTOR_TEST_H_

#include <stdbool.h>
#include <stdint.h>

#include "drivers/motor_driver.h"

typedef enum {
    MOTOR_TEST_IDLE = 0,
    MOTOR_TEST_RUNNING,
    MOTOR_TEST_COMPLETE,
    MOTOR_TEST_ABORTED,
    MOTOR_TEST_REJECTED,
    MOTOR_TEST_FAULT
} MotorTestState;

typedef enum {
    MOTOR_TEST_START_OK = 0,
    MOTOR_TEST_START_DISABLED,
    MOTOR_TEST_START_BUSY,
    MOTOR_TEST_START_SAFETY_FAULT,
    MOTOR_TEST_START_DRIVER_REJECTED
} MotorTestStartResult;

typedef struct {
    MotorTestState state;
    MotorWheel wheel;
    uint16_t dutyPermille;
    uint32_t startedMs;
    uint32_t deadlineMs;
    bool outputsActive;
} MotorTestStatus;

void MotorTest_init(uint32_t nowMs);
MotorTestStartResult MotorTest_start(MotorWheel wheel, uint32_t nowMs);
void MotorTest_service(uint32_t nowMs);
void MotorTest_onTimebaseTick(uint32_t nowMs);
void MotorTest_abort(void);
MotorTestStatus MotorTest_snapshot(void);
const char *MotorTest_stateName(MotorTestState state);

#endif /* APP_MOTOR_TEST_H_ */
