/**********************************************************************
  Robot.ino

  Robot layer support routines
**********************************************************************/

bool robotPreviousNtpTime;
bool robotNtpTimeRestoreNecessary;

//*********************************************************************
// Called when the robot starts
bool robotCheckBatteryLevel()
{
    float voltage;

    // Check for ignoring the battery check
    if ((!enableBatteryCheck) || ignoreBatteryCheck)
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

    // Display the seconds and tenths on the LED matrix
    if (vk16k33Present)
        vk16k33DeltaTime(milliseconds);

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
    if ((currentMsec - previousMsec) > 50)
    {
        previousMsec = currentMsec;

        // Determine if the display should be updated.  Initially when the
        // robot stops, the brake lights are on and the runtime is displayed.
        // After some time (robotRunTimeSec), the WS2812 RGB LEDs and the
        // display switch to something else.
        if ((currentMsec - r4aRobotGetStopTime(&robot)) >= (robotRunTimeSec * R4A_MILLISECONDS_IN_A_SECOND))
        {
            // Update the display data selection selection
            robotNtpTimeRestore();

            // Display Idle on the LED matrix
            if (vk16k33Present && robotDisplayIdle && (!robotNtpTime || !r4aNtpIsTimeValid()))
                r4aVk16k33DisplayIdle(&vk16k33);

            // Turn off the brake lights
            if (robotUseWS2812)
            {
                car.ledsOff();

                // Determine if line sensor LEDs should be updated
                if (robotLineSensorLEDs && (pcf8574Present || sx1509Present))
                {
                    // Read the line sensors
                    if (sx1509Present)
                    {
#ifdef USE_SPARKFUN_SEN_13582
                        r4aSx1509RegisterRead(&sx1509,
                                              SX1509_DATA_A,
                                              &lineSensors);
                        r4aLEDSetColorRgb(FRONT_L1, lineSensors & 0x02 ? R4A_LED_YELLOW : R4A_LED_OFF);
                        r4aLEDSetColorRgb(FRONT_L2, lineSensors & 0x04 ? R4A_LED_YELLOW : R4A_LED_OFF);
                        r4aLEDSetColorRgb(FRONT_L3, lineSensors & 0x08 ? R4A_LED_YELLOW : R4A_LED_OFF);
                        r4aLEDSetColorRgb(FRONT_R3, lineSensors & 0x10 ? R4A_LED_YELLOW : R4A_LED_OFF);
                        r4aLEDSetColorRgb(FRONT_R2, lineSensors & 0x20 ? R4A_LED_YELLOW : R4A_LED_OFF);
                        r4aLEDSetColorRgb(FRONT_R1, lineSensors & 0x40 ? R4A_LED_YELLOW : R4A_LED_OFF);
#endif  // USE_SPARKFUN_SEN_13582
                    }
                    else if (pcf8574Present)
                    {
                        pcf8574.read(&lineSensors);
                        lineSensors &= 7;
                        r4aLEDSetColorRgb(FRONT_L1, lineSensors & 1 ? R4A_LED_YELLOW : R4A_LED_OFF);
                        r4aLEDSetColorRgb(FRONT_L3, lineSensors & 2 ? R4A_LED_YELLOW : R4A_LED_OFF);
                        r4aLEDSetColorRgb(FRONT_R3, lineSensors & 2 ? R4A_LED_YELLOW : R4A_LED_OFF);
                        r4aLEDSetColorRgb(FRONT_R1, lineSensors & 4 ? R4A_LED_YELLOW : R4A_LED_OFF);
                    }
                }
            }
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
    // Skip of the speed is the same
    if ((robotLeftSpeed == left) && (robotRightSpeed == right))
        return true;

    // Remember the new speeds
    robotLeftSpeed = left;
    robotRightSpeed = right;

    // Update motor speeds
    return motorFrontLeft.speed(left, display)
           && motorBackLeft.speed(left, display)
           && motorFrontRight.speed(right, display)
           && motorBackRight.speed(right, display)
           && pca9685.writeBufferedRegisters(display);
}

//*********************************************************************
// Restore the time display state
void robotNtpTimeRestore()
{
    if (robotNtpTimeRestoreNecessary)
    {
        robotNtpTimeRestoreNecessary = false;
        robotNtpTime = robotPreviousNtpTime;
    }
}

//*********************************************************************
// Save the time display state
void robotNtpTimeSave()
{
    robotPreviousNtpTime = robotNtpTime;
    robotNtpTimeRestoreNecessary = robotNtpTime;
    robotNtpTime = false;
}
