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
#define REQ002_TRACKING_FAULT_CONFIRM_MS     (100U)
#define REQ002_TRACKING_RECOVERY_LEFT_PERMILLE  (600U)
#define REQ002_TRACKING_RECOVERY_RIGHT_PERMILLE (290U)

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
/* Allow one complete 1 kHz PWM period for IN1 to become continuously high
 * before IN2 is asserted high for DRV8870 1/1 electrical braking. */
#define REQ002_FINISH_BRAKE_PREPARE_MS         (1U)
#define REQ002_MARKER_MIN_BLACK              (4U)
#define REQ002_BASE_PULSE_PERMILLE           (850U)
#define REQ002_RIGHT_TRIM_PERMILLE            (420U)
#define REQ002_RIGHT_CURVE_SLOWDOWN_PERMILLE  (220U)
#define REQ002_LEFT_CURVE_SLOWDOWN_PERMILLE   (100U)
#define REQ002_MAX_PULSE_PERMILLE             (900U)
#define REQ002_TURN_MAX_PULSE_PERMILLE        (1000U)
#define REQ002_TURN_RIGHT_MIN_PULSE_PERMILLE   (420U)
#define REQ002_RIGHT_TURN_LEFT_BASE_MAX_PERMILLE (880U)
#define REQ002_TURN_BASE_PULSE_PERMILLE        (650U)
#define REQ002_RIGHT_TURN_PULSE_PERMILLE      (1340U)
#define REQ002_LEFT_TURN_PULSE_PERMILLE       (400U)
#define REQ002_TURN_MIN_CORRECTION             (0.25f)

/* Clockwise-only course optimization. The approach stage slows the vehicle
 * before the half-circle, the arc stage latches through a transient centered
 * sample at the apex, and recovery ramps back to straight travel. */
#define REQ002_RIGHT_APPROACH_ENTER_ERROR          (0.06f)
#define REQ002_RIGHT_APPROACH_CONFIRM_MS           (10U)
#define REQ002_RIGHT_APPROACH_MAX_MS              (150U)
#define REQ002_RIGHT_APPROACH_LEFT_PERMILLE       (820U)
#define REQ002_RIGHT_APPROACH_RIGHT_PERMILLE      (420U)
#define REQ002_RIGHT_ARC_ENTER_ERROR               (0.10f)
#define REQ002_RIGHT_ARC_CONFIRM_MS                 (5U)
#define REQ002_RIGHT_ARC_EXIT_ERROR                (0.04f)
#define REQ002_RIGHT_ARC_MIN_MS                   (300U)
#define REQ002_RIGHT_ARC_MAX_MS                   (700U)
#define REQ002_RIGHT_ARC_RECENTER_MS               (60U)
#define REQ002_RIGHT_ARC_LEFT_PERMILLE            (800U)
#define REQ002_RIGHT_ARC_RIGHT_PERMILLE             (0U)
/* Strict v1-style severe-right trigger. Right-wheel braking is one-shot per
 * latched curve and returns to the normal 80/0 arc after the bounded pulse. */
#define REQ002_RIGHT_SHARP_ENTER_ERROR             (0.15f)
#define REQ002_RIGHT_SHARP_CONFIRM_MS              (10U)
#define REQ002_RIGHT_SHARP_PREPARE_MS               (1U)
#define REQ002_RIGHT_SHARP_BRAKE_MS               (100U)
#define REQ002_RIGHT_SHARP_LEFT_PERMILLE          (800U)
#define REQ002_RIGHT_RECOVER_MS                   (200U)
#define REQ002_RIGHT_RECOVER_LEFT_START_PERMILLE  (800U)
#define REQ002_RIGHT_RECOVER_RIGHT_START_PERMILLE (450U)
#define REQ002_RIGHT_CURVE_LINE_LOSS_HOLD_MS      (100U)

/* Initial straight-line wheel-speed PI candidate. The left GPIO path counts
 * one A rising edge per encoder cycle; the right hardware QEI counts 4x. */
#define REQ002_SPEED_PI_ENABLED                REQ002_ACTUATION_BUILD
#define REQ002_LEFT_ENCODER_TO_QEI_SCALE       (4.0f)
#define REQ002_LEFT_SPEED_TARGET_RATIO          (1.20f)
#define REQ002_SPEED_PI_STRAIGHT_THRESHOLD     (0.10f)
#define REQ002_SPEED_PI_MIN_DEMAND_PERMILLE    (250U)
#define REQ002_SPEED_PI_KP                     (4.0f)
#define REQ002_SPEED_PI_KI                     (12.0f)
#define REQ002_SPEED_PI_OUTPUT_LIMIT           (50.0f)
#define REQ002_SPEED_PI_INTEGRAL_LIMIT         (4.0f)
#define REQ002_ENCODER_FEEDBACK_FAULT_MS        (500U)

#endif /* CONFIG_FIRMWARE_CONFIG_H_ */
