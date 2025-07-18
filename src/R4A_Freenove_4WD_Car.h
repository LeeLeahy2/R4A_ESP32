/**********************************************************************
  R4A_Freenove_4WD_Car.h

  Robots-For-All (R4A)
  Freenove 4WD Car robot declarations
  https://store.freenove.com/products/fnk0053
  https://www.amazon.com/gp/product/B08X6PTQFM/ref=ox_sc_act_title_1
**********************************************************************/

#ifndef __FREENOVE_4WD_CAR_H__
#define __FREENOVE_4WD_CAR_H__

#include "R4A_ESP32.h"          // Robots-For-All ESP32 support

//****************************************
// Battery
//****************************************

#define BATTERY_VOLTAGE_MULTIPLIER  4

#define DISPLAY_BATTERY_VOLTAGE(display)  \
    r4aEsp32VoltageDisplay(BATTERY_WS2812_PIN, 0, BATTERY_VOLTAGE_MULTIPLIER, display)

#define READ_BATTERY_VOLTAGE(adcValue)  \
    r4aEsp32VoltageGet(BATTERY_WS2812_PIN, 0, BATTERY_VOLTAGE_MULTIPLIER, adcValue)

//****************************************
// Buzzer
//****************************************

#define BUZZER_PWM_CHANNEL      0
#define BUZZER_FREQUENCY        2000

//****************************************
// Car class
//****************************************

// The Freenove 4WD Car has 12 multicolor LEDs (WS2812) arranged as:
//
//                      Front
//          L1  L2  L3         R3  R2  R1
//          10  11  12         13  14  15
//
//          21  20  19         18  17  16
//                       Back

#define FRONT_L1        0
#define FRONT_L2        1
#define FRONT_L3        2
#define FRONT_R3        3
#define FRONT_R2        4
#define FRONT_R1        5
#define BACK_R1         6
#define BACK_R2         7
#define BACK_R3         8
#define BACK_L3         9
#define BACK_L2         10
#define BACK_L1         11

class R4A_Freenove_4WD_Car
{
  private:

    uint32_t _backupLightColor; // Color for LEDS 17 - 20
    const uint32_t _blinkMsec;  // Milliseconds between LED state changes
    uint32_t _brakeLightColor;  // Color for LEDS 17 - 20
    uint32_t _headlightColor;   // Color for LEDS 11 - 14
    uint8_t  _ledState;         // State of the LEDs
    volatile uint32_t _timer;   // Turn signal timer in milliseconds
    volatile bool _updateLEDs;  // Set true when changing LED color

    enum
    {
        STATE_OFF = 0,
        STATE_BLINK_L3,
        STATE_BLINK_L2,
        STATE_BLINK_L1,
        STATE_BLINK_L_OFF,
        STATE_BLINK_R3,
        STATE_BLINK_R2,
        STATE_BLINK_R1,
        STATE_BLINK_R_OFF,
    };

  public:

    const int numberOfLEDs = 12;

    // Constructor
    R4A_Freenove_4WD_Car(uint32_t blinkMsec = 150)
        : _backupLightColor{R4A_LED_OFF}, _blinkMsec{blinkMsec},
          _brakeLightColor{R4A_LED_OFF}, _headlightColor{R4A_LED_OFF},
          _ledState{0}, _timer{0}, _updateLEDs{false}
    {
    }

    // Get the backup light color
    // Outputs:
    //   Returns the color of the backup lights
    uint32_t backupLightColorGet();

    // Set the backup light color
    // Inputs:
    //   color: New color to use for the backup lights
    void backupLightColorSet(uint32_t color);

    // Turn off the backup lights
    void backupLightsOff();

    // Turn on the backup lights
    void backupLightsOn();

    // Toggle the backup lights
    void backupLightsToggle();

    // Get the brake light color
    // Outputs:
    //   Returns the color of the brake lights
    uint32_t brakeLightColorGet();

    // Set the brake light color
    // Inputs:
    //   color: New color to use for the brake lights
    void brakeLightColorSet(uint32_t color);

    // Turn off the brake lights
    void brakeLightsOff();

    // Turn on the brake lights
    void brakeLightsOn();

    // Toggle the brake lights
    void brakeLightsToggle();

    // Get the headlight color
    // Outputs:
    //   Returns the color of the headlights
    uint32_t headlightColorGet();

    // Set the headlight color
    // Inputs:
    //   color: New color to use for the headlights
    void headlightColorSet(uint32_t color);

    // Turn off the headlights
    void headlightsOff();

    // Turn on the headlights
    void headlightsOn();

    // Toggle the headlights
    void headlightsToggle();

    // Turn off all the LEDs
    void ledsOff();

    // Use LEDs to indicate left turn
    void ledsTurnLeft();

    // Stop using the LEDs to indicate a turn
    void ledsTurnOff();

    // Use LEDs to indicate right turn
    void ledsTurnRight();

    // Write the new color values to the WS2812 LEDs
    // Inputs:
    //   currentMsec: Number of milliseconds since boot
    void ledsUpdate(uint32_t currentMsec);
};

//****************************************
// ESP32 WRover Module Pins
//****************************************

#define BLUE_LED_BUZZER_PIN     2   // Blue LED and buzzer output
#define BATTERY_WS2812_PIN      32  // WS2812 output, Battery voltage input
#define LIGHT_SENSOR_PIN        33

#define ESP32_WROVER_BLUE_LED_OFF       0
#define ESP32_WROVER_BLUE_LED_ON        1

//****************************************
// LED Matrix
//****************************************

extern const uint8_t r4aLedMatrixColumnMap[R4A_VK16K33_MAX_COLUMNS];
extern const uint8_t r4aLedMatrixRowPixelMap[R4A_VK16K33_MAX_ROWS];

//****************************************
// I2C bus configuration
//****************************************

// I2C controller pins
#define I2C_SDA                 13  // I2C data line
#define I2C_SCL                 14  // I2C clock line

// I2C addresses
#define OV2640_I2C_ADDRESS      0x70
#define PCA9685_I2C_ADDRESS     0x5f
#define PCF8574_I2C_ADDRESS     0x20
#define VK16K33_I2C_ADDRESS     0x71

// Description table
#define USE_I2C_DEVICE_TABLE   \
const R4A_I2C_DEVICE_DESCRIPTION i2cBusDeviceTable[] =  \
{   \
    {OV2640_I2C_ADDRESS,   "OV2640 Camera"},    \
    {PCA9685_I2C_ADDRESS,  "PCA9685 16-Channel LED controller, motors & servos"},   \
    {PCF8574_I2C_ADDRESS,  "PCF8574 8-Bit I/O Expander, line tracking"},    \
    {VK16K33_I2C_ADDRESS,  "VT16K33 16x8 LED controller, LED matrix"},  \
};  \
const int i2cBusDeviceTableEntries = sizeof(i2cBusDeviceTable) / sizeof(i2cBusDeviceTable[0]);

#define USE_I2C_BUS_TABLE   \
R4A_ESP32_I2C_BUS esp32I2cBus =                 \
{                                               \
    {   /* R4A_I2C_BUS */                       \
        i2cBusDeviceTable,  /* _deviceTable */  \
        sizeof(i2cBusDeviceTable) / sizeof(i2cBusDeviceTable[0]), /* _deviceTableEntries */ \
        {0,},               /* _present */      \
        false,              /* _enumerated */   \
    },                                          \
    0,                      /* _busNumber */    \
};                                              \
    R4A_PCA9685 pca9685(&esp32I2cBus._i2cBus, PCA9685_I2C_ADDRESS, 50, 25 * 1000 * 1000);   \
        R4A_PCA9685_SERVO servoPan(&pca9685, 0, 0, 180);        \
        R4A_PCA9685_SERVO servoTilt(&pca9685, 1, 2, 150);       \
        R4A_PCA9685_MOTOR motorBackLeft(&pca9685, 8, 9);        \
        R4A_PCA9685_MOTOR motorBackRight(&pca9685, 11, 10);     \
        R4A_PCA9685_MOTOR motorFrontRight(&pca9685, 13, 12);    \
        R4A_PCA9685_MOTOR motorFrontLeft(&pca9685, 14, 15);     \
    R4A_PCF8574 pcf8574(&esp32I2cBus._i2cBus, PCF8574_I2C_ADDRESS); \
    R4A_VK16K33 vk16k33 = {&esp32I2cBus._i2cBus,                \
                           VK16K33_I2C_ADDRESS,                 \
                           r4aLedMatrixColumnMap,               \
                           r4aLedMatrixRowPixelMap,             \
                           16,                                  \
                           8,                                   \
                           15};

//****************************************
// Line sensors
//****************************************

// Line sensor bits
#define LINE_SENSOR_LEFT_SHIFT      0
#define LINE_SENSOR_CENTER_SHIFT    1
#define LINE_SENSOR_RIGHT_SHIFT     2

#define LINE_SENSOR_LEFT_MASK       (1 << LINE_SENSOR_LEFT_SHIFT)
#define LINE_SENSOR_CENTER_MASK     (1 << LINE_SENSOR_CENTER_SHIFT)
#define LINE_SENSOR_RIGHT_MASK      (1 << LINE_SENSOR_RIGHT_SHIFT)

//****************************************
// Motors
//****************************************

#define USE_MOTOR_TABLE     \
R4A_PCA9685_MOTOR * const r4aPca9685MotorTable[] =  \
{                       \
    &motorFrontLeft,    \
    &motorFrontRight,   \
    &motorBackLeft,     \
    &motorBackRight     \
};                      \
const int r4aPca9685MotorTableEntries = sizeof(r4aPca9685MotorTable) / sizeof(r4aPca9685MotorTable[0]);

//****************************************
// Servos
//****************************************

#define USE_SERVO_TABLE     \
R4A_PCA9685_SERVO * const r4aPca9685ServoTable[] =  \
{               \
    &servoPan,  \
    &servoTilt, \
};              \
const int r4aPca9685ServoTableEntries = sizeof(r4aPca9685ServoTable) / sizeof(r4aPca9685ServoTable[0]);

#endif  // __FREENOVE_4WD_CAR_H__
