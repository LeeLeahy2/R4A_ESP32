/**********************************************************************
  Menu.ino

  Robots-For-All (R4A)
  Menu routines
**********************************************************************/

//*********************************************************************

enum MENU_TABLE_INDEX
{
    MTI_I2C = R4A_MENU_MAIN + 1,
    MTI_NVM,
    // Add new menu values before this line
    MTI_MAX
};
const int menuTableIndexMax = MTI_MAX;

// Main menu
const R4A_MENU_ENTRY mainMenuTable[] =
{
    // Command  menuRoutine         menuParam       HelpRoutine align   HelpText
    {"I2C",     nullptr,            MTI_I2C,        nullptr,    0,      "Enter the I2C menu"},
    {"nvm",     nullptr,            MTI_NVM,        nullptr,    0,      "Enter the NVM menu"},
    {"r",  r4aEsp32MenuSystemReset, 0,              nullptr,    0,      "System reset"},
    {"x",       nullptr,            R4A_MENU_NONE,  nullptr,    0,      "Exit the menu system"},
};
#define MAIN_MENU_ENTRIES       sizeof(mainMenuTable) / sizeof(mainMenuTable[0])

const R4A_MENU_TABLE menuTable[] =
{
    // menuName         preMenu routine firstEntry          entryCount
    {"Main Menu",       nullptr,        mainMenuTable,      MAIN_MENU_ENTRIES},
    {"I2C Menu",        nullptr,        r4aI2cMenuTable,    R4A_I2C_MENU_ENTRIES},
    {"NVM Menu",        nullptr,      r4aEsp32NvmMenuTable, R4A_ESP32_NVM_MENU_ENTRIES},
};
const int menuTableEntries = sizeof(menuTable) / sizeof(menuTable[0]);
