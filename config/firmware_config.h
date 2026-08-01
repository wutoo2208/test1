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
#define LINE_SENSOR_STARTUP_DELAY_MS       (200U)
#define LINE_SENSOR_BUS_RECOVERY_PULSES    (9U)
#define LINE_SENSOR_BUS_RECOVERY_HALF_PERIOD_CYCLES (160U)
#define LINE_SENSOR_BUS_RECOVERY_WAIT_LOOPS (10000U)
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

#ifndef MOTOR_SELFTEST_BUILD
#define MOTOR_SELFTEST_BUILD                (0U)
#endif

#if MOTOR_SELFTEST_BUILD
#define MOTOR_TEST_DUTY_PERMILLE            (1000U)
#define MOTOR_TEST_DURATION_MS              (120U)
#endif

#define RIGHT_ENCODER_COUNTS_PER_REV         (1650U)

/*
 * Default Debug remains actuator-locked. The isolated MotorSelfTest build is
 * the only build allowed to exercise the REQ-002 adapter during bring-up.
 */
#ifndef REQ002_ACTUATION_BUILD
#define REQ002_ACTUATION_BUILD              MOTOR_SELFTEST_BUILD
#endif

#define REQ002_CALIBRATION_VALID            REQ002_ACTUATION_BUILD
#define REQ002_ACTUATION_GATE_VALID          REQ002_ACTUATION_BUILD
#define REQ002_PHYSICAL_PARAMETERS_VALID     REQ002_ACTUATION_BUILD
#define REQ002_ACTUATOR_ADAPTER_ENABLED      REQ002_ACTUATION_BUILD
#define REQ002_PID_ENABLED                   REQ002_ACTUATION_BUILD

/* Reuse the verified LineTracking shadow PID; do not instantiate another PID. */
#define REQ002_PID_KP                        LINE_TRACKING_SHADOW_KP
#define REQ002_PID_KI                        LINE_TRACKING_SHADOW_KI
#define REQ002_PID_KD                        LINE_TRACKING_SHADOW_KD
#define REQ002_PID_OUTPUT_MIN                (-1.0f)
#define REQ002_PID_OUTPUT_MAX                (1.0f)
#define REQ002_PID_INTEGRAL_MIN              (-0.5f)
#define REQ002_PID_INTEGRAL_MAX              (0.5f)
#define REQ002_PID_DERIVATIVE_ALPHA          (0.8f)

/* Initial bench-only actuator parameters; ground tuning is separately gated. */
#define REQ002_CONTROL_PERIOD_MS             (5U)
#define REQ002_SOFT_START_MS                 (300U)
#define REQ002_DEPART_CONFIRM_MS             (50U)
#define REQ002_MARKER_CONFIRM_MS             (50U)
#define REQ002_MARKER_MIN_BLACK              (4U)
#define REQ002_BASE_PULSE_PERMILLE           (600U)
#define REQ002_RIGHT_TRIM_PERMILLE            (60U)
#define REQ002_RIGHT_CURVE_SLOWDOWN_PERMILLE  (220U)
#define REQ002_LEFT_CURVE_SLOWDOWN_PERMILLE   (100U)
#define REQ002_MAX_PULSE_PERMILLE             (800U)
#define REQ002_RIGHT_TURN_PULSE_PERMILLE      (350U)
#define REQ002_LEFT_TURN_PULSE_PERMILLE       (150U)

#endif /* CONFIG_FIRMWARE_CONFIG_H_ */
