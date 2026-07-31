#ifndef DRIVERS_OLED_SSD1306_H_
#define DRIVERS_OLED_SSD1306_H_

#include <stdbool.h>

typedef struct {
    bool initialized;
    bool lastTransferOk;
} OledSsd1306Status;

bool OledSsd1306_showTestPattern(void);
bool OledSsd1306_forceAllPixelsOn(void);
bool OledSsd1306_forceAllPixelsOnSh1106(void);
void OledSsd1306_clear(void);
OledSsd1306Status OledSsd1306_getStatus(void);

#endif /* DRIVERS_OLED_SSD1306_H_ */
