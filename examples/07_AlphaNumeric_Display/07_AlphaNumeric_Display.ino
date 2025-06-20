/**********************************************************************
  07_AlphaNumeric_Display.ino

  Robots-For-All (R4A)
  Display data on the SparkFun AlphaNumeric display (COM-18565)
  connected to a SparkFun SparkFun Thing Plus - ESP32 WROOM (USB-C)
**********************************************************************/

#define FLIP_X_FLIP_Y           0
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
// Constants
//****************************************

#define I2C_SCL                 22
#define I2C_SDA                 21

#define VK16K33_I2C_ADDRESS     0x70

//      aaaaaaaaa
//      fi  j  kb
//      f i j k b
//      f  ijk  b
//       ggg hhh    Display
//      e  nml  c   0 1 colon 2 period 3
//      e n m l c
//      en  m  lc
//      ddddddddd
//
//      D0a     (0,0)   D1a     (0,1)   D2a     (0,2)   D3a     (0,3)
//      D0b     (2,0)   D1b     (2,1)   D2b     (2,2)   D3b     (2,3)
//      D0c     (4,0)   D1c     (4,1)   D2c     (4,2)   D3c     (4,3)
//      D0d     (6,0)   D1d     (6,1)   D2d     (6,2)   D3d     (6,3)
//      D0e     (8,0)   D1e     (8,1)   D2e     (8,2)   D3e     (8,3)
//      D0f     (a,0)   D1f     (a,0)   D2f     (a,2)   D3f     (a,3)
//      D0g     (c,0)   D1g     (c,1)   D2g     (c,2)   D3g     (c,3)
//      D0h     (0,4)   D1h     (0,5)   D2h     (0,6)   D3h     (0,7)
//      D0i     (2,4)   D1j     (2,5)   D2j     (2,6)   D3j     (2,7)
//      D0j     (4,4)   D1i     (4,5)   D2i     (4,6)   D3i     (4,7)
//      D0k     (6,4)   D1k     (6,5)   D2k     (6,6)   D3k     (6,7)
//      D0l     (8,4)   D1l     (8,5)   D2l     (8,6)   D3l     (8,7)
//      D0m     (a,4)   D1m     (a,5)   D2m     (a,6)   D3m     (a,7)
//      D0n     (c,4)   D1n     (c,5)   D2n     (c,6)   D3n     (c,7)
//      colon   (1,0)
//      period  (3,0)

typedef struct _ALPHANUMERIC_DISPLAY_SEGMENT_MAP
{
    uint8_t column;
    uint8_t row;
} ALPHANUMERIC_DISPLAY_SEGMENT_MAP;

#define SEGMENT_a           0
#define SEGMENT_b           1
#define SEGMENT_c           2
#define SEGMENT_d           3
#define SEGMENT_e           4
#define SEGMENT_f           5
#define SEGMENT_g           6
#define SEGMENT_h           7
#define SEGMENT_i           8
#define SEGMENT_j           9
#define SEGMENT_k           10
#define SEGMENT_l           11
#define SEGMENT_m           12
#define SEGMENT_n           13
#define SEGMENT_COLON       14
#define SEGMENT PERIOD      15

const ALPHANUMERIC_DISPLAY_SEGMENT_MAP segmentMap[] =
{
#if FLIP_X_FLIP_Y
    {6,0},  // a --> d
    {8,0},  // b --> e
    {10,0}, // c --> f
    {0,0},  // d --> a
    {2,0},  // e --> b
    {4,0},  // f --> c
    {0,4},  // g --> h
    {12,0}, // h --> g
    {8,4},  // i --> l
    {10,4}, // j --> m
    {12,4}, // k --> n
    {2,4},  // l --> i
    {4,4},  // m --> j
    {6,4},  // n --> k
    {1,0},  // colon
    {3,0}   // period
    //
    //      aaaaaaaaa
    //      fi  j  kb
    //      f i j k b
    //      f  ijk  b
    //       ggg hhh    Display
    //      e  nml  c   0 1 colon 2 period 3
    //      e n m l c
    //      en  m  lc
    //      ddddddddd
    //
#else   // FLIP_X_FLIP_Y
    {0,0},  // a
    {2,0},  // b
    {4,0},  // c
    {6,0},  // d
    {8,0},  // e
    {10,0}, // f
    {12,0}, // g
    {0,4},  // h
    {2,4},  // i
    {4,4},  // j
    {6,4},  // k
    {8,4},  // l
    {10,4}, // m
    {12,4}, // n
    {1,0},  // colon
    {3,0}   // period
#endif  // FLIP_X_FLIP_Y
};

#define LIGHT(x)        (1 << x)

const uint8_t ledMatrixColumnMap[R4A_VK16K33_MAX_COLUMNS] =
{
    0, 2, 4, 6, 8, 10, 12, 14, 1, 3, 5, 7, 9, 11, 13, 15
};

const uint8_t ledMatrixRowPixelMap[R4A_VK16K33_MAX_ROWS] =
{
    0, 1, 2, 3, 4, 5, 6, 7
};

const uint8_t ledDigitMap[4] =
{
#if FLIP_X_FLIP_Y
    3, 2, 1, 0
#else   // FLIP_X_FLIP_Y
    0, 1, 2, 3
#endif  // FLIP_X_FLIP_Y
};

//****************************************
// Font
//****************************************

#define SPACE_INDEX         10

const uint16_t fontNumbers[] =
{
    // 0
    //      aaaaaaaaa
    //      f       b
    //      f       b
    //      f       b
    //
    //      e       c
    //      e       c
    //      e       c
    //      ddddddddd
    LIGHT(SEGMENT_a)
    | LIGHT(SEGMENT_b)
    | LIGHT(SEGMENT_c)
    | LIGHT(SEGMENT_d)
    | LIGHT(SEGMENT_e)
    | LIGHT(SEGMENT_f),

    // 1
    //          j
    //          j
    //          j
    //
    //          m
    //          m
    //          m
    LIGHT(SEGMENT_j)
    | LIGHT(SEGMENT_m),

    // 2
    //      aaaaaaaaa
    //              b
    //              b
    //              b
    //       ggg hhh
    //      e
    //      e
    //      e
    //      ddddddddd
    LIGHT(SEGMENT_a)
    | LIGHT(SEGMENT_b)
    | LIGHT(SEGMENT_d)
    | LIGHT(SEGMENT_e)
    | LIGHT(SEGMENT_g)
    | LIGHT(SEGMENT_h),

    // 3
    //      aaaaaaaaa
    //              b
    //              b
    //              b
    //           hhh
    //              c
    //              c
    //              c
    //      ddddddddd
    LIGHT(SEGMENT_a)
    | LIGHT(SEGMENT_b)
    | LIGHT(SEGMENT_c)
    | LIGHT(SEGMENT_d)
    | LIGHT(SEGMENT_h),

    // 4
    //      f       b
    //      f       b
    //      f       b
    //       ggg hhh
    //              c
    //              c
    //              c
    LIGHT(SEGMENT_b)
    | LIGHT(SEGMENT_c)
    | LIGHT(SEGMENT_f)
    | LIGHT(SEGMENT_g)
    | LIGHT(SEGMENT_h),

    // 5
    //      aaaaaaaaa
    //      f
    //      f
    //      f
    //       ggg hhh
    //              c
    //              c
    //              c
    //      ddddddddd
    LIGHT(SEGMENT_a)
    | LIGHT(SEGMENT_c)
    | LIGHT(SEGMENT_d)
    | LIGHT(SEGMENT_f)
    | LIGHT(SEGMENT_g)
    | LIGHT(SEGMENT_h),

    // 6
    //      aaaaaaaaa
    //      f
    //      f
    //      f
    //       ggg hhh
    //      e       c
    //      e       c
    //      e       c
    //      ddddddddd
    LIGHT(SEGMENT_a)
    | LIGHT(SEGMENT_c)
    | LIGHT(SEGMENT_d)
    | LIGHT(SEGMENT_e)
    | LIGHT(SEGMENT_f)
    | LIGHT(SEGMENT_g)
    | LIGHT(SEGMENT_h),

    // 7
    //      aaaaaaaaa
    //             k
    //            k
    //           k
    //
    //          m
    //          m
    //          m
    LIGHT(SEGMENT_a)
    | LIGHT(SEGMENT_k)
    | LIGHT(SEGMENT_m),

    // 8
    //      aaaaaaaaa
    //      f       b
    //      f       b
    //      f       b
    //       ggg hhh
    //      e       c
    //      e       c
    //      e       c
    //      ddddddddd
    LIGHT(SEGMENT_a)
    | LIGHT(SEGMENT_b)
    | LIGHT(SEGMENT_c)
    | LIGHT(SEGMENT_d)
    | LIGHT(SEGMENT_e)
    | LIGHT(SEGMENT_f)
    | LIGHT(SEGMENT_g)
    | LIGHT(SEGMENT_h),

    // 9
    //      aaaaaaaaa
    //      f       b
    //      f       b
    //      f       b
    //       ggg hhh
    //              c
    //              c
    //              c
    //      ddddddddd
    LIGHT(SEGMENT_a)
    | LIGHT(SEGMENT_b)
    | LIGHT(SEGMENT_c)
    | LIGHT(SEGMENT_d)
    | LIGHT(SEGMENT_f)
    | LIGHT(SEGMENT_g)
    | LIGHT(SEGMENT_h),

    // space
    0,
};

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

R4A_VK16K33 vk16k33 = {&i2cBus,
                       VK16K33_I2C_ADDRESS,
                       ledMatrixColumnMap,
                       ledMatrixRowPixelMap,
                       16,
                       8,
                       0};
#else   // USE_SPARKFUN_THING_PLUS_ESP32_WROOM
    USE_I2C_DEVICE_TABLE;
    USE_I2C_BUS_TABLE;
#endif  // USE_SPARKFUN_THING_PLUS_ESP32_WROOM

R4A_I2C_BUS * r4aI2cBus; // I2C bus for menu system
bool vk16k33Present;

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

    // Determine if the LED controller is available
    log_v("Calling vk16k33Setup");
    vk16k33Present = r4aI2cBusIsDevicePresent(&i2cBus, VK16K33_I2C_ADDRESS);

    // Initialize the VK16K33 LED controller
    if (vk16k33Present)
    {
        log_v("Calling vk16k33Setup");
        r4aVk16k33Setup(&vk16k33);
    }
    else
        Serial.printf("VK16K33 LED controller not detected on the I2C bus!\r\n");

    // Start WiFi if enabled
    log_v("Calling r4aWifiBegin");
    r4aWifiBegin();

    // Initialize the NTP client
    log_v("Calling r4aNtpSetup");
    r4aNtpSetup(-10 * R4A_SECONDS_IN_AN_HOUR, true);

    // Execute loop forever
}

//*********************************************************************
// Idle loop for core 1 of the application
void loop()
{
    static int previousMinutes = -1;
    static time_t previousSeconds;
    time_t seconds;

    do
    {
        // Update the WiFi status
        r4aWifiUpdate();

        // Process serial commands
        r4aSerialMenu(&serialMenu);

        // Determine if WiFi station mode is configured
        if (r4aWifiSsidPasswordEntries == 0)
            break;

        // Check for NTP updates
        r4aNtpUpdate(r4aWifiStationOnline);

        // Display the current time
        seconds = r4aNtpGetEpochTime();
        if ((!vk16k33Present) || (seconds == previousSeconds))
            break;
        previousSeconds = seconds;

        // Parse the time
        int hours = hourFormat12(seconds);
        int minutes = minute(seconds);
        int value;
        if (minutes == previousMinutes)
            break;
        previousMinutes = minutes;


        // Clear the display
        r4aVk16k33BufferClear(&vk16k33);

        // Display the hours
        value = hours / 10;
        if (value)
            displayDigit(0, value);
        else
            displayDigit(0, SPACE_INDEX);
        value = hours - (value * 10);
        displayDigit(1, value);

        // Display the colon
        r4aVk16k33PixelSet(&vk16k33,
                           segmentMap[SEGMENT_COLON].column,
                           segmentMap[SEGMENT_COLON].row);

        // Display the minutes
        value = minutes / 10;
        displayDigit(2, value);
        value = minutes - (value * 10);
        displayDigit(3, value);

        // Pass the buffer to the display
        r4aVk16k33DisplayPixels(&vk16k33);
    } while (0);
}

//*********************************************************************
// Display a digit on the display
void displayDigit(int digit, int value)
{
    int index;
    uint16_t segments;

    // Determine which segments to display
    segments = fontNumbers[value];

    // Walk the segments
    for (index = 0; index < 16; index++)
    {
        // Determine if this segment should be lit up
        if (segments & (1 << index))
        {
            // Display the segment
            r4aVk16k33PixelSet(&vk16k33,
                               segmentMap[index].column,
                               segmentMap[index].row + ledDigitMap[digit]);
        }
    }
}
