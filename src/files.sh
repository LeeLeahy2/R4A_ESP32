#!/bin/bash
#
#   files.sh
#
#   Shell script to copy SparkFun files
#       settings.h --> R4A_Wifi.h
#       WiFi.ino ----> WiFi.cpp
########################################################################

# Files
srcH=~/SparkFun/rc/Everywhere/Firmware/RTK_Everywhere/settings.h
srcIno=~/SparkFun/rc/Everywhere/Firmware/RTK_Everywhere/WiFi.ino

destCpp=WiFi.cpp
destH=R4A_WiFi.h
