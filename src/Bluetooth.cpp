/**********************************************************************
  Bluetooth.cpp

  Bluetooth support
**********************************************************************/

#include "R4A_ESP32.h"

//****************************************
// Constants
//****************************************

enum R4A_BLUETOOTH_STATE
{
    R4A_BLUETOOTH_STATE_OFF = 0,
    R4A_BLUETOOTH_STATE_WAIT_CONNECT,
    R4A_BLUETOOTH_STATE_DATA,
};

//****************************************
// Globals
//****************************************

bool r4aBluetoothDebug;
bool r4aBluetoothVerbose;
bool r4aBluetoothEnable;
BluetoothSerial * r4aBtSerial;

//****************************************
// Locals
//****************************************

static const char * r4aBluetoothServiceName;
static bool r4aBluetoothStarted;
static uint8_t r4aBluetoothState;

//*********************************************************************
// Display the Bluetooth address
void r4aBluetoothAddress(Print * display)
{
    uint8_t mac[6];

    // Display the Bluetooth address
    r4aBtSerial->getBtAddress(mac);
    display->printf("Bluetooth: %s (%02x:%02x:%02x:%02x:%02x:%02x)\r\n",
                    r4aBluetoothServiceName,
                    mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
}

//*********************************************************************
// Initialize the Bluetooth serial device
// Inputs:
//   name: Service name advertised by Bluetooth
// Outputs:
//   Returns true if the Bluetooth serial device was successfully initialized,
//   and returns false upon failure.
bool r4aBluetoothInit(const char * name)
{
    r4aBluetoothServiceName = name;
    r4aBtSerial = new BluetoothSerial();
    return (r4aBtSerial != nullptr);
}

//*********************************************************************
// Determine if the Bluetooth serial device is connected
bool r4aBluetoothIsConnected()
{
    return r4aBtSerial->hasClient();
}

//*********************************************************************
// Start Bluetooth
bool r4aBluetoothStart()
{
    // Start Bluetooth
    if (r4aBluetoothEnable)
    {
        if (r4aBluetoothVerbose && r4aBluetoothDebug)
            Serial.printf("Bluetooth calling SerialBT.begin(%p%s%s%s)\r\n",
                          r4aBluetoothServiceName,
                          r4aBluetoothServiceName ? " (" : "",
                          r4aBluetoothServiceName ? r4aBluetoothServiceName : "",
                          r4aBluetoothServiceName ? ")" : "");
        r4aBluetoothStarted = r4aBtSerial->begin(r4aBluetoothServiceName);
        if (r4aBluetoothDebug)
        {
            if (r4aBluetoothStarted)
            {
                if (r4aBluetoothServiceName)
                    Serial.printf("Bluetooth started: %s\r\n", r4aBluetoothServiceName);
                else
                    Serial.printf("Bluetooth started\r\n");
                if (r4aBluetoothVerbose)
                    Serial.printf("Bluetooth waiting for client connection\r\n");
            }
            else
                Serial.printf("ERROR: Failed to start bluetooth!\r\n");
        }
    }
    return r4aBluetoothStarted;
}

//*********************************************************************
// Stop Bluetooth
void r4aBluetoothStop()
{
    if (r4aBluetoothStarted)
    {
        if (r4aBtSerial->hasClient())
        {
            if (r4aBluetoothVerbose && r4aBluetoothDebug)
                Serial.printf("Bluetooth calling SerialBT.disconnect\r\n");
            r4aBtSerial->disconnect();
            if (r4aBluetoothVerbose && r4aBluetoothDebug)
                Serial.printf("Bluetooth disconnected\r\n");
        }
        if (r4aBluetoothVerbose && r4aBluetoothDebug)
            Serial.printf("Bluetooth calling SerialBT.end\r\n");
        r4aBtSerial->end();
        if (r4aBluetoothDebug)
            Serial.printf("Bluetooth stopped\r\n");
        r4aBluetoothState = R4A_BLUETOOTH_STATE_OFF;
    }
}

//*********************************************************************
// Update the Bluetooth state
R4A_BLUETOOTH_STATE_TRANSITION r4aBluetoothUpdate()
{
    static uint32_t bluetoothTimer;
    R4A_BLUETOOTH_STATE_TRANSITION btTransition;

    // Shutdown Bluetooth when disabled
    btTransition = R4A_BST_NONE;
    if (r4aBluetoothEnable == false)
    {
        btTransition = R4A_BST_DISCONNECTED;
        r4aBluetoothStop();
    }

    // Update the Bluetooth state
    switch (r4aBluetoothState)
    {
    case R4A_BLUETOOTH_STATE_OFF:
        if (r4aBluetoothEnable && r4aBluetoothStart())
        {
            bluetoothTimer = millis();
            r4aBluetoothState = R4A_BLUETOOTH_STATE_WAIT_CONNECT;

            // Display the Bluetooth name and MAC address
            r4aBluetoothAddress(&Serial);
        }
        break;

    case R4A_BLUETOOTH_STATE_WAIT_CONNECT:
        // Determine if a client has connected
        if ((millis() - bluetoothTimer) >= 100)
        {
            bluetoothTimer = millis();
            if (r4aBtSerial->hasClient())
            {
                if (r4aBluetoothDebug)
                    Serial.printf("Bluetooth connected\r\n");
                btTransition = R4A_BST_CONNECTED;
                r4aBluetoothState = R4A_BLUETOOTH_STATE_DATA;
            }
        }
        break;

    case R4A_BLUETOOTH_STATE_DATA:
        // Handle disconnects
        if (r4aBtSerial->hasClient() == false)
        {
            if (r4aBluetoothVerbose && r4aBluetoothDebug)
                Serial.printf("Bluetooth calling SerialBT.disconnect\r\n");
            r4aBtSerial->disconnect();
            if (r4aBluetoothDebug)
                Serial.printf("Bluetooth disconnected\r\n");
            btTransition = R4A_BST_DISCONNECTED;
            r4aBluetoothState = R4A_BLUETOOTH_STATE_WAIT_CONNECT;
        }
        break;
    }
    return btTransition;
}
