#ifndef DRIVERS_I2C_DIAG_H_
#define DRIVERS_I2C_DIAG_H_

#include <stdbool.h>
#include <stdint.h>

typedef struct {
    bool oled3c;
    bool oled3d;
    bool mpu68;
    bool mpu69;
} I2cDiagScan;

typedef struct {
    bool found;
    uint8_t address;
    uint8_t whoAmI;
} I2cDiagMpu;

I2cDiagScan I2cDiag_scan(void);
I2cDiagMpu I2cDiag_readMpuWhoAmI(void);

#endif /* DRIVERS_I2C_DIAG_H_ */
