/**********************************************************************
  Pixel_Grid.ino

  Sample sketch to manipulate the WS2812 LEDs
**********************************************************************/

//****************************************
// Includes
//****************************************

#include <R4A_ESP32.h>

//****************************************
// Constants
//****************************************

#define PIXEL_GRID_TESTER           1
#define DISPLAY_REGISTERS           0
#define GENERATE_TRANSLATION_TABLE  0

#define LED_INTENSITY       15      // (0 - 255)
#define LED_UPDATE_MSEC     200

#if PIXEL_GRID_TESTER
// Pixel Grid tester
#define GPIO_SPI_MOSI       27
#define NUMBER_OF_LEDS      150
#else
// Freenove 4WD Car
#define GPIO_SPI_MOSI       32
#define NUMBER_OF_LEDS      12
#endif  // PIXEL_GRID_TESTER

// Red Bits:   23 - 16
// Green Bits: 15 -  8
// Blue Bits:   7 -  0
const uint32_t colors[] =
{
//    0xc00000,   //  0
    0xff0000,   //  1
//    0xc00000,   //  2
//    0xc0c000,   //  3
    0xffff00,   //  4
//    0xc0c000,   //  5
//    0x00c000,   //  6
    0x00ff00,   //  7
//    0x00c000,   //  8
//    0x00c0c0,   //  9
    0x00ffff,   // 10
//    0x00c0c0,   // 11
//    0x0000c0,   // 12
    0x0000ff,   // 13
//    0x0000c0,   // 14
//    0xc000c0,   // 15
    0xff00ff,   // 16
//    0xc000c0,   // 17
};
const uint32_t colorCount = sizeof(colors) / sizeof(colors[0]);

//****************************************
// SPI support - WS2812 LEDs
//****************************************

// SPI controller connected to the SPI bus
const R4A_SPI_BUS spiBus =
{
    1,      // SPI bus number
    -1,     // SCLK GPIO
    GPIO_SPI_MOSI,  // MOSI GPIO
    -1,     // MISO GPIO
    r4aEsp32SpiTransfer // SPI transfer routine
};

// Handle for device data for SPI driver
spi_device_handle_t rgbLedHandle;

// RGB LED chain connected to the SPI bus
const R4A_SPI_DEVICE rgbLed =
{
    &spiBus,         // SPI bus
    &rgbLedHandle,   // Device handle in RAM
    4 * 1000 * 1000, // Clock frequency
    -1,              // No chip select pin
    0,               // Chip select value
    0,               // Clock polarity
    0,               // Clock phase
};

//*********************************************************************
// Entry point for the application
void setup()
{
    // Initialize the USB serial port
    Serial.begin(115200);
    Serial.println();
    Serial.printf("%s\r\n", __FILE__);

    // Delay to allow the hardware initialize
    delay(200);

    // Initialize the SPI controller
    if (!r4aEsp32SpiBegin(&spiBus))
        r4aReportFatalError("Failed to initialize the SPI controller!");

    // Connect the RGB LED chain to the SPI bus
    if (!r4aEsp32SpiDeviceHandleInit(&rgbLed))
        r4aReportFatalError("Failed to get the RGB LED chain handle!");

    // Set the translation table
    if (GENERATE_TRANSLATION_TABLE)
    {
        const uint16_t zero = 0b10000;
        const uint16_t one  = 0b11100;
        if (r4aLedTranslationTable((uint8_t *)&zero, (uint8_t *)&one, 5) == false)
            r4aReportFatalError("Failed to initialize the translation table!");
    }

    // Set the translation table
    else
        r4aLedTranslationTableSet(r4aLEDIntensityTable_1_3_4MHz, 5);

    // Initialize the SPI controller for the RGB LED chain
    if (!r4aLEDSetup(&rgbLed, NUMBER_OF_LEDS))
        r4aReportFatalError("Failed to initialize the RGB LED chain!");

    // Delay to allow the SPI lines to stablize
    delay(100);

    // Reduce the LED intensity
    r4aLEDSetIntensity(LED_INTENSITY);

    // Set the initial LED values
    r4aLEDUpdate(true);

    if (DISPLAY_REGISTERS)
    {
        // Display the clock configuraion
        r4aEsp32ClockDisplay(&Serial);

        // Display the GPIO configuration
        r4aEsp32GpioDisplayRegisters(&Serial);
        r4aEsp32GpioDisplayIoMuxRegisters(&Serial);

        // Display the SPI configuration
        r4aEsp32SpiDisplayRegisters(R4A_ESP32_SPI_0_CONTROLLER, &Serial);
        r4aEsp32SpiDisplayRegisters(R4A_ESP32_SPI_1_CONTROLLER, &Serial);
        r4aEsp32SpiDisplayRegisters(R4A_ESP32_SPI_2_CONTROLLER, &Serial);
        r4aEsp32SpiDisplayRegisters(R4A_ESP32_SPI_3_CONTROLLER, &Serial);
    }

    //****************************************
    // Execute loop forever
    //****************************************
}

//*********************************************************************
// Idle loop for core 1 of the application
void loop()
{
    uint32_t currentMsec;
    uint32_t index;
    static uint32_t lastUpdate;
    uint32_t nextColor;
    static uint32_t offset;

    // Wait for the next time to update the LEDs
    currentMsec = millis();
    if ((currentMsec - lastUpdate) >= LED_UPDATE_MSEC)
    {
        lastUpdate = currentMsec;

        // Update the LEDs
        for (index = 0; index < NUMBER_OF_LEDS; index++)
        {
            // Determine the index into the color table
            nextColor = (offset + index) % colorCount;

            // Set the LED color
            if (PIXEL_GRID_TESTER)
                r4aLEDSetWs2811Color(index, colors[nextColor]);
            else
                r4aLEDSetWs2812Color(index, colors[nextColor]);
        }

        // Shift the colors by one LED for the next display
        offset += 1;

        // Display the LEDs
        r4aLEDUpdate(true);
    }

    // Display the translation table
    if (Serial.available())
    {
        Serial.read();

        // Display the translation table
        r4aLedTranslationTableToC("rgbLedTranslationTable", &Serial);
    }
}
