/**********************************************************************
  Advanced_Line_Following.ino

  Advanced line following support
**********************************************************************/

//****************************************
// Constants
//****************************************

enum ALF_STATE
{
    ALF_STATE_STOP = 0,
    ALF_STATE_FORWARD,
};

const char * alfStateTable[] =
{
    "Stop",
    "Forward",
};

typedef void (* ALF_STATE_UPDATE)();

// Forward routine declarations
void alfForward();
void alfStop();

const ALF_STATE_UPDATE alfStateRoutine[] =
{
    alfStop,
    alfForward,
};

//****************************************
// Locals
//****************************************

static volatile uint8_t alfState;

//*********************************************************************
// Do the line following
void alfChallenge(R4A_ROBOT_CHALLENGE * object)
{
    uint32_t currentUsec;

    // Read the line sensors
    currentUsec = micros();
    pcf8574.read(&lineSensors);
    lineSensors &= 7;

    // Determine if the line sensor value has changed
    if (previousLineSensors != lineSensors)
    {
        previousLineSensors = lineSensors;

        // Process the sensors
        alfStateRoutine[alfState]();

        // Log the sensors
        if (logBuffer)
            logData(currentUsec, alfState);
    }
}

//*********************************************************************
// Robot moving forward
void alfForward()
{
    // Adjust the motors
    switch (lineSensors)
    {
    //     RcL
    case 0b000:
    case 0b001:
    case 0b010:
    case 0b011:
    case 0b100:
    case 0b101:
    case 0b110:
    case 0b111: // Crossing a 90 degree turn, a T, or the circle, continue straight
        challengeHalt("Code missing in alfChallenge, reads sensors and make decisions to drive motors!");
        break;
    }
}

//*********************************************************************
// The robotStart calls this routine before switching to the initial
// delay state.
void alfInit(R4A_ROBOT_CHALLENGE * object)
{
    alfState = ALF_STATE_STOP;
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
    uint8_t sensorMask;
    const char * const * sensorTable;
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

    // Determine the sensor being used
    sensorTable = pcf8574SensorTable;
    sensorMask = 7;

    // Attempt to allocate the log buffer
    if (logInit(alfStateTable,
                ALF_STATE_STOP,
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
                      &advancedLineFollowing,
                      robotStartDelaySec,
                      display);
}

//*********************************************************************
// The initial delay routine calls this routine just before calling
// the challenge routine for the first time.
void alfStart(R4A_ROBOT_CHALLENGE * object)
{
    uint32_t currentUsec;

    // Remember the start time
    currentUsec = micros();
    logStartUsec = currentUsec;

    // Read the line sensors
    pcf8574.read(&lineSensors);
    lineSensors &= 7;

    // Start moving forward
    robotMotorSetSpeeds(alfSpeedForward, alfSpeedForward);
    alfState = ALF_STATE_FORWARD;
    challengeStart();

    // Log the sensors
    if (logBuffer)
        logData(currentUsec, alfState);
    previousLineSensors = lineSensors;
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
    uint32_t currentUsec;

    // Stop the robot
    currentUsec = micros();
    alfState = ALF_STATE_STOP;
    robotMotorSetSpeeds(0, 0);

    if (logBuffer)
    {
        // Read the line sensors
        pcf8574.read(&lineSensors);
        lineSensors &= 7;

        // Log the sensors
        logData(currentUsec, alfState);
    }

    // Turn on the brake lights
    challengeStop();
}

//*********************************************************************
// Stop the robot
void alfStop()
{
    // Stop the robot
    alfState = ALF_STATE_STOP;
    r4aRobotStop(&robot, millis());
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
