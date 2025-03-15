/**********************************************************************
  NVM_Menu.cpp

  Robots-For-All (R4A)
  Menu for non-volatile memory (NVM) support
**********************************************************************/

#include "R4A_ESP32.h"

//****************************************
// NVM Menu
//****************************************

const R4A_MENU_ENTRY r4aEsp32NvmMenuTable[] =
{
    // Command  menuRoutine                     menuParam           HelpRoutine         align   HelpText
    {"cat",     r4aEsp32NvmMenuFileCat,         (intptr_t)"ffff",   r4aMenuHelpSuffix,  4,      "Display the contents of file ffff"},   //  0
    {"cp",      r4aEsp32NvmMenuFileCopy,      (intptr_t)"src dest", r4aMenuHelpSuffix,  8,      "Copy src file to dest file"},          //  1
    {"d",     r4aEsp32NvmMenuDisplayParameters, 0,                  nullptr,            0,      "Display NVM parameters"},              //  2
    {"dump",    r4aEsp32NvmMenuFileDump,        (intptr_t)"ffff",   r4aMenuHelpSuffix,  4,      "Dump the contents of file ffff"},      //  3
    {"f",     r4aEsp32NvmMenuDumpParameterFile, 0,                  nullptr,            0,      "Dump the parameter file"},             //  4
    {"g",  r4aEsp32NvmMenuGetDefaultParameters, 0,                  nullptr,            0,      "Get default parameters"},              //  5
    {"l",     r4aEsp32NvmMenuParameterFileRead, 0,                  nullptr,            0,      "Load the parameter file"},             //  6
    {"ls",      r4aEsp32NvmMenuFileList,        0,                  nullptr,            0,      "List the NVM directory"},              //  7
    {"nd",      r4aMenuBoolToggle,   (intptr_t)&r4aEsp32NvmDebug,   r4aMenuBoolHelp,    0,      "Toggle NVM debugging"},                //  8
    {"mv",      r4aEsp32NvmMenuFileMove,      (intptr_t)"src dest", r4aMenuHelpSuffix,  8,      "Rename a file"},                       //  9
    {"p",      r4aEsp32NvmMenuParameterDisplay, (intptr_t)"pppp",   r4aMenuHelpSuffix,  4,      "Display parameter pppp value"},        // 10
    {"rm",      r4aEsp32NvmMenuFileRemove,      (intptr_t)"ffff",   r4aMenuHelpSuffix,  4,      "Remove file ffff"},                    // 11
    {"s",       r4aEsp32NvmMenuParameterSet, (intptr_t)"pppp ssss", r4aMenuHelpSuffix,  9,      "Set parameter pppp value to xxxx"},    // 12
    {"w",    r4aEsp32NvmMenuParameterFileWrite, 0,                  nullptr,            0,      "Write the parameter file"},            // 13
    {"wget",    r4aEsp32NvmMenuHttpFileGet,     (intptr_t)"url",    r4aMenuHelpSuffix,  3,      "Get a file from a web server"},        // 14
    {"x",       nullptr,                        R4A_MENU_MAIN,      nullptr,            0,      "Exit the menu system"},                // 15
    {"z",       r4aEsp32NvmMenuParameterClear,  (intptr_t)"pppp",   r4aMenuHelpSuffix,  4,      "Zero the parameter"},                  // 16
};                                                                                                                                      // 17
