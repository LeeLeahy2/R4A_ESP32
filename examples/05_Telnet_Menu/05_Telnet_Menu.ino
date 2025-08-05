/**********************************************************************
  05_Telnet_Menu.ino

  Sample sketch supports telnet access to menus and parameters

  Menus
  -----

  The list of menus are found in menuTable at the end of the Menu.ino
  module.  The table contains the menu address and number of entries in
  the menu.  The values in the MENU_TABLE_INDEX enum found above the menus
  must match the order of the menus to properly display the menus.

  Each of the menus above the menu table contain a command, a routine and
  parameter to process the command.  When the routine is nullptr, the
  parameter one of the values from MENU_TABLE_INDEX indicating which menu
  to display.

  The rest of the entries in the menu provide data on how to display the
  help text for the command.  By default the menu is displayed with the
  command on the left followed by a colon followed by the help text.  The
  help text is aligned so that it starts in the same column for each
  command.  There are currently two uses for the help routine, both of
  which display additional text:

  1.  Common code elimination such as displaying a boolean value following
      the help text
  2.  Display additional text following the command, in this case the
      align values need to be adjusted for the menu

  Parameters
  ----------

  The parameters are listed in the Parameters.h file.  The nvmParameters
  table contains the address of the variable containing the parameter,
  the variable type, minimum and maximum values for type checking and the
  default value for the variable.  The call to r4aEsp32NvmGetParameters
  in the setup routine loads the variables with the values found in the
  parameter file.  If the file does not exist, the default values are
  used and the parameter files is created.

  Parameters are stored as a file in the spiffs (Serial Peripheral
  Interface (SPI) Flash File System) partition of flash, also known as
  non-volatile memory (NVM).  The "p" command in the debug menu is able
  to display the partitions.

  Parameters may be set and displayed using the nvm menu.  This menu also
  supports manipulating files in the spiffs partition (display, copy,
  rename, delete and download).

  NTP
  ---

  This example also includes the Network Time Protocol (NTP) support.  A
  call to r4aNtpSetup in setup establishes the NTP client and the call
  to r4aNtpUpdate in loop updates the time.

  Debugging
  ---------

  Debugging is generally done using serial output.  Calls to Serial.print*
  functions are always built into the final flash image.  The Arduino CLI
  and IDE provide a "Debug Level" to control which log_* functions are
  included into a debugging flash image.  The log_* routines are great for
  initial debugging but are usually compiled out for the final flash.  For
  the final flash image it is best to use debug variables to enable/disable
  debugging of sections of the code and Serial.print* functions to display
  the debug output.

  Memory Use and Debugging
  ------------------------

  Set the MallocMax parameter to the largest size that is allocated from
  the heap in RAM.  Larger size chunks are instead allocated from PSRAM
  (SPI RAM device).  The default value is 128 bytes.

  Use r4aMalloc to allocate memory with a unique text value describing
  how the memory is being used.  Use the same text value when calling
  r4aFree to return that block of memory to the heap.  Set r4aMallocDebug
  parameter to true to display the allocation address, size and text on
  the serial port.

  User Line Following
  -------------------

  The menu contains the "ulf" command to invoke user line following.  Add
  code to the ulfChallenge routine in the User_Line_Following.ino file.
  The routine ulfChallenge is called within an infinite loop during robot
  operation.  As such, the added code should be straight line code (no
  loops).  State variables may be used to indicate what to do on the next
  time through the routine.

**********************************************************************/

//****************************************
// Includes
//****************************************

#include <R4A_Freenove_4WD_Car.h>   // Freenove 4WD Car configuration

#define DOWNLOAD_AREA       "/nvm/"

#include "Parameters.h"

//****************************************
// Constants
//****************************************

// Telnet port number
#define TELNET_PORT         23

//****************************************
// Forward routine declarations
//****************************************

bool contextCreate(void ** contextData, NetworkClient * client);

//****************************************
// I2C bus configuration
//****************************************

USE_I2C_DEVICE_TABLE;
USE_I2C_BUS_TABLE;

R4A_I2C_BUS * r4aI2cBus; // I2C bus for menu system

bool vk16k33Present;

//****************************************
// Battery macros
//****************************************

#define ADC_REFERENCE_VOLTAGE   3.48    // Volts

//****************************************
// Line sensors
//****************************************

uint8_t lineSensors;        // Last value of the line sensors

//****************************************
// Loop globals
//****************************************

#define LOOP_CORE_1_TIME_ENTRIES    8192

R4A_TIME_USEC_t * loopCore1OutTimeUsec;
R4A_TIME_USEC_t * loopCore1TimeUsec;
uint32_t loopsCore1;

//****************************************
// Menus
//****************************************

extern const R4A_MENU_TABLE menuTable[];
extern const int menuTableEntries;

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

#define ROBOT_LINE_FOLLOW_DURATION_SEC      (3 * R4A_SECONDS_IN_A_MINUTE)

#define ROBOT_START_DELAY_SEC               5
#define ROBOT_START_DELAY_MILLISECONDS      (ROBOT_START_DELAY_SEC * R4A_MILLISECONDS_IN_A_SECOND)

#define ROBOT_STOP_TO_IDLE_SEC              30

bool ignoreBatteryCheck;
void robotIdle(uint32_t currentMsec);
void robotDisplayTime(uint32_t milliseconds);

R4A_ROBOT robot;

//****************************************
// Serial menu support
//****************************************

R4A_MENU serialMenu;

//****************************************
// WiFi support
//****************************************

R4A_TELNET_SERVER telnet(4,
                         r4aTelnetContextProcessInput,
                         contextCreate,
                         r4aTelnetContextDelete);

const R4A_SSID_PASSWORD r4aWifiSsidPassword[] =
{
    {&wifiSSID,  &wifiPassword},
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
    // Initialize the USB serial port
    Serial.begin(115200);
    Serial.println();
    Serial.printf("%s\r\n", __FILE__);

    // Get the parameters
    log_d("Calling r4aEsp32NvmGetParameters");
    r4aEsp32NvmGetParameters(&parameterFilePath);

    // Initialize the menus
    log_d("Calling r4aMenuBegin");
    r4aMenuBegin(&serialMenu, menuTable, menuTableEntries);

    // Set the ADC reference voltage
    log_d("Calling r4aEsp32VoltageSetReference");
    r4aEsp32VoltageSetReference(ADC_REFERENCE_VOLTAGE);

    // Turn off ESP32 Wrover blue LED when battery power is applied
    log_d("Setting the blue LED\r\n");
    float batteryVoltage = READ_BATTERY_VOLTAGE(nullptr);
    int blueLED = (batteryVoltage > 2.)
                ? ESP32_WROVER_BLUE_LED_ON : ESP32_WROVER_BLUE_LED_OFF;
    digitalWrite(BLUE_LED_BUZZER_PIN, blueLED);

    // Setup and enumerate the I2C devices
    log_d("Calling i2cBus.begin");
    r4aEsp32I2cBusBegin(&esp32I2cBus,
                        I2C_SDA,
                        I2C_SCL,
                        R4A_I2C_FAST_MODE_HZ);
    r4aI2cBus = &esp32I2cBus._i2cBus;

    // Delay to allow the hardware initialize
    delay(1000);

    // Determine if the LED controller is available
    log_v("Calling r4aI2cBusIsDevicePresent");
    vk16k33Present = r4aI2cBusIsDevicePresent(&esp32I2cBus._i2cBus, VK16K33_I2C_ADDRESS);

    // Initialize the PCA9685
    log_d("Calling pca9685.begin");
    pca9685.begin();

    // Initialize the PCF8574
    log_d("Calling pcf8574.write");
    pcf8574.write(0xff);

    // Start WiFi if enabled
    log_d("Calling wifiBegin");
    r4aWifiBegin();

    // Initialize the NTP client
    log_d("Calling r4aNtpSetup");
    r4aNtpSetup(-10 * R4A_SECONDS_IN_AN_HOUR, true);

    // Initialize the robot
    log_d("Calling r4aRobotInit");
    r4aRobotInit(&robot,
                 xPortGetCoreID(),       // CPU core
                 ROBOT_START_DELAY_SEC,  // Challenge start delay
                 ROBOT_STOP_TO_IDLE_SEC, // Delay after running the challenge
                 nullptr,                // Idle routine
                 nullptr);               // Time display routine

    // Allocate the loop buffers
    uint32_t length = sizeof(R4A_TIME_USEC_t) * LOOP_CORE_1_TIME_ENTRIES;
    loopCore1TimeUsec = (R4A_TIME_USEC_t *)r4aMalloc(length, "Core 1 loop time buffer (loopCore1TimeUsec)");
    if (!loopCore1TimeUsec)
        r4aReportFatalError("Failed to allocate loopCore1TimeUsec!");
    loopCore1OutTimeUsec = (R4A_TIME_USEC_t *)r4aMalloc(length, "Core 1 out of loop time buffer (loopCore1OutTimeUsec)");
    if (!loopCore1OutTimeUsec)
        r4aReportFatalError("Failed to allocate loopCore1OutTimeUsec!");

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
    static bool previousConnected;

    // Computing the time outside the loop
    currentUsec = esp_timer_get_time();
    loopCore1OutTimeUsec[loopIndex] = currentUsec - loopEndTimeUsec;

    // Turn on the ESP32 WROVER blue LED when the battery power is OFF
    currentMsec = millis();
    if ((currentMsec - lastBatteryCheckMsec) >= 100)
    {
        lastBatteryCheckMsec = currentMsec;
        log_v("READ_BATTERY_VOLTAGE");
        float batteryVoltage = READ_BATTERY_VOLTAGE(nullptr);
        int blueLED = (batteryVoltage > 2.)
                    ? ESP32_WROVER_BLUE_LED_ON : ESP32_WROVER_BLUE_LED_OFF;
        digitalWrite(BLUE_LED_BUZZER_PIN, blueLED);
    }

    // Update the WiFi status
    r4aWifiUpdate();

    // Determine if WiFi station mode is configured
    if (r4aWifiSsidPasswordEntries)
    {
        // Check for NTP updates
        log_v("r4aNtpUpdate");
        r4aNtpUpdate(r4aWifiStationOnline);

        // Notify the telnet server of WiFi changes
        if (previousConnected != r4aWifiStationOnline)
        {
            previousConnected = r4aWifiStationOnline;
            if (r4aWifiStationOnline)
            {
                log_v("telnet.begin");
                telnet.begin(WiFi.STA.localIP(), TELNET_PORT);
                Serial.printf("Telnet: %s:%d\r\n", WiFi.localIP().toString().c_str(),
                              telnet.port());
            }
            else
            {
                log_v("telnet.end");
                telnet.end();
            }
        }
        log_v("telnet.update");
        telnet.update(r4aWifiStationOnline);
    }

    // Perform the robot challenge
    log_v("r4aRobotUpdate");
    r4aRobotUpdate(&robot, currentMsec);

    // Process serial commands
    log_v("r4aSerialMenu");
    r4aSerialMenu(&serialMenu);

    // Update the loop time
    if (loopsCore1 < LOOP_CORE_1_TIME_ENTRIES)
        loopsCore1 += 1;
    endUsec = esp_timer_get_time();
    loopEndTimeUsec = endUsec;
    loopCore1TimeUsec[loopIndex] = endUsec - currentUsec;
    loopIndex = (loopIndex + 1) % LOOP_CORE_1_TIME_ENTRIES;
}
