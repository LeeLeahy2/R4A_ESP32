/**********************************************************************
  08_LED_Matrix_Display.ino

  Robots-For-All (R4A)
  Display data on the Freenove 4WD Car 16x8 LED matrix
**********************************************************************/

#define DISPLAY_FONT        0
#define USE_SPARKFUN_THING_PLUS_ESP32_WROOM     0

#if     USE_SPARKFUN_THING_PLUS_ESP32_WROOM
    // SparkFun Thing Plus ESP32 WROOM: https://www.sparkfun.com/sparkfun-thing-plus-esp32-wroom-usb-c.html

    #include <R4A_ESP32.h>

    #define I2C_SCL                 22
    #define I2C_SDA                 21

    #define VK16K33_I2C_ADDRESS     0x71
#else   // USE_SPARKFUN_THING_PLUS_ESP32_WROOM
    #include <R4A_Freenove_4WD_Car.h>   // Freenove 4WD Car configuration
#endif  // USE_SPARKFUN_THING_PLUS_ESP32_WROOM

#include "Parameters.h"

//****************************************
// External routines and data
//****************************************

#if     USE_SPARKFUN_THING_PLUS_ESP32_WROOM
    extern const uint8_t r4aLedMatrixColumnMap[];

    // Display a character on the LED matrix
    // Inputs:
    //   vk16k33: Address of a R4A_VK16K33 data structure
    //   xColumn: Column number to modify, zero on left
    //   data: Data to write into the column, bit zero on top, bit 7 on bottom
    void r4aLedMatrixDisplayChar(R4A_VK16K33 * vk16k33, int xColumn, char data);
#endif  // USE_SPARKFUN_THING_PLUS_ESP32_WROOM

//****************************************
// I2C bus configuration
//****************************************

#if     USE_SPARKFUN_THING_PLUS_ESP32_WROOM
    const R4A_I2C_DEVICE_DESCRIPTION i2cBusDeviceTable[] =
    {
        {VK16K33_I2C_ADDRESS,  "VT16K33 16x8 LED controller, LED matrix"},
    };

    R4A_I2C_BUS i2cBus =
    {
        &Wire,              // _i2cBus
        i2cBusDeviceTable,  // _deviceTable
        sizeof(i2cBusDeviceTable) / sizeof(i2cBusDeviceTable[0]), // _deviceTableEntries
        0,                  // _lock
        {0,},               // _present
        r4aEsp32I2cBusWriteWithLock, // _writeWithLock
        r4aEsp32I2cBusRead, // _read
        false,              // _enumerated
    };

    // LED matrix is 16 pixels wide by 8 high, use maximum brightness (15)
    R4A_VK16K33 vk16k33 = {&i2cBus,
                           VK16K33_I2C_ADDRESS,
                           16,
                           8,
                           15};
#else   // USE_SPARKFUN_THING_PLUS_ESP32_WROOM
    USE_I2C_DEVICE_TABLE;
    USE_I2C_BUS_TABLE;
#endif  // USE_SPARKFUN_THING_PLUS_ESP32_WROOM

R4A_I2C_BUS * r4aI2cBus; // I2C bus for menu system

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

const R4A_SSID_PASSWORD r4aWifiSsidPassword[] =
{
    {&wifiSSID1, &wifiPassword1},
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
    log_v("Calling r4aEsp32NvmGetParameters");
    r4aEsp32NvmGetParameters(&parameterFilePath);

    // Initialize the menus
    log_v("Calling r4aMenuBegin");
    r4aMenuBegin(&serialMenu, menuTable, menuTableEntries);

    // Initialize the I2C bus and DON'T do enumeration
    log_v("Calling i2cBus.begin");
    r4aEsp32I2cBusBegin(&i2cBus,
                        I2C_SDA,
                        I2C_SCL,
                        R4A_I2C_FAST_MODE_HZ,
                        nullptr);
    r4aI2cBus = &i2cBus;

    // Delay to allow the hardware initialize
    delay(1000);

    // Enumerate the I2C bus
    r4aI2cBusEnumerate(&i2cBus);

    // Initialize the VK16K33 LED controller
    log_v("Calling r4aVk16k33Setup");
    r4aVk16k33Setup(&vk16k33);

    // Start WiFi if enabled
    log_v("Calling r4aWifiBegin");
    r4aWifiBegin();

    // Initialize the NTP client
    log_v("Calling r4aNtpSetup");
    r4aNtpSetup(-10 * R4A_SECONDS_IN_AN_HOUR, true);

    // Remove any input from the FIFO
    if (DISPLAY_FONT)
    {
        while (Serial.available())
            Serial.read();
        Serial.printf("Press any key to display the pixel columns (x,0)\r\n");
    }

    // Execute loop forever
}

//*********************************************************************
// Idle loop for core 1 of the application
void loop()
{
    if (DISPLAY_FONT)
        displayFont();
    else
        displayTime();
}

//*********************************************************************
// Walk through the font and display each character after serial input
void displayFont()
{
    static int previousIndex;
    static int index;
    int value;

    if (Serial.available())
    {
        while (Serial.available())
            Serial.read();

        // Increment and wrap the index
        index += 1;
        if (index == 49)
            index = 0;

        // Clear the display
        r4aVk16k33ClearBuffer(&vk16k33);

        // Display a pixel in each of the columns at (x, 0)
        if (previousIndex < 16)     // 0 - 15
        {
            value = previousIndex;
            Serial.printf("Column: %d\r\n", value);
            r4aVk16k33PixelSet(&vk16k33, r4aLedMatrixColumnMap[value], 0);
            if (value == 15)
                Serial.printf("Press any key to display the pixel rows (0, y)\r\n");
        }

        // Display a pixel in each of the rows at (0, y)
        else if (previousIndex < 24)    // 16 - 23
        {
            value = previousIndex - 16;
            Serial.printf("Row: %d\r\n", value);
            r4aVk16k33PixelSet(&vk16k33, r4aLedMatrixColumnMap[0], value);
            if (value == 7)
                Serial.printf("Press any key to display the numbers\r\n");
        }

        // Display the numbers
        else if (previousIndex < 34)    // 24 - 33
        {
            value = previousIndex - 24 + '0';
            Serial.printf("Character: %c\r\n", value);
            r4aLedMatrixDisplayChar(&vk16k33, 6, value);
            if (value == '9')
                Serial.printf("Press any key to display the letters\r\n");
        }

        // Display the upper case letters
        else if (previousIndex < 40)    // 34 - 39
        {
            value = previousIndex - 34 + 'A';
            Serial.printf("Character: %c\r\n", value);
            r4aLedMatrixDisplayChar(&vk16k33, 6, value);
        }

        // Display the lower case letters
        else if (previousIndex < 46)    // 40 - 45
        {
            value = previousIndex - 40 + 'a';
            Serial.printf("Character: %c\r\n", value);
            r4aLedMatrixDisplayChar(&vk16k33, 6, value);
        }

        // Display the lower case letter L
        else if (previousIndex == 46)
        {
            Serial.printf("Character: l\r\n");
            r4aLedMatrixDisplayChar(&vk16k33, 6, 'l');
        }

        // Display the lower case letter T
        else if (previousIndex == 47)
        {
            Serial.printf("Character: t\r\n");
            r4aLedMatrixDisplayChar(&vk16k33, 6, 't');
            Serial.printf("Press any key to display the period\r\n");
        }

        // Display the period
        else if (previousIndex == 48)
        {
            Serial.printf("Character: .\r\n");
            r4aLedMatrixDisplayChar(&vk16k33, 6, '.');
            Serial.printf("Press any key to display the pixel columns (x,0)\r\n");
        }

        // Display the pixels
        r4aVk16k33DisplayPixels(&vk16k33);
        previousIndex = index;
    }
}

//*********************************************************************
// Display the time on the LED matrix
void displayTime()
{
    static time_t previousSeconds;
    time_t seconds;

    // Update the WiFi status
    r4aWifiUpdate();

    // Determine if WiFi station mode is configured
    if (r4aWifiSsidPasswordEntries)
    {
        // Check for NTP updates
        r4aNtpUpdate(r4aWifiStationOnline);

        // Display the current time
        seconds = r4aNtpGetEpochTime();
        if (seconds != previousSeconds)
        {
            previousSeconds = seconds;

            int hours = hourFormat12(seconds);
            int minutes = minute(seconds);
            int value;

            // Clear the display
            r4aVk16k33ClearBuffer(&vk16k33);

            // Display the hours
            value = hours / 10;
            if (value)
                displayDigit(0, value);
            value = hours - (value * 10);
            displayDigit(1, value);

            // Display the minutes
            value = minutes / 10;
            displayDigit(6, value);
            value = minutes - (value * 10);
            displayDigit(11, value);

            // Pass the buffer to the display
            r4aVk16k33DisplayPixels(&vk16k33);
        }
    }

    // Process serial commands
    r4aSerialMenu(&serialMenu);
}

//*********************************************************************
// Display a digit on the display
void displayDigit(int column, int value)
{
    // Convert the value to ASCII
    if (column)
        value += '0';
    else if (value)
        value = 'l';

    // Display the value
    r4aLedMatrixDisplayChar(&vk16k33, column, value);
}
