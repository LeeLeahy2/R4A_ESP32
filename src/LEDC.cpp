/**********************************************************************
  LEDC.cpp

  Robots-For-All (R4A)
  LED controller support
**********************************************************************/

#include "R4A_ESP32.h"
#include <hal/clk_tree_hal.h>   // IDF built-in
#include <hal/ledc_hal.h>       // IDF built-in
#include <hal/ledc_ll.h>        // IDF built-in

//****************************************
// Support routines
//****************************************

extern "C"
{

// The following definition came from
// Espressif idf esp-camera/driver/private_include/xclk.h
esp_err_t xclk_timer_conf(int ledc_timer, int xclk_freq_hz);

}

//*********************************************************************
// Verify the channel number
bool r4aEsp32LedCValidChannel(uint8_t channelNumber, Print * display)
{
    if (channelNumber < LEDC_CHANNEL_MAX)
        return true;

    // Display the channel number error
    if (display)
        display->printf("ERROR: Invalid channel number, must be less than 8\r\n");
    else
        log_e("ERROR: Invalid channel number, must be less than 8");
    return false;
}

//*********************************************************************
// Verify the group number
bool r4aEsp32LedCValidGroup(uint8_t groupNumber, Print * display)
{
    if (groupNumber < 2)
        return true;

    // Display the group number error
    if (display)
        display->printf("ERROR: Invalid group number, must be less than 2\r\n");
    else
        log_e("ERROR: Invalid group number, must be less than 2");
    return false;
}

//*********************************************************************
// Verify the timer resolution
bool r4aEsp32LedCValidResolution(uint8_t resolutionBits, Print * display)
{
    // Validate the resolution
    if ((resolutionBits >= 1) && (resolutionBits < 8))
        return true;

    // Display the resolution error
    if (display)
        display->printf("ERROR: Invalid resolutionBits value, use (1 - 7)\r\n");
    else
        log_e("ERROR: Invalid resolutionBits value, use (1 - 7)");
    return false;
}

//*********************************************************************
// Verify the timer number
bool r4aEsp32LedCValidTimer(uint8_t timerNumber, Print * display)
{
    if (timerNumber < LEDC_TIMER_MAX)
        return true;

    // Display the timer number error
    if (display)
        display->printf("ERROR: Invalid timer number, must be less than 4\r\n");
    else
        log_e("ERROR: Invalid timer number, must be less than 4");
    return false;
}

//****************************************
// LEDC API
//****************************************

//*********************************************************************
// Display a LEDC Channel
void r4aEsp32LedCChannelDisplay(uint8_t groupNumber,
                                uint8_t channelNumber,
                                bool displayAll,
                                Print * display,
                                const char * indent)
{
    // Validate the group and timer values
    if (display
        && r4aEsp32LedCValidGroup(groupNumber, display)
        && r4aEsp32LedCValidChannel(channelNumber, display))
    {
        bool done;
        double phaseDegrees;
        double pulsePercent;
        uint32_t value;

        // Get the channel
        R4A_LEDC_CHANNEL_t * channel = groupNumber ? &r4aEsp32LedC->lsCh[channelNumber]
                                                   : &r4aEsp32LedC->hsCh[channelNumber];

        // Get the CONF0 value
        value = channel->conf0;
        uint32_t paraUp = value & LEDC_CH_CONF0_PARA_UP_LSCH;
        uint32_t idleLv = value & LEDC_CH_CONF0_IDLE_LV;
        uint32_t sigOutEn = value & LEDC_CH_CONF0_SIG_OUT_EN;
        uint32_t timerNumber = value & LEDC_CH_CONF0_TIMER_SEL;

        // Display the header
        done = (displayAll == false) && (sigOutEn == false);
        display->printf("%s%s Speed Channel %d%s\r\n",
                        indent, groupNumber ? "Low" : "High", channelNumber,
                        done ? ": disabled" : "");
        if (done)
            return;

        // Display the CONF0 value
        display->printf("    %sCONF0  0x%08lx: %sOutput %s, Idle %d, Timer %d\r\n",
                        indent,
                        value,
                        paraUp ? "PARA_UP, " : "",
                        sigOutEn ? "enabled" : "disabled",
                        idleLv ? 1 : 0,
                        timerNumber);

        // Display the HPOINT value
        display->printf("    %sHPOINT 0x%08lx\r\n", indent, channel->hPoint);

        // Display the DUTY value
        display->printf("    %sDUTY   0x%08lx\r\n", indent, channel->duty);

        // Display the CONF1 value
        value = channel->conf1;
        uint32_t dutyStart = value & LEDC_CH_CONF1_DUTY_START;
        uint32_t dutyInc = value & LEDC_CH_CONF1_DUTY_INC;
        uint32_t dutyNum = (value & LEDC_CH_CONF1_DUTY_NUM) >> 20;
        uint32_t dutyCycle = (value & LEDC_CH_CONF1_DUTY_CYCLE) >> 10;
        uint32_t dutyScale = value & LEDC_CH_CONF1_DUTY_SCALE;
        display->printf("    %sCONF1  0x%08lx: %s%s, %ld adjustments, each after %ld clocks, Adjustment %ld\r\n",
                        indent,
                        channel->conf1,
                        dutyStart ? "Start" : "",
                        dutyInc ? "Increment" : "Decrement",
                        dutyNum,
                        dutyCycle,
                        dutyScale);

        // Display the frequency and duty cycle
        R4A_LEDC_TIMER_t * timer = groupNumber ? &r4aEsp32LedC->lsTimer[timerNumber]
                                               : &r4aEsp32LedC->hsTimer[timerNumber];
        uint32_t counterWidth = 1 << (timer->conf & LEDC_TIM_CONF_DUTY_RES);
        uint32_t clockKHz = r4aEsp32LedCTimerHz(groupNumber, timerNumber, display) / 1000;
        phaseDegrees = 0;
        pulsePercent = 0;
        if (counterWidth)
        {
            phaseDegrees = (360. * (double)channel->hPoint) / (double)counterWidth;
            pulsePercent = (100. * (double)channel->duty) / (16. * (double)counterWidth);
        }
        display->printf("    %sFrequency: %ld.%03ld MHz, %6.2f %% duty cycle, phase %6.2f degrees\r\n",
                        indent,
                        clockKHz / 1000, clockKHz % 1000,
                        pulsePercent,
                        phaseDegrees);
    }
}

//*********************************************************************
// Get the LEDC APB_CLOCK frequency in Hz
uint32_t r4aEsp32LedCClockHz()
{
    return (r4aEsp32LedC->conf & LEDC_CONF_APB_CLK_SEL)
        ? r4aEsp32ClockGetApb() : clk_hal_lp_slow_get_freq_hz();
}

//*********************************************************************
// Display the LEDC
void r4aEsp32LedCDisplay(bool displayAll, Print * display)
{
    const char * indent = "    ";
    int index;

    if (display)
    {
        display->printf("LED Controller\r\n");
        uint32_t apbClkKHz = r4aEsp32LedCClockHz() / 1000;
        display->printf("    CONF   0x%08lx: APB_CLK = %ld.%03ld MHz\r\n",
                        r4aEsp32LedC->conf,
                        apbClkKHz / 1000,
                        apbClkKHz % 1000);
    }

    // Display the channels
    for (index = 0; index < 8; index++)
        r4aEsp32LedCChannelDisplay(0, index, displayAll, display, indent);
    for (index = 0; index < 8; index++)
        r4aEsp32LedCChannelDisplay(1, index, displayAll, display, indent);

    // Display the timers
    for (index = 0; index < 4; index++)
        r4aEsp32LedCTimerDisplay(0, index, displayAll, display, indent);
    for (index = 0; index < 4; index++)
        r4aEsp32LedCTimerDisplay(1, index, displayAll, display, indent);

    // Display the header
    if (display)
    {
        display->printf("%sLED Controller Interrupts\r\n", indent);
        display->printf("   %sINT_RAW 0x%08lx\r\n", indent, r4aEsp32LedC->intRaw);
        display->printf("   %sINT_ST  0x%08lx\r\n", indent, r4aEsp32LedC->intSt);
        display->printf("   %sINT_ENA 0x%08lx\r\n", indent, r4aEsp32LedC->intEna);
        display->printf("   %sINT_CLR 0x%08lx\r\n", indent, r4aEsp32LedC->intClr);
    }
};

//*********************************************************************
// Configure a LEDC timer
bool r4aEsp32LedCTimerConfig(bool lowSpeedMode,
                             int timerNumber,
                             uint32_t clockHz,
                             uint32_t resolutionBits,
                             Print * display)
{
    esp_err_t status;
    ledc_timer_config_t timer_conf;

    do
    {
        // Validate the timer
        status = -1;
        if (r4aEsp32LedCValidTimer(timerNumber, display) == false)
            break;

        // Validate the timer resolution
        if (r4aEsp32LedCValidResolution(resolutionBits, display) == false)
            break;

        // Configure the timer
        timer_conf.speed_mode = lowSpeedMode ? LEDC_LOW_SPEED_MODE
                                             : LEDC_HIGH_SPEED_MODE;
        timer_conf.timer_num = (ledc_timer_t)timerNumber;
        timer_conf.freq_hz = clockHz;
        timer_conf.duty_resolution = (ledc_timer_bit_t)resolutionBits;
        timer_conf.deconfigure = false;
        timer_conf.clk_cfg = LEDC_AUTO_CLK;
        status = ledc_timer_config(&timer_conf);
        if (status != ESP_OK)
        {
            log_e("ERROR: r4aEsp32LedCTimerConfig failed for freq %d, status=%x, %s",
                  clockHz, status, esp_err_to_name(status));
            if (display)
                display->printf("ERROR: r4aEsp32LedCTimerConfig failed for freq %d, status=%x, %s\r\n",
                                clockHz, status, esp_err_to_name(status));
        }
    } while (0);

    // Return the configuration status
    return (status == ESP_OK);
}

#if 0
esp_err_t ledc_timer_config(const ledc_timer_config_t *timer_conf)
{
    LEDC_ARG_CHECK(timer_conf != NULL, "timer_conf");
    uint32_t freq_hz = timer_conf->freq_hz;
    uint32_t duty_resolution = timer_conf->duty_resolution;
    uint32_t timer_num = timer_conf->timer_num;
    uint32_t speed_mode = timer_conf->speed_mode;
    LEDC_ARG_CHECK(speed_mode < LEDC_SPEED_MODE_MAX, "speed_mode");
    LEDC_ARG_CHECK(timer_num < LEDC_TIMER_MAX, "timer_num");
    if (timer_conf->deconfigure) {
        return ledc_timer_del(speed_mode, timer_num);
    }
    LEDC_ARG_CHECK(!((timer_conf->clk_cfg == LEDC_USE_RC_FAST_CLK) && (speed_mode != LEDC_LOW_SPEED_MODE)), "Only low speed channel support RC_FAST_CLK");
    if (freq_hz == 0 || duty_resolution == 0 || duty_resolution >= LEDC_TIMER_BIT_MAX) {
        ESP_LOGE(LEDC_TAG, "freq_hz=%"PRIu32" duty_resolution=%"PRIu32, freq_hz, duty_resolution);
        return ESP_ERR_INVALID_ARG;
    }

    if (!ledc_speed_mode_ctx_create(speed_mode) && !p_ledc_obj[speed_mode]) {
        return ESP_ERR_NO_MEM;
    }

    esp_err_t ret = ledc_set_timer_div(speed_mode, timer_num, timer_conf->clk_cfg, freq_hz, duty_resolution);
    if (ret == ESP_OK) {
        /* Make sure timer is running and reset the timer. */
        ledc_timer_resume(speed_mode, timer_num);
        ledc_timer_rst(speed_mode, timer_num);
    }
    return ret;
}

    // Attempt to initialize the low speed timer
    status = xclk_timer_conf(timer, clockHz);
    if (status != ESP_OK)
    {
        log_e("ERROR: Failed to initialize the LED controller timer, status: %d, %s",
              status, esp_err_to_name(status));
        if (display)
            display->printf("ERROR: Failed to initialize the LED controller timer, status: %d, %s\r\n",
                            status, esp_err_to_name(status));
    }
    return (status == ESP_OK);
}
#endif  // 0

//*********************************************************************
// Display a LEDC timer
void r4aEsp32LedCTimerDisplay(uint8_t groupNumber,
                              uint8_t timerNumber,
                              bool displayAll,
                              Print * display,
                              const char * indent)
{
    if (display
        && r4aEsp32LedCValidGroup(groupNumber, display)
        && r4aEsp32LedCValidTimer(timerNumber, display))
    {
        uint32_t clockKHz;
        bool done;

        // Get the timer
        R4A_LEDC_TIMER_t * timer = groupNumber ? &r4aEsp32LedC->lsTimer[timerNumber]
                                               : &r4aEsp32LedC->hsTimer[timerNumber];

        // Display the header
        uint32_t rst = timer->conf & LEDC_TIM_CONF_RST;
        done = (displayAll == false) && rst;
        display->printf("%s%s Speed Timer %d%s\r\n",
                        indent, groupNumber ? "Low" : "High", timerNumber,
                        done ? ": disabled" : "");
        if (done)
            return;

        // Display the CONF register
        uint32_t tickSel = timer->conf & LEDC_TIM_CONF_TICK_SEL;
        uint32_t apbClkHz = r4aEsp32LedCClockHz();
        uint32_t KHz = (tickSel ? r4aEsp32LedCClockHz() : r4aEsp32ClockGetRefTick())
            / 1000;
        uint32_t pause = timer->conf & LEDC_TIM_CONF_PAUSE;
        uint32_t divNum = (timer->conf & LEDC_TIM_CONF_DIV_NUM) >> 5;
        uint32_t dutyRes = timer->conf & LEDC_TIM_CONF_DUTY_RES;
        clockKHz = r4aEsp32LedCTimerHz(groupNumber, timerNumber, display) / 1000;
        display->printf("    %sCONF   0x%08lx: %s (%ld.%03ld MHz) * 256 / (%ld * (1 << %d)) = %ld.%03d MHz%s%s\r\n",
                        indent,
                        timer->conf,
                        tickSel ? "APB_CLK" : "REF_TICK",
                        KHz / 1000, KHz % 1000,
                        divNum,
                        dutyRes,
                        clockKHz / 1000, clockKHz % 1000,
                        rst ? ", Reset" : "",
                        pause ? ", Pause" : "");

        // Display the VALUE register
        display->printf("    %sVALUE  0x%08lx\r\n", indent, timer->value);
    }
}

//*********************************************************************
// Get the LEDC timer clock frequency
uint32_t r4aEsp32LedCTimerHz(uint8_t groupNumber,
                             uint8_t timerNumber,
                             Print * display)
{
    uint32_t timerHz;

    timerHz = 0;
    if (r4aEsp32LedCValidGroup(groupNumber, display)
        && r4aEsp32LedCValidTimer(timerNumber, display))
    {
        uint32_t clockKHz;

        // Get the timer
        R4A_LEDC_TIMER_t * timer = groupNumber ? &r4aEsp32LedC->lsTimer[timerNumber]
                                               : &r4aEsp32LedC->hsTimer[timerNumber];

        // Get the input clock frequency
        uint32_t inputHz = (timer->conf & LEDC_TIM_CONF_TICK_SEL)
                         ? r4aEsp32LedCClockHz() : r4aEsp32ClockGetRefTick();

        // Determine the divisor
        uint32_t divNum = (timer->conf & LEDC_TIM_CONF_DIV_NUM) >> 5;
        uint32_t dutyRes = timer->conf & LEDC_TIM_CONF_DUTY_RES;
        uint32_t divisor = divNum * (1 << dutyRes);

        // Compute the timer frequency
        if (divisor)
            timerHz = (((uint64_t)inputHz << 8)) / divisor;
    }
    return timerHz;
}

//*********************************************************************
// Display the LED controller registers
void r4aEsp32MenuLedCDisplay(const R4A_MENU_ENTRY * menuEntry,
                             const char * command,
                             Print * display)
{
    // Display the LED controller registers
    r4aEsp32LedCDisplay(true, display);
}

//*********************************************************************
// Display a summary of the LED controller registers
void r4aEsp32MenuLedCDisplaySummary(const R4A_MENU_ENTRY * menuEntry,
                                    const char * command,
                                    Print * display)
{
    // Display the LED controller registers
    r4aEsp32LedCDisplay(false, display);
}
