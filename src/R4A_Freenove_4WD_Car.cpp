/**********************************************************************
  R4A_Freenove_4WD_Car.cpp

  Robots-For-All (R4A)
  Freenove 4WD Car robot support
  https://store.freenove.com/products/fnk0053
  https://www.amazon.com/gp/product/B08X6PTQFM/ref=ox_sc_act_title_1
**********************************************************************/

#include "R4A_Freenove_4WD_Car.h"
#include "Font_5x7.h"           // Font for the LED matrix display

//****************************************
// Constants
//****************************************

const uint8_t r4aLedMatrixColumnMap[R4A_VK16K33_MAX_COLUMNS] =
{
    0, 2, 4, 6, 8, 10, 12, 14, 1, 3, 5, 7, 9, 11, 13, 15
};

const uint8_t r4aLedMatrixRowPixelMap[R4A_VK16K33_MAX_ROWS] =
{
    0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80
};

//****************************************
// Globals
//****************************************

R4A_Freenove_4WD_Car r4aFreenove4wdCar;

//*********************************************************************
// Get the backup light color
uint32_t R4A_Freenove_4WD_Car::backupLightColorGet()
{
    return _backupLightColor;
}

//*********************************************************************
// Set the backup light color
void R4A_Freenove_4WD_Car::backupLightColorSet(uint32_t color)
{
    _backupLightColor = color;
    _updateLEDs = true;
}

//*********************************************************************
// Turn off the backup lights
void R4A_Freenove_4WD_Car::backupLightsOff()
{
    backupLightColorSet(R4A_LED_OFF);
}

//*********************************************************************
// Turn on the backup lights
void R4A_Freenove_4WD_Car::backupLightsOn()
{
    backupLightColorSet(R4A_LED_WHITE_RGB);
}

//*********************************************************************
// Toggle the backup lights
void R4A_Freenove_4WD_Car::backupLightsToggle()
{
    if (_backupLightColor)
        backupLightsOff();
    else
        backupLightsOn();
}

//*********************************************************************
// Get the brake light color
uint32_t R4A_Freenove_4WD_Car::brakeLightColorGet()
{
    return _brakeLightColor;
}

//*********************************************************************
// Set the brake light color
void R4A_Freenove_4WD_Car::brakeLightColorSet(uint32_t color)
{
    _brakeLightColor = color;
    _updateLEDs = true;
}

//*********************************************************************
// Turn off the brake lights
void R4A_Freenove_4WD_Car::brakeLightsOff()
{
    brakeLightColorSet(R4A_LED_OFF);
}

//*********************************************************************
// Turn on the brake lights
void R4A_Freenove_4WD_Car::brakeLightsOn()
{
    brakeLightColorSet(R4A_LED_RED);
}

//*********************************************************************
// Toggle the brake lights
void R4A_Freenove_4WD_Car::brakeLightsToggle()
{
    if (_brakeLightColor)
        brakeLightsOff();
    else
        brakeLightsOn();
}

//*********************************************************************
// Get the headlight color
uint32_t R4A_Freenove_4WD_Car::headlightColorGet()
{
    return _headlightColor;
}

//*********************************************************************
// Set the headlight color
void R4A_Freenove_4WD_Car::headlightColorSet(uint32_t color)
{
    _headlightColor = color;
    _updateLEDs = true;
}

//*********************************************************************
// Turn off the headlights
void R4A_Freenove_4WD_Car::headlightsOff()
{
    headlightColorSet(R4A_LED_OFF);
}

//*********************************************************************
// Turn on the headlights
void R4A_Freenove_4WD_Car::headlightsOn()
{
    headlightColorSet(R4A_LED_WHITE_RGB);
}

//*********************************************************************
// Toggle the headlights
void R4A_Freenove_4WD_Car::headlightsToggle()
{
    if (_headlightColor)
        headlightsOff();
    else
        headlightsOn();
}

//*********************************************************************
// Turn off all the LEDs
void R4A_Freenove_4WD_Car::ledsOff()
{
    _backupLightColor = R4A_LED_OFF;
    _brakeLightColor = R4A_LED_OFF;
    _headlightColor = R4A_LED_OFF;
    for (uint8_t led = 0; led < numberOfLEDs; led++)
        r4aLEDSetColorRgb(led, R4A_LED_OFF);
    _updateLEDs = true;
}

//*********************************************************************
// Use LEDs to indicate left turn
void R4A_Freenove_4WD_Car::ledsTurnLeft()
{
    _ledState = STATE_BLINK_L_OFF;
    _timer = 1;
    _updateLEDs = true;
}

//*********************************************************************
// Stop using the LEDs to indicate a turn
void R4A_Freenove_4WD_Car::ledsTurnOff()
{
    _ledState = STATE_OFF;
    _timer = 0;
    _updateLEDs = true;
}

//*********************************************************************
// Use LEDs to indicate right turn
void R4A_Freenove_4WD_Car::ledsTurnRight()
{
    _ledState = STATE_BLINK_R_OFF;
    _timer = 1;
    _updateLEDs = true;
}

//*********************************************************************
// Update the color of the LEDs
void R4A_Freenove_4WD_Car::ledsUpdate(uint32_t currentMsec)
{
    uint32_t color;
    uint32_t timer;
    bool updateNeeded;

    // Determine if the LEDs need updating
    updateNeeded = __atomic_exchange_1(&_updateLEDs, false, 0);

    // Determine if the timer is running
    timer = _timer;
    if (timer && ((currentMsec - timer) >= _blinkMsec))
    {
        // Handle the race where it was just turned off
        if (!__atomic_exchange_4(&_timer, currentMsec, 0))
            _timer = 0;
        updateNeeded = true;

        // Update the LED state
        if (_ledState)
        {
            if (_ledState == STATE_BLINK_L_OFF)
                _ledState = STATE_BLINK_L3;
            else if (_ledState == STATE_BLINK_R_OFF)
                _ledState = STATE_BLINK_R3;
            else
                _ledState += 1;
        }
    }

    // Determine if the timer is running
    if (updateNeeded)
    {
        // Turn off all the LEDs
        r4aLEDsOff();

        // Handle the blinkers
        switch (_ledState)
        {
        // O O O        O O O
        case STATE_BLINK_L_OFF:
        case STATE_BLINK_R_OFF:
        case STATE_OFF:
            break;

        // O O *        O O O
        case STATE_BLINK_L3:
            r4aLEDSetColorRgb(FRONT_L1, R4A_LED_YELLOW);
            if (_backupLightColor | _brakeLightColor)
                r4aLEDSetColorRgb(BACK_L1, R4A_LED_YELLOW);
            else
                r4aLEDSetColorRgb(BACK_L3, R4A_LED_YELLOW);
            break;

        // O * *        O O O
        case STATE_BLINK_L2:
            if (_backupLightColor | _brakeLightColor)
                r4aLEDSetColorRgb(BACK_L1, R4A_LED_OFF);
            else
            {
                r4aLEDSetColorRgb(BACK_L3, R4A_LED_YELLOW);
                r4aLEDSetColorRgb(BACK_L2, R4A_LED_YELLOW);
            }
            break;

        // * * *        O O O
        case STATE_BLINK_L1:
            r4aLEDSetColorRgb(FRONT_L1, R4A_LED_YELLOW);
            if (_backupLightColor | _brakeLightColor)
                r4aLEDSetColorRgb(BACK_L1, R4A_LED_YELLOW);
            else
            {
                r4aLEDSetColorRgb(BACK_L3, R4A_LED_YELLOW);
                r4aLEDSetColorRgb(BACK_L2, R4A_LED_YELLOW);
                r4aLEDSetColorRgb(BACK_L1, R4A_LED_YELLOW);
            }
            break;

        // O O O        * O O
        case STATE_BLINK_R3:
            r4aLEDSetColorRgb(FRONT_R1, R4A_LED_YELLOW);
            if (_backupLightColor | _brakeLightColor)
                r4aLEDSetColorRgb(BACK_R1, R4A_LED_YELLOW);
            else
                r4aLEDSetColorRgb(BACK_R3, R4A_LED_YELLOW);
            break;

        // O O O        * * O
        case STATE_BLINK_R2:
            if (_backupLightColor | _brakeLightColor)
                r4aLEDSetColorRgb(BACK_R1, R4A_LED_OFF);
            else
            {
                r4aLEDSetColorRgb(BACK_R3, R4A_LED_YELLOW);
                r4aLEDSetColorRgb(BACK_R2, R4A_LED_YELLOW);
            }
            break;

        // O O O        * * *
        case STATE_BLINK_R1:
            r4aLEDSetColorRgb(FRONT_R1, R4A_LED_YELLOW);
            if (_backupLightColor | _brakeLightColor)
                r4aLEDSetColorRgb(BACK_R1, R4A_LED_YELLOW);
            else
            {
                r4aLEDSetColorRgb(BACK_R3, R4A_LED_YELLOW);
                r4aLEDSetColorRgb(BACK_R2, R4A_LED_YELLOW);
                r4aLEDSetColorRgb(BACK_R1, R4A_LED_YELLOW);
            }
            break;
        }

        // Turn on or off the headlights
        color = _headlightColor;
        if (color)
        {
            r4aLEDSetColorRgb(FRONT_L2, color);
            r4aLEDSetColorRgb(FRONT_L3, color);
            r4aLEDSetColorRgb(FRONT_R3, color);
            r4aLEDSetColorRgb(FRONT_R2, color);
        }

        // Turn on the backup lights
        color = _backupLightColor;
        if (color)
        {
            r4aLEDSetColorRgb(BACK_L2, color);
            r4aLEDSetColorRgb(BACK_L3, color);
            r4aLEDSetColorRgb(BACK_R3, color);
            r4aLEDSetColorRgb(BACK_R2, color);
        }

        // Turn on the brake lights
        color = _brakeLightColor;
        if (color)
        {
            if ((_ledState < STATE_BLINK_L3) || (_ledState > STATE_BLINK_L_OFF))
                r4aLEDSetColorRgb(BACK_L1, color);
            r4aLEDSetColorRgb(BACK_L2, color);
            r4aLEDSetColorRgb(BACK_L3, color);
            r4aLEDSetColorRgb(BACK_R3, color);
            r4aLEDSetColorRgb(BACK_R2, color);
            if ((_ledState < STATE_BLINK_R3) || (_ledState > STATE_BLINK_R_OFF))
                r4aLEDSetColorRgb(BACK_R1, color);
        }
    }

    // Update the LED colors
    r4aLEDUpdate(true);
}
