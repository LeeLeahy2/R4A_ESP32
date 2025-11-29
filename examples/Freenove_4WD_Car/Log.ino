/**********************************************************************
  Log.ino

  Support logging to a serial port
**********************************************************************/

//****************************************
// Constants
//****************************************

const size_t logBufferSize = 256 * sizeof(LOG_ENTRY);

#define LOG_STATE_WIDTH         20

#define LOG_DASHES_LENGTH       80
#define LOG_SPACES_LENGTH       80

//****************************************
// Locals
//****************************************

//                                       1         2         3         4         5         6         7         8
//                              12345678901234567890123456789012345678901234567890123456789012345678901234567890
const char * const logDashes = "--------------------------------------------------------------------------------";
const char * const logSpaces = "                                                                                ";

volatile uint8_t * logBufHead;  // Log entry insertion location
volatile uint8_t * logBufTail;  // Log entry removal location
bool logPrintHeader;
const char ** logStateTable;
uint8_t logStopState;

//*********************************************************************
// Log the change in robot state
void logData(uint32_t currentUsec, uint8_t state)
{
    LOG_ENTRY * logEntry;
    LOG_ENTRY * logNext;

    logEntry = (LOG_ENTRY *)logBufHead;
    logNext = logEntry + 1;
    if (logNext >= (LOG_ENTRY *)&logBuffer[logBufferSize])
        logNext = (LOG_ENTRY *)logBuffer;

    // Verify that logging is running
    if ((logPrint == nullptr)       // Not logging
        || (logBuffer == nullptr)   // No buffer
        || (logBufHead == nullptr)) // No head
        return;

    // Verify that there is space in the buffer
    if (logNext == (LOG_ENTRY *)logBufTail)  // Buffer full
    {
        Serial.printf("ERROR: Log buffer full, %d uSec!\r\n", currentUsec);
        return;
    }

    // Add an entry to the log buffer
    logEntry->_microSec = currentUsec;
    logEntry->_leftSpeed = robotLeftSpeed;
    logEntry->_rightSpeed = robotRightSpeed;
    logEntry->_state = state;
    logEntry->_lineSensors = lineSensors;
    logEntry->_reserved = 0;

    // Insert this entry into the log buffer
    logBufHead = (uint8_t *)logNext;
}

//*********************************************************************
// Initialize the log buffer
bool logInit(const char ** stateTable, uint8_t stopState)
{
    // Attempt to allocate the log buffer
    if (stateTable)
    {
        if (logBuffer == nullptr)
            logBuffer = (uint8_t *)r4aMalloc(logBufferSize, "Log Buffer");

        // Save the state table and stop state
        logStateTable = stateTable;
        logStopState = stopState;
        logStartUsec = 0;
        logPrintHeader = true;
        logBufHead = logBuffer;
        logBufTail = logBuffer;
        return (logBuffer != nullptr);
    }
    return false;
}

//*********************************************************************
// Display the robot state change
bool logDataPrint()
{
    uint32_t deltaSec;
    uint32_t deltaUsec;
    static char line[128];
    LOG_ENTRY * logEntry;
    LOG_ENTRY * logNext;
    uint32_t microseconds;
    static uint32_t previousUsec;
    uint32_t seconds;
    int sensorLength;
    const char * sensorTable[8] =
    {
        ".       .", // 0
        ".     |x|", // 1
        ".  |x|  .", // 2
        ".  |xxxx|", // 3
        "|x|     .", // 4
        "|x|   |x|", // 5
        "|xxxx|  .", // 6
        "|xxxxxxx|", // 7
    };

    logEntry = (LOG_ENTRY *)logBufTail;
    logNext = logEntry + 1;
    if (logNext >= (LOG_ENTRY *)&logBuffer[logBufferSize])
        logNext = (LOG_ENTRY *)logBuffer;

    // Verify that logging is enabled
    if ((logPrint == nullptr)           // No output device
        || (logBuffer == nullptr)       // No buffer
        || (logBufHead == nullptr)      // No head
        || (logBufTail == nullptr))     // No tail
    {
        return false;
    }

    // Determine if the buffer is empty
    if (logBufHead == logBufTail)   // Empty list
        return false;

    // Display the header if necessary
    if (logPrintHeader)
    {
        logPrintHeader = false;
        logPrint->printf("\r\n");
        logPrint->printf("--------------------------------------------------------------------------------\r\n");
        logPrint->printf("%s\r\n", robot._challenge->_name);

        // Display the current time
        if (ntpEnable && r4aNtpIsTimeValid())
            r4aNtpDisplayDateTime(logPrint);

        // Display the WIFI status
        const char * hostName = r4aWifiHostName;
        if (hostName)
            logPrint->printf("%s (%s): %s channel %d\r\n",
                             hostName,
                             WiFi.localIP().toString().c_str(),
                             r4aWifiStationSsid(),
                             r4aWifiChannel);
        else
            logPrint->printf("%s: %s channel %d\r\n",
                             WiFi.localIP().toString().c_str(),
                             r4aWifiStationSsid(),
                             r4aWifiChannel);

        // Display the battery voltage
        DISPLAY_BATTERY_VOLTAGE(logPrint);

        // Finish displaying the log header
        logPrint->printf("--------------------------------------------------------------------------------\r\n");
        logPrint->printf("\r\n");
        sensorLength = strlen(sensorTable[0]);
        logDisplayHeader(sensorLength, logPrint);
        logDisplayHeaderDashes(sensorLength, logPrint);

        // Set the start time
        previousUsec = logEntry->_microSec;
    }

    // Compute the challenge time and delta time
    microseconds = logEntry->_microSec - logStartUsec;
    seconds = microseconds / (1000 * 1000);
    microseconds -= seconds * 1000 * 1000;

    deltaUsec = logEntry->_microSec - previousUsec;
    deltaSec = deltaUsec / (1000 * 1000);
    deltaUsec -= deltaSec * 1000 * 1000;

    // Format the log entry
    sprintf(line, "%6ld.%06ld, %6ld.%06ld: %5d  %s  %5d  %s\r\n",
            seconds,
            microseconds,
            deltaSec,
            deltaUsec,
            logEntry->_leftSpeed,
            sensorTable[logEntry->_lineSensors],
            logEntry->_rightSpeed,
            logStateTable[logEntry->_state]);
    previousUsec = logEntry->_microSec;

    // Display the log entry
    logPrint->printf("%s", line);

    // Remove this entry from the log buffer
    logBufTail = (uint8_t *)logNext;

    // Done printing the data
    if (logEntry->_state == logStopState)
    {
        sensorLength = strlen(sensorTable[0]);
        logDisplayHeaderDashes(sensorLength, logPrint);
        logDisplayHeader(sensorLength, logPrint);
        logPrint->printf("\r\n");

        // Display the loop times
        loopTimesMenu(nullptr, nullptr, logPrint);
        logPrint->printf("\r\n");

        // Display the parameters
        logPrint->printf("Parameters\r\n");
        logPrint->printf("----------\r\n");
        r4aEsp32NvmDisplayParameters(nvmParameters, nvmParameterCount, logPrint);
        logPrint->printf("\r\n");

        // Display the current time
        if (ntpEnable && r4aNtpIsTimeValid())
        {
            logPrint->printf("--------------------------------------------------------------------------------\r\n");
            r4aNtpDisplayDateTime(logPrint);
            logPrint->printf("--------------------------------------------------------------------------------\r\n");
            logPrint->printf("\r\n");
        }

        // Disable further logging
        logPrint = nullptr;
    }
    return true;
}

//*********************************************************************
// Display the header for the log
void logDisplayHeader(int sensorHeaderWidth, Print * display)
{
    const char * const headerLeft = " Elapsed Time     Delta Time   Left  ";
    const char * const headerSensors = "Sensors";
    const char * const headerRight = "  Right  State\r\n";
    int sensorLength;
    int spacesLeft;
    int spacesRight;

    // Determine left and right spaces
    sensorLength = strlen(headerSensors);
    spacesLeft = LOG_SPACES_LENGTH - ((sensorHeaderWidth - sensorLength) >> 1);
    spacesRight = LOG_SPACES_LENGTH - ((sensorHeaderWidth - sensorLength + 1) >> 1);

    // Display the header
    display->printf("%s%s%s%s%s\r\n",
                    headerLeft,
                    &logSpaces[spacesLeft],
                    headerSensors,
                    &logSpaces[spacesRight],
                    headerRight);
}

//*********************************************************************
// Display the dashes for the log header
void logDisplayHeaderDashes(int sensorHeaderWidth, Print * display)
{
    const char * const dashesLeft = "-------------  -------------  -----  ";
    const char * const dashesRight = "  -----  ";
    int dashesSensors;
    int dashesState;

    // Determine sensors and state dashes
    dashesSensors = LOG_DASHES_LENGTH - sensorHeaderWidth;
    dashesState = LOG_DASHES_LENGTH - LOG_STATE_WIDTH;
    display->printf("%s%s%s%s\r\n",
                    dashesLeft,
                    &logDashes[dashesSensors],
                    dashesRight,
                    &logDashes[dashesState]);
}
