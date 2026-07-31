#ifndef CONFIG_FIRMWARE_CONFIG_H_
#define CONFIG_FIRMWARE_CONFIG_H_

#define FW_VERSION                         "req002-safe-0.5"
#define PIN_PLAN_VERSION                   "1.5"

#define DIAG_UART_RX_BUFFER_SIZE           (128U)
#define DIAG_COMMAND_BUFFER_SIZE           (80U)
#define DIAG_LINE_MIRROR_SIZE              (192U)

#define I2C_DIAG_TIMEOUT_LOOPS             (200000U)
#define LINE_SENSOR_POLL_PERIOD_MS         (5U)
#define LINE_SENSOR_RETRY_PERIOD_MS        (50U)
#define LINE_TRACKING_CENTER_OFFSET        (0.0821f)
#define LINE_TRACKING_MIN_SIGNAL_SUM       (0.35f)
#define LINE_TRACKING_SHADOW_KP            (0.80f)
#define LINE_TRACKING_SHADOW_KI            (0.0f)
#define LINE_TRACKING_SHADOW_KD            (0.015f)
#define RADIO_SPI_TIMEOUT_LOOPS            (200000U)
#define RADIO_TX_QUEUE_SLOTS               (16U)
#define RADIO_MAX_PAYLOAD                  (32U)
#define RADIO_TX_TIMEOUT_MS                (100U)
#define RADIO_POR_DELAY_MS                 (100U)
#define RADIO_POWERUP_DELAY_MS             (2U)

/* Approved working-copy Baoqian one-shot profile. Do not restore these to 0. */
#define RADIO_ALLOW_TX                     (1U)
#define RADIO_PROFILE_VALID                (1U)
#define RADIO_AUTO_ARM                     (1U)
#define RADIO_ONE_SHOT_TEST                (1U)

#define START_BUTTON_DEBOUNCE_MS            (30U)
#define REQ002_TIMEOUT_MS                   (20000U)

/*
 * REQ-002 remains actuator-locked. These gates are deliberately false until
 * calibration evidence, physical parameters, and a separately approved motor
 * adapter exist. No module in this firmware can create a motor command.
 */
#define REQ002_CALIBRATION_VALID            (0U)
#define REQ002_ACTUATION_GATE_VALID          (0U)
#define REQ002_PHYSICAL_PARAMETERS_VALID     (0U)
#define REQ002_ACTUATOR_ADAPTER_ENABLED      (0U)

/* Unknown control values stay invalid/zero rather than being guessed. */
#define REQ002_PID_ENABLED                   (0U)
#define REQ002_PID_KP                        (0.0f)
#define REQ002_PID_KI                        (0.0f)
#define REQ002_PID_KD                        (0.0f)
#define REQ002_PID_OUTPUT_MIN                (0.0f)
#define REQ002_PID_OUTPUT_MAX                (0.0f)
#define REQ002_PID_INTEGRAL_MIN              (0.0f)
#define REQ002_PID_INTEGRAL_MAX              (0.0f)
#define REQ002_PID_DERIVATIVE_ALPHA          (0.0f)

#endif /* CONFIG_FIRMWARE_CONFIG_H_ */
