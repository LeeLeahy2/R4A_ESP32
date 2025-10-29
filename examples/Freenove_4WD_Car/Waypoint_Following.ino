/**********************************************************************
  Waypoint_Following.ino

  Waypoint following support
**********************************************************************/

#ifdef  USE_WAYPOINT_FOLLOWING

//****************************************
// Types
//****************************************

typedef struct _WAYPOINT_FOLLOWING
{
    double _altitude; // Altitude in meters
    String _comment;
    double _horizontalAccuracy; // Horizontal accuracy in meters
    double _horizontalAccuracyStdDev; // Horizontal accuracy standard deviation in meters
    double _latitude; // Latitude in degrees (-90 to 90)
    double _longitude; // Longitude in degrees (-180 to 180)
    double _previousLat; // Previous latitude in degrees (-90 to 90)
    uint32_t _previousLogMsec;
    double _previousLong; // Previous longitude in degrees (-180 to 180)
    uint8_t _satellitesInView; // Number of satellites in view
    int _wpCount; // Number of waypoints
    File _wpFile; // Waypoint file
    File _wpLogFile; // Waypoint log file
} WAYPOINT_FOLLOWING;

//****************************************
// Locals
//****************************************

static WAYPOINT_FOLLOWING wpfData;

//*********************************************************************
// Start the waypoint following at boot
void menuStartWpf(const struct _R4A_MENU_ENTRY * menuEntry,
                  const char * command,
                  Print * display)
{
    startIndex = CHALLENGE_WPF;
    r4aEsp32NvmMenuParameterFileWrite(menuEntry, command, display);
}

//*********************************************************************
// Display the heading
// Inputs:
//   menuEntry: Address of the object describing the menu entry
//   command: Zero terminated command string
//   display: Device used for output
void menuWpfDisplayHeading(const R4A_MENU_ENTRY * menuEntry,
                           const char * command,
                           Print * display)
{
    double altitude;
    String comment;
    double deltaLatitude;
    double deltaLongitude;
    File file;
    double horizontalAccuracy;
    double horizontalAccuracyStdDev;
    double latitude;
    double longitude;
    uint8_t satellitesInView;

    // Determine if a waypoint was set
    if (wpfData._latitude || wpfData._longitude)
    {
        latitude = wpfData._latitude;
        longitude = wpfData._longitude;
    }
    else
    {
        // No, get the first waypoint
        if (r4aEsp32WpReadPoint(&file,
                                &latitude,
                                &longitude,
                                &altitude,
                                &horizontalAccuracy,
                                &horizontalAccuracyStdDev,
                                &satellitesInView,
                                &comment,
                                display))

        // Done with the file
        file.close();
    }

    // Compute the direction from the current location to the waypoint
    deltaLatitude = latitude - zedf9p._latitude;
    deltaLongitude = longitude - zedf9p._longitude;

    // Display the heading
    display->printf("   Latitude         Longitude\r\n");
    display->printf("--------------   --------------\r\n");
    display->printf("%14.9f   %14.9f   Target Waypoint\r\n",
                    latitude, longitude);
    display->printf("%14.9f   %14.9f   Current Position\r\n",
                    zedf9p._latitude, zedf9p._longitude);
    display->printf("%14.9f   %14.9f   Heading\r\n",
                    deltaLatitude, deltaLongitude);
}

//*********************************************************************
// The robotRunning routine calls this routine to actually perform
// the challenge.  This routine typically reads a sensor and may
// optionally adjust the motors based upon the sensor reading.  The
// routine then must return.  The robot layer will call this routine
// multiple times during the robot operation.
void wpfChallenge(R4A_ROBOT_CHALLENGE * object)
{
    static uint32_t lastDisplayMsec;
    _R4A_LAT_LONG_POINT_PAIR location;

    // Use the destination waypoint as the current value
    location.current.latitude = wpfData._latitude;
    location.current.longitude = wpfData._latitude;

    // Use the current location as the previous value
    location.previous.latitude = zedf9p._latitude;
    location.previous.longitude = zedf9p._longitude;

    // Determine if the next waypoint was reached
    if (r4aWaypointReached(&location))
    {
        wpfData._wpCount += 1;
        if (!wpfGetWaypoint())
        {
            r4aRobotStop(&robot, millis(), (Print *)&wpfData._wpLogFile);
            return;
        }
    }

    // Process the current waypoint (_latitude, _longitude)
    challengeHalt("Code missing in wpfChallenge, reads sensors and make decisions to drive motors!");
}

//*********************************************************************
// Get the next waypoint
// Outputs:
//   Returns true if another waypoint is available
bool wpfGetWaypoint()
{
    bool wayPointAvailable;
    wayPointAvailable = r4aEsp32WpReadPoint(&wpfData._wpFile,
                                            &wpfData._latitude,
                                            &wpfData._longitude,
                                            &wpfData._altitude,
                                            &wpfData._horizontalAccuracy,
                                            &wpfData._horizontalAccuracyStdDev,
                                            &wpfData._satellitesInView,
                                            &wpfData._comment,
                                            (Print *)&wpfData._wpLogFile);
    if (wayPointAvailable)
        wpfLogWayPoint();
    else
        // Error or end-of-file, stop the robot
        r4aRobotStop(&robot, millis(), (Print *)&wpfData._wpLogFile);
    return wayPointAvailable;
}

//*********************************************************************
// The robotStart calls this routine before switching to the initial
// delay state.
// Inputs:
//   object: Address of an R4A_ROBOT_CHALLENGE instance
void wpfInit(R4A_ROBOT_CHALLENGE * object)
{
    challengeInit();

    // Get the initial waypoint
    wpfData._wpCount = 0;
    if (r4aEsp32WpReadPoint(&wpfData._wpFile,
                            &wpfData._latitude,
                            &wpfData._longitude,
                            &wpfData._altitude,
                            &wpfData._horizontalAccuracy,
                            &wpfData._horizontalAccuracyStdDev,
                            &wpfData._satellitesInView,
                            &wpfData._comment,
                            (Print *)&wpfData._wpLogFile) == false)
    {
        // Failed to read the initial waypoint, stop the robot
        r4aRobotStop(&robot, millis(), (Print *)&wpfData._wpLogFile);
        return;
    }

    // Log the current position
    wpfData._previousLogMsec = 0;
    wpfLogCurrentPosition();
}

//*********************************************************************
// Place the current position into the log file
void wpfLogCurrentPosition()
{
    const char * format  = "%2d:%02d:%02d   %14.9f   %14.9f   %9.3f   %3d\r\n";
    //                      12:12:12   -123.123456789   -123.123456789   12345.123   123

    // Save the current position
    wpfData._previousLogMsec = millis();
    wpfData._previousLat = zedf9p._latitude;
    wpfData._previousLong = zedf9p._longitude;

    // Add the waypoint to the log file
    wpfData._wpLogFile.printf(format,
                              zedf9p._hour,
                              zedf9p._minute,
                              zedf9p._second,
                              wpfData._previousLat,
                              wpfData._previousLong,
                              zedf9p._horizontalAccuracy,
                              zedf9p._satellitesInView);
}

//*********************************************************************
// Place a waypoint into the log file
void wpfLogWayPoint()
{
    const char * format  = "%2d:%02d:%02d   %14.9f   %14.9f   %9.3f   %3d   %10.3f   %9.3f   %3d   %s\r\n";
    //                      12:12:12   -123.123456789   -123.123456789   12345.123   123   123456.123   12345.123   123   1234567
    const char * header1 = "  Time       Latitude         Longitude      Horiz Acc   SIV     Altitude   Std. Dev.   Num   Comment\r\n";
    const char * header2 = "--------   --------------   --------------   ---------   ---   ----------   ---------   ---   -------\r\n";

    // Add the header to the log file
    wpfData._wpLogFile.printf("%s", header1);
    wpfData._wpLogFile.printf("%s", header2);

    // Add the waypoint to the log file
    wpfData._wpLogFile.printf(format,
                              zedf9p._hour,
                              zedf9p._minute,
                              zedf9p._second,
                              wpfData._latitude,
                              wpfData._longitude,
                              wpfData._horizontalAccuracy,
                              wpfData._satellitesInView,
                              wpfData._altitude,
                              wpfData._horizontalAccuracyStdDev,
                              wpfData._wpCount + 1,
                              wpfData._comment.c_str());
}

//*********************************************************************
// Start the waypoint following
// Inputs:
//   display: Device used for output
void wpfStart(Print * display)
{
    String filePath;
    String logFilePath;
    const char * logPath;
    const char * path;
    float voltage;
    static R4A_ROBOT_CHALLENGE wayPointFollowing =
    {
        wpfChallenge,
        wpfInit,
        wpfStart,
        wpfStop,
        "Waypoint Following",                   // _name
        R4A_CHALLENGE_SEC_WAYPOINT_FOLLOWING,   // Challenge duration in seconds
    };

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
    if (!zedf9pPresent)
    {
        display->printf("ERROR: ZED-F9P (GNSS receiver) not responding on I2C bus!\r\n");
        return;
    }

    // Only start the robot if the battery is on
    if (!robotCheckBatteryLevel())
    {
        voltage = READ_BATTERY_VOLTAGE(nullptr);
        display->printf("Battery: %f4.2V\r\n", voltage);
        display->printf("WARNING: Battery voltage to run the robot!\r\n");
    }
    else
    {
        // Verify the waypoint file path
        filePath = String("/") + String(r4aEsp32WpFileName);
        path = filePath.c_str();
        logFilePath = String("/") + String(wpLogFileName);
        logPath = logFilePath.c_str();
        wpfData._wpLogFile = LittleFS.open(logPath, FILE_WRITE);
        if (!wpfData._wpLogFile)
            display->printf("ERROR: Failed to open the log file %s!\r\n", wpLogFileName);
        else if (LittleFS.exists(path) == false)
            display->printf("ERROR: Waypoint file %s does not exist!\r\n", r4aEsp32WpFileName);
        else
        {
            double altitude;
            String comment;
            File file;
            double horizontalAccuracy;
            double horizontalAccuracyStdDev;
            double latitude;
            double longitude;
            uint8_t satellitesInView;
            const char * format  = "%3d   %14.9f   %14.9f   %10.3f   %9.3f   %9.3f   %3d   %s\r\n";
            //                      123   -123.123456789   -123.123456789   123456.123   12345.123   12345.123   123   1234567
            const char * header1 = "Num     Latitude         Longitude       Altitude    Horiz Acc   Std. Dev.   SIV   Comment\r\n";
            const char * header2 = "---   --------------   --------------   ----------   ---------   ---------   ---   -------\r\n";

            // Display the date
            display->printf("Date: %4d:%02d:%02d",
                            zedf9p._year,
                            zedf9p._month,
                            zedf9p._day);
            wpfData._wpLogFile.printf("Date: %4d:%02d:%02d",
                                      zedf9p._year,
                                      zedf9p._month,
                                      zedf9p._day);

            // Display the file name
            display->printf("Waypoint file: %s\r\n", r4aEsp32WpFileName);
            wpfData._wpLogFile.printf("Waypoint file: %s\r\n", r4aEsp32WpFileName);
            display->println();
            wpfData._wpLogFile.println();

            // Display the waypoint file header
            display->printf("%s", header1);
            wpfData._wpLogFile.printf("%s", header1);
            display->printf("%s", header2);
            wpfData._wpLogFile.printf("%s", header2);

            // Display the waypoints
            for (int count = 1; ; count++)
            {
                if (r4aEsp32WpReadPoint(&file,
                                        &latitude,
                                        &longitude,
                                        &altitude,
                                        &horizontalAccuracy,
                                        &horizontalAccuracyStdDev,
                                        &satellitesInView,
                                        &comment,
                                        display) == false)
                    break;
                display->printf(format,
                                count,
                                latitude,
                                longitude,
                                altitude,
                                horizontalAccuracy,
                                horizontalAccuracyStdDev,
                                satellitesInView,
                                comment.c_str());
                wpfData._wpLogFile.printf(format,
                                          count,
                                          latitude,
                                          longitude,
                                          altitude,
                                          horizontalAccuracy,
                                          horizontalAccuracyStdDev,
                                          satellitesInView,
                                          comment.c_str());
            }
            display->println();
            wpfData._wpLogFile.println();

            // Start the robot challenge if the robot is not active
            r4aRobotStart(&robot,
                          &wayPointFollowing,
                          display);
        }
    }
}

//*********************************************************************
// The initial delay routine calls this routine just before calling
// the challenge routine for the first time.
// Inputs:
//   object: Address of an R4A_ROBOT_CHALLENGE instance
void wpfStart(R4A_ROBOT_CHALLENGE * object)
{
    challengeStart();
}

//*********************************************************************
// Start the line following
void wpfStartMenu(const struct _R4A_MENU_ENTRY * menuEntry,
                  const char * command,
                  Print * display)
{
    wpfStart(display);
}

//*********************************************************************
// The robot.stop routine calls this routine to stop the motors and
// perform any other actions.
// Inputs:
//   object: Address of an R4A_ROBOT_CHALLENGE instance
void wpfStop(R4A_ROBOT_CHALLENGE * object)
{
    challengeStop();
}

#endif  // USE_WAYPOINT_FOLLOWING
