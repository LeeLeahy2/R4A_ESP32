/**********************************************************************
  FPGA_Robot.ino

  Sample sketch to control the FPGA robot

         Freenove FNK0060
          ESP32 WROVER-E               FPGA            SPI Flash
        .----------------.        .------------.      .---------.
        |                |        |            |      | 26f032B |
        |        SCLK 27 |------->| W7         |----->| SCLK    |
        |        MOSI 26 |------->| V8         |----->| MOSI    |
        |        MISO 25 |<-------| AA4        |<-----| MISO    |
        |                |        |            |----->| CS#     |
        |                |        |            |----->| WP#     |
        |                |        |            |----->| HOLD#   |
        |                |        |            |      '---------'
        |         SDA 13 |<------>| L6         |
        |         SCL 14 |<------>| F4         |
        |                |        |            |
        '----------------'        '------------'

**********************************************************************/

//****************************************
// Includes
//****************************************

#include <R4A_Freenove_4WD_Car.h>   // Freenove 4WD Car configuration

#define USE_I2C
//#define USE_OV2640

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
#ifdef  USE_OV2640
    CHALLENGE_CLF,      // 3
#endif  // USE_OV2640
    // Add new values before this line
    CHALLENGE_MAX       // 4
};

//****************************************
// Parameter default values
//****************************************

#define DOWNLOAD_AREA       "/nvm/"

// Telnet port number
#define TELNET_PORT         23

//****************************************
// Parameters
//****************************************

// Use the defalut values above in Parameters.h
#include "Parameters.h"

//****************************************
// FPGA configuration
//****************************************

// Define the bits in the SPI Flash control register
#define FPGA_SPI_FLASH_WRITE_ENABLE     1

//****************************************
// Forward routine declarations
//****************************************

typedef void (*START_CHALLENGE)(Print * display);

bool contextCreate(void ** contextData, NetworkClient * client);
void alfStart(Print * display);
void blfStart(Print * display);
void clfStart(Print * display);

void spiFlashDisplayStatus(uint8_t status, Print * display);
bool spiFlashWriteEnable(bool enable);

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

#define I2C_SCL                 14
#define I2C_SDA                 13

// I2C addresses
#define SPI_FLASH_I2C_ADDRESS   0x32

const R4A_I2C_DEVICE_DESCRIPTION i2cBusDeviceTable[] =
{
    {R4A_I2C_GENERAL_CALL_DEVICE_ADDRESS, "General Call"},  // 0x00
    {PCF8574_I2C_ADDRESS,     "PCF8574 8-Bit I/O Expander, line tracking"}, // 0x20
    {SPI_FLASH_I2C_ADDRESS,   "SPI Flash Controller"}, // 0x32
    {PCA9685_I2C_ADDRESS,     "PCA9685 16-Channel LED controller, motors & servos"}, // 0x5f
    {OV2640_I2C_ADDRESS,      "OV2640 Camera"}, // 0x70
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

bool generalCallPresent;
bool ov2640Present;
bool pca9685Present;
bool pcf8574Present;

//****************************************
// Light Sensor
//****************************************

int16_t lsAdcReference;
int16_t lsAdcValue;

//****************************************
// Line sensors
//****************************************

uint8_t lineSensors;        // Last value of the line sensors

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
#ifdef  USE_OV2640
    clfStart,   // 3
#endif  // USE_OV2640
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
// SPI support
//****************************************

// SPI controller connected to the SPI bus
const R4A_SPI_BUS spiBus =
{
    1,      // SPI bus number
    27,     // SCLK GPIO
    26,     // MOSI GPIO
    25,     // MISO GPIO
    r4aEsp32SpiTransfer // SPI transfer routine
};

// SPI Flash Status register
#define STATUS_BUSY         0x80    // Write operation in progress
#define STATUS_SEC          0x20    // Security ID status
#define STATUS_WPLD         0x10    // Write protect lock down
#define STATUS_WSP          0x08    // Programming suspended
#define STATUS_WSE          0x04    // Erase suspended
#define STATUS_WEL          0x02    // Write enable

#define MAP_BPR_BIT(x)      (((79 - x) & ~7) + (x & 7))

const R4A_SPI_FLASH_PROTECTION spiFlashBlockProtection[]
{
    {       0, MAP_BPR_BIT(65), MAP_BPR_BIT(64)},
    {  0x2000, MAP_BPR_BIT(67), MAP_BPR_BIT(66)},
    {  0x4000, MAP_BPR_BIT(69), MAP_BPR_BIT(68)},
    {  0x6000, MAP_BPR_BIT(71), MAP_BPR_BIT(70)},
    {  0x8000,              -1, MAP_BPR_BIT(62)},
    { 0x10000,              -1, MAP_BPR_BIT( 0)},
    { 0x20000,              -1, MAP_BPR_BIT( 1)},
    { 0x30000,              -1, MAP_BPR_BIT( 2)},
    { 0x40000,              -1, MAP_BPR_BIT( 3)},
    { 0x50000,              -1, MAP_BPR_BIT( 4)},
    { 0x60000,              -1, MAP_BPR_BIT( 5)},
    { 0x70000,              -1, MAP_BPR_BIT( 6)},
    { 0x80000,              -1, MAP_BPR_BIT( 7)},
    { 0x90000,              -1, MAP_BPR_BIT( 8)},
    { 0xa0000,              -1, MAP_BPR_BIT( 9)},
    { 0xb0000,              -1, MAP_BPR_BIT(10)},
    { 0xc0000,              -1, MAP_BPR_BIT(11)},
    { 0xd0000,              -1, MAP_BPR_BIT(12)},
    { 0xe0000,              -1, MAP_BPR_BIT(13)},
    { 0xf0000,              -1, MAP_BPR_BIT(14)},
    {0x100000,              -1, MAP_BPR_BIT(15)},
    {0x110000,              -1, MAP_BPR_BIT(16)},
    {0x120000,              -1, MAP_BPR_BIT(17)},
    {0x130000,              -1, MAP_BPR_BIT(18)},
    {0x140000,              -1, MAP_BPR_BIT(19)},
    {0x150000,              -1, MAP_BPR_BIT(20)},
    {0x160000,              -1, MAP_BPR_BIT(21)},
    {0x170000,              -1, MAP_BPR_BIT(22)},
    {0x180000,              -1, MAP_BPR_BIT(23)},
    {0x190000,              -1, MAP_BPR_BIT(24)},
    {0x1a0000,              -1, MAP_BPR_BIT(25)},
    {0x1b0000,              -1, MAP_BPR_BIT(26)},
    {0x1c0000,              -1, MAP_BPR_BIT(27)},
    {0x1d0000,              -1, MAP_BPR_BIT(28)},
    {0x1e0000,              -1, MAP_BPR_BIT(29)},
    {0x1f0000,              -1, MAP_BPR_BIT(30)},
    {0x200000,              -1, MAP_BPR_BIT(31)},
    {0x210000,              -1, MAP_BPR_BIT(32)},
    {0x220000,              -1, MAP_BPR_BIT(33)},
    {0x230000,              -1, MAP_BPR_BIT(34)},
    {0x240000,              -1, MAP_BPR_BIT(35)},
    {0x250000,              -1, MAP_BPR_BIT(36)},
    {0x260000,              -1, MAP_BPR_BIT(37)},
    {0x270000,              -1, MAP_BPR_BIT(38)},
    {0x280000,              -1, MAP_BPR_BIT(39)},
    {0x290000,              -1, MAP_BPR_BIT(40)},
    {0x2a0000,              -1, MAP_BPR_BIT(41)},
    {0x2b0000,              -1, MAP_BPR_BIT(42)},
    {0x2c0000,              -1, MAP_BPR_BIT(43)},
    {0x2d0000,              -1, MAP_BPR_BIT(44)},
    {0x2e0000,              -1, MAP_BPR_BIT(45)},
    {0x2f0000,              -1, MAP_BPR_BIT(46)},
    {0x300000,              -1, MAP_BPR_BIT(47)},
    {0x310000,              -1, MAP_BPR_BIT(48)},
    {0x320000,              -1, MAP_BPR_BIT(49)},
    {0x330000,              -1, MAP_BPR_BIT(50)},
    {0x340000,              -1, MAP_BPR_BIT(51)},
    {0x350000,              -1, MAP_BPR_BIT(52)},
    {0x360000,              -1, MAP_BPR_BIT(53)},
    {0x370000,              -1, MAP_BPR_BIT(54)},
    {0x380000,              -1, MAP_BPR_BIT(55)},
    {0x390000,              -1, MAP_BPR_BIT(56)},
    {0x3a0000,              -1, MAP_BPR_BIT(57)},
    {0x3b0000,              -1, MAP_BPR_BIT(58)},
    {0x3c0000,              -1, MAP_BPR_BIT(59)},
    {0x3d0000,              -1, MAP_BPR_BIT(60)},
    {0x3e0000,              -1, MAP_BPR_BIT(61)},
    {0x3f0000,              -1, MAP_BPR_BIT(63)},
    {0x3f8000, MAP_BPR_BIT(73), MAP_BPR_BIT(72)},
    {0x3fa000, MAP_BPR_BIT(75), MAP_BPR_BIT(74)},
    {0x3fc000, MAP_BPR_BIT(77), MAP_BPR_BIT(76)},
    {0x3fe000, MAP_BPR_BIT(79), MAP_BPR_BIT(78)},
    {0x400000,              -1,              -1},
};
const int spiFlashBlockProtectionEntries = sizeof(spiFlashBlockProtection) / sizeof(spiFlashBlockProtection[0]);

// Handle for device data for SPI driver
spi_device_handle_t spiFlashHandle;

// SPI flash connected to the SPI bus
const R4A_SPI_FLASH spiFlash =
{
    {
        &spiBus,            // SPI bus
        &spiFlashHandle,    // Handle for the SPI flash device
        1 * 1000 * 1000,    // Clock frequency
        33,                 // Chip select bar pin
        0,                  // Value to enable chip operations
        0,                  // Clock polarity
        0,                  // Clock phase
    },
    -1,                     // Hold pin
    -1,                     // Write protect pin
    spiFlashWriteEnable,    // Routine to update the SPI chip write enable pin
    spiFlashDisplayStatus,  // Routine to display the SPI Flash status
    spiFlashBlockProtection, // Flash block protection table
    0x00400000,             // Flash size in bytes
    10,                     // Length in bytes of the block protect register
    STATUS_BUSY,            // Status bit indicating write activity
    STATUS_WSE,             // Status bits indicating erase errors
    STATUS_WSP              // Status bits indicating program errors
};

const R4A_SPI_FLASH * r4aSpiFlash;

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

    // Initialize the SPI controller
    log_v("r4aEsp32SpiBegin");
    if (!r4aEsp32SpiBegin(&spiBus))
        r4aReportFatalError("Failed to initialize the SPI controller!");

    // Select the SPI flash device
    log_v("Calling r4aEsp32SpiDeviceSelect");
    if (!r4aEsp32SpiDeviceHandleInit(&spiFlash._flashChip))
        r4aReportFatalError("Failed to select the flash device on the SPI bus!");

    // Initialize the SPI flash
    r4aSpiFlash = &spiFlash;
    if (!r4aSpiFlashBegin(r4aSpiFlash))
        r4aReportFatalError("Failed to initialize the SPI Flash!");

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

    // Computing the time outside the loop
    currentUsec = esp_timer_get_time();
    loopCore1OutTimeUsec[loopIndex] = currentUsec - loopEndTimeUsec;

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

        // Update the telnet server and clients
        if (DEBUG_LOOP_CORE_1)
            callingRoutine("telnet.update");
        telnet.update(telnetEnable, r4aWifiStationOnline);

        // Update the SPI Flash server
        r4aSpiFlashServerUpdate(spiFlashServerEnable, r4aWifiStationOnline);

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

    // Process serial commands
    if (DEBUG_LOOP_CORE_1)
        callingRoutine("r4aSerialMenu");
    r4aSerialMenu(&serialMenu);

    // Update the loop time
    if (loopsCore1 < LOOP_CORE_1_TIME_ENTRIES)
        loopsCore1 += 1;
    endUsec = esp_timer_get_time();
    loopEndTimeUsec = endUsec;
    loopCore1TimeUsec[loopIndex] = endUsec - currentUsec;
    loopIndex = (loopIndex + 1) % LOOP_CORE_1_TIME_ENTRIES;
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
    generalCallPresent = r4aI2cBusIsDevicePresent(i2cBus, R4A_I2C_GENERAL_CALL_DEVICE_ADDRESS);
    ov2640Present = r4aI2cBusIsDevicePresent(i2cBus, OV2640_I2C_ADDRESS);
    pca9685Present = r4aI2cBusIsDevicePresent(i2cBus, PCA9685_I2C_ADDRESS);
    pcf8574Present = r4aI2cBusIsDevicePresent(i2cBus, PCF8574_I2C_ADDRESS);

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

    // Computing the time outside the loop
    currentUsec = esp_timer_get_time();
    loopCore0OutTimeUsec[loopIndex] = currentUsec - loopEndTimeUsec;

    // Get the time since boot
    currentMsec = millis();

    // Perform the robot challenge
    if (DEBUG_LOOP_CORE_0)
        callingRoutine("r4aRobotUpdate");
    r4aRobotUpdate(&robot, currentMsec);

    // Update the loop time
    if (loopsCore0 < LOOP_CORE_0_TIME_ENTRIES)
        loopsCore0 += 1;
    endUsec = esp_timer_get_time();
    loopEndTimeUsec = endUsec;
    loopCore0TimeUsec[loopIndex] = endUsec - currentUsec;
    loopIndex = (loopIndex + 1) % LOOP_CORE_0_TIME_ENTRIES;
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
}

//*********************************************************************
// Display the name of the next routine that will be executed
void callingRoutine(const char * name)
{
    Serial.printf("Calling %s on core %d\r\n", name, xPortGetCoreID());
    Serial.flush();
}
