#ifndef ALGORITHM_PID_H_
#define ALGORITHM_PID_H_

#include <stdbool.h>

typedef struct {
    float kp;
    float ki;
    float kd;
    float outputMin;
    float outputMax;
    float integralMin;
    float integralMax;
    float derivativeAlpha;
} PidConfig;

typedef struct {
    PidConfig config;
    float integral;
    float previousMeasurement;
    float filteredDerivative;
    bool initialized;
} PidController;

void Pid_init(PidController *pid, const PidConfig *config);
void Pid_reset(PidController *pid, float measurement);
float Pid_step(PidController *pid, float setpoint, float measurement,
    float dtSeconds, bool freezeIntegral);

#endif /* ALGORITHM_PID_H_ */
