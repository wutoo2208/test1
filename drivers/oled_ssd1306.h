#ifndef DRIVERS_OLED_SSD1306_H_
#define DRIVERS_OLED_SSD1306_H_

#include <stdbool.h>
#include <stdint.h>

typedef struct {
    bool initialized;
    bool lastTransferOk;
    uint8_t address;
} OledSsd1306Status;

typedef enum {
    OLED_REQ002_READY = 0,
    OLED_REQ002_RUNNING,
    OLED_REQ002_COMPLETE,
    OLED_REQ002_FAULT
} OledReq002State;

bool OledSsd1306_showTestPattern(void);
bool OledSsd1306_forceAllPixelsOn(void);
bool OledSsd1306_forceAllPixelsOnSh1106(void);
bool OledSsd1306_showReq002Status(OledReq002State state,
    uint32_t elapsedMs, bool lineValid);
void OledSsd1306_clear(void);
OledSsd1306Status OledSsd1306_getStatus(void);

#endif /* DRIVERS_OLED_SSD1306_H_ */
