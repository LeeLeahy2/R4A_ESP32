/**********************************************************************
  Log.ino

  Support logging to a serial port
**********************************************************************/

//****************************************
// Constants
//****************************************

const size_t logBufferSize = 256 * sizeof(LOG_ENTRY);

//****************************************
// Locals
//****************************************

volatile uint8_t * logBufHead;  // Log entry insertion location
volatile uint8_t * logBufTail;  // Log entry removal location
uint32_t logPreviousUsec;
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
void logInit(const char ** stateTable, uint8_t stopState)
{
    // Attempt to allocate the log buffer
    if (stateTable && (logBuffer == nullptr))
    {
        logBuffer = (uint8_t *)r4aMalloc(logBufferSize, "Log Buffer");
        if (logBuffer)
        {
            logBufHead = logBuffer;
            logBufTail = logBuffer;
        }

        // Save the state table and stop state
        logStateTable = stateTable;
        logStopState = stopState;
        logStartUsec = 0;
        logPreviousUsec = 0;
    }
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
    uint32_t seconds;
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

    if (logPrintHeader)
    {
        logPrintHeader = false;

        // Set the start time
        logPreviousUsec = logEntry->_microSec;
    }

    // Compute the challenge time and delta time
    microseconds = logEntry->_microSec - logStartUsec;
    seconds = microseconds / (1000 * 1000);
    microseconds -= seconds * 1000 * 1000;

    deltaUsec = logEntry->_microSec - logPreviousUsec;
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
    logPreviousUsec = logEntry->_microSec;

    // Display the log entry
    logPrint->printf("%s", line);

    // Remove this entry from the log buffer
    logBufTail = (uint8_t *)logNext;

    // Done printing the data
    if (logEntry->_state == logStopState)
    {
        logPrint->printf("\r\n");

        // Display the parameters
        logPrint->printf("Parameters\r\n");
        logPrint->printf("----------\r\n");
        r4aEsp32NvmDisplayParameters(nvmParameters, nvmParameterCount, logPrint);
        logPrint->printf("\r\n");

        // Disable further logging
        logPrint = nullptr;
    }
    return true;
}
