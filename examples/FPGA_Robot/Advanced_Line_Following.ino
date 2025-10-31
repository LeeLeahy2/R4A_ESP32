/**********************************************************************
  Advanced_Line_Following.ino

  Advanced line following support
**********************************************************************/

//*********************************************************************
// Do the line following
void alfChallenge(R4A_ROBOT_CHALLENGE * object)
{
    challengeHalt("Code missing in alfChallenge, reads sensors and make decisions to drive motors!");
}

//*********************************************************************
// The robotStart calls this routine before switching to the initial
// delay state.
void alfInit(R4A_ROBOT_CHALLENGE * object)
{
    challengeInit();
}

//*********************************************************************
// Start the line following
void alfStart(Print * display)
{
    static R4A_ROBOT_CHALLENGE advancedLineFollowing =
    {
        alfChallenge,
        alfInit,
        alfStart,
        alfStop,
        "Advanced Line Following",          // _name
        R4A_CHALLENGE_SEC_LINE_FOLLOWING    // Challenge duration in seconds
    };
    float voltage;

    // Verify the I2C bus configuration
    if (!pca9685Present)
    {
        display->printf("ERROR: PCA9685 (motors) not responding on I2C bus!\r\n");
        return;
    }
    if (!pcf8574Present)
    {
        display->printf("ERROR: PCF8574 (line sensor) not responding on I2C bus!\r\n");
        return;
    }

    // Only start the robot if the battery is on
    if (!robotCheckBatteryLevel())
    {
        voltage = READ_BATTERY_VOLTAGE(nullptr);
        display->printf("Battery: %f4.2V\r\n", voltage);
        display->println("WARNING: Battery voltage to run the robot!");
    }
    else
        // Start the robot challenge if the robot is not active
        r4aRobotStart(&robot,
                      &advancedLineFollowing,
                      robotStartDelaySec,
                      display);
}

//*********************************************************************
// The initial delay routine calls this routine just before calling
// the challenge routine for the first time.
void alfStart(R4A_ROBOT_CHALLENGE * object)
{
    challengeStart();
}

//*********************************************************************
// Start the line following
void alfStartMenu(const struct _R4A_MENU_ENTRY * menuEntry,
                  const char * command,
                  Print * display)
{
    alfStart(display);
}

//*********************************************************************
// Stop the robot and perform any other actions
void alfStop(R4A_ROBOT_CHALLENGE * object)
{
    // Stop the robot
    challengeStop();
}

//*********************************************************************
// Start the line following at boot
void menuStartAlf(const struct _R4A_MENU_ENTRY * menuEntry,
                  const char * command,
                  Print * display)
{
    startIndex = CHALLENGE_ALF;
    r4aEsp32NvmMenuParameterFileWrite(menuEntry, command, display);
}
