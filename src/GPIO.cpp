/**********************************************************************
  GPIO.cpp

  Robots-For-All (R4A)
  Decode and display the GPIO pins
**********************************************************************/

#include "R4A_ESP32.h"

//****************************************
// Constants
//****************************************

R4A_GPIO_REGS * const r4aGpioRegs = (R4A_GPIO_REGS *)0x3ff44000;
R4A_IO_MUX_REGS * const r4aIoMux = (R4A_IO_MUX_REGS *)0x3ff49000;
R4A_RTCIO_REGS * const r4aRtcioRegs = (R4A_RTCIO_REGS *)0x3ff48400;

const int r4aGpioPortToIoMuxIndex[R4A_GPIO_MAX_PORTS] =
{ // 0   1   2   3   4   5   6   7   8   9
    17, 34, 16, 33, 18, 27, 24, 25, 26, 21,     //  0
    22, 23, 13, 14, 12, 15, 19, 20, 28, 29,     // 10
    30, 31, 32, 35, 36,  9, 10, 11, -1, -1,     // 20
    -1, -1,  7,  8,  5,  6,  1,  2,  3,  4,     // 30
};

const char * const r4aIoMuxFunctionNames[R4A_GPIO_MAX_PORTS][8] =
{ //    0            1         2          3            4             5           6     7
    "GPIO0",    "CLK_OUT1", "GPIO0",  "-",          "-",        "EMAC_TX_CLK",  "-",  "-", //  0
    "U0TXD",    "CLK_OUT3", "GPIO1",  "-",          "-",        "EMAC_RXD2",    "-",  "-", //  1
    "GPIO2",    "HSPIWP",   "GPIO2",  "HS2_DATA0",  "SD_DATA0", "-",            "-",  "-", //  2
    "U0RXD",    "CLK_OUT2", "GPIO3",  "-",          "-",        "-",            "-",  "-", //  3
    "GPIO4",    "HSPIHD",   "GPIO4",  "HS2_DATA1",  "SD_DATA1", "EMAC_TX_ER",   "-",  "-", //  4
    "GPIO5",    "VSPICS0",  "GPIO5",  "HS1_DATA6",  "-",        "EMAC_RX_CLK",  "-",  "-", //  5
    "SD_CLK",   "SPICLK",   "GPIO6",  "HS1_CLK",    "U1CTS",    "-",            "-",  "-", //  6
    "SD_DATA0", "SPIQ",     "GPIO7",  "HS1_DATA0",  "U2RTS",    "-",            "-",  "-", //  7
    "SD_DATA1", "SPID",     "GPIO8",  "HS1_DATA1",  "U2CTS",    "-",            "-",  "-", //  8
    "SD_DATA2", "SPIHD",    "GPIO9",  "HS1_DATA2",  "U1RXD",    "-",            "-",  "-", //  9
    "SD_DATA3", "SPIWP",    "GPIO10", "HS1_DATA3",  "U1TXD",    "-",            "-",  "-", //  10
    "SD_CMD",   "SPICS0",   "GPIO11", "HS1_CMD",    "U1RTS",    "-",            "-",  "-", //  11
    "MTDI",     "HSPIQ",    "GPIO12", "HS2_DATA2",  "SD_DATA2", "EMAC_TXD3",    "-",  "-", //  12
    "MTCK",     "HSPID",    "GPIO13", "HS2_DATA3",  "SD_DATA3", "EMAC_RX_ER",   "-",  "-", //  13
    "MTMS",     "HSPICLK",  "GPIO14", "HS2_CLK",    "SD_CLK",   "EMAC_TXD2",    "-",  "-", //  14
    "MTDO",     "HSPICS0",  "GPIO15", "HS2_CMD",    "SD_CMD",   "EMAC_RXD3",    "-",  "-", //  15
    "GPIO16",   "-",        "GPIO16", "HS1_DATA4",  "U2RXD",    "EMAC_CLK_OUT", "-",  "-", //  16
    "GPIO17",   "-",        "GPIO17", "HS1_DATA5",  "U2TXD",    "EMAC_CLK_180", "-",  "-", //  17
    "GPIO18",   "VSPICLK",  "GPIO18", "HS1_DATA7",  "-",        "-",            "-",  "-", //  18
    "GPIO19",   "VSPIQ",    "GPIO19", "U0CTS",      "-",        "EMAC_TXD0",    "-",  "-", //  19
    "-",        "-",        "-",      "-",          "-",        "-",            "-",  "-", //  20
    "GPIO21",   "VSPIHD",   "GPIO21", "-",          "-",        "EMAC_TX_EN",   "-",  "-", //  21
    "GPIO22",   "VSPIWP",   "GPIO22", "U0RTS",      "-",        "EMAC_TXD1",    "-",  "-", //  22
    "GPIO23",   "VSPID",    "GPIO23", "HS1_STROBE", "-",        "-",            "-",  "-", //  23
    "-",        "-",        "-",      "-",          "-",        "-",            "-",  "-", //  24
    "GPIO25",   "-",        "GPIO25", "-",          "-",        "EMAC_RXD0",    "-",  "-", //  25
    "GPIO26",   "-",        "GPIO26", "-",          "-",        "EMAC_RXD1",    "-",  "-", //  26
    "GPIO27",   "-",        "GPIO27", "-",          "-",        "EMAC_RX_DV",   "-",  "-", //  27
    "-",        "-",        "-",      "-",          "-",        "-",            "-",  "-", //  28
    "-",        "-",        "-",      "-",          "-",        "-",            "-",  "-", //  29
    "-",        "-",        "-",      "-",          "-",        "-",            "-",  "-", //  30
    "-",        "-",        "-",      "-",          "-",        "-",            "-",  "-", //  31
    "GPIO32",   "-",        "GPIO32", "-",          "-",        "-",            "-",  "-", //  32
    "GPIO33",   "-",        "GPIO33", "-",          "-",        "-",            "-",  "-", //  33
    "GPIO34",   "-",        "GPIO34", "-",          "-",        "-",            "-",  "-", //  34
    "GPIO35",   "-",        "GPIO35", "-",          "-",        "-",            "-",  "-", //  35
    "GPIO36",   "-",        "GPIO36", "-",          "-",        "-",            "-",  "-", //  36
    "GPIO37",   "-",        "GPIO37", "-",          "-",        "-",            "-",  "-", //  37
    "GPIO38",   "-",        "GPIO38", "-",          "-",        "-",            "-",  "-", //  38
    "GPIO39",   "-",        "GPIO39", "-",          "-",        "-",            "-",  "-", //  39
};

const uint8_t r4aIoMuxIsGpio[R4A_GPIO_MAX_PORTS] =
{
    0b00000101, //  0
    0b00000101, //  1
    0b00000101, //  2
    0b00000101, //  3
    0b00000101, //  4
    0b00000101, //  5
    0b00000100, //  6
    0b00000100, //  7
    0b00000100, //  8
    0b00000100, //  9
    0b00000100, // 10
    0b00000100, // 11
    0b00000100, // 12
    0b00000100, // 13
    0b00000100, // 14
    0b00000100, // 15
    0b00000101, // 16
    0b00000101, // 17
    0b00000101, // 18
    0b00000101, // 19
    0b00000000, // 20
    0b00000101, // 21
    0b00000101, // 22
    0b00000101, // 23
    0b00000000, // 24
    0b00000101, // 25
    0b00000101, // 26
    0b00000101, // 27
    0b00000000, // 28
    0b00000000, // 29
    0b00000000, // 30
    0b00000000, // 31
    0b00000101, // 32
    0b00000101, // 33
    0b00000101, // 34
    0b00000101, // 35
    0b00000101, // 36
    0b00000101, // 37
    0b00000101, // 38
    0b00000101, // 39
};

const R4A_GPIO_MATRIX r4aGpioMatrixNames[256] =
{ //  Input                    Output
    {"SPICLK_in",             "SPICLK_out"},        //   0
    {"SPIQ_in",               "SPIQ_out"},          //   1
    {"SPID_in",               "SPID_out"},          //   2
    {"SPIHD_in",              "SPIHD_out"},         //   3
    {"SPIWP_in",              "SPIWP_out"},         //   4
    {"SPICS0_in",             "SPICS0_out"},        //   5
    {"SPICS1_in",             "SPICS1_out"},        //   6
    {"SPICS2_in",             "SPICS2_out"},        //   7
    {"HSPICLK_in",            "HSPICLK_out"},       //   8
    {"HSPIQ_in",              "HSPIQ_out"},         //   9
    {"HSPID_in",              "HSPID_out"},         //  10
    {"HSPICS0_in",            "HSPICS0_out"},       //  11
    {"HSPIHD_in",             "HSPIHD_out"},        //  12
    {"HSPIWP_in",             "HSPIWP_out"},        //  13
    {"U0RXD_in",              "U0RXD_out"},         //  14
    {"U0CTS_in",              "U0CTS_out"},         //  15
    {"U0DSR_in",              "U0DSR_out"},         //  16
    {"U1RXD_in",              "U1RXD_out"},         //  17
    {"U1CTS_in",              "U1CTS_out"},         //  18
    {"-",                     "-"},                 //  19
    {"-",                     "-"},                 //  20
    {"-",                     "-"},                 //  21
    {"-",                     "-"},                 //  22
    {"I2S0O_BCK_in",          "I2S0O_BCK_out"},     //  23
    {"I2S10_BCK_in",          "I2S10_BCK_out"},     //  24
    {"I2S0O_WS_in",           "I2S0O_WS_out"},      //  25
    {"I2S1O_WS_in",           "I2S1O_WS_out"},      //  26
    {"I2S0I_BCK_in",          "I2S0I_BCK_out"},     //  27
    {"I2S0I_WS_in",           "I2S0I_WS_out"},      //  28
    {"I2CEXT0_SCL_in",        "I2CEXT0_SCL_out"},   //  29
    {"I2CEXT0_SDA_in",        "I2CEXT0_SDA_out"},   //  30
    {"pwm0_sync0_in",         "sdio_tohost_int_out"},   //  31
    {"pwm0_sync1_in",         "pwm0_out0a"},        //  32
    {"pwm0_sync2_in",         "pwm0_out0b"},        //  33
    {"pwm0_f0_in",            "pwm0_out1a"},        //  34
    {"pwm0_f1_in",            "pwm0_out1b"},        //  35
    {"pwm0_f2_in",            "pwm0_out2a"},        //  36
    {"-",                     "pwm0_out2b"},        //  37
    {"-",                     "-"},                 //  38
    {"pcnt_sig_ch0_in0",      "-"},                 //  39
    {"pcnt_sig_ch1_in0",      "-"},                 //  40
    {"pcnt_ctrl_ch0_in0",     "-"},                 //  41
    {"pcnt_ctrl_ch1_in0",     "-"},                 //  42
    {"pcnt_sig_ch0_in1-",     "-"},                 //  43
    {"pcnt_sig_ch1_in1",      "-"},                 //  44
    {"pcnt_ctrl_ch0_in1",     "-"},                 //  45
    {"pcnt_ctrl_ch1_in1",     "-"},                 //  46
    {"pcnt_sig_ch0_in2",      "-"},                 //  47
    {"pcnt_sig_ch1_in2",      "-"},                 //  48
    {"pcnt_ctrl_ch0_in2",     "-"},                 //  49
    {"pcnt_ctrl_ch1_in2",     "-"},                 //  50
    {"pcnt_sig_ch0_in3",      "-"},                 //  51
    {"pcnt_sig_ch1_in3",      "-"},                 //  52
    {"pcnt_ctrl_ch0_in3",     "-"},                 //  53
    {"pcnt_ctrl_ch1_in3",     "-"},                 //  54
    {"pcnt_sig_ch0_in4",      "-"},                 //  55
    {"pcnt_sig_ch1_in4",      "-"},                 //  56
    {"pcnt_ctrl_ch0_in4",     "-"},                 //  57
    {"pcnt_ctrl_ch1_in4",     "-"},                 //  58
    {"-",                     "-"},                 //  59
    {"-",                     "-"},                 //  60
    {"HSPICS1_in",            "HSPICS1_out"},       //  61
    {"HSPICS2_in",            "HSPICS2_out"},       //  62
    {"VSPICLK_in",            "VSPICLK_out"},       //  63
    {"VSPIQ_in",              "VSPIQ_out"},         //  64
    {"VSPID_in",              "VSPID_out"},         //  65
    {"VSPIHD_in",             "VSPIHD_out"},        //  66
    {"VSPIWP_in",             "VSPIWP_out"},        //  67
    {"VSPICS0_in",            "VSPICS0_out"},       //  68
    {"VSPICS1_in",            "VSPICS1_out"},       //  69
    {"VSPICS2_in",            "VSPICS2_out"},       //  70
    {"pcnt_sig_ch0_in5",      "ledc_hs_sig_out0"},  //  71
    {"pcnt_sig_ch1_in5",      "ledc_hs_sig_out1"},  //  72
    {"pcnt_ctrl_ch0_in5",     "ledc_hs_sig_out2"},  //  73
    {"pcnt_ctrl_ch1_in5",     "ledc_hs_sig_out3"},  //  74
    {"pcnt_sig_ch0_in6",      "ledc_hs_sig_out4"},  //  75
    {"pcnt_sig_ch1_in6",      "ledc_hs_sig_out5"},  //  76
    {"pcnt_ctrl_ch0_in6",     "ledc_hs_sig_out6"},  //  77
    {"pcnt_ctrl_ch1_in6",     "ledc_hs_sig_out7"},  //  78
    {"pcnt_sig_ch0_in7",      "ledc_ls_sig_out0"},  //  79
    {"pcnt_sig_ch1_in7",      "ledc_ls_sig_out1"},  //  80
    {"pcnt_ctrl_ch0_in7",     "ledc_ls_sig_out2"},  //  81
    {"pcnt_ctrl_ch1_in7",     "ledc_ls_sig_out3"},  //  82
    {"rmt_sig_in0",           "ledc_ls_sig_out4"},  //  83
    {"rmt_sig_in1",           "ledc_ls_sig_out5"},  //  84
    {"rmt_sig_in2",           "ledc_ls_sig_out6"},  //  85
    {"rmt_sig_in3",           "ledc_ls_sig_out7"},  //  86
    {"rmt_sig_in4",           "rmt_sig_out0"},      //  87
    {"rmt_sig_in5",           "rmt_sig_out1"},      //  88
    {"rmt_sig_in6",           "rmt_sig_out2"},      //  89
    {"rmt_sig_in7",           "rmt_sig_out3"},      //  90
    {"-",                     "rmt_sig_out4"},      //  91
    {"-",                     "rmt_sig_out5"},      //  92
    {"-",                     "rmt_sig_out6"},      //  93
    {"-",                     "rmt_sig_out7"},      //  94
    {"I2CEXT1_SCL_in",        "I2CEXT1_SCL_out"},   //  95
    {"I2CEXT1_SDA_in",        "I2CEXT1_SDA_out"},   //  96
    {"host_card_detect_n_1",  "host_ccmd_od_pullup_en_n"},    //  97
    {"host_card_detect_n_2",  "host_rst_n_1"},      //  98
    {"host_card_write_prt_1", "host_rst_n_2"},      //  99
    {"host_card_write_prt_2", "gpio_sd0_out"},      // 100
    {"host_card_int_n_1",     "gpio_sd1_out"},      // 101
    {"host_card_int_n_2",     "gpio_sd2_out"},      // 102
    {"pwm1_sync0_in",         "gpio_sd3_out"},      // 103
    {"pwm1_sync1_in",         "gpio_sd4_out"},      // 104
    {"pwm1_sync2_in",         "gpio_sd5_out"},      // 105
    {"pwm1_f0_in",            "gpio_sd6_out"},      // 106
    {"pwm1_f1_in",            "gpio_sd7_out"},      // 107
    {"pwm1_f2_in",            "pwm1_out0a"},        // 108
    {"pwm0_cap0_in",          "pwm1_out0b"},        // 109
    {"pwm0_cap1_in",          "pwm1_out1a"},        // 110
    {"pwm0_cap2_in",          "pwm1_out1b"},        // 111
    {"pwm1_cap0_in",          "pwm1_out2a"},        // 112
    {"pwm1_cap1_in",          "pwm1_out2b"},        // 113
    {"pwm1_cap2_in",          "pwm2_out1h"},        // 114
    {"-pwm2_flta",            "pwm2_out1h"},        // 115
    {"pwm2_fltb",             "pwm2_out2h"},        // 116
    {"pwm2_cap1_in",          "pwm2_outl"},         // 117
    {"pwm2_cap2_in",          "pwm2_out3h"},        // 118
    {"pwm2_cap3_in",          "pwm2_out3l"},        // 119
    {"pwm3_flta",             "pwm2_out4h"},        // 120
    {"pwm3_fltb",             "pwm2_out4l"},        // 121
    {"pwm3_cap1_in",          "-"},                 // 122
    {"pwm3_cap2_in",          "-"},                 // 123
    {"pwm3_cap3_in",          "-"},                 // 124
    {"-",                     "-"},                 // 125
    {"-",                     "-"},                 // 126
    {"-",                     "-"},                 // 127
    {"-",                     "-"},                 // 128
    {"-",                     "-"},                 // 129
    {"-",                     "-"},                 // 130
    {"-",                     "-"},                 // 131
    {"-",                     "-"},                 // 132
    {"-",                     "-"},                 // 133
    {"-",                     "-"},                 // 134
    {"-",                     "-"},                 // 135
    {"-",                     "-"},                 // 136
    {"-",                     "-"},                 // 137
    {"-",                     "-"},                 // 138
    {"-",                     "-"},                 // 139
    {"I2S0I_DATA_in0",        "I2S0I_DATA_out0"},   // 140
    {"I2S0I_DATA_in1",        "I2S0I_DATA_out1"},   // 141
    {"I2S0I_DATA_in2",        "I2S0I_DATA_out2"},   // 142
    {"I2S0I_DATA_in3",        "I2S0I_DATA_out3"},   // 143
    {"I2S0I_DATA_in4",        "I2S0I_DATA_out4"},   // 144
    {"I2S0I_DATA_in5",        "I2S0I_DATA_out5"},   // 145
    {"I2S0I_DATA_in6",        "I2S0I_DATA_out6"},   // 146
    {"I2S0I_DATA_in7",        "I2S0I_DATA_out7"},   // 147
    {"I2S0I_DATA_in8",        "I2S0I_DATA_out8"},   // 148
    {"I2S0I_DATA_in9",        "I2S0I_DATA_out9"},   // 149
    {"I2S0I_DATA_in10",       "I2S0I_DATA_out10"},  // 150
    {"I2S0I_DATA_in11",       "I2S0I_DATA_out11"},  // 151
    {"I2S0I_DATA_in12",       "I2S0I_DATA_out12"},  // 152
    {"I2S0I_DATA_in13",       "I2S0I_DATA_out13"},  // 153
    {"I2S0I_DATA_in14",       "I2S0I_DATA_out14"},  // 154
    {"I2S0I_DATA_in15",       "I2S0I_DATA_out15"},  // 155
    {"-",                     "I2S0I_DATA_out16"},  // 156
    {"-",                     "I2S0I_DATA_out17"},  // 157
    {"-",                     "I2S0I_DATA_out18"},  // 158
    {"-",                     "I2S0I_DATA_out19"},  // 159
    {"-",                     "I2S0I_DATA_out20"},  // 160
    {"-",                     "I2S0I_DATA_out21"},  // 161
    {"-",                     "I2S0I_DATA_out22"},  // 162
    {"-",                     "I2S0I_DATA_out13"},  // 163
    {"I2S1I_BCK_in",          "I2S1I_BCK_out"},     // 164
    {"I2S1I_WS_in",           "I2S1I_WS_outt"},     // 165
    {"I2S1I_DATA_in0",        "I2S1I_DATA_out0"},   // 166
    {"I2S1I_DATA_in1",        "I2S1I_DATA_out1"},   // 167
    {"I2S1I_DATA_in2",        "I2S1I_DATA_out2"},   // 168
    {"I2S1I_DATA_in3",        "I2S1I_DATA_out3"},   // 169
    {"I2S1I_DATA_in4",        "I2S1I_DATA_out4"},   // 170
    {"I2S1I_DATA_in5",        "I2S1I_DATA_out5"},   // 171
    {"I2S1I_DATA_in6",        "I2S1I_DATA_out6"},   // 172
    {"I2S1I_DATA_in7",        "I2S1I_DATA_out7"},   // 173
    {"I2S1I_DATA_in8",        "I2S1I_DATA_out8"},   // 174
    {"I2S1I_DATA_in9",        "I2S1I_DATA_out9"},   // 175
    {"I2S1I_DATA_in10",       "I2S1I_DATA_out10"},  // 176
    {"I2S1I_DATA_in11",       "I2S1I_DATA_out11"},  // 177
    {"I2S1I_DATA_in12",       "I2S1I_DATA_out12"},  // 178
    {"I2S1I_DATA_in13",       "I2S1I_DATA_out13"},  // 179
    {"I2S1I_DATA_in14",       "I2S1I_DATA_out14"},  // 180
    {"I2S1I_DATA_in15",       "I2S1I_DATA_out15"},  // 181
    {"-",                     "I2S1I_DATA_out16"},  // 182
    {"-",                     "I2S1I_DATA_out17"},  // 183
    {"-",                     "I2S1I_DATA_out18"},  // 184
    {"-",                     "I2S1I_DATA_out19"},  // 185
    {"-",                     "I2S1I_DATA_out20"},  // 186
    {"-",                     "I2S1I_DATA_out21"},  // 187
    {"-",                     "I2S1I_DATA_out22"},  // 188
    {"-",                     "I2S1I_DATA_out23"},  // 189
    {"I2SOI_H_SYNC",          "pwm3_out1h"},        // 190
    {"I2SOI_v_SYNC",          "pwm3_out1l"},        // 191
    {"I2SOI_H_ENABLE",        "pwm3_out2h"},        // 192
    {"I2S1I_H_SYNC",          "pwm3_out2l"},        // 193
    {"I2S1I_V_SYNC",          "pwm3_out3h"},        // 194
    {"I2S1I_H_ENABLE",        "pwm3_out3l"},        // 195
    {"-",                     "pwm3_out4h"},        // 196
    {"-",                     "pwm3_out4l"},        // 197
    {"U2RXD_in",              "U2TXD_out"},         // 198
    {"U2CTS_in",              "U2RTS_out"},         // 199
    {"emac_mdc_i",            "emac_mdc_o"},        // 200
    {"emac_mdi_i",            "emac_mdo_o"},        // 201
    {"emac_crs_i",            "emac_crs_o"},        // 202
    {"emac_col_i",            "emac_col_o"},        // 203
    {"pcmfsync_in",           "bt_audio0_irq"},     // 204
    {"pcmclk_in",             "bt_audio1_irq"},     // 205
    {"pcmdin",                "bt_audio2_irq"},     // 206
    {"-",                     "ble_audio0_irq"},    // 207
    {"-",                     "ble_audio1_irq"},    // 208
    {"-",                     "ble_audio2_irq"},    // 209
    {"-",                     "pcmfsync_out"},      // 210
    {"-",                     "pcmclk_out"},        // 211
    {"-",                     "pcmdout"},           // 212
    {"-",                     "ble_audio_sync0_p"}, // 213
    {"-",                     "ble_audio_sync1_p"}, // 214
    {"-",                     "ble_audio_sync2_p"}, // 215
    {"-",                     "-"},                 // 216
    {"-",                     "-"},                 // 217
    {"-",                     "-"},                 // 218
    {"-",                     "-"},                 // 219
    {"-",                     "-"},                 // 220
    {"-",                     "-"},                 // 221
    {"-",                     "-"},                 // 222
    {"-",                     "-"},                 // 223
    {"-",                     "sig_in_func224"},    // 224
    {"-",                     "sig_in_func225"},    // 225
    {"-",                     "sig_in_func226"},    // 226
    {"-",                     "sig_in_func227"},    // 227
    {"-",                     "sig_in_func228"},    // 228
    {"-",                     "-"},                 // 229
    {"-",                     "-"},                 // 230
    {"-",                     "-"},                 // 231
    {"-",                     "-"},                 // 232
    {"-",                     "-"},                 // 233
    {"-",                     "-"},                 // 234
    {"-",                     "-"},                 // 235
    {"-",                     "-"},                 // 236
    {"-",                     "-"},                 // 237
    {"-",                     "-"},                 // 238
    {"-",                     "-"},                 // 239
    {"-",                     "-"},                 // 240
    {"-",                     "-"},                 // 241
    {"-",                     "-"},                 // 242
    {"-",                     "-"},                 // 243
    {"-",                     "-"},                 // 244
    {"-",                     "-"},                 // 245
    {"-",                     "-"},                 // 246
    {"-",                     "-"},                 // 247
    {"-",                     "-"},                 // 248
    {"-",                     "-"},                 // 249
    {"-",                     "-"},                 // 250
    {"-",                     "-"},                 // 251
    {"-",                     "-"},                 // 252
    {"-",                     "-"},                 // 253
    {"-",                     "-"},                 // 254
    {"-",                     "-"},                 // 255
};

//*********************************************************************
// Return the index into the I/O mux registers, return -1 if none
int r4aEsp32GpioGetIoMuxRegisterIndex(int portNumber)
{
    // Validate the port number
    if (portNumber >= R4A_GPIO_MAX_PORTS)
        return -1;

    // Lookup the I/O mux register index
    return r4aGpioPortToIoMuxIndex[portNumber];
}

//*********************************************************************
// Return the I/O mux register value, return -1
int r4aEsp32GpioGetIoMuxRegister(int portNumber)
{
    uint32_t * muxReg;
    int index;
    uint32_t regValue;

    // Validate the register index
    index = r4aEsp32GpioGetIoMuxRegisterIndex(portNumber);
    if (index < 0)
        return (uint32_t)-1;

    // Lookup the
    muxReg = &r4aIoMux->R4A_IO_MUX_GPIO36_REG;
    regValue = muxReg[index];
    return regValue;
}

//*********************************************************************
// Return the output function select value, return -1 if none
int r4aEsp32GpioGetOutputFuncSelect(int portNumber)
{
    uint32_t outputSelect;

    // Validate the port number
    if (portNumber >= R4A_GPIO_MAX_PORTS)
        return -1;

    // Get the function select value
    outputSelect = r4aGpioRegs->R4A_GPIO_FUNC_OUT_SEL_CFG_REG[portNumber];
    return outputSelect;
}

//*********************************************************************
// Display the GPIO registers
void r4aEsp32GpioDisplayRegisters(Print * display)
{
    // Display the GPIO registers
    display->println("GPIO Registers");
    display->printf("    0x%08lx: GPIO_OUT_REG\r\n", r4aGpioRegs->R4A_GPIO_OUT_REG);
    display->printf("    0x%08lx: GPIO_OUT_W1TS_REG\r\n", r4aGpioRegs->R4A_GPIO_OUT_W1TS_REG);
    display->printf("    0x%08lx: GPIO_OUT_W1TC_REG\r\n", r4aGpioRegs->R4A_GPIO_OUT_W1TC_REG);
    display->printf("    0x%08lx: GPIO_OUT1_REG\r\n", r4aGpioRegs->R4A_GPIO_OUT1_REG);
    display->printf("    0x%08lx: GPIO_OUT1_W1TS_REG\r\n", r4aGpioRegs->R4A_GPIO_OUT1_W1TS_REG);
    display->printf("    0x%08lx: GPIO_OUT1_W1TC_REG\r\n", r4aGpioRegs->R4A_GPIO_OUT1_W1TC_REG);
    display->printf("    0x%08lx: GPIO_ENABLE_REG\r\n", r4aGpioRegs->R4A_GPIO_ENABLE_REG);
    display->printf("    0x%08lx: GPIO_ENABLE_W1TS_REG\r\n", r4aGpioRegs->R4A_GPIO_ENABLE_W1TS_REG);
    display->printf("    0x%08lx: GPIO_ENABLE_W1TC_REG\r\n", r4aGpioRegs->R4A_GPIO_ENABLE_W1TC_REG);
    display->printf("    0x%08lx: GPIO_ENABLE1_REG\r\n", r4aGpioRegs->R4A_GPIO_ENABLE1_REG);
    display->printf("    0x%08lx: GPIO_ENABLE1_W1TS_REG\r\n", r4aGpioRegs->R4A_GPIO_ENABLE1_W1TS_REG);
    display->printf("    0x%08lx: GPIO_ENABLE1_W1TC_REG\r\n", r4aGpioRegs->R4A_GPIO_ENABLE1_W1TC_REG);
    display->printf("    0x%08lx: GPIO_STRAP_REG\r\n", r4aGpioRegs->R4A_GPIO_STRAP_REG);
    display->printf("    0x%08lx: GPIO_IN_REG\r\n", r4aGpioRegs->R4A_GPIO_IN_REG);
    display->printf("    0x%08lx: GPIO_IN1_REG\r\n", r4aGpioRegs->R4A_GPIO_IN1_REG);
    display->printf("    0x%08lx: GPIO_STATUS_REG\r\n", r4aGpioRegs->R4A_GPIO_STATUS_REG);
    display->printf("    0x%08lx: GPIO_STATUS_W1TS_REG\r\n", r4aGpioRegs->R4A_GPIO_STATUS_W1TS_REG);
    display->printf("    0x%08lx: GPIO_STATUS_W1TC_REG\r\n", r4aGpioRegs->R4A_GPIO_STATUS_W1TC_REG);
    display->printf("    0x%08lx: GPIO_STATUS1_REG\r\n", r4aGpioRegs->R4A_GPIO_STATUS1_REG);
    display->printf("    0x%08lx: GPIO_STATUS1_W1TS_REG\r\n", r4aGpioRegs->R4A_GPIO_STATUS1_W1TS_REG);
    display->printf("    0x%08lx: GPIO_STATUS1_W1TC_REG\r\n", r4aGpioRegs->R4A_GPIO_STATUS1_W1TC_REG);
    display->printf("    0x%08lx: GPIO_ACPU_INT_REG\r\n", r4aGpioRegs->R4A_GPIO_ACPU_INT_REG);
    display->printf("    0x%08lx: GPIO_ACPUNMI_INT_REG\r\n", r4aGpioRegs->R4A_GPIO_ACPUNMI_INT_REG);
    display->printf("    0x%08lx: GPIO_PCPU_INT_REG\r\n", r4aGpioRegs->R4A_GPIO_PCPU_INT_REG);
    display->printf("    0x%08lx: GPIO_PCPU_NMI_INT_REG\r\n", r4aGpioRegs->R4A_GPIO_PCPU_NMI_INT_REG);
    display->printf("    0x%08lx: GPIO_ACPU_INT1_REG\r\n", r4aGpioRegs->R4A_GPIO_ACPU_INT1_REG);
    display->printf("    0x%08lx: GPIO_ACPU_NMI_INT1_REG\r\n", r4aGpioRegs->R4A_GPIO_ACPU_NMI_INT1_REG);
    display->printf("    0x%08lx: GPIO_PCPU_INT1_REG\r\n", r4aGpioRegs->R4A_GPIO_PCPU_INT1_REG);
    display->printf("    0x%08lx: GPIO_PCPU_NMI_INT1_REG\r\n", r4aGpioRegs->R4A_GPIO_PCPU_NMI_INT1_REG);
    for (int i = 0; i < R4A_GPIO_MAX_PORTS; i++)
        display->printf("    0x%08lx: GPIO_PIN%d_REG\r\n", r4aGpioRegs->R4A_GPIO_PIN_REG[i], i);
    for (int i = 0; i < 256; i++)
        display->printf("    0x%08lx: GPIO_FUNC%d_IN_SEL_CFG_REG\r\n", r4aGpioRegs->R4A_GPIO_FUNC_IN_SEL_CFG_REG[i], i);
    for (int i = 0; i < R4A_GPIO_MAX_PORTS; i++)
        display->printf("    0x%08lx: GPIO_FUNC%d_OUT_SEL_CFG_REG\r\n", r4aGpioRegs->R4A_GPIO_FUNC_OUT_SEL_CFG_REG[i], i);
}

//*********************************************************************
// Decode the IO Mux registers
void r4aEsp32GpioDisplayIoMuxRegisters(int portNumber, uint32_t regValue, Print * display)
{
    const char * function;

    function = r4aIoMuxFunctionNames[portNumber][(regValue & IO_MUX_MCU_SEL) >> 12];
    display->printf(", %s", function);
    display->printf(", DRV: %ld", (regValue & IO_MUX_FUN_DRV) >> 10);
    if (regValue & IO_MUX_IN_IE) display->print(", IE");
    if (regValue & IO_MUX_FUN_WPU) display->print(", Pull-up");
    if (regValue & IO_MUX_FUN_WPD) display->print(", Pull-down");
    display->print(", Sleep Mode: ");
    display->printf(", DRV: %ld", (regValue & IO_MUX_MCU_DRV) >> 5);
    if (regValue & IO_MUX_MCU_IE) display->print(", IE");
    if (regValue & IO_MUX_MCU_WPU) display->print(", PU");
    if (regValue & IO_MUX_MCU_WPD) display->print(", PD");
    if (regValue & IO_MUX_SLP_SEL) display->print(", Sleep");
    if (regValue & IO_MUX_MCU_OE) display->print(", OE");
    display->println();
}

//*********************************************************************
// Display the IO MUX registers
void r4aEsp32GpioDisplayIoMuxRegisters(Print * display)
{
    int portNumber;
    uint32_t regValue;

    display->println("IO Mux Registers");
    display->printf("    0x%08lx: R4A_IO_MUX_PIN_CTRL\r\n", r4aIoMux->R4A_IO_MUX_PIN_CTRL);
    for (portNumber = 0; portNumber < R4A_GPIO_MAX_PORTS; portNumber++)
    {
        regValue = r4aEsp32GpioGetIoMuxRegister(portNumber);
        if (regValue != (uint32_t)-1)
        {
            display->printf("    0x%08lx: IO_MUX_GPIO%d_REG", regValue, portNumber);
            r4aEsp32GpioDisplayIoMuxRegisters(portNumber, regValue, display);
        }
    }
}

//*********************************************************************
// Enable or disable the pull-down resistor
bool r4aEsp32GpioPullDown(int gpioNumber, bool enable)
{
    uint32_t * ioMux;
    int muxIndex;

    // Verify that an IOMUX register exists for this port
    muxIndex = r4aGpioPortToIoMuxIndex[gpioNumber];
    if (muxIndex < 0)
        // This port does not have an IOMUX register
        return false;

    // Update the pull-down state for this port
    ioMux = (uint32_t *)r4aIoMux;
    if (enable)
        ioMux[muxIndex] |= IO_MUX_MCU_WPD;
    else
        ioMux[muxIndex] &= ~IO_MUX_MCU_WPD;
    return true;
}

//*********************************************************************
// Enable or disable the pullup resistor
bool r4aEsp32GpioPullUp(int gpioNumber, bool enable)
{
    uint32_t * ioMux;
    int muxIndex;

    // Verify that an IOMUX register exists for this port
    muxIndex = r4aGpioPortToIoMuxIndex[gpioNumber];
    if (muxIndex < 0)
        // This port does not have an IOMUX register
        return false;

    // Update the pullup state for this port
    ioMux = (uint32_t *)r4aIoMux;
    if (enable)
        ioMux[muxIndex] |= IO_MUX_MCU_WPU;
    else
        ioMux[muxIndex] &= ~IO_MUX_MCU_WPU;
    return true;
}

