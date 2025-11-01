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
    r4aReportFatalError(errorMessage);
}

//*********************************************************************
// Called when the robot starts
void challengeInit()
{
    // Save the time display state
    robotNtpTimeSave();

    // Apply the brakes
    r4aPca9685MotorBrakeAll();
}

//*********************************************************************
// Called when the robot starts
void challengeStart()
{
}

//*********************************************************************
// Called when the robot stops
void challengeStop()
{
    // Apply the brakes
    r4aPca9685MotorBrakeAll();
}
