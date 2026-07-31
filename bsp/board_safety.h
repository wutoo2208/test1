#ifndef BSP_BOARD_SAFETY_H_
#define BSP_BOARD_SAFETY_H_

#include <stdbool.h>

typedef enum {
    BOARD_SAFETY_STOP_BOOT = 0,
    BOARD_SAFETY_STOP_OPERATOR,
    BOARD_SAFETY_STOP_SELFTEST,
    BOARD_SAFETY_STOP_TEST_TIMEOUT,
    BOARD_SAFETY_STOP_TEST_ABORT,
    BOARD_SAFETY_STOP_FAULT
} BoardSafetyStopReason;

typedef struct {
    BoardSafetyStopReason lastStopReason;
    bool motorStopped;
    bool d36aDisabled;
    bool faultLatched;
} BoardSafetyStatus;

void BoardSafety_init(void);
void BoardSafety_service(void);
void BoardSafety_stop(BoardSafetyStopReason reason);
bool BoardSafety_outputsSafe(void);
BoardSafetyStatus BoardSafety_snapshot(void);
bool BoardSafety_faultLatched(void);
const char *BoardSafety_buzzerPolicy(void);

#endif /* BSP_BOARD_SAFETY_H_ */
