/**********************************************************************
  User_Line_Following.ino

  User line following support
**********************************************************************/

//*********************************************************************
// Do the line following
void ulfChallenge(R4A_ROBOT_CHALLENGE * object)
{
    challengeHalt("Code missing in alfChallenge, reads sensors and make decisions to drive motors!");
}

//*********************************************************************
// The robotStart calls this routine before switching to the initial
// delay state.
void ulfInit(R4A_ROBOT_CHALLENGE * object)
{
    challengeInit();
}

//*********************************************************************
// The initial delay routine calls this routine just before calling
// the challenge routine for the first time.
void ulfStart(R4A_ROBOT_CHALLENGE * object)
{
    challengeStart();
}

//*********************************************************************
// Stop the robot and perform any other actions
void ulfStop(R4A_ROBOT_CHALLENGE * object)
{
    // Stop the robot
    challengeStop();
}

//*********************************************************************
// Start the line following
void menuUlfStart(const struct _R4A_MENU_ENTRY * menuEntry,
                  const char * command,
                  Print * display)
{
    static R4A_ROBOT_CHALLENGE userLineFollowing =
    {
        ulfChallenge,
        ulfInit,
        ulfStart,
        ulfStop,

        "Advanced Line Following",          // _name
        R4A_CHALLENGE_SEC_LINE_FOLLOWING    // Challenge duration in seconds
    };
    float voltage;

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
                      &userLineFollowing,
                      display);
}
