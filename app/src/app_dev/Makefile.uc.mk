INCLUDE_THIS_COMPONENT := YES 

INCLUDE_DIR =  $(SW_PACKAGES_ROOT_DIR)/u_boot_shell/include
INCLUDE_DIR += $(EXTERNAL_SOURCE_ROOT_DIR)/u-boot/include

#DEFINES = 

#CFLAGS = 

#ASMFLAGS =  



SRC = app_dev.c
SRC += cmd_set_compressor.c
SRC += cmd_set_cutoff.c
SRC += cmd_set_vb.c
SRC += cmd_set_voice_3d.c
SRC += cmd_ctl.c

SPEED_CRITICAL_FILES += app_dev.c

VPATH = src


include $(COMMON_CC)
