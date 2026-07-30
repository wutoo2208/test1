#ifndef DRIVERS_LINE_SENSORS_H_
#define DRIVERS_LINE_SENSORS_H_

#include <stdint.h>

#define LINE_SENSOR_COUNT (5U)

uint8_t LineSensors_readRawBits(void);

#endif /* DRIVERS_LINE_SENSORS_H_ */
