/**********************************************************************
  Menu.ino

  Menu routines
**********************************************************************/

#ifdef  USE_ZED_F9P

//*********************************************************************
// Display the computed point
// Inputs:
//   parameter: Callback parameter passed to computeWayPoint
//   comment: Text to display at the start of the line
//   latitude: Latitude in degrees
//   longitude: Longitude in degrees
//   altitude: Altitude in meters
//   horizontalAccuracy: Accuracy in meters
//   satellitesInView: The number of satellites feeding the GNSS receiver
//   latitudeStdDev: Latitude standard deviation in degrees
//   longitudeStdDev: Longitude standard deviation in degrees
//   altitudeStdDev: Altitude standard deviation in meters
//   horizontalAccuracyStdDev: Horizontal accuracy standard deviation in meters
//   display: Device used for output, passed to computeWayPoint
void gnssDisplayPoint(intptr_t parameter,
                      const char * comment,
                      double latitude,
                      double latitudeStdDev,
                      double longitude,
                      double longitudeStdDev,
                      double altitude,
                      double altitudeStdDev,
                      double horizontalAccuracy,
                      double horizontalAccuracyStdDev,
                      uint8_t satellitesInView,
                      Print * display)
{
    zedf9p.displayLocation(comment,
                           latitude,
                           0,
                           longitude,
                           0,
                           altitude,
                           0,
                           horizontalAccuracy,
                           0,
                           satellitesInView,
                           true,    // unitsFeetInches,
                           false,   // displayTime,
                           true,    // displaySiv,
                           true,    // displayLatitude,
                           false,   // displayLatStdDev,
                           true,    // displayLongitude,
                           false,   // displayLongStdDev,
                           true,    // displayHorizAcc,
                           false,   // displayHorizAccStdDev,
                           false,   // displayAltitude,
                           false,   // displayAltitudeStdDev,
                           false,   // displayFixType,
                           display);
}

//*********************************************************************
// Compute point and display point
void gnssMenuComputePoint(const R4A_MENU_ENTRY * menuEntry,
                          const char * command,
                          Print * display)
{
    zedf9p.computePoint(gnssDisplayPoint,
                        0,
                        GNSS_POINTS_TO_AVERAGE,
                        "Location",
                        display);
}

//*********************************************************************
// Display the location
void gnssMenuDisplayLocation(const R4A_MENU_ENTRY * menuEntry,
                             const char * command,
                             Print * display)
{
    zedf9p.displayLocation("Location: ",
                           true,    // unitsFeetInches,
                           false,   // displayTime,
                           true,    // displaySiv,
                           true,    // displayLatitude
                           true,    // displayLongitude
                           true,    // displayHorizAcc,
                           false,   // displayAltitude,
                           false,   // displayFixType,
                           display);
}
#endif  // USE_ZED_F9P

//*********************************************************************
// Display loop times
void loopTimesMenu(const R4A_MENU_ENTRY * menuEntry,
                   const char * command,
                   Print * display)
{
    // Display the header
    display->printf("  Average      Maximum      Minimum       StdDev     Loops\r\n");
    display->printf("-----------  -----------  -----------  -----------  ------\r\n");
    r4aTimeDisplayLoopTimes(display, loopCore0TimeUsec, loopsCore0, "Core 0 in loopCore0");
    r4aTimeDisplayLoopTimes(display, loopCore0OutTimeUsec, loopsCore0, "Core 0 outside loopCore0");
    r4aTimeDisplayLoopTimes(display, loopCore1TimeUsec, loopsCore1, "Core 1 in loop");
    r4aTimeDisplayLoopTimes(display, loopCore1OutTimeUsec, loopsCore1, "Core 1 outside loop");
}

//*********************************************************************
// Display data before the main menu header
void mainMenuPre(Print * display)
{
    if (r4aWifiSsidPasswordEntries)
    {
        // Display the WIFI status
        const char * hostName = r4aWifiHostName;
        if (hostName)
            display->printf("%s (%s): %s channel %d\r\n",
                            hostName,
                            WiFi.localIP().toString().c_str(),
                            r4aWifiStationSsid(),
                            r4aWifiChannel);
        else
            display->printf("%s: %s channel %d\r\n",
                            WiFi.localIP().toString().c_str(),
                            r4aWifiStationSsid(),
                            r4aWifiChannel);

        // Display the current time
        r4aNtpDisplayDateTime(display);
    }
    DISPLAY_BATTERY_VOLTAGE(display);
}

//*********************************************************************
// Stop the robot
// Inputs:
//   menuEntry: Address of the object describing the menu entry
//   command: Zero terminated command string
//   display: Device used for output
void robotMenuStop(const R4A_MENU_ENTRY * menuEntry,
                   const char * command,
                   Print * display)
{
    r4aRobotStop(&robot, millis(), display);
}

//*********************************************************************
// Start the line following at boot
void startNone(const struct _R4A_MENU_ENTRY * menuEntry,
               const char * command,
               Print * display)
{
    startIndex = CHALLENGE_NONE;
    r4aEsp32NvmMenuParameterFileWrite(menuEntry, command, display);
}

//*********************************************************************
// Restart the WiFi station
// Inputs:
//   menuEntry: Address of the object describing the menu entry
//   command: Zero terminated command string
//   display: Device used for output
void wifiMenuRestart(const R4A_MENU_ENTRY * menuEntry,
                     const char * command,
                     Print * display)
{
    r4aWifiRestartRequested = true;
}

//*********************************************************************
// Display data before the waypoint menu header
void wpMenuPre(Print * display)
{
#ifdef  USE_ZED_F9P
    zedf9p.displayLocation(nullptr, // comment
                           zedf9p._latitude,
                           0.,      // latitudeStdDev
                           zedf9p._longitude,
                           0.,      // longitudeStdDev
                           zedf9p._altitude,
                           0.,      // altitudeStdDev
                           zedf9p._horizontalAccuracy,
                           0.,      // horizontalAccuracyStdDev
                           zedf9p._satellitesInView,
                           true,    // unitsFeetInches
                           false,   // displayTime
                           true,    // displaySiv
                           true,    // displayLatitude
                           false,   // displayLatStdDev
                           true,    // displayLongitude
                           false,   // displayLongStdDev
                           true,    // displayHorizAcc
                           false,   // displayHorizAccStdDev
                           false,   // displayAltitude
                           false,   // displayAltitudeStdDev
                           false,   // displayFixType
                           display);
#endif  // USE_ZED_F9P
}

//*********************************************************************

enum MENU_TABLE_INDEX
{
    MTI_DEBUG = R4A_MENU_MAIN + 1,
#ifdef  USE_ZED_F9P
    MTI_GNSS,
#endif  // USE_ZED_F9P
    MTI_GPIO,
    MTI_I2C,
    MTI_LED_MATRIX,
    MTI_MOTOR,
#ifdef  USE_NTRIP
    MTI_NTRIP,
#endif  // USE_NTRIP
    MTI_NVM,
    MTI_ROBOT,
#ifdef  USE_SPARKFUN_SEN_13582
    MTI_SEN13582,
#endif  // USE_SPARKFUN_SEN_13582
    MTI_SERVO,
    MTI_START,
    MTI_TELNET,
#ifdef  USE_ZED_F9P
    MTI_WAY_POINT,
#endif  // USE_ZED_F9P
    MTI_WS2812_LED,

//#define USE_WAYPOINT_FOLLOWING
//#define USE_ZED_F9P
    // Add new menu values before this line
    MTI_MAX
};
const int menuTableIndexMax = MTI_MAX;

// Debug menu
const R4A_MENU_ENTRY debugMenuTable[] =
{
    // Command  menuRoutine                 menuParam       HelpRoutine align   HelpText
    {"g",       nullptr,                    MTI_GPIO,       nullptr,    0,      "Enter the GPIO menu"},
    {"h",       r4aEsp32MenuDisplayHeap,    0,              nullptr,    0,      "Display the heap"},
    {"i",       nullptr,                    MTI_I2C,        nullptr,    0,      "I2C menu"},
    {"l",       nullptr,                    MTI_LED_MATRIX, nullptr,    0,      "LED matrix menu"},
    {"m",       nullptr,                    MTI_MOTOR,      nullptr,    0,      "Motor menu"},
    {"p",    r4aEsp32MenuDisplayPartitions, 0,              nullptr,    0,      "Display the partitions"},
    {"s",       nullptr,                    MTI_SERVO,      nullptr,    0,      "Servo menu"},
    {"w",       nullptr,                    MTI_WS2812_LED, nullptr,    0,      "WS2812 RGB LED menu"},
    {"x",       nullptr,                    R4A_MENU_MAIN,  nullptr,    0,      "Return to the main menu"},
};
#define DEBUG_MENU_ENTRIES      sizeof(debugMenuTable) / sizeof(debugMenuTable[0])

#ifdef  USE_ZED_F9P
// GNSS menu
const R4A_MENU_ENTRY gnssMenuTable[] =
{
    // Command  menuRoutine             menuParam       HelpRoutine     align   HelpText
#ifdef  USE_WAYPOINT_FOLLOWING
    {"h",       menuWpfDisplayHeading,  0,              nullptr,        0,      "Display heading"},
#endif  // USE_WAYPOINT_FOLLOWING
    {"l",      gnssMenuDisplayLocation, 0,              nullptr,        0,      "Display location"},
    {"p",       gnssMenuComputePoint,   0,              nullptr,        0,      "Compute point and display point"},
    {"x",       nullptr,                R4A_MENU_MAIN,  nullptr,        0,      "Return to the main menu"},
};
#define GNSS_MENU_ENTRIES       sizeof(gnssMenuTable) / sizeof(gnssMenuTable[0])
#endif  // USE_ZED_F9P

// GPIO menu
const R4A_MENU_ENTRY gpioMenuTable[] =
{
    // Command  menuRoutine                 menuParam       HelpRoutine         align   HelpText
    {"g",  r4aEsp32GpioMenuDisplayAllPorts, 0,              nullptr,            2,      "Display all GPIO port registers"},
    {"p",  r4aEsp32GpioMenuDisplayPort,     (intptr_t)"pp", r4aMenuHelpSuffix,  2,      "Display GPIO port pp configuration"},
    {"x",       nullptr,                    R4A_MENU_MAIN,  nullptr,            0,      "Return to the main menu"},
};
#define GPIO_MENU_ENTRIES      sizeof(gpioMenuTable) / sizeof(gpioMenuTable[0])

// Robot menu
const R4A_MENU_ENTRY robotMenuTable[] =
{
    // Command  menuRoutine         menuParam                   HelpRoutine    align    HelpText
    {"w",       r4aMenuBoolToggle, (intptr_t)&robotUseWS2812,   r4aMenuBoolHelp, 0,     "Enable/disable WS2812 use"},
    {"x",       nullptr,            R4A_MENU_MAIN,              nullptr,         0,     "Return to the main menu"},
};
#define ROBOT_MENU_ENTRIES      sizeof(robotMenuTable) / sizeof(robotMenuTable[0])

// Define the table index values
#define SERVO_PAN       0
#define SERVO_TILT      1

// Servo menu
const R4A_MENU_ENTRY servoMenuTable[] =
{
    // Command  menuRoutine                 menuParam       HelpRoutine                     align   HelpText
    {"d",       r4aPca9685ServoMenuDisplay, 0,              nullptr,                        0,      "Display the servo states"},    // 1
    {"p",       r4aPca9685ServoMenuMove,    SERVO_PAN,      r4aPca9685ServoMenuHelpDdd,     4,      "Pan to ddd degrees"},          // 2
    {"s",       r4aPca9685ServoMenuSet,     0,              r4aPca9685ServoMenuHelpSDdd,    6,      "Set servo s to ddd degrees"},  // 3
    {"t",       r4aPca9685ServoMenuMove,    SERVO_TILT,     r4aPca9685ServoMenuHelpDdd,     4,      "Tilt to ddd degrees"},         // 4
    {"x",       nullptr,                    R4A_MENU_MAIN,  nullptr,                        0,      "Return to the main menu"},     // 5
};                                                                                                                                  // 6
#define SERVO_MENU_ENTRIES      sizeof(servoMenuTable) / sizeof(servoMenuTable[0])

// Start menu
const R4A_MENU_ENTRY startMenuTable[] =
{
    // Command  menuRoutine         menuParam               HelpRoutine         align   HelpText
    {"alf",     menuStartAlf,       0,                      nullptr,            0,      "Start advanced line following at boot"},
    {"blf",     menuStartBlt,       0,                      nullptr,            0,      "Start basic line following at boot"},
    {"blt",     menuStartBlf,       0,                      nullptr,            0,      "Start basic light tracking at boot"},
#ifdef  USE_OV2640
    {"clf",     menuStartClf,       0,                      nullptr,            0,      "Start camera line following at boot"},
#endif  // USE_OV2640
    {"None",    startNone,          0,                      nullptr,            0,      "Don't start anything at boot"},
#ifdef  USE_ZED_F9P
#ifdef  USE_WAYPOINT_FOLLOWING
    {"wpf",     menuStartWpf,       0,                      nullptr,            0,      "Start waypoint following at boot"},
#endif  // USE_WAYPOINT_FOLLOWING
#endif  // USE_ZED_F9P
    {"x",       nullptr,            R4A_MENU_MAIN,          nullptr,            0,      "Return to the main menu"},
};
#define START_MENU_ENTRIES      sizeof(startMenuTable) / sizeof(startMenuTable[0])

#ifdef  USE_SPARKFUN_SEN_13582
const R4A_MENU_ENTRY sfeSen13582MenuTable[] =
{
    // Command  menuRoutine                          menuParam   HelpRoutine align   HelpText
    {"d",       r4aSfeSen13582MenuDisplayRegisters,  0,          nullptr,    0,      "Display the SX1509 registers"},
    {"f",       r4aSfeSen13582MenuFeedbackLedToggle, 0,          nullptr,    0,      "Toggle the feedback LEDs"},
    {"i",       r4aSfeSen13582MenuIrLedToggle,       0,          nullptr,    0,      "Toggle the IR LEDs"},
    {"x",       nullptr,                          R4A_MENU_MAIN, nullptr,    0,      "Return to the main menu"},
};
#define SFE_SEN13582_MENU_ENTRIES   sizeof(sfeSen13582MenuTable) / sizeof(sfeSen13582MenuTable[0])
#endif  //USE_SPARKFUN_SEN_13582

// Telnet menu
const R4A_MENU_ENTRY telnetMenuTable[] =
{
    // Command  menuRoutine     menuParam       HelpRoutine align   HelpText
    {"c",       listClients,    0,              nullptr,    0,      "Client list"},
    {"s",       serverInfo,     0,              nullptr,    0,      "Server information"},
    {"x",       nullptr,        R4A_MENU_MAIN,  nullptr,    0,      "Return to the main menu"},
};
#define TELNET_MENU_ENTRIES  sizeof(telnetMenuTable) / sizeof(telnetMenuTable[0])

// Way Point menu
#ifdef  USE_ZED_F9P
const R4A_MENU_ENTRY wayPointMenuTable[] =
{
    // Command  menuRoutine                 menuParam               HelpRoutine         align   HelpText
    {"a",       r4aEsp32WpMenuAddPoint,     (intptr_t)"comment",    r4aMenuHelpSuffix,  7,      "Add a point to the file"},
    {"cat",     r4aEsp32NvmMenuFileCat,     (intptr_t)"ffff",       r4aMenuHelpSuffix,  4,      "Display the contents of file ffff"},
    {"cp",      r4aEsp32NvmMenuFileCopy,    (intptr_t)"src dest",   r4aMenuHelpSuffix,  8,      "Copy src file to dest file"},
    {"dp",      r4aEsp32WpMenuDisplayPoint, 0,                      nullptr,            0,      "Display the next waypoint"},
    {"dump",    r4aEsp32NvmMenuFileDump,    (intptr_t)"ffff",       r4aMenuHelpSuffix,  4,      "Dump the contents of file ffff"},
    {"f",       r4aEsp32WpMenuFileName,     (intptr_t)"nnnn",       r4aMenuHelpSuffix,  4,      "Set waypoint file name"},
    {"ls",      r4aEsp32NvmMenuFileList,    0,                      nullptr,            0,      "List the NVM directory"},
    {"mv",      r4aEsp32NvmMenuFileMove,    (intptr_t)"src dest",   r4aMenuHelpSuffix,  8,      "Rename a file"},
    {"p",       r4aEsp32WpMenuPrintFile,    0,                      nullptr,            0,      "Print the waypoint file contents"},
    {"rm",      r4aEsp32NvmMenuFileRemove,  (intptr_t)"ffff",       r4aMenuHelpSuffix,  4,      "Remove file ffff"},
    {"wget",    r4aEsp32NvmMenuHttpFileGet, (intptr_t)"url",        r4aMenuHelpSuffix,  3,      "Get a file from a web server"},
    {"x",       nullptr,                    R4A_MENU_MAIN,          nullptr,            0,      "Exit the menu system"},
};
#define WAYPOINT_MENU_ENTRIES  sizeof(wayPointMenuTable) / sizeof(wayPointMenuTable[0])
#endif  // USE_ZED_F9P

// Main menu
const R4A_MENU_ENTRY mainMenuTable[] =
{
    // Command  menuRoutine         menuParam       HelpRoutine align   HelpText
    {"alf",     alfStartMenu,       0,              nullptr,    0,      "Advanced line following"},
    {"blf",     blfStartMenu,       0,              nullptr,    0,      "Basic line following"},
    {"blt",     bltStartMenu,       0,              nullptr,    0,      "Basic light tracking"},
#ifdef  USE_OV2640
    {"c", r4aMenuBoolToggle, (intptr_t)&ov2640Enable, r4aMenuBoolHelp, 0, "Toggle OV2640 camera"},
    {"clf",     clfStartMenu,       0,              nullptr,    0,      "Camera line following"},
#endif  // USE_OV2640
    {"d",       nullptr,            MTI_DEBUG,      nullptr,    0,      "Enter the debug menu"},
#ifdef  USE_ZED_F9P
    {"g",       nullptr,            MTI_GNSS,       nullptr,    0,      "Enter the GNSS menu"},
#endif  // USE_ZED_F9P
    {"i",  r4aMenuBoolToggle, (intptr_t)&ignoreBatteryCheck, r4aMenuBoolHelp, 0, "Ignore the battery check"},
    {"l",       loopTimesMenu,      0,              nullptr,    0,      "Loop times"},
#ifdef  USE_NTRIP
    {"NTRIP",   nullptr,            MTI_NTRIP,      nullptr,    0,      "Enter the NTRIP menu"},
#endif  // USE_NTRIP
    {"nvm",     nullptr,            MTI_NVM,        nullptr,    0,      "Enter the NVM menu"},
    {"r",  r4aEsp32MenuSystemReset, 0,              nullptr,    0,      "System reset"},
    {"robot",   nullptr,            MTI_ROBOT,      nullptr,    0,      "Enter the robot menu"},
    {"s",       robotMenuStop,      0,              nullptr,    0,      "Stop the robot"},
#ifdef  USE_SPARKFUN_SEN_13582
    {"sen",     nullptr,            MTI_SEN13582,   nullptr,    0,      "Enter the SparkFun SEN-13582 menu"},
#endif  // USE_SPARKFUN_SEN_13582
    {"Start",   nullptr,            MTI_START,      nullptr,    0,      "Start challenge at boot menu"},
    {"t",       nullptr,            MTI_TELNET,     nullptr,    0,      "Enter the telnet menu"},
    {"w", r4aMenuBoolToggle, (intptr_t)&webServerEnable, r4aMenuBoolHelp, 0, "Toggle web server"},
    {"wd", r4aMenuBoolToggle, (intptr_t)&r4aWifiDebug, r4aMenuBoolHelp, 0, "Toggle WiFi debugging"},
#ifdef  USE_ZED_F9P
    {"wp",      nullptr,            MTI_WAY_POINT,  nullptr,    0,      "Enter the waypoint menu"},
#ifdef  USE_WAYPOINT_FOLLOWING
    {"wpf",     wpfStartMenu,       0,              nullptr,    0,      "Waypoint following"},
#endif  // USE_WAYPOINT_FOLLOWING
#endif  // USE_ZED_F9P
    {"wr",      wifiMenuRestart,    0,              nullptr,    0,      "Restart WiFi"},
    {"wv", r4aMenuBoolToggle, (intptr_t)&r4aWifiVerbose, r4aMenuBoolHelp, 0, "Toggle WiFi verbose output"},
    {"x",       nullptr,            R4A_MENU_NONE,  nullptr,    0,      "Exit the menu system"},
};
#define MAIN_MENU_ENTRIES       sizeof(mainMenuTable) / sizeof(mainMenuTable[0])

// List of menus
const R4A_MENU_TABLE menuTable[] =
{
    // menuName         preMenu routine firstEntry          entryCount
    {"Main Menu",       mainMenuPre,    mainMenuTable,      MAIN_MENU_ENTRIES},
    {"Debug Menu",      nullptr,        debugMenuTable,     DEBUG_MENU_ENTRIES},
#ifdef  USE_ZED_F9P
    {"GNSS Menu",       nullptr,        gnssMenuTable,      GNSS_MENU_ENTRIES},
#endif  // USE_ZED_F9P
    {"GPIO Menu",       nullptr,        gpioMenuTable,      GPIO_MENU_ENTRIES},
    {"I2C Menu",        nullptr,        r4aI2cMenuTable,    R4A_I2C_MENU_ENTRIES},
    {"LED Matrix Menu", nullptr,       r4aVk16k33MenuTable, R4A_VK16K33_MENU_ENTRIES},
    {"Motor Menu",      nullptr,  r4aPca9685MotorMenuTable, R4A_PCA9685_MOTOR_MENU_ENTRIES},
#ifdef  USE_NTRIP
    {"NTRIP Menu",      nullptr,  r4aNtripClientMenuTable,  R4A_NTRIP_CLIENT_MENU_ENTRIES},
#endif  // USE_NTRIP
    {"NVM Menu",        nullptr,      r4aEsp32NvmMenuTable, R4A_ESP32_NVM_MENU_ENTRIES},
    {"Robot Menu",      nullptr,      robotMenuTable,       ROBOT_MENU_ENTRIES},
#ifdef  USE_SPARKFUN_SEN_13582
    {"SEN-13582 Menu",  nullptr,      sfeSen13582MenuTable, SFE_SEN13582_MENU_ENTRIES},
#endif  // USE_SPARKFUN_SEN_13582
    {"Servo Menu",      nullptr,        servoMenuTable,     SERVO_MENU_ENTRIES},
    {"Start Menu",      nullptr,        startMenuTable,     START_MENU_ENTRIES},
    {"Telnet Menu",     nullptr,        telnetMenuTable,    TELNET_MENU_ENTRIES},
#ifdef  USE_ZED_F9P
    {"Waypoint Menu",   wpMenuPre,      wayPointMenuTable,  WAYPOINT_MENU_ENTRIES},
#endif  // USE_ZED_F9P
    {"WS2812 LED Menu", nullptr,     r4a4wdCarLedMenuTable, R4A_4WD_CAR_LED_MENU_ENTRIES},
};
const int menuTableEntries = sizeof(menuTable) / sizeof(menuTable[0]);
