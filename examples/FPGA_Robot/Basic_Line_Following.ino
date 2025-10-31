/**********************************************************************
  Basic_Line_Following.ino

  Perform basic line following, modified code from 04.2_Track_Car example
**********************************************************************/

//****************************************
// Constants
//****************************************

#define BLF_DEBUG_STATES        0

//****************************************
// Locals
//****************************************

int blfTimeBefore = 0;  //Record each non-blocking time
int blfTimeCount = 0;   //Record the number of non-blocking times
int blfTimeFlag = 0;    //Record the blink time

//*********************************************************************
// The robotRunning routine calls this routine to actually perform
// the challenge.  This routine typically reads a sensor and may
// optionally adjust the motors based upon the sensor reading.  The
// routine then must return.  The robot layer will call this routine
// multiple times during the robot operation.
void blfChallenge(R4A_ROBOT_CHALLENGE * object)
{
    // Read the line sensors
    pcf8574.read(&lineSensors);
    lineSensors &= 7;
    if (BLF_DEBUG_STATES)
        Serial.printf("%d %d %d\r\n",
                      lineSensors & 1,
                      (lineSensors & 2) ? 1 : 0,
                      (lineSensors & 4) ? 1 : 0);

    // Update the robot direction
    switch (lineSensors)
    {
    //     RcL
    case 0b000:
    case 0b111:
    default:
        // No line or stop circle detected
        r4aPca9685MotorBrakeAll();
        r4aRobotStop(&robot, millis());
        break;

    //     RcL
    case 0b010:
    case 0b101:
        // Robot over center of line
        robotMotorSetSpeeds(blfSpeedSlow,  blfSpeedSlow); // Move Forward
        break;

    //     RcL
    case 0b001:
    case 0b011:
        // Robot over left sensor, need to turn left
        robotMotorSetSpeeds(-blfSpeedMedium, blfSpeedFast); // Turn left
        break;

    //     RcL
    case 0b100:
    case 0b110:
        // Robot over right sensor, need to turn right
        robotMotorSetSpeeds(blfSpeedFast, -blfSpeedMedium); // Turn right
        break;
    }
}

//*********************************************************************
// The robotStart calls this routine before switching to the initial
// delay state.
void blfInit(R4A_ROBOT_CHALLENGE * object)
{
    challengeInit();
}

//*********************************************************************
// Start the line following
void blfStart(Print * display)
{
    static R4A_ROBOT_CHALLENGE basicLineFollowing =
    {
        blfChallenge,
        blfInit,
        blfStart,
        blfStop,

        "Basic Line Following",             // _name
        R4A_CHALLENGE_SEC_LINE_FOLLOWING    // Challenge duration in seconds
    };
    R4A_ROBOT_CHALLENGE * challengeStructure;
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
    {
        // Start the robot challenge if the robot is not active
        challengeStructure = &basicLineFollowing;
        r4aRobotStart(&robot,
                      challengeStructure,
                      robotStartDelaySec,
                      display);
    }
}

//*********************************************************************
// The initial delay routine calls this routine just before calling
// the challenge routine for the first time.
void blfStart(R4A_ROBOT_CHALLENGE * object)
{
    challengeStart();
}

//*********************************************************************
// Start the line following
void blfStartMenu(const struct _R4A_MENU_ENTRY * menuEntry,
                  const char * command,
                  Print * display)
{
    blfStart(display);
}

//*********************************************************************
// The robot.stop routine calls this routine to stop the motors and
// perform any other actions.
void blfStop(R4A_ROBOT_CHALLENGE * object)
{
    challengeStop();
}

//*********************************************************************
// Start the line following at boot
void menuStartBlf(const struct _R4A_MENU_ENTRY * menuEntry,
                  const char * command,
                  Print * display)
{
    startIndex = CHALLENGE_BLF;
    r4aEsp32NvmMenuParameterFileWrite(menuEntry, command, display);
}
