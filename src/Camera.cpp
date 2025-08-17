/**********************************************************************
  Camera.cpp

  Robots-For-All (R4A)
  Camera support
**********************************************************************/

#include "R4A_ESP32.h"

//*********************************************************************
// Get the initialization status
sensor_t * r4aCameraGetSensor(Print * display)
{
    sensor_t * sensor;   // Sensor routine pointers

    // Get access to the sensor structure
    sensor = esp_camera_sensor_get();
    if (sensor == nullptr)
        display->printf("ERROR: Failed to locate sensor structure!\r\n");
    return sensor;
}

//*********************************************************************
// Get a register value
int r4aCameraGetRegister(int regAddress, Print * display)
{
    sensor_t * sensor;   // Sensor routine pointers
    int status;

    // Get access to the sensor structure
    sensor = r4aCameraGetSensor(display);
    if (sensor == nullptr)
        return -1;

    // Return the initialization status
    status = sensor->get_reg(sensor, regAddress, 0xff);
    if ((status < 0) && display)
        display->printf("Failed to initialize sensor status structure!\r\n");
    return status;
}
