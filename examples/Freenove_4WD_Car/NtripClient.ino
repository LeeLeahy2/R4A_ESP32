/**********************************************************************
  NtripClient.ino

  NTRIP client support
**********************************************************************/

#ifdef  USE_NTRIP

//****************************************
// Includes
//****************************************

#include <R4A_Freenove_4WD_Car.h>   // Freenove 4WD Car configuration

//****************************************
// Constants
//****************************************

// Define the back-off intervals between connection attempts in milliseconds
const uint32_t r4aNtripClientBbackoffIntervalMsec[] =
{
    0,
    15 * R4A_MILLISECONDS_IN_A_SECOND,
    30 * R4A_MILLISECONDS_IN_A_SECOND,
     1 * R4A_MILLISECONDS_IN_A_MINUTE,
     2 * R4A_MILLISECONDS_IN_A_MINUTE,
};

const int r4aNtripClientBbackoffCount = sizeof(r4aNtripClientBbackoffIntervalMsec) / sizeof(r4aNtripClientBbackoffIntervalMsec[0]);

//*********************************************************************
// Get the I2C bus transaction size
uint8_t r4aNtripClientI2cTransactionSize()
{
#ifdef  USE_ZED_F9P
    if (zedf9pPresent)
        return zedf9p._i2cTransactionSize;
#endif  // USE_ZED_F9P
    return 32;
}

//*********************************************************************
// Push data to the GNSS
int r4aNtripClientPushRawData(uint8_t * buffer,
                              int bytesToPush,
                              Print * display)
{
#ifdef  USE_ZED_F9P
    if (zedf9pPresent)
        return zedf9p.pushRawData(buffer, bytesToPush, display);
#endif  // USE_ZED_F9P
    return bytesToPush;
}

#endif  // USE_NTRIP
