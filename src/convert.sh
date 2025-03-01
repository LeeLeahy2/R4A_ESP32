#!/bin/bash
#
#   convert.sh
#
#   Shell script to make batch edits to the copied SparkFun files:
#       R4A_WiFi.h
#       WiFi.cpp
#
#   This script is based upon commit number:
#       1329fe5b2e0e48031f42d6756c59ec161ffb97bf
#
#   Edits need to be reviewed when the commit number changes.  Deletes
#   may remove the wrong amount of data.
########################################################################

# Files
source ./rtk-copy.sh
text=./text.txt

########################################
# Convert from settings.h to R4A_WiFi.h
########################################

# Remove most of the file (3 lines before GNSS.h for a total of 1930 lines)
sed   -i   ':a;N;s/\n/&/3;Ta;/GNSS.h/!{P;D};:b;N;s/\n/&/1930;Tb;d'   $destH

# Delete the private line
sed   -i   '/private/,+1 d'   $destH

# Delete the private routines (4 lines before displayComponents for 132 lines)
sed   -i   ':a;N;s/\n/&/4;Ta;/displayComponents/!{P;D};:b;N;s/\n/&/132;Tb;d'   $destH

# Remove the last 5 lines
sed   -i   '$d'   $destH
sed   -i   '$d'   $destH
sed   -i   '$d'   $destH
sed   -i   '$d'   $destH
sed   -i   '$d'   $destH

# Add the file header
echo "/**********************************************************************" > $text
echo "  R4A_WiFi.h" >> $text
echo "" >> $text
echo "  Robots-For-All (R4A)" >> $text
echo "  Definitions and declarations for WiFi support" >> $text
echo "" >> $text
echo "  Modified from https://raw.githubusercontent.com/sparkfun/SparkFun_RTK_Everywhere_Firmware/refs/heads/main/Firmware/RTK_Everywhere/settings.h" >> $text
echo "**********************************************************************/" >> $text
echo "" >> $text
echo "#ifndef __R4A_WIFI_H__" >> $text
echo "#define __R4A_WIFI_H__" >> $text
echo "" >> $text
echo "#include \"R4A_ESP32.h\"" >> $text
cat   $destH   >>   $text
mv   $text   $destH

# Add text to the end of the file
sed   -i   -e '$a\\n#endif  // __R4A_WIFI_H__'   $destH

# Add the missing type
sed   -i   's/typedef uint32_t WIFI_ACTION_t;/typedef uint8_t WIFI_CHANNEL_t;\ntypedef uint32_t WIFI_ACTION_t;/'   $destH

# Convert from class to data structure
sed   -i   's/class RTK_WIFI/typedef struct _R4A_WIFI/'   $destH
sed   -i   '/_verbose/a } R4A_WIFI;'   $destH
sed   -i   's/Class to simplify WiFi handling/Structure containing WiFi private data/'   $destH

# Remove 4 spaces from the beginning of the lines
sed   -i   's/    //'   $destH

# Change the constructor
sed   -i   's/Constructor/Perform the WiFi initialization/'   $destH
sed   -i   's/RTK_WIFI(/wifiBegin(bool debug = false, /'   $destH

# Add the R4A_WIFI structure address as the first routine parameter
sed   -i   's/(/(R4A_WIFI * wifi, /'   $destH

# Fix routines with only one parameter
sed   -i   's/, )/)/'   $destH

########################################
# Convert from WiFi.ino to WiFi.cpp
########################################

# Remove the large comment at the beginning of the file
sed   -i   ':a;N;s/\n/&/1;Ta;/ESP-NOW bringup from example 4_9_ESP_NOW/!{P;D};:b;N;s/\n/&/189;Tb;d'   $destCpp

# Remove the NETWORK_SEQUENCES
sed   -i   ':a;N;s/\n/&/2;Ta;/wifiStartSequence/!{P;D};:b;N;s/\n/&/7;Tb;d'   $destCpp
sed   -i   ':a;N;s/\n/&/2;Ta;/wifiStopSequence/!{P;D};:b;N;s/\n/&/7;Tb;d'   $destCpp

# Convert from constructor to the begin routine
sed   -i   's/RTK_WIFI::RTK_WIFI(/RTK_WIFI::begin(bool debug, /'   $destCpp

# Add the R4A_WIFI structure address as the first routine parameter
sed   -i   '/RTK_WIFI::/s/(/(R4A_WIFI * wifi, /'   $destCpp

# Fix routines with only one parameter
sed   -i   's/, )/)/'   $destCpp

# Uppercase the first letter of the routine name
sed   -i   -e   's/\(RTK_WIFI::.\)/\U\1/'   $destCpp

# Replace RTK_WIFI:: with wifi
sed   -i   's/RTK_WIFI::/wifi/'   $destCpp

# Switch the debug variable
sed   -i   's/settings.debugWifiState/_debug/'   $destCpp

# Convert from class members to structure members
sed   -i   's/ _/ wifi->_/'   $destCpp

# Switch the print command
sed   -i   's/systemPrint/Serial.print/'   $destCpp

# Replace the SSID and password table reference
sed   -i   's/settings.wifiNetworks\[/*r4aWifiSsidPassword\[/'   $destCpp

########################################
# Compare the files
########################################

meld   $srcH   $destH
meld   $srcIno   $destCpp
