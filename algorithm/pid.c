#include "algorithm/pid.h"

static float clampValue(float value, float minimum, float maximum)
{
    if (value < minimum) return minimum;
    if (value > maximum) return maximum;
    return value;
}

void Pid_init(PidController *pid, const PidConfig *config)
{
    pid->config = *config;
    pid->integral = 0.0f;
    pid->previousMeasurement = 0.0f;
    pid->filteredDerivative = 0.0f;
    pid->initialized = false;
}

void Pid_reset(PidController *pid, float measurement)
{
    pid->integral = 0.0f;
    pid->previousMeasurement = measurement;
    pid->filteredDerivative = 0.0f;
    pid->initialized = true;
}

float Pid_step(PidController *pid, float setpoint, float measurement,
    float dtSeconds, bool freezeIntegral)
{
    float error = setpoint - measurement;
    float derivative = 0.0f;
    float candidateIntegral = pid->integral;
    float unsaturated;
    float output;

    if (!pid->initialized) {
        Pid_reset(pid, measurement);
    } else if (dtSeconds <= 0.0f) {
        return clampValue(pid->config.kp * error,
            pid->config.outputMin, pid->config.outputMax);
    } else {
        derivative = -(measurement - pid->previousMeasurement) / dtSeconds;
    }

    pid->filteredDerivative =
        (pid->config.derivativeAlpha * pid->filteredDerivative) +
        ((1.0f - pid->config.derivativeAlpha) * derivative);

    if (!freezeIntegral && (dtSeconds > 0.0f)) {
        candidateIntegral = clampValue(
            pid->integral + (error * dtSeconds),
            pid->config.integralMin, pid->config.integralMax);
    }

    unsaturated = (pid->config.kp * error) +
        (pid->config.ki * candidateIntegral) +
        (pid->config.kd * pid->filteredDerivative);
    output = clampValue(unsaturated,
        pid->config.outputMin, pid->config.outputMax);

    /* Conditional integration prevents pushing farther into saturation. */
    if (!freezeIntegral &&
        ((unsaturated == output) ||
         ((unsaturated > output) && (error < 0.0f)) ||
         ((unsaturated < output) && (error > 0.0f)))) {
        pid->integral = candidateIntegral;
    }

    pid->previousMeasurement = measurement;
    return output;
}
