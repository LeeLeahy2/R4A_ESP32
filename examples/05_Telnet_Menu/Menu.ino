/**********************************************************************
  Menu.ino

  Menu routines
**********************************************************************/

//*********************************************************************
// Display loop times
void loopTimesMenu(const R4A_MENU_ENTRY * menuEntry,
                   const char * command,
                   Print * display)
{
    // Display the header
    display->printf("  Average      Maximum      Minimum       StdDev     Loops\r\n");
    display->printf("-----------  -----------  -----------  -----------  ------\r\n");
    r4aTimeDisplayLoopTimes(display, loopCore1TimeUsec, loopsCore1, "Core 1 in loop");
    r4aTimeDisplayLoopTimes(display, loopCore1OutTimeUsec, loopsCore1, "Core 1 outside loop");
}

//*********************************************************************
// Display data before the main menu header
bool mainMenuPre(Print * display)
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
    return true;
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
// Toggle WiFi boolean
// Inputs:
//   menuEntry: Address of the object describing the menu entry
//   command: Zero terminated command string
//   display: Device used for output
void wifiToggleBool(const R4A_MENU_ENTRY * menuEntry,
                    const char * command,
                    Print * display)
{
    r4aMenuBoolToggle(menuEntry, command, display);
    r4aEsp32NvmMenuParameterFileWrite(menuEntry, command, display);
}

//*********************************************************************

enum MENU_TABLE_INDEX
{
    MTI_DEBUG = R4A_MENU_MAIN + 1,
    MTI_I2C,
    MTI_LED_MATRIX,
    MTI_MOTOR,
    MTI_NVM,
    MTI_SERVO,
    MTI_TELNET,
    MTI_WS2812_LED,
};

// Debug menu
const R4A_MENU_ENTRY debugMenuTable[] =
{
    // Command  menuRoutine                 menuParam       HelpRoutine align   HelpText
    {"h",       r4aEsp32MenuDisplayHeap,    0,              nullptr,    0,      "Display the heap"},
    {"i",       nullptr,                    MTI_I2C,        nullptr,    0,      "I2C menu"},
    {"l",       nullptr,                    MTI_LED_MATRIX, nullptr,    0,      "LED Matrix menu"},
    {"m",       nullptr,                    MTI_MOTOR,      nullptr,    0,      "Motor menu"},
    {"p",    r4aEsp32MenuDisplayPartitions, 0,              nullptr,    0,      "Display the partitions"},
    {"s",       nullptr,                    MTI_SERVO,      nullptr,    0,      "Servo menu"},
    {"w",       nullptr,                    MTI_WS2812_LED, nullptr,    0,      "WS2812 LED menu"},
    {"x",       nullptr,                    R4A_MENU_MAIN,  nullptr,    0,      "Return to the main menu"},
};
#define DEBUG_MENU_ENTRIES      sizeof(debugMenuTable) / sizeof(debugMenuTable[0])

// Telnet menu
const R4A_MENU_ENTRY telnetMenuTable[] =
{
    // Command  menuRoutine     menuParam       HelpRoutine align   HelpText
    {"c",       listClients,    0,              nullptr,    0,      "Client list"},
    {"s",       serverInfo,     0,              nullptr,    0,      "Server information"},
    {"x",       nullptr,        R4A_MENU_MAIN,  nullptr,    0,      "Return to the main menu"},
};
#define TELNET_MENU_ENTRIES  sizeof(telnetMenuTable) / sizeof(telnetMenuTable[0])

// Main menu
const R4A_MENU_ENTRY mainMenuTable[] =
{
    // Command  menuRoutine         menuParam       HelpRoutine align   HelpText
    {"blf",     menuBlfStart,       0,              nullptr,    0,      "Basic line following"},
    {"d",       nullptr,            MTI_DEBUG,      nullptr,    0,      "Enter the debug menu"},
    {"i",  r4aMenuBoolToggle, (intptr_t)&ignoreBatteryCheck, r4aMenuBoolHelp, 0, "Ignore the battery check,"},
    {"l",       loopTimesMenu,      0,              nullptr,    0,      "Loop times"},
    {"nvm",     nullptr,            MTI_NVM,        nullptr,    0,      "Enter the NVM menu"},
    {"r",  r4aEsp32MenuSystemReset, 0,              nullptr,    0,      "System reset"},
    {"s",       robotMenuStop,      0,              nullptr,    0,      "Stop the robot"},
    {"t",       nullptr,            MTI_TELNET,     nullptr,    0,      "Enter the telnet menu"},
    {"ulf",     menuUlfStart,       0,              nullptr,    0,      "User line following"},
    {"wd",    wifiToggleBool, (intptr_t)&r4aWifiDebug, r4aMenuBoolHelp, 0, "Toggle WiFi debugging,"},
    {"wr",      wifiMenuRestart,    0,              nullptr,    0,      "Restart WiFi"},
    {"wv",  wifiToggleBool, (intptr_t)&r4aWifiVerbose, r4aMenuBoolHelp, 0, "Toggle WiFi verbose output,"},
    {"x",       nullptr,            R4A_MENU_NONE,  nullptr,    0,      "Exit the menu system"},
};
#define MAIN_MENU_ENTRIES       sizeof(mainMenuTable) / sizeof(mainMenuTable[0])

const R4A_MENU_TABLE menuTable[] =
{
    // menuName         preMenu routine firstEntry          entryCount
    {"Main Menu",       mainMenuPre,    mainMenuTable,      MAIN_MENU_ENTRIES},
    {"Debug Menu",      nullptr,        debugMenuTable,     DEBUG_MENU_ENTRIES},
    {"I2C Menu",        nullptr,        r4aI2cMenuTable,    R4A_I2C_MENU_ENTRIES},
    {"LED Matrix Menu", nullptr,       r4aVk16k33MenuTable, R4A_VK16K33_MENU_ENTRIES},
    {"Motor Menu",      nullptr,  r4aPca9685MotorMenuTable, R4A_PCA9685_MOTOR_MENU_ENTRIES},
    {"NVM Menu",        nullptr,      r4aEsp32NvmMenuTable, R4A_ESP32_NVM_MENU_ENTRIES},
    {"Servo Menu",      nullptr,  r4aPca9685ServoMenuTable, R4A_PCA9685_SERVO_MENU_ENTRIES},
    {"Telnet Menu",     nullptr,        telnetMenuTable,    TELNET_MENU_ENTRIES},
    {"WS2812 LED Menu", nullptr,     r4a4wdCarLedMenuTable, R4A_4WD_CAR_LED_MENU_ENTRIES},
};
const int menuTableEntries = sizeof(menuTable) / sizeof(menuTable[0]);
