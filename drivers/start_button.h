#ifndef DRIVERS_START_BUTTON_H_
#define DRIVERS_START_BUTTON_H_

#include <stdbool.h>
#include <stdint.h>

typedef struct {
    bool stablePressed;
    bool candidatePressed;
    bool pressPending;
    uint32_t candidateSinceMs;
} StartButton;

void StartButton_init(StartButton *button, uint32_t nowMs);
void StartButton_poll(StartButton *button, uint32_t nowMs);
bool StartButton_takePress(StartButton *button);
bool StartButton_isPressed(const StartButton *button);

#endif /* DRIVERS_START_BUTTON_H_ */
