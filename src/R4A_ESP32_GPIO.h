/**********************************************************************
  R4A_ESP32_GPIO.h

  Robots-For-All (R4A)
  Declare the GPIO constants and data structures
**********************************************************************/

#ifndef __R4A_ESP32_GPIO_H__
#define __R4A_ESP32_GPIO_H__

#define R4A_GPIO_MAX_PORTS              40

typedef volatile struct _R4A_GPIO_REGS
{
    uint32_t reserved_0;                //     0
    uint32_t R4A_GPIO_OUT_REG;          //     4
    uint32_t R4A_GPIO_OUT_W1TS_REG;     //     8
    uint32_t R4A_GPIO_OUT_W1TC_REG;     // 0x00c
    uint32_t R4A_GPIO_OUT1_REG;         // 0x010
    uint32_t R4A_GPIO_OUT1_W1TS_REG;    // 0x014
    uint32_t R4A_GPIO_OUT1_W1TC_REG;    // 0x018
    uint32_t reserved_01c;              // 0x01c
    uint32_t R4A_GPIO_ENABLE_REG;       // 0x020
    uint32_t R4A_GPIO_ENABLE_W1TS_REG;  // 0x024
    uint32_t R4A_GPIO_ENABLE_W1TC_REG;  // 0x028
    uint32_t R4A_GPIO_ENABLE1_REG;      // 0x02c
    uint32_t R4A_GPIO_ENABLE1_W1TS_REG; // 0x030
    uint32_t R4A_GPIO_ENABLE1_W1TC_REG; // 0x034
    uint32_t R4A_GPIO_STRAP_REG;        // 0x038
    uint32_t R4A_GPIO_IN_REG;           // 0x03c
    uint32_t R4A_GPIO_IN1_REG;          // 0x040
    uint32_t R4A_GPIO_STATUS_REG;       // 0x044
    uint32_t R4A_GPIO_STATUS_W1TS_REG;  // 0x048
    uint32_t R4A_GPIO_STATUS_W1TC_REG;  // 0x04c
    uint32_t R4A_GPIO_STATUS1_REG;      // 0x050
    uint32_t R4A_GPIO_STATUS1_W1TS_REG; // 0x054
    uint32_t R4A_GPIO_STATUS1_W1TC_REG; // 0x058
    uint32_t reserved_05c;              // 0x05c
    uint32_t R4A_GPIO_ACPU_INT_REG;     // 0x060
    uint32_t R4A_GPIO_ACPUNMI_INT_REG;  // 0x064
    uint32_t R4A_GPIO_PCPU_INT_REG;     // 0x068
    uint32_t R4A_GPIO_PCPU_NMI_INT_REG; // 0x06c
    uint32_t reserved_070;              // 0x070
    uint32_t R4A_GPIO_ACPU_INT1_REG;    // 0x074
    uint32_t R4A_GPIO_ACPU_NMI_INT1_REG;// 0x078
    uint32_t R4A_GPIO_PCPU_INT1_REG;    // 0x07c
    uint32_t R4A_GPIO_PCPU_NMI_INT1_REG;// 0x080
    uint32_t reserved_84;               // 0x084
    uint32_t R4A_GPIO_PIN_REG[R4A_GPIO_MAX_PORTS]; // 0x088 - 0x12f
    uint32_t reserved_128;              // 0x128
    uint32_t reserved_12c;              // 0x12c
    uint32_t R4A_GPIO_FUNC_IN_SEL_CFG_REG[256]; //0x130 - 0x52f
    uint32_t R4A_GPIO_FUNC_OUT_SEL_CFG_REG[R4A_GPIO_MAX_PORTS]; //0x530 - 0x5cf
} R4A_GPIO_REGS;

#define GPIO_FUNC_SIG_IN_SEL        0x00000080  // Force output to port
#define GPIO_FUNC_IN_INV_SEL        0x00000040  // Invert the signal
#define GPIO_FUNC_IN_SEL            0x0000002f  // Port number or 0x30 low or 0x38 high

#define GPIO_FUNC_OEN_INV_SEL       0x00000800  // Invert the output
#define GPIO_FUNC_OEN_SEL           0x00000400  // 1: Use GPIO_ENABLE_REG to enable output, 0: Use peripheral direction
#define GPIO_FUNC_OUT_INV_SEL       0x00000200  // Invert the output
#define GPIO_FUNC_OUT_SEL           0x000001ff  // Index into the GPIO matrix

// GPIO output
//
//               GPIO_FUNC_OUT_SEL          MCU SEL
//                       |                     |
//                       v                     v
//      Signals --> GPIO_MATRIX -----------> IO_MUX ------> I/O pad
//    0x100 = GPIO out           GPIOx_out
//

typedef volatile struct _R4A_IO_MUX_REGS
{
    uint32_t R4A_IO_MUX_PIN_CTRL;       //    0,  0
    uint32_t R4A_IO_MUX_GPIO36_REG;     //    4,  1
    uint32_t R4A_IO_MUX_GPIO37_REG;     //    8,  2
    uint32_t R4A_IO_MUX_GPIO38_REG;     // 0x0c,  3
    uint32_t R4A_IO_MUX_GPIO39_REG;     // 0x10,  4
    uint32_t R4A_IO_MUX_GPIO34_REG;     // 0x14,  5
    uint32_t R4A_IO_MUX_GPIO35_REG;     // 0x18,  6
    uint32_t R4A_IO_MUX_GPIO32_REG;     // 0x1c,  7
    uint32_t R4A_IO_MUX_GPIO33_REG;     // 0x20,  8
    uint32_t R4A_IO_MUX_GPIO25_REG;     // 0x24,  9
    uint32_t R4A_IO_MUX_GPIO26_REG;     // 0x28, 10
    uint32_t R4A_IO_MUX_GPIO27_REG;     // 0x2c, 11
    uint32_t R4A_IO_MUX_MTMS_REG;       // 0x30, 12
    uint32_t R4A_IO_MUX_MTDI_REG;       // 0x34, 13
    uint32_t R4A_IO_MUX_MTCK_REG;       // 0x38, 14
    uint32_t R4A_IO_MUX_MTDO_REG;       // 0x3c, 15
    uint32_t R4A_IO_MUX_GPIO2_REG;      // 0x40, 16
    uint32_t R4A_IO_MUX_GPIO0_REG;      // 0x44, 17
    uint32_t R4A_IO_MUX_GPIO4_REG;      // 0x48, 18
    uint32_t R4A_IO_MUX_GPIO16_REG;     // 0x4c, 19
    uint32_t R4A_IO_MUX_GPIO17_REG;     // 0x50, 20
    uint32_t R4A_IO_MUX_SD_DATA2_REG;   // 0x54, 21
    uint32_t R4A_IO_MUX_SD_DATA3_REG;   // 0x58, 22
    uint32_t R4A_IO_MUX_SD_CMD_REG;     // 0x5c, 23
    uint32_t R4A_IO_MUX_SD_CLK_REG;     // 0x60, 24
    uint32_t R4A_IO_MUX_SD_DATA0_REG;   // 0x64, 25
    uint32_t R4A_IO_MUX_SD_DATA1_REG;   // 0x68, 26
    uint32_t R4A_IO_MUX_GPIO5_REG;      // 0x6c, 27
    uint32_t R4A_IO_MUX_GPIO18_REG;     // 0x70, 28
    uint32_t R4A_IO_MUX_GPIO19_REG;     // 0x74, 29
    uint32_t R4A_IO_MUX_GPIO20_REG;     // 0x78, 30
    uint32_t R4A_IO_MUX_GPIO21_REG;     // 0x7c, 31
    uint32_t R4A_IO_MUX_GPIO22_REG;     // 0x80, 32
    uint32_t R4A_IO_MUX_U0RXD_REG;      // 0x84, 33
    uint32_t R4A_IO_MUX_U0TXD_REG;      // 0x88, 34
    uint32_t R4A_IO_MUX_GPIO23_REG;     // 0x8c, 35
    uint32_t R4A_IO_MUX_GPIO24_REG;     // 0x90, 36
} R4A_IO_MUX_REGS;

#define IO_MUX_MCU_SEL              0x00007000  // IO mux for pin
#define IO_MUX_FUN_DRV              0x00000c00  // Drive strength
#define IO_MUX_IN_IE                0x00000200  // Enable input
#define IO_MUX_FUN_WPU              0x00000100  // Enble pull up
#define IO_MUX_FUN_WPD              0x00000080  // Enable pull down
#define IO_MUX_MCU_DRV              0x00000060  // Drive strength during sleep mode
#define IO_MUX_MCU_IE               0x00000010  // Input enable during sleep mode
#define IO_MUX_MCU_WPU              0x00000008  // Enable pull up during sleep mode
#define IO_MUX_MCU_WPD              0x00000004  // Enable pull down during sleep mode
#define IO_MUX_SLP_SEL              0x00000002  // Place pad in sleep mode
#define IO_MUX_MCU_OE               0x00000001  // Output enable during sleep mode

typedef struct _R4A_GPIO_MATRIX
{
    const char * _input;
    const char * _output;
} R4A_GPIO_MATRIX;

typedef volatile struct _R4A_RTCIO_REGS
{
    uint32_t R4A_RTCIO_RTC_GPIO_OUT_REG;        //    0
    uint32_t R4A_RTCIO_RTC_GPIO_OUT_W1TS_REG;   //    4
    uint32_t R4A_RTCIO_RTC_GPIO_OUT_W1TC_REG;   //    8
    uint32_t R4A_RTCIO_RTC_GPIO_ENABLE_REG;     // 0x0c
    uint32_t R4A_RTCIO_RTC_GPIO_ENABLE_W1TS_REG;// 0x10
    uint32_t R4A_RTCIO_RTC_GPIO_ENABLE_W1TC_REG;// 0x14
    uint32_t R4A_RTCIO_RTC_GPIO_STATUS_REG;     // 0x18
    uint32_t R4A_RTCIO_RTC_GPIO_STATUS_W1TS_REG;// 0x1c
    uint32_t R4A_RTCIO_RTC_GPIO_STATUS_W1TC_REG;// 0x20
    uint32_t R4A_RTCIO_RTC_GPIO_IN_REG;         // 0x24
    uint32_t R4A_RTCIO_RTC_GPIO_PIN_REG[18];    // 0x28 - 0x6f
    uint32_t reserved_70;                       // 0x70
    uint32_t R4A_RTCIO_DIG_PAD_HOLD_REG;        // 0x74
    uint32_t R4A_RTCIO_HALL_SENS_REG;           // 0x78
    uint32_t R4A_RTCIO_SENSOR_PADS_REG;         // 0x7c
    uint32_t R4A_RTCIO_ADC_PAD_REG;             // 0x80
    uint32_t R4A_RTCIO_PAD_DAC1_REG;            // 0x84
    uint32_t R4A_RTCIO_PAD_DAC2_REG;            // 0x88
    uint32_t R4A_RTCIO_XTAL_32K_PAD_REG;        // 0x8c
    uint32_t R4A_RTCIO_TOUCH_CFG_REG;           // 0x90
    uint32_t R4A_RTCIO_TOUCH_PAD_REG[10];       // 0x94 - 0xbb
    uint32_t R4A_RTCIO_EXT_WAKEUP0_REG;         // 0xbc
    uint32_t R4A_RTCIO_XTL_EXT_CTR_REG;         // 0xc0
    uint32_t R4A_RTCIO_SAR_I2C_IO_REG;          // 0xc4
} R4A_RTCIO_REGS;

#endif  // __R4A_ESP32_GPIO_H__
