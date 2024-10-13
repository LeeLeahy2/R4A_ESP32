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
    uint32_t seconds;

    // Display the seconds on the LEDs
    seconds = milliseconds / R4A_MILLISECONDS_IN_A_SECOND;
    r4aLEDSetColorRgb(FRONT_L1, (seconds & 1) ? R4A_LED_AQUA : R4A_LED_OFF);
    r4aLEDSetColorRgb(FRONT_L2, (seconds & 2) ? R4A_LED_AQUA : R4A_LED_OFF);
    r4aLEDSetColorRgb(FRONT_L3, (seconds & 4) ? R4A_LED_AQUA : R4A_LED_OFF);
    r4aLEDSetColorRgb(FRONT_R3, (seconds & 8) ? R4A_LED_AQUA : R4A_LED_OFF);
    r4aLEDSetColorRgb(FRONT_R2, (seconds & 16) ? R4A_LED_AQUA : R4A_LED_OFF);
    r4aLEDSetColorRgb(FRONT_R1, (seconds & 32) ? R4A_LED_AQUA : R4A_LED_OFF);
}

//*********************************************************************
// Update the WS2812 LEDs when the robot is idle
void robotIdle(uint32_t currentMsec)
{
    static uint32_t previousMsec;

    // Update the LEDs on a regular interval
    if ((currentMsec - previousMsec) > 50)
    {
        previousMsec = currentMsec;

        // Read the line sensors
        pcf8574.read(&lineSensors);
        lineSensors &= 7;

        // Display the line sensors
        r4aLEDSetColorRgb(FRONT_L1, lineSensors & 1 ? R4A_LED_YELLOW : R4A_LED_OFF);
        r4aLEDSetColorRgb(FRONT_L3, lineSensors & 2 ? R4A_LED_YELLOW : R4A_LED_OFF);
        r4aLEDSetColorRgb(FRONT_R3, lineSensors & 2 ? R4A_LED_YELLOW : R4A_LED_OFF);
        r4aLEDSetColorRgb(FRONT_R1, lineSensors & 4 ? R4A_LED_YELLOW : R4A_LED_OFF);
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
