## imported variable $(APP_ROOT_DIR) is location of main
## makefile of current project
## 

PROJECT_NAME := audio_nuc505

###################################
#####   global flags :  ###########
GLOBAL_PROJECT_SPECIFIC_CFLAGS +=  
#GLOBAL_PROJECT_SPECIFIC_CFLAGS += -fstack-usage -fcallgraph-info
GLOBAL_PROJECT_SPECIFIC_ASMLAGS += 
GLOBAL_PROJECT_SPECIFIC_LDFLAGS += 




###################################
#####   global defines :  #########
GLOBAL_DEFINES += __EVAL __MICROLIB


###################################
#####   include components :  #####  

CONFIG_SOC_TYPE = nuc505#{nuc505,stm32f10x,stm8,atmega328,poleg}
CONFIG_INCLUDE_DEV_MENAGMENT = YES
CONFIG_INCLUDE_VERSION_MENAGMENT = YES
CONFIG_INCLUDE_HEARTBEAT = YES AS_SPEED_CRITICAL
CONFIG_INCLUDE_PRINTF = YES
CONFIG_INCLUDE_OS = freeRtos AS_SPEED_CRITICAL#{freeRtos,atomthreads} 
##CONFIG_INCLUDE_INTERNAL_FLASH = YES
#CONFIG_INCLUDE_HTTP = YES
#CONFIG_INCLUDE_ESP8266 = YES
CONFIG_INCLUDE_SW_UART_WRAPPER = YES
CONFIG_INCLUDE_INTERNAL_UART = YES
CONFIG_INCLUDE_INTERNAL_I2S = YES
#CONFIG_INCLUDE_UART_USB_VIRTUAL_COM = YES
#CONFIG_INCLUDE_CONFIG = YES
#CONFIG_INCLUDE_JSMN = YES
CONFIG_INCLUDE_UBOOT_SHELL = YES
#CONFIG_INCLUDE_USB_VIRTUAL_COM = YES
#CONFIG_INCLUDE_USB_MASS_STORAGE = YES
#CONFIG_INCLUDE_SW_GPIO_WRAPPER = YES
CONFIG_INCLUDE_INTERNAL_GPIO = YES
#CONFIG_INCLUDE_INTERNAL_SERIAL_NUMBER = YES
CONFIG_INCLUDE_INTERNAL_CLOCK_CONTROL = YES
#CONFIG_INCLUDE_FAT_FS = YES
CONFIG_INCLUDE_CORTEXM_SYSTICK = YES
CONFIG_INCLUDE_SHELL = YES
CONFIG_INCLUDE_NVIC = YES
#CONFIG_INCLUDE_LM35 = YES
#CONFIG_INCLUDE_INTERNAL_ADC = YES
#CONFIG_INCLUDE_INTERNAL_FLASH = YES
#CONFIG_INCLUDE_INTERNAL_CRC = YES
#CONFIG_INCLUDE_GPIO_REMOTE = YES
#CONFIG_INCLUDE_WIRELESS_UART = YES
#CONFIG_INCLUDE_BUTTON_MANAGER =YES
#CONFIG_INCLUDE_INTERNAL_SPI = YES
#CONFIG_INCLUDE_SPI_FLASH = YES
#CONFIG_INCLUDE_SPI_FLASH_PARTITION_MANAGER = YES
CONFIG_INCLUDE_EQUALIZER = YES AS_SPEED_CRITICAL
CONFIG_INCLUDE_MIXER = YES AS_SPEED_CRITICAL
CONFIG_INCLUDE_I2S_SPLITTER = YES AS_SPEED_CRITICAL
CONFIG_INCLUDE_I2S_MIXER = YES AS_SPEED_CRITICAL
CONFIG_INCLUDE_COMPRESSOR = YES AS_SPEED_CRITICAL
CONFIG_INCLUDE_MEMORY_POOL = YES AS_SPEED_CRITICAL
#CONFIG_INCLUDE_FFMPEG = YES
CONFIG_INCLUDE_TIMER =YES


CONFIG_INCLUDE_STD_LIBRARIES = libc.a libm.a libc_nano.a libgcc.a
#CONFIG_CRITICAL_SPEED_STD_LIBRARIES = libc.a libm.a libc_nano.a libgcc.a
CONFIG_CRITICAL_SPEED_STD_LIBRARIES = ibm.a libc_nano.a libgcc.a



######################################
#####   misc configs  #####  

#CONFIG_OUTPUT_NAME 						= xxx#if not used then name is 'out'
CONFIG_CPU_TYPE  						= cortex-m4#{cortex-a9,arm926ej-s,cortex-m3,stm8}
CONFIG_CPU_SPECIFIC_FILES_ARE_SPEAD_CRITICAL = YES
#CONFIG_USE_NANO_STD_LIBS				= YES
CONFIG_INCLUDE_FPU						= YES
CONFIG_USE_COMPILER 					= ARM-NONE-EABI-GCC#{GCC-HOST,GCC-AVR,ARM-NONE-EABI-GCC,ARMCC,CXSTM8}
CONFIG_GCC_COMPILER_VERSION				= 4.9.3#{4.9.2,4.9.3}
#CONFIG_USE_STANDARD_STARTUP			= YES
CONFIG_RAM_START_ADDR  					= 0x20000000
CONFIG_RAM_SIZE							= 128k#add K for kilobytes or M for megabytes
CONFIG_FLASH_START_ADDR 				= 0x0
CONFIG_FLASH_SIZE						= 256k#add K for kilobytes or M for megabytes
CONFIG_CODE_LOCATION					= flash#{ram,flash}
CONFIG_PUT_STD_LIBRARIES_IN_RAM			= YES
CONFIG_USED_FOR_SEMIHOSTING_UPLOADING 	= YES
CONFIG_OPTIMIZE_LEVEL 					= Og#{O0,O1,O2,O3,Os}
#CONFIG_POSITION_INDEPENDENT 			= YES // not yet implemented
CONFIG_TEST_TASK_STACK					= YES
CONFIG_CALCULATE_CRC32 					= YES
