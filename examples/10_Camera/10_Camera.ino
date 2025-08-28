/**********************************************************************
  10_Camera.ino

  Sample sketch to use a browser to display camera images

  Modified from:
    Filename    : Camera Tcp Serrver
    Product     : Freenove 4WD Car for ESP32
    Auther      : www.freenove.com
    Modification: 2021/12/02
**********************************************************************/

#include <R4A_ESP32.h>
#include <R4A_Freenove_4WD_Car.h>

#include "esp_camera.h"
#include <WiFi.h>
#include <WiFiClient.h>
#include <WiFiAP.h>

//****************************************
// Constants
//****************************************

// Servo default starting position
#define SERVO_PAN_START     90  // Degrees
#define SERVO_TILT_START     2  // Degrees

#define CMD_PORT        4000    // Connect with raw terminal
#define CAMERA_PORT     7000

#define DOWNLOAD_AREA       "/nvm/"

// The constants above are used in Parameters.h
#include "Parameters.h"

//****************************************
// I2C bus configuration
//****************************************

USE_I2C_DEVICE_TABLE;
USE_I2C_BUS_TABLE;

R4A_I2C_BUS * r4aI2cBus; // I2C bus for menu system

//****************************************
// Menus
//****************************************

extern const R4A_MENU_TABLE menuTable[];
extern const int menuTableEntries;

//****************************************
// OV2640 Camera
//****************************************

// List the users of the camera, one bit per user
enum CAMERA_USERS
{
    CAMERA_USER_DISABLED = 0,
    CAMERA_USER_WEB_SERVER,
};

// Forward routines
bool ov2640ProcessWebServerFrameBuffer(camera_fb_t * frameBuffer, Print * display);

const R4A_OV2640_SETUP ov2640Parameters =
{
    &r4a4wdCarOv2640Pins,   // ESP32 GPIO pins for the 0V2640 camera
    20 * 1000 * 1000,       // Input clock frequency for the OV2640
    LEDC_TIMER_0,           // Timer producing the 2x clock frequency
    LEDC_CHANNEL_0,         // Channel dividing the clock frequency to 1x
    OV2640_I2C_ADDRESS,     // Device address of the OV2640 on the SCCB bus
    10,                     // Value for JPEG quality
    PIXFORMAT_JPEG,         // Value specifying the pixel format
    FRAMESIZE_CIF,          // Value specifying the frame size
    1                       // Number of frame buffers to allocate
};

//****************************************
// Servos
//****************************************

USE_SERVO_TABLE;

//****************************************
// Serial menu support
//****************************************

R4A_MENU serialMenu;

//****************************************
// Telnet support
//****************************************

/*
R4A_TELNET_SERVER telnet(4,
                         r4aTelnetContextProcessInput,
                         contextCreate,
                         r4aTelnetContextDelete);
*/

//****************************************
// Web server
//****************************************

// Forward routine declarations
void webServerConfigUpdate(R4A_WEB_SERVER * object, httpd_config_t * config);
bool webServerRegisterErrorHandlers(R4A_WEB_SERVER * object);
bool webServerRegisterUriHandlers(R4A_WEB_SERVER * object);

// Web server object
R4A_WEB_SERVER webServer =
{
    webServerConfigUpdate,          // _configUpdate
    webServerRegisterErrorHandlers, // _registerErrorHandlers
    webServerRegisterUriHandlers,   // _registerUriHandlers
    80,         // _port
    nullptr,    // _webServer
};

//****************************************
// WiFi support
//****************************************

const R4A_SSID_PASSWORD r4aWifiSsidPassword[] =
{
    {&wifiSSID1, &wifiPassword1},
    {&wifiSSID2, &wifiPassword2},
    {&wifiSSID3, &wifiPassword3},
    {&wifiSSID4, &wifiPassword4},
};
const int r4aWifiSsidPasswordEntries = sizeof(r4aWifiSsidPassword)
                                     / sizeof(r4aWifiSsidPassword[0]);

WiFiServer server_Cmd(CMD_PORT);
WiFiServer server_Camera(CAMERA_PORT);
extern TaskHandle_t loopTaskHandle;

//*********************************************************************
// Entry point for the application
void setup()
{
    Serial.begin(115200);
    Serial.setDebugOutput(false);
    Serial.println();
    Serial.printf("%s\r\n", __FILE__);

    // Verify the tables
    log_d("Calling r4aEsp32CameraVerifyTables");
    r4aEsp32CameraVerifyTables();

    // Get the parameters
    log_d("Calling r4aEsp32NvmGetParameters");
    r4aEsp32NvmGetParameters(&parameterFilePath);

    // Initialize the menus
    log_d("Calling r4aMenuBegin");
    r4aMenuBegin(&serialMenu, menuTable, menuTableEntries);

    // Connect WiFi station to a remote WiFi access point
    log_d("Calling r4aWifiBegin");
    r4aWifiBegin();

    // Initialize the web server
    r4aWebServerInit(CAMERA_USER_WEB_SERVER);

    // Enable web server debugging
    r4aWebServerDebug = webServerDebug ? &Serial : nullptr;

    // Initialize the I2C bus
    log_v("Calling r4aEsp32I2cBusBegin");
    r4aEsp32I2cBusBegin(&esp32I2cBus,
                        I2C_SDA,
                        I2C_SCL,
                        R4A_I2C_FAST_MODE_HZ);
    r4aI2cBus = &esp32I2cBus._i2cBus;

    // Initialize the PCA9685
    log_v("Calling pca9685.begin");
    if (pca9685.begin())
    {
        // Initialize the Pan/Tilt servos
        log_v("Calling servoPan.positionSet");
        servoPan.positionSet(servoPanStartDegrees);
        log_v("Calling servoTilt.positionSet");
        servoTilt.positionSet(servoTiltStartDegrees);
    }

    // Initialize the camera
    log_d("Calling r4aOv2640Setup");
    r4aOv2640Setup(&ov2640Parameters);

    disableCore0WDT();
}

//*********************************************************************
// Idle loop for core 1 of the application
void loop()
{
    static bool connected;

    // Update the WiFi status
    r4aWifiUpdate();

    // Update the web server
    r4aWebServerUpdate(&webServer,
                       r4aWebServerEnable && (r4aWifiStationOnline || r4aWifiSoftApOnline));

    // Initialize the camera and command servers
    if ((connected == false) && r4aWifiStationOnline)
    {
        connected = true;

        // Initialize the servers
        log_d("Calling server_Camera.begin");
        server_Camera.begin(CAMERA_PORT);
        log_d("Calling server_Cmd.begin");
        server_Cmd.begin(CMD_PORT);
    }

    // Update the camera client
    cameraServerUpdate();

    // Update the command client
    commandServerUpdate();

    // Stop the servers
    if (connected && (r4aWifiStationOnline == false))
    {
        server_Camera.stop();
        server_Cmd.stop();
    }

    // Process serial commands
    r4aSerialMenu(&serialMenu);
}

//*********************************************************************
// Update the camera server
void cameraServerUpdate()
{
    static WiFiClient cameraClient;
    static bool cameraClientAttached;
    static camera_fb_t * fb;
    static size_t offset;
    static size_t bytesRemaining;

    // Listen for incoming camera clients
    if (cameraClientAttached == false)
    {
        cameraClient = server_Camera.available();
        if (cameraClient)
        {
            Serial.println("Camera Server connected to a client.");// print a message out the serial port
            cameraClientAttached = true;
        }
    }

    // Process the camera frames
    else if (cameraClient.connected())
    {
        // Get the next frame
        if (fb == nullptr)
        {
            fb = esp_camera_fb_get();
            if (fb != NULL)
            {
                uint8_t slen[4];
                slen[0] = fb->len >> 0;
                slen[1] = fb->len >> 8;
                slen[2] = fb->len >> 16;
                slen[3] = fb->len >> 24;
                cameraClient.write(slen, 4);
                bytesRemaining = fb->len;
                offset = 0;
            }
            else
                Serial.println("Camera Error");
        }

        // Send more data to the client
        if (fb && bytesRemaining)
        {
            size_t bytesWritten = cameraClient.write(&fb->buf[offset], bytesRemaining);
            if (bytesWritten)
            {
                // Account for the data sent
                bytesRemaining -= bytesWritten;
                offset += bytesWritten;
                Serial.printf("bytesWritten: %ld, offset: %ld, bytesRemaining: %ld\r\n", bytesWritten, offset, bytesRemaining);

                // Return the frame buffer when done
                if (bytesRemaining == 0)
                {
                    esp_camera_fb_return(fb);
                    fb = nullptr;
                }
            }
        }
    }

    // Disconnect the camera client
    else if (cameraClientAttached)
    {
        // close the connection:
        cameraClient.stop();
        Serial.println("Camera Client Disconnected.");

        // Return the frame buffer
        if (fb)
        {
            esp_camera_fb_return(fb);
            fb = nullptr;
        }
        cameraClientAttached = false;
    }
}

//*********************************************************************
// Update the command client
void commandServerUpdate()
{
    static WiFiClient cmdClient;
    static bool cmdClientAttached;
    static String currentLine = "";
    static size_t offset;
    static size_t bytesRemaining;

    // Check for a command client
    if (cmdClientAttached == false)
    {
        //listen for incoming clients
        cmdClient = server_Cmd.available();
        if (cmdClient)
        {
            Serial.println("Command Server connected to a client.");
            cmdClientAttached = true;
        }
    }

    // Process command client input
    else if (cmdClient.connected())
    {
        // Read the command
        if (cmdClient.available())
        {
            String dataBuffer = cmdClient.readStringUntil('\n') + String("\n");

            // Print the command
            Serial.print(dataBuffer);
        }
    }

    // Disconnect the command client
    else if (cmdClientAttached)
    {
        cmdClient.stop();// close the connection:
        Serial.println("Command Client Disconnected.");
        cmdClientAttached = false;
    }
}

