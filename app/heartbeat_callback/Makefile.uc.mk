INCLUDE_THIS_COMPONENT := y


#DEFINES =

#CFLAGS =

#ASMFLAGS =




SRC = heartbeat_callback.c

SPEED_CRITICAL_FILES +=  heartbeat_callback.c

VPATH = src


include $(COMMON_CC)
