/**********************************************************************
  R4A_ESP32_LEDC.h

  Robots-For-All (R4A)
  Declare the LED controller constants and data structures
**********************************************************************/

#ifndef __R4A_ESP32_LEDC_H__
#define __R4A_ESP32_LEDC_H__

#define LEDC_TIMER_COUNT        4
#define LEDC_CHANNEL_COUNT      8

typedef struct _R4A_LEDC_CHANNEL_t
{
    uint32_t conf0;     //    0 Configuration 0
    uint32_t hPoint;    //    4 High-level comparitor value
    uint32_t duty;      //    8 Initial duty cycle
    uint32_t conf1;     // 0x0c Configuration 1
    uint32_t lpoint;    // 0x10 Low-level comparitor value
} R4A_LEDC_CHANNEL_t;

// CONF0 register
#define LEDC_CH_CONF0_PARA_UP_LSCH  0x10    // Update low speed HPOINT and DUTY registers
#define LEDC_CH_CONF0_IDLE_LV          8    // Output value when channel inactive
#define LEDC_CH_CONF0_SIG_OUT_EN       4    // Enable output
#define LEDC_CH_CONF0_TIMER_SEL        3    // Select timer 0 - 3

// HPOINT register
#define LEDC_CH_HPOINT_MASK         0xfffff // Output goes high when 20-bit timer
                                            // reaches this value

// DUTY register
#define LEDC_CH_DUTY_MASK           0xffffff    // Output goes low when 20-bit timer
                                                // reaches this value or value+1

// CONF1 register, used to gradually increase/decrease the duty cycle for fades
#define LEDC_CH_CONF1_DUTY_START    0x80000000  // Start the channel
#define LEDC_CH_CONF1_DUTY_INC      0x40000000  // Increase (1) or decrease (0) the duty cycle
#define LEDC_CH_CONF1_DUTY_NUM      0x3ff00000  // Number of times to adjust the duty cycle
#define LEDC_CH_CONF1_DUTY_CYCLE    0x000ffc00  // Duty cycle adjustment value
#define LEDC_CH_CONF1_DUTY_SCALE    0x000003ff  // Scale for duty cycle adjustment

typedef struct _R4A_LEDC_TIMER_t
{
    uint32_t conf;      //  0
    uint32_t value;     //  4
} R4A_LEDC_TIMER_t;

// CONF register
#define LEDC_TIM_CONF_TICK_SEL      0x02000000  // Select APB_CLK (1) or REF_TICK (0)
#define LEDC_TIM_CONF_RST           0x01000000  // Reset the timer to zero
#define LEDC_TIM_CONF_PAUSE         0x00800000  // Suspend the timer
#define LEDC_TIM_CONF_DIV_NUM       0x007fffe0  // Most significant 10-bits divider
                                                // Least significant 8-bit fractional part
#define LEDC_TIM_CONF_DUTY_RES      0x0000001f  // Number of bits in duty register (1 - 20)

// VALUE register
#define LEDC_TIM_VALUE              0x000fffff  // Timer value

typedef struct _R4A_LEDC
{
    R4A_LEDC_CHANNEL_t hsCh[LEDC_CHANNEL_COUNT]; //     0 -  0x9f
    R4A_LEDC_CHANNEL_t lsCh[LEDC_CHANNEL_COUNT]; //  0xa0 - 0x13f
    R4A_LEDC_TIMER_t hsTimer[LEDC_TIMER_COUNT];  // 0x140 - 0x15f
    R4A_LEDC_TIMER_t lsTimer[LEDC_TIMER_COUNT];  // 0x160 - 0x17f
    uint32_t intRaw;                // 0x180
    uint32_t intSt;                 // 0x184
    uint32_t intEna;                // 0x188
    uint32_t intClr;                // 0x18c
    uint32_t conf;                  // 0x190
} R4A_LEDC;

// CONF register
#define LEDC_CONF_APB_CLK_SEL   1   // RTC_SLOW_CLK (0, 8 MHz) or APB_CLK (1, 80 MHz)

//                                 +--------+
//         80 MHz APB_CLK -------->| LEDC   |
//                                 | Clock  |
//          8 MHz RTC_SLOW_CLK --->| Select |
//                                 +--------+
//                                      |
//                                      | LEDC CLK
//                                      V
//                           +--------------------------------+
//                           |          |          LEDC Timer |
//                           |          V                     |
//                           |     +--------+                 |
//       1 MHz REF_TICK ---->|---->| LEDC   |                 |
//                           |     | Timer  |                 |
//                           |     | Clock  |                 |
//                           |     | Select |                 |
//                           |     +--------+                 |
//                           |          |                     |
//                           |          | Clock to Divider    |
//                           |          V                     |
//                           |     +---------+                |
//                           |     | LEDC    |                |
//          Divider -------->|---->| Timer   |                |
//                           |     | Divider |                |
//                           |     +---------+                |
//                           |          |                     |
//                           |          | Counter Clock       |
//                           |          V                     |
//                           |     +---------+                |
//                           |     | LEDC    |                |
//       Duty Resolution --->|---->| Timer   |                |
//                           |     | Counter |                |
//                           |     +---------+                |
//                           |          |                     |
//                           |          V                     |
//                           +--------------------------------+
//                                      |
//                                      | Timer Count Value
//                                      V
//                           +--------------------------------+
//                           |          |        LEDC Channel |
//                           |          V                     |
//                           |   +-------------+              |
//          High Point ----->|-->| LEDC        |              |
//                           |   | Channel     |              |
//     High Point + Duty --->|-->| Comparitors |              |
//                           |   |             |              |
//          CONF1 ---------->|-->|             |              |
//                           |   +-------------+              |
//                           |          |                     |
//                           |          V                     |
//                           +--------------------------------+
//                                      |
//                                      | Output
//                                      V

R4A_LEDC * const r4aEsp32LedC = (R4A_LEDC *)0x3ff59000;

#endif  // __R4A_ESP32_LEDC_H__
