/**********************************************************************
  WiFi_Test.ino

  Sample sketch to test WiFi
**********************************************************************/

#include <R4A_ESP32.h>

#define DOWNLOAD_AREA       "/nvm/"

#include "Parameters.h"

//****************************************
// Constants
//****************************************

// Telnet port number
#define TELNET_PORT         23

#define WIFI_TEST_TIME      (15 * 1000)     // 15 Seconds

//****************************************
// Forward routine declarations
//****************************************

bool contextCreate(void ** contextData, NetworkClient * client);
void listClients(const R4A_MENU_ENTRY * menuEntry, const char * command, Print * display);
void serverInfo(const R4A_MENU_ENTRY * menuEntry, const char * command, Print * display);

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

    // Initialize the menu
    r4aMenuBegin(&serialMenu, menuTable, menuTableEntries);

    // Get the parameters
    r4aEsp32NvmGetParameters(&parameterFilePath);

    // Start the WiFi network
    r4aWifiBegin();

    // Initialize the NTP client
    r4aNtpSetup(-10 * R4A_SECONDS_IN_AN_HOUR, true);
}

//*********************************************************************
// Idle loop for core 1 of the application
void loop()
{
    uint32_t currentMsec;
    static uint32_t lastWiFiStateChangeMsec;
    static bool previousConnected;
    static bool wifiStop;

    // Determine if it is time to change the WiFi state for testing
    currentMsec = millis();
    if ((currentMsec - lastWiFiStateChangeMsec) >= WIFI_TEST_TIME)
    {
        lastWiFiStateChangeMsec = currentMsec;

        // Toggle the WiFi state
        wifiStop = !wifiStop;
        if (wifiStop)
            r4aWifiStationOn(__FILE__, __LINE__);
        else
            r4aWifiStationOff(__FILE__, __LINE__);
    }

    // Update the WiFi state
    r4aWifiUpdate();

    // Check for NTP updates
    r4aNtpUpdate(r4aWifiStationOnline);

    // Notify the telnet server of WiFi changes
    if (previousConnected != r4aWifiStationOnline)
    {
        previousConnected = r4aWifiStationOnline;
        if (r4aWifiStationOnline)
        {
            telnet.begin(WiFi.STA.localIP(), TELNET_PORT);
            Serial.printf("Telnet: %s:%d\r\n", WiFi.localIP().toString().c_str(),
                          telnet.port());
        }
        else
            telnet.end();
    }
    telnet.update(r4aWifiStationOnline);

    // Process serial commands
    r4aSerialMenu(&serialMenu);
}
