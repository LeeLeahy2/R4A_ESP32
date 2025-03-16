/**********************************************************************
  Challenge.ino

  Challenge support routines
**********************************************************************/

//*********************************************************************
// Called when the robot starts
void challengeInit()
{
    // Update the lights
    car.ledsOff();
    car.brakeLightsOn();

    // Apply the brakes
    r4aPca9685MotorBrakeAll();
}

//*********************************************************************
// Called when the robot starts
void challengeStart()
{
    // Update the lights
    car.headlightsOn();
    car.brakeLightsOff();
}

//*********************************************************************
// Called when the robot stops
void challengeStop()
{
    // Update the lights
    car.brakeLightsOn();
    car.ledsTurnOff();
    car.headlightsOff();
    car.backupLightsOff();

    // Apply the brakes
    r4aPca9685MotorBrakeAll();
}
