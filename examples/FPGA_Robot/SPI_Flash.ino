/**********************************************************************
  SPI_Flash.ino

  SPI Flash support routines
**********************************************************************/

//*********************************************************************
// Control the signal for the SPI flash write protection pin
// Inputs:
//   enable: Set true to enable writes to the SPI flash
bool spiFlashWriteEnable(bool enable)
{
    uint8_t data;

    do
    {
        // Update the write enable
        data = enable ? 1 : 0;
        if (r4aI2cBusWrite(r4aI2cBus,
                           SPI_FLASH_I2C_ADDRESS,
                           &data,
                           sizeof(data),
                           nullptr) == false)
        {
            Serial.printf("ERROR: Failed to update the SPI Flash write enable\r\n");
            break;
        }
        return true;
    } while (0);
    return false;
}

//*********************************************************************
// Display the status value
// Inputs:
//   status: Status register value
//   display: Address of Print object for output, may be nullptr
void spiFlashDisplayStatus(uint8_t status, Print * display)
{
    display->printf("0x%02x: status\r\n", status);

    // Decode the status register
    if (status & STATUS_BUSY)
        display->printf("    Write in progress\r\n");
    if (status & STATUS_SEC)
        display->printf("    Security ID\r\n");
    if (status & STATUS_WPLD)
        display->printf("    Write protect lock down\r\n");
    if (status & STATUS_WSP)
        display->printf("    Program suspended\r\n");
    if (status & STATUS_WSE)
        display->printf("    Erase suspended\r\n");
    if (status & STATUS_WEL)
        display->printf("    Write enabled\r\n");
}

//*********************************************************************
// Dump block protection register bit numbers
// Inputs:
//   menuEntry: Address of an R4A_MENU_ENTRY data structure
//   command: Address of a zero terminated string containing the command
//   display: Address of the serial output device
void spiFlashMenuBlockProtectionDump(const R4A_MENU_ENTRY * menuEntry,
                                     const char * command,
                                     Print * display)
{
    int index;

    // Walk the block protection table
    display->printf("Read Lock Bit\r\n");
    display->printf("Bit   Spec    Address Range\r\n");
    display->printf("--     --  --------   --------\r\n");
    for (index = 0; index < spiFlashBlockProtectionEntries; index++)
    {
        // Check for no bit to display
        if (spiFlashBlockProtection[index]._readProtectBit < 0)
            continue;

        // Display the read lock bit
        display->printf("%2d --> %2d: 0x%08x - 0x%08x\r\n",
                        spiFlashBlockProtection[index]._readProtectBit,
                        MAP_BPR_BIT(spiFlashBlockProtection[index]._readProtectBit),
                        spiFlashBlockProtection[index]._flashAddress,
                        spiFlashBlockProtection[index + 1]._flashAddress - 1);
    }

    // Walk the block protection table
    display->printf("Write Lock Bit\r\n");
    display->printf("Bit   Spec    Address Range\r\n");
    display->printf("--     --  --------   --------\r\n");
    for (index = 0; index < spiFlashBlockProtectionEntries; index++)
    {
        // Check for no bit to display
        if (spiFlashBlockProtection[index]._writeProtectBit < 0)
            continue;

        // Display the read lock bit
        display->printf("%2d --> %2d: 0x%08x - 0x%08x\r\n",
                        spiFlashBlockProtection[index]._writeProtectBit,
                        MAP_BPR_BIT(spiFlashBlockProtection[index]._writeProtectBit),
                        spiFlashBlockProtection[index]._flashAddress,
                        spiFlashBlockProtection[index + 1]._flashAddress - 1);
    }
}
