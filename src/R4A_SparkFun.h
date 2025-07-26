/**********************************************************************
  R4A_SparkFun.h

  Robots-For-All (R4A)
  SparkFun Electronics declarations
**********************************************************************/

#ifndef __R4A_SPARKFUN_H__
#define __R4A_SPARKFUN_H__

#include "R4A_ESP32.h"
#include <R4A_SX1509.h>

//****************************************
// SEN-13582
//****************************************

#define SFE_SEN13582_IR_LED_ENABLE          0x01
#define SFE_SEN13582_FEEDBACK_LED_ENABLE    0x02

// Initialize the SX1509
// Inputs:
//   sx1509: Address of a R4A_SX1509 data structure
//   display: Device used for output
// Outputs:
//   Returns true if successful and false upon failure
bool r4aSfeSen13582Begin(R4A_SX1509 * sx1509, Print * display = nullptr);

// Enable/disable the feedback LEDs
// Inputs:
//   on: Set to true to turn on LEDs and false to turn off LEDs
//   display: Device used for output
// Outputs:
//   Returns true if successful and false upon failure
bool r4aSfeSen13582LedsFeedback(bool on, Print * display = nullptr);

// Enable/disable the IR LEDs
// Inputs:
//   on: Set to true to turn on LEDs and false to turn off LEDs
//   display: Device used for output
// Outputs:
//   Returns true if successful and false upon failure
bool r4aSfeSen13582LedsIr(bool on, Print * display = nullptr);

// Print the SX1509 registers
// Inputs:
//   menuEntry: Address of the object describing the menu entry
//   command: Zero terminated command string
//   display: Device used for output
void r4aSfeSen13582MenuDisplayRegisters(const R4A_MENU_ENTRY * menuEntry,
                                        const char * command,
                                        Print * display = &Serial);

// Toggle (turn on/off) the feedback LEDs
// Inputs:
//   menuEntry: Address of the object describing the menu entry
//   command: Zero terminated command string
//   display: Device used for output
void r4aSfeSen13582MenuFeedbackLedToggle(const R4A_MENU_ENTRY * menuEntry,
                                         const char * command,
                                         Print * display);

// Toggle (turn on/off) the IR LEDs
// Inputs:
//   menuEntry: Address of the object describing the menu entry
//   command: Zero terminated command string
//   display: Device used for output
void r4aSfeSen13582MenuIrLedToggle(const R4A_MENU_ENTRY * menuEntry,
                                   const char * command,
                                   Print * display);

#endif  // __R4A_SPARKFUN_H__
