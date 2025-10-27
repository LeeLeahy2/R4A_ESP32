######################################################################
# makefile
#
# Robots-For-All (R4A)
# Build the SPI NOR Flash client support applications
######################################################################

.ONESHELL:
SHELL=/bin/bash

##########
# Source files
##########

EXECUTABLES =  SPI_Flash_Read
EXECUTABLES += SPI_Flash_Write

INCLUDES  = Dump_Buffer.h
INCLUDES += SPI_Flash_Protocol.h

##########
# Buid all the sources - must be first
##########

.PHONY: all

all: $(EXECUTABLES)

SPI_Flash_Read:  SPI_Flash_Read.c   Dump_Buffer.c   makefile   $(INCLUDES)
	gcc   -o $@   $<   Dump_Buffer.c

SPI_Flash_Write:  SPI_Flash_Write.c   Dump_Buffer.c   makefile   $(INCLUDES)
	gcc   -o $@   $<   Dump_Buffer.c

########
# Clean the build directory
##########

.PHONY: clean

clean:
	rm   $(EXECUTABLES)
