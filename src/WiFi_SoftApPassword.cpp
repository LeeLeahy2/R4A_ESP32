/**********************************************************************
  Wifi_SoftPassword.cpp

  Provides weak references for WiFi data.
**********************************************************************/

#include "R4A_ESP32.h"

//****************************************
// Globals
//****************************************

// The weak attributes tells the linker to use this value when the program
// or sketch does not include the variable.
const char * r4aWifiSoftApPassword __attribute__((weak));
