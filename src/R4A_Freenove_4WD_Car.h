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
    car.batteryVoltageDisplay(0, BATTERY_VOLTAGE_MULTIPLIER, display)

#define READ_BATTERY_VOLTAGE(adcValue)  \
    car.batteryVoltageGet(0, BATTERY_VOLTAGE_MULTIPLIER, adcValue)

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
#define WS2812_MAX      12

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
    R4A_Freenove_4WD_Car(uint32_t blinkMsec = 150);

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

    // Display the voltage
    // Inputs:
    //   offset: Ground level offset correction
    //   multiplier: Multiplier for each of the ADC bits
    //   display: Device used for output
    void batteryVoltageDisplay(float offset,
                               float multiplier,
                               Print * display = &Serial);

    // Read the voltage
    // Inputs:
    //   offset: Ground level offset correction
    //   multiplier: Multiplier for each of the ADC bits
    //   adcValue: Return the value read from the ADC
    // Outputs:
    //   Returns the computed voltage
    float batteryVoltageGet(float offset,
                            float multiplier,
                            int16_t * adcValue);

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
    //   forceUpdate: Set true to force an update to the LEDS
    void ledsUpdate(uint32_t currentMsec, bool forceUpdate = false);
};

extern R4A_Freenove_4WD_Car r4aFreenove4wdCar;

#define car         r4aFreenove4wdCar

//****************************************
// ESP32 WRover Module Pins
//****************************************

#define BLUE_LED_BUZZER_PIN     2   // Blue LED and buzzer output
#define BATTERY_WS2812_PIN      32  // WS2812 output, Battery voltage input
#define LIGHT_SENSOR_PIN        33

#define ESP32_WROVER_BLUE_LED_OFF       0
#define ESP32_WROVER_BLUE_LED_ON        1

//****************************************
// I2C bus configuration
//****************************************

#define R4A_4WD_CAR_I2C_BUS_NUMBER      0
#define R4A_4WD_CAR_SCCB_BUS_NUMBER     1

// I2C controller pins
#define I2C_SDA                 13  // I2C data line
#define I2C_SCL                 14  // I2C clock line

// SCCB (I2C) controller pins
#define SCCB_SDA                26  // I2C data line
#define SCCB_SCL                27  // I2C clock line

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

#define USE_I2C_BUS_TABLE                           \
R4A_ESP32_I2C_BUS esp32I2cBus =                     \
{                                                   \
    {   /* R4A_I2C_BUS */                           \
        i2cBusDeviceTable,      /* _deviceTable */  \
        sizeof(i2cBusDeviceTable) / sizeof(i2cBusDeviceTable[0]), /* _deviceTableEntries */ \
        {0,},                   /* _present */      \
        false,                  /* _enumerated */   \
    },                                              \
    R4A_4WD_CAR_I2C_BUS_NUMBER, /* _busNumber */    \
};                                                  \
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
// LED Matrix
//****************************************

extern const uint8_t r4aLedMatrixColumnMap[R4A_VK16K33_MAX_COLUMNS];
extern const uint8_t r4aLedMatrixRowPixelMap[R4A_VK16K33_MAX_ROWS];

//****************************************
// LED Menu
//****************************************

extern const R4A_MENU_ENTRY r4a4wdCarLedMenuTable[];
#define R4A_4WD_CAR_LED_MENU_ENTRIES    12

// Toggle the backup lights
// Inputs:
//   menuEntry: Address of the object describing the menu entry
//   command: Zero terminated command string
//   display: Device used for output
void r4aLedMenuBackup(const R4A_MENU_ENTRY * menuEntry, const char * command, Print * display);

// Toggle the brake lights
// Inputs:
//   menuEntry: Address of the object describing the menu entry
//   command: Zero terminated command string
//   display: Device used for output
void r4aLedMenuBrake(const R4A_MENU_ENTRY * menuEntry, const char * command, Print * display);

// Toggle the headlights
// Inputs:
//   menuEntry: Address of the object describing the menu entry
//   command: Zero terminated command string
//   display: Device used for output
void r4aLedMenuHeadlights(const R4A_MENU_ENTRY * menuEntry, const char * command, Print * display);

// Turn off all the LEDs
// Inputs:
//   menuEntry: Address of the object describing the menu entry
//   command: Zero terminated command string
//   display: Device used for output
void r4aLedMenuOff(const R4A_MENU_ENTRY * menuEntry, const char * command, Print * display);

// Turn left indicator
// Inputs:
//   menuEntry: Address of the object describing the menu entry
//   command: Zero terminated command string
//   display: Device used for output
void r4aLedMenuTurnLeft(const R4A_MENU_ENTRY * menuEntry, const char * command, Print * display);

// Stop the turn signal blinking
// Inputs:
//   menuEntry: Address of the object describing the menu entry
//   command: Zero terminated command string
//   display: Device used for output
void r4aLedMenuTurnOff(const R4A_MENU_ENTRY * menuEntry, const char * command, Print * display);

// Turn right indicator
// Inputs:
//   menuEntry: Address of the object describing the menu entry
//   command: Zero terminated command string
//   display: Device used for output
void r4aLedMenuTurnRight(const R4A_MENU_ENTRY * menuEntry, const char * command, Print * display);

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
// OV2640 Camera
//****************************************

extern const R4A_CAMERA_PINS r4a4wdCarOv2640Pins;

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

//****************************************
// SPI support - WS2812 LEDs
//****************************************

#define R4A_4WD_CAR_SPI_WS2812_GLOBALS                  \
/* SPI controller connected to the SPI bus */           \
R4A_ESP32_SPI_CONTROLLER spiBus =                       \
{                                                       \
    {                                                   \
        1,      /* SPI bus number */                    \
        -1,     /* SCLK GPIO */                         \
        BATTERY_WS2812_PIN,   /* MOSI GPIO */           \
        -1,     /* MISO GPIO  */                        \
        r4aEsp32SpiTransfer /* SPI transfer routine */  \
    },                                                  \
    nullptr,                                            \
};                                                      \
                                                        \
/* WS2812 - RGB LED connected to the SPI bus */         \
const R4A_SPI_DEVICE ws2812 =                           \
{                                                       \
    &spiBus._spiBus, /* SPI bus */                      \
    4 * 1000 * 1000, /* Clock frequency */              \
    -1,              /* No chip select pin */           \
    0,               /* Clock polarity */               \
    0,               /* Clock phase */                  \
}

#define R4A_4WD_CAR_SPI_WS2812_SETUP(intensity)             \
    /* Initialize the SPI controller */                     \
    log_v("r4aEsp32SpiBegin");                              \
    if (!r4aEsp32SpiBegin(&spiBus))                         \
        r4aReportFatalError("Failed to initialize the SPI controller!");    \
                                                            \
    /* Select the WS2812 devices on the SPI bus */          \
    log_v("Calling r4aEsp32SpiDeviceSelect");               \
    if (!r4aEsp32SpiDeviceSelect(&ws2812))                  \
        r4aReportFatalError("Failed to select the WS2812 devices on the SPI bus!"); \
                                                            \
    /* Initialize the SPI controller for the WD2812 LEDs */ \
    log_v("Calling r4aLEDSetup");                           \
    if (!r4aLEDSetup(&ws2812, car.numberOfLEDs))            \
        r4aReportFatalError("Failed to allocate the SPI device for the WS2812 LEDs!");  \
                                                            \
    /* Turn off all of the 3 color LEDs */                  \
    log_v("Calling car.ledsOff");                           \
    car.ledsOff();                                          \
                                                            \
    /* Reduce the LED intensity */                          \
    log_v("Calling r4aLEDSetIntensity");                    \
    r4aLEDSetIntensity(intensity);                          \
                                                            \
    /* Set the initial LED values */                        \
    log_v("Calling r4aLEDUpdate");                          \
    r4aLEDUpdate(true)

#endif  // __FREENOVE_4WD_CAR_H__
