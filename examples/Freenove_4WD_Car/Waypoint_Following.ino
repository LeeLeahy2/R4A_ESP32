/**********************************************************************
  Waypoint_Following.ino

  Waypoint following support
**********************************************************************/

#ifdef  USE_WAYPOINT_FOLLOWING
#ifdef  USE_ZED_F9P

//*********************************************************************
// Implement the basic light tracking class
class WPF : public R4A_ROBOT_CHALLENGE
{
  private:

    double _altitude;
    String _comment;
    double _horizontalAccuracy;
    double _horizontalAccuracyStdDev;
    double _latitude;
    double _longitude;
    double _previousLat;
    uint32_t _previousLogMsec;
    double _previousLong;
    uint8_t _satellitesInView;
    int _wpCount;
    File _wpFile;
    File _wpLogFile;

  public:

    // Constructor
    WPF(const char * name);

    // The robotStart routine calls this routine to verify the battery
    // level.
    // Outputs:
    //   Returns true if the battery level is high enough to run the robot
    //   challenge and returns false if the battery needs to be changed.
    bool batteryLevel();

    // The robotRunning routine calls this routine to actually perform
    // the challenge.  This routine typically reads a sensor and may
    // optionally adjust the motors based upon the sensor reading.  The
    // routine then must return.  The robot layer will call this routine
    // multiple times during the robot operation.
    void challenge();

    // Get the next waypoint
    // Outputs:
    //   Returns true if another waypoint is available
    bool getWaypoint();

    // The robotStart calls this routine before switching to the initial
    // delay state.
    void init();

    // Place the current position into the log file
    void logCurrentPosition();

    // Place a waypoint into the log file
    void logWayPoint();

    // The initial delay routine calls this routine just before calling
    // the challenge routine for the first time.
    void start();

    // The robot.stop routine calls this routine to stop the motors and
    // perform any other actions.
    void stop();

    // Start the waypoint following
    void wpfStart(const struct _R4A_MENU_ENTRY * menuEntry,
                  const char * command,
                  Print * display);
};

//*********************************************************************
// Constructor
WPF::WPF(const char * name)
    : R4A_ROBOT_CHALLENGE(name, ROBOT_LIGHT_TRACKING_DURATION_SEC)
{
}

//*********************************************************************
// The robotStart routine calls this routine to verify the battery
// level.
// Outputs:
//   Returns true if the battery level is high enough to run the robot
//   challenge and returns false if the battery needs to be changed.
bool WPF::batteryLevel()
{
    return robotCheckBatteryLevel();
}

//*********************************************************************
// The robotRunning routine calls this routine to actually perform
// the challenge.  This routine typically reads a sensor and may
// optionally adjust the motors based upon the sensor reading.  The
// routine then must return.  The robot layer will call this routine
// multiple times during the robot operation.
void WPF::challenge()
{
    // Determine if the next waypoint was reached
    if (r4aWaypointReached(_latitude, _longitude, zedf9p._latitude, zedf9p._longitude))
    {
        _wpCount += 1;
        if (!getWaypoint())
        {
            robot.stop(millis());
            return;
        }
    }

    // Process the current waypoint (_latitude, _longitude)

    static uint32_t lastDisplayMsec;
    if ((millis() - lastDisplayMsec) >= (15 * 1000))
    {
        lastDisplayMsec = millis();
        Serial.printf("Add your code to WLF::challenge()\r\n");
    }
}

//*********************************************************************
// Get the next waypoint
// Outputs:
//   Returns true if another waypoint is available
bool WPF::getWaypoint()
{
    bool wayPointAvailable;
    wayPointAvailable = r4aEsp32WpReadPoint(&_wpFile,
                                            &_latitude,
                                            &_longitude,
                                            &_altitude,
                                            &_horizontalAccuracy,
                                            &_horizontalAccuracyStdDev,
                                            &_satellitesInView,
                                            &_comment,
                                            (Print *)&_wpLogFile);
    if (wayPointAvailable)
        logWayPoint();
    else
        // Error or end-of-file, stop the robot
        robot.stop(millis(), (Print *)&_wpLogFile);
    return wayPointAvailable;
}

//*********************************************************************
// The robotStart calls this routine before switching to the initial
// delay state.
void WPF::init()
{
    challengeInit();

    // Get the initial waypoint
    _wpCount = 0;
    if (r4aEsp32WpReadPoint(&_wpFile,
                            &_latitude,
                            &_longitude,
                            &_altitude,
                            &_horizontalAccuracy,
                            &_horizontalAccuracyStdDev,
                            &_satellitesInView,
                            &_comment,
                            (Print *)&_wpLogFile) == false)
    {
        // Failed to read the initial waypoint, stop the robot
        robot.stop(millis(), (Print *)&_wpLogFile);
        return;
    }

    // Log the current position
    _previousLogMsec = 0;
    logCurrentPosition();
}

//*********************************************************************
// Place the current position into the log file
void WPF::logCurrentPosition()
{
    const char * format  = "%2d:%02d:%02d   %14.9f   %14.9f   %9.3f   %3d\r\n";
    //                      12:12:12   -123.123456789   -123.123456789   12345.123   123

    // Save the current position
    _previousLogMsec = millis();
    _previousLat = zedf9p._latitude;
    _previousLong = zedf9p._longitude;

    // Add the waypoint to the log file
    _wpLogFile.printf(format,
                      zedf9p._hour,
                      zedf9p._minute,
                      zedf9p._second,
                      _previousLat,
                      _previousLong,
                      zedf9p._horizontalAccuracy,
                      zedf9p._satellitesInView);
}

//*********************************************************************
// Place a waypoint into the log file
void WPF::logWayPoint()
{
    const char * format  = "%2d:%02d:%02d   %14.9f   %14.9f   %9.3f   %3d   %10.3f   %9.3f   %3d   %s\r\n";
    //                      12:12:12   -123.123456789   -123.123456789   12345.123   123   123456.123   12345.123   123   1234567
    const char * header1 = "  Time       Latitude         Longitude      Horiz Acc   SIV     Altitude   Std. Dev.   Num   Comment\r\n";
    const char * header2 = "--------   --------------   --------------   ---------   ---   ----------   ---------   ---   -------\r\n";

    // Add the header to the log file
    _wpLogFile.printf("%s", header1);
    _wpLogFile.printf("%s", header2);

    // Add the waypoint to the log file
    _wpLogFile.printf(format,
                      zedf9p._hour,
                      zedf9p._minute,
                      zedf9p._second,
                      _latitude,
                      _longitude,
                      _horizontalAccuracy,
                      _satellitesInView,
                      _altitude,
                      _horizontalAccuracyStdDev,
                      _wpCount + 1,
                      _comment.c_str());
}

//*********************************************************************
// The initial delay routine calls this routine just before calling
// the challenge routine for the first time.
void WPF::start()
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
void WPF::stop()
{
    challengeStop();
}

//*********************************************************************
// Start the waypoint following
void WPF::wpfStart(const struct _R4A_MENU_ENTRY * menuEntry,
              const char * command,
              Print * display)
{
    String filePath;
    String logFilePath;
    const char * logPath;
    const char * path;
    float voltage;

    // Only start the robot if the battery is on
    if (!robotCheckBatteryLevel())
    {
        voltage = READ_BATTERY_VOLTAGE(nullptr);
        display->printf("Battery: %f4.2V\r\n", voltage);
        display->printf("WARNING: Battery voltage to run the robot!\r\n");
    }

    // Verify the waypoint file path
    filePath = String("/") + String(r4aEsp32WpFileName);
    path = filePath.c_str();
    logFilePath = String("/") + String(wpLogFileName);
    logPath = logFilePath.c_str();
    _wpLogFile = LittleFS.open(logPath, FILE_WRITE);
    if (!_wpLogFile)
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
        _wpLogFile.printf("Date: %4d:%02d:%02d",
                          zedf9p._year,
                          zedf9p._month,
                          zedf9p._day);

        // Display the file name
        display->printf("Waypoint file: %s\r\n", r4aEsp32WpFileName);
        _wpLogFile.printf("Waypoint file: %s\r\n", r4aEsp32WpFileName);
        display->println();
        _wpLogFile.println();

        // Display the waypoint file header
        display->printf("%s", header1);
        _wpLogFile.printf("%s", header1);
        display->printf("%s", header2);
        _wpLogFile.printf("%s", header2);

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
            _wpLogFile.printf(format,
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
        _wpLogFile.println();

        // Start the robot challenge if the robot is not active
        robot.init(this,
                   ROBOT_WAYPOINT_FOLLOW_DURATION_SEC,
                   display);
    }
}

//*********************************************************************
// Start the waypoint following
void wpfStart(const struct _R4A_MENU_ENTRY * menuEntry,
              const char * command,
              Print * display)
{
    static WPF wayPointFollowing("Waypoint Following");

    wayPointFollowing.wpfStart(menuEntry, command, display);
}

#endif  // USE_ZED_F9P
#endif  // USE_WAYPOINT_FOLLOWING
