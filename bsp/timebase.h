#ifndef BSP_TIMEBASE_H_
#define BSP_TIMEBASE_H_

#include <stdbool.h>
#include <stdint.h>

void Timebase_init(void);
uint32_t Timebase_nowMs(void);
bool Timebase_reached(uint32_t nowMs, uint32_t deadlineMs);
void Timebase_onSysTick(void);

#endif /* BSP_TIMEBASE_H_ */
