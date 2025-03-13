/**********************************************************************
  Menu.ino

  Menu routines
**********************************************************************/

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
                            r4aWifiStationIpAddress().toString().c_str(),
                            r4aWifiStationSsid(),
                            r4aWifiChannel);
        else
            display->printf("%s: %s channel %d\r\n",
                            r4aWifiStationIpAddress().toString().c_str(),
                            r4aWifiStationSsid(),
                            r4aWifiChannel);

        // Display the current time
        r4aNtpDisplayDateTime(display);
    }
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
    r4aWifiStationOff(__FILE__, __LINE__);
    delay(5 * 1000);
    r4aWifiStationOn(__FILE__, __LINE__);
}

//*********************************************************************

enum MENU_TABLE_INDEX
{
    MTI_DEBUG = R4A_MENU_MAIN + 1,
    MTI_NVM,
    MTI_TELNET,
};

// Debug menu
const R4A_MENU_ENTRY debugMenuTable[] =
{
    // Command  menuRoutine                 menuParam       HelpRoutine align   HelpText
    {"h",       r4aEsp32MenuDisplayHeap,    0,              nullptr,    0,      "Display the heap"},
    {"p",    r4aEsp32MenuDisplayPartitions, 0,              nullptr,    0,      "Display the partitions"},
    {"x",       nullptr,                    R4A_MENU_MAIN,  nullptr,    0,      "Return to the main menu"},
};
#define DEBUG_MENU_ENTRIES      sizeof(debugMenuTable) / sizeof(debugMenuTable[0])

// Main menu
const R4A_MENU_ENTRY mainMenuTable[] =
{
    // Command  menuRoutine         menuParam                   HelpRoutine     align   HelpText
    {"d",       nullptr,            MTI_DEBUG,                  nullptr,        0,      "Enter the debug menu"},
    {"nvm",     nullptr,            MTI_NVM,                    nullptr,        0,      "Enter the NVM menu"},
    {"r",  r4aEsp32MenuSystemReset, 0,                          nullptr,        0,      "System reset"},
    {"telnet",  nullptr,            MTI_TELNET,                 nullptr,        0,      "Enter the Telnet menu"},
    {"wd",      r4aMenuBoolToggle,  (intptr_t)&r4aWifiDebug,   r4aMenuBoolHelp, 0,      "Toggle WiFi debugging"},
    {"wr",      wifiMenuRestart,    0,                          nullptr,        0,      "Restart WiFi"},
    {"wv",      r4aMenuBoolToggle,  (intptr_t)&r4aWifiVerbose, r4aMenuBoolHelp, 0,      "Toggle WiFi verbose debugging"},
    {"x",       nullptr,            R4A_MENU_NONE,              nullptr,        0,      "Exit the menu system"},
};
#define MAIN_MENU_ENTRIES       sizeof(mainMenuTable) / sizeof(mainMenuTable[0])

const R4A_MENU_ENTRY telnetMenuTable[] =
{
    // Command  menuRoutine     menuParam       HelpRoutine align   HelpText
    {"List",    listClients,    0,              nullptr,    0,      "List the telnet clients"}, // 0
    {"Server",  serverInfo,     0,              nullptr,    0,      "Server info"},             // 1
    {"x",       nullptr,        R4A_MENU_MAIN,  nullptr,    0,      "Return to the main menu"}, // 2
};                                                                                                          // 3
#define TELNET_MENU_ENTRIES       sizeof(telnetMenuTable) / sizeof(telnetMenuTable[0])

const R4A_MENU_TABLE menuTable[] =
{
    // menuName         preMenu routine firstEntry          entryCount
    {"Main Menu",       mainMenuPre,    mainMenuTable,      MAIN_MENU_ENTRIES},
    {"Debug Menu",      nullptr,        debugMenuTable,     DEBUG_MENU_ENTRIES},
    {"NVM Menu",        nullptr,      r4aEsp32NvmMenuTable, R4A_ESP32_NVM_MENU_ENTRIES},
    {"Telnet Menu",     nullptr,        telnetMenuTable,    TELNET_MENU_ENTRIES},
};
const int menuTableEntries = sizeof(menuTable) / sizeof(menuTable[0]);
