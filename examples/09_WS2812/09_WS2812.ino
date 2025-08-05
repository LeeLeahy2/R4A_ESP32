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

#define DISPLAY_BATTERY_VOLTAGE(display)  \
    r4aEsp32VoltageDisplay(BATTERY_WS2812_PIN, 0, BATTERY_VOLTAGE_MULTIPLIER, display)

#define READ_BATTERY_VOLTAGE(adcValue)  \
    r4aEsp32VoltageGet(BATTERY_WS2812_PIN, 0, BATTERY_VOLTAGE_MULTIPLIER, adcValue)

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

// SPI controller connected to the SPI bus
R4A_ESP32_SPI_CONTROLLER spiBus =
{
    {
        1,      // SPI bus number
        -1,     // SCLK GPIO
        BATTERY_WS2812_PIN,   // MOSI GPIO
        -1,     // MISO GPIO
        r4aEsp32SpiTransfer // SPI transfer routine
    },
    nullptr,
};

// WS2812 - RGB LED connected to the SPI bus
const R4A_SPI_DEVICE ws2812 =
{
    &spiBus._spiBus, // SPI bus
    4 * 1000 * 1000, // Clock frequency
    -1,              // No chip select pin
    0,               // Clock polarity
    0,               // Clock phase
};

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

    // Turn off the buzzer
    log_v("Turning off the buzzer\r\n");
    pinMode(BLUE_LED_BUZZER_PIN, OUTPUT);
    digitalWrite(BLUE_LED_BUZZER_PIN, ESP32_WROVER_BLUE_LED_OFF);

    // Turn off ESP32 Wrover blue LED when battery power is applied
    log_v("Setting the blue LED\r\n");
    float batteryVoltage = READ_BATTERY_VOLTAGE(nullptr);
    int blueLED = (batteryVoltage > 2.)
                ? ESP32_WROVER_BLUE_LED_ON : ESP32_WROVER_BLUE_LED_OFF;
    digitalWrite(BLUE_LED_BUZZER_PIN, blueLED);

    // Delay to allow the hardware initialize
    delay(1000);

    // Initialize the SPI controller
    log_v("r4aEsp32SpiBegin");
    if (!r4aEsp32SpiBegin(&spiBus))
        r4aReportFatalError("Failed to initialize the SPI controller!");

    // Select the WS2812 devices on the SPI bus
    if (!r4aEsp32SpiDeviceSelect(&ws2812))
        r4aReportFatalError("Failed to select the WS2812 devices on the SPI bus!");

    // Initialize the SPI controller for the WD2812 LEDs
    if (!r4aLEDSetup(&ws2812, car.numberOfLEDs))
        r4aReportFatalError("Failed to allocate the SPI device for the WS2812 LEDs!");

    // Turn off all of the 3 color LEDs
    log_v("Calling car.ledsOff");
    car.ledsOff();

    // Reduce the LED intensity
    log_v("Calling r4aLEDSetIntensity");
    r4aLEDSetIntensity(1);

    // Set the initial LED values
    log_v("Calling r4aLEDUpdate");
    r4aLEDUpdate(true);

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
    static bool previousConnected;

    // Turn on the ESP32 WROVER blue LED when the battery power is OFF
    currentMsec = millis();
    if ((currentMsec - lastBatteryCheckMsec) >= 100)
    {
        lastBatteryCheckMsec = currentMsec;
//        batteryVoltage = READ_BATTERY_VOLTAGE(nullptr);
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

        // Notify the telnet server of WiFi changes
        if (previousConnected != r4aWifiStationOnline)
        {
            previousConnected = r4aWifiStationOnline;
            if (r4aWifiStationOnline)
            {
                log_v("telnet.begin");
                telnet.begin(WiFi.STA.localIP(), TELNET_PORT);
                Serial.printf("Telnet: %s:%d\r\n", WiFi.localIP().toString().c_str(),
                              telnet.port());
            }
            else
            {
                log_v("telnet.end");
                telnet.end();
            }
        }
        log_v("telnet.update");
        telnet.update(r4aWifiStationOnline);
    }

    // Process serial commands
    log_v("r4aSerialMenu");
    r4aSerialMenu(&serialMenu);

    // Update the WS2812 LEDs
    car.ledsUpdate(millis());
}
