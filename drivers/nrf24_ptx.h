#ifndef DRIVERS_NRF24_PTX_H_
#define DRIVERS_NRF24_PTX_H_

#include <stdbool.h>
#include <stdint.h>

typedef struct {
    const char *stateName;
    bool armed;
    uint8_t queued;
    uint32_t txSuccess;
    uint32_t maxRetry;
    uint32_t txTimeout;
    uint32_t spiErrors;
    uint32_t initErrors;
    uint32_t droppedLines;
} Nrf24PtxStatus;

typedef struct {
    bool ok;
    uint8_t config;
    uint8_t channel;
    uint8_t rfSetup;
} Nrf24PtxRegisters;

void Nrf24Ptx_init(void);
void Nrf24Ptx_service(void);
void Nrf24Ptx_captureDiagnosticChar(char value);
bool Nrf24Ptx_arm(void);
void Nrf24Ptx_disarm(void);
Nrf24PtxStatus Nrf24Ptx_getStatus(void);
Nrf24PtxRegisters Nrf24Ptx_readCoreRegisters(void);
bool Nrf24Ptx_safeWhenDisarmed(void);

#endif /* DRIVERS_NRF24_PTX_H_ */
