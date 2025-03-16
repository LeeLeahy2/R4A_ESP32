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
