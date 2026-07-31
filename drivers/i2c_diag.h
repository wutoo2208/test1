#ifndef DRIVERS_I2C_DIAG_H_
#define DRIVERS_I2C_DIAG_H_

#include <stdbool.h>

typedef struct {
    bool line5c;
    bool oled3c;
    bool oled3d;
} I2cDiagScan;

I2cDiagScan I2cDiag_scan(void);

#endif /* DRIVERS_I2C_DIAG_H_ */
