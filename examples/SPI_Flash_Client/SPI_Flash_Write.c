/**********************************************************************
  SPI_Flash_Write.c

  Program to write the SPI NOR Flash.
**********************************************************************/

#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>

#include "Dump_Buffer.h"
#include "SPI_Flash_Protocol.h"

#define BUFFER_LENGTH   1024

uint8_t writeData[BUFFER_LENGTH];
uint8_t readData[BUFFER_LENGTH];

// SPI Flash Status register
#define STATUS_BUSY         0x80    // Write operation in progress
#define STATUS_SEC          0x20    // Security ID status
#define STATUS_WPLD         0x10    // Write protect lock down
#define STATUS_WSP          0x08    // Programming suspended
#define STATUS_WSE          0x04    // Erase suspended
#define STATUS_WEL          0x02    // Write enable

//*********************************************************************
// Enable block write access
// Inputs:
//   sockfd: Socket file descriptor
//   enable: Set true to enable write access and false to prevent write access
// Outputs:
//   Returns the erase status value
int flashBlockWriteEnable(int sockfd, int enable)
{
    ssize_t bytesReceived;
    ssize_t bytesWritten;
    R4A_SPI_FLASH_COMMAND cmd;
    int exitStatus;

    do
    {
        exitStatus = 0;

        // Build the block write enable command
        cmd.command = CMD_BLOCK_WRITE_ENABLE;
        cmd.address = enable;
        cmd.lengthInBytes = -1;

        // Send the block write enable command to the SPI Flash server
        if (send(sockfd, &cmd, sizeof(cmd), 0) < 0)
        {
            exitStatus = errno;
            perror("ERROR: Failed to send block write enable command!\n");
            break;
        }

        // Get the block write enable response from the server
        bytesReceived = recv(sockfd,
                             &cmd,
                             sizeof(cmd),
                             0);
        if (bytesReceived < sizeof(cmd))
        {
            exitStatus = errno;
            perror("ERROR: Failed to update the block write enables in the SPI flash!\n");
            break;
        }

        // Verify the block write enable operation status
        if ((cmd.command != CMD_BLOCK_ENABLE_SUCCESS)
            || (cmd.address != 0)
            || (cmd.lengthInBytes != 0))
        {
            exitStatus = -1;
            fprintf(stderr, "ERROR: Invalid block write enable status received!\n");
            break;
        }

        // Set the block write enable status
        printf("All blocks write %s\n", enable ? "enabled" : "protected");
        exitStatus = 0;
    } while (0);

    // Return the block write enable status
    return exitStatus;
}

//*********************************************************************
// Erase the SPI NOR Flash chip
// Inputs:
//   sockfd: Socket file descriptor
// Outputs:
//   Returns the erase status value
int flashEraseChip(int sockfd)
{
    ssize_t bytesReceived;
    ssize_t bytesWritten;
    R4A_SPI_FLASH_COMMAND cmd;
    int exitStatus;
    uint8_t status;

    do
    {
        exitStatus = 0;

        // Build the erase chip command
        cmd.command = CMD_ERASE_CHIP;
        cmd.address = 0;
        cmd.lengthInBytes = 0;

        // Send the erase chip command to the SPI Flash server
        if (send(sockfd, &cmd, sizeof(cmd), 0) < 0)
        {
            exitStatus = errno;
            perror("ERROR: Failed to send erase chip command!\n");
            break;
        }

        // Get the erase chip response from the server
        bytesReceived = recv(sockfd,
                             &cmd,
                             sizeof(cmd),
                             0);
        if (bytesReceived < sizeof(cmd))
        {
            exitStatus = errno;
            perror("ERROR: Failed to erase the SPI flash!\n");
            break;
        }

        // Verify the erase chip operation status
        status = (uint8_t)cmd.lengthInBytes & ~STATUS_WEL;
        if ((cmd.command != CMD_ERASE_SUCCESS)
            || (cmd.address != 0)
            || (status != 0))
        {
            exitStatus = -1;
            fprintf(stderr, "SPI Flash Status: 0x%02x\n", cmd.lengthInBytes);
            fprintf(stderr, "ERROR: Invalid erase status received!\n");
            break;
        }

        // Set the erase chip status
        printf("Chip erased\n");
        exitStatus = 0;
    } while (0);

    // Return the erase chip status
    return exitStatus;
}

//*********************************************************************
// Read data from the SPI NOR Flash device
// Inputs:
//   sockfd: Socket file descriptor
//   flashAddress: Address in the SPI flash to start reading data
//   transferLength: Number of bytes to read
//   buffer: Address of a buffer to receive the SPI flash data
// Outputs:
//   Returns the exit status value
int flashRead(int sockfd,
              uint32_t flashAddress,
              size_t transferLength,
              uint8_t * buffer)
{
    ssize_t bytesReceived;
    R4A_SPI_FLASH_COMMAND cmd;
    int exitStatus;
    ssize_t offset;

    do
    {
        exitStatus = 0;

        // Build the read data command
        cmd.command = CMD_READ_DATA;
        cmd.address = flashAddress;
        cmd.lengthInBytes = transferLength;

        // Send the read command to the SPI Flash server
        if (send(sockfd, &cmd, sizeof(cmd), 0) < 0)
        {
            exitStatus = errno;
            perror("ERROR: Failed to send read command!\n");
            break;
        }

        // Get the SPI flash data from the server
        offset = 0;
        while (offset < transferLength)
        {
            bytesReceived = recv(sockfd,
                                 &buffer[offset],
                                 transferLength - offset,
                                 0);
            if (bytesReceived < 0)
            {
                exitStatus = errno;
                perror("ERROR: Read data failure!\n");
                break;
            }

            // Account for this data
            offset += bytesReceived;
        }
    } while (0);

    // Return the read status
    return exitStatus;
}

//*********************************************************************
// Write data to the SPI NOR Flash device
// Inputs:
//   sockfd: Socket file descriptor
//   flashAddress: Address in the SPI flash to start reading data
//   transferLength: Number of bytes to read
//   buffer: Address of a buffer containing the data to write
// Outputs:
//   Returns the exit status value
int flashWrite(int sockfd,
               uint32_t flashAddress,
               size_t transferLength,
               uint8_t * buffer)
{
    ssize_t bytesReceived;
    ssize_t bytesWritten;
    R4A_SPI_FLASH_COMMAND cmd;
    int exitStatus;
    ssize_t offset;
    uint8_t status;

    do
    {
        exitStatus = 0;

        // Build the write command
        cmd.command = CMD_WRITE_DATA;
        cmd.address = flashAddress;
        cmd.lengthInBytes = transferLength;

        // Send the write command to the SPI Flash server
        if (send(sockfd, &cmd, sizeof(cmd), 0) < 0)
        {
            exitStatus = errno;
            perror("ERROR: Failed to send write command!\n");
            break;
        }

        // Send the SPI flash data to the server
        offset = 0;
        while (offset < transferLength)
        {
            bytesWritten = send(sockfd,
                                &buffer[offset],
                                transferLength - offset,
                                0);
            if (bytesWritten < 0)
            {
                exitStatus = errno;
                perror("ERROR: Failed to send write data!\n");
                break;
            }

            // Account for this data
            offset += bytesWritten;
        }
        if (exitStatus)
            break;

        // Wait for the response
        bytesReceived = recv(sockfd,
                             &cmd,
                             sizeof(cmd),
                             0);
        if (bytesReceived != sizeof(cmd))
        {
            exitStatus = errno;
            perror("ERROR: Failed write to SPI flash!\n");
            break;
        }

        // Verify the write operation status
        status = (uint8_t)cmd.lengthInBytes & ~STATUS_WEL;
        if ((cmd.command != CMD_WRITE_SUCCESS)
            || (cmd.address != 0)
            || (status != 0))
        {
            exitStatus = -1;
            fprintf(stderr, "SPI Flash Status: 0x%02x\n", cmd.lengthInBytes);
            fprintf(stderr, "ERROR: Invalid write status received!\n");
            break;
        }

        // Set the write status
        exitStatus = 0;
    } while (0);

    // Return the write status
    return exitStatus;
}

//*********************************************************************
// Display the difference between expected and actual
// Inputs:
//   flashAddress: Address of the first byte of flash data
//   expectedData: Buffer containing the expected data
//   actualData: Buffer containing the actual data from the SPI NOR Flash
//   transferLength: Number of valid data bytes
void displayDifference(uint32_t flashAddress,
                       uint8_t * expectedData,
                       uint8_t * actualData,
                       size_t transferLength)
{
    size_t endOffset;
    size_t length;
    size_t startOffset;

    startOffset = 0;
    while (startOffset < transferLength)
    {
        // Check for a difference in the data
        if (expectedData[startOffset] != actualData[startOffset])
        {
            endOffset = startOffset + 1;
            while (endOffset < transferLength)
            {
                // Check for matching data
                if (expectedData[endOffset] == actualData[endOffset])
                    break;

                // Different data, check the next data byte
                endOffset += 1;
            }

            // Display the first difference
            flashAddress += startOffset;
            length = endOffset - startOffset;

            // Display the expected data
            printf("Expected Data\n");
            dumpBuffer(flashAddress, &expectedData[startOffset], length);

            // Display the actual data
            printf("Actual Data\n");
            dumpBuffer(flashAddress, &actualData[startOffset], length);
        }

        // No difference, check the next data byte
        startOffset += 1;
    }
}

//*********************************************************************
// Inputs:
//   argc: Argument count
//   argv: Array of argument values
// Outputs:
//   Returns the value to the command line
int main(int argc, char **argv)
{
    size_t bytesRead;
    int displayHelp;
    int exitStatus;
    int file;
    size_t fileLength;
    char * fileName;
    uint32_t flashAddress;
    size_t lengthInBytes;
    ssize_t offset;
    struct hostent * server;
    struct sockaddr_in serverIpAddress;
    char * serverName;
    int sockfd;
    char * string;
    size_t transferLength;

    displayHelp = 1;
    exitStatus = -1;
    file = -1;
    sockfd = -1;
    do
    {
        // Verify the command line contains the correct number of parameters
        if (argc != 3)
            break;

        // Get the server name
        serverName = argv[1];

        // Determine if the line contains the file name
        fileName = argv[2];

        // Help is no longer necessary
        displayHelp = 0;
        exitStatus = 0;

        // Open the file for read
        file = open(fileName, O_RDONLY, 0);
        if (file < 0)
        {
            exitStatus = errno;
            perror("ERROR: Failed to open the file!\n");
            break;
        }

        // Get the file length
        fileLength = lseek(file, 0, SEEK_END);
        if (fileLength == -1)
        {
            exitStatus = errno;
            perror("ERROR: Error determining file size!\n");
            break;
        }

        // Start reading from the beginning of the file
        if (lseek(file, 0, SEEK_SET) == -1)
        {
            exitStatus = errno;
            perror("ERROR: Error setting read starting point!\n");
            break;
        }

        // Translate from a name to an IP address
        server = gethostbyname( serverName );
        if (!server)
        {
            exitStatus = h_errno;
            errno = h_errno;
            perror("ERROR: Server not found!\n");
            break;
        }

        // Create the socket
        sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (sockfd < 0)
        {
            exitStatus = errno;
            perror("ERROR: Failed to create the socket!\n");
            break;
        }

        // Initialize the server address
        memset(&serverIpAddress, 0, sizeof(serverIpAddress));
        serverIpAddress.sin_family = AF_INET;
        memcpy(&serverIpAddress.sin_addr.s_addr, server->h_addr, server->h_length);
        serverIpAddress.sin_port = htons( SPI_FLASH_SERVER_PORT );

        // Attempt to connect to the server
        if (connect(sockfd,(struct sockaddr *)&serverIpAddress, sizeof(serverIpAddress)))
        {
            exitStatus = errno;
            perror("ERROR: Failed to connect to the SPI Flash server!\n");
            break;
        }

        // Enable block writes
        exitStatus = flashBlockWriteEnable(sockfd, 1);
        if (exitStatus)
            break;

        do
        {
            // Erase the SPI flash
            exitStatus = flashEraseChip(sockfd);
            if (exitStatus)
                break;

            // Write all of the data
            flashAddress = 0;
            lengthInBytes = fileLength;
            while (lengthInBytes)
            {
                // Determine the transfer length
                transferLength = lengthInBytes;
                if (transferLength > BUFFER_LENGTH)
                    transferLength = BUFFER_LENGTH;

                // Get the data from the file
                bytesRead = read(file, writeData, transferLength);
                if (bytesRead < 0)
                {
                    exitStatus = errno;
                    perror("ERROR: Failed to read data from the file!\n");
                    break;
                }

                // Write the data to the SPI flash
                exitStatus = flashWrite(sockfd,
                                        flashAddress,
                                        transferLength,
                                        writeData);
                if (exitStatus)
                    break;

                // Read the data from the SPI flash
                exitStatus = flashRead(sockfd,
                                       flashAddress,
                                       transferLength,
                                       readData);
                if (exitStatus)
                    break;

                // Verify the received data
                if (memcmp(writeData, readData, transferLength) != 0)
                {
                    exitStatus = -1;
                    displayDifference(flashAddress, writeData, readData, transferLength);
                    perror("ERROR: Failed to verify data successfully written to flash!\n");
                    break;
                }

                // Account for the data
                printf("0x%08x\n", flashAddress);
                lengthInBytes -= transferLength;
                flashAddress += transferLength;
            }
        } while (0);

        // Disable block writes
        flashBlockWriteEnable(sockfd, 0);
    } while (0);

    // Close the socket
    if (sockfd >= 0)
        close(sockfd);

    // Close the file
    if (file >= 0)
        close(file);

    // Display the help message
    if (displayHelp)
        printf("%s   server_name   file_name\n",
               argv[0]);

    // Pass the status to the command line
    exit(exitStatus);
}
