#ifndef BSP_BOARD_SAFETY_H_
#define BSP_BOARD_SAFETY_H_

#include <stdbool.h>

void BoardSafety_init(void);
void BoardSafety_service(void);
bool BoardSafety_outputsLocked(void);
const char *BoardSafety_buzzerPolicy(void);

#endif /* BSP_BOARD_SAFETY_H_ */
