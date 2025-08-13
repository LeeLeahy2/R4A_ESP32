/**********************************************************************
  I2S.cpp

  Robots-For-All (R4A)
  I2S controller support
**********************************************************************/

#include "R4A_ESP32.h"
#include <hal/clk_tree_hal.h>   // IDF built-in
#include <hal/ledc_hal.h>       // IDF built-in
#include <hal/ledc_ll.h>        // IDF built-in

//****************************************
// Constants
//****************************************

const char * r4aEsp32I2sNames[] =
{
    "I2S_FIFO_WR_REG",          // 0x00
    "I2S_FIFO_RD_REG",          // 0x04
    "I2S_CONF_REG",             // 0x08
    "I2S_INT_RAW_REG",          // 0x0c
    "I2S_INT_ST_REG",           // 0x10
    "I2S_INT_ENA_REG",          // 0x14
    "I2S_INT_CLR_REG",          // 0x18
    "I2S_TIMING_REG",           // 0x1c
    "I2S_FIFO_CONF_REG",        // 0x20
    "I2S_RXEOF_NUM_REG",        // 0x24
    "I2S_CONF_SINGLE_DATA_REG", // 0x28
    "I2S_CONF_CHAN_REG",        // 0x2c
    "I2S_OUT_LINK_REG",         // 0x30
    "I2S_IN_LINK_REG",          // 0x34
    "I2S_OUT_EOF_DES_ADDR_REG", // 0x38
    "I2S_IN_EOF_DES_ADDR_REG",  // 0x3c
    "I2S_OUT_EOF_BFR_DES_ADDR_REG", // 0x40
    "I2S_RESERVED_0X44",        // 0x44
    "I2S_INLINK_DSCR_REG",      // 0x48
    "I2S_INLINK_DSCR_BF0_REG",  // 0x4c
    "I2S_INLINK_DSCR_BF1_REG",  // 0x50
    "I2S_OUTLINK_DSCR_REG",     // 0x54
    "I2S_OUTLINK_DSCR_BF0_REG", // 0x58
    "I2S_OUTLINK_DSCR_BF1_REG", // 0x5c
    "I2S_LC_CONF_REG",          // 0x60
    "I2S_RESERVED_0X64",        // 0x64
    "I2S_RESERVED_0X68",        // 0x68
    "I2S_LC_STATE0_REG",        // 0x6c
    "I2S_LC_STATE1_REG",        // 0x70
    "I2S_LU_HUNG_CONF_REG",     // 0x74
    nullptr, //"reserved_0x78",            // 0x78
    nullptr, //"reserved_0x7c",            // 0x7c
    nullptr, //"reserved_0x80",            // 0x80
    nullptr, //"reserved_0x84",            // 0x84
    nullptr, //"reserved_0x88",            // 0x88
    nullptr, //"reserved_0x8c",            // 0x8c
    nullptr, //"reserved_0x90",            // 0x90
    nullptr, //"reserved_0x94",            // 0x94
    nullptr, //"reserved_0x98",            // 0x98
    nullptr, //"reserved_0x9c",            // 0x9c
    "I2S_CONF1_REG",            // 0xa0
    "I2S_PD_CONF_REG",          // 0xa4
    "I2S_CONF2_REG",            // 0xa8
    "I2S_CLKM_CONF_REG",        // 0xac
    "I2S_SAMPLE_RATE_CONF_REG", // 0xb0
    "I2S_PDM_CONF_REG",         // 0xb4
    "I2S_PDM_FREQ_CONF_REG",    // 0xb8
    "I2S_STATE_REG",            // 0xbc
};
const uint32_t r4aEsp32I2sNamesBytes = sizeof(r4aEsp32I2sNames);
const uint32_t r4aEsp32I2sNamesEntries = sizeof(r4aEsp32I2sNames) / sizeof(r4aEsp32I2sNames[0]);

//*********************************************************************
// Display the I2S registers
void r4aEsp32I2sDisplayRegisters(uintptr_t i2sAddress, Print * display)
{
    uint32_t * i2s;

    i2s = (uint32_t *)i2sAddress;

    // Validate the I2C address;
    if ((i2sAddress != R4A_ESP32_I2S_0_CONTROLLER)
        && (i2sAddress != R4A_ESP32_I2S_1_CONTROLLER))
    {
        display->printf("ERROR: Invalid I2S controller address!\r\n");
        return;
    }
    display->printf("I2S Controller @ %p\r\n", (void *)i2s);

    // Display the registers
    for (int index = 0; index < r4aEsp32I2sNamesEntries; index++)
    {
        if (r4aEsp32I2sNames[index])
            display->printf("%p: 0x%08x, %s\r\n", &i2s[index], i2s[index], r4aEsp32I2sNames[index]);
        else
            display->printf("0x%08x:             Reserved\r\n", &i2s[index]);
    }
}

//*********************************************************************
// Verify the I2S tables
void r4aEsp32I2sVerifyTables()
{
    // Verify register names
    if (r4aEsp32I2sNamesBytes != sizeof(R4A_ESP32_I2S_REGS))
        r4aReportFatalError("Please fix R4A_ESP32_I2S_NAMES to match R4A_ESP32_I2S_REGS");
}

//*********************************************************************
// Display the I2S 0 registers
void r4aEsp32MenuI2s0Display(const R4A_MENU_ENTRY * menuEntry,
                             const char * command,
                             Print * display)
{
    r4aEsp32I2sDisplayRegisters(R4A_ESP32_I2S_0_CONTROLLER, display);
}

//*********************************************************************
// Display the I2S 1 registers
void r4aEsp32MenuI2s1Display(const R4A_MENU_ENTRY * menuEntry,
                             const char * command,
                             Print * display)
{
    r4aEsp32I2sDisplayRegisters(R4A_ESP32_I2S_1_CONTROLLER, display);
}
