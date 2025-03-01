#!/bin/bash
#
#   rtk-copy.sh
#
#   Shell script to copy SparkFun WiFi.ino into local WiFi.ino and
#   settings.h to R4A_WiFi.h
########################################################################

# Files
source ./files.sh

########################################
# Copy the files
########################################

# Header file
cp   $srcH     $destH

# INO file to CPP file
cp   $srcIno   $destCpp
