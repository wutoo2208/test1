/*
 * Copyright (c) 2023, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 *  ============ ti_msp_dl_config.c =============
 *  Configured MSPM0 DriverLib module definitions
 *
 *  DO NOT EDIT - This file is generated for the MSPM0G350X
 *  by the SysConfig tool.
 */

#include "ti_msp_dl_config.h"

DL_TimerA_backupConfig gMOTOR_PWMBackup;
DL_TimerG_backupConfig gRIGHT_QEIBackup;
DL_TimerA_backupConfig gLEFT_CAPTUREBackup;
DL_SPI_backupConfig gRADIO_SPIBackup;

/*
 *  ======== SYSCFG_DL_init ========
 *  Perform any initialization needed before using any board APIs
 */
SYSCONFIG_WEAK void SYSCFG_DL_init(void)
{
    SYSCFG_DL_initPower();
    SYSCFG_DL_GPIO_init();
    /* Module-Specific Initializations*/
    SYSCFG_DL_SYSCTL_init();
    SYSCFG_DL_MOTOR_PWM_init();
    SYSCFG_DL_RIGHT_QEI_init();
    SYSCFG_DL_LEFT_CAPTURE_init();
    SYSCFG_DL_LINE_I2C_init();
    SYSCFG_DL_OLED_I2C_init();
    SYSCFG_DL_DIAG_UART_init();
    SYSCFG_DL_RADIO_SPI_init();
    /* Ensure backup structures have no valid state */
	gMOTOR_PWMBackup.backupRdy 	= false;
	gRIGHT_QEIBackup.backupRdy 	= false;
	gLEFT_CAPTUREBackup.backupRdy 	= false;

	gRADIO_SPIBackup.backupRdy 	= false;

}
/*
 * User should take care to save and restore register configuration in application.
 * See Retention Configuration section for more details.
 */
SYSCONFIG_WEAK bool SYSCFG_DL_saveConfiguration(void)
{
    bool retStatus = true;

	retStatus &= DL_TimerA_saveConfiguration(MOTOR_PWM_INST, &gMOTOR_PWMBackup);
	retStatus &= DL_TimerG_saveConfiguration(RIGHT_QEI_INST, &gRIGHT_QEIBackup);
	retStatus &= DL_TimerA_saveConfiguration(LEFT_CAPTURE_INST, &gLEFT_CAPTUREBackup);
	retStatus &= DL_SPI_saveConfiguration(RADIO_SPI_INST, &gRADIO_SPIBackup);

    return retStatus;
}


SYSCONFIG_WEAK bool SYSCFG_DL_restoreConfiguration(void)
{
    bool retStatus = true;

	retStatus &= DL_TimerA_restoreConfiguration(MOTOR_PWM_INST, &gMOTOR_PWMBackup, false);
	retStatus &= DL_TimerG_restoreConfiguration(RIGHT_QEI_INST, &gRIGHT_QEIBackup, false);
	retStatus &= DL_TimerA_restoreConfiguration(LEFT_CAPTURE_INST, &gLEFT_CAPTUREBackup, false);
	retStatus &= DL_SPI_restoreConfiguration(RADIO_SPI_INST, &gRADIO_SPIBackup);

    return retStatus;
}

SYSCONFIG_WEAK void SYSCFG_DL_initPower(void)
{
    DL_GPIO_reset(GPIOA);
    DL_GPIO_reset(GPIOB);
    DL_TimerA_reset(MOTOR_PWM_INST);
    DL_TimerG_reset(RIGHT_QEI_INST);
    DL_TimerA_reset(LEFT_CAPTURE_INST);
    DL_I2C_reset(LINE_I2C_INST);
    DL_I2C_reset(OLED_I2C_INST);
    DL_UART_Main_reset(DIAG_UART_INST);
    DL_SPI_reset(RADIO_SPI_INST);

    DL_GPIO_enablePower(GPIOA);
    DL_GPIO_enablePower(GPIOB);
    DL_TimerA_enablePower(MOTOR_PWM_INST);
    DL_TimerG_enablePower(RIGHT_QEI_INST);
    DL_TimerA_enablePower(LEFT_CAPTURE_INST);
    DL_I2C_enablePower(LINE_I2C_INST);
    DL_I2C_enablePower(OLED_I2C_INST);
    DL_UART_Main_enablePower(DIAG_UART_INST);
    DL_SPI_enablePower(RADIO_SPI_INST);
    delay_cycles(POWER_STARTUP_DELAY);
}

SYSCONFIG_WEAK void SYSCFG_DL_GPIO_init(void)
{

    DL_GPIO_initPeripheralOutputFunction(GPIO_MOTOR_PWM_C0_IOMUX,GPIO_MOTOR_PWM_C0_IOMUX_FUNC);
    DL_GPIO_enableOutput(GPIO_MOTOR_PWM_C0_PORT, GPIO_MOTOR_PWM_C0_PIN);
    DL_GPIO_initPeripheralOutputFunction(GPIO_MOTOR_PWM_C2_IOMUX,GPIO_MOTOR_PWM_C2_IOMUX_FUNC);
    DL_GPIO_enableOutput(GPIO_MOTOR_PWM_C2_PORT, GPIO_MOTOR_PWM_C2_PIN);

    DL_GPIO_initPeripheralInputFunction(GPIO_RIGHT_QEI_PHA_IOMUX,GPIO_RIGHT_QEI_PHA_IOMUX_FUNC);
    DL_GPIO_initPeripheralInputFunction(GPIO_RIGHT_QEI_PHB_IOMUX,GPIO_RIGHT_QEI_PHB_IOMUX_FUNC);

    DL_GPIO_initPeripheralInputFunction(GPIO_LEFT_CAPTURE_C0_IOMUX,GPIO_LEFT_CAPTURE_C0_IOMUX_FUNC);
    DL_GPIO_initPeripheralInputFunction(GPIO_LEFT_CAPTURE_C1_IOMUX,GPIO_LEFT_CAPTURE_C1_IOMUX_FUNC);

    DL_GPIO_initPeripheralInputFunctionFeatures(GPIO_LINE_I2C_IOMUX_SDA,
        GPIO_LINE_I2C_IOMUX_SDA_FUNC, DL_GPIO_INVERSION_DISABLE,
        DL_GPIO_RESISTOR_NONE, DL_GPIO_HYSTERESIS_DISABLE,
        DL_GPIO_WAKEUP_DISABLE);
    DL_GPIO_initPeripheralInputFunctionFeatures(GPIO_LINE_I2C_IOMUX_SCL,
        GPIO_LINE_I2C_IOMUX_SCL_FUNC, DL_GPIO_INVERSION_DISABLE,
        DL_GPIO_RESISTOR_NONE, DL_GPIO_HYSTERESIS_DISABLE,
        DL_GPIO_WAKEUP_DISABLE);
    DL_GPIO_enableHiZ(GPIO_LINE_I2C_IOMUX_SDA);
    DL_GPIO_enableHiZ(GPIO_LINE_I2C_IOMUX_SCL);
    DL_GPIO_initPeripheralInputFunctionFeatures(GPIO_OLED_I2C_IOMUX_SDA,
        GPIO_OLED_I2C_IOMUX_SDA_FUNC, DL_GPIO_INVERSION_DISABLE,
        DL_GPIO_RESISTOR_NONE, DL_GPIO_HYSTERESIS_DISABLE,
        DL_GPIO_WAKEUP_DISABLE);
    DL_GPIO_initPeripheralInputFunctionFeatures(GPIO_OLED_I2C_IOMUX_SCL,
        GPIO_OLED_I2C_IOMUX_SCL_FUNC, DL_GPIO_INVERSION_DISABLE,
        DL_GPIO_RESISTOR_NONE, DL_GPIO_HYSTERESIS_DISABLE,
        DL_GPIO_WAKEUP_DISABLE);
    DL_GPIO_enableHiZ(GPIO_OLED_I2C_IOMUX_SDA);
    DL_GPIO_enableHiZ(GPIO_OLED_I2C_IOMUX_SCL);

    DL_GPIO_initPeripheralOutputFunction(
        GPIO_DIAG_UART_IOMUX_TX, GPIO_DIAG_UART_IOMUX_TX_FUNC);
    DL_GPIO_initPeripheralInputFunction(
        GPIO_DIAG_UART_IOMUX_RX, GPIO_DIAG_UART_IOMUX_RX_FUNC);

    DL_GPIO_initPeripheralOutputFunction(
        GPIO_RADIO_SPI_IOMUX_SCLK, GPIO_RADIO_SPI_IOMUX_SCLK_FUNC);
    DL_GPIO_initPeripheralOutputFunction(
        GPIO_RADIO_SPI_IOMUX_PICO, GPIO_RADIO_SPI_IOMUX_PICO_FUNC);
    DL_GPIO_initPeripheralInputFunction(
        GPIO_RADIO_SPI_IOMUX_POCI, GPIO_RADIO_SPI_IOMUX_POCI_FUNC);

    DL_GPIO_initDigitalOutput(DIAG_GPIO_MOTOR_AIN2_SAFE_IOMUX);

    DL_GPIO_initDigitalOutput(DIAG_GPIO_MOTOR_BIN2_SAFE_IOMUX);

    DL_GPIO_initDigitalOutput(DIAG_GPIO_RADIO_CE_IOMUX);

    DL_GPIO_initDigitalOutput(DIAG_GPIO_RADIO_CSN_IOMUX);

    DL_GPIO_initDigitalInputFeatures(DIAG_GPIO_RADIO_IRQ_IOMUX,
		 DL_GPIO_INVERSION_DISABLE, DL_GPIO_RESISTOR_NONE,
		 DL_GPIO_HYSTERESIS_DISABLE, DL_GPIO_WAKEUP_DISABLE);

    DL_GPIO_initDigitalInputFeatures(DIAG_GPIO_START_BUTTON_IOMUX,
		 DL_GPIO_INVERSION_DISABLE, DL_GPIO_RESISTOR_PULL_UP,
		 DL_GPIO_HYSTERESIS_DISABLE, DL_GPIO_WAKEUP_DISABLE);

    DL_GPIO_initDigitalOutput(DIAG_GPIO_D36A_EN_SAFE_IOMUX);

    DL_GPIO_initDigitalOutput(DIAG_GPIO_D36A_DIR_SAFE_IOMUX);

    DL_GPIO_initDigitalOutput(DIAG_GPIO_D36A_STEP_SAFE_IOMUX);

    DL_GPIO_clearPins(GPIOA, DIAG_GPIO_D36A_DIR_SAFE_PIN |
		DIAG_GPIO_D36A_STEP_SAFE_PIN);
    DL_GPIO_enableOutput(GPIOA, DIAG_GPIO_D36A_DIR_SAFE_PIN |
		DIAG_GPIO_D36A_STEP_SAFE_PIN);
    DL_GPIO_clearPins(GPIOB, DIAG_GPIO_MOTOR_AIN2_SAFE_PIN |
		DIAG_GPIO_MOTOR_BIN2_SAFE_PIN |
		DIAG_GPIO_RADIO_CE_PIN |
		DIAG_GPIO_D36A_EN_SAFE_PIN);
    DL_GPIO_setPins(GPIOB, DIAG_GPIO_RADIO_CSN_PIN);
    DL_GPIO_enableOutput(GPIOB, DIAG_GPIO_MOTOR_AIN2_SAFE_PIN |
		DIAG_GPIO_MOTOR_BIN2_SAFE_PIN |
		DIAG_GPIO_RADIO_CE_PIN |
		DIAG_GPIO_RADIO_CSN_PIN |
		DIAG_GPIO_D36A_EN_SAFE_PIN);

}



SYSCONFIG_WEAK void SYSCFG_DL_SYSCTL_init(void)
{

	//Low Power Mode is configured to be SLEEP0
    DL_SYSCTL_setBORThreshold(DL_SYSCTL_BOR_THRESHOLD_LEVEL_0);

    
	DL_SYSCTL_setSYSOSCFreq(DL_SYSCTL_SYSOSC_FREQ_BASE);
	/* Set default configuration */
	DL_SYSCTL_disableHFXT();
	DL_SYSCTL_disableSYSPLL();

}


/*
 * Timer clock configuration to be sourced by  / 1 (32000000 Hz)
 * timerClkFreq = (timerClkSrc / (timerClkDivRatio * (timerClkPrescale + 1)))
 *   32000000 Hz = 32000000 Hz / (1 * (0 + 1))
 */
static const DL_TimerA_ClockConfig gMOTOR_PWMClockConfig = {
    .clockSel = DL_TIMER_CLOCK_BUSCLK,
    .divideRatio = DL_TIMER_CLOCK_DIVIDE_1,
    .prescale = 0U
};

static const DL_TimerA_PWMConfig gMOTOR_PWMConfig = {
    .pwmMode = DL_TIMER_PWM_MODE_EDGE_ALIGN,
    .period = 1600,
    .isTimerWithFourCC = true,
    .startTimer = DL_TIMER_STOP,
};

SYSCONFIG_WEAK void SYSCFG_DL_MOTOR_PWM_init(void) {

    DL_TimerA_setClockConfig(
        MOTOR_PWM_INST, (DL_TimerA_ClockConfig *) &gMOTOR_PWMClockConfig);

    DL_TimerA_initPWMMode(
        MOTOR_PWM_INST, (DL_TimerA_PWMConfig *) &gMOTOR_PWMConfig);

    // Set Counter control to the smallest CC index being used
    DL_TimerA_setCounterControl(MOTOR_PWM_INST,DL_TIMER_CZC_CCCTL0_ZCOND,DL_TIMER_CAC_CCCTL0_ACOND,DL_TIMER_CLC_CCCTL0_LCOND);

    DL_TimerA_setCaptureCompareOutCtl(MOTOR_PWM_INST, DL_TIMER_CC_OCTL_INIT_VAL_LOW,
		DL_TIMER_CC_OCTL_INV_OUT_DISABLED, DL_TIMER_CC_OCTL_SRC_FUNCVAL,
		DL_TIMERA_CAPTURE_COMPARE_0_INDEX);

    DL_TimerA_setCaptCompUpdateMethod(MOTOR_PWM_INST, DL_TIMER_CC_UPDATE_METHOD_IMMEDIATE, DL_TIMERA_CAPTURE_COMPARE_0_INDEX);
    DL_TimerA_setCaptureCompareValue(MOTOR_PWM_INST, 1600, DL_TIMER_CC_0_INDEX);

    DL_TimerA_setCaptureCompareOutCtl(MOTOR_PWM_INST, DL_TIMER_CC_OCTL_INIT_VAL_LOW,
		DL_TIMER_CC_OCTL_INV_OUT_DISABLED, DL_TIMER_CC_OCTL_SRC_FUNCVAL,
		DL_TIMERA_CAPTURE_COMPARE_2_INDEX);

    DL_TimerA_setCaptCompUpdateMethod(MOTOR_PWM_INST, DL_TIMER_CC_UPDATE_METHOD_IMMEDIATE, DL_TIMERA_CAPTURE_COMPARE_2_INDEX);
    DL_TimerA_setCaptureCompareValue(MOTOR_PWM_INST, 1600, DL_TIMER_CC_2_INDEX);

    DL_TimerA_enableClock(MOTOR_PWM_INST);


    
    DL_TimerA_setCCPDirection(MOTOR_PWM_INST , DL_TIMER_CC0_OUTPUT | DL_TIMER_CC2_OUTPUT );


}


static const DL_TimerG_ClockConfig gRIGHT_QEIClockConfig = {
    .clockSel = DL_TIMER_CLOCK_BUSCLK,
    .divideRatio = DL_TIMER_CLOCK_DIVIDE_1,
    .prescale = 0U
};


SYSCONFIG_WEAK void SYSCFG_DL_RIGHT_QEI_init(void) {

    DL_TimerG_setClockConfig(
        RIGHT_QEI_INST, (DL_TimerG_ClockConfig *) &gRIGHT_QEIClockConfig);

    DL_TimerG_configQEI(RIGHT_QEI_INST, DL_TIMER_QEI_MODE_2_INPUT,
        DL_TIMER_CC_INPUT_INV_NOINVERT, DL_TIMER_CC_0_INDEX);
    DL_TimerG_configQEI(RIGHT_QEI_INST, DL_TIMER_QEI_MODE_2_INPUT,
        DL_TIMER_CC_INPUT_INV_NOINVERT, DL_TIMER_CC_1_INDEX);
    DL_TimerG_setLoadValue(RIGHT_QEI_INST, 65535);
    DL_TimerG_enableInterrupt(RIGHT_QEI_INST , DL_TIMER_EVENT_DC_EVENT);

    DL_TimerG_enableClock(RIGHT_QEI_INST);
}



/*
 * Timer clock configuration to be sourced by BUSCLK /  (32000000 Hz)
 * timerClkFreq = (timerClkSrc / (timerClkDivRatio * (timerClkPrescale + 1)))
 *   32000000 Hz = 32000000 Hz / (1 * (0 + 1))
 */
static const DL_TimerA_ClockConfig gLEFT_CAPTUREClockConfig = {
    .clockSel    = DL_TIMER_CLOCK_BUSCLK,
    .divideRatio = DL_TIMER_CLOCK_DIVIDE_1,
    .prescale = 0U
};

/*
 * Timer load value (where the counter starts from) is calculated as (timerPeriod * timerClockFreq) - 1
 * LEFT_CAPTURE_INST_LOAD_VALUE = (0 ms * 32000000 Hz) - 1
 */

SYSCONFIG_WEAK void SYSCFG_DL_LEFT_CAPTURE_init(void) {

    DL_TimerA_setClockConfig(LEFT_CAPTURE_INST,
        (DL_TimerA_ClockConfig *) &gLEFT_CAPTUREClockConfig);

    DL_TimerA_setLoadValue(LEFT_CAPTURE_INST,-1);

    DL_TimerA_setCounterMode(LEFT_CAPTURE_INST,DL_TIMER_COUNT_MODE_UP);

    DL_TimerA_setCounterRepeatMode(LEFT_CAPTURE_INST,DL_TIMER_REPEAT_MODE_ENABLED);

    DL_TimerA_setCounterValueAfterEnable(LEFT_CAPTURE_INST,DL_TIMER_COUNT_AFTER_EN_ZERO);

    DL_TimerA_setCaptureCompareCtl(LEFT_CAPTURE_INST,
    DL_TIMER_CC_MODE_CAPTURE, (DL_TIMER_CC_ZCOND_NONE | DL_TIMER_CC_ACOND_TIMCLK | DL_TIMER_CC_CCOND_TRIG_EDGE),
    DL_TIMER_CC_0_INDEX);

    DL_TimerA_setCaptureCompareInput(LEFT_CAPTURE_INST,
        DL_TIMER_CC_INPUT_INV_NOINVERT,DL_TIMER_CC_IN_SEL_CCPX, DL_TIMER_CC_0_INDEX);

    DL_TimerA_setCaptureCompareCtl(LEFT_CAPTURE_INST,
    DL_TIMER_CC_MODE_CAPTURE, (DL_TIMER_CC_ZCOND_TRIG_FALL | DL_TIMER_CC_ACOND_TIMCLK | DL_TIMER_CC_CCOND_TRIG_EDGE),
    DL_TIMER_CC_1_INDEX);

    DL_TimerA_setCaptureCompareInput(LEFT_CAPTURE_INST,
        DL_TIMER_CC_INPUT_INV_NOINVERT,DL_TIMER_CC_IN_SEL_CCPX, DL_TIMER_CC_1_INDEX);


    DL_TimerA_setCounterControl(LEFT_CAPTURE_INST,
        DL_TIMER_CZC_CCCTL1_ZCOND,
        DL_TIMER_CAC_CCCTL1_ACOND,
        DL_TIMER_CLC_CCCTL1_LCOND
    );

    DL_TimerA_enableInterrupt(LEFT_CAPTURE_INST , DL_TIMERA_INTERRUPT_CC0_UP_EVENT |
		DL_TIMERA_INTERRUPT_CC1_UP_EVENT |
		DL_TIMERA_INTERRUPT_LOAD_EVENT);

    DL_TimerA_enableClock(LEFT_CAPTURE_INST);

}

static const DL_I2C_ClockConfig gLINE_I2CClockConfig = {
    .clockSel = DL_I2C_CLOCK_BUSCLK,
    .divideRatio = DL_I2C_CLOCK_DIVIDE_1,
};

SYSCONFIG_WEAK void SYSCFG_DL_LINE_I2C_init(void) {

    DL_I2C_setClockConfig(LINE_I2C_INST,
        (DL_I2C_ClockConfig *) &gLINE_I2CClockConfig);
    DL_I2C_disableAnalogGlitchFilter(LINE_I2C_INST);

    /* Configure Controller Mode */
    DL_I2C_resetControllerTransfer(LINE_I2C_INST);
    /* Set frequency to 100000 Hz*/
    DL_I2C_setTimerPeriod(LINE_I2C_INST, 31);
    DL_I2C_setControllerTXFIFOThreshold(LINE_I2C_INST, DL_I2C_TX_FIFO_LEVEL_EMPTY);
    DL_I2C_setControllerRXFIFOThreshold(LINE_I2C_INST, DL_I2C_RX_FIFO_LEVEL_BYTES_1);
    DL_I2C_enableControllerClockStretching(LINE_I2C_INST);


    /* Enable module */
    DL_I2C_enableController(LINE_I2C_INST);


}
static const DL_I2C_ClockConfig gOLED_I2CClockConfig = {
    .clockSel = DL_I2C_CLOCK_BUSCLK,
    .divideRatio = DL_I2C_CLOCK_DIVIDE_1,
};

SYSCONFIG_WEAK void SYSCFG_DL_OLED_I2C_init(void) {

    DL_I2C_setClockConfig(OLED_I2C_INST,
        (DL_I2C_ClockConfig *) &gOLED_I2CClockConfig);
    DL_I2C_disableAnalogGlitchFilter(OLED_I2C_INST);

    /* Configure Controller Mode */
    DL_I2C_resetControllerTransfer(OLED_I2C_INST);
    /* Set frequency to 100000 Hz*/
    DL_I2C_setTimerPeriod(OLED_I2C_INST, 31);
    DL_I2C_setControllerTXFIFOThreshold(OLED_I2C_INST, DL_I2C_TX_FIFO_LEVEL_EMPTY);
    DL_I2C_setControllerRXFIFOThreshold(OLED_I2C_INST, DL_I2C_RX_FIFO_LEVEL_BYTES_1);
    DL_I2C_enableControllerClockStretching(OLED_I2C_INST);


    /* Enable module */
    DL_I2C_enableController(OLED_I2C_INST);


}

static const DL_UART_Main_ClockConfig gDIAG_UARTClockConfig = {
    .clockSel    = DL_UART_MAIN_CLOCK_BUSCLK,
    .divideRatio = DL_UART_MAIN_CLOCK_DIVIDE_RATIO_1
};

static const DL_UART_Main_Config gDIAG_UARTConfig = {
    .mode        = DL_UART_MAIN_MODE_NORMAL,
    .direction   = DL_UART_MAIN_DIRECTION_TX_RX,
    .flowControl = DL_UART_MAIN_FLOW_CONTROL_NONE,
    .parity      = DL_UART_MAIN_PARITY_NONE,
    .wordLength  = DL_UART_MAIN_WORD_LENGTH_8_BITS,
    .stopBits    = DL_UART_MAIN_STOP_BITS_ONE
};

SYSCONFIG_WEAK void SYSCFG_DL_DIAG_UART_init(void)
{
    DL_UART_Main_setClockConfig(DIAG_UART_INST, (DL_UART_Main_ClockConfig *) &gDIAG_UARTClockConfig);

    DL_UART_Main_init(DIAG_UART_INST, (DL_UART_Main_Config *) &gDIAG_UARTConfig);
    /*
     * Configure baud rate by setting oversampling and baud rate divisors.
     *  Target baud rate: 115200
     *  Actual baud rate: 115211.52
     */
    DL_UART_Main_setOversampling(DIAG_UART_INST, DL_UART_OVERSAMPLING_RATE_16X);
    DL_UART_Main_setBaudRateDivisor(DIAG_UART_INST, DIAG_UART_IBRD_32_MHZ_115200_BAUD, DIAG_UART_FBRD_32_MHZ_115200_BAUD);


    /* Configure Interrupts */
    DL_UART_Main_enableInterrupt(DIAG_UART_INST,
                                 DL_UART_MAIN_INTERRUPT_RX);

    /* Configure FIFOs */
    DL_UART_Main_enableFIFOs(DIAG_UART_INST);
    DL_UART_Main_setRXFIFOThreshold(DIAG_UART_INST, DL_UART_RX_FIFO_LEVEL_ONE_ENTRY);
    DL_UART_Main_setTXFIFOThreshold(DIAG_UART_INST, DL_UART_TX_FIFO_LEVEL_1_2_EMPTY);

    DL_UART_Main_enable(DIAG_UART_INST);
}

static const DL_SPI_Config gRADIO_SPI_config = {
    .mode        = DL_SPI_MODE_CONTROLLER,
    .frameFormat = DL_SPI_FRAME_FORMAT_MOTO3_POL0_PHA0,
    .parity      = DL_SPI_PARITY_NONE,
    .dataSize    = DL_SPI_DATA_SIZE_8,
    .bitOrder    = DL_SPI_BIT_ORDER_MSB_FIRST,
};

static const DL_SPI_ClockConfig gRADIO_SPI_clockConfig = {
    .clockSel    = DL_SPI_CLOCK_BUSCLK,
    .divideRatio = DL_SPI_CLOCK_DIVIDE_RATIO_1
};

SYSCONFIG_WEAK void SYSCFG_DL_RADIO_SPI_init(void) {
    DL_SPI_setClockConfig(RADIO_SPI_INST, (DL_SPI_ClockConfig *) &gRADIO_SPI_clockConfig);

    DL_SPI_init(RADIO_SPI_INST, (DL_SPI_Config *) &gRADIO_SPI_config);

    /* Configure Controller mode */
    /*
     * Set the bit rate clock divider to generate the serial output clock
     *     outputBitRate = (spiInputClock) / ((1 + SCR) * 2)
     *     500000 = (32000000)/((1 + 31) * 2)
     */
    DL_SPI_setBitRateSerialClockDivider(RADIO_SPI_INST, 31);
    /* Set RX and TX FIFO threshold levels */
    DL_SPI_setFIFOThreshold(RADIO_SPI_INST, DL_SPI_RX_FIFO_LEVEL_1_2_FULL, DL_SPI_TX_FIFO_LEVEL_1_2_EMPTY);

    /* Enable module */
    DL_SPI_enable(RADIO_SPI_INST);
}

