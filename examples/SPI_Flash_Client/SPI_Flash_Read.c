/**********************************************************************
  SPI_Flash_Read.c

  Program to read the SPI NOR Flash.
**********************************************************************/

#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>

#include "Dump_Buffer.h"
#include "SPI_Flash_Protocol.h"

#define BUFFER_LENGTH   1024

uint8_t readData[BUFFER_LENGTH];

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
            perror("ERROR: Failed to send read command!\r\n");
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
                perror("ERROR: Read data failure!\r\n");
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
// Inputs:
//   argc: Argument count
//   argv: Array of argument values
// Outputs:
//   Returns the value to the command line
int main(int argc, char **argv)
{
    ssize_t bytesWritten;
    int displayHelp;
    int exitStatus;
    int file;
    char * fileName;
    uint32_t flashAddress;
    uint32_t flashOffset;
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
        if ((argc < 4) || (argc > 5))
            break;

        // Get the server name
        serverName = argv[1];

        // Get the flash address
        string = argv[2];
        if ((sscanf(string, "0x%x", &flashAddress) != 1)
            && (sscanf(string, "0X%x", &flashAddress) != 1)
            && (sscanf(string, "%d", &flashAddress) != 1))
        {
            printf("ERROR: Invalid flash address\r\n");
            break;
        }

        // Get the length in bytes
        string = argv[3];
        if ((sscanf(string, "0x%lx", &lengthInBytes) != 1)
            && (sscanf(string, "0X%lx", &lengthInBytes) != 1)
            && (sscanf(string, "%ld", &lengthInBytes) != 1))
        {
            printf("ERROR: Invalid length in bytes\r\n");
            break;
        }

        // Determine if the line contains the file name
        fileName = NULL;
        if (argc == 5)
        {
            fileName = argv[4];

            // Open the file for write
            file = creat(fileName, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
            if (file < 0)
            {
                exitStatus = errno;
                perror("ERROR: Failed to open the file!\r\n");
                break;
            }
        }

        // Help is no longer necessary
        displayHelp = 0;
        exitStatus = 0;
        flashOffset = flashAddress;

        // Translate from a name to an IP address
        server = gethostbyname( serverName );
        if (!server)
        {
            exitStatus = h_errno;
            errno = h_errno;
            perror("ERROR: Server not found!\r\n");
            break;
        }

        // Create the socket
        sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (sockfd < 0)
        {
            exitStatus = errno;
            perror("ERROR: Failed to create the socket!\r\n");
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
            perror("ERROR: Failed to connect to the SPI Flash server!\r\n");
            break;
        }

        // Read all of the data
        while (lengthInBytes)
        {
            // Determine the transfer length
            transferLength = lengthInBytes;
            if (transferLength > BUFFER_LENGTH)
                transferLength = BUFFER_LENGTH;

            // Read the data from the SPI flash
            exitStatus = flashRead(sockfd,
                                   flashAddress,
                                   transferLength,
                                   readData);
            if (exitStatus)
                break;

            // Process the received data
            if (file < 0)
                dumpBuffer(flashOffset, readData, transferLength);
            else
            {
                bytesWritten = write(file, readData, transferLength);
                if (bytesWritten < 0)
                {
                    exitStatus = errno;
                    perror("ERROR: Failed to write data to the file!\r\n");
                    break;
                }
            }

            // Account for the data
            lengthInBytes -= transferLength;
            flashAddress += transferLength;
            flashOffset += transferLength;
        }
    } while (0);

    // Close the socket
    if (sockfd >= 0)
        close(sockfd);

    // Close the file
    if (file >= 0)
        close(file);

    // Display the help message
    if (displayHelp)
        printf("%s   server_name   flash_address   length_in_bytes   [file_name]\r\n",
               argv[0]);

    // Pass the status to the command line
    exit(exitStatus);
}
