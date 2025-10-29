/**********************************************************************
  01_Basic_Line_Following.ino

  Perform basic line following, modified code from the Freenove
  04.2_Track_Car example

  Modify the blfChallenge routine later in this file to read the line
  sensors and set the motor speeds (run the challenge.

  Flow of Control (Very Simplified):

                           Power Up
                               |
                               V
                             Setup
                               |
                               V
                     Robot Initialization
                               |
                               V
                               +<----------.
                               |           |
                               V           |
                             Loop          |
                               |           |
                               V           |
                         r4aRobotUpdate    | Infinite loop
                               |           |
                               '-----------'

  Call stack:

    setup - Sketch entry point
        r4aRobotInit - Initialize the robot layer
        r4aRobotStart - Starts the initial delay followed by the challenge

    loop - main routine
        r4aRobotUpdate - Updates the robot layer
            blfChallenge - Perform the basic line following
                pcf8574.read - Read line sensors
                    _i2cBus->_read - R4A I2C layer read routine
                        r4aEsp32I2cBusRead - R4A ESP32 I2C interface
                            requestFrom - ESP32 I2C controller API
                            endTransmission - ESP32 I2C controller API
                robotMotorSetSpeeds - Control motors
                    motor*.speed - Select PWM value
                        bufferLedOnOff - Select one PWM value
                    pca9685.writeBufferedRegisters - Write the PWM values
                        r4aI2cBusWrite - R4A I2C layer write routine
                            _writeWithLock - Generic I2C write routine
                                r4aEsp32I2cBusWriteWithLock - R4A ESP32 I2C interface
                                    write - ESP32 I2C controller API
                                    endTransmission - ESP32 I2C controller API
                r4aRobotStop - Stops the robot

    Where is the code?

        libraries
            R4A_Robot
                Robot layer
            R4A_I2C
                PWM code for motors
            R4A_ESP32
                This example
                ESP32 I2C controller layer
**********************************************************************/

#include <R4A_Freenove_4WD_Car.h>   // Freenove 4WD Car configuration

//****************************************
// Constants
//****************************************

#define BLF_DEBUG_LINE_SENSORS      0

#define BLF_SPEED_LV4   4000
#define BLF_SPEED_LV3   3000
#define BLF_SPEED_LV1   1500

//****************************************
// I2C bus configuration
//****************************************

// List of devices on the I2C bus
const R4A_I2C_DEVICE_DESCRIPTION i2cBusDeviceTable[] =
{
    {R4A_I2C_GENERAL_CALL_DEVICE_ADDRESS, "General Call"},
    {PCA9685_I2C_ADDRESS,  "PCA9685 16-Channel LED controller, motors & servos"},
    {PCF8574_I2C_ADDRESS,  "PCF8574 8-Bit I/O Expander, line tracking"},
    {VK16K33_I2C_ADDRESS,  "VT16K33 16x8 LED controller, LED matrix"},
};

// Specify the R4A ESP32 I2C layer parameters
R4A_ESP32_I2C_BUS esp32I2cBus =
{
    {   // R4A_I2C_BUS
        i2cBusDeviceTable,  // _deviceTable
        sizeof(i2cBusDeviceTable) / sizeof(i2cBusDeviceTable[0]), // _deviceTableEntries
        {0,},               // _present
        false,              // _enumerated
    },
    0,                  // _busNumber
};

R4A_I2C_BUS * r4aI2cBus; // I2C bus for menu system

// Connect the R4A I2C layer to the ESP32 I2C controller code
R4A_PCA9685 pca9685(&esp32I2cBus._i2cBus, PCA9685_I2C_ADDRESS, 50, 25 * 1000 * 1000);

    // Connect the motors to the PWM controller (PCA9685)
    R4A_PCA9685_MOTOR motorBackLeft(&pca9685, 8, 9);
    R4A_PCA9685_MOTOR motorBackRight(&pca9685, 11, 10);
    R4A_PCA9685_MOTOR motorFrontRight(&pca9685, 13, 12);
    R4A_PCA9685_MOTOR motorFrontLeft(&pca9685, 14, 15);

// Connect the GPIO controller (PCF8574) to the R4A I2C layer
R4A_PCF8574 pcf8574(&esp32I2cBus._i2cBus, PCF8574_I2C_ADDRESS);

//****************************************
// Forward routine declarations
//****************************************

void blfChallenge(R4A_ROBOT_CHALLENGE * object);
void blfStop(R4A_ROBOT_CHALLENGE * object);
bool robotMotorSetSpeeds(int16_t left, int16_t right, Print * display = nullptr);

//****************************************
// Locals
//****************************************

USE_MOTOR_TABLE;

#define ROBOT_START_DELAY_SEC               5
#define ROBOT_STOP_TO_IDLE_SEC              30

R4A_ROBOT robot;

R4A_ROBOT_CHALLENGE basicLineFollowing =
{
    blfChallenge,
    nullptr,        // init
    nullptr,        // start
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
    uint8_t lineSensors;

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
        r4aRobotStop(&robot, millis());
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

    // Initialize the robot
    r4aRobotInit(&robot,
                 xPortGetCoreID(),       // CPU core
                 ROBOT_START_DELAY_SEC,  // Challenge start delay
                 ROBOT_STOP_TO_IDLE_SEC, // Delay after running the challenge
                 nullptr,                // Idle routine
                 nullptr);               // Time display routine

    // Start the robot challenge
    r4aRobotStart(&robot,
                  &basicLineFollowing,
                  &Serial);

    // Execute loop forever
}

//*********************************************************************
// Idle loop for core 1 of the application
void loop()
{
    // Perform the robot challenge
    r4aRobotUpdate(&robot, millis());
}

//*********************************************************************
// The robot.stop routine calls this routine to stop the motors and
// perform any other actions.
void blfStop(R4A_ROBOT_CHALLENGE * object)
{
    // Apply the brakes
    r4aPca9685MotorBrakeAll();
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
