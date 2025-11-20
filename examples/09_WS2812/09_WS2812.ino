/**********************************************************************
  09_WS2812.ino

  Sample sketch to manipulate the WS2812 LEDs
**********************************************************************/

//****************************************
// Includes
//****************************************

#include <R4A_Freenove_4WD_Car.h>   // Freenove 4WD Car configuration

#define DOWNLOAD_AREA       "/nvm/"

#include "Parameters.h"

//****************************************
// Constants
//****************************************

// Telnet port number
#define TELNET_PORT         23

//****************************************
// Forward routine declarations
//****************************************

bool contextCreate(void ** contextData, NetworkClient * client);

//****************************************
// Battery macros
//****************************************

#define ADC_REFERENCE_VOLTAGE   3.48    // Volts

//****************************************
// Menus
//****************************************

extern const R4A_MENU_TABLE menuTable[];
extern const int menuTableEntries;

//****************************************
// Serial menu support
//****************************************

R4A_MENU serialMenu;

//****************************************
// SPI support - WS2812 LEDs
//****************************************

R4A_4WD_CAR_SPI_WS2812_GLOBALS;

//****************************************
// WiFi support
//****************************************

R4A_TELNET_SERVER telnet(4,
                         r4aTelnetContextProcessInput,
                         contextCreate,
                         r4aTelnetContextDelete);

const R4A_SSID_PASSWORD r4aWifiSsidPassword[] =
{
    {&wifiSSID,  &wifiPassword},
    {&wifiSSID2, &wifiPassword2},
    {&wifiSSID3, &wifiPassword3},
    {&wifiSSID4, &wifiPassword4},
};
const int r4aWifiSsidPasswordEntries = sizeof(r4aWifiSsidPassword)
                                     / sizeof(r4aWifiSsidPassword[0]);

//*********************************************************************
// Entry point for the application
void setup()
{
    // Initialize the USB serial port
    Serial.begin(115200);
    Serial.println();
    Serial.printf("%s\r\n", __FILE__);

    // Get the parameters
    log_d("Calling r4aEsp32NvmGetParameters");
    r4aEsp32NvmGetParameters(&parameterFilePath);

    // Initialize the menus
    log_d("Calling r4aMenuBegin");
    r4aMenuBegin(&serialMenu, menuTable, menuTableEntries);

    // Set the ADC reference voltage
    log_v("Calling r4aEsp32VoltageSetReference");
    r4aEsp32VoltageSetReference(ADC_REFERENCE_VOLTAGE);

    // Turn off ESP32 Wrover blue LED when battery power is applied
    log_v("Setting the blue LED\r\n");
    float batteryVoltage = READ_BATTERY_VOLTAGE(nullptr);
    int blueLED = (batteryVoltage > 2.)
                ? ESP32_WROVER_BLUE_LED_ON : ESP32_WROVER_BLUE_LED_OFF;
    digitalWrite(BLUE_LED_BUZZER_PIN, blueLED);

    // Delay to allow the hardware initialize
    delay(1000);

    // Initialize the SPI controller and WS2812 LEDs
    R4A_4WD_CAR_SPI_WS2812_SETUP(1);

    // Start WiFi if enabled
    log_d("Calling wifiBegin");
    r4aWifiBegin();

    // Initialize the NTP client
    log_d("Calling r4aNtpSetup");
    r4aNtpSetup(-10 * R4A_SECONDS_IN_AN_HOUR, true);

    //****************************************
    // Execute loop forever
    //****************************************
}

//*********************************************************************
// Idle loop for core 1 of the application
void loop()
{
    float batteryVoltage;
    uint32_t currentMsec;
    static uint32_t lastBatteryCheckMsec;

    // Turn on the ESP32 WROVER blue LED when the battery power is OFF
    currentMsec = millis();
    if ((currentMsec - lastBatteryCheckMsec) >= 100)
    {
        lastBatteryCheckMsec = currentMsec;
        batteryVoltage = READ_BATTERY_VOLTAGE(nullptr);
        int blueLED = (batteryVoltage > 2.)
                    ? ESP32_WROVER_BLUE_LED_ON : ESP32_WROVER_BLUE_LED_OFF;
        digitalWrite(BLUE_LED_BUZZER_PIN, blueLED);
    }

    // Update the WiFi status
    r4aWifiUpdate();

    // Determine if WiFi station mode is configured
    if (r4aWifiSsidPasswordEntries)
    {
        // Check for NTP updates
        log_v("r4aNtpUpdate");
        r4aNtpUpdate(r4aWifiStationOnline);

        log_v("telnet.update");
        telnet.update(true, r4aWifiStationOnline);
    }

    // Process serial commands
    log_v("r4aSerialMenu");
    r4aSerialMenu(&serialMenu);

    // Update the WS2812 LEDs
    car.ledsUpdate(millis());
}
