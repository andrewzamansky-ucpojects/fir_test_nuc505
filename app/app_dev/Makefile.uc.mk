INCLUDE_THIS_COMPONENT := y


#DEFINES =

#CFLAGS =

#ASMFLAGS =




SRC = app_dev.c
SRC += cmd_ctl.c
SRC += cmd_set_i2s_loopback.c

SPEED_CRITICAL_FILES += app_dev.c

VPATH = src


include $(COMMON_CC)
