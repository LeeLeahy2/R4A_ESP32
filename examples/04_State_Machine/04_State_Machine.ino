/**********************************************************************
  04_State_Machine.ino

  Sample sketch to demonstrate state machine design and implementation

  State Machines, State Diagrams and State Variables
  --------------------------------------------------

  State machines are a mathmetical representation of the operation of a
  system.  They are best represented by a diagram consisting of named
  states and arrows between states.  The text next to the transition arrows
  documents what inputs caused the transition and what changes are made
  during that transition.  Note that changes are only made during the
  transition to the next state.  Within the state, checks are made to
  determine if a transition is necessary and if so, which transition is
  taken.

  Traffic Light Example:

            .---------.
     .----->|   RED   |<-----------------------------.
     |      '---------'                              |
     |           |                                   |
     |           | lightOnTimesMsec[0] Expired       | Start timer
     |           | Turn on green light               | Turn on red light
     |           | Start timer                       |
     |           V                                   |
     |      .---------.                         .---------.
     |      |  GREEN  |                         | Reset 1 |
     |      '---------'                         '---------'
     |           |
     |           | lightOnTimesMsec[1] Expired
     |           | Turn on yellow light
     |           | Start timer
     |           V
     |      .---------.                         .---------.
     |      |  YELLOW |                         | Reset 2 |
     |      '---------'                         '---------'
     |           |                                   |
     |           | lightOnTimesMsec[2] Expired       |
     |           | Turn on red light                 | Turn on red light
     |           | Start timer                       | Start timer
     |           V                                   |
     |      .---------.                              |
     |      | BOTH RED|<-----------------------------'
     |      '---------'
     |           |
     |           | bothLightsOnTimeMsec Expired
     |           | Start timer
     |           |
     '-----------'

**********************************************************************/

//****************************************
// Includes
//****************************************

#include <R4A_ESP32.h>

//****************************************
// Constants
//****************************************

const uint32_t lightOnTimesMsec[3] =
{
    2 * R4A_MILLISECONDS_IN_A_SECOND,   // Red
    10 * R4A_MILLISECONDS_IN_A_SECOND,  // Green
    3 * R4A_MILLISECONDS_IN_A_SECOND,   // Yellow
};
const uint32_t lightOnEntries = sizeof(lightOnTimesMsec) / sizeof(lightOnTimesMsec[0]);

enum TRAFFIC_LIGHT_STATES
{
    RED = 0,
    GREEN,
    YELLOW,
    BOTH_RED,
    RESET_1,
    RESET_2,
};

const char * const stateNames[] =
{
    "  Red  ", " Green ", "Yellow ", "  Red  ", "Reset 1", "Reset 2"
};

//****************************************
// Globals
//****************************************

uint32_t bothLightsOnTimeMsec;

//*********************************************************************
// Update the traffic light state based upon the time that has expired
bool trafficLightState(uint32_t currentMsec,
                       TRAFFIC_LIGHT_STATES * state,
                       uint32_t * timer)
{
    bool lightChanged;

    // Perform the traffic light operation
    lightChanged = false;
    switch (*state)
    {
    case RED:
        if ((currentMsec - *timer) >= lightOnTimesMsec[RED])
        {
            *timer = currentMsec;
            lightChanged = true;
            // Turn on green light
            *state = GREEN;
        }
        break;

    case GREEN:
        if ((currentMsec - *timer) >= lightOnTimesMsec[GREEN])
        {
            *timer = currentMsec;
            lightChanged = true;
            // Turn on red light
            *state = YELLOW;
        }
        break;

    case YELLOW:
        if ((currentMsec - *timer) >= lightOnTimesMsec[YELLOW])
        {
            *timer = currentMsec;
            lightChanged = true;
            // Turn on red light
            *state = BOTH_RED;
        }
        break;

    case BOTH_RED:
        if ((currentMsec - *timer) >= bothLightsOnTimeMsec)
        {
            *timer = currentMsec;
            lightChanged = true;
            *state = RED;
        }
        break;

    case RESET_1:
        *timer = currentMsec;
        // Turn on red light
        *state = RED;
        lightChanged = true;
        break;

    case RESET_2:
        *timer = currentMsec;
        // Turn on red light
        *state = BOTH_RED;
        lightChanged = true;
        break;
    }
    return lightChanged;
}

//*********************************************************************
// Update both traffic lights
void trafficLightUpdate()
{
    uint32_t currentMsec;
    static bool lightChanged = true;
    static TRAFFIC_LIGHT_STATES state1 = RESET_1;
    static TRAFFIC_LIGHT_STATES state2 = RESET_2;
    static uint32_t timer1;
    static uint32_t timer2;

    // Get the current time
    currentMsec = millis();

    // Display the traffic lights
    if (lightChanged)
    {
        int hours;
        int minutes;
        int seconds;

        seconds = currentMsec / R4A_MILLISECONDS_IN_A_SECOND;
        minutes = seconds / R4A_SECONDS_IN_A_MINUTE;
        seconds -= minutes * R4A_SECONDS_IN_A_MINUTE;
        hours = minutes / R4A_MINUTES_IN_AN_HOUR;
        minutes -= hours * R4A_MINUTES_IN_AN_HOUR;
        Serial.printf("%2d:%02d:%02d    %s    %s\r\n",
                      hours, minutes, seconds,
                      stateNames[state1], stateNames[state2]);
        lightChanged = false;
    }

    // Update the two traffic lights
    lightChanged |= trafficLightState(currentMsec, &state1, &timer1);
    lightChanged |= trafficLightState(currentMsec, &state2, &timer2);
}

//*********************************************************************
// Entry point for the application
void setup()
{
    uint32_t totalMsec;

    // Initialize the USB serial port
    Serial.begin(115200);
    Serial.println();
    Serial.printf("%s\r\n", __FILE__);

    // Compute the both on time
    totalMsec = 0;
    for (int index = 0; index < lightOnEntries; index++)
        totalMsec += lightOnTimesMsec[index];
    bothLightsOnTimeMsec = totalMsec;

    // Display the header
    Serial.println();
    Serial.println("  Time      Light 1    Light 2");
    Serial.println("--------    -------    -------");
}

//*********************************************************************
// Idle loop
void loop()
{
    trafficLightUpdate();
}
