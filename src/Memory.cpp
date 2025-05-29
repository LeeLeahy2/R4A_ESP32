/**********************************************************************
  Memory.cpp

  Robots-For-All (R4A)
  Memory support

  Useful links:
    https://www.gnu.org/software/libc/manual/html_node/Replacing-malloc.html
    https://en.cppreference.com/w/cpp/memory/new
    https://en.cppreference.com/w/cpp/memory/new/operator_new
    https://en.cppreference.com/w/cpp/memory/new/operator_delete
**********************************************************************/

#include "R4A_ESP32.h"

//****************************************
// Globals
//****************************************

bool r4aMallocDebug;
size_t r4aMallocMaxBytes = 128;

//*********************************************************************
// User defined delete function, see https://en.cppreference.com/w/cpp/memory/new/operator_delete
// Inputs:
//   ptr: Address of the buffer to free
//   text: Address of zero terminated string of characters
void operator delete(void * ptr, const char * text) noexcept
{
    // Display the free operation
    r4aFree(ptr, text);
}

//*********************************************************************
// Replace the delete function, see https://en.cppreference.com/w/cpp/memory/new/operator_delete
// Inputs:
//   ptr: Address of the buffer to free
void operator delete(void * ptr) noexcept
{
    // Display the free operation
    r4aFree(ptr, "object");
}

//*********************************************************************
// Replace the delete function, see https://en.cppreference.com/w/cpp/memory/new/operator_delete
// Inputs:
//   ptr: Address of the buffer to free
//   size: Number of bytes to free
void operator delete(void * ptr, size_t size) noexcept
{
    // Display the free operation
    r4aFree(ptr, "object");
}

//*********************************************************************
// Replace the delete[] function, see https://en.cppreference.com/w/cpp/memory/new/operator_delete
// Inputs:
//   ptr: Address of the array to free
void operator delete[](void * ptr) noexcept
{
    // Display the free operation
    r4aFree(ptr, "array");
}

//*********************************************************************
// Replace the delete[] function, see https://en.cppreference.com/w/cpp/memory/new/operator_delete
// Inputs:
//   ptr: Address of the array to free
//   size: Number of bytes to free
void operator delete[](void * ptr, size_t size) noexcept
{
    // Display the free operation
    r4aFree(ptr, "array");
}

//*********************************************************************
// User defined new function, see https://en.cppreference.com/w/cpp/memory/new/operator_new
// Inputs:
//   numberOfBytes: Number of bytes to allocate for the buffer
//   text: Address of zero terminated string of characters
// Outputs:
//   Returns the buffer address when successful or nullptr if failure
void* operator new(std::size_t numberOfBytes, const char * text)
{
    return r4aMalloc(numberOfBytes, text);
}

//*********************************************************************
// Replace the new function, see https://en.cppreference.com/w/cpp/memory/new/operator_new
// Inputs:
//   numberOfBytes: Number of bytes to allocate for the buffer
// Outputs:
//   Returns the buffer address when successful or nullptr if failure
void* operator new(std::size_t numberOfBytes)
{
    return r4aMalloc(numberOfBytes, "New object");
}

//*********************************************************************
// Replace the new[] function, see https://en.cppreference.com/w/cpp/memory/new/operator_new
// Inputs:
//   numberOfBytes: Number of bytes to allocate for the array
// Outputs:
//   Returns the array address when successful or nullptr if failure
void* operator new[](std::size_t numberOfBytes)
{
    return r4aMalloc(numberOfBytes, "New array");
}

//*********************************************************************
// Free a DMA buffer, set the pointer to nullptr after it is freed
// Inputs:
//   buffer: Address of the buffer to be freed
//   text: Text to display when debugging is enabled
void r4aDmaFree(void * buffer, const char * text)
{
    // Display the free operation
    if (r4aMallocDebug)
        Serial.printf("%p: %s, Freeing %s used for DMA\r\n",
                      buffer, r4aMemoryLocation(buffer), text);

    // Free the DMA buffer
    free(buffer);
}

//*********************************************************************
// Allocate a buffer that support DMA
// Inputs:
//   numberOfBytes: Number of bytes to allocate
//   text: Text to display when debugging is enabled
// Outputs:
//   Returns the buffer address or nullptr upon failure
void * r4aDmaMalloc(size_t numberOfBytes, const char * text)
{
    void * buffer;

    // Attempt to allocate the DMA buffer
    buffer = r4aEsp32AllocateDmaBuffer(numberOfBytes);

    // Display the malloc operation
    if (r4aMallocDebug)
    {
        if (buffer)
            Serial.printf("%p: %s, %s, Allocated %d (0x%x) bytes for DMA\r\n",
                          buffer, r4aMemoryLocation(buffer), text,
                          numberOfBytes, numberOfBytes);
        else
            Serial.printf("Error: Failed to allocate DMA buffer, %s\r\n", text);
    }
    return buffer;
}

//*********************************************************************
// Free a buffer, set the pointer to nullptr after it is freed
// Inputs:
//   buffer: Address of the buffer to be freed
//   text: Text to display when debugging is enabled
void r4aFree(void * buffer, const char * text)
{
    // Display the free operation
    if (r4aMallocDebug)
        Serial.printf("%p: %s, Freeing %s\r\n",
                      buffer, r4aMemoryLocation(buffer), text);

    // Free the buffer
    free(buffer);
}

//*********************************************************************
// Allocate a buffer
// Inputs:
//   numberOfBytes: Number of bytes to allocate
//   text: Text to display when debugging is enabled
// Outputs:
//   Returns the buffer address or nullptr upon failure
void * r4aMalloc(size_t numberOfBytes, const char * text)
{
    void * buffer;
    static bool psramCheckDone;
    static bool psramAvailable;

    // Check for PSRAM
    if (psramCheckDone == false)
    {
        psramCheckDone = true;
        log_v("Checking for PSRAM");
        psramAvailable = psramFound();
        log_v("PSRAM: %s", psramAvailable ? "Available" : "NOT available");
    }

    // Attempt to allocate the buffer
    if ((r4aMallocMaxBytes < numberOfBytes) && psramAvailable)
        buffer = ps_malloc(numberOfBytes);
    else
        buffer = malloc(numberOfBytes);

    // Display the malloc operation
    if (r4aMallocDebug)
    {
        if (buffer)
            Serial.printf("%p: %s, %s, Allocated %d (0x%x) bytes\r\n",
                          buffer, r4aMemoryLocation(buffer), text,
                          numberOfBytes, numberOfBytes);
        else
            Serial.printf("Error: Failed to allocate buffer, %s\r\n", text);
    }
    return buffer;
}

//*********************************************************************
// Determine the memory location
// Inputs:
//   addr: Address of the buffer
// Outputs:
//   Returns a zero terminated string containing the location of the buffer
const char * r4aMemoryLocation(void * addr)
{
    if (r4aEsp32IsAddressInSRAM0(addr))
        return "SRAM0";
    if (r4aEsp32IsAddressInSRAM1(addr))
        return "SRAM1";
    if (r4aEsp32IsAddressInSRAM2(addr))
        return "SRAM2";
    if (r4aEsp32IsAddressInPSRAM(addr))
        return "PSRAM";
    if (r4aEsp32IsAddressInRtcFastMemory(addr))
        return "RTC Fast Memory";
    if (r4aEsp32IsAddressInEEPROM(addr))
        return "EEPROM";
    if (r4aEsp32IsAddressInROM(addr))
        return "ROM";
    return "Unknown";
}
