/**********************************************************************
  Robot.ino

  Robot layer support routines
**********************************************************************/

//*********************************************************************
// Called when the robot starts
bool robotCheckBatteryLevel()
{
    float voltage;

    // Check for ignoring the battery check
    if (ignoreBatteryCheck)
        return true;

    // Verify the voltage level
    voltage = READ_BATTERY_VOLTAGE(nullptr);
    return (voltage >= 8.0);
}

//*********************************************************************
// Display the delta time
void robotDisplayTime(uint32_t milliseconds)
{
    uint32_t thousands;
    uint32_t hundreds;
    uint32_t seconds;
    uint32_t tens;
    uint32_t tenths;

    // Display the seconds and tenths on the LED matrix
    seconds = milliseconds / R4A_MILLISECONDS_IN_A_SECOND;
    if (vk16k33Present)
    {
        // Parse the time
        milliseconds -= seconds * R4A_MILLISECONDS_IN_A_SECOND;
        thousands = seconds / 1000;
        seconds -= thousands * 1000;
        hundreds = seconds / 100;
        seconds -= hundreds * 100;
        tens = seconds / 10;
        seconds -= tens * 10;
        tenths = milliseconds / 100;

        // Handle overflow
        if (thousands > 1)
        {
            thousands = ' ';
            hundreds = '*';
            tens = '*';
            seconds = '*';
        }
        else
        {
            // Handle leading zero suppression
            if (thousands)
            {
                thousands += '0';
                hundreds += '0';
                tens += '0';
            }
            else if (hundreds)
            {
                thousands = ' ';
                hundreds += '0';
                tens += '0';
            }
            else if (tens)
                tens += '0';
            seconds += '0';
            tenths += '0';
        }

        // Display the time with zero suppression
        r4aVk16k33BufferClear(&vk16k33);
        if (hundreds)
        {
            // Display the range 100 - 1999 or overflow
            r4aVk16k33DisplayChar(&vk16k33, 0, (char)thousands);
            r4aVk16k33DisplayChar(&vk16k33, 1, (char)hundreds);
            r4aVk16k33DisplayChar(&vk16k33, 6, (char)tens);
            r4aVk16k33DisplayChar(&vk16k33, 11, (char)seconds);
        }
        else
        {
            // Display the range 0 - 99.9
            if (tens)
                r4aVk16k33DisplayChar(&vk16k33, 0, (char)tens);
            r4aVk16k33DisplayChar(&vk16k33, 5, (char)seconds);
            r4aVk16k33DisplayChar(&vk16k33, 10, '.');
            r4aVk16k33DisplayChar(&vk16k33, 11, (char)tenths);
        }
        r4aVk16k33DisplayPixels(&vk16k33);
    }

    // Display the seconds on the LEDs
    else if (robotUseWS2812)
    {
        seconds = milliseconds / R4A_MILLISECONDS_IN_A_SECOND;
        r4aLEDSetColorRgb(FRONT_L1, (seconds & 1) ? R4A_LED_AQUA : R4A_LED_OFF);
        r4aLEDSetColorRgb(FRONT_L2, (seconds & 2) ? R4A_LED_AQUA : R4A_LED_OFF);
        r4aLEDSetColorRgb(FRONT_L3, (seconds & 4) ? R4A_LED_AQUA : R4A_LED_OFF);
        r4aLEDSetColorRgb(FRONT_R3, (seconds & 8) ? R4A_LED_AQUA : R4A_LED_OFF);
        r4aLEDSetColorRgb(FRONT_R2, (seconds & 16) ? R4A_LED_AQUA : R4A_LED_OFF);
        r4aLEDSetColorRgb(FRONT_R1, (seconds & 32) ? R4A_LED_AQUA : R4A_LED_OFF);
    }
}

//*********************************************************************
// Update the WS2812 LEDs when the robot is idle
void robotIdle(uint32_t currentMsec)
{
    static uint32_t previousMsec;

    // Update the LEDs on a regular interval
    if (robotUseWS2812 && ((currentMsec - previousMsec) > 50))
    {
        previousMsec = currentMsec;

        // Display Idle on the LED matrix
        if (vk16k33Present)
            r4aVk16k33DisplayIdle(&vk16k33);

        // Read the line sensors
        pcf8574.read(&lineSensors);
        lineSensors &= 7;

        // Display the line sensors
        car.ledsOff();
        if (sx1509Present)
        {
            r4aLEDSetColorRgb(FRONT_L1, lineSensors & 0x02 ? R4A_LED_YELLOW : R4A_LED_OFF);
            r4aLEDSetColorRgb(FRONT_L2, lineSensors & 0x04 ? R4A_LED_YELLOW : R4A_LED_OFF);
            r4aLEDSetColorRgb(FRONT_L3, lineSensors & 0x08 ? R4A_LED_YELLOW : R4A_LED_OFF);
            r4aLEDSetColorRgb(FRONT_R3, lineSensors & 0x10 ? R4A_LED_YELLOW : R4A_LED_OFF);
            r4aLEDSetColorRgb(FRONT_R2, lineSensors & 0x20 ? R4A_LED_YELLOW : R4A_LED_OFF);
            r4aLEDSetColorRgb(FRONT_R1, lineSensors & 0x40 ? R4A_LED_YELLOW : R4A_LED_OFF);
        }
        else
        {
            r4aLEDSetColorRgb(FRONT_L1, lineSensors & 1 ? R4A_LED_YELLOW : R4A_LED_OFF);
            r4aLEDSetColorRgb(FRONT_L3, lineSensors & 2 ? R4A_LED_YELLOW : R4A_LED_OFF);
            r4aLEDSetColorRgb(FRONT_R3, lineSensors & 2 ? R4A_LED_YELLOW : R4A_LED_OFF);
            r4aLEDSetColorRgb(FRONT_R1, lineSensors & 4 ? R4A_LED_YELLOW : R4A_LED_OFF);
        }
    }
}

//*********************************************************************
// Set the speeds of the motors
// Start bit, I2C device address, ACK, register address, ACK, 8 data bytes
// with ACKs and a stop bit, all at 400 KHz
// 770 uSec = (1+8+1+8+1+((8+1)×32)+1)÷(400×1000)
// Returns true if successful, false otherwise
bool robotMotorSetSpeeds(int16_t left, int16_t right, Print * display)
{
    // Update motor speeds
    return motorFrontLeft.speed(left, display)
           && motorBackLeft.speed(left, display)
           && motorFrontRight.speed(right, display)
           && motorBackRight.speed(right, display)
           && pca9685.writeBufferedRegisters(display);
}
