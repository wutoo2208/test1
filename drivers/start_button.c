#include "drivers/start_button.h"

#include "bsp/timebase.h"
#include "config/firmware_config.h"
#include "ti_msp_dl_config.h"

bool StartButton_readHardwarePressed(void)
{
    return DL_GPIO_readPins(DIAG_GPIO_START_BUTTON_PORT,
               DIAG_GPIO_START_BUTTON_PIN) == 0U;
}

void StartButton_init(StartButton *button, uint32_t nowMs)
{
    bool pressed = StartButton_readHardwarePressed();
    button->stablePressed = pressed;
    button->candidatePressed = pressed;
    button->pressPending = false;
    button->candidateSinceMs = nowMs;
}

void StartButton_poll(StartButton *button, uint32_t nowMs)
{
    bool pressed = StartButton_readHardwarePressed();

    if (pressed != button->candidatePressed) {
        button->candidatePressed = pressed;
        button->candidateSinceMs = nowMs;
    } else if ((pressed != button->stablePressed) &&
               Timebase_reached(nowMs,
                   button->candidateSinceMs + START_BUTTON_DEBOUNCE_MS)) {
        button->stablePressed = pressed;
        if (pressed) {
            button->pressPending = true;
        }
    }
}

bool StartButton_takePress(StartButton *button)
{
    bool pending = button->pressPending;
    button->pressPending = false;
    return pending;
}

bool StartButton_isPressed(const StartButton *button)
{
    return button->stablePressed;
}
