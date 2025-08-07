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
// Add a point to the waypoint file
// Inputs:
//   menuEntry: Address of the object describing the menu entry
//   command: Zero terminated command string
//   display: Device used for output
void ws2812MenuLedColors(const R4A_MENU_ENTRY * menuEntry,
                         const char * command,
                         Print * display)
{
    int color[WS2812_MAX];
    uint32_t count;
    static String parameters;

    // Locate the parameters
    parameters = r4aMenuGetParameters(menuEntry, command);

    // Turn off all the LEDs
    memset(color, 0, sizeof(color));

    // Parse the parameters
    count = sscanf(parameters.c_str(), "%lx %lx %lx %lx %lx %lx %lx %lx %lx %lx %lx %lx",
                   &color[0], &color[1], &color[2], &color[3],
                   &color[4], &color[5], &color[6], &color[7],
                   &color[8], &color[9], &color[10], &color[11]);
    if (count < 1)
        display->printf("ERROR: Please specify at least one color in hex (rrggbb)\r\n");
    else
    {
        // Set the WS2812 LED colors
        for (int led = 0; led < WS2812_MAX; led++)
            r4aLEDSetColorRgb(led, color[led]);

        // Update the WS2812 LEDs
        r4aLEDUpdate(true);
    }
}

//*********************************************************************

enum MENU_TABLE_INDEX
{
    MTI_DEBUG = R4A_MENU_MAIN + 1,
    MTI_GPIO,
    MTI_LED,
    MTI_NVM,
    MTI_WS2812,
};

// Debug menu
const R4A_MENU_ENTRY debugMenuTable[] =
{
    // Command  menuRoutine                 menuParam       HelpRoutine align   HelpText
    {"h",       r4aEsp32MenuDisplayHeap,    0,              nullptr,    0,      "Display the heap"},
    {"l",       nullptr,                    MTI_LED,        nullptr,    0,      "Enter the LED menu"},
    {"p",    r4aEsp32MenuDisplayPartitions, 0,              nullptr,    0,      "Display the partitions"},
    {"x",       nullptr,                    R4A_MENU_MAIN,  nullptr,    0,      "Return to the main menu"},
};
#define DEBUG_MENU_ENTRIES      sizeof(debugMenuTable) / sizeof(debugMenuTable[0])

// GPIO menu
const R4A_MENU_ENTRY gpioMenuTable[] =
{
    // Command  menuRoutine                 menuParam       HelpRoutine         align   HelpText
    {"g",  r4aEsp32GpioMenuDisplayAllPorts, 0,              nullptr,            2,      "Display all GPIO port registers"},
    {"p",  r4aEsp32GpioMenuDisplayPort,     (intptr_t)"pp", r4aMenuHelpSuffix,  2,      "Display GPIO port pp configuration"},
    {"x",       nullptr,                    R4A_MENU_MAIN,  nullptr,            0,      "Return to the main menu"},
};
#define GPIO_MENU_ENTRIES      sizeof(gpioMenuTable) / sizeof(gpioMenuTable[0])

const R4A_MENU_ENTRY ws2812MenuTable[] =
{
    // Command  menuRoutine         menuParam           HelpRoutine  align  HelpText
    {"l",       nullptr,            MTI_LED,                nullptr,    0,  "Enter the LED menu"},
    {"w",      ws2812MenuLedColors, (intptr_t)"rrggbb ...", nullptr,    10, "Enter the WS2812 LED colors in hex (rrggbb)"},
    {"x",       nullptr,            R4A_MENU_MAIN,          nullptr,    0,  "Exit the menu system"},
};
#define WS2812_MENU_ENTRIES     sizeof(ws2812MenuTable) / sizeof(ws2812MenuTable[0])

// Main menu
const R4A_MENU_ENTRY mainMenuTable[] =
{
    // Command  menuRoutine         menuParam       HelpRoutine align   HelpText
    {"d",       nullptr,            MTI_DEBUG,      nullptr,    0,      "Enter the debug menu"},
    {"g",       nullptr,            MTI_GPIO,       nullptr,    0,      "Enter the GPIO menu"},
    {"l",       nullptr,            MTI_LED,        nullptr,    0,      "Enter the LED menu"},
    {"nvm",     nullptr,            MTI_NVM,        nullptr,    0,      "Enter the NVM menu"},
    {"r",  r4aEsp32MenuSystemReset, 0,              nullptr,    0,      "System reset"},
    {"w",       nullptr,            MTI_WS2812,     nullptr,    0,      "Enter the WS2812 menu"},
    {"x",       nullptr,            R4A_MENU_NONE,  nullptr,    0,      "Exit the menu system"},
};
#define MAIN_MENU_ENTRIES       sizeof(mainMenuTable) / sizeof(mainMenuTable[0])

const R4A_MENU_TABLE menuTable[] =
{
    // menuName         preMenu routine firstEntry          entryCount
    {"Main Menu",       mainMenuPre,    mainMenuTable,      MAIN_MENU_ENTRIES},
    {"Debug Menu",      nullptr,        debugMenuTable,     DEBUG_MENU_ENTRIES},
    {"GPIO Menu",       nullptr,        gpioMenuTable,      GPIO_MENU_ENTRIES},
    {"LED Menu",        nullptr,     r4a4wdCarLedMenuTable, R4A_4WD_CAR_LED_MENU_ENTRIES},
    {"NVM Menu",        nullptr,      r4aEsp32NvmMenuTable, R4A_ESP32_NVM_MENU_ENTRIES},
    {"WS2812 Menu",     nullptr,        ws2812MenuTable,    WS2812_MENU_ENTRIES},
};
const int menuTableEntries = sizeof(menuTable) / sizeof(menuTable[0]);
