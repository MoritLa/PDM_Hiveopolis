
#This is a template to build your own project with the e-puck2_main-processor folder as a library.
#Simply adapt the lines below to be able to compile

# Define project name here
PROJECT = first_epuck

#Define path to the e-puck2_main-processor folder
GLOBAL_PATH = ../e-puck2_main-processor

#Source files to include
CSRC += ./main.c \
		./source/com_api.c \
		./source/com_CAN_heartbeat.c \
		./source/com_CAN_input.c \
		./source/com_CAN_output.c \
		./source/com_input_buffer.c \
		./source/com_main.c \
		./source/com_osal.c \
		./source/com_output_buffer.c \
		

#Header folders to include
INCDIR += ./header

#Jump to the main Makefile
include $(GLOBAL_PATH)/Makefile