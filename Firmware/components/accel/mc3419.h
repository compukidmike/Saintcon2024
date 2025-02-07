#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

// Chip ID of the MC3419 accelerometer - fixed value of 0xA4
#define MC3419_CHIP_ID 0xA4

// I2C address of the MC3419 accelerometer
#define MC3419_I2C_ADDR 0x4C

// MC3419 registers enum
typedef enum {
    MC3419_REG_DEV_STAT              = 0x05, // [RO] Device Status
    MC3419_REG_INTR_CTRL             = 0x06, // [RW] Interrupt Enable
    MC3419_REG_MODE                  = 0x07, // [RW] Mode
    MC3419_REG_SR                    = 0x08, // [RW] Sample Rate
    MC3419_REG_MOTION_CTRL           = 0x09, // [RW] Motion Control
    MC3419_REG_FIFO_STAT             = 0x0A, // [RO] FIFO Status
    MC3419_REG_FIFO_RD_P             = 0x0B, // [RO] FIFO Read Pointer
    MC3419_REG_FIFO_WR_P             = 0x0C, // [RO] FIFO Write Pointer
    MC3419_REG_XOUT_EX_L             = 0x0D, // [RO] X-axis output LSB
    MC3419_REG_XOUT_EX_H             = 0x0E, // [RO] X-axis output MSB
    MC3419_REG_YOUT_EX_L             = 0x0F, // [RO] Y-axis output LSB
    MC3419_REG_YOUT_EX_H             = 0x10, // [RO] Y-axis output MSB
    MC3419_REG_ZOUT_EX_L             = 0x11, // [RO] Z-axis output LSB
    MC3419_REG_ZOUT_EX_H             = 0x12, // [RO] Z-axis output MSB
    MC3419_REG_STATUS                = 0x13, // [RO] Status
    MC3419_REG_INTR_STAT             = 0x14, // [RO] Interrupt Status
    MC3419_REG_CHIP_ID               = 0x18, // [RO] Chip ID
    MC3419_REG_RANGE                 = 0x20, // [RW] Range Select Control
    MC3419_REG_XOFFL                 = 0x21, // [RW] X-axis offset LSB
    MC3419_REG_XOFFH                 = 0x22, // [RW] X-axis offset MSB
    MC3419_REG_YOFFL                 = 0x23, // [RW] Y-axis offset LSB
    MC3419_REG_YOFFH                 = 0x24, // [RW] Y-axis offset MSB
    MC3419_REG_ZOFFL                 = 0x25, // [RW] Z-axis offset LSB
    MC3419_REG_ZOFFH                 = 0x26, // [RW] Z-axis offset MSB
    MC3419_REG_XGAIN                 = 0x27, // [RW] X-axis gain
    MC3419_REG_YGAIN                 = 0x28, // [RW] Y-axis gain
    MC3419_REG_ZGAIN                 = 0x29, // [RW] Z-axis gain
    MC3419_REG_FIFO_CTRL             = 0x2D, // [RW] FIFO Control
    MC3419_REG_FIFO_TH               = 0x2E, // [RW] FIFO Threshold
    MC3419_REG_FIFO_INTR             = 0x2F, // [RW] FIFO Interrupt Status
    MC3419_REG_FIFO_CTRL2_SR2        = 0x30, // [RW] FIFO Control 2 / Sample Rate 2
    MC3419_REG_COMM_CTRL             = 0x31, // [RW] Communication Control
    MC3419_REG_GPIO_CTRL             = 0x33, // [RW] GPIO Control
    MC3419_REG_TF_THRESH_LSB         = 0x40, // [RW] Tilt/Flip Threshold LSB
    MC3419_REG_TF_THRESH_MSB         = 0x41, // [RW] Tilt/Flip Threshold MSB
    MC3419_REG_TF_DB                 = 0x42, // [RW] Tilt/Flip Debounce
    MC3419_REG_AM_THRESH_LSB         = 0x43, // [RW] AnyMotion Threshold LSB
    MC3419_REG_AM_THRESH_MSB         = 0x44, // [RW] AnyMotion Threshold MSB
    MC3419_REG_AM_DB                 = 0x45, // [RW] AnyMotion Debounce
    MC3419_REG_SHK_THRESH_LSB        = 0x46, // [RW] Shake Threshold LSB
    MC3419_REG_SHK_THRESH_MSB        = 0x47, // [RW] Shake Threshold MSB
    MC3419_REG_PK_P2P_DUR_THRESH_LSB = 0x48, // [RW] Peak-to-Peak Duration Threshold LSB
    MC3419_REG_PK_P2P_DUR_THRESH_MSB = 0x49, // [RW] Shake Duration and Peak-to-Peak Duration Threshold MSB
    MC3419_REG_TIMER_CTRL            = 0x4A, // [RW] Timer Control
    MC3419_REG_RD_CNT                = 0x4B  // [RW] Read Count
} mc3419_register_t;

/**
 * DEV_STAT (R) Register definitions
 */

typedef enum {
    MODE_SLEEP   = 0b00,
    MODE_WAKE    = 0b01,
    MODE_STANDBY = 0b11,
} mc3419_mode_state_t;

typedef union {
    uint8_t raw;
    struct {
        mc3419_mode_state_t state : 2; // 00: Sleep, 01: Wake, 10: Reserved, 11: Standby
        bool res_mode : 1;             // 0: 16-bit (high) resolution, 1: reserved
        uint8_t _reserved0 : 1;
        bool i2c_wdt : 1;       // 0: No I2C watchdog event detected, 1: I2C watchdog event detected and state machine is in reset
        uint8_t _reserved1 : 2; //
        bool otp_busy : 1;      // 0: Internal memory idle - device ready, 1: Internal memory active - device not ready
    };
} mc3419_dev_stat_t;

/**
 * INTR_CTRL (W) Register definition
 */

typedef enum {
    MC3419_INT_TILT    = 0x01, // Tilt interrupt
    MC3419_INT_FLIP    = 0x02, // Flip interrupt
    MC3419_INT_ANYM    = 0x04, // AnyMotion interrupt
    MC3419_INT_SHAKE   = 0x08, // Shake interrupt
    MC3419_INT_TILT_35 = 0x10, // Tilt > 35° interrupt
    MC3419_INT_ACQ     = 0x20, // Sample interrupt
} mc3419_interrupt_t;

typedef union {
    uint8_t raw;
    struct {
        bool tilt_int_en : 1;    // 0: Tilt interrupt disabled, 1: Tilt interrupt enabled
        bool flip_int_en : 1;    // 0: Flip interrupt disabled, 1: Flip interrupt enabled
        bool anym_int_en : 1;    // 0: AnyMotion interrupt disabled, 1: AnyMotion interrupt enabled
        bool shake_int_en : 1;   // 0: Shake interrupt disabled, 1: Shake interrupt enabled
        bool tilt_35_int_en : 1; // 0: Tilt > 35° interrupt disabled, 1: Tilt > 35° interrupt enabled
        bool auto_clr_en : 1; // 0: Clear pending interrupts by writing to register 0x14, 1: Interrupts automatically cleared if
                              // the interrupt condition is no longer present
        bool acq_int_en : 1;  // 0: Disable automatic interrupt after each sample (default), 1: Enable automatic interrupt after
                              // each sample
    };
} mc3419_intr_ctrl_t;

/**
 * MODE (W) Register definition
 */

typedef union {
    uint8_t raw;
    struct {
        mc3419_mode_state_t state : 2; // 00: Sleep, 01: Wake, 10: Reserved, 11: Standby
        uint8_t _reserved0 : 2;
        bool i2c_wdt_neg : 1; // 0: The I2C watchdog timer for negative SCL stalls is disabled (default), 1: The I2C watchdog
                              // timer for negative SCL stalls is enabled
        bool i2c_wdt_pos : 1; // 0: The I2C watchdog timer for positive SCL stalls is disabled (default), 1: The I2C watchdog
                              // timer for positive SCL stalls is enabled
        uint8_t _reserved1 : 2;
    };
} mc3419_mode_t;

/**
 * SR (RW) Register definition
 */

// typedef enum {
//     // Interface: I2C or SPI
//     // I2C Speed: <= 1MHz
//     // SPI Speed: <= 4MHz
//     ODR_I2C_SPI_25_HZ   = 0x10, // Rate 0, 25Hz
//     ODR_I2C_SPI_50_HZ   = 0x11, // Rate 1, 50Hz
//     ODR_I2C_SPI_62_5_HZ = 0x12, // Rate 2, 62.5Hz
//     ODR_I2C_SPI_100_HZ  = 0x13, // Rate 3, 100Hz
//     ODR_I2C_SPI_125_HZ  = 0x14, // Rate 4, 125Hz
//     ODR_I2C_SPI_250_HZ  = 0x15, // Rate 5, 250Hz
//     ODR_I2C_SPI_500_HZ  = 0x16, // Rate 6, 500Hz
//     ODR_I2C_SPI_1000_HZ = 0x17, // Rate 7, 1000Hz

//     // Interface: SPI Only
//     // SPI Speed: 4MHz <= SPI Speed <= 10MHz
//     ODR_SPI_ONLY_50_HZ   = 0x08, // Rate 0, 50Hz
//     ODR_SPI_ONLY_62_5_HZ = 0x09, // Rate 1, 62.5Hz
//     ODR_SPI_ONLY_100_HZ  = 0x0A, // Rate 2, 100Hz
//     ODR_SPI_ONLY_125_HZ  = 0x0B, // Rate 3, 125Hz
//     ODR_SPI_ONLY_250_HZ  = 0x0C, // Rate 4, 250Hz
//     ODR_SPI_ONLY_500_HZ  = 0x0D, // Rate 5, 500Hz
//     ODR_SPI_ONLY_1000_HZ = 0x0E, // Rate 6, 1000Hz
// } mc3419_odr_t;

typedef enum {
    RATE_50_HZ   = 0x08, // Rate 0, 50Hz
    RATE_100_HZ  = 0x09, // Rate 1, 100Hz
    RATE_125_HZ  = 0x0A, // Rate 2, 125Hz
    RATE_200_HZ  = 0x0B, // Rate 3, 200Hz
    RATE_250_HZ  = 0x0C, // Rate 4, 250Hz
    RATE_500_HZ  = 0x0D, // Rate 5, 500Hz
    RATE_1000_HZ = 0x0E, // Rate 6, 1000Hz
    RATE_2000_HZ = 0x0F, // Rate 7, 2000Hz
} mc3419_rate_t;

typedef union {
    uint8_t raw;
    struct {
        mc3419_rate_t rate : 5; // Output data rate (ODR) selection
        uint8_t _reserved0 : 3;
    };
} mc3419_sr_t;

/**
 * MOTION_CTRL (RW) Register definition
 */

typedef enum {
    MC3419_FEATURE_TF      = 0x01, // Tilt/Flip feature enabled
    MC3419_FEATURE_ANYM    = 0x02, // AnyMotion feature enabled
    MC3419_FEATURE_SHAKE   = 0x04, // Shake feature enabled
    MC3419_FEATURE_TILT_35 = 0x08, // Tilt > 35° feature enabled
} mc3419_feature_t;

typedef union {
    uint8_t raw;
    struct {
        bool tf_enable : 1;     // 0: Tilt/Flip feature disabled, 1: Tilt/Flip feature enabled
        bool motion_latch : 1;  // 0: Motion block does not latch outputs, 1: Motion block latches outputs
        bool anym_en : 1;       // 0: AnyMotion feature disabled (default), 1: AnyMotion feature enabled
        bool shake_en : 1;      // 0: Shake feature disabled (default), 1: Shake feature enabled
        bool tilt_35_en : 1;    // 0: Tilt > 35° feature disabled (default), 1: Tilt > 35° feature enabled
        bool z_axis_ort : 1;    // 0: Z-axis orientation is positive up (default), 1: Z-axis orientation is positive down
        bool raw_proc_stat : 1; // 0: Motion flag bits are filtered by debouce and other settings (default), 1: Motion flag bits
                                // are real-time raw data
        bool motion_reset : 1;  // 0: Motion block is not in reset (default), 1: Motion block is held in reset. The software must
                                // set this bit to clear the reset.
    };
} mc3419_motion_ctrl_t;

/**
 * FIFO_STAT (R) Register definition
 */

typedef union {
    uint8_t raw;
    struct {
        bool fifo_empty : 1;  // 0: FIFO is not empty, 1: FIFO is empty
        bool fifo_full : 1;   // 0: FIFO is not full, 1: FIFO is full
        bool fifo_thresh : 1; // 0: FIFO threshold is less than the threshold setting (default), 1: FIFO threshold is greater
                              // than or equal to the threshold setting.
                              // NOTE: default threshold level is 16 or 1/2 of the 32 sample FIFO capacity.
        uint8_t _reserved0 : 5;
    };
} mc3419_fifo_stat_t;

/**
 * FIFO_RD_P (R) Register definition
 */

typedef union {
    uint8_t raw;
    struct {
        uint8_t fifo_rd_p : 5;    // FIFO read pointer (0-31) - current address the FIFO read pointer is accessing
        uint8_t fifo_rd_p_hw : 1; // Used by hardware to manage the full/empty status of the FIFO - not a physical address bit
        uint8_t _reserved0 : 2;
    };
} mc3419_fifo_rd_p_t;

/**
 * FIFO_WR_P (R) Register definition
 */

typedef union {
    uint8_t raw;
    struct {
        uint8_t fifo_wr_p : 5;    // FIFO write pointer (0-31) - current address the FIFO write pointer is accessing
        uint8_t fifo_wr_p_hw : 1; // Used by hardware to manage the full/empty status of the FIFO - not a physical address bit
        uint8_t _reserved0 : 2;
    };
} mc3419_fifo_wr_p_t;

/**
 * XOUT_EX_L, XOUT_EX_H, YOUT_EX_L, YOUT_EX_H, ZOUT_EX_L, ZOUT_EX_H (R) Registers definition
 *     INFO: X, Y, and Z-axis accelerometer measurements are in 16-bit, signed 2’s complement format. Register addresses 0x0D to
 *           0x12 hold the latest sampled data from the X, Y, and Z accelerometers. When the FIFO is enabled (register 0x2D bit
 *           5), reading from address 0x0D supplies data from the FIFO instead of the output registers.
 */

typedef union {
    uint16_t raw;
    struct {
        uint8_t lsb;
        uint8_t msb;
    };
    struct {
        int16_t value;
    };
} mc3419_axis_data_t;

typedef struct {
    mc3419_axis_data_t x;
    mc3419_axis_data_t y;
    mc3419_axis_data_t z;
} mc3419_accel_data_t;

// Accelerometer range in m/s² - ±2g, ±4g, ±8g, ±16g, ±12g
extern const float mc3419_range_si[];
// Accelerometer resolution in m/s²
extern const float mc3419_resolution_si;
// Convert raw accelerometer data to m/s²
#define MC3419_RAW_TO_SI(raw, range) ((float)(raw) * mc3419_range_si[range] / mc3419_resolution_si)
// Convert m/s² to g's
#define SI_TO_G(si) ((si) / 9.80665f)

/**
 * STATUS (R) Register definition
 */

typedef union {
    uint8_t raw;
    struct {
        bool tilt : 1;    // 0: No tilt detected, 1: Tilt detected
        bool flip : 1;    // 0: No flip detected, 1: Flip detected
        bool anym : 1;    // 0: No AnyMotion detected, 1: AnyMotion detected
        bool shake : 1;   // 0: No shake detected, 1: Shake detected
        bool tilt_35 : 1; // 0: No tilt > 35° detected, 1: Tilt > 35° detected
        bool fifo : 1;    // OR of FIFO flags from register 0x0A (FIFO_FULL, FIFO_THRESH, FIFO_EMPTY)
        uint8_t _reserved0 : 1;
        bool new_data : 1; // 0: No new data since last read, 1: New data in output registers (0x0D-0x12)
    };
} mc3419_status_t;

/**
 * INTR_STAT (RW) Register definition
 */

typedef union {
    uint8_t raw;
    struct {
        bool tilt_int : 1;    // 0: No tilt interrupt pending, 1: Tilt interrupt pending
        bool flip_int : 1;    // 0: No flip interrupt pending, 1: Flip interrupt pending
        bool anym_int : 1;    // 0: No AnyMotion interrupt pending, 1: AnyMotion interrupt pending
        bool shake_int : 1;   // 0: No shake interrupt pending, 1: Shake interrupt pending
        bool tilt_35_int : 1; // 0: No tilt > 35° interrupt pending, 1: Tilt > 35° interrupt pending
        bool fifo_int : 1;    // OR of the three FIFO flags from register 0x2F (FIFO_FULL_INTR, FIFO_THRESH_INTR, FIFO_EMPTY_INTR)
        uint8_t _reserved0 : 1;
        bool acq_int : 1; // 0: Sample interrupt is not pending, 1: Sample interrupt is pending
    };
} mc3419_intr_stat_t;

/**
 * CHIP_ID (R) Register definition
 */

typedef union {
    uint8_t raw;
    struct {
        uint8_t chip_id; // Chip ID - Fixed value for the MC3419: 0b10100100 (0xA4)
    };
} mc3419_chip_id_t;

/**
 * RANGE (W) Register definition
 */

typedef enum {
    RANGE_2G  = 0b000, // ±2g
    RANGE_4G  = 0b001, // ±4g
    RANGE_8G  = 0b010, // ±8g
    RANGE_16G = 0b011, // ±16g
    RANGE_12G = 0b100, // ±12g
    // 0b101-0b111: Reserved
} mc3419_range_res_t;

typedef enum {
    // 000: Reserved
    LPF_BW_1 = 0b001, // Bandwidth setting 1, Fc = IDR / 4.255
    LPF_BW_2 = 0b010, // Bandwidth setting 2, Fc = IDR / 6
    LPF_BW_3 = 0b011, // Bandwidth setting 3, Fc = IDR / 12
    // 100: Reserved
    LPF_BW_4 = 0b101, // Bandwidth setting 4, Fc = IDR / 16
    // 110: Reserved
    // 111: Reserved
} mc3419_range_lpf_t;

typedef union {
    uint8_t raw;
    struct {
        mc3419_range_res_t range : 3;  // Resolution range of the accelerometer, based on the current resolution
        bool lpf_en : 1;               // 0: Low-pass filter disabled, 1: Low-pass filter enabled
        mc3419_range_lpf_t lpf_bw : 3; // Low-pass filter bandwidth selection
        uint8_t _reserved0 : 1;
    };
} mc3419_range_t;

/**
 * XOFFL and XOFFH (W) Register definition
 *     INFO: The X-axis digital offset registers contain a signed 2's complement 15-bit value used to offset the output of the
 *           X-axis filter. Also, register 0x22 bit 7 is actually the ninth bit of X-axis gain (XGAIN)
 */

typedef union {
    uint16_t raw;
    struct {
        uint8_t xoffl : 8; // X-axis offset LSB
        uint8_t xoffh : 7; // X-axis offset MSB
        bool xgain_8 : 1;  // X-axis gain bit 8
    } bytes;
    struct {
        uint16_t xoff : 15; // X-axis offset
        bool xgain_8 : 1;   // X-axis gain bit 8
    };
} mc3419_xoff_t;

/**
 * YOFFL and YOFFH (W) Register definition
 *     INFO: The Y-axis digital offset registers contain a signed 2's complement 15-bit value used to offset the output of the
 *           Y-axis filter. Also, register 0x24 bit 7 is actually the ninth bit of Y-axis gain (YGAIN)
 */

typedef union {
    uint16_t raw;
    struct {
        uint8_t yoffl : 8; // Y-axis offset LSB
        uint8_t yoffh : 7; // Y-axis offset MSB
        bool ygain_8 : 1;  // Y-axis gain bit 8
    } bytes;
    struct {
        uint16_t yoff : 15; // Y-axis offset
        bool ygain_8 : 1;   // Y-axis gain bit 8
    };
} mc3419_yoff_t;

/**
 * ZOFFL and ZOFFH (W) Register definition
 *     INFO: The Z-axis digital offset registers contain a signed 2's complement 15-bit value used to offset the output of the
 *           Z-axis filter. Also, register 0x26 bit 7 is actually the ninth bit of Z-axis gain (ZGAIN)
 */

typedef union {
    uint16_t raw;
    struct {
        uint8_t zoffl : 8; // Z-axis offset LSB
        uint8_t zoffh : 7; // Z-axis offset MSB
        bool zgain_8 : 1;  // Z-axis gain bit 8
    } bytes;
    struct {
        uint16_t zoff : 15; // Z-axis offset
        bool zgain_8 : 1;   // Z-axis gain bit 8
    };

} mc3419_zoff_t;

/**
 * XGAIN (W) Register definition
 *     INFO: The X-axis digital gain registers contain an unsigned 9-bit value. 0x22 bit 7 is the ninth bit of the X-axis gain
 */

typedef union {
    uint8_t raw;
    struct {
        uint8_t xgain : 7; // X-axis gain
    };
} mc3419_xgain_t;

/**
 * YGAIN (W) Register definition
 *     INFO: The Y-axis digital gain registers contain an unsigned 9-bit value. 0x24 bit 7 is the ninth bit of the Y-axis gain
 */

typedef union {
    uint8_t raw;
    struct {
        uint8_t ygain : 7; // Y-axis gain
    };
} mc3419_ygain_t;

/**
 * ZGAIN (W) Register definition
 *     INFO: The Z-axis digital gain registers contain an unsigned 9-bit value. 0x26 bit 7 is the ninth bit of the Z-axis gain
 */

typedef union {
    uint8_t raw;
    struct {
        uint8_t zgain : 7; // Z-axis gain
    };
} mc3419_zgain_t;

/**
 * FIFO_CTRL (RW) Register definition
 */

typedef union {
    uint8_t raw;
    struct {
        bool fifo_empty_int_en : 1; // 0: FIFO empty interrupt disabled (default), 1: FIFO empty interrupt enabled
        bool fifo_full_int_en : 1;  // 0: FIFO full interrupt disabled (default), 1: FIFO full interrupt enabled
        bool fifo_th_int_en : 1;    // 0: FIFO threshold interrupt disabled (default), 1: FIFO threshold interrupt enabled
        bool comb_int_en : 1; // 0: Motion/interrupt on sample interrupts are routed to INTN1, and FIFO interrupts are routed to
                              // INTN2 (default) 1: All interrupts are routed to INTN1
        bool fifo_reset : 1;  // 0: FIFO is not reset (default), 1: FIFO is reset, read and write pointers are cleared
        bool fifo_en : 1;     // 0: FIFO disabled (default), 1: FIFO enabled
        bool fifo_mode : 1;   // 0: FIFO in normal mode (default)
                              // 1: FIFO in watermark (threshold) mode - once threshold is reached, additional sample data is
                              //    dropped
        uint8_t _reserved0 : 1;
    };
} mc3419_fifo_ctrl_t;

/**
 * FIFO_TH (RW) Register definition
 */

typedef union {
    uint8_t raw;
    struct {
        uint8_t fifo_th : 5; // FIFO threshold level (0-31)
        uint8_t _reserved0 : 3;
    };
} mc3419_fifo_th_t;

/**
 * FIFO_INTR (R) Register definition
 */

typedef union {
    uint8_t raw;
    struct {
        bool fifo_empty_int : 1; // 0: FIFO empty interrupt not pending, 1: FIFO empty interrupt pending
        bool fifo_full_int : 1;  // 0: FIFO full interrupt not pending, 1: FIFO full interrupt pending
        bool fifo_th_intr : 1;   // 0: FIFO threshold interrupt not pending, 1: FIFO threshold interrupt pending
        uint8_t _reserved0 : 5;
    };
} mc3419_fifo_intr_t;

/**
 * FIFO_CTRL2_SR2 (RW) Register definition
 */

typedef enum {
    DEC_MODE_RATE_0 = 0b0000, // Decimation mode disabled
    DEC_MODE_RATE_1 = 0b0001, // Divide sample rate by 2
    DEC_MODE_RATE_2 = 0b0010, // Divide sample rate by 4
    DEC_MODE_RATE_3 = 0b0011, // Divide sample rate by 5
    DEC_MODE_RATE_4 = 0b0100, // Divide sample rate by 8
    DEC_MODE_RATE_5 = 0b0101, // Divide sample rate by 10
    DEC_MODE_RATE_6 = 0b0110, // Divide sample rate by 16
    DEC_MODE_RATE_7 = 0b0111, // Divide sample rate by 20
    DEC_MODE_RATE_8 = 0b1000, // Divide sample rate by 40
    DEC_MODE_RATE_9 = 0b1001, // Divide sample rate by 67
    DEC_MODE_RATE_A = 0b1010, // Divide sample rate by 80
    DEC_MODE_RATE_B = 0b1011, // Divide sample rate by 100
    DEC_MODE_RATE_C = 0b1100, // Divide sample rate by 200
    DEC_MODE_RATE_D = 0b1101, // Divide sample rate by 250
    DEC_MODE_RATE_E = 0b1110, // Divide sample rate by 500
    DEC_MODE_RATE_F = 0b1111, // Divide sample rate by 1000
} mc3419_fifo_dec_mode_rate_t;

typedef union {
    uint8_t raw;
    struct {
        mc3419_fifo_dec_mode_rate_t dec_mode : 4; // FIFO decimation mode
        bool enable_wrap : 1;      // 0: Internal register address pointer will "wrap" at address selected by bit 5 (default)
                                   // 1: Internal register address pointer will increment to the next consecutive value
        bool select_wrap_addr : 1; // 0: Wrap from address 0x12 to 0x0D on read cycles (default)
                                   // 1: Wrap from address 0x14 to 0x0D on read cycles
        uint8_t _reserved0 : 1;
        bool fifo_burst : 1; // 0: FIFO read cycle reads a single 6 byte XYZ sample (default)
                             // 1: FIFO read cycle reads 2 or more (up to 32) 6 byte XYZ samples - must be set in register 0x4B
    };
} mc3419_fifo_ctrl2_sr2_t;

/**
 * COMM_CTRL (W) Register definition
 */

typedef union {
    uint8_t raw;
    struct {
        uint8_t _reserved0 : 4;
        bool int1_int2_req_swap : 1; // 0: INT1 requests are routed to INTN1, INT2 requests are routed to INTN2 (default)
                                     // 1: INT1 requests are routed to INTN2, INT2 requests are routed to INTN1
        bool spi_3wire_en : 1;       // 0: SPI 3-wire mode disabled (default), 1: SPI 3-wire mode enabled
        bool indiv_intr_clr : 1; // 0: Individual interrupt clear disabled (default) - all interrupts cleared by writing to 0x14
                                 // 1: Individual interrupt clear enabled - each interrupt must be cleared by writing to 0x14 as a
                                 //    bitmask
        uint8_t _reserved1 : 1;
    };
} mc3419_comm_ctrl_t;

/**
 * GPIO_CTRL (W) Register definition
 */

typedef union {
    uint8_t raw;
    struct {
        uint8_t _reserved0 : 2;
        bool gpio1_intn1_iah : 1; // 0: The INTN1 pin is active low (default), 1: The INTN1 pin is active high
        bool gpio1_intn1_ipp : 1; // 0: The INTN1 pin is open-drain (default), 1: The INTN1 pin is push-pull
        uint8_t _reserved1 : 2;
        bool gpio2_intn2_iah : 1; // 0: The INTN2 pin is active low (default), 1: The INTN2 pin is active high
        bool gpio2_intn2_ipp : 1; // 0: The INTN2 pin is open-drain (default), 1: The INTN2 pin is push-pull
    };
} mc3419_gpio_ctrl_t;

/**
 * TF_THRESH_LSB and TF_THRESH_MSB (W) Register definition
 */

typedef union {
    uint16_t raw;
    struct {
        uint8_t tf_thresh_lsb : 8; // Tilt/Flip threshold LSB (0x40)
        uint8_t tf_thresh_msb : 7; // Tilt/Flip threshold MSB (0x41)
        uint8_t _reserved0 : 1;
    } bytes;
    struct {
        uint16_t tf_thresh : 15; // Tilt/Flip threshold
        uint8_t _reserved0 : 1;
    };
} mc3419_tf_thresh_t;

/**
 * TF_DB (W) Register definition
 */

typedef union {
    uint8_t raw;
    struct {
        uint8_t tf_db : 8; // Tilt/Flip debounce duration
    };
} mc3419_tf_db_t;

/**
 * AM_THRESH_LSB and AM_THRESH_MSB (W) Register definition
 */

typedef union {
    uint16_t raw;
    struct {
        uint8_t am_thresh_lsb : 8; // AnyMotion threshold LSB (0x43)
        uint8_t am_thresh_msb : 7; // AnyMotion threshold MSB (0x44)
        uint8_t _reserved0 : 1;
    } bytes;
    struct {
        uint16_t am_thresh : 15; // AnyMotion threshold
        uint8_t _reserved0 : 1;
    };
} mc3419_am_thresh_t;

/**
 * AM_DB (W) Register definition
 */

typedef union {
    uint8_t raw;
    struct {
        uint8_t am_db : 8; // AnyMotion debounce duration
    };
} mc3419_am_db_t;

/**
 * SHK_THRESH_LSB and SHK_THRESH_MSB (W) Register definition
 */

typedef union {
    uint16_t raw;
    struct {
        uint8_t shk_thresh_lsb : 8; // Shake threshold LSB (0x46)
        uint8_t shk_thresh_msb : 7; // Shake threshold MSB (0x47)
        uint8_t _reserved0 : 1;
    } bytes;
    struct {
        uint16_t shk_thresh : 15; // Shake threshold
        uint8_t _reserved0 : 1;
    };
} mc3419_shk_thresh_t;

/**
 * PK_P2P_DUR_THRESH_LSB and PK_P2P_DUR_THRESH_MSB (W) Register definition
 *     INFO: The shake duration and peak-to-peak registers hold the programmed 12-bit threshold value of a peak and the
 *           peak-to-peak width of a shake and the programmed 3-bit threshold value of the shake counter.
 */

typedef union {
    uint16_t raw;
    struct {
        uint8_t pk_p2p_dur_thresh_lsb : 8; // Peak-to-Peak Duration threshold LSB (0x48)
        uint8_t pk_p2p_dur_thresh_msb : 4; // Peak-to-Peak Duration threshold MSB (0x49)
        uint8_t shk_thresh : 3;            // Shake threshold (0x49)
    } bytes;
    struct {
        uint16_t pk_p2p_dur_thresh : 12; // Peak-to-Peak Duration threshold
        uint8_t shk_thresh : 3;          // Shake threshold
    };
} mc3419_pk_p2p_dur_thresh_t;

/**
 * TIMER_CTRL (W) Register definition
 */

typedef enum {
    TILT35_1_6 = 0b000, // 1.6s (default)
    TILT35_1_8 = 0b001, // 1.8s
    TILT35_2_0 = 0b010, // 2.0s
    TILT35_2_2 = 0b011, // 2.2s
    TILT35_2_4 = 0b100, // 2.4s
    TILT35_2_6 = 0b101, // 2.6s
    TILT35_2_8 = 0b110, // 2.8s
    TILT35_3_0 = 0b111, // 3.0s
} mc3419_tilt35_dur_t;

typedef enum {
    TEMP_PERIOD_200MS  = 0b000, // 200ms (default)
    TEMP_PERIOD_400MS  = 0b001, // 400ms
    TEMP_PERIOD_800MS  = 0b010, // 800ms
    TEMP_PERIOD_1600MS = 0b011, // 1600ms
    TEMP_PERIOD_3200MS = 0b100, // 3200ms
    TEMP_PERIOD_6400MS = 0b101, // 6400ms
    // 0b110-0b111: Reserved
} mc3419_temp_period_t;

typedef union {
    uint8_t raw;
    struct {
        mc3419_tilt35_dur_t tilt_35 : 3; // Tilt > 35° duration
        uint8_t _reserved0 : 1;
        mc3419_temp_period_t temp_period : 3; // Temporary latch period on the TEST_INT pin
        bool temp_per_int_en : 1;             // 0: The temporary latch feature is disabled (default)
                                              // 1: The temporary latch feature is enabled
    };
} mc3419_timer_ctrl_t;

/**
 * RD_CNT (R) Register definition
 *     INFO: 0x06: POR value (default).
 *           If register 0x30 bit 7 (FIFO_BURST) is enabled, this register is the number of samples to be read in single burst
 *           read transaction. A sample is one 6-byte sample from the FIFO and optionally one or two status bytes from registers
 *           0x13 and 0x14 (a sample can be 6, 7, or 8-bytes long). Note this parameter is a sample count, not a byte count. If
 *           FIFO burst mode is disabled, this parameter is not used.
 */

typedef union {
    uint8_t raw;
    struct {
        uint8_t rd_cnt : 8; // Read count - sets the length of FIFO burst read transactions
    };
} mc3419_rd_cnt_t;

#ifdef __cplusplus
}
#endif
