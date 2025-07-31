/**********************************************************************
  I2C_Bus.cpp

  Robots-For-All (R4A)
  I2C bus support for the ESP32
**********************************************************************/

#include "R4A_ESP32.h"

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
    uint8_t data;
    R4A_I2C_BUS * i2cBus;
    bool status;

    // Connect the pins to the I2C controller
    i2cBus = &esp32I2cBus->_i2cBus;
    esp32I2cBus->_twoWire->begin(sdaPin, sclPin);
    esp32I2cBus->_twoWire->setClock(clockHz);

    // Delay to let the signals stablize
    delay(10);

    // Reset the devices on the I2C bus
    data = R4A_I2C_SWRST;
    status = r4aI2cBusWrite(i2cBus,
                            R4A_I2C_GENERAL_CALL_DEVICE_ADDRESS,
                            &data,
                            sizeof(data),
                            debug);
    if (status == false)
    {
        const char * errorMessage = "ERROR: Failed to reset the I2C bus!\r\n";
        if (display)
            display->printf(errorMessage);
        else if (debug)
            debug->printf(errorMessage);
    }

    // Delay while the I2C devices reset
    else
        delay(10);

    // Enumerate the I2C devices
    if (enumerate)
        r4aI2cBusEnumerate(i2cBus, display);
    return status;
}

//*********************************************************************
// Ping an I2C device and see if it responds
// Return true if device detected, false otherwise
bool r4aI2cBusEnumerateDevice(R4A_I2C_BUS * i2cBus, R4A_I2C_ADDRESS_t i2cAddress)
{
    R4A_ESP32_I2C_BUS  * esp32I2cBus;
    int status;

    // Get access to the ESP32 specific data
    esp32I2cBus = (R4A_ESP32_I2C_BUS *)i2cBus;

    // Single thread the I2C requests
    r4aLockAcquire(&esp32I2cBus->_lock);

    // Check for an I2C device
    esp32I2cBus->_twoWire->beginTransmission(i2cAddress);
    status = esp32I2cBus->_twoWire->endTransmission();

    // Release the lock
    r4aLockRelease(&esp32I2cBus->_lock);

    // Return the I2C device found status
    if (status == 0)
        return true;
    return false;
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

    do
    {
        // Get access to the ESP32 specific data
        esp32I2cBus = (R4A_ESP32_I2C_BUS *)i2cBus;

        // Single thread the I2C requests
        r4aLockAcquire(&esp32I2cBus->_lock);

        // Empty the I2C RX buffer
        esp32I2cBus->_twoWire->flush();

        // Address the I2C device
        esp32I2cBus->_twoWire->beginTransmission(i2cAddress);

        // Read the data from the I2C device into the I2C RX buffer
        bytesRead = esp32I2cBus->_twoWire->requestFrom(i2cAddress, readByteCount);

        // Move the data into the read buffer
        for (size_t index = 0; index < bytesRead; index++)
            readBuffer[index] = esp32I2cBus->_twoWire->read();

        // Display the I2C transaction results
        if (display)
        {
            display->printf("    bytesRead: %d\r\n", bytesRead);
            r4aDumpBuffer((intptr_t)readBuffer, readBuffer, bytesRead, display);
        }
    } while (0);

    // Release the lock
    r4aLockRelease(&esp32I2cBus->_lock);

    // Return the number of bytes read
    if (bytesReadAddr)
        *bytesReadAddr = bytesRead;
    return (bytesRead == readByteCount);
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
    uint8_t status;

    do
    {
        status = 1;

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

        // Single thread the I2C requests
        r4aLockAcquire(&esp32I2cBus->_lock);

        // Address the I2C device
        esp32I2cBus->_twoWire->beginTransmission(i2cAddress);

        // Send the data to the device
        if (dataByteCount)
            bytesWritten = esp32I2cBus->_twoWire->write(dataBuffer, dataByteCount);

        // Done sending data to the I2C device
        status = esp32I2cBus->_twoWire->endTransmission();

        // Release the lock
        r4aLockRelease(&esp32I2cBus->_lock);
    } while (0);

    // Display the I2C transaction results
    if (display)
        display->printf("    bytesWritten: %d\r\n", bytesWritten);

    // Return the write status
    return (status == ESP_OK) && (bytesWritten == dataByteCount);
}

//*********************************************************************
// Send data, restart, read data from an I2C peripheral
// Return true upon success, false otherwise
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
    size_t bytesWritten;
    R4A_ESP32_I2C_BUS  * esp32I2cBus;
    uint8_t status;

    do
    {
        status = 1;

        // Get access to the ESP32 specific data
        esp32I2cBus = (R4A_ESP32_I2C_BUS *)i2cBus;

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

        // Single thread the I2C requests
        r4aLockAcquire(&esp32I2cBus->_lock);

        // Address the I2C device
        esp32I2cBus->_twoWire->beginTransmission(i2cAddress);

        // Assume write failure
        bytesWritten = 0;

        // Send the data to the device
        if (dataByteCount)
            bytesWritten = esp32I2cBus->_twoWire->write(dataBuffer, dataByteCount);

        // Done sending data to the I2C device
        if (bytesWritten != dataByteCount)
        {
            status = esp32I2cBus->_twoWire->endTransmission();
            break;
        }

        // Issue a restart
        status = esp32I2cBus->_twoWire->endTransmission(false);
        if (status != ESP_OK)
        {
            if (display)
                display->printf("ERROR: I2C write failed, %d, %s\r\n", status, esp_err_to_name(status));
            break;
        }

        // Read the data from the I2C device into the I2C RX buffer
        bytesRead = esp32I2cBus->_twoWire->requestFrom(i2cAddress, readByteCount);

        // Move the data into the read buffer
        for (size_t index = 0; index < bytesRead; index++)
            readBuffer[index] = esp32I2cBus->_twoWire->read();

        // Release the lock
        r4aLockRelease(&esp32I2cBus->_lock);

        // Save the received data length
        if (bytesReadAddr)
            *bytesReadAddr = bytesRead;

        // Display the I2C transaction results
        if (display)
        {
            display->printf("    bytesRead: %d\r\n", bytesRead);
            r4aDumpBuffer((intptr_t)readBuffer, readBuffer, bytesRead, display);
        }
    } while (0);

    // Display the I2C transaction results
    if (display)
        display->printf("    bytesWritten: %d\r\n", bytesWritten);

    // Return the write status
    return (status == ESP_OK) && (bytesWritten == dataByteCount);
}
