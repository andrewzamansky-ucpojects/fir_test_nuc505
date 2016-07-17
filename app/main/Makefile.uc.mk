

INCLUDE_THIS_COMPONENT := y

#DEFINES +=

#CFLAGS +=

#ASMFLAGS +=



SRC = main.c
SRC += device_tree.c

SPEED_CRITICAL_FILES += app_routines.c

VPATH = src

include $(COMMON_CC)