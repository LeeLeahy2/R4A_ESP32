/**********************************************************************
  R4A_ESP32_I2S.h

  Robots-For-All (R4A)
  Declare the I2S controller constants and data structures
**********************************************************************/

#ifndef __R4A_ESP32_I2S_H__
#define __R4A_ESP32_I2S_H__

#define R4A_ESP32_I2S_0_CONTROLLER      0x3ff4f000
#define R4A_ESP32_I2S_1_CONTROLLER      0x3ff6d000

typedef struct _R4A_ESP32_I2S_REGS
{
    uint32_t I2S_FIFO_WR_REG;           // 0x00
    uint32_t I2S_FIFO_RD_REG;           // 0x04
    uint32_t I2S_CONF_REG;              // 0x08
    uint32_t I2S_INT_RAW_REG;           // 0x0c
    uint32_t I2S_INT_ST_REG;            // 0x10
    uint32_t I2S_INT_ENA_REG;           // 0x14
    uint32_t I2S_INT_CLR_REG;           // 0x18
    uint32_t I2S_TIMING_REG;            // 0x1c
    uint32_t I2S_FIFO_CONF_REG;         // 0x20
    uint32_t I2S_RXEOF_NUM_REG;         // 0x24
    uint32_t I2S_CONF_SINGLE_DATA_REG;  // 0x28
    uint32_t I2S_CONF_CHAN_REG;         // 0x2c
    uint32_t I2S_OUT_LINK_REG;          // 0x30
    uint32_t I2S_IN_LINK_REG;           // 0x34
    uint32_t I2S_OUT_EOF_DES_ADDR_REG;  // 0x38
    uint32_t I2S_IN_EOF_DES_ADDR_REG;   // 0x3c
    uint32_t I2S_OUT_EOF_BFR_DES_ADDR_REG;  // 0x40
    uint32_t I2S_RESERVED_0X44;         // 0x44
    uint32_t I2S_INLINK_DSCR_REG;       // 0x48
    uint32_t I2S_INLINK_DSCR_BF0_REG;   // 0x4c
    uint32_t I2S_INLINK_DSCR_BF1_REG;   // 0x50
    uint32_t I2S_OUTLINK_DSCR_REG;      // 0x54
    uint32_t I2S_OUTLINK_DSCR_BF0_REG;  // 0x58
    uint32_t I2S_OUTLINK_DSCR_BF1_REG;  // 0x5c
    uint32_t I2S_LC_CONF_REG;           // 0x60
    uint32_t I2S_RESERVED_0X64;         // 0x64
    uint32_t I2S_RESERVED_0X68;         // 0x68
    uint32_t I2S_LC_STATE0_REG;         // 0x6c
    uint32_t I2S_LC_STATE1_REG;         // 0x70
    uint32_t I2S_LU_HUNG_CONF_REG;      // 0x74
    uint32_t reserved_0x78;             // 0x78
    uint32_t reserved_0x7c;             // 0x7c
    uint32_t reserved_0x80;             // 0x80
    uint32_t reserved_0x84;             // 0x84
    uint32_t reserved_0x88;             // 0x88
    uint32_t reserved_0x8c;             // 0x8c
    uint32_t reserved_0x90;             // 0x90
    uint32_t reserved_0x94;             // 0x94
    uint32_t reserved_0x98;             // 0x98
    uint32_t reserved_0x9c;             // 0x9c
    uint32_t I2S_CONF1_REG;             // 0xa0
    uint32_t I2S_PD_CONF_REG;           // 0xa4
    uint32_t I2S_CONF2_REG;             // 0xa8
    uint32_t I2S_CLKM_CONF_REG;         // 0xac
    uint32_t I2S_SAMPLE_RATE_CONF_REG;  // 0xb0
    uint32_t I2S_PDM_CONF_REG;          // 0xb4
    uint32_t I2S_PDM_FREQ_CONF_REG;     // 0xb8
    uint32_t I2S_STATE_REG;             // 0xbc
} R4A_ESP32_I2S_REGS;

#endif  // __R4A_ESP32_I2S_H__
