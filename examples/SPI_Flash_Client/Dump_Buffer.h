/**********************************************************************
  Dump_Buffer.h

  Support routine declaration
**********************************************************************/

#include <stdint.h>
#include <sys/types.h>
#include <unistd.h>

#ifndef __DUMP_BUFFER_H__
#define __DUMP_BUFFER_H__

// Dump the contents of a buffer
void dumpBuffer(uint32_t offset,
                const uint8_t *buffer,
                uint32_t length);

#endif  // __DUMP_BUFFER_H__
