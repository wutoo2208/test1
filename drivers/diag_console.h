#ifndef DRIVERS_DIAG_CONSOLE_H_
#define DRIVERS_DIAG_CONSOLE_H_

#include <stdint.h>

void DiagConsole_init(void);
void DiagConsole_service(void);
void DiagConsole_reportBoot(void);
void DiagConsole_onUartInterrupt(void);
uint32_t DiagConsole_rxOverflow(void);

#endif /* DRIVERS_DIAG_CONSOLE_H_ */
