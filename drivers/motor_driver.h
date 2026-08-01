#ifndef DRIVERS_MOTOR_DRIVER_H_
#define DRIVERS_MOTOR_DRIVER_H_

#include <stdbool.h>
#include <stdint.h>

typedef enum {
    MOTOR_WHEEL_LEFT = 0,
    MOTOR_WHEEL_RIGHT,
    MOTOR_WHEEL_BOTH
} MotorWheel;

typedef enum {
    MOTOR_DRIVER_OK = 0,
    MOTOR_DRIVER_INVALID_DUTY,
    MOTOR_DRIVER_BUSY
} MotorDriverResult;

typedef struct {
    uint16_t leftDutyPermille;
    uint16_t rightDutyPermille;
    bool pwmCounterRunning;
} MotorDriverStatus;

void MotorDriver_init(void);
MotorDriverResult MotorDriver_driveSinglePrimary(MotorWheel wheel,
    uint16_t dutyPermille);
MotorDriverResult MotorDriver_driveVehicleForward(MotorWheel wheel,
    uint16_t dutyPermille);
MotorDriverResult MotorDriver_setVehicleForwardDuties(
    uint16_t leftDutyPermille, uint16_t rightDutyPermille);
void MotorDriver_stopAll(void);
bool MotorDriver_outputsStopped(void);
MotorDriverStatus MotorDriver_snapshot(void);

#endif /* DRIVERS_MOTOR_DRIVER_H_ */
