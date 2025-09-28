/**********************************************************************
  I2C_Bus.cpp

  Robots-For-All (R4A)
  I2C bus support for the ESP32
**********************************************************************/

#include "R4A_ESP32.h"

#define I2C_TIMEOUT_MSEC                500

//*********************************************************************
// Initialize the I2C bus
bool r4aEsp32I2cBusBegin(R4A_ESP32_I2C_BUS * esp32I2cBus,
                         int sdaPin,
                         int sclPin,
                         int clockHz,
                         bool enumerate,
                         Print * display,
                         Print * debug)
{
    R4A_I2C_BUS * i2cBus;
    esp_err_t status;
    bool success;

    do
    {
        success = false;

        // Connect the pins to the I2C controller
        i2cBus = &esp32I2cBus->_i2cBus;
        status = i2cInit(esp32I2cBus->_busNumber, sdaPin, sclPin, clockHz);
        success = (status == ESP_OK);
        if (success == false)
        {
            const char * errorMessage = "ERROR: Failed to initialize I2C bus %d!\r\n";
            if (display)
                display->printf(errorMessage, esp32I2cBus->_busNumber);
            else if (debug)
                debug->printf(errorMessage, esp32I2cBus->_busNumber);
            break;
        }

        // Enumerate the I2C devices
        if (enumerate)
        {
            // Delay while the I2C devices reset
            delay(10);
            r4aI2cBusEnumerate(i2cBus, display);
        }
    } while (0);
    return success;
}

//*********************************************************************
// Ping an I2C device and see if it responds
// Return true if device detected, false otherwise
bool r4aI2cBusEnumerateDevice(R4A_I2C_BUS * i2cBus, R4A_I2C_ADDRESS_t i2cAddress)
{
    R4A_ESP32_I2C_BUS  * esp32I2cBus;
    esp_err_t status;

    // Get access to the ESP32 specific data
    esp32I2cBus = (R4A_ESP32_I2C_BUS *)i2cBus;

    // Check for an I2C device
    status = i2cWrite(esp32I2cBus->_busNumber,
                      i2cAddress,
                      nullptr,
                      0,
                      I2C_TIMEOUT_MSEC);

    // Return the I2C device found status
    return (status == ESP_OK);
}

//*********************************************************************
// Read data from an I2C peripheral
bool r4aI2cBusRead(R4A_I2C_BUS * i2cBus,
                   R4A_I2C_ADDRESS_t i2cAddress,
                   uint8_t * readBuffer,
                   size_t readByteCount,
                   size_t * bytesReadAddr,
                   Print * display)
{
    size_t bytesRead;
    R4A_ESP32_I2C_BUS  * esp32I2cBus;
    esp_err_t status;

    do
    {
        // Get access to the ESP32 specific data
        esp32I2cBus = (R4A_ESP32_I2C_BUS *)i2cBus;

        // Assume read failure
        bytesRead = 0;

        // Read the data from the I2C device into the I2C RX buffer
        status = i2cRead(esp32I2cBus->_busNumber,
                         i2cAddress,
                         readBuffer,
                         readByteCount,
                         I2C_TIMEOUT_MSEC,
                         &bytesRead);

        // Display the I2C transaction results
        if (display)
        {
            display->printf("    status: %d, %s\r\n", status, esp_err_to_name(status));
            display->printf("    bytesRead: %d (0x%0x)\r\n", bytesRead, bytesRead);
            r4aDumpBuffer((intptr_t)readBuffer, readBuffer, readByteCount, display);
        }
    } while (0);

    // Return the number of bytes read
    if (bytesReadAddr)
        *bytesReadAddr = bytesRead;
    return (status == ESP_OK) && (bytesRead == readByteCount);
}

//*********************************************************************
// Send data to an I2C peripheral
// Return true upon success, false otherwise
bool r4aI2cBusWrite(R4A_I2C_BUS * i2cBus,
                    R4A_I2C_ADDRESS_t i2cAddress,
                    const uint8_t * dataBuffer,
                    size_t dataByteCount,
                    Print * display)
{
    size_t bytesWritten;
    R4A_ESP32_I2C_BUS  * esp32I2cBus;
    esp_err_t status;

    do
    {
        status = ESP_OK;

        // Get access to the ESP32 specific data
        esp32I2cBus = (R4A_ESP32_I2C_BUS *)i2cBus;

        // Assume write failure
        bytesWritten = 0;

        // Display the I2C transaction request
        if (display)
        {
            display->println("I2C Transaction");
            display->printf("    i2cAddress: 0x%02x\r\n", i2cAddress);
            display->printf("    dataBuffer: %p\r\n", dataBuffer);
            display->printf("    dataByteCount: %d\r\n", dataByteCount);
            if (dataByteCount)
                r4aDumpBuffer((intptr_t)dataBuffer, dataBuffer, dataByteCount, display);
        }

        // Send the data to the device

        if (dataByteCount)
            status = i2cWrite(esp32I2cBus->_busNumber,
                              i2cAddress,
                              dataBuffer,
                              dataByteCount,
                              I2C_TIMEOUT_MSEC);

    } while (0);

    // Display the I2C transaction results
    if (display)
        display->printf("    status: %d, %s\r\n", status, esp_err_to_name(status));

    // Return the write status
    return (status == ESP_OK);
}

//*********************************************************************
// Send data, restart, read data from an I2C peripheral
// Return true upon success and false upon failure
bool r4aI2cBusWriteRead(R4A_I2C_BUS * i2cBus,
                        R4A_I2C_ADDRESS_t i2cAddress,
                        const uint8_t * dataBuffer,
                        size_t dataByteCount,
                        uint8_t * readBuffer,
                        size_t readByteCount,
                        size_t * bytesReadAddr,
                        Print * display)
{
    size_t bytesRead;
    R4A_ESP32_I2C_BUS  * esp32I2cBus;
    esp_err_t status;

    do
    {
        // Get access to the ESP32 specific data
        esp32I2cBus = (R4A_ESP32_I2C_BUS *)i2cBus;

        // Assume read failure
        bytesRead = 0;

        // Read the data from the I2C device into the I2C RX buffer
        status = i2cWriteReadNonStop(esp32I2cBus->_busNumber,
                         i2cAddress,
                         dataBuffer,
                         dataByteCount,
                         readBuffer,
                         readByteCount,
                         I2C_TIMEOUT_MSEC,
                         &bytesRead);

        // Display the I2C transaction results
        if (display)
        {
            display->printf("    status: %d, %s\r\n", status, esp_err_to_name(status));
            display->printf("    bytesRead: %d (0x%0x)\r\n", bytesRead, bytesRead);
            r4aDumpBuffer((intptr_t)readBuffer, readBuffer, readByteCount, display);
        }
    } while (0);

    // Return the number of bytes read
    if (bytesReadAddr)
        *bytesReadAddr = bytesRead;
    return (status == ESP_OK) && (bytesRead == readByteCount);
}
