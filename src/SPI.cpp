/**********************************************************************
  SPI.cpp

  Robots-For-All (R4A)
  SPI support
**********************************************************************/

#include "R4A_ESP32.h"

//****************************************
// Constants
//****************************************

const char * r4aEsp32SpiNames[] =
{
    "SPI_CMD",                  // 0x000
    "SPI_ADDR",                 // 0x004
    "SPI_CTRL",                 // 0x008
    nullptr,                    // 0x00c
    nullptr,                    // 0x010
    "SPI_CTRL2",                // 0x014
    "SPI_CLOCK",                // 0x018
    "SPI_USER",                 // 0x01c
    "SPI_USER1",                // 0x020
    "SPI_USER2",                // 0x024
    "SPI_MOSI_DLEN",            // 0x028
    nullptr,                    // 0x02c
    "SPI_SLV_WR_STATUS",        // 0x030
    "SPI_PIN_REG",              // 0x034
    "SPI_SLAVE",                // 0x038
    "SPI_SLAVE1",               // 0x03c
    "SPI_SLAVE2",               // 0x040
    nullptr,                    // 0x044
    "SPI_SLV_WRBUF_DLEN",       // 0x048
    "SPI_SLV_RDBUF_DLEN",       // 0x04c
    nullptr,                    // 0x050
    nullptr,                    // 0x054
    nullptr,                    // 0x058
    nullptr,                    // 0x05c
    nullptr,                    // 0x060
    "SPI_SLV_RD_BIT",           // 0x064
    nullptr,                    // 0x068
    nullptr,                    // 0x06c
    nullptr,                    // 0x070
    nullptr,                    // 0x074
    nullptr,                    // 0x078
    nullptr,                    // 0x07c
    "SPI_W0",                   // 0x080
    "SPI_W1",                   // 0x084
    "SPI_W2",                   // 0x088
    "SPI_W3",                   // 0x08c
    "SPI_W4",                   // 0x090
    "SPI_W5",                   // 0x094
    "SPI_W6",                   // 0x098
    "SPI_W7",                   // 0x09c
    "SPI_W8",                   // 0x0a0
    "SPI_W9",                   // 0x0a4
    "SPI_W10",                  // 0x0a8
    "SPI_W11",                  // 0x0ac
    "SPI_W12",                  // 0x0b0
    "SPI_W13",                  // 0x0b4
    "SPI_W14",                  // 0x0b8
    "SPI_W15",                  // 0x0bc
    nullptr,                    // 0x0c0
    nullptr,                    // 0x0c4
    nullptr,                    // 0x0c8
    nullptr,                    // 0x0cc
    nullptr,                    // 0x0d0
    nullptr,                    // 0x0d4
    nullptr,                    // 0x0d8
    nullptr,                    // 0x0dc
    nullptr,                    // 0x0e0
    nullptr,                    // 0x0e4
    nullptr,                    // 0x0e8
    nullptr,                    // 0x0ec
    nullptr,                    // 0x0f0
    nullptr,                    // 0x0f4
    nullptr,                    // 0x0f8
    nullptr,                    // 0x0fc
    "SPI_DMA_CONF",             // 0x100
    "SPI_DMA_OUT_LINK",         // 0x104
    "SPI_DMA_IN_LINK",          // 0x108
    "SPI_DMA_STATUS",           // 0x10c
    "SPI_DMA_INT_ENA",          // 0x110
    "SPI_DMA_INT_RAW",          // 0x114
    "SPI_DMA_INT_ST",           // 0x118
    "SPI_DMA_INT_CLR",          // 0x11c
    "SPI_IN_ERR_EOF_DES_ADDR",  // 0x120
    "SPI_IN_SUC_EOF_DES_ADDR",  // 0x124
    "SPI_INLINK_DSCR",          // 0x128
    "SPI_INLINK_DSCR_BFO",      // 0x12c
    "SPI_INLINK_DSCR_BF1",      // 0x130
    "SPI_OUT_EOF_BFR_DES_ADDR", // 0x134
    "SPI_OUT_EOF_DES_ADDR",     // 0x138
    "SPI_OUTLINK_DSCR",         // 0x13c
    "SPI_OUTLINK_DSCR_BF0",     // 0x140
    "SPI_OUTLINK_DSCR_BF1",     // 0x144
    "SPI_DMA_RSTATUS",          // 0x148
    "SPI_DMA_TSTATUS",          // 0x14c
};
const uint32_t r4aEsp32SpiNamesBytes = sizeof(r4aEsp32SpiNames);
const uint32_t r4aEsp32SpiNamesEntries = sizeof(r4aEsp32SpiNames) / sizeof(r4aEsp32SpiNames[0]);

//*********************************************************************
// Initialize the SPI controller
// Inputs:
//   spi: Address of an R4A_ESP32_SPI data structure
//   spiNumber: Number of the SPI controller
//   pinMOSI: SPI TX data pin number
//   clockHz: SPI clock frequency in Hertz
// Outputs:
//   Return true if successful and false upon failure
bool r4aEsp32SpiBegin(struct _R4A_ESP32_SPI * spi,
                      uint8_t spiNumber,
                      uint8_t pinMOSI,
                      uint32_t clockHz)
{
    // Determine the SPI clock divider
    spi->_div = spiFrequencyToClockDiv(clockHz);

    // Configure the SPI device
    spi->_spi = spiStartBus(spiNumber, spi->_div, SPI_MODE0, SPI_MSBFIRST);
    if (spi->_spi)
    {
        // Connect the SPI TX output to the MOSI pin
        spiAttachMOSI(spi->_spi, pinMOSI);

        // Set the routine addresses
        spi->spi.allocateDmaBuffer = r4aEsp32AllocateDmaBuffer;
        spi->spi.transfer = r4aEsp32SpiTransfer;
        return true;
    }
    return false;
}

//*********************************************************************
// Translate a controller number into a controller base register address
R4A_ESP32_SPI_REGS * r4aEsp32SpiControllerAddress(uint8_t number)
{
    const uintptr_t controller[] =
    {
        R4A_ESP32_SPI_0_CONTROLLER,
        R4A_ESP32_SPI_1_CONTROLLER,
        R4A_ESP32_SPI_2_CONTROLLER,
        R4A_ESP32_SPI_3_CONTROLLER
    };
    const int controllerCount = sizeof(controller) / sizeof(controller[0]);

    // Validate the controller number
    if (number >= controllerCount)
    {
        Serial.printf("Invalid SPI controller number!");
        return nullptr;
    }
    return (R4A_ESP32_SPI_REGS *)controller[number];
};

//*********************************************************************
// Display the SPI registers
void r4aEsp32SpiDisplayRegisters(uintptr_t spiAddress, Print * display)
{
    uint8_t controller;
    uint32_t * spi;
    uint32_t value;

    // Validate display
    if (!display)
        return;

    // Validate the I2C address;
    spi = (uint32_t *)spiAddress;
    switch ((uintptr_t)spi)
    {
    default:
        display->printf("ERROR: Invalid SPI controller address!\r\n");
        return;
    case R4A_ESP32_SPI_0_CONTROLLER: controller = 0; break;
    case R4A_ESP32_SPI_1_CONTROLLER: controller = 1; break;
    case R4A_ESP32_SPI_2_CONTROLLER: controller = 2; break;
    case R4A_ESP32_SPI_3_CONTROLLER: controller = 3; break;
    }

    // Display the controller number and address
    display->printf("SPI Controller %d @ %p\r\n", controller, (void *)spi);

    // Display the registers
    for (int index = 0; index < r4aEsp32SpiNamesEntries; index++)
    {
        uint32_t value = spi[index];
        if (r4aEsp32SpiNames[index])
            display->printf("%p: 0x%08x, %s\r\n", &spi[index], value, r4aEsp32SpiNames[index]);
        else
            display->printf("0x%08x:             Reserved\r\n", &spi[index]);

        // Parse the register
        switch ((uintptr_t)(&spi[index]) & 0x1ff)
        {
        case 0x18:  // SPI_CLOCK
            r4aEsp32SpiGetClock((R4A_ESP32_SPI_REGS *)spiAddress, display);
            break;
        }
    }
}

//*********************************************************************
// Get the SPI clock frequency
uint32_t r4aEsp32SpiGetClock(R4A_ESP32_SPI_REGS * spi, Print * display)
{
    uint32_t clkcnt_h;
    uint32_t clkcnt_l;
    uint32_t clkcnt_n;
    uint32_t clockHz;
    uint8_t controller;
    uint32_t kHz;
    uint32_t mHz;
    uint32_t pre;
    uint32_t sysclk;
    uint32_t value;

    // Validate the I2C address;
    switch ((uintptr_t)spi)
    {
    default:
        if (display)
            display->printf("ERROR: Invalid SPI controller address!\r\n");
        return 0;
    case R4A_ESP32_SPI_0_CONTROLLER: controller = 0; break;
    case R4A_ESP32_SPI_1_CONTROLLER: controller = 1; break;
    case R4A_ESP32_SPI_2_CONTROLLER: controller = 2; break;
    case R4A_ESP32_SPI_3_CONTROLLER: controller = 3; break;
    }

    // Get the SPI clock register value
    value = spi->SPI_CLOCK;

    // Parse the register value
    sysclk = (value >> 31) & 1;
    if (sysclk)
    {
        pre = 0;
        clkcnt_n = 0;
        clkcnt_h = 0;
        clkcnt_l = 0;
    }
    else
    {
        pre = (value >> 18) & 0x1fff;
        clkcnt_n = (value >> 12) & 0x3f;
        clkcnt_h = (value >> 6) & 0x3f;
        clkcnt_l = value & 0x3f;
    }

    // Compute the clock frequency
    clockHz = r4aEsp32ClockGetApb();
    clockHz /= ((pre + 1) * (clkcnt_n + 1));

    // Display the register value
    if (display)
    {
        display->printf("    SYSCLK: %ld\r\n", sysclk);
        display->printf("    PRE: 0x%04lx\r\n", pre);
        display->printf("    CLKCNT_N: 0x%02lx\r\n", clkcnt_n);
        display->printf("    CLKCNT_H: 0x%02lx\r\n", clkcnt_h);
        display->printf("    CLKCNT_L: 0x%02lx\r\n", clkcnt_l);
        mHz = clockHz / (1000 * 1000);
        kHz = (clockHz / 1000) - (mHz * 1000);
        display->printf("    SPI clock: %ld.%03ld MHz\r\n", mHz, kHz);
    }

    // Return the SPI clock frequency
    return clockHz;
}

//*********************************************************************
// Transfer the data to the SPI device
// Inputs:
//   txBuffer: Address of the buffer containing the data to send
//   rxBuffer: Address of the receive data buffer
//   length: Number of data bytes in the buffer
void r4aEsp32SpiTransfer(struct _R4A_SPI * spi,
                         const uint8_t * txBuffer,
                         uint8_t * rxBuffer,
                         uint32_t length)
{
    spiTransferBytes(((R4A_ESP32_SPI *)spi)->_spi, txBuffer, rxBuffer, length);
}

//*********************************************************************
// Validate the SPI tables
void r4aEsp32SpiValidateTables()
{
    // Validate register names
    if (r4aEsp32SpiNamesBytes != sizeof(R4A_ESP32_SPI_REGS))
        r4aReportFatalError("Please fix R4A_ESP32_SPI_NAMES to match R4A_ESP32_SPI_REGS");
}
