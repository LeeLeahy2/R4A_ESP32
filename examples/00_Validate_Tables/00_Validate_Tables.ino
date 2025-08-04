/**********************************************************************
  00_Validate_Tables.ino

  Sample sketch to validate the constant tables
**********************************************************************/

#include <R4A_ESP32.h>

//*********************************************************************
// Entry point for the application
void setup()
{
    // Initialize the USB serial port
    Serial.begin(115200);
    Serial.println();
    Serial.println(__FILE__);

    // Validate the tables
    r4aNtripClientValidateTables();
    r4aWifiValidateTables();

    Serial.println("All tables validated successfully!");

    //****************************************
    // Execute loop forever
    //****************************************
}

//*********************************************************************
// Idle loop for core 1 of the application
void loop()
{
}
