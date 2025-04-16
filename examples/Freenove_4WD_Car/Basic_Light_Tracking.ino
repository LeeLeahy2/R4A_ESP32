/**********************************************************************
  Basic_Light_Tracking.ino

  Light tracking support
**********************************************************************/

//****************************************
// Constants
//****************************************

#define BLT_SPEED_FAST      1500
#define BLT_SPEED_MEDIUM    1200
#define BLT_SPEED_SLOW      1000

#define BLT_TURN_THRESHOLD  100

//*********************************************************************
// The robotRunning routine calls this routine to actually perform
// the challenge.  This routine typically reads a sensor and may
// optionally adjust the motors based upon the sensor reading.  The
// routine then must return.  The robot layer will call this routine
// multiple times during the robot operation.
void bltChallenge(R4A_ROBOT_CHALLENGE * object)
{
    // Read the voltage from the photo-resistor voltage divider
    r4aEsp32VoltageGet(LIGHT_SENSOR_PIN,
                       0,
                       1,
                       &lsAdcValue);

    // Is the light on the left side?
    if (lsAdcValue <= (lsAdcReference - BLT_TURN_THRESHOLD))
        robotMotorSetSpeeds(BLT_SPEED_MEDIUM, BLT_SPEED_FAST);

    // Is the light on the right side?
    else if (lsAdcValue >= (lsAdcReference + BLT_TURN_THRESHOLD))
        robotMotorSetSpeeds(BLT_SPEED_FAST, BLT_SPEED_MEDIUM);

    // Go straight
    else
        robotMotorSetSpeeds(BLT_SPEED_SLOW, BLT_SPEED_SLOW);
}

//*********************************************************************
// The robotStart calls this routine before switching to the initial
// delay state.
void bltInit(R4A_ROBOT_CHALLENGE * object)
{
    challengeInit();
}

//*********************************************************************
// The initial delay routine calls this routine just before calling
// the challenge routine for the first time.
void bltStart(R4A_ROBOT_CHALLENGE * object)
{
    challengeStart();

    // Set the reference voltage from the photo-resistor voltage divider
    r4aEsp32VoltageGet(LIGHT_SENSOR_PIN,
                       0,
                       1,
                       &lsAdcReference);
}

//*********************************************************************
// The robot.stop routine calls this routine to stop the motors and
// perform any other actions.
void bltStop(R4A_ROBOT_CHALLENGE * object)
{
    challengeStop();
}

//*********************************************************************
// Start the light tracking
void bltStart(Print * display)
{
    static R4A_ROBOT_CHALLENGE basicLightTracking =
    {
        bltChallenge,
        bltInit,
        bltStart,
        bltStop,

        "Basic Light Tracking",             // _name
        ROBOT_LIGHT_TRACKING_DURATION_SEC,  // _duration
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
                      &basicLightTracking,
                      display);
}

//*********************************************************************
// Start the light following
void bltStartMenu(const struct _R4A_MENU_ENTRY * menuEntry,
                  const char * command,
                  Print * display)
{
    bltStart(display);
}

//*********************************************************************
// Start the light tracking at boot
void menuStartBlt(const struct _R4A_MENU_ENTRY * menuEntry,
                  const char * command,
                  Print * display)
{
    startIndex = CHALLENGE_BLT;
    r4aEsp32NvmMenuParameterFileWrite(menuEntry, command, display);
}
