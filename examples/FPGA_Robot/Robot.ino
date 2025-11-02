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
    // Display the delta time
    if (s7dPresent)
        sevenSegmentDeltaTime(milliseconds);
}

//*********************************************************************
// Update the robot idle state
void robotIdle(uint32_t currentMsec)
{
    static uint32_t previousMsec;

    // Update the LEDs on a regular interval
    if ((currentMsec - previousMsec) > 50)
    {
        previousMsec = currentMsec;

        // Determine if the display should be updated.  Initially when the
        // robot stops, the brake lights are on and the runtime is displayed.
        // After some time (robotRunTimeSec), the display switch to something
        // else.
        if ((currentMsec - r4aRobotGetStopTime(&robot)) >= (robotRunTimeSec * R4A_MILLISECONDS_IN_A_SECOND))
            // Update the display data selection selection
            robotNtpTimeRestore();

        // Display idle on the seven segment display
        if (robotDisplayIdle && (!robotNtpTime || !r4aNtpIsTimeValid()))
            sevenSegmentIdle();

        // Determine if line sensor LEDs should be updated
        if (robotLineSensorLEDs && pcf8574Present)
        {
            // Read the line sensors
            pcf8574.read(&lineSensors);
            lineSensors &= 7;

            // Update the LEDs
            r4aI2cBusWrite(r4aI2cBus,
                           LEDS_I2C_ADDRESS,
                           &lineSensors,
                           sizeof(lineSensors));
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
