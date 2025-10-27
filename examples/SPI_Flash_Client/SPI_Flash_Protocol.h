/**********************************************************************
  SPI_Flash_Protocol.h

  Command protocol for the SPI Flash server.
**********************************************************************/

#include <sys/types.h>
#include <unistd.h>

#ifndef __SPI_FLASH_PROTOCOL_H__
#define __SPI_FLASH_PROTOCOL_H__

// Port for SPI Flash server
#define SPI_FLASH_SERVER_PORT   6868

// SPI Flash server commands
#define CMD_READ_COMMAND            0
#define CMD_READ_DATA               1
#define CMD_WRITE_DATA              2
#define CMD_WRITE_SUCCESS           3
#define CMD_ERASE_CHIP              4
#define CMD_ERASE_SUCCESS           5
#define CMD_BLOCK_WRITE_ENABLE      6
#define CMD_BLOCK_ENABLE_SUCCESS    7

// SPI Flash server command message
typedef struct _R4A_SPI_FLASH_COMMAND
{
    uint32_t address;       // Flash address
    uint16_t lengthInBytes; // Number of bytes to read or write
    uint8_t command;        // See CMD_* above
} R4A_SPI_FLASH_COMMAND;

#endif  // __SPI_FLASH_PROTOCOL_H__
