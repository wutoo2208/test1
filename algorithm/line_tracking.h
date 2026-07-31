#ifndef ALGORITHM_LINE_TRACKING_H_
#define ALGORITHM_LINE_TRACKING_H_

#include <stdbool.h>
#include <stdint.h>

#include "drivers/line_sensors.h"

typedef struct {
    float normalized[LINE_SENSOR_COUNT];
    float position;
    float centeredError;
    float confidence;
    float signalSum;
    float shadowCorrection;
    uint32_t sequence;
    uint32_t sensorErrors;
    uint32_t lastUpdateMs;
    bool valid;
    bool lineLost;
} LineTrackingStatus;

void LineTracking_init(uint32_t nowMs);
void LineTracking_service(uint32_t nowMs);
const LineTrackingStatus *LineTracking_getStatus(void);

#endif /* ALGORITHM_LINE_TRACKING_H_ */
