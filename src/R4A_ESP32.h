/**********************************************************************
  R4A_ESP32.h

  Robots-For-All (R4A)
  Declare the ESP32 support
**********************************************************************/

#ifndef __R4A_ESP32_H__
#define __R4A_ESP32_H__

#include <Arduino.h>            // Built-in
#include "ESPmDNS.h"            // Built-in
#include <HTTPClient.h>         // Built-in
#include <LittleFS.h>           // Built-in, load and store files in flash
#include <WiFi.h>               // Built-in
#include <WiFiClient.h>         // Built-in
#include <WiFiMulti.h>          // Built-in, multiple WiFi AP support
#include <WiFiServer.h>         // Built-in

#include <esp_camera.h>         // IDF built-in, needed for OV2640 camera
#include <esp32-hal-spi.h>      // IDF built-in
#include <esp_http_server.h>    // IDF built-in, needed for camera web server
#include <esp_wifi.h>           // IDF built-in

#include <R4A_Robot.h>          // Robots-For-All robot support
#include <R4A_I2C.h>            // Robots-For-All I2C support
#include "R4A_ESP32_GPIO.h"     // Robots-For-All ESP32 GPIO declarations
#include "R4A_ESP32_Timer.h"    // Robots-For-All ESP32 Timer declarations

//****************************************
// Constants
//****************************************

extern R4A_GPIO_REGS * const r4aGpioRegs;
extern R4A_IO_MUX_REGS * const r4aIoMux;
extern R4A_RTCIO_REGS * const r4aRtcioRegs;

extern const int r4aGpioPortToIoMuxIndex[R4A_GPIO_MAX_PORTS];
extern const char * const r4aIoMuxNames[R4A_GPIO_MAX_PORTS][8];
extern const uint8_t r4aIoMuxIsGpio[R4A_GPIO_MAX_PORTS];
extern const R4A_GPIO_MATRIX r4aGpioMatrixNames[256];

//****************************************
// ESP32 API
//****************************************

// Allocate DMA buffer
// Inputs:
//   length: Number of data bytes to allocate
// Outputs:
//   Returns the buffer address if successful and nullptr otherwise
uint8_t * r4aEsp32AllocateDmaBuffer(int length);

// Display the name of a zero terminated string and it's value.
// Inputs:
//   name: Name of the string variable
//   value: Value of the string variable
//   display: Device used for output
void r4aEsp32DisplayCharPointer(const char * name,
                                const char * value,
                                Print * display = &Serial);

// Determine if the address is SRAM that supports DMA
// Inputs:
//   addr: Address in question
// Outputs:
//   Returns true if the address is in SRAM that supports DMA and false
//   otherwise
bool r4aEsp32IsAddressInDMARAM(void * addr);

// Determine if the address is in the EEPROM (Flash)
// Inputs:
//   addr: Address in question
// Outputs:
//   Returns true if the address is in EEPROM and false otherwise
bool r4aEsp32IsAddressInEEPROM(void * addr);

// Determine if the address is in PSRAM (SPI RAM)
// Inputs:
//   addr: Address in question
// Outputs:
//   Returns true if the address is in PSRAM and false otherwise
bool r4aEsp32IsAddressInPSRAM(void * addr);

// Determine if the address is in PSRAM or SRAM
// Inputs:
//   addr: Address in question
// Outputs:
//   Returns true if the address is in PSRAM or SRAM and false otherwise
bool r4aEsp32IsAddressInRAM(void * addr);

// Determine if the address is in ROM
// Inputs:
//   addr: Address in question
// Outputs:
//   Returns true if the address is in ROM and false otherwise
bool r4aEsp32IsAddressInROM(void * addr);

// Determine if the address is in RTC fast memory
// Inputs:
//   addr: Address in question
// Outputs:
//   Returns true if the address is in RTC fast memory and false otherwise
bool r4aEsp32IsAddressInRtcFastMemory(void * addr);

// Determine if the address is in SRAM
// Inputs:
//   addr: Address in question
// Outputs:
//   Returns true if the address is in SRAM and false otherwise
bool r4aEsp32IsAddressInSRAM(void * addr);

// Determine if the address is in SRAM0
// Inputs:
//   addr: Address in question
// Outputs:
//   Returns true if the address is in SRAM0 and false otherwise
bool r4aEsp32IsAddressInSRAM0(void * addr);

// Determine if the address is in SRAM1
// Inputs:
//   addr: Address in question
// Outputs:
//   Returns true if the address is in SRAM1 and false otherwise
bool r4aEsp32IsAddressInSRAM1(void * addr);

// Determine if the address is in SRAM2
// Inputs:
//   addr: Address in question
// Outputs:
//   Returns true if the address is in SRAM2 and false otherwise
bool r4aEsp32IsAddressInSRAM2(void * addr);

// Find the specified partition
// Inputs:
//   name: Partition name to be found
// Outputs:
//   Returns true if the partition was found and false otherwise
bool r4aEsp32PartitionFind(const char * name);

// Display the partition table
// Inputs:
//   display: Device used for output
void r4aEsp32PartitionTableDisplay(Print * display = &Serial);

// Set and save the pin mode
// Inputs:
//   pin: Number of the pin to set
//   mode: Mode of operation for the pin
// Outputs:
//   Returns the previous mode of operation for the pin
uint8_t r4aEsp32PinMode(uint8_t pin, uint8_t mode);

// System reset
void r4aEsp32SystemReset();

// Display the voltage
// Inputs:
//   adcPin: GPIO pin number for the ADC pin
//   offset: Ground level offset correction
//   multiplier: Multiplier for each of the ADC bits
//   display: Device used for output
void r4aEsp32VoltageDisplay(int adcPin,
                            float offset,
                            float multiplier,
                            Print * display = &Serial);

// Read the voltage
// Inputs:
//   adcPin: GPIO pin number for the ADC pin
//   offset: Ground level offset correction
//   multiplier: Multiplier for each of the ADC bits
//   adcValue: Return the value read from the ADC
// Outputs:
//   Returns the computed voltage
float r4aEsp32VoltageGet(int adcPin,
                         float offset,
                         float multiplier,
                         int16_t * adcValue);

// Set the reference voltage
// Inputs:
//   maximumVoltage: Maximum voltage represented by the ADC
void r4aEsp32VoltageSetReference(float maximumVoltage);

//****************************************
// ESP32 Menu API
//****************************************

// Display the heap
// Inputs:
//   menuEntry: Address of the object describing the menu entry
//   command: Zero terminated command string
//   display: Device used for output
void r4aEsp32MenuDisplayHeap(const struct _R4A_MENU_ENTRY * menuEntry,
                             const char * command,
                             Print * display);

// Display the partitions
// Inputs:
//   menuEntry: Address of the object describing the menu entry
//   command: Zero terminated command string
//   display: Device used for output
void r4aEsp32MenuDisplayPartitions(const struct _R4A_MENU_ENTRY * menuEntry,
                                   const char * command,
                                   Print * display);

// Reset the system
// Inputs:
//   menuEntry: Address of the object describing the menu entry
//   command: Zero terminated command string
//   display: Device used for output
void r4aEsp32MenuSystemReset(const struct _R4A_MENU_ENTRY * menuEntry,
                             const char * command,
                             Print * display);

//****************************************
// GPIO API
//****************************************

// Display the IO MUX registers
// Inputs:
//   display: Device used for output
void r4aEsp32GpioDisplayIoMuxRegisters(Print * display = &Serial);

// Display the GPIO registers
// Inputs:
//   display: Device used for output
void r4aEsp32GpioDisplayRegisters(Print * display = &Serial);

//****************************************
// Heap support
//****************************************
// Display the heap
// Inputs:
//   display: Device used for output
void r4aEsp32HeapDisplay(Print * display = &Serial);

//****************************************
// HTTP API
//****************************************

extern const httpd_err_code_t r4aHttpError[];
extern const int r4aHttpErrorCount;
extern const char * r4aHttpErrorName[];

typedef struct _R4A_JPEG_CHUNKING_T
{
        httpd_req_t *req;
        size_t length;
} R4A_JPEG_CHUNKING_T;

typedef struct _R4A_TAG_NAME_T
{
    int tag;
    const char * name;
} R4A_TAG_NAME_T;

//****************************************
// ESP32 I2C Bus support
//****************************************

// Initialize the I2C bus
// Inputs:
//   object: Address of a R4A_I2C_BUS data structure
//   sdaPin: Number of the pin used for the SDA signal
//   sclPin: Number of the pin used for the SCL signal
//   clockHz: Clock speed for the I2C bus in Hertz
//   display: Device used for output
void r4aEsp32I2cBusBegin(R4A_I2C_BUS * object,
                         int sdaPin,
                         int sclPin,
                         int clockHz,
                         Print * display = &Serial);

// Get the TwoWire pointer
// Outputs:
//   Returns the TwoWire object address
TwoWire * r4aEsp32I2cBusGetTwoWire(R4A_I2C_BUS * object);

// Read data from an I2C peripheral
// Inputs:
//   object: Address of a R4A_I2C_BUS data structure
//   deviceAddress: Device address on the I2C bus (0 - 0x7f)
//   cmdBuffer: Address of the buffer containing the command bytes, may be nullptr
//   cmdByteCount: Number of bytes to send from the command buffer
//   dataBuffer: Address of the buffer to receive the data bytes, may be nullptr
//   dataByteCount: Size in bytes of the data buffer, maximum receive bytes
//   display: Device used for debug output
//   releaseI2cBus: A value of true releases the I2C bus after the transaction
// Outputs:
//   Returns the number of bytes read
size_t r4aEsp32I2cBusRead(R4A_I2C_BUS * object,
                          uint8_t deviceI2cAddress,
                          const uint8_t * cmdBuffer, // Does not include I2C address
                          size_t cmdByteCount,
                          uint8_t * readBuffer,
                          size_t readByteCount,
                          Print * display = nullptr,
                          bool releaseI2cBus = true);

// Send data to an I2C peripheral, entered with the I2C bus lock held
// Inputs:
//   object: Address of a R4A_I2C_BUS data structure
//   deviceAddress: Device address on the I2C bus (0 - 0x7f)
//   cmdBuffer: Address of the buffer containing the command bytes, may be nullptr
//   cmdByteCount: Number of bytes to send from the command buffer
//   dataBuffer: Address of the buffer containing the data bytes, may be nullptr
//   dataByteCount: Number of bytes to send from the data buffer
//   display: Device used for debug output
//   releaseI2cBus: A value of true releases the I2C bus after the transaction
// Outputs:
//   Returns true upon success, false otherwise
bool r4aEsp32I2cBusWriteWithLock(R4A_I2C_BUS * object,
                                 uint8_t deviceI2cAddress,
                                 const uint8_t * cmdBuffer,
                                 size_t cmdByteCount,
                                 const uint8_t * dataBuffer,
                                 size_t dataByteCount,
                                 Print * display = nullptr,
                                 bool releaseI2cBus = true);

//****************************************
// NVM API
//****************************************

#define R4A_ESP32_NVM_STRING(x)     ((uint64_t)(intptr_t)(const char *)x)
#define R4A_ESP32_NVM_FLOAT_CONV    ((double)(0x10000000ull))
#define R4A_ESP32_NVM_FLT(x)        ((uint64_t)(((double)x) * R4A_ESP32_NVM_FLOAT_CONV))

enum R4A_ESP32_NVM_PARAMETER_TYPE
{
    R4A_ESP32_NVM_PT_NULLPTR = 0,
    R4A_ESP32_NVM_PT_BOOL,        //  1
    R4A_ESP32_NVM_PT_INT8,        //  2
    R4A_ESP32_NVM_PT_UINT8,       //  3
    R4A_ESP32_NVM_PT_INT16,       //  4
    R4A_ESP32_NVM_PT_UINT16,      //  5
    R4A_ESP32_NVM_PT_INT32,       //  6
    R4A_ESP32_NVM_PT_UINT32,      //  7
    R4A_ESP32_NVM_PT_INT64,       //  8
    R4A_ESP32_NVM_PT_UINT64,      //  9
    R4A_ESP32_NVM_PT_FLOAT,       // 10
    R4A_ESP32_NVM_PT_DOUBLE,      // 11
    R4A_ESP32_NVM_PT_P_CHAR,      // 12
};

typedef union
{
    bool     b;
    int8_t   i8;
    uint8_t  u8;
    int16_t  i16;
    uint16_t u16;
    int32_t  i32;
    uint32_t u32;
    int64_t  i64;
    uint64_t u64;
    double   d;     // Float values are cast when read and written
    const char * pcc;
    void * pv;
} R4A_ESP32_NVM_VALUE;

typedef struct _R4A_ESP32_NVM_PARAMETER
{
    bool required;
    uint8_t type;
    uint64_t minimum;
    uint64_t maximum;
    void * addr;
    const char * name;
    uint64_t value;
} R4A_ESP32_NVM_PARAMETER;

extern const char * parameterFilePath; // Path to the parameter file
extern const R4A_ESP32_NVM_PARAMETER nvmParameters[];
extern const int nvmParameterCount;
extern bool r4aEsp32NvmDebug; // Set to true to enable debug output

// Clear a parameter by setting its value to zero
// Inputs:
//   filePath: Path to the file to be stored in NVM
//   parameterTable: Address of the first entry in the parameter table
//   parameterCount: Number of entries in the parameter table
//   name: Name of the parameter to be cleared
//   display: Device used for output
void r4aEsp32NvmParameterClear(const char * filePath,
                               const R4A_ESP32_NVM_PARAMETER * parameterTable,
                               int parameterCount,
                               const char * name,
                               Print * display = &Serial);

// Display a parameter
// Inputs:
//   parameter: Address of the entry in the parameter table to display
//   display: Device used for output
void r4aEsp32NvmDisplayParameter(const R4A_ESP32_NVM_PARAMETER * parameter,
                                 Print * display = &Serial);

// Display the parameters
// Inputs:
//   parameterTable: Address of the first entry in the parameter table
//   parameterCount: Number of entries in the parameter table
//   display: Device used for output
void r4aEsp32NvmDisplayParameters(const R4A_ESP32_NVM_PARAMETER * parameterTable,
                                  int parameterCount,
                                  Print * display = &Serial);

// Dump the parameter file
// Inputs:
//   filePath: Path to the file contained in the NVM
//   display: Device used for output
void r4aEsp32NvmDumpParameterFile(const char * filePath,
                                  Print * display = &Serial);

// Display the contents of the file
// Inputs:
//   filePath: Name of the file to dump
//   display: Device used for output
void r4aEsp32NvmFileCat(String filePath, Print * display);

// Get the default set of parameters
// Inputs:
//   parameterTable: Address of the first entry in the parameter table
//   parametersCount: Number of parameters in the table
void r4aEsp32NvmGetDefaultParameters(const R4A_ESP32_NVM_PARAMETER * parameterTable,
                                     int parametersCount);

// Get a set of parameters
// Inputs:
//   filePath: Address of the address of the path to the file contained in the NVM
//   display: Device used for output, may be nullptr
//   debug:   Set to true to enable parameter debugging
// Outputs:
//   Returns true if successful and false upon failure
bool r4aEsp32NvmGetParameters(const char ** filePath,
                              Print * display = nullptr,
                              bool debug = r4aEsp32NvmDebug);

// Look up a parameter by address
// Inputs:
//   parameterTable: Address of the first entry in the parameter table
//   parameterCount: Number of entries in the parameter table
//   address: Address of the parameter value
//   display: Device used for output
// Outputs:
//   Returns the address of the found entry in the parameter table or
//   nullptr if the parameter was not found
const R4A_ESP32_NVM_PARAMETER * r4aEsp32NvmParameterLookup(const R4A_ESP32_NVM_PARAMETER * parameterTable,
                                                           int parameterCount,
                                                           void * address,
                                                           Print * display);

// Look up a parameter by name
// Inputs:
//   parameterTable: Address of the first entry in the parameter table
//   parameterCount: Number of entries in the parameter table
//   name: Name of the parameter to be found
//   display: Device used for output
// Outputs:
//   Returns the address of the found entry in the parameter table or
//   nullptr if the parameter was not found
const R4A_ESP32_NVM_PARAMETER * r4aEsp32NvmParameterLookup(const R4A_ESP32_NVM_PARAMETER * parameterTable,
                                                           int parameterCount,
                                                           const char * name,
                                                           Print * display = &Serial);

// Set a parameter value
// Inputs:
//   filePath: Path to the file to be stored in NVM
//   parameterTable: Address of the first entry in the parameter table
//   parametersCount: Number of parameters in the table
//   parameter: Address of the specified parameter in the table
//   valueString: Character string containing the new value
//   display: Device used for output
bool r4aEsp32NvmParameterSet(const char * filePath,
                             const R4A_ESP32_NVM_PARAMETER * parameterTable,
                             int parameterCount,
                             const R4A_ESP32_NVM_PARAMETER * parameter,
                             const char * valueString,
                             Print * display = &Serial,
                             bool debug = r4aEsp32NvmDebug);

// Read a line from the file
// Inputs:
//   file: Address of the File object
//   line: Address of the buffer to receive the zero terminated line
//   lineLength: Number of bytes in the buffer
//   display: Device used for output, passed to computeWayPoint
// Outputs:
//   Returns true if the line was successfully read
bool r4aEsp32NvmReadLine(File * file,
                uint8_t * line,
                size_t lineLength,
                Print * display);

// Read the parameters from a file
// Inputs:
//   filePath: Path to the file contained in the NVM
//   parameterTable: Address of the first entry in the parameter table
//   parametersCount: Number of parameters in the table
//   display: Device used for output
// Outputs:
//   Returns true if successful and false upon failure
bool r4aEsp32NvmReadParameters(const char * filePath,
                               const R4A_ESP32_NVM_PARAMETER * parameterTable,
                               int parametersCount,
                               Print * display = &Serial);

// Write a string to the parameter file
// Inputs:
//   file: File to which the string is written
//   string: Address of a zero terminated string of characters
// Outputs:
//   Returns true if all of the data was successfully written and false
//   upon error
bool r4aEsp32NvmWriteFileString(File &file, const char * string);

// Write a string to the parameter file
// Inputs:
//   file: File to which the string is written
//   string: Address of a zero terminated string of characters
//   length: Length of the string in bytes
// Outputs:
//   Returns true if all of the data was successfully written and false
//   upon error
bool r4aEsp32NvmWriteFileString(File &file, const char * string, size_t length);

// Write the parameters to a file
// Inputs:
//   filePath: Path to the file to be stored in NVM
//   parameterTable: Address of the first entry in the parameter table
//   parametersCount: Number of parameters in the table
//   display: Device used for output
// Outputs:
//   Returns true if successful and false upon failure
bool r4aEsp32NvmWriteParameters(const char * filePath,
                                const R4A_ESP32_NVM_PARAMETER * parameterTable,
                                int parametersCount,
                                Print * display = &Serial,
                                bool debug = r4aEsp32NvmDebug);

//****************************************
// NVM Menu API
//****************************************

extern const R4A_MENU_ENTRY r4aEsp32NvmMenuTable[]; // Menu table for NVM menu
#define R4A_ESP32_NVM_MENU_ENTRIES    17            // Length of NVM menu table

// Display all of the parameters
// Inputs:
//   menuEntry: Address of the object describing the menu entry
//   command: Zero terminated command string
//   display: Device used for output
void r4aEsp32NvmMenuDisplayParameters(const struct _R4A_MENU_ENTRY * menuEntry,
                                      const char * command,
                                      Print * display);

// Dump the parameter file
// Inputs:
//   menuEntry: Address of the object describing the menu entry
//   command: Zero terminated command string
//   display: Device used for output
void r4aEsp32NvmMenuDumpParameterFile(const struct _R4A_MENU_ENTRY * menuEntry,
                                      const char * command,
                                      Print * display);

// Display the contents of the file
// Inputs:
//   menuEntry: Address of the object describing the menu entry
//   command: Zero terminated command string
//   display: Device used for output
void r4aEsp32NvmMenuFileCat(const R4A_MENU_ENTRY * menuEntry,
                            const char * command,
                            Print * display);

// Copy the file contents to a new file
// Inputs:
//   menuEntry: Address of the object describing the menu entry
//   command: Zero terminated command string
//   display: Device used for output
void r4aEsp32NvmMenuFileCopy(const R4A_MENU_ENTRY * menuEntry,
                             const char * command,
                             Print * display);

// Dump the file contents
// Inputs:
//   menuEntry: Address of the object describing the menu entry
//   command: Zero terminated command string
//   display: Device used for output
void r4aEsp32NvmMenuFileDump(const R4A_MENU_ENTRY * menuEntry,
                             const char * command,
                             Print * display);

// List the contents of the current directory
// Inputs:
//   menuEntry: Address of the object describing the menu entry
//   command: Zero terminated command string
//   display: Device used for output
void r4aEsp32NvmMenuFileList(const R4A_MENU_ENTRY * menuEntry,
                             const char * command,
                             Print * display);

// Move (rename) a file
// Inputs:
//   menuEntry: Address of the object describing the menu entry
//   command: Zero terminated command string
//   display: Device used for output
void r4aEsp32NvmMenuFileMove(const R4A_MENU_ENTRY * menuEntry,
                             const char * command,
                             Print * display);

// Remove the file
// Inputs:
//   menuEntry: Address of the object describing the menu entry
//   command: Zero terminated command string
//   display: Device used for output
void r4aEsp32NvmMenuFileRemove(const R4A_MENU_ENTRY * menuEntry,
                               const char * command,
                               Print * display);

// Get default parameters
// Inputs:
//   menuEntry: Address of the object describing the menu entry
//   command: Zero terminated command string
//   display: Device used for output
void r4aEsp32NvmMenuGetDefaultParameters(const struct _R4A_MENU_ENTRY * menuEntry,
                                         const char * command,
                                         Print * display);

// Display the help text with PPP
// Inputs:
//   menuEntry: Address of the object describing the menu entry
//   align: Zero terminated string of spaces for alignment
//   display: Device used for output
[[deprecated("Use r4aMenuHelpSuffix instead.")]]
void r4aEsp32NvmMenuHelpPppp(const struct _R4A_MENU_ENTRY * menuEntry,
                             const char * align,
                             Print * display);

// Download a file from a web server
// Inputs:
//   menuEntry: Address of the object describing the menu entry
//   command: Zero terminated command string
//   display: Device used for output
void r4aEsp32NvmMenuHttpFileGet(const R4A_MENU_ENTRY * menuEntry,
                                const char * command,
                                Print * display);

// Display the help text with PPPP XXXX
// Inputs:
//   menuEntry: Address of the object describing the menu entry
//   align: Zero terminated string of spaces for alignment
//   display: Device used for output
[[deprecated("Use r4aMenuHelpSuffix instead.")]]
void r4aEsp32NvmMenuHelpPpppXxxx(const struct _R4A_MENU_ENTRY * menuEntry,
                                 const char * align,
                                 Print * display);

// Clear the parameter
// Inputs:
//   menuEntry: Address of the object describing the menu entry
//   command: Zero terminated command string
//   display: Device used for output
void r4aEsp32NvmMenuParameterClear(const struct _R4A_MENU_ENTRY * menuEntry,
                                   const char * command,
                                   Print * display);

// Display the parameter
// Inputs:
//   menuEntry: Address of the object describing the menu entry
//   command: Zero terminated command string
//   display: Device used for output
void r4aEsp32NvmMenuParameterDisplay(const struct _R4A_MENU_ENTRY * menuEntry,
                                     const char * command,
                                     Print * display);

// Read the parameters from the parameter file
// Inputs:
//   menuEntry: Address of the object describing the menu entry
//   command: Zero terminated command string
//   display: Device used for output
void r4aEsp32NvmMenuParameterFileRead(const struct _R4A_MENU_ENTRY * menuEntry,
                                      const char * command,
                                      Print * display);

// Write the parameters to the parameter file
// Inputs:
//   menuEntry: Address of the object describing the menu entry
//   command: Zero terminated command string
//   display: Device used for output
void r4aEsp32NvmMenuParameterFileWrite(const struct _R4A_MENU_ENTRY * menuEntry,
                                       const char * command,
                                       Print * display);

// Set the parameter value
// Inputs:
//   menuEntry: Address of the object describing the menu entry
//   command: Zero terminated command string
//   display: Device used for output
void r4aEsp32NvmMenuParameterSet(const struct _R4A_MENU_ENTRY * menuEntry,
                                 const char * command,
                                 Print * display);

// Write a string to the parameter file
// Inputs:
//   file: File to which the string is written
//   string: Address of a zero terminated string of characters
// Outputs:
//   Returns true if all of the data was successfully written and false
//   upon error
bool r4aEsp32NvmWriteFileString(File &file, const char * string);

//****************************************
// OV2640 API
//****************************************

typedef struct _R4A_OV2640_PINS
{
    // Control pins
    int pinReset;
    int pinPowerDown;
    int pinXCLK;

    // I2C pins
    int pinI2cClk;
    int pinI2cData;

    // Frame synchronization
    int pinVSYNC;   // High at beginning of frame
    int pinHREF;    // High during each horizontal line
    int pinPCLK;    // Pixel clock

    // Image data pins
    int pinY2;
    int pinY3;
    int pinY4;
    int pinY5;
    int pinY6;
    int pinY7;
    int pinY8;
    int pinY9;
} R4A_OV2640_PINS;

// Process the frame buffer
// Inputs:
//   object: Address of a R4A_OV2640 data structure
//   frameBuffer: Buffer containing the raw image data
//   display: Address of Print object for output
// Outputs:
//   Returns true if the processing was successful and false upon error
typedef bool (* R4A_OV2640_PROCESS_FRAME_BUFFER)(struct _R4A_OV2640 * object,
                                                 camera_fb_t * frameBuffer,
                                                 Print * display);

// Process the web server's frame buffer
// Inputs:
//   object: Address of a R4A_OV2640 data structure
//   frameBuffer: Buffer containing the raw image data
// Outputs:
//   Returns true if the processing was successful and false upon error
typedef bool (* R4A_OV2640_PROCESS_WEB_SERVER_FRAME_BUFFER)(struct _R4A_OV2640 * object,
                                                            camera_fb_t * frameBuffer);

// OV2640 data structure declaration
typedef struct _R4A_OV2640
{
    // Constants, DO NOT MODIFY, set during structure initialization
    R4A_OV2640_PROCESS_FRAME_BUFFER _processFrameBuffer;
    R4A_OV2640_PROCESS_WEB_SERVER_FRAME_BUFFER _processWebServerFrameBuffer;

    uint32_t _clockHz;        // Input clock frequency for the OV2640
    R4A_I2C_BUS * _i2cBus;    // I2C bus to access the OV2640
    uint8_t  _i2cAddress;     // Address of the OV2640
    const R4A_OV2640_PINS * _pins; // ESP32 GPIO pins for the 0V2640 camera
} R4A_OV2640;

// Display a group of registers
// Inputs:
//   object: Address of a R4A_OV2640 data structure
//   firstRegister: The register address of the first register to be displayed
//   bytesToRead: The number of registers to display
//   display: Address of Print object for output
void r4aOv2640DisplayRegisters(R4A_OV2640 * object,
                               uint8_t firstRegister,
                               size_t bytesToRead,
                               Print * display);

// Dump all of the OV2640 registers in hexadecimal
// Inputs:
//   object: Address of a R4A_OV2640 data structure
//   display: Address of Print object for output
void r4aOv2640DumpRegisters(R4A_OV2640 * object,
                            Print * display);

// Initialize the camera
// Inputs:
//   object: Address of a R4A_OV2640 data structure
//   pixelFormat: Pixel format to use for the image
//   display: Address of Print object for debug output, may be nullptr
bool r4aOv2640Setup(R4A_OV2640 * object,
                    pixformat_t pixelFormat,
                    Print * display = nullptr);

// Update the camera processing state
// Inputs:
//   object: Address of a R4A_OV2640 data structure
//   display: Address of Print object for debug output, may be nullptr
void r4aOv2640Update(R4A_OV2640 * object,
                     Print * display = nullptr);

// Return a webpage to the requester containing a JPEG image
// Inputs:
//   request: Request from the browser
esp_err_t r4aOV2640JpegHandler(httpd_req_t *request);

extern bool r4aOv2640JpegDisplayTime;   // Set to true to display the JPEG conversion time
extern const R4A_OV2640_PINS r4aOV2640Pins; // ESP32 WRover camera pins

//****************************************
// SPI API
//****************************************

typedef struct _R4A_ESP32_SPI
{
    R4A_SPI spi;

    // Private
    uint32_t _div;
    spi_t * _spi;
} R4A_ESP32_SPI;

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
                      uint32_t clockHz);

// Transfer the data to the SPI device
// Inputs:
//   spi: Address of an R4A_SPI data structure
//   txBuffer: Address of the buffer containing the data to send
//   rxBuffer: Address of the receive data buffer
//   length: Number of data bytes in the buffer
void r4aEsp32SpiTransfer(struct _R4A_SPI * spi,
                         const uint8_t * txBuffer,
                         uint8_t * rxBuffer,
                         uint32_t length);

//****************************************
// Timer API
//****************************************

// Display the timer registers
// Inputs:
//   timerAddr: Address of the timer registers of interest
//   display: Device used for output
void r4aEsp32TimerDisplayTimerRegs(uint32_t timerAddr, Print * display = &Serial);

// Display the timer interrupt registers
// Inputs:
//   timerAddr: Address of the timer registers of interest
//   display: Device used for output
void r4aEsp32TimerDisplayTimerIntRegs(uint32_t timerAddr, Print * display = &Serial);

// Display the watchdog registers
// Inputs:
//   timerAddr: Address of the timer registers of interest
//   display: Device used for output
void r4aEsp32TimerDisplayWatchdogRegs(uint32_t timerAddr, Print * display = &Serial);

// Display the timer registers
// Inputs:
//   display: Device used for output
void r4aEsp32TimerDisplayRegs(Print * display = &Serial);

//****************************************
// Waypoint API
//****************************************

extern int r4aEsp32WpPointsToAverage;      // Number of points to average
extern const char * r4aEsp32WpFileName;    // Waypoint file name

// Add a point to the waypoint file
// Inputs:
//   menuEntry: Address of the object describing the menu entry
//   command: Zero terminated command string
//   display: Device used for output
void r4aEsp32WpMenuAddPoint(const R4A_MENU_ENTRY * menuEntry,
                            const char * command,
                            Print * display);

// Display a point from the waypoint file
// Inputs:
//   menuEntry: Address of the object describing the menu entry
//   command: Zero terminated command string
//   display: Device used for output
void r4aEsp32WpMenuDisplayPoint(const R4A_MENU_ENTRY * menuEntry,
                                const char * command,
                                Print * display);

// Set the waypoint file name
// Inputs:
//   menuEntry: Address of the object describing the menu entry
//   command: Zero terminated command string
//   display: Device used for output
void r4aEsp32WpMenuFileName(const R4A_MENU_ENTRY * menuEntry,
                            const char * command,
                            Print * display);

// Print the waypoint file contents
// Inputs:
//   menuEntry: Address of the object describing the menu entry
//   command: Zero terminated command string
//   display: Device used for output
void r4aEsp32WpMenuPrintFile(const R4A_MENU_ENTRY * menuEntry,
                             const char * command,
                             Print * display);

// Read a from the waypoint file
// Inputs:
//   file: Address of tha address of the waypoint file object
//   fileSize: Address of the value to receive the file size
//   latitude: Address to receive latitude in degrees
//   longitude: Address to receive longitude in degrees
//   altitude: Address to receive altitude in meters
//   horizontalAccuracy: Address to receive horizontal accuracy in meters
//   horizontalAccuracyStdDev: Address to receive horizontal accuracy standard deviation in meters
//   satellitesInView: Address to receive the number of satellites feeding the GNSS receiver
//   display: Device used for output, passed to computeWayPoint
// Outputs:
//   Returns true if the point was found and false when no more points
//   are available
bool r4aEsp32WpReadPoint(File * file,
                         double * latitude,
                         double * longitude,
                         double * altitude,
                         double * horizontalAccuracy,
                         double * horizontalAccuracyStdDev,
                         uint8_t * satellitesInView,
                         String * comment,
                         Print * display);

//****************************************
// Web Server API
//****************************************

// Update the configuration
// Inputs:
//   object: Address of a R4A_WEB_SERVER data structure
//   config: Address of the HTTP config object
typedef void (* R4A_WEB_SERVER_CONFIG_UPDATE)(struct _R4A_WEB_SERVER * object,
                                              httpd_config_t * config);

// Register the error handlers
// Inputs:
//   object: Address of a R4A_WEB_SERVER data structure
// Outputs:
//   Returns true if the all of the error handlers were installed and
//   false upon failure
typedef bool (* R4A_WEB_SERVER_REGISTER_ERROR_HANDLERS)(struct _R4A_WEB_SERVER * object);

// Register the URI handlers
// Inputs:
//   object: Address of a R4A_WEB_SERVER data structure
// Outputs:
//   Returns true if the all of the error handlers were installed and
//   false upon failure
typedef bool (* R4A_WEB_SERVER_REGISTER_URI_HANDLERS)(struct _R4A_WEB_SERVER * object);

typedef struct _R4A_WEB_SERVER
{
    R4A_WEB_SERVER_CONFIG_UPDATE _configUpdate;
    R4A_WEB_SERVER_REGISTER_ERROR_HANDLERS _registerErrorHandlers;
    R4A_WEB_SERVER_REGISTER_URI_HANDLERS _registerUriHandlers;
    uint16_t _port;             // Port number for the web server
    httpd_handle_t _webServer;  // HTTP server object
} R4A_WEB_SERVER;

extern Print * r4aWebServerDebug;   // Address of a Print object for web server debugging
extern const char * r4aWebServerDownloadArea;   // Directory path for the download area
extern const char * r4aWebServerNvmArea;   // Directory path for the NVM download area

// Check for extension
// Inputs:
//   object: Address of a R4A_WEB_SERVER data structure
//   path: Zero terminated string containing the file's path
//   extension: Zero terminated string containing the extension for comparison
// Outputs:
//   Returns true if the extension matches and false otherwise
bool webServerCheckExtension(R4A_WEB_SERVER * object,
                             const char * path,
                             const char * extension);

// Handle the web server errors
// Inputs:
//   request: httpd_req_t object containing the request from the browser
//   error: Error detected by the web server
// Outputs:
//   Returns status indicating if the response was successfully sent
//   to the browser
esp_err_t r4aWebServerError (httpd_req_t *request, httpd_err_code_t error);

// Download a file from the robot to the browser
// Inputs:
//   request: Address of a HTTP request object
// Outputs:
//   Returns the file download status
esp_err_t r4aWebServerFileDownload(httpd_req_t *request);

// Start the web server
// Inputs:
//   object: Address of a R4A_WEB_SERVER data structure
// Outputs:
//   Returns true if the web server was successfully started and false
//   upon failure
bool r4aWebServerStart(R4A_WEB_SERVER * object);

// Stop the web server
// Inputs:
//   object: Address of a R4A_WEB_SERVER data structure
void r4aWebServerStop(R4A_WEB_SERVER * object);

// Update the camera processing state
// Inputs:
//   object: Address of a R4A_WEB_SERVER data structure
//   wifiConnected: True when WiFi has an IP address and false otherwise
void r4aWebServerUpdate(R4A_WEB_SERVER * object, bool wifiConnected);

//****************************************
// WiFi API
//****************************************

typedef uint8_t R4A_WIFI_CHANNEL_t;

// Entry in the SSID and password table r4aSsidPassword
typedef struct _R4A_SSID_PASSWORD
{
    const char ** ssid;     // ID of access point
    const char ** password; // Password for the access point
} R4A_SSID_PASSWORD;

// List of known access points (APs)
extern const R4A_SSID_PASSWORD r4aWifiSsidPassword[];
extern const int r4aWifiSsidPasswordEntries;

// Class to simplify WiFi handling
class R4A_WIFI
{
  private:

    uint8_t _apCount;               // Number of AP in the list
    bool _apFound;                  // Remote AP found
    const char * _apPassword;       // Remote AP password
    const char * _apSSID;           // Remote AP SSID
    int _authType;                  // Remote AP authorization type
    const char * _hostName;         // Name of this host
    volatile bool _mdnsAvailable;   // True when the mDNS address translation is available
    volatile bool _stationConnected;// True if station is connected to remote AP
    volatile bool _stationHasIp;    // True if the station has an IP address
    R4A_WIFI_CHANNEL_t _wifiChannel;// WiFi channel in use
    bool _wifiScanRunning;          // WiFi scan is running
    uint32_t _wifiTimer;            // WiFi timer is running when non-zero

    // Connect to an access point
    // Inputs:
    //   apCount: Number of access points found during the scan
    // Outputs:
    //   Returns the channel number of the access point to which the connection
    //   attempt was made
    uint8_t connect(uint8_t apCount);

    // Start the WiFi event handler
    void eventHandlerStart();

    // Stop the WiFi event handler
    void eventHandlerStop();

    // Start the multicast domain name server
    // Outputs:
    //   Returns true if successful and false upon failure
    bool mDNSStart();

    // Stop the multicast domain name server
    void mDNSStop();

    // Connect the station to a remote AP
    bool stationConnectAP();

    // Disconnect the station from an AP
    // Outputs:
    //   Returns true if successful and false upon failure
    bool stationDisconnect();

    // Set the station's host name
    // Inputs:
    //   hostName: Zero terminated host name character string
    // Outputs:
    //   Returns true if successful and false upon failure
    bool stationHostName(const char * hostName);

    // Start the WiFi scan
    // Inputs:
    //   channel: Channel number for the scan, zero (0) scan all channels
    // Outputs:
    //   Returns true if successful and false upon failure
    bool stationScanStart(R4A_WIFI_CHANNEL_t channel);

    // Select the AP and channel to use for WiFi station
    // Inputs:
    //   connect: Set to true for connection
    //   list: Address of a Print object used to list the APs
    // Outputs:
    //   Returns the channel number of the AP
    R4A_WIFI_CHANNEL_t stationSelectAP(bool connect, Print * list);

    // Verify the WiFi tables
    void verifyTables();

    // Handle the WiFi event
    // Inputs:
    //   event: Arduino ESP32 event number found on
    //          https://github.com/espressif/arduino-esp32
    //          in libraries/Network/src/NetworkEvents.h
    //   info: Additional data about the event
    void wifiEvent(arduino_event_id_t event, arduino_event_info_t info);

  public:

    volatile Print * _debug;        // Display debugging data
    volatile Print * _display;      // Display WiFi IP address and loss

    // Constructor
    // Inputs:
    //   display: Print object to display WiFi startup summary
    //   debug: Print object to display WiFi debugging messages
    R4A_WIFI(Print * display = nullptr,
             Print * debug = nullptr);

    // Setup WiFi
    // Inputs:
    //   hostName: Zero terminated string of characters
    void begin(const char * hostName);

    // Get the active channel
    //   Returns the channel number or zero (0) if not connected
    uint8_t channelGet();

    // Handle the WiFi event
    // Inputs:
    //   events: Event number found in libraries/Network/src/NetworkEvent.h
    //   info: Information associated with the event
    void eventHandler(arduino_event_id_t event, arduino_event_info_t info);

    // Get the mDNS host name
    // Outputs:
    //   Returns the address of a zero terminated string of characters or nullptr
    const char * hostNameGet();

    // List the remote APs
    // Inputs:
    //   display: Address of a Print object used to list the APs
    void listAPs(Print * display);

    // Determine if mDNS is available
    // Outputs:
    //   True when the mDNS address translation is available
    bool mdnsAvailable();

    // Get the connected SSID
    //   Returns the address of a zero terminated string of characters or nullptr
    const char * ssidGet();

    // Determine if the stataion is connected to a remote access point
    // Outputs:
    //   True if station is connected to remote AP
    bool stationConnected();

    // Determine if the station has an IP address
    // Outputs:
    //    True if the station has an IP address
    bool stationHasIp();

    // Start the Wifi station
    // Outputs:
    //   Returns true if the WiFi station was started
    bool stationStart();

    // Stop the WiFi station
    // Outputs:
    //   Returns true if the WiFi station was stopped
    bool stationStop();

    // Test the WiFi modes, call from loop
    void test();

    // Check for reconnection after disconnect, call from loop
    void update();
};

#endif  // __R4A_ESP32_H__
