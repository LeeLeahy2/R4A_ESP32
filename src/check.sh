#!/bin/bash
#
# check.sh
#    Script to modify Freenove_4WD_Car.ino to enable various defines and
#    verify that the code still builds successfully.  This script stops
#    execution upon error when building Freenove_4WD_Car.ino.
########################################################################
set -e
#set -o verbose
#set -o xtrace

# Start fresh
git reset --hard --quiet HEAD

# Select the initial example directory
pushd   ../examples/00_Validate_Tables

# Validate tables
make clean
make
make clean

# Basic line following
cd   ../01_Basic_Line_Following
make clean
make
make clean

# Bluetooth output
cd ../02_Bluetooth_Output
make clean
make
make clean

# Telnet output
cd ../03_Telnet_Output
make clean
make
make clean

# State machine
cd ../04_State_Machine
make clean
make
make clean

# Telnet menu
cd ../05_Telnet_Menu
make clean
make
make clean

# Bluetooth Output
cd ../06_Bluetooth_Output
make clean
make
make clean

# Alpha-numeric display
cd ../07_AlphaNumeric_Display
make clean
make
make clean
sed 's|#define FLIP_X_FLIP_Y           0|#define FLIP_X_FLIP_Y           1|'  07_AlphaNumeric_Display.ino  >  temp.txt
sed 's|#define USE_SPARKFUN_THING_PLUS_ESP32_WROOM     0|#define USE_SPARKFUN_THING_PLUS_ESP32_WROOM     1|'  temp.txt  >  07_AlphaNumeric_Display.ino
sed 's|#ESP32_CHIP=esp32|ESP32_CHIP=esp32|' makefile  >  temp.txt
sed 's|ESP32_CHIP=esp32wrover|#ESP32_CHIP=esp32wrover|'  temp.txt  >  makefile
rm  temp.txt
make clean
make
make clean

# LED matrix display
cd ../08_LED_Matrix_Display
make clean
make
make clean
sed 's|#define FLIP_X_FLIP_Y           0|#define FLIP_X_FLIP_Y           1|'  08_LED_Matrix_Display.ino   >   temp.txt
sed 's|#define USE_SPARKFUN_THING_PLUS_ESP32_WROOM     0|#define USE_SPARKFUN_THING_PLUS_ESP32_WROOM     1|'  temp.txt  >  08_LED_Matrix_Display.ino
sed 's|#ESP32_CHIP=esp32|ESP32_CHIP=esp32|' makefile  >  temp.txt
sed 's|ESP32_CHIP=esp32wrover|#ESP32_CHIP=esp32wrover|'  temp.txt > makefile
rm  temp.txt
make clean
make
make clean

# WS2812 LEDs
cd ../09_WS2812
make clean
make
make clean

# Select the example directory
cd   ../Freenove_4WD_Car

# Start fresh
git reset --hard --quiet HEAD
make clean
make

# NTRIP
cp  Freenove_4WD_Car.ino  temp.txt
sed 's|//#define USE_NTRIP|#define USE_NTRIP|'  temp.txt  >  Freenove_4WD_Car.ino
rm  temp.txt
make
git reset --hard --quiet HEAD

# OV2640 Camera
cp  Freenove_4WD_Car.ino  temp.txt
sed 's|//#define USE_OV2640|#define USE_OV2640|'  temp.txt  >  Freenove_4WD_Car.ino
rm  temp.txt
make
git reset --hard --quiet HEAD

# SparkFun SEN-13582
cp  Freenove_4WD_Car.ino  temp.txt
sed 's|//#define USE_SPARKFUN_SEN_13582|#define USE_SPARKFUN_SEN_13582|'  temp.txt  >  Freenove_4WD_Car.ino
rm  temp.txt
make
git reset --hard --quiet HEAD

# ZED F9P
cp  Freenove_4WD_Car.ino  temp.txt
sed 's|//#define USE_ZED_F9P|#define USE_ZED_F9P|'  temp.txt  >  Freenove_4WD_Car.ino
rm  temp.txt
make
git reset --hard --quiet HEAD

# Waypoint Following (everything)
sed 's|//#define USE_NTRIP|#define USE_NTRIP|'  Freenove_4WD_Car.ino  >  temp.txt
sed 's|//#define USE_OV2640|#define USE_OV2640|'  temp.txt  >  Freenove_4WD_Car.ino
sed 's|//#define USE_SPARKFUN_SEN_13582|#define USE_SPARKFUN_SEN_13582|'  Freenove_4WD_Car.ino  >  temp.txt
sed 's|//#define USE_WAYPOINT_FOLLOWING|#define USE_WAYPOINT_FOLLOWING|'  temp.txt  >  Freenove_4WD_Car.ino
sed 's|//#define USE_ZED_F9P|#define USE_ZED_F9P|'  Freenove_4WD_Car.ino  >  temp.txt
cp -f temp.txt  Freenove_4WD_Car.ino
rm  temp.txt
make
git reset --hard --quiet  HEAD
make clean

# Restore the source directory
popd
