/**********************************************************************
  02_Bluetooth_Output.ino

  Perform basic line following with Bluetooth output
**********************************************************************/

#include <R4A_Freenove_4WD_Car.h>   // Freenove 4WD Car configuration

//****************************************
// Constants
//****************************************

#define BLUETOOTH_NAME      "Robot"

#define BLF_DEBUG_LINE_SENSORS      0

#define BLF_SPEED_LV4   4000
#define BLF_SPEED_LV3   3000
#define BLF_SPEED_LV1   1500

//****************************************
// I2C bus configuration
//****************************************

USE_I2C_DEVICE_TABLE;
USE_I2C_BUS_TABLE;

R4A_I2C_BUS * r4aI2cBus; // I2C bus for menu system

//****************************************
// Forward routine declarations
//****************************************

void blfChallenge(R4A_ROBOT_CHALLENGE * object);
void blfDisplayTime(uint32_t deltaMsec);
void blfIdle(uint32_t mSecToStart);
void blfInit(struct _R4A_ROBOT_CHALLENGE * object);
void blfStop(R4A_ROBOT_CHALLENGE * object);
void blfStart(struct _R4A_ROBOT_CHALLENGE * object);
bool robotMotorSetSpeeds(int16_t left, int16_t right, Print * display = nullptr);

//****************************************
// Locals
//****************************************

bool btFailed;
bool idleDisplayed;
uint32_t lastDisplayMsec;
uint32_t startMsec;

USE_MOTOR_TABLE;

#define ROBOT_STOP_TO_IDLE_SEC              30

R4A_ROBOT robot;

R4A_ROBOT_CHALLENGE basicLineFollowing =
{
    blfChallenge,
    blfInit,
    blfStart,
    blfStop,

    "Basic Line Following",             // _name
    R4A_CHALLENGE_SEC_LINE_FOLLOWING    // Challenge duration in seconds
};

//*********************************************************************
// The robotRunning routine calls this routine to actually perform
// the challenge.  This routine typically reads a sensor and may
// optionally adjust the motors based upon the sensor reading.  The
// routine then must return.  The robot layer will call this routine
// multiple times during the robot operation.
void blfChallenge(R4A_ROBOT_CHALLENGE * object)
{
    uint32_t currentMsec;
    uint8_t lineSensors;

    // Display the time
    currentMsec = millis();
    if ((currentMsec - lastDisplayMsec) >= 100)
    {
        lastDisplayMsec = currentMsec;
        blfDisplayTime(currentMsec - startMsec);
    }

    // Read the line sensors
    pcf8574.read(&lineSensors);
    lineSensors &= 7;
    log_d("%d %d %d",
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
        r4aRobotStop(&robot, millis(), r4aBtSerial);
        break;

    //     RcL
    case 0b010:
    case 0b101:
        // Robot over center of line
        robotMotorSetSpeeds(BLF_SPEED_LV1,  BLF_SPEED_LV1); // Move Forward
        break;

    //     RcL
    case 0b001:
    case 0b011:
        // Robot over left sensor, need to turn left
        robotMotorSetSpeeds(-BLF_SPEED_LV3, BLF_SPEED_LV4); // Turn left
        break;

    //     RcL
    case 0b100:
    case 0b110:
        // Robot over right sensor, need to turn right
        robotMotorSetSpeeds(BLF_SPEED_LV4, -BLF_SPEED_LV3); // Turn right
        break;
    }
}

//*********************************************************************
// Entry point for the application
void setup()
{
    // Initialize the USB serial port
    Serial.begin(115200);
    Serial.println();
    Serial.printf("%s\r\n", __FILE__);

    // Delay to allow the hardware initialize
    delay(1000);

    // Initialize the I2C bus for line sensors and motor control
    log_v("Calling i2cBus.begin");
    r4aEsp32I2cBusBegin(&esp32I2cBus,
                        I2C_SDA,
                        I2C_SCL,
                        R4A_I2C_FAST_MODE_HZ);
    r4aI2cBus = &esp32I2cBus._i2cBus;

    // Initialize the PCA9685 for motor control
    log_v("Calling pca9685.begin");
    pca9685.begin();

    // Initialize the Bluetooth serial device
    if (r4aBluetoothInit(BLUETOOTH_NAME) == false)
        r4aReportFatalError("Failed to initialize the Bluetooth serial device");
    r4aBluetoothEnable = true;
    r4aBluetoothDebug = true;


    // Initialize the robot
    Serial.printf("Initialize the robot\r\n");
    r4aRobotInit(&robot,
                 xPortGetCoreID(),       // CPU core
                 ROBOT_STOP_TO_IDLE_SEC, // Delay after running the challenge
                 robotIdle,              // Idle routine
                 blfDisplayTime);        // Time display routine

    // Execute loop forever
}

//*********************************************************************
// Idle loop for core 1 of the application
void loop()
{
    R4A_BLUETOOTH_STATE_TRANSITION btTransition;

    // Connect to Bluetooth to start the robot
    btTransition = r4aBluetoothUpdate();
    if (btTransition == R4A_BST_CONNECTED)
    {
        // Start the robot challenge
        btFailed = false;
        r4aBtSerial->printf("%s\r\n", __FILE__);
        r4aBtSerial->printf("Calling r4aRobotStart to select the robot challenge\r\n");
        r4aRobotStart(&robot,
                      &basicLineFollowing,
                      R4A_CHALLENGE_SEC_START_DELAY, // Start delay in seconds
                      r4aBtSerial);
    }
    else if (btTransition == R4A_BST_DISCONNECTED)
    {
        // Stop the robot
        btFailed = true;
        r4aRobotStop(&robot, millis(), &Serial);
    }

    // Perform the robot challenge
    r4aRobotUpdate(&robot, millis());
}

//*********************************************************************
// Display the delta time
// Called by the init routine to display the countdown time
// Called by the initial delay routine to display the countdown time
// Called by the stop routine to display the actual challenge duration
// Inputs:
//   deltaMsec: Milliseconds to display
void blfDisplayTime(uint32_t deltaMsec)
{
    uint32_t minutes;
    uint32_t seconds;
    uint32_t milliseconds;

    milliseconds = deltaMsec;
    seconds = milliseconds / R4A_MILLISECONDS_IN_A_SECOND;
    milliseconds -= seconds * R4A_MILLISECONDS_IN_A_SECOND;
    minutes = seconds / R4A_SECONDS_IN_A_MINUTE;
    seconds -= minutes * R4A_SECONDS_IN_A_MINUTE;
    if (r4aBluetoothIsConnected())
        r4aBtSerial->printf("%ld:%02ld.%ld\r\n", minutes, seconds, milliseconds / 100);
    else
        btFailed = true;
}

//*********************************************************************
// Initialize the robot challenge
// The robotStart calls this routine before switching to the initial
// delay state.
// Inputs:
//   object: Address of a R4A_ROBOT_CHALLENGE data structure
void blfInit(struct _R4A_ROBOT_CHALLENGE * object)
{
    Serial.printf("blfInit\r\n");
}

//*********************************************************************
// Start the robot challenge
// The initial delay routine calls this routine just before calling
// the challenge routine for the first time.
// Inputs:
//   object: Address of a R4A_ROBOT_CHALLENGE data structure
void blfStart(struct _R4A_ROBOT_CHALLENGE * object)
{
    Serial.printf("blfStart\r\n");
    if (r4aBluetoothIsConnected())
        r4aBtSerial->printf("blfStart\r\n");
    else
        btFailed = true;
    startMsec = millis();
    lastDisplayMsec = 0;
}

//*********************************************************************
// The robot.stop routine calls this routine to stop the motors and
// perform any other actions.
void blfStop(R4A_ROBOT_CHALLENGE * object)
{
    Serial.printf("blfStop\r\n");
    if (r4aBluetoothIsConnected())
        r4aBtSerial->printf("blfStop\r\n");
    else
        btFailed = true;

    // Display idle state again
    idleDisplayed = false;

    // Apply the brakes
    r4aPca9685MotorBrakeAll();
}

//*********************************************************************
// Called by the update routine when the robot is not running a challenge
// The initial delay routine calls this routine just before calling
// the challenge routine for the first time.
// Inputs:
//   deltaMsec: Milliseconds to display
void robotIdle(uint32_t mSecToStart)
{
    const char * const text = "Robot idle\r\n";

    if (!idleDisplayed)
    {
        idleDisplayed = true;
        Serial.printf(text);
        if (r4aBluetoothIsConnected())
            r4aBtSerial->printf(text);
    }

    // Check for Bluetooth connection
    if ((btFailed == false) && (r4aBluetoothIsConnected() == false))
        btFailed = true;
}

//*********************************************************************
// Set the speeds of the motors
// Start bit, I2C device address, ACK, register address, ACK, 8 data bytes
// with ACKs and a stop bit, all at 400 KHz
// 770 uSec = (1+8+1+8+1+((8+1)×32)+1)÷(400×1000)
// Returns true if successful, false otherwise
bool robotMotorSetSpeeds(int16_t left, int16_t right, Print * display)
{
    // Update motor speeds
    return motorFrontLeft.speed(left, display)
           && motorBackLeft.speed(left, display)
           && motorFrontRight.speed(right, display)
           && motorBackRight.speed(right, display)
           && pca9685.writeBufferedRegisters(display);
}
