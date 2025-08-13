/**********************************************************************
  Challenge.ino

  Challenge support routines
**********************************************************************/

//*********************************************************************
// Called upon fatal error
// Inputs:
//   errorMessage: Zero terminated string of characters containing the
//                 error mesage to be displayed
void challengeHalt(const char * errorMessage)
{
    challengeStop();
    if (vk16k33Present)
        r4aVk16k33DisplayHalt(&vk16k33);
    r4aReportFatalError(errorMessage);
}

//*********************************************************************
// Called when the robot starts
void challengeInit()
{
    // Turn off the LED matrix
    if (vk16k33Present)
    {
        r4aVk16k33BufferClear(&vk16k33);
        r4aVk16k33DisplayPixels(&vk16k33);
    }

    // Update the lights
    if (robotUseWS2812)
    {
        car.ledsOff();
        car.brakeLightsOn();
    }

    // Apply the brakes
    r4aPca9685MotorBrakeAll();
}

//*********************************************************************
// Called when the robot starts
void challengeStart()
{
    // Update the lights
    if (robotUseWS2812)
    {
        car.headlightsOn();
        car.brakeLightsOff();
    }
}

//*********************************************************************
// Called when the robot stops
void challengeStop()
{
    // Apply the brakes
    r4aPca9685MotorBrakeAll();

    // Update the lights
    if (robotUseWS2812)
    {
        car.brakeLightsOn();
        car.ledsTurnOff();
        car.headlightsOff();
        car.backupLightsOff();
    }
}
