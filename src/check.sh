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
pushd   ../examples/01_Basic_Line_Following

# Basic line following
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
sed -i 's|#define USE_SPARKFUN_THING_PLUS_ESP32_WROOM     0|#define USE_SPARKFUN_THING_PLUS_ESP32_WROOM     1|' 07_AlphaNumeric_Display.ino
sed -i 's|#ESP32_CHIP=esp32|ESP32_CHIP=esp32|' makefile
sed -i 's|ESP32_CHIP=esp32wrover|#ESP32_CHIP=esp32wrover|' makefile
make clean
make
make clean

# LED matrix display
cd ../08_LED_Matrix_Display
make clean
make
make clean
sed -i 's|#define USE_SPARKFUN_THING_PLUS_ESP32_WROOM     0|#define USE_SPARKFUN_THING_PLUS_ESP32_WROOM     1|' 08_LED_Matrix_Display.ino
sed -i 's|#ESP32_CHIP=esp32|ESP32_CHIP=esp32|' makefile
sed -i 's|ESP32_CHIP=esp32wrover|#ESP32_CHIP=esp32wrover|' makefile
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
sed -i 's|//#define USE_NTRIP|#define USE_NTRIP|' Freenove_4WD_Car.ino
make
git reset --hard --quiet HEAD

# OV2640 Camera
#sed -i 's|//#define USE_OV2640|#define USE_OV2640|' Freenove_4WD_Car.ino
#make
#git reset --hard --quiet HEAD

# ZED F9P
sed -i 's|//#define USE_ZED_F9P|#define USE_ZED_F9P|' Freenove_4WD_Car.ino
make
git reset --hard --quiet HEAD

# Waypoint Following (everything)
sed -i 's|//#define USE_NTRIP|#define USE_NTRIP|' Freenove_4WD_Car.ino
#sed -i 's|//#define USE_OV2640|#define USE_OV2640|' Freenove_4WD_Car.ino
sed -i 's|//#define USE_WAYPOINT_FOLLOWING|#define USE_WAYPOINT_FOLLOWING|' Freenove_4WD_Car.ino
sed -i 's|//#define USE_ZED_F9P|#define USE_ZED_F9P|' Freenove_4WD_Car.ino
make
git reset --hard --quiet  HEAD
make clean

# Restore the source directory
popd
