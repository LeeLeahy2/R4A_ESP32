/**********************************************************************
  SFE_SEN_13582.cpp

  Robots-For-All (R4A)
  SparkFun SEN-13582 support
**********************************************************************/

#include "R4A_SparkFun.h"

//****************************************
// Locals
//****************************************

static R4A_SX1509 * r4aSx1509;

//*********************************************************************
// Initialize the SX1509
bool r4aSfeSen13582Begin(R4A_SX1509 * sx1509, Print * display)
{
    // Save the pointer for menu calls
    r4aSx1509 = sx1509;

    // Inputs: IR feedback 0-7
    if (r4aSx1509RegisterWrite(r4aSx1509, SX1509_DIR_A, 0xff, display) == false)
    {
        if (display)
            display->printf("ERROR: Failed to set ports 0-7 as inputs\r\n");
        return false;
    }

    // Outputs: IR enable, feedback enable, N/C pins
    if (r4aSx1509RegisterWrite(r4aSx1509, SX1509_DIR_B, 0, display) == false)
    {
        if (display)
            display->printf("ERROR: Failed to set ports 8-15 as outputs\r\n");
        return false;
    }

    // Enable the IR LEDs
    if (r4aSfeSen13582LedsIr(true, display) == false)
    {
        if (display)
            display->printf("ERROR: Failed to turn on the feedback LEDs\r\n");
        return false;
    }

    // Enable the feedback LEDS
    if (r4aSfeSen13582LedsFeedback(true) == false)
    {
        if (display)
            display->printf("ERROR: Failed to turn on the feedback LEDs\r\n");
        return false;
    }
    return true;
}

//*********************************************************************
// Enable/disable the feedback LEDs
bool r4aSfeSen13582LedsFeedback(bool on, Print * display)
{
    uint8_t andMask;
    uint8_t xorMask;

    // Enable the IR and feedback
    andMask = 0xff & ~SFE_SEN13582_FEEDBACK_LED_ENABLE;
    xorMask = on ? 0 : SFE_SEN13582_FEEDBACK_LED_ENABLE;
    return r4aSx1509RegisterModify(r4aSx1509, SX1509_DATA_B, andMask, xorMask, display);
}

//*********************************************************************
// Enable/disable the IR LEDs
bool r4aSfeSen13582LedsIr(bool on, Print * display)
{
    uint8_t andMask;
    uint8_t xorMask;

    // Enable the IR and feedback
    andMask = 0xff & ~SFE_SEN13582_IR_LED_ENABLE;
    xorMask = on ? 0 : SFE_SEN13582_IR_LED_ENABLE;
    return r4aSx1509RegisterModify(r4aSx1509, SX1509_DATA_B, andMask, xorMask, display);
}

//*********************************************************************
// Print the SX1509 registers
void r4aSfeSen13582MenuDisplayRegisters(const R4A_MENU_ENTRY * menuEntry,
                                        const char * command,
                                        Print * display)
{
    // Display the SX1509 registers
    r4aSx1509DisplayRegisters(r4aSx1509, display);
}

//*********************************************************************
// Toggle (turn on/off) the feedback LEDs
void r4aSfeSen13582MenuFeedbackLedToggle(const R4A_MENU_ENTRY * menuEntry,
                                         const char * command,
                                         Print * display)
{
    // Toggle the register value
    if (r4aSx1509RegisterModify(r4aSx1509, SX1509_DATA_B, 0xff, SFE_SEN13582_FEEDBACK_LED_ENABLE)
        && display)
    {
        uint8_t data;

        // Display the new state
        if (r4aSx1509RegisterRead(r4aSx1509, SX1509_DATA_B, &data))
            display->printf("Feedback LEDs: %s\r\n", (data & SFE_SEN13582_FEEDBACK_LED_ENABLE) ? "Off" : "On");
        else if (display)
            display->printf("ERROR: Failed to read the SX1509 data register!\r\n");
    }
    else if (display)
        display->printf("ERROR: Failed to update the SX1509 data register!\r\n");
}

//*********************************************************************
// Toggle (turn on/off) the IR LEDs
void r4aSfeSen13582MenuIrLedToggle(const R4A_MENU_ENTRY * menuEntry,
                                   const char * command,
                                   Print * display)
{
    // Toggle the register value
    if (r4aSx1509RegisterModify(r4aSx1509, SX1509_DATA_B, 0xff, SFE_SEN13582_IR_LED_ENABLE)
        && display)
    {
        uint8_t data;

        // Display the new state
        if (r4aSx1509RegisterRead(r4aSx1509, SX1509_DATA_B, &data))
            display->printf("Feedback LEDs: %s\r\n", (data & SFE_SEN13582_IR_LED_ENABLE) ? "Off" : "On");
        else if (display)
            display->printf("ERROR: Failed to read the SX1509 data register!\r\n");
    }
    else if (display)
        display->printf("ERROR: Failed to update the SX1509 data register!\r\n");
}
