/**********************************************************************
  Basic_Line_Following.ino

  Perform basic line following, modified code from 04.2_Track_Car example
**********************************************************************/

//****************************************
// Constants
//****************************************

#define BLF_DEBUG_STATES        0

enum BLF_STATE
{
    BLF_STATE_STOP = 0,
    BLF_STATE_FORWARD,
};

const char * blfStateTable[] =
{
    "Stop",
    "Forward",
};

//****************************************
// Locals
//****************************************

static int blfTimeBefore = 0;   //Record each non-blocking time
static int blfTimeCount = 0;    //Record the number of non-blocking times
static int blfTimeFlag = 0;     //Record the blink time

static volatile uint8_t blfState;

//*********************************************************************
// The robotRunning routine calls this routine to actually perform
// the challenge.  This routine typically reads a sensor and may
// optionally adjust the motors based upon the sensor reading.  The
// routine then must return.  The robot layer will call this routine
// multiple times during the robot operation.
void blfChallenge(R4A_ROBOT_CHALLENGE * object)
{
    uint32_t currentUsec;
    static int16_t previousLeftSpeed;
    static int16_t previousRightSpeed;

    // Read the line sensors
    currentUsec = micros();
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
        robotMotorSetSpeeds(0, 0);
        r4aRobotStop(&robot, millis());
        blfState = BLF_STATE_STOP;
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

    // Log the sensors
    if (logBuffer && ((lineSensors != previousLineSensors)
                        || (robotLeftSpeed != previousLeftSpeed)
                        || (robotRightSpeed != previousRightSpeed)))
        logLineSensorData(currentUsec, blfState, LOG_LINE_SENSOR_DATA_POW2LENGTH);
    previousLeftSpeed = robotLeftSpeed;
    previousRightSpeed = robotRightSpeed;
}

//*********************************************************************
// The robotStart calls this routine before switching to the initial
// delay state.
void blfInit(R4A_ROBOT_CHALLENGE * object)
{
    blfState = BLF_STATE_STOP;
    challengeInit();
}

//*********************************************************************
// Read the line sensors
void blfReadLineSensors()
{
    // Read the line sensors
#ifdef USE_SPARKFUN_SEN_13582
    if (sx1509Present)
        r4aSx1509RegisterRead(&sx1509,
                              SX1509_DATA_A,
                              &lineSensors);
    else
#endif  // USE_SPARKFUN_SEN_13582
    if (pcf8574Present)
    {
        pcf8574.read(&lineSensors);
        lineSensors &= 7;
    }
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
#ifdef USE_SPARKFUN_SEN_13582
    static R4A_ROBOT_CHALLENGE blfSx1509LineFollowing =
    {
        blfSx1509Challenge,
        blfInit,
        blfStart,
        blfStop,

        "Basic Line Following",             // _name
        R4A_CHALLENGE_SEC_LINE_FOLLOWING    // Challenge duration in seconds
    };
#endif  // USE_SPARKFUN_SEN_13582
    R4A_ROBOT_CHALLENGE * challengeStructure;
    const char * const * sensorTable;
    uint8_t sensorMask;
    float voltage;

    // Verify the I2C bus configuration
    if (!pca9685Present)
    {
        display->printf("ERROR: PCA9685 (motors) not responding on I2C bus!\r\n");
        return;
    }
    if ((!sx1509Present) && (!pcf8574Present))
    {
        display->printf("ERROR: PCF8574 (line sensor) not responding on I2C bus!\r\n");
        return;
    }

    // Determine the sensor being used
    challengeStructure = &basicLineFollowing;
    sensorTable = pcf8574SensorTable;
    sensorMask = 7;
#ifdef USE_SPARKFUN_SEN_13582
    if (sx1509Present)
    {
        challengeStructure = &blfSx1509LineFollowing;
        sensorTable = sx1509SensorTable;
        sensorMask = 255;
    }
#endif  // USE_SPARKFUN_SEN_13582

    // Attempt to allocate the log buffer
    if (logInit(blfStateTable,
                BLF_STATE_STOP,
                sensorTable,
                sensorMask) == false)
    {
        display->printf("ERROR: Failed to allocate log buffer!\r\n");
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
                      challengeStructure,
                      robotStartDelaySec,
                      display);
}

//*********************************************************************
// The initial delay routine calls this routine just before calling
// the challenge routine for the first time.
void blfStart(R4A_ROBOT_CHALLENGE * object)
{
    uint32_t currentUsec;

    // Remember the start time
    currentUsec = micros();
    logStartUsec = currentUsec;

    // Read the line sensors
    blfReadLineSensors();

    // Start moving forward
    robotLeftSpeed = 0;
    robotRightSpeed = 0;
    blfState = BLF_STATE_FORWARD;
    challengeStart();

    // Log the sensors
    if (logBuffer)
        logLineSensorData(currentUsec, blfState, LOG_LINE_SENSOR_DATA_POW2LENGTH);
    previousLineSensors = lineSensors;
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
    uint32_t currentUsec;

    // Stop the robot
    currentUsec = micros();
    blfState = BLF_STATE_STOP;
    robotMotorSetSpeeds(0, 0);

    if (logBuffer)
    {
        // Read the line sensors
        blfReadLineSensors();

        // Log the sensors
        logLineSensorData(currentUsec, blfState, LOG_LINE_SENSOR_DATA_POW2LENGTH);
    }

    challengeStop();
}

//*********************************************************************
#ifdef USE_SPARKFUN_SEN_13582
// The robotRunning routine calls this routine to actually perform
// the challenge.  This routine typically reads a sensor and may
// optionally adjust the motors based upon the sensor reading.  The
// routine then must return.  The robot layer will call this routine
// multiple times during the robot operation.
void blfSx1509Challenge(R4A_ROBOT_CHALLENGE * object)
{
    // Read the line sensors
    r4aSx1509RegisterRead(&sx1509,
                          SX1509_DATA_A,
                          &lineSensors);
    if (BLF_DEBUG_STATES)
        Serial.printf("l %d %d %d %d %d %d %d %d r\r\n",
                      (lineSensors & 0x80) ? 1 : 0,
                      (lineSensors & 0x40) ? 1 : 0,
                      (lineSensors & 0x20) ? 1 : 0,
                      (lineSensors & 0x10) ? 1 : 0,
                      (lineSensors & 8) ? 1 : 0,
                      (lineSensors & 4) ? 1 : 0,
                      (lineSensors & 2) ? 1 : 0,
                      lineSensors & 1);

    // Update the robot direction
    switch (lineSensors)
    {
    default:
        Serial.printf("lineSensors: 0x%02x\r\n", lineSensors);

        // Fall through
        //      |
        //      |
        //      V

        // No line or stop circle detected, stop
                // Left                         Right
                //   7   6   5   4   3   2   1   0
    case 0:     //
    case 0x3f:  //           |xxxxxxxxxxxxxxxxxxx|
    case 0x7e:  //       |xxxxxxxxxxxxxxxxxxx|
    case 0xfc:  //   |xxxxxxxxxxxxxxxxxxx|
    case 0x7f:  //       |xxxxxxxxxxxxxxxxxxxxxxx|
    case 0xfe:  //   |xxxxxxxxxxxxxxxxxxxxxxx|
    case 0xff:  //   |xxxxxxxxxxxxxxxxxxxxxxxxxxx|
                //   7   6   5   4   3   2   1   0
        r4aPca9685MotorBrakeAll();
        r4aRobotStop(&robot, millis());
        break;

        // Robot over center of line, continue straight
                // Left                         Right
                //   7   6   5   4   3   2   1   0
    case 0x08:  //                 |XXX|
    case 0x18:  //               |XXX|
    case 0x10:  //             |XXX|
                //   7   6   5   4   3   2   1   0
        robotMotorSetSpeeds(blfSpeedSlow,  blfSpeedSlow); // Move Forward
        break;

        // Robot over left sensor, need to turn left
                // Left                         Right
                //   7   6   5   4   3   2   1   0
    case 0x30:  //           |XXX|
    case 0x20:  //         |XXX|
    case 0x60:  //       |XXX|
    case 0x40:  //     |XXX|
    case 0xc0:  //   |XXX|
    case 0x80:  // |XXX|
    case 0x38:  //           |XXXXXXX|
    case 0x70:  //       |XXXXXXX|
    case 0xe0:  //   |XXXXXXX|
    case 0x78:  //       |XXXXXXXXXXX|
    case 0xf0:  //   |XXXXXXXXXXX|
    case 0xf8:  //   |XXXXXXXXXXXXXXX|
                //   7   6   5   4   3   2   1   0
        robotMotorSetSpeeds(-blfSpeedMedium, blfSpeedFast); // Turn left
        break;

        // Robot over right sensor, need to turn right
                // Left                         Right
                //   7   6   5   4   3   2   1   0
    case 0x01:  //                             |XXX|
    case 0x03:  //                           |XXX|
    case 0x02:  //                         |XXX|
    case 0x06:  //                       |XXX|
    case 0x04:  //                     |XXX|
    case 0x0c:  //                   |XXX|
    case 0x07:  //                       |XXXXXXX|
    case 0x0e:  //                   |XXXXXXX|
    case 0x1c:  //               |XXXXXXX|
    case 0x0f:  //                   |XXXXXXXXXXX|
    case 0x1e:  //               |XXXXXXXXXXX|
    case 0x1f:  //               |XXXXXXXXXXXXXXX|
                //   7   6   5   4   3   2   1   0
        robotMotorSetSpeeds(blfSpeedFast, -blfSpeedMedium); // Turn right
        break;
    }
}
#endif  // USE_SPARKFUN_SEN_13582

