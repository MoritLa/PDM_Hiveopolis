
#This is a template to build your own project with the e-puck2_main-processor folder as a library.
#Simply adapt the lines below to be able to compile

# Define project name here
PROJECT = first_epuck

#Define path to the e-puck2_main-processor folder
GLOBAL_PATH = ../e-puck2_main-processor
COM_PATH = ../com_protocol

#Source files to include
CSRC += ./main.c \
		

#Header folders to include
INCDIR += 

USE_OPT += -DCHIBIOS

#Add source of Comunication protocol
include $(COM_PATH)/com.mk

#Jump to the main Makefile
include $(GLOBAL_PATH)/Makefile