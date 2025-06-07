/**********************************************************************
  I2C_Bus.cpp

  Robots-For-All (R4A)
  I2C bus support for the ESP32
**********************************************************************/

#include "R4A_ESP32.h"

//*********************************************************************
// Initialize the I2C bus
void r4aEsp32I2cBusBegin(R4A_ESP32_I2C_BUS * esp32I2cBus,
                         int sdaPin,
                         int sclPin,
                         int clockHz,
                         bool enumerate,
                         Print * display)
{
    R4A_I2C_BUS * i2cBus;

    // Connect the pins to the I2C controller
    i2cBus = &esp32I2cBus->_i2cBus;
    esp32I2cBus->_twoWire->begin(sdaPin, sclPin);
    esp32I2cBus->_twoWire->setClock(clockHz);

    // Reset the devices on the I2C bus
    esp32I2cBus->_twoWire->beginTransmission(R4A_I2C_GENERAL_CALL_DEVICE_ADDRESS);
    esp32I2cBus->_twoWire->write(R4A_I2C_SWRST);
    esp32I2cBus->_twoWire->endTransmission();

    // Delay while the I2C devices reset
    delay(10);

    // Enumerate the I2C devices
    if (enumerate)
        r4aI2cBusEnumerate(i2cBus, display);
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
// Get the TwoWire pointer
//
// Warning: Using the I2C bus outside of these routines will break the
// I2C controller synchronization leading to hangs, crashes and unspecified
// behavior!
//
// Inputs:
//   i2cBus: Address of a R4A_I2C_BUS data structure
// Outputs:
//   Returns the TwoWire object address
TwoWire * r4aI2cBusGetTwoWire(R4A_I2C_BUS * i2cBus)
{
    R4A_ESP32_I2C_BUS * espI2cBus;

    return espI2cBus->_twoWire;
}

//*********************************************************************
// Read data from an I2C peripheral
bool r4aI2cBusRead(R4A_I2C_BUS * i2cBus,
                   R4A_I2C_ADDRESS_t i2cAddress,
                   const uint8_t * cmdBuffer, // Does not include I2C address
                   size_t cmdByteCount,
                   uint8_t * readBuffer,
                   size_t readByteCount,
                   Print * display,
                   bool releaseI2cBus)
{
    size_t bytesRead;
    R4A_ESP32_I2C_BUS  * esp32I2cBus;

    do
    {
        // Get access to the ESP32 specific data
        esp32I2cBus = (R4A_ESP32_I2C_BUS *)i2cBus;

        // Assume read failure
        bytesRead = 0;

        // Single thread the I2C requests
        r4aLockAcquire(&esp32I2cBus->_lock);

        // Empty the I2C RX buffer
        esp32I2cBus->_twoWire->flush();

        // Address the I2C device
        if (cmdByteCount)
        {
            if (!r4aEsp32I2cBusWriteWithLock(i2cBus,
                                             i2cAddress,
                                             cmdBuffer,   // Register address or other data
                                             cmdByteCount,
                                             nullptr,
                                             0,
                                             display,
                                             false))
                break;
        }

        // Read the data from the I2C device into the I2C RX buffer
        bytesRead = esp32I2cBus->_twoWire->requestFrom(i2cAddress, readByteCount);
        esp32I2cBus->_twoWire->endTransmission(releaseI2cBus);

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
    return (bytesRead == (cmdByteCount + readByteCount));
}

//*********************************************************************
// Send data to an I2C peripheral
// Return true upon success, false otherwise
bool r4aEsp32I2cBusWriteWithLock(R4A_I2C_BUS * i2cBus,
                                 R4A_I2C_ADDRESS_t i2cAddress,
                                 const uint8_t * cmdBuffer,
                                 size_t cmdByteCount,
                                 const uint8_t * dataBuffer,
                                 size_t dataByteCount,
                                 Print * display,
                                 bool releaseI2cBus)
{
    size_t bytesWritten;
    size_t cmdBytesWritten;
    size_t dataBytesWritten;
    R4A_ESP32_I2C_BUS  * esp32I2cBus;

    do
    {
        // Get access to the ESP32 specific data
        esp32I2cBus = (R4A_ESP32_I2C_BUS *)i2cBus;

        // Assume write failure
        bytesWritten = 0;
        cmdBytesWritten = 0;
        dataBytesWritten = 0;

        // Display the I2C transaction request
        if (display)
        {
            display->println("I2C Transaction");
            display->printf("    i2cAddress: 0x%02x\r\n", i2cAddress);
            display->printf("    cmdBuffer: %p\r\n", cmdBuffer);
            display->printf("    cmdByteCount: %d\r\n", cmdByteCount);
            display->printf("    dataBuffer: %p\r\n", dataBuffer);
            display->printf("    dataByteCount: %d\r\n", dataByteCount);
            if (cmdByteCount)
                r4aDumpBuffer((intptr_t)cmdBuffer, cmdBuffer, cmdByteCount, display);
            if (dataByteCount)
                r4aDumpBuffer((intptr_t)dataBuffer, dataBuffer, dataByteCount, display);
        }

        // Address the I2C device
        esp32I2cBus->_twoWire->beginTransmission(i2cAddress);

        // Send the command to the device
        if (cmdByteCount)
        {
            cmdBytesWritten = esp32I2cBus->_twoWire->write(cmdBuffer, cmdByteCount);
            bytesWritten += cmdBytesWritten;
        }

        // Send the data to the device
        if (dataByteCount)
        {
            dataBytesWritten = esp32I2cBus->_twoWire->write(dataBuffer, dataByteCount);
            bytesWritten += dataBytesWritten;
        }

        // Done sending data to the I2C device
        if (bytesWritten == (cmdByteCount + dataByteCount))
            esp32I2cBus->_twoWire->endTransmission(releaseI2cBus);
        else
            esp32I2cBus->_twoWire->endTransmission();
    } while (0);

    // Display the I2C transaction results
    if (display)
    {
        display->printf("    cmdBytesWritten: %d\r\n", cmdBytesWritten);
        display->printf("    dataBytesWritten: %d\r\n", dataBytesWritten);
        display->printf("    bytesWritten: %d\r\n", bytesWritten);
    }

    // Return the write status
    return (bytesWritten == (cmdByteCount + dataByteCount));
}

//*********************************************************************
// Send data to an I2C peripheral
// Return true upon success, false otherwise
bool r4aI2cBusWrite(R4A_I2C_BUS * i2cBus,
                    R4A_I2C_ADDRESS_t i2cAddress,
                    const uint8_t * cmdBuffer,
                    size_t cmdByteCount,
                    const uint8_t * dataBuffer,
                    size_t dataByteCount,
                    Print * display,
                    bool releaseI2cBus)
{
    R4A_ESP32_I2C_BUS * esp32I2cBus;
    bool status;

    // Get access to the ESP32 specific data
    esp32I2cBus = (R4A_ESP32_I2C_BUS *)i2cBus;

    // Single thread the I2C requests
    r4aLockAcquire(&esp32I2cBus->_lock);

    // Send data to an I2C peripheral
    status = r4aEsp32I2cBusWriteWithLock(i2cBus,
                                         i2cAddress,
                                         cmdBuffer,
                                         cmdByteCount,
                                         dataBuffer,
                                         dataByteCount,
                                         display,
                                         releaseI2cBus);

    // Release the lock
    r4aLockRelease(&esp32I2cBus->_lock);

    // Return the write status
    return status;
}
