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
    r4aTimeDisplayLoopTimes(display, loopCore0TimeUsec, loopsCore0, "Core 0 in loopCore0");
    r4aTimeDisplayLoopTimes(display, loopCore0OutTimeUsec, loopsCore0, "Core 0 outside loopCore0");
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

enum MENU_TABLE_INDEX
{
    MTI_DEBUG = R4A_MENU_MAIN + 1,
    MTI_FLASH,
    MTI_GPIO,
    MTI_I2C,
    MTI_MOTOR,
    MTI_NVM,
    MTI_SERVO,
    MTI_START,
    MTI_TELNET,
    // Add new menu values before this line
    MTI_MAX
};
const int menuTableIndexMax = MTI_MAX;

// Debug menu
const R4A_MENU_ENTRY debugMenuTable[] =
{
    // Command  menuRoutine                 menuParam       HelpRoutine align   HelpText
    {"f",       nullptr,                    MTI_FLASH,      nullptr,    0,      "Enter the flash menu"},
    {"g",       nullptr,                    MTI_GPIO,       nullptr,    0,      "Enter the GPIO menu"},
    {"h",       r4aEsp32MenuDisplayHeap,    0,              nullptr,    0,      "Display the heap"},
#ifdef  USE_I2C
    {"i",       nullptr,                    MTI_I2C,        nullptr,    0,      "I2C menu"},
    {"m",       nullptr,                    MTI_MOTOR,      nullptr,    0,      "Motor menu"},
#endif  // USE_I2C
    {"p",    r4aEsp32MenuDisplayPartitions, 0,              nullptr,    0,      "Display the partitions"},
#ifdef  USE_I2C
    {"s",       nullptr,                    MTI_SERVO,      nullptr,    0,      "Servo menu"},
#endif  // USE_I2C
    {"x",       nullptr,                    R4A_MENU_MAIN,  nullptr,    0,      "Return to the main menu"},
};
#define DEBUG_MENU_ENTRIES      sizeof(debugMenuTable) / sizeof(debugMenuTable[0])

// Flash menu
const R4A_MENU_ENTRY flashMenuTable[] =
{
    // Command  menuRoutine                 menuParam               HelpRoutine         align   HelpText
    {"abpre", r4aSpiFlashMenuBlockProtectionReadAll,  1,            nullptr,            0,      "All blocks read enabled"},
    {"abprp", r4aSpiFlashMenuBlockProtectionReadAll,  0,            nullptr,            0,      "All blocks read protected"},
    {"abpwe", r4aSpiFlashMenuBlockProtectionWriteAll, 1,            nullptr,            0,      "All blocks write enabled"},
    {"abpwp", r4aSpiFlashMenuBlockProtectionWriteAll, 0,            nullptr,            0,      "All blocks write protected"},
    {"bpd", spiFlashMenuBlockProtectionDump, 0,                     nullptr,            0,      "Dump the block protection table"},
    {"bpr", r4aSpiFlashMenuBlockProtectionRead,  (intptr_t)"addr en", r4aMenuHelpSuffix, 7,     "Enable (1) or disable (0) reads from block containing addr"},
    {"bps", r4aSpiFlashMenuBlockProtectionStatus, 0,                nullptr,            0,      "Read block protections"},
    {"bpw", r4aSpiFlashMenuBlockProtectionWrite, (intptr_t)"addr en", r4aMenuHelpSuffix, 7,     "Enable (1) or disable (0) writes to block containing addr"},
    {"ce",      r4aSpiFlashMenuEraseChip,   0,                      nullptr,            0,      "Erase the chip"},
    {"dp",   r4aSpiFlashMenuReadParameters, (intptr_t)"addr len",   r4aMenuHelpSuffix,  8,      "Read parameters len bytes from addr"},
    {"e4k",     r4aSpiFlashMenuErase4K,     (intptr_t)"addr",       r4aMenuHelpSuffix,  4,      "Erase a 4K block"},
    {"e65k",    r4aSpiFlashMenuErase65K,    (intptr_t)"addr",       r4aMenuHelpSuffix,  4,      "Erase a 65K sector"},
    {"id9e",    r4aSpiFlashMenuReadId9e,    0,                      nullptr,            0,      "Read 20 byte ID"},
    {"id9f",    r4aSpiFlashMenuReadId9f,    0,                      nullptr,            0,      "Read 3 byte ID"},
    {"rd",      r4aSpiFlashMenuReadData,    (intptr_t)"addr len",   r4aMenuHelpSuffix,  8,      "Read len bytes from addr"},
    {"rs", r4aSpiFlashMenuReadStatusRegister, 0,                    nullptr,            0,      "Read status register"},
    {"we",   r4aSpiFlashMenuWriteEnable,    1,                      nullptr,            0,      "Enable writes to SPI flash"},
    {"wp",   r4aSpiFlashMenuWriteEnable,    0,                      nullptr,            0,      "Write protect SPI flash"},
    {"x",       nullptr,                    R4A_MENU_MAIN,          nullptr,            0,      "Return to the main menu"},
};
#define FLASH_MENU_ENTRIES      sizeof(flashMenuTable) / sizeof(flashMenuTable[0])

// GPIO menu
const R4A_MENU_ENTRY gpioMenuTable[] =
{
    // Command  menuRoutine                 menuParam       HelpRoutine         align   HelpText
    {"g",  r4aEsp32GpioMenuDisplayAllPorts, 0,              nullptr,            2,      "Display all GPIO port registers"},
    {"p",  r4aEsp32GpioMenuDisplayPort,     (intptr_t)"pp", r4aMenuHelpSuffix,  2,      "Display GPIO port pp configuration"},
    {"x",       nullptr,                    R4A_MENU_MAIN,  nullptr,            0,      "Return to the main menu"},
};
#define GPIO_MENU_ENTRIES      sizeof(gpioMenuTable) / sizeof(gpioMenuTable[0])

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
    {"blf",     menuStartBlf,       0,                      nullptr,            0,      "Start basic line following at boot"},
#ifdef  USE_OV2640
    {"clf",     menuStartClf,       0,                      nullptr,            0,      "Start camera line following at boot"},
#endif  // USE_OV2640
    {"None",    startNone,          0,                      nullptr,            0,      "Don't start anything at boot"},
    {"x",       nullptr,            R4A_MENU_MAIN,          nullptr,            0,      "Return to the main menu"},
};
#define START_MENU_ENTRIES      sizeof(startMenuTable) / sizeof(startMenuTable[0])

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
#ifdef  USE_I2C
    {"alf",     alfStartMenu,       0,              nullptr,    0,      "Advanced line following"},
    {"blf",     blfStartMenu,       0,              nullptr,    0,      "Basic line following"},
#endif  // USE_I2C
#ifdef  USE_OV2640
    {"c", r4aMenuBoolToggle, (intptr_t)&ov2640Enable, r4aMenuBoolHelp, 0, "Toggle OV2640 camera"},
#ifdef  USE_I2C
    {"clf",     clfStartMenu,       0,              nullptr,    0,      "Camera line following"},
#endif  // USE_I2C
#endif  // USE_OV2640
    {"d",       nullptr,            MTI_DEBUG,      nullptr,    0,      "Enter the debug menu"},
    {"i",  r4aMenuBoolToggle, (intptr_t)&ignoreBatteryCheck, r4aMenuBoolHelp, 0, "Ignore the battery check"},
    {"l",       loopTimesMenu,      0,              nullptr,    0,      "Loop times"},
    {"nvm",     nullptr,            MTI_NVM,        nullptr,    0,      "Enter the NVM menu"},
    {"r",  r4aEsp32MenuSystemReset, 0,              nullptr,    0,      "System reset"},
#ifdef  USE_I2C
    {"s",       robotMenuStop,      0,              nullptr,    0,      "Stop the robot"},
    {"Start",   nullptr,            MTI_START,      nullptr,    0,      "Start challenge at boot menu"},
#endif  // USE_I2C
    {"t",       nullptr,            MTI_TELNET,     nullptr,    0,      "Enter the telnet menu"},
    {"w", r4aMenuBoolToggle, (intptr_t)&webServerEnable, r4aMenuBoolHelp, 0, "Toggle web server"},
    {"wd", r4aMenuBoolToggle, (intptr_t)&r4aWifiDebug, r4aMenuBoolHelp, 0, "Toggle WiFi debugging"},
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
    {"Flash Menu",      nullptr,        flashMenuTable,     FLASH_MENU_ENTRIES},
    {"GPIO Menu",       nullptr,        gpioMenuTable,      GPIO_MENU_ENTRIES},
    {"I2C Menu",        nullptr,        r4aI2cMenuTable,    R4A_I2C_MENU_ENTRIES},
    {"Motor Menu",      nullptr,  r4aPca9685MotorMenuTable, R4A_PCA9685_MOTOR_MENU_ENTRIES},
    {"NVM Menu",        nullptr,      r4aEsp32NvmMenuTable, R4A_ESP32_NVM_MENU_ENTRIES},
    {"Servo Menu",      nullptr,        servoMenuTable,     SERVO_MENU_ENTRIES},
    {"Start Menu",      nullptr,        startMenuTable,     START_MENU_ENTRIES},
    {"Telnet Menu",     nullptr,        telnetMenuTable,    TELNET_MENU_ENTRIES},
};
const int menuTableEntries = sizeof(menuTable) / sizeof(menuTable[0]);
