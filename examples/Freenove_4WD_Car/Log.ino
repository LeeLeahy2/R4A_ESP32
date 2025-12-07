/**********************************************************************
  Log.ino

  Support logging to a serial port
**********************************************************************/

//****************************************
// Constants
//****************************************

const size_t logBufferSize = 8192;

#define LOG_STATE_WIDTH         17

#define LOG_DASHES_LENGTH       80
#define LOG_SPACES_LENGTH       80

enum LOG_TYPE
{
    LOG_TYPE_LINE_SENSOR = 0,
};

//****************************************
// New Types
//****************************************

typedef struct _LOG_ENTRY
{
    uint8_t _logType;
    uint8_t _pow2Length;
    uint8_t _state;
    uint8_t _data8;
    uint32_t _microSec;
    uint32_t _loopCount;
    int16_t _leftSpeed;
    int16_t _rightSpeed;
} LOG_ENTRY;

typedef bool (*LOG_PRINT_DATA_ROUTINE)(LOG_ENTRY * logEntry, LOG_ENTRY * logNext);

//****************************************
// Forward routines declarations
//****************************************

bool logLineSensorPrint(LOG_ENTRY * logEntry, LOG_ENTRY * logNext);

//****************************************
// Log data routines
//****************************************

const LOG_PRINT_DATA_ROUTINE logPrintDataRoutine[] =
{
    logLineSensorPrint,
};

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
uint8_t logSensorMask;
const char * const * logSensorTable;
const char ** logStateTable;
uint8_t logStopState;

//*********************************************************************
// Initialize the log buffer
bool logInit(const char ** stateTable,
             uint8_t stopState,
             const char * const * sensorTable,
             uint8_t sensorMask)
{
    // Attempt to allocate the log buffer
    if (stateTable && sensorTable)
    {
        if (logBuffer == nullptr)
            logBuffer = (uint8_t *)r4aMalloc(logBufferSize, "Log Buffer");

        // Save the state table and stop state
        logStateTable = stateTable;
        logSensorTable = sensorTable;
        logSensorMask = sensorMask;
        logStopState = stopState;
        logStartUsec = 0;
        logPrintHeader = true;
        logBufTail = logBuffer;
        logBufHead = logBuffer;
        return (logBuffer != nullptr);
    }
    return false;
}

//*********************************************************************
// Log the line sensor data and robot state
void logLineSensorData(uint32_t currentUsec, uint8_t state, uint8_t pow2Length)
{
    LOG_ENTRY * logEntry;
    LOG_ENTRY * logNext;

    // Verify that logging is running
    if ((logPrint == nullptr)       // Not logging
        || (logBuffer == nullptr)   // No buffer
        || (logBufHead == nullptr)) // No head
        return;

    // Determine the next buffer location
    logEntry = (LOG_ENTRY *)logBufHead;
    if (pow2Length)
        logNext = (LOG_ENTRY *)((uint8_t *)logEntry + (1 << pow2Length));
    else
        logNext = logEntry;
    if (logNext >= (LOG_ENTRY *)&logBuffer[logBufferSize])
        logNext = (LOG_ENTRY *)logBuffer;

    // Verify that there is space in the buffer
    if (logNext == (LOG_ENTRY *)logBufTail)  // Buffer full
    {
        Serial.printf("ERROR: Log buffer full, %d uSec!\r\n", currentUsec);
        return;
    }

    // Add an entry to the log buffer
    logEntry->_logType = LOG_TYPE_LINE_SENSOR;
    logEntry->_pow2Length = pow2Length;
    logEntry->_state = state;
    logEntry->_data8 = lineSensors;
    logEntry->_microSec = currentUsec;
    logEntry->_loopCount = loopCount;
    logEntry->_leftSpeed = robotLeftSpeed;
    logEntry->_rightSpeed = robotRightSpeed;

    // Insert this entry into the log buffer
    logBufHead = (uint8_t *)logNext;
}

//*********************************************************************
// Display the header for the log
void logLineSensorDisplayHeader(int sensorHeaderWidth, Print * display)
{
    const char * const headerLeft = " Elapsed Time     Delta Time   Left  ";
    const char * const headerSensors = "Sensors";
    const char * const headerRight = "  Right   Loops  State";
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
void logLineSensorDisplayHeaderDashes(int sensorHeaderWidth, Print * display)
{
    const char * const dashesLeft = "-------------  -------------  -----  ";
    const char * const dashesRight = "  -----  ------  ";
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

//*********************************************************************
// Display the line sensor log entries
bool logLineSensorPrint(LOG_ENTRY * logEntry, LOG_ENTRY * logNext)
{
    uint32_t deltaSec;
    uint32_t deltaUsec;
    static char line[128];
    uint32_t microseconds;
    static LOG_ENTRY previousEntry;
    uint8_t previousState;
    uint32_t seconds;
    int sensorLength;
    uint8_t sensors;

    // Display the header if necessary
    if (logPrintHeader)
    {
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
        sensorLength = strlen(logSensorTable[0]);
        logLineSensorDisplayHeader(sensorLength, logPrint);
        logLineSensorDisplayHeaderDashes(sensorLength, logPrint);
    }
    else
    {
        // Compute the challenge time and delta time
        microseconds = logEntry->_microSec - logStartUsec;
        seconds = microseconds / (1000 * 1000);
        microseconds -= seconds * 1000 * 1000;

        deltaUsec = logEntry->_microSec - previousEntry._microSec;
        deltaSec = deltaUsec / (1000 * 1000);
        deltaUsec -= deltaSec * 1000 * 1000;

        // Format the log entry
        sensors = previousEntry._data8 & logSensorMask;
        sprintf(line, "%6ld.%06ld, %6ld.%06ld: %5d  %s  %5d %7d  %s\r\n",
                seconds,
                microseconds,
                deltaSec,
                deltaUsec,
                previousEntry._leftSpeed,
                logSensorTable[sensors],
                previousEntry._rightSpeed,
                logEntry->_loopCount - previousEntry._loopCount,
                logStateTable[previousEntry._state]);

        // Display the log entry
        logPrint->printf("%s", line);
    }

    // Save the previous entry
    previousState = previousEntry._state;
    memcpy(&previousEntry, logEntry, sizeof(previousEntry));

    // Log the start entry
    if (logPrintHeader)
    {
        logPrintHeader = false;
        return logPrintData();
    }

    // Done printing the data
    if (logEntry->_state == logStopState)
    {
        // Display the change that caused the stop
        if (previousState != logStopState)
            return logPrintData();

        // Add a header below the log
        sensorLength = strlen(logSensorTable[0]);
        logLineSensorDisplayHeaderDashes(sensorLength, logPrint);
        logLineSensorDisplayHeader(sensorLength, logPrint);
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
        logBufHead = nullptr;
        logBufTail = nullptr;
        logNext = nullptr;
    }

    // Remove this entry from the log buffer
    logBufTail = (uint8_t *)logNext;

    // Successfully output the data
    return true;
}

//*********************************************************************
// Print the log entry
bool logPrintData()
{
    LOG_ENTRY * logEntry;
    LOG_ENTRY * logNext;

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

    // Compute the location of the next log entry
    logEntry = (LOG_ENTRY *)logBufTail;
    if (logEntry->_pow2Length)
        logNext = (LOG_ENTRY *)((uint8_t *)logEntry + (1 << logEntry->_pow2Length));
    else
        logNext = logEntry;
    if (logNext >= (LOG_ENTRY *)&logBuffer[logBufferSize])
        logNext = (LOG_ENTRY *)logBuffer;

    // Print the log entry
    return logPrintDataRoutine[logEntry->_logType](logEntry, logNext);
}
