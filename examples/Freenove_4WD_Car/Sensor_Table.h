/**********************************************************************
  Sensor_Table.h

  Line sensor tables
**********************************************************************/

//****************************************
// PCF8574 Line Sensor Table
//****************************************

const char * const pcf8574SensorTable[8] =
{ //  L  C  R

    ".       .", // 0
    ".     |x|", // 1
    ".  |x|  .", // 2
    ".  |xxxx|", // 3
    "|x|     .", // 4
    "|x|   |x|", // 5
    "|xxxx|  .", // 6
    "|xxxxxxx|", // 7
};

//****************************************
// SX1509 Line Sensor Table
//****************************************

const char * const sx1509SensorTable[256] =
{ //  L  C  R

    ".                      .", // 0x00
    ".                    |x|", // 0x01
    ".                 |x|  .", // 0x02
    ".                 |xxxx|", // 0x03
    "               |x|     .", // 0x04
    ".              |x|   |x|", // 0x05
    ".              |xxxx|  .", // 0x06
    ".              |xxxxxxx|", // 0x07
    "            |x|        .", // 0x08
    ".           |x|      |x|", // 0x09
    ".           |x|   |x|  .", // 0x0a
    ".           |x|   |xxxx|", // 0x0b
    "            |xxxx|     .", // 0x0c
    ".           |xxxx|   |x|", // 0x0d
    ".           |xxxxxxx|  .", // 0x0e
    ".           |xxxxxxxxxx|", // 0x0f
    "         |x|           .", // 0x10
    ".        |x|         |x|", // 0x11
    ".        |x|      |x|  .", // 0x12
    ".        |x|      |xxxx|", // 0x13
    "         |x|   |x|     .", // 0x14
    ".        |x|   |x|   |x|", // 0x15
    ".        |x|   |xxxx|  .", // 0x16
    ".        |x|   |xxxxxxx|", // 0x17
    "         |xxxx|        .", // 0x18
    ".        |xxxx|      |x|", // 0x19
    ".        |xxxx|   |x|  .", // 0x1a
    ".        |xxxx|   |xxxx|", // 0x1b
    "         |xxxxxxx|     .", // 0x1c
    ".        |xxxxxxx|   |x|", // 0x1d
    ".        |xxxxxxxxxx|  .", // 0x1e
    ".        |xxxxxxxxxxxxx|", // 0x1f
    "      |x|              .", // 0x20
    ".     |x|            |x|", // 0x21
    ".     |x|         |x|  .", // 0x22
    ".     |x|         |xxxx|", // 0x23
    "      |x|      |x|     .", // 0x24
    ".     |x|      |x|   |x|", // 0x25
    ".     |x|      |xxxx|  .", // 0x26
    ".     |x|      |xxxxxxx|", // 0x27
    "      |x|   |x|        .", // 0x28
    ".     |x|   |x|      |x|", // 0x29
    ".     |x|   |x|   |x|  .", // 0x2a
    ".     |x|   |x|   |xxxx|", // 0x2b
    "      |x|   |xxxx|     .", // 0x2c
    ".     |x|   |xxxx|   |x|", // 0x2d
    ".     |x|   |xxxxxxx|  .", // 0x2e
    ".     |x|   |xxxxxxxxxx|", // 0x2f
    "      |xxxx|           .", // 0x30
    ".     |xxxx|         |x|", // 0x31
    ".     |xxxx|      |x|  .", // 0x32
    ".     |xxxx|      |xxxx|", // 0x33
    "      |xxxx|   |x|     .", // 0x34
    ".     |xxxx|   |x|   |x|", // 0x35
    ".     |xxxx|   |xxxx|  .", // 0x36
    ".     |xxxx|   |xxxxxxx|", // 0x37
    "      |xxxxxxx|        .", // 0x38
    ".     |xxxxxxx|      |x|", // 0x39
    ".     |xxxxxxx|   |x|  .", // 0x3a
    ".     |xxxxxxx|   |xxxx|", // 0x3b
    "      |xxxxxxxxxx|     .", // 0x3c
    ".     |xxxxxxxxxx|   |x|", // 0x3d
    ".     |xxxxxxxxxxxxx|  .", // 0x3e
    ".     |xxxxxxxxxxxxxxxx|", // 0x3f
    "   |x|                 .", // 0x40
    ".  |x|               |x|", // 0x41
    ".  |x|            |x|  .", // 0x42
    ".  |x|            |xxxx|", // 0x43
    "   |x|         |x|     .", // 0x44
    ".  |x|         |x|   |x|", // 0x45
    ".  |x|         |xxxx|  .", // 0x46
    ".  |x|         |xxxxxxx|", // 0x47
    "   |x|      |x|        .", // 0x48
    ".  |x|      |x|      |x|", // 0x49
    ".  |x|      |x|   |x|  .", // 0x4a
    ".  |x|      |x|   |xxxx|", // 0x4b
    "   |x|      |xxxx|     .", // 0x4c
    ".  |x|      |xxxx|   |x|", // 0x4d
    ".  |x|      |xxxxxxx|  .", // 0x4e
    ".  |x|      |xxxxxxxxxx|", // 0x4f
    "   |x|   |x|           .", // 0x50
    ".  |x|   |x|         |x|", // 0x51
    ".  |x|   |x|      |x|  .", // 0x52
    ".  |x|   |x|      |xxxx|", // 0x53
    "   |x|   |x|   |x|     .", // 0x54
    ".  |x|   |x|   |x|   |x|", // 0x55
    ".  |x|   |x|   |xxxx|  .", // 0x56
    ".  |x|   |x|   |xxxxxxx|", // 0x57
    "   |x|   |xxxx|        .", // 0x58
    ".  |x|   |xxxx|      |x|", // 0x59
    ".  |x|   |xxxx|   |x|  .", // 0x5a
    ".  |x|   |xxxx|   |xxxx|", // 0x5b
    "   |x|   |xxxxxxx|     .", // 0x5c
    ".  |x|   |xxxxxxx|   |x|", // 0x5d
    ".  |x|   |xxxxxxxxxx|  .", // 0x5e
    ".  |x|   |xxxxxxxxxxxxx|", // 0x5f
    "   |xxxx|              .", // 0x60
    ".  |xxxx|            |x|", // 0x61
    ".  |xxxx|         |x|  .", // 0x62
    ".  |xxxx|         |xxxx|", // 0x63
    "   |xxxx|      |x|     .", // 0x64
    ".  |xxxx|      |x|   |x|", // 0x65
    ".  |xxxx|      |xxxx|  .", // 0x66
    ".  |xxxx|      |xxxxxxx|", // 0x67
    "   |xxxx|   |x|        .", // 0x68
    ".  |xxxx|   |x|      |x|", // 0x69
    ".  |xxxx|   |x|   |x|  .", // 0x6a
    ".  |xxxx|   |x|   |xxxx|", // 0x6b
    "   |xxxx|   |xxxx|     .", // 0x6c
    ".  |xxxx|   |xxxx|   |x|", // 0x6d
    ".  |xxxx|   |xxxxxxx|  .", // 0x6e
    ".  |xxxx|   |xxxxxxxxxx|", // 0x6f
    "   |xxxxxxx|           .", // 0x70
    ".  |xxxxxxx|         |x|", // 0x71
    ".  |xxxxxxx|      |x|  .", // 0x72
    ".  |xxxxxxx|      |xxxx|", // 0x73
    "   |xxxxxxx|   |x|     .", // 0x74
    ".  |xxxxxxx|   |x|   |x|", // 0x75
    ".  |xxxxxxx|   |xxxx|  .", // 0x76
    ".  |xxxxxxx|   |xxxxxxx|", // 0x77
    "   |xxxxxxxxxx|        .", // 0x78
    ".  |xxxxxxxxxx|      |x|", // 0x79
    ".  |xxxxxxxxxx|   |x|  .", // 0x7a
    ".  |xxxxxxxxxx|   |xxxx|", // 0x7b
    "   |xxxxxxxxxxxxx|     .", // 0x7c
    ".  |xxxxxxxxxxxxx|   |x|", // 0x7d
    ".  |xxxxxxxxxxxxxxxx|  .", // 0x7e
    ".  |xxxxxxxxxxxxxxxxxxx|", // 0x7f
    "|x|                    .", // 0x80
    "|x|                  |x|", // 0x81
    "|x|               |x|  .", // 0x82
    "|x|               |xxxx|", // 0x83
    "|x|            |x|     .", // 0x84
    "|x|            |x|   |x|", // 0x85
    ".|x|            |xxxx|  .", // 0x86
    ".|x|            |xxxxxxx|", // 0x87
    " |x|         |x|        .", // 0x88
    ".|x|         |x|      |x|", // 0x89
    ".|x|         |x|   |x|  .", // 0x8a
    ".|x|         |x|   |xxxx|", // 0x8b
    " |x|         |xxxx|     .", // 0x8c
    ".|x|         |xxxx|   |x|", // 0x8d
    ".|x|         |xxxxxxx|  .", // 0x8e
    ".|x|         |xxxxxxxxxx|", // 0x8f
    " |x|      |x|           .", // 0x90
    ".|x|      |x|         |x|", // 0x91
    ".|x|      |x|      |x|  .", // 0x92
    ".|x|      |x|      |xxxx|", // 0x93
    " |x|      |x|   |x|     .", // 0x94
    ".|x|      |x|   |x|   |x|", // 0x95
    ".|x|      |x|   |xxxx|  .", // 0x96
    ".|x|      |x|   |xxxxxxx|", // 0x97
    " |x|      |xxxx|        .", // 0x98
    ".|x|      |xxxx|      |x|", // 0x99
    ".|x|      |xxxx|   |x|  .", // 0x9a
    ".|x|      |xxxx|   |xxxx|", // 0x9b
    " |x|      |xxxxxxx|     .", // 0x9c
    ".|x|      |xxxxxxx|   |x|", // 0x9d
    ".|x|      |xxxxxxxxxx|  .", // 0x9e
    ".|x|      |xxxxxxxxxxxxx|", // 0x9f
    " |x|   |x|              .", // 0xa0
    ".|x|   |x|            |x|", // 0xa1
    ".|x|   |x|         |x|  .", // 0xa2
    ".|x|   |x|         |xxxx|", // 0xa3
    " |x|   |x|      |x|     .", // 0xa4
    ".|x|   |x|      |x|   |x|", // 0xa5
    ".|x|   |x|      |xxxx|  .", // 0xa6
    ".|x|   |x|      |xxxxxxx|", // 0xa7
    " |x|   |x|   |x|        .", // 0xa8
    ".|x|   |x|   |x|      |x|", // 0xa9
    ".|x|   |x|   |x|   |x|  .", // 0xaa
    ".|x|   |x|   |x|   |xxxx|", // 0xab
    " |x|   |x|   |xxxx|     .", // 0xac
    ".|x|   |x|   |xxxx|   |x|", // 0xad
    ".|x|   |x|   |xxxxxxx|  .", // 0xae
    ".|x|   |x|   |xxxxxxxxxx|", // 0xaf
    " |x|   |xxxx|           .", // 0xb0
    ".|x|   |xxxx|         |x|", // 0xb1
    ".|x|   |xxxx|      |x|  .", // 0xb2
    ".|x|   |xxxx|      |xxxx|", // 0xb3
    " |x|   |xxxx|   |x|     .", // 0xb4
    ".|x|   |xxxx|   |x|   |x|", // 0xb5
    ".|x|   |xxxx|   |xxxx|  .", // 0xb6
    ".|x|   |xxxx|   |xxxxxxx|", // 0xb7
    " |x|   |xxxxxxx|        .", // 0xb8
    ".|x|   |xxxxxxx|      |x|", // 0xb9
    ".|x|   |xxxxxxx|   |x|  .", // 0xba
    ".|x|   |xxxxxxx|   |xxxx|", // 0xbb
    " |x|   |xxxxxxxxxx|     .", // 0xbc
    ".|x|   |xxxxxxxxxx|   |x|", // 0xbd
    ".|x|   |xxxxxxxxxxxxx|  .", // 0xbe
    ".|x|   |xxxxxxxxxxxxxxxx|", // 0xbf
    " |xxxx|                 .", // 0xc0
    ".|xxxx|               |x|", // 0xc1
    ".|xxxx|            |x|  .", // 0xc2
    ".|xxxx|            |xxxx|", // 0xc3
    " |xxxx|         |x|     .", // 0xc4
    ".|xxxx|         |x|   |x|", // 0xc5
    ".|xxxx|         |xxxx|  .", // 0xc6
    ".|xxxx|         |xxxxxxx|", // 0xc7
    " |xxxx|      |x|        .", // 0xc8
    ".|xxxx|      |x|      |x|", // 0xc9
    ".|xxxx|      |x|   |x|  .", // 0xca
    ".|xxxx|      |x|   |xxxx|", // 0xcb
    " |xxxx|      |xxxx|     .", // 0xcc
    ".|xxxx|      |xxxx|   |x|", // 0xcd
    ".|xxxx|      |xxxxxxx|  .", // 0xce
    ".|xxxx|      |xxxxxxxxxx|", // 0xcf
    " |xxxx|   |x|           .", // 0xd0
    ".|xxxx|   |x|         |x|", // 0xd1
    ".|xxxx|   |x|      |x|  .", // 0xd2
    ".|xxxx|   |x|      |xxxx|", // 0xd3
    " |xxxx|   |x|   |x|     .", // 0xd4
    ".|xxxx|   |x|   |x|   |x|", // 0xd5
    ".|xxxx|   |x|   |xxxx|  .", // 0xd6
    ".|xxxx|   |x|   |xxxxxxx|", // 0xd7
    " |xxxx|   |xxxx|        .", // 0xd8
    ".|xxxx|   |xxxx|      |x|", // 0xd9
    ".|xxxx|   |xxxx|   |x|  .", // 0xda
    ".|xxxx|   |xxxx|   |xxxx|", // 0xdb
    " |xxxx|   |xxxxxxx|     .", // 0xdc
    ".|xxxx|   |xxxxxxx|   |x|", // 0xdd
    ".|xxxx|   |xxxxxxxxxx|  .", // 0xde
    ".|xxxx|   |xxxxxxxxxxxxx|", // 0xdf
    " |xxxxxxx|              .", // 0xe0
    ".|xxxxxxx|            |x|", // 0xe1
    ".|xxxxxxx|         |x|  .", // 0xe2
    ".|xxxxxxx|         |xxxx|", // 0xe3
    " |xxxxxxx|      |x|     .", // 0xe4
    ".|xxxxxxx|      |x|   |x|", // 0xe5
    ".|xxxxxxx|      |xxxx|  .", // 0xe6
    ".|xxxxxxx|      |xxxxxxx|", // 0xe7
    " |xxxxxxx|   |x|        .", // 0xe8
    ".|xxxxxxx|   |x|      |x|", // 0xe9
    ".|xxxxxxx|   |x|   |x|  .", // 0xea
    ".|xxxxxxx|   |x|   |xxxx|", // 0xeb
    " |xxxxxxx|   |xxxx|     .", // 0xec
    ".|xxxxxxx|   |xxxx|   |x|", // 0xed
    ".|xxxxxxx|   |xxxxxxx|  .", // 0xee
    ".|xxxxxxx|   |xxxxxxxxxx|", // 0xef
    " |xxxxxxxxxx|           .", // 0xf0
    ".|xxxxxxxxxx|         |x|", // 0xf1
    ".|xxxxxxxxxx|      |x|  .", // 0xf2
    ".|xxxxxxxxxx|      |xxxx|", // 0xf3
    " |xxxxxxxxxx|   |x|     .", // 0xf4
    ".|xxxxxxxxxx|   |x|   |x|", // 0xf5
    ".|xxxxxxxxxx|   |xxxx|  .", // 0xf6
    ".|xxxxxxxxxx|   |xxxxxxx|", // 0xf7
    " |xxxxxxxxxxxxx|        .", // 0xf8
    ".|xxxxxxxxxxxxx|      |x|", // 0xf9
    ".|xxxxxxxxxxxxx|   |x|  .", // 0xfa
    ".|xxxxxxxxxxxxx|   |xxxx|", // 0xfb
    " |xxxxxxxxxxxxxxxx|     .", // 0xfc
    ".|xxxxxxxxxxxxxxxx|   |x|", // 0xfd
    ".|xxxxxxxxxxxxxxxxxxx|  .", // 0xfe
    ".|xxxxxxxxxxxxxxxxxxxxxx|", // 0xff
};
