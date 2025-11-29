/**********************************************************************
  Freenove_4WD_Car.ino

  Sample sketch to control the Freenove 4WD car
  https://store.freenove.com/products/fnk0053
  https://www.amazon.com/gp/product/B08X6PTQFM/ref=ox_sc_act_title_1
**********************************************************************/

//****************************************
// Includes
//****************************************

#include <R4A_Freenove_4WD_Car.h>   // Freenove 4WD Car configuration
#include <R4A_SparkFun.h>           // SparkFun Electronics boards
#include "Sensor_Table.h"

#define USE_I2C
//#define USE_NTRIP
//#define USE_OV2640
//#define USE_SPARKFUN_SEN_13582
//#define USE_WAYPOINT_FOLLOWING
//#define USE_ZED_F9P

//****************************************
// Constants
//****************************************

#define DEBUG_BOOT              0
#define DEBUG_LOOP_CORE_0       0
#define DEBUG_LOOP_CORE_1       0

// Servo default starting position
#define SERVO_PAN_START     90  // Degrees
#define SERVO_TILT_START    15  // Degrees

enum CHALLENGE_INDEX
{
    CHALLENGE_NONE = 0,
    CHALLENGE_ALF,      // 1
    CHALLENGE_BLF,      // 2
    CHALLENGE_BLT,      // 3
#ifdef  USE_OV2640
    CHALLENGE_CLF,      // 4
#endif  // USE_OV2640
#ifdef  USE_WAYPOINT_FOLLOWING
#ifdef  USE_ZED_F9P
    CHALLENGE_WPF,      // 5
#endif  // USE_ZED_F9P
#endif  // USE_WAYPOINT_FOLLOWING
    // Add new values before this line
    CHALLENGE_MAX       // 6
};

//****************************************
// Parameter default values
//****************************************

#define DOWNLOAD_AREA       "/nvm/"

#define GNSS_POINTS_PER_SECOND      1
#define GNSS_POINTS_TO_AVERAGE      (10 * GNSS_POINTS_PER_SECOND)   // 10 Seconds

// Telnet port number
#define TELNET_PORT         23

//****************************************
// Parameters
//****************************************

// Use the defalut values above in Parameters.h
#include "Parameters.h"

//****************************************
// Forward routine declarations
//****************************************

typedef void (*START_CHALLENGE)(Print * display);

bool contextCreate(void ** contextData, NetworkClient * client);
void alfStart(Print * display);
void blfStart(Print * display);
void bltStart(Print * display);
void clfStart(Print * display);
void wpfStart(Print * display);

//****************************************
// OV2640 camera
//****************************************

// List the users of the camera, one bit per user
enum CAMERA_USERS
{
    CAMERA_USER_DISABLED = 0,
    CAMERA_USER_CLF,
    CAMERA_USER_SERVO,
    CAMERA_USER_WEB_SERVER,
};

#ifdef  USE_OV2640
// Forward routine declarations
bool ov2640ProcessWebServerFrameBuffer(camera_fb_t * frameBuffer, Print * display);
#endif  // USE_OV2640

//****************************************
// Battery macros
//****************************************

#define ADC_REFERENCE_VOLTAGE   3.48    // Volts

//****************************************
// CPU core management
//****************************************

volatile bool core0Initialized;
volatile bool core1Initialized;

//****************************************
// I2C bus configuration
//****************************************

// I2C addresses
#define AK09916_I2C_ADDRESS     0x0c
#define SX1509_I2C_ADDRESS      0x3e
#define ICM20948_I2C_ADDRESS    0x69
#define ZEDF9P_I2C_ADDRESS      0x42

const R4A_I2C_DEVICE_DESCRIPTION i2cBusDeviceTable[] =
{
    {R4A_I2C_GENERAL_CALL_DEVICE_ADDRESS, "General Call"},  // 0x00
    {AK09916_I2C_ADDRESS,  "AK09916 3-axis Electronic Compass"},    // 0x0c
    {PCF8574_I2C_ADDRESS,  "PCF8574 8-Bit I/O Expander, line tracking"}, // 0x20
    {SX1509_I2C_ADDRESS,   "SX1509 Level Shifting GPIO Expander"}, // 0x3e
    {ZEDF9P_I2C_ADDRESS,   "u-blox ZED F9P GNSS receiver"}, // 0x42
    {PCA9685_I2C_ADDRESS,  "PCA9685 16-Channel LED controller, motors & servos"}, // 0x5f
    {ICM20948_I2C_ADDRESS, "ICM-20948 9-Axis MEMS Motion Tracking Device"}, // 0x69
    {OV2640_I2C_ADDRESS,   "OV2640 Camera"}, // 0x70
    {VK16K33_I2C_ADDRESS,  "VT16K33 16x8 LED controller, LED matrix"}, // 0x71
};

R4A_ESP32_I2C_BUS esp32I2cBus =
{
    {   // R4A_I2C_BUS
        i2cBusDeviceTable,      // _deviceTable
        sizeof(i2cBusDeviceTable) / sizeof(i2cBusDeviceTable[0]), // _deviceTableEntries
        {0,},                   // _present
        false,                  // _enumerated
    },
    R4A_4WD_CAR_I2C_BUS_NUMBER, // _busNumber
};

R4A_I2C_BUS * r4aI2cBus; // I2C bus for menu system

    R4A_PCA9685 pca9685(&esp32I2cBus._i2cBus, PCA9685_I2C_ADDRESS, 50, 25 * R4A_FREQ_MHz);
        R4A_PCA9685_SERVO servoPan(&pca9685, 0, 0, 180);
        R4A_PCA9685_SERVO servoTilt(&pca9685, 1, 2, 150);
        R4A_PCA9685_MOTOR motorBackLeft(&pca9685, 8, 9);
        R4A_PCA9685_MOTOR motorBackRight(&pca9685, 11, 10);
        R4A_PCA9685_MOTOR motorFrontRight(&pca9685, 13, 12);
        R4A_PCA9685_MOTOR motorFrontLeft(&pca9685, 14, 15);
    R4A_PCF8574 pcf8574(&esp32I2cBus._i2cBus, PCF8574_I2C_ADDRESS);
#ifdef  USE_OV2640
    const R4A_OV2640_SETUP ov2640 =
    {
        &r4a4wdCarOv2640Pins,   // ESP32 GPIO pins for the 0V2640 camera
        20 * 1000 * 1000,       // Input clock frequency for the OV2640
        LEDC_TIMER_0,           // Timer producing the 2x clock frequency
        LEDC_CHANNEL_0,         // Channel dividing the clock frequency to 1x
        OV2640_I2C_ADDRESS,     // Device address of the OV2640 on the SCCB bus
        10,                     // Value for JPEG quality
        PIXFORMAT_JPEG,         // Value specifying the pixel format
        FRAMESIZE_CIF,          // Value specifying the frame size
        3                       // Number of frame buffers to allocate
    };
#endif  // USE_OV2640
#ifdef  USE_SPARKFUN_SEN_13582
    R4A_SX1509 sx1509 =
    {
        &esp32I2cBus._i2cBus,
        SX1509_I2C_ADDRESS
    };
#endif  // USE_SPARKFUN_SEN_13582
    // LED matrix is 16 pixels wide by 8 high, use maximum brightness (15)
    R4A_VK16K33 vk16k33 = {&esp32I2cBus._i2cBus,
                           VK16K33_I2C_ADDRESS,
                           r4aLedMatrixColumnMap,
                           r4aLedMatrixRowPixelMap,
                           16,
                           8,
                           1}; // Brightness (0 - 15)
#ifdef  USE_ZED_F9P
    R4A_ZED_F9P zedf9p(&esp32I2cBus._i2cBus, ZEDF9P_I2C_ADDRESS);
#endif  // USE_ZED_F9P

bool ak09916Present;
bool generalCallPresent;
bool icm20948Present;
bool ov2640Present;
bool pca9685Present;
bool pcf8574Present;
bool sx1509Present;
bool vk16k33Present;
bool zedf9pPresent;

//****************************************
// Light Sensor
//****************************************

int16_t lsAdcReference;
int16_t lsAdcValue;

//****************************************
// Logging
//****************************************

typedef struct _LOG_ENTRY
{
    uint32_t _microSec;
    int16_t _leftSpeed;
    int16_t _rightSpeed;
    uint8_t _state;
    uint8_t _lineSensors;
    uint16_t _reserved;
} LOG_ENTRY;

uint8_t * logBuffer;    // Buffer for logging
uint32_t logStartUsec;  // Start time in microseconds

uint8_t lineSensors;    // Last value of the line sensors
uint8_t previousLineSensors;
int16_t robotLeftSpeed;
int16_t robotRightSpeed;

Print * logPrint;       // Network connection for logging

//****************************************
// Loop globals
//****************************************

#define LOOP_CORE_0_TIME_ENTRIES    8192
#define LOOP_CORE_1_TIME_ENTRIES    8192

R4A_TIME_USEC_t * loopCore0OutTimeUsec;
R4A_TIME_USEC_t * loopCore0TimeUsec;
R4A_TIME_USEC_t * loopCore1OutTimeUsec;
R4A_TIME_USEC_t * loopCore1TimeUsec;
uint32_t loopsCore0;
uint32_t loopsCore1;

//****************************************
// Menus
//****************************************

extern const R4A_MENU_TABLE menuTable[];
extern const int menuTableEntries;
extern const int menuTableIndexMax;

//****************************************
// Motor support
//****************************************

USE_MOTOR_TABLE;

bool robotMotorSetSpeeds(int16_t left, int16_t right, Print * display = nullptr);

//****************************************
// Robot
//****************************************

#define ROBOT_MINIMUM_VOLTAGE       8.0 // Don't start robot if below this voltage

//****************************************
// Robot operation
//****************************************

bool ignoreBatteryCheck;
void robotIdle(uint32_t currentMsec);
void robotDisplayTime(uint32_t milliseconds);

R4A_ROBOT robot;

START_CHALLENGE challengeList[] =
{
    nullptr,
    alfStart,   // 1
    blfStart,   // 2
    bltStart,   // 3
#ifdef  USE_OV2640
    clfStart,   // 4
#endif  // USE_OV2640
#ifdef  USE_WAYPOINT_FOLLOWING
#ifdef  USE_ZED_F9P
    wpfStart,   // 5
#endif  // USE_ZED_F9P
#endif  // USE_WAYPOINT_FOLLOWING
    // Add new values before this line
};
const int challengeListEntries = sizeof(challengeList) / sizeof(challengeList[0]);

//****************************************
// Serial menu support
//****************************************

R4A_MENU serialMenu;

//****************************************
// Servos
//****************************************

USE_SERVO_TABLE;

//****************************************
// SPI support - WS2812 LEDs
//****************************************

R4A_4WD_CAR_SPI_WS2812_GLOBALS;

//****************************************
// Web server
//****************************************

// Forward routine declarations
void webServerConfigUpdate(R4A_WEB_SERVER * object, httpd_config_t * config);
bool webServerRegisterErrorHandlers(R4A_WEB_SERVER * object);
bool webServerRegisterUriHandlers(R4A_WEB_SERVER * object);

// Web server object
R4A_WEB_SERVER webServer =
{
    webServerConfigUpdate,          // _configUpdate
    webServerRegisterErrorHandlers, // _registerErrorHandlers
    webServerRegisterUriHandlers,   // _registerUriHandlers
    80,         // _port
    nullptr,    // _webServer
};

//****************************************
// WiFi support
//****************************************

R4A_TELNET_SERVER telnet(4,
                         r4aTelnetContextProcessInput,
                         contextCreate,
                         r4aTelnetContextDelete);

const R4A_SSID_PASSWORD r4aWifiSsidPassword[] =
{
    {&wifiSSID1, &wifiPassword1},
    {&wifiSSID2, &wifiPassword2},
    {&wifiSSID3, &wifiPassword3},
    {&wifiSSID4, &wifiPassword4},
};
const int r4aWifiSsidPasswordEntries = sizeof(r4aWifiSsidPassword)
                                     / sizeof(r4aWifiSsidPassword[0]);

//*********************************************************************
// Entry point for the application
void setup()
{
    BaseType_t status;

    // Initialize the USB serial port
    Serial.begin(115200);
    Serial.println();
    Serial.printf("Freenove 4WD Car\r\n");

    // Display the core number
    log_v("setup() running on core %d\r\n", xPortGetCoreID());

    // Validate the tables
    log_v("validateTables");
    validateTables();

    // Get the parameters
    log_v("Calling r4aEsp32NvmGetParameters");
    r4aEsp32NvmGetParameters(&parameterFilePath);

    // Initialize the menus
    log_v("Calling r4aMenuBegin");
    r4aMenuBegin(&serialMenu, menuTable, menuTableEntries);

    // Set the ADC reference voltage
    log_v("Calling r4aEsp32VoltageSetReference");
    r4aEsp32VoltageSetReference(ADC_REFERENCE_VOLTAGE);

    // Turn off ESP32 Wrover blue LED when battery power is applied
    if (enableBatteryCheck)
    {
        log_v("Setting the blue LED\r\n");
        float batteryVoltage = READ_BATTERY_VOLTAGE(nullptr);
        int blueLED = (batteryVoltage > 2.)
                    ? ESP32_WROVER_BLUE_LED_ON : ESP32_WROVER_BLUE_LED_OFF;
        digitalWrite(BLUE_LED_BUZZER_PIN, blueLED);
    }

    // Delay to allow the hardware initialize
    delay(1000);

    // Start the core 0 task
    log_v("Calling xTaskCreatePinnedToCore");
    status = xTaskCreatePinnedToCore(
                  setupCore0,   // Function to implement the task
                  "Core 0",     // Name of the task
                  10000,        // Stack size in words
                  NULL,         // Task input parameter
                  0,            // Priority of the task
                  nullptr,      // Task handle.
                  0);           // Core where the task should run
    if (status != pdPASS)
        r4aReportFatalError("Failed to create the core 0 task!");
    log_v("Core 0 task started");

    // Initialize the SPI controller and WS2812 LEDs
    R4A_4WD_CAR_SPI_WS2812_SETUP(1);

    // Start WiFi if enabled
    log_v("Calling r4aWifiBegin");
    r4aWifiBegin();

    // Initialize the NTP client
    if (ntpEnable)
    {
        log_v("Calling r4aNtpSetup");
        r4aNtpSetup(-10 * R4A_SECONDS_IN_AN_HOUR, true);
    }

    // Initialize the web server
    r4aWebServerInit(CAMERA_USER_WEB_SERVER);

    // Enable web server debugging
    r4aWebServerDebug = webServerDebug ? &Serial : nullptr;

    // Allocate the loop buffers
    uint32_t length = sizeof(R4A_TIME_USEC_t) * LOOP_CORE_1_TIME_ENTRIES;
    loopCore1TimeUsec = (R4A_TIME_USEC_t *)r4aMalloc(length, "Core 1 loop time buffer (loopCore1TimeUsec)");
    if (!loopCore1TimeUsec)
        r4aReportFatalError("Failed to allocate loopCore1TimeUsec!");
    loopCore1OutTimeUsec = (R4A_TIME_USEC_t *)r4aMalloc(length, "Core 1 out of loop time buffer (loopCore1OutTimeUsec)");
    if (!loopCore1OutTimeUsec)
        r4aReportFatalError("Failed to allocate loopCore1OutTimeUsec!");

    //****************************************
    // Synchronize with core 0
    //****************************************

    // Wait for the other core to finish initialization
    log_v("Waiting for setupCore0 to complete");
    while (!core0Initialized)
        delayMicroseconds(1);

    // Initialize the camera
#ifdef USE_OV2640
    r4aCameraUserAdd(CAMERA_USER_DISABLED);
    if (ov2640Present)
    {
        if (ov2640Enable == false)
        {
            ov2640Present = false;
            Serial.printf("WARNING: OV2640 camera is disabled!\r\n");
        }
        else
        {
            log_v("Calling r4aOv2640Setup");
            Serial.printf("Initializing the OV2640 camera\r\n");
            if (r4aOv2640Setup(&ov2640))
                r4aCameraUserRemove(CAMERA_USER_DISABLED);
        }
    }
#endif  // USE_OV2640

    //****************************************
    // Core 1 completed initialization
    //****************************************

    // Finished with the initialization
    log_v("setup complete");
    core1Initialized = true;

    //****************************************
    // Start the requested challenge
    //****************************************

    if (startIndex && (startIndex < CHALLENGE_MAX))
    {
        bool ignore;

        ignore = ignoreBatteryCheck;
        ignoreBatteryCheck = true;
        challengeList[startIndex](&Serial);
        ignoreBatteryCheck = ignore;
    }

    //****************************************
    // Execute loop forever
    //****************************************
}

//*********************************************************************
// Idle loop for core 1 of the application
void loop()
{
    uint32_t currentMsec;
    R4A_TIME_USEC_t currentUsec;
    R4A_TIME_USEC_t endUsec;
    static uint32_t lastBatteryCheckMsec;
    static R4A_TIME_USEC_t loopEndTimeUsec;
    static uint32_t loopIndex;
    bool saveLoopTime;

    // Remember the start of the look
    currentUsec = esp_timer_get_time();

    // Determine if the loop times should be saved;
    saveLoopTime = r4aRobotIsRunning(&robot);

    // Turn on the ESP32 WROVER blue LED when the battery power is OFF
    currentMsec = millis();
    if (enableBatteryCheck)
    {
        if ((currentMsec - lastBatteryCheckMsec) >= 100)
        {
            lastBatteryCheckMsec = currentMsec;
            if (DEBUG_LOOP_CORE_1)
                callingRoutine("READ_BATTERY_VOLTAGE");
            float batteryVoltage = READ_BATTERY_VOLTAGE(nullptr);
            int blueLED = (batteryVoltage > 2.)
                        ? ESP32_WROVER_BLUE_LED_ON : ESP32_WROVER_BLUE_LED_OFF;
            digitalWrite(BLUE_LED_BUZZER_PIN, blueLED);
        }
    }

    // Update the location
#ifdef  USE_ZED_F9P
    if (zedf9pPresent)
    {
        if (DEBUG_LOOP_CORE_1)
            callingRoutine("zedf9p.update");
        zedf9p.update(currentMsec, nullptr);
    }
#endif  // USE_ZED_F9P

    // Update the LEDs
    if (DEBUG_LOOP_CORE_1)
        callingRoutine("car.ledsUpdate");
    car.ledsUpdate(currentMsec);

    // Update the WiFi status
    r4aWifiUpdate();

    // Determine if WiFi station mode is configured
    if (r4aWifiSsidPasswordEntries)
    {
        // Check for NTP updates
        if (ntpEnable && (r4aNtpIsTimeValid() == false))
        {
            if (DEBUG_LOOP_CORE_1)
                callingRoutine("r4aNtpUpdate");
            r4aNtpUpdate(r4aWifiStationOnline);
        }

#ifdef  USE_NTRIP
        // Update the NTRIP client state
        if (r4aNtripClientEnable)
        {
            if (DEBUG_LOOP_CORE_1)
                callingRoutine("r4aNtripClientUpdate");
            r4aNtripClientUpdate(r4aWifiStationOnline, &Serial);
        }
#endif  // USE_NTRIP

        // Update the telnet server and clients
        if (DEBUG_LOOP_CORE_1)
            callingRoutine("telnet.update");
        telnet.update(telnetEnable, r4aWifiStationOnline);

        // Output the debug log data
        while (logDataPrint());

        // Update the web server
        if (DEBUG_LOOP_CORE_1)
            callingRoutine("r4aWebServerUpdate");
        r4aWebServerUpdate(&webServer, r4aWifiStationOnline && webServerEnable);
    }

#ifdef  USE_OV2640
    // Discard frame buffers
    if (r4aCameraUsers == 0)
    {
        if (DEBUG_LOOP_CORE_1)
            callingRoutine("r4aCameraFrameBufferGet");
        camera_fb_t * frameBuffer = r4aCameraFrameBufferGet();
        if (frameBuffer)
        {
            if (DEBUG_LOOP_CORE_1)
                callingRoutine("r4aCameraFrameBufferFree");
            r4aCameraFrameBufferFree(frameBuffer);
        }
    }
#endif  // USE_OV2640

    // Display the robot's runtime
    if (robotRunTime && r4aRobotIsRunning(&robot))
    {
        static uint32_t lastMsec;
        if ((currentMsec - lastMsec) >= 100)
        {
            lastMsec = currentMsec;
            robotDisplayTime(r4aRobotGetRunTime(&robot, currentMsec));
        }
    }

    // Update the time display
    else if (robotNtpTime && r4aNtpIsTimeValid() && (!r4aRobotIsActive(&robot)))
        vk16k33NtpTime(currentMsec);

    // Process serial commands
    if (DEBUG_LOOP_CORE_1)
        callingRoutine("r4aSerialMenu");
    r4aSerialMenu(&serialMenu);

    // Determine if the loop times should be saved
    endUsec = esp_timer_get_time();
    if (saveLoopTime)
    {
        // Count the loops
        if (loopsCore1 < LOOP_CORE_1_TIME_ENTRIES)
            loopsCore1 += 1;

        // Computing the time outside the loop
        loopCore1OutTimeUsec[loopIndex] = currentUsec - loopEndTimeUsec;

        // Compute the time inside the loop
        loopCore1TimeUsec[loopIndex] = endUsec - currentUsec;

        // Set the next loop index
        loopIndex = (loopIndex + 1) % LOOP_CORE_1_TIME_ENTRIES;
    }

    // Update the loop time
    loopEndTimeUsec = endUsec;
}

//*********************************************************************
// Setup for core 0
void setupCore0(void *parameter)
{
    R4A_I2C_BUS * i2cBus;

    // Display the core number
    log_v("setupCore0() running on core %d", xPortGetCoreID());

#ifdef USE_I2C
    // Initialize the I2C bus
    log_v("Calling r4aEsp32I2cBusBegin");
    r4aEsp32I2cBusBegin(&esp32I2cBus,
                        I2C_SDA,
                        I2C_SCL,
                        R4A_I2C_FAST_MODE_HZ,
                        false);
    i2cBus = &esp32I2cBus._i2cBus;
    r4aI2cBus = i2cBus;

    // Allow I2C devices time to power up
    delay(100);

    // Enumerate the I2C bus
    log_v("Calling r4aI2cBusEnumerate");
    r4aI2cBusEnumerate(r4aI2cBus);

    // Determine which devices are present
    log_v("Calling r4aI2cBusIsDevicePresent");
    ak09916Present = r4aI2cBusIsDevicePresent(i2cBus, AK09916_I2C_ADDRESS);
    generalCallPresent = r4aI2cBusIsDevicePresent(i2cBus, R4A_I2C_GENERAL_CALL_DEVICE_ADDRESS);
    icm20948Present = r4aI2cBusIsDevicePresent(i2cBus, ICM20948_I2C_ADDRESS);
    ov2640Present = r4aI2cBusIsDevicePresent(i2cBus, OV2640_I2C_ADDRESS);
    pca9685Present = r4aI2cBusIsDevicePresent(i2cBus, PCA9685_I2C_ADDRESS);
    pcf8574Present = r4aI2cBusIsDevicePresent(i2cBus, PCF8574_I2C_ADDRESS);
    sx1509Present = r4aI2cBusIsDevicePresent(i2cBus, SX1509_I2C_ADDRESS);
    vk16k33Present = r4aI2cBusIsDevicePresent(i2cBus, VK16K33_I2C_ADDRESS);
    zedf9pPresent = r4aI2cBusIsDevicePresent(i2cBus, ZEDF9P_I2C_ADDRESS);

    // Reset the I2C devices
    if (generalCallPresent)
    {
        log_v("Calling r4aI2cCallSwReset");
        r4aI2cCallSwReset(r4aI2cBus);
    }

    // Initialize the PCA9685
    log_v("Calling pca9685.begin");
    if (pca9685.begin())
    {
        // Initialize the Pan/Tilt servos
        log_v("Calling servoPan.positionSet");
        servoPan.positionSet(servoPanStartDegrees);
        log_v("Calling servoTilt.positionSet");
        servoTilt.positionSet(servoTiltStartDegrees);
    }

    // Initialize the PCF8574
    log_v("Calling pcf8574.write");
    pcf8574.write(0xff);
#endif  // USE_I2C

    // Initialize the SX1509
#ifdef USE_SPARKFUN_SEN_13582
    if (sx1509Present)
    {
        if (r4aSfeSen13582Begin(&sx1509) == false)
            r4aReportFatalError("Failed to initialize the SX1509!");
    }
#endif  // USE_SPARKFUN_SEN_13582

    // Initialize the VK16K33
    if (vk16k33Present)
    {
        log_v("Calling r4aVk16k33Setup");
        if (!r4aVk16k33Setup(&vk16k33))
            r4aReportFatalError("Failed to initialize the VK16K33 LED Matrix controller!");
    }

    // Initialize the GPS receiver
#ifdef  USE_ZED_F9P
    if (zedf9pPresent)
    {
        log_v("Calling zedf9p.begin");
        Serial.printf("Initializing the ZED-F9P GNSS receiver\r\n");
        zedf9p.begin();
    }
#endif  // USE_ZED_F9P

    // Initialize the robot
    r4aRobotInit(&robot,
                 xPortGetCoreID(),       // CPU core
                 robotIdle,              // Idle routine
                 robotDisplayTime);      // Time display routine

    // Allocate the loop buffers
    uint32_t length = sizeof(R4A_TIME_USEC_t) * LOOP_CORE_0_TIME_ENTRIES;
    loopCore0TimeUsec = (R4A_TIME_USEC_t *)r4aMalloc(length, "Core 0 loop time buffer (loopCore0TimeUsec)");
    if (!loopCore0TimeUsec)
        r4aReportFatalError("Failed to allocate loopCore0TimeUsec!");
    loopCore0OutTimeUsec = (R4A_TIME_USEC_t *)r4aMalloc(length, "Core 0 out of loop time buffer (loopCore0OutTimeUsec)");
    if (!loopCore0OutTimeUsec)
        r4aReportFatalError("Failed to allocate loopCore0OutTimeUsec!");

    //****************************************
    // Core 0 completed initialization
    //****************************************

    log_v("setupCore0 complete");

    // Finished with the initialization
    core0Initialized = true;

    //****************************************
    // Synchronize with core 1
    //****************************************

    log_v("Waiting for setup to complete");

    // Wait for the other core to finish initialization
    while (!core1Initialized)
        delayMicroseconds(1);

    log_v("Calling loopCore0");

    //****************************************
    // Execute loopCore0 forever
    //****************************************

    while (1)
        loopCore0();
}

//*********************************************************************
// Idle loop for core 0 the application
void loopCore0()
{
    uint32_t currentMsec;
    R4A_TIME_USEC_t currentUsec;
    R4A_TIME_USEC_t endUsec;
    static R4A_TIME_USEC_t loopEndTimeUsec;
    static uint32_t loopIndex;
    bool saveLoopTime;

    // Remember the start of the look
    currentUsec = esp_timer_get_time();

    // Determine if the loop times should be saved;
    saveLoopTime = r4aRobotIsRunning(&robot);

    // Get the time since boot
    currentMsec = millis();

#ifdef  USE_ZED_F9P
    // Update the location
    if (zedf9pPresent)
    {
        static uint32_t lastGnssI2cPollMsec;
        if ((currentMsec - lastGnssI2cPollMsec) >= r4aZedF9pPollMsec)
        {
            lastGnssI2cPollMsec = currentMsec;
            if (DEBUG_LOOP_CORE_0)
                callingRoutine("zedf9p.i2cPoll");
            zedf9p.i2cPoll();
        }
    }
#endif  // USE_ZED_F9P

#ifdef  USE_NTRIP
    // Send navigation data to the GNSS radio
    if (r4aNtripClientEnable)
    {
        if (DEBUG_LOOP_CORE_0)
            callingRoutine("r4aNtripClientRbRemoveData");
        r4aNtripClientRbRemoveData(&Serial);
    }
#endif  // USE_NTRIP

    // Perform the robot challenge
    if (DEBUG_LOOP_CORE_0)
        callingRoutine("r4aRobotUpdate");
    r4aRobotUpdate(&robot, currentMsec);

    // Determine if the loop times should be saved
    endUsec = esp_timer_get_time();
    if (saveLoopTime)
    {
        // Count the loops
        if (loopsCore0 < LOOP_CORE_0_TIME_ENTRIES)
            loopsCore0 += 1;

        // Computing the time outside the loop
        loopCore0OutTimeUsec[loopIndex] = currentUsec - loopEndTimeUsec;

        // Compute the time inside the loop
        loopCore0TimeUsec[loopIndex] = endUsec - currentUsec;

        // Set the next loop index
        loopIndex = (loopIndex + 1) % LOOP_CORE_0_TIME_ENTRIES;
    }
    loopEndTimeUsec = endUsec;
}

//*********************************************************************
// Validate the tables
void validateTables()
{
    // Validate the challenge entries
    if (CHALLENGE_MAX != challengeListEntries)
        r4aReportFatalError("CHALLENGE_MAX must equal challengeListEntries!");

    // Validate the menu table index values
    if (menuTableEntries != (menuTableIndexMax - 1))
    {
        Serial.printf("menuTableEntries: %d\r\n", menuTableEntries);
        Serial.printf("menuTableIndexMax - 1: %d\r\n", menuTableIndexMax - 1);
        r4aReportFatalError("Fix menuTableEntries to match MENU_TABLE_INDEX");
    }

#ifdef  USE_NTRIP
    // Validate the NTRIP tables
    log_v("r4aNtripClientValidateTables");
    r4aNtripClientValidateTables();
#endif  // USE_NTRIP
}

//*********************************************************************
// Display the name of the next routine that will be executed
void callingRoutine(const char * name)
{
    Serial.printf("Calling %s on core %d\r\n", name, xPortGetCoreID());
    Serial.flush();
}
