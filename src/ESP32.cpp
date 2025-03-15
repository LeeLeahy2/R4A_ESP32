/**********************************************************************
  ESP32.cpp

  Robots-For-All (R4A)
  ESP32 WROVER module support
**********************************************************************/

#include "R4A_ESP32.h"

//****************************************
// Locals
//****************************************

static uint8_t r4aEsp32GpioPinMode[256];
static float r4aEsp32VoltageReference;

//*********************************************************************
// Display the name of a zero terminated string and it's value.
// Inputs:
//   name: Name of the string variable
//   value: Value of the string variable
//   display: Device used for output
void r4aEsp32DisplayCharPointer(const char * name,
                                const char * value,
                                Print * display)
{
    display->printf("%s: %p%s%s%s\r\n",
                    name,
                    value,
                    value ? ", (" : "",
                    value ? value : "",
                    value ? ")"   : "");
}

//*********************************************************************
// Allocate DMA buffer
// Inputs:
//   length: Number of data bytes to allocate
// Outputs:
//   Returns the buffer address if successful and nullptr otherwise
uint8_t * r4aEsp32AllocateDmaBuffer(int length)
{
    return (uint8_t *)heap_caps_malloc(length, MALLOC_CAP_DMA);
}

//*********************************************************************
// Display heap usage
void r4aEsp32HeapDisplay(Print * display)
{
    display->println("              Size        Free      In Use");
    display->println("        ----------  ----------  ----------");
    display->printf("Heap:   %10ld  %10ld  %10ld\r\n",
                    ESP.getHeapSize(),
                    ESP.getFreeHeap(),
                    ESP.getHeapSize() - ESP.getFreeHeap());
    display->printf("PSRAM:  %10ld  %10ld  %10ld\r\n",
                    ESP.getPsramSize(),
                    ESP.getFreePsram(),
                    ESP.getPsramSize() - ESP.getFreePsram());
    display->println();
    display->println("          Lowest Point    Largest Block");
    display->println("          ------------    -------------");
    display->printf ("Combined:   %10d       %10d\r\n",
                    xPortGetMinimumEverFreeHeapSize(),
                    heap_caps_get_largest_free_block(MALLOC_CAP_8BIT));
}

//*********************************************************************
// Determine if the address is SRAM that supports DMA
bool r4aEsp32IsAddressInDMARAM(void * addr)
{
    return r4aEsp32IsAddressInSRAM1(addr) || r4aEsp32IsAddressInSRAM2(addr);
}

//*********************************************************************
// Determine if the address is in the EEPROM (Flash)
bool r4aEsp32IsAddressInEEPROM(void * addr)
{
    return ((addr >= (void *)0x3f400000) && (addr <= (void *)0x3f7fffff));
}

//*********************************************************************
// Determine if the address is in PSRAM (SPI RAM)
bool r4aEsp32IsAddressInPSRAM(void * addr)
{
    return ((addr >= (void *)0x3f800000) && (addr <= (void *)0x3fbfffff));
}

//*********************************************************************
// Determine if the address is in PSRAM, SRAM or RTC fast memory
bool r4aEsp32IsAddressInRAM(void * addr)
{
    return r4aEsp32IsAddressInSRAM(addr) || r4aEsp32IsAddressInPSRAM(addr)
        || r4aEsp32IsAddressInRtcFastMemory(addr);
}

//*********************************************************************
bool r4aEsp32IsAddressInROM(void * addr)
{
    return (((addr >= (void *)0x3ff90000) && (addr <= (void *)0x3ff9ffff))
        ||  ((addr >= (void *)0x40000000) && (addr <= (void *)0x4005ffff)));
}

//*********************************************************************
// Determine if the address is in RTC fast memory
bool r4aEsp32IsAddressInRtcFastMemory(void * addr)
{
    // Instruction bus addresses, PRO_CPU only, see pages 28-29 in the TRM
    return (xPortGetCoreID() == 0)
        && ((addr >= (void *)0x3ff80000) && (addr <= (void *)0x3ff81fff));
}

//*********************************************************************
// Determine if the address is in SRAM
bool r4aEsp32IsAddressInSRAM(void * addr)
{
    return r4aEsp32IsAddressInSRAM0(addr) || r4aEsp32IsAddressInSRAM1(addr)
        || r4aEsp32IsAddressInSRAM2(addr);
}

//*********************************************************************
// Determine if the address is in SRAM0
bool r4aEsp32IsAddressInSRAM0(void * addr)
{
    // Instruction bus addresses
    return ((addr >= (void *)0x40070000) && (addr <= (void *)0x4007ffff));
}

//*********************************************************************
// Determine if the address is in SRAM1
bool r4aEsp32IsAddressInSRAM1(void * addr)
{
    // Data bus addresses
    return ((addr >= (void *)0x3ffe0000) && (addr <= (void *)0x3fffffff))
        // Instruction bus addresses
        || ((addr >= (void *)0x400a0000) && (addr <= (void *)0x400bffff));
}

//*********************************************************************
// Determine if the address is in SRAM2
bool r4aEsp32IsAddressInSRAM2(void * addr)
{
    return ((addr >= (void *)0x3ffae000) && (addr <= (void *)0x3ffdffff));
}

//*********************************************************************
// Find the specified partition
bool r4aEsp32PartitionFind(const char * name)
{
    // Initialize the partition list iterator
    esp_partition_iterator_t pi = esp_partition_find(ESP_PARTITION_TYPE_ANY,
                                                     ESP_PARTITION_SUBTYPE_ANY,
                                                     NULL);

    // Determine if any partitions exist in the list
    if (pi != NULL)
    {
        // Walk through the list of partitions
        do
        {
            // Get the partition data
            const esp_partition_t *p = esp_partition_get(pi);

            // Determine if the requested partition was found
            if (strcmp(p->label, name) == 0)
                // The partition was found
                return true;

            // Get the next partition in the list
        } while ((pi = (esp_partition_next(pi))));
    }

    // The spiffs partition does not exist
    return false;
}

//*********************************************************************
// Display the partition table
void r4aEsp32PartitionTableDisplay(Print * display)
{
    // Display the table header
    display->println(".-----------------------------------------------------.");
    display->println("|                ESP32 Partition table                |");
    display->println("|-----------------------------------------------------|");
    display->println("| Type | Sub |  Offset  |   Size   |       Label      |");
    display->println("| ---- | --- | -------- | -------- | ---------------- |");

    // Initialize the partition list iterator
    esp_partition_iterator_t pi = esp_partition_find(ESP_PARTITION_TYPE_ANY, ESP_PARTITION_SUBTYPE_ANY, NULL);

    // Determine if any partitions exist in the list
    if (pi != NULL)
    {
        // Walk through the list of partitions
        do
        {
            // Get the partition data
            const esp_partition_t *p = esp_partition_get(pi);

            // Display the partition data
            display->printf("|  %02x  | %02x  | 0x%06lx | 0x%06lx | %-16s |\r\n",
                            p->type,
                            p->subtype,
                            p->address,
                            p->size,
                            p->label);

            // Get the next partition in the list
        } while ((pi = (esp_partition_next(pi))));
    }

    // Display the table footer
    display->println("'-----------------------------------------------------'");
}

//*********************************************************************
// Set and save the pin mode
uint8_t r4aEsp32PinMode(uint8_t pin, uint8_t mode)
{
    pinMode(pin, mode);
    uint8_t previousMode = r4aEsp32GpioPinMode[pin];
    r4aEsp32GpioPinMode[pin] = mode;
    return previousMode;
}

//*********************************************************************
// System reset
void r4aEsp32SystemReset()
{
    ESP.restart();
    while (1);
}

//*********************************************************************
// Display the voltage
void r4aEsp32VoltageDisplay(int adcPin, float offset, float multiplier, Print * display)
{
    float voltage;
    int16_t adcValue;

    // Get the battery voltage
    voltage = r4aEsp32VoltageGet(adcPin, offset, multiplier, &adcValue);

    // Display the battery voltage
    if (adcValue < 5)
        display->printf("Power switch is off (0x%04x)!\r\n", adcValue);
    else
        display->printf("Battery Voltage (%d, 0x%04x): %.2f Volts\r\n",
                       adcValue, adcValue, voltage);
}

//*********************************************************************
// Get the voltage
float r4aEsp32VoltageGet(int adcPin, float offset, float multiplier, int16_t * adcValue)
{
    int averageAdcReading;
    uint32_t previousFunction;
    float voltage;

    // Bug: No WS2812 output
    //      The WS2812 code uses the RMT (remote control peripheral) and
    //      the GPIO mux is making the connection.  The pinMode call below
    //      switches the GPIO mux for pin 32 from the RMT to the GPIO
    //      controller.  However setting the pinMode back to output does
    //      not restore the GPIO mux value.
    //
    // Fix: Share the pin between battery voltage input and WS2812 output
    //      Save and restore the GPIO mux value.
    //
    // Remember the GPIO pin mux value
    previousFunction = r4aGpioRegs->R4A_GPIO_FUNC_OUT_SEL_CFG_REG[adcPin];

    // Switch from RMT output for the WS2812 to GPIO input for ADC
    pinMode(adcPin, INPUT);

    // Read the voltage multiple times and take the average
    averageAdcReading = 0;
    for (int index = 0; index < 8; index++)
        averageAdcReading += analogRead(adcPin);
    averageAdcReading >>= 3;

    // Restore the GPIO pin to an output for the WS2812 and reconnect the
    // pin to the RMT (remote control peripheral)
    pinMode(adcPin, r4aEsp32GpioPinMode[adcPin]);
    r4aGpioRegs->R4A_GPIO_FUNC_OUT_SEL_CFG_REG[adcPin] = previousFunction;

    // Return the ADC value
    if (adcValue)
        *adcValue = (int16_t)averageAdcReading;

    // Convert the ADC reading into a voltage value
    voltage = (float)averageAdcReading * r4aEsp32VoltageReference / 4095.;
    if (multiplier != 1.)
        voltage *= multiplier;
    if (offset)
        voltage -= offset;
    return voltage;
}

//*********************************************************************
// Set the reference voltage
void r4aEsp32VoltageSetReference(float maximumVoltage)
{
    r4aEsp32VoltageReference = maximumVoltage;
}

//*********************************************************************
// Display the heap
void r4aEsp32MenuDisplayHeap(const struct _R4A_MENU_ENTRY * menuEntry,
                             const char * command,
                             Print * display)
{
    r4aEsp32HeapDisplay(display);
}

//*********************************************************************
// Display the partitions
void r4aEsp32MenuDisplayPartitions(const struct _R4A_MENU_ENTRY * menuEntry,
                                   const char * command,
                                   Print * display)
{
    r4aEsp32PartitionTableDisplay(display);
}

//*********************************************************************
// Reset the system
void r4aEsp32MenuSystemReset(const struct _R4A_MENU_ENTRY * menuEntry,
                             const char * command,
                             Print * display)
{
    r4aEsp32SystemReset();
}
