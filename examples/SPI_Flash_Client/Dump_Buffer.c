/**********************************************************************
  Dump_Buffer.c

  Support routine to display the contents of a buffer
**********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Dump_Buffer.h"

// Dump the contents of a buffer
void dumpBuffer(uint32_t offset,
                const uint8_t *buffer,
                uint32_t length)
{
    int bytes;
    const uint8_t *end;
    int index;
    char line[132];

    end = &buffer[length];
    while (buffer < end)
    {
        // Determine the number of bytes to display on the line
        bytes = end - buffer;
        if (bytes > (16 - (offset & 0xf)))
            bytes = 16 - (offset & 0xf);

        // Display the offset
        sprintf(line, "0x%08x: ", offset);

        // Skip leading bytes
        for (index = 0; index < (offset & 0xf); index++)
            strcat(&line[strlen(line)], "   ");

        // Display the data bytes
        for (index = 0; index < bytes; index++)
            sprintf(&line[strlen(line)], "%02x ", buffer[index]);

        // Separate the data bytes from the ASCII
        for (; index < (16 - (offset & 0xf)); index++)
            strcat(&line[strlen(line)], "   ");
        strcat(&line[strlen(line)], " ");

        // Skip leading bytes
        for (index = 0; index < (offset & 0xf); index++)
            strcat(&line[strlen(line)], " ");

        // Display the ASCII values
        for (index = 0; index < bytes; index++)
            sprintf(&line[strlen(line)], "%c", ((buffer[index] < ' ') || (buffer[index] >= 0x7f)) ? '.' : buffer[index]);

        // Output the line
        printf("%s\r\n", line);

        // Set the next line of data
        buffer += bytes;
        offset += bytes;
    }
}
