/**********************************************************************
  R4A_ESP32_SPI.h

  Robots-For-All (R4A)
  Declare the SPI constants
**********************************************************************/

#ifndef __R4A_ESP32_SPI_H__
#define __R4A_ESP32_SPI_H__

#define R4A_ESP32_SPI_0_CONTROLLER      0x3FF43000
#define R4A_ESP32_SPI_1_CONTROLLER      0x3FF42000
#define R4A_ESP32_SPI_2_CONTROLLER      0x3FF64000
#define R4A_ESP32_SPI_3_CONTROLLER      0x3FF65000

typedef struct _R4A_ESP32_SPI_REGS
{
    uint32_t SPI_CMD;                   // 0x000
    uint32_t SPI_ADDR;                  // 0x004
    uint32_t SPI_CTRL;                  // 0x008
    uint32_t reserved_0x00c;
    uint32_t reserved_0x010;
    uint32_t SPI_CTRL2;                 // 0x014
    uint32_t SPI_CLOCK;                 // 0x018
    uint32_t SPI_USER;                  // 0x01c
    uint32_t SPI_USER1;                 // 0x020
    uint32_t SPI_USER2;                 // 0x024
    uint32_t SPI_MOSI_DLEN;             // 0x028
    uint32_t reserved_0x02c;
    uint32_t SPI_SLV_WR_STATUS;         // 0x030
    uint32_t SPI_PIN_REG;               // 0x034
    uint32_t SPI_SLAVE;                 // 0x038
    uint32_t SPI_SLAVE1;                // 0x03c
    uint32_t SPI_SLAVE2;                // 0x040
    uint32_t reserved_0x044;
    uint32_t SPI_SLV_WRBUF_DLEN;        // 0x048
    uint32_t SPI_SLV_RDBUF_DLEN;        // 0x04c
    uint32_t reserved_0x050;
    uint32_t reserved_0x054;
    uint32_t reserved_0x058;
    uint32_t reserved_0x05c;
    uint32_t reserved_0x060;
    uint32_t SPI_SLV_RD_BIT;            // 0x064
    uint32_t reserved_0x068;
    uint32_t reserved_0x06c;
    uint32_t reserved_0x070;
    uint32_t reserved_0x074;
    uint32_t reserved_0x078;
    uint32_t reserved_0x07c;
    uint32_t SPI_W0;                    // 0x080
    uint32_t SPI_W1;                    // 0x084
    uint32_t SPI_W2;                    // 0x088
    uint32_t SPI_W3;                    // 0x08c
    uint32_t SPI_W4;                    // 0x090
    uint32_t SPI_W5;                    // 0x094
    uint32_t SPI_W6;                    // 0x098
    uint32_t SPI_W7;                    // 0x09c
    uint32_t SPI_W8;                    // 0x0a0
    uint32_t SPI_W9;                    // 0x0a4
    uint32_t SPI_W10;                   // 0x0a8
    uint32_t SPI_W11;                   // 0x0ac
    uint32_t SPI_W12;                   // 0x0b0
    uint32_t SPI_W13;                   // 0x0b4
    uint32_t SPI_W14;                   // 0x0b8
    uint32_t SPI_W15;                   // 0x0bc
    uint32_t reserved_0x0c0;
    uint32_t reserved_0x0c4;
    uint32_t reserved_0x0c8;
    uint32_t reserved_0x0cc;
    uint32_t reserved_0x0d0;
    uint32_t reserved_0x0d4;
    uint32_t reserved_0x0d8;
    uint32_t reserved_0x0dc;
    uint32_t reserved_0x0e0;
    uint32_t reserved_0x0e4;
    uint32_t reserved_0x0e8;
    uint32_t reserved_0x0ec;
    uint32_t reserved_0x0f0;
    uint32_t reserved_0x0f4;
    uint32_t reserved_0x0f8;
    uint32_t reserved_0x0fc;
    uint32_t SPI_DMA_CONF;              // 0x100
    uint32_t SPI_DMA_OUT_LINK;          // 0x104
    uint32_t SPI_DMA_IN_LINK;           // 0x108
    uint32_t SPI_DMA_STATUS;            // 0x10c
    uint32_t SPI_DMA_INT_ENA;           // 0x110
    uint32_t SPI_DMA_INT_RAW;           // 0x114
    uint32_t SPI_DMA_INT_ST;            // 0x118
    uint32_t SPI_DMA_INT_CLR;           // 0x11c
    uint32_t SPI_IN_ERR_EOF_DES_ADDR;   // 0x120
    uint32_t SPI_IN_SUC_EOF_DES_ADDR;   // 0x124
    uint32_t SPI_INLINK_DSCR;           // 0x128
    uint32_t SPI_INLINK_DSCR_BFO;       // 0x12c
    uint32_t SPI_INLINK_DSCR_BF1;       // 0x130
    uint32_t SPI_OUT_EOF_BFR_DES_ADDR;  // 0x134
    uint32_t SPI_OUT_EOF_DES_ADDR;      // 0x138
    uint32_t SPI_OUTLINK_DSCR;          // 0x13c
    uint32_t SPI_OUTLINK_DSCR_BF0;      // 0x140
    uint32_t SPI_OUTLINK_DSCR_BF1;      // 0x144
    uint32_t SPI_DMA_RSTATUS;           // 0x148
    uint32_t SPI_DMA_TSTATUS;           // 0x14c
} R4A_ESP32_SPI_REGS;

#endif  // __R4A_ESP32_SPI_H__
