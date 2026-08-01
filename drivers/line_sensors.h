#ifndef DRIVERS_LINE_SENSORS_H_
#define DRIVERS_LINE_SENSORS_H_

#include <stdbool.h>
#include <stdint.h>

#define LINE_SENSOR_COUNT (6U)
#define LINE_SENSOR_BUS_LEVEL_SDA (1U << 0)
#define LINE_SENSOR_BUS_LEVEL_SCL (1U << 1)

typedef enum {
    LINE_SENSOR_FAILURE_NONE = 0,
    LINE_SENSOR_FAILURE_BUS_IDLE,
    LINE_SENSOR_FAILURE_TX_FIFO,
    LINE_SENSOR_FAILURE_TX_DONE,
    LINE_SENSOR_FAILURE_RX_DATA,
    LINE_SENSOR_FAILURE_BUS_RELEASE
} LineSensorFailureStage;

typedef struct {
    uint16_t analog[LINE_SENSOR_COUNT];
    uint32_t lastSuccessMs;
    uint32_t sequence;
    uint32_t errorCount;
    uint32_t lastControllerStatus;
    uint32_t recoveryCount;
    uint32_t recoveryFailureCount;
    uint8_t digitalBits;
    uint8_t lastFailureStage;
    uint8_t lastRegister;
    uint8_t lastReceivedBytes;
    uint8_t lastBusLevels;
    bool valid;
    bool lastRecoverySucceeded;
} LineSensorSample;

void LineSensors_init(uint32_t nowMs);
void LineSensors_service(uint32_t nowMs);
LineSensorSample LineSensors_snapshot(void);
uint8_t LineSensors_readRawBits(void);

#endif /* DRIVERS_LINE_SENSORS_H_ */
