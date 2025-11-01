/**********************************************************************
  Seven_Segment_Display.ino

  Support the seven segment display
**********************************************************************/

#define DECIMAL_POINT       0x80

//*********************************************************************
// Clear the display
void sevenSegmentClear()
{
    uint8_t display[4];

    // Clear the display
    memset(display, 0, sizeof(display));
    r4aI2cBusWrite(r4aI2cBus,
                   SEVEN_SEG_DISP_I2C_ADDR,
                   display,
                   sizeof(display),
                   nullptr);
}

//*********************************************************************
// Translate value into the seven segments for display
uint8_t sevenSegmentGet(uint8_t value)
{
    switch (value)
    {
    //                 gfedcba
    case  0: return 0b0111111;
    case  1: return 0b0000110;
    case  2: return 0b1011011;
    case  3: return 0b1001111;
    case  4: return 0b1100110;
    case  5: return 0b1101101;
    case  6: return 0b1111101;
    case  7: return 0b0000111;
    case  8: return 0b1111111;
    case  9: return 0b1100111;
    case 10: return 0b1110111;  // A
    case 11: return 0b1111100;  // b
    case 12: return 0b0111001;  // C
    case 13: return 0b1011110;  // d
    case 14: return 0b1111001;  // E
    case 15: return 0b1110001;  // F
    default: return 0b0000000;  // Blank
    }
}

//*********************************************************************
// Display idle on the seven segment display
void sevenSegmentIdle()
{
    uint8_t display[4];

    //                a
    //               ---
    //            f |   | b
    //               -g-
    //            e |   | c
    //               ---
    //                d    dp
    //
    //            dpgfedcba
    display[3] = 0b00110000;   // I
    display[2] = 0b01011110;   // d
    display[1] = 0b00111000;   // L
    display[0] = 0b01111001;   // E
    r4aI2cBusWrite(r4aI2cBus,
                   SEVEN_SEG_DISP_I2C_ADDR,
                   display,
                   sizeof(display),
                   nullptr);
}

//*********************************************************************
// Display the delta time on seven segment displays
void sevenSegmentDeltaTime(uint32_t milliseconds)
{
    uint8_t display[4];
    uint32_t thousands;
    uint32_t hundreds;
    uint32_t tens;
    uint32_t seconds;
    uint32_t tenths;

    // Parse the time
    seconds = milliseconds / R4A_MILLISECONDS_IN_A_SECOND;
    milliseconds -= seconds * R4A_MILLISECONDS_IN_A_SECOND;
    thousands = seconds / 1000;
    seconds -= thousands * 1000;
    hundreds = seconds / 100;
    seconds -= hundreds * 100;
    tens = seconds / 10;
    seconds -= tens * 10;
    tenths = milliseconds / 100;

    // Handle overflow
    if (thousands > 9)
    {
        display[3] = sevenSegmentGet(9) | DECIMAL_POINT;
        display[2] = sevenSegmentGet(9) | DECIMAL_POINT;
        display[1] = sevenSegmentGet(9) | DECIMAL_POINT;
        display[0] = sevenSegmentGet(9) | DECIMAL_POINT;
    }
    else if (thousands)
    {
        display[3] = sevenSegmentGet(thousands);
        display[2] = sevenSegmentGet(hundreds);
        display[1] = sevenSegmentGet(tens);
        display[0] = sevenSegmentGet(seconds);
    }
    else if (hundreds)
    {
        display[3] = sevenSegmentGet(hundreds);
        display[2] = sevenSegmentGet(tens);
        display[1] = sevenSegmentGet(seconds) | DECIMAL_POINT;
        display[0] = sevenSegmentGet(tenths);
    }
    else if (tens)
    {
        display[3] = sevenSegmentGet(' ');
        display[2] = sevenSegmentGet(tens);
        display[1] = sevenSegmentGet(seconds) | DECIMAL_POINT;
        display[0] = sevenSegmentGet(tenths);
    }
    else
    {
        display[3] = sevenSegmentGet(' ');
        display[2] = sevenSegmentGet(' ');
        display[1] = sevenSegmentGet(seconds) | DECIMAL_POINT;
        display[0] = sevenSegmentGet(tenths);
    }

    // Update the seven segment display
    r4aI2cBusWrite(r4aI2cBus,
                   SEVEN_SEG_DISP_I2C_ADDR,
                   display,
                   sizeof(display),
                   nullptr);
}

//*********************************************************************
// Display the time on seven segment displays
void sevenSegmentNtpTime(uint32_t currentMsec)
{
    uint8_t display[4];
    uint8_t hours;
    static uint32_t lastMsec;
    uint8_t minutes;

    // Display time once per second
    if ((currentMsec - lastMsec) >= 1000)
    {
        lastMsec = currentMsec;

        // Get the time value
        time_t seconds = r4aNtpGetEpochTime();
        hours = hourFormat12(seconds);
        minutes = minute(seconds);

        // Convert to segments
        display[3] = (hours > 9) ? sevenSegmentGet(1) : 0;
        display[2] = sevenSegmentGet(hours % 10) | DECIMAL_POINT;
        display[1] = sevenSegmentGet(minutes / 10);
        display[0] = sevenSegmentGet(minutes % 10);

        // Update the seven segment display
        r4aI2cBusWrite(r4aI2cBus,
                       SEVEN_SEG_DISP_I2C_ADDR,
                       display,
                       sizeof(display),
                       nullptr);
    }
}
