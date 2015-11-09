

INCLUDE_THIS_COMPONENT := YES   

GLOBAL_INCLUDE_DIR += $(EXTERNAL_SOURCE_ROOT_DIR)/BSP_NUC505_v3.00.003/Library/Device/Nuvoton/NUC505Series/Include
GLOBAL_INCLUDE_DIR +=$(EXTERNAL_SOURCE_ROOT_DIR)/BSP_NUC505_v3.00.003/Library/StdDriver/inc
GLOBAL_INCLUDE_DIR += $(EXTERNAL_SOURCE_ROOT_DIR)/ARM-CMSIS/CMSIS/Include

INCLUDE_DIR =  $(SW_PACKAGES_ROOT_DIR)/u_boot_shell/include
INCLUDE_DIR += $(EXTERNAL_SOURCE_ROOT_DIR)/u-boot/include

#DEFINES +=

#CFLAGS += 

#ASMFLAGS +=  



SRC = main.c 
SRC += init.c 
SRC += cmd_change_volume.c
SRC += cmd_heap_test.c
SRC += app_routines.c

SPEED_CRITICAL_FILES += app_routines.c

VPATH = src

include $(COMMON_CC)

