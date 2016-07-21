/* !! DONT PUT HEADER FILE PROTECTIONS IN THIS FILE !! */

#include  "heartbeat_callback_api.h"

#ifndef HEARTBEAT_CALLBACK_HEARTBEAT_PDEV
	#error "HEARTBEAT_CALLBACK_HEARTBEAT_PDEV should be defined"
#endif

#ifndef HEARTBEAT_CALLBACK_BLINKING_GPIO_PDEV
	#error "HEARTBEAT_CALLBACK_BLINKING_GPIO_PDEV should be defined"
#endif

uint8_t heartbeat_callback_ioctl( void * const aHandle ,const uint8_t aIoctl_num
		, void * aIoctl_param1 , void * aIoctl_param2);
uint8_t heartbeat_callback_callback(void * const aHandle ,
		const uint8_t aCallback_num , void * aCallback_param1, void * aCallback_param2);

#include "src/heartbeat_callback.h"

EXTERN_DECLARATION_TO_STATIC_DEVICE_INST(HEARTBEAT_CALLBACK_HEARTBEAT_PDEV) ;
EXTERN_DECLARATION_TO_STATIC_DEVICE_INST(HEARTBEAT_CALLBACK_BLINKING_GPIO_PDEV) ;
#define STATIC_DEV_DATA_STRUCT_TYPE		heartbeat_callback_instance_t
#define STATIC_DEV_DATA_STRUCT						\
	{												\
		P_TO_STATIC_DEVICE_INST(HEARTBEAT_CALLBACK_HEARTBEAT_PDEV),	\
		P_TO_STATIC_DEVICE_INST(HEARTBEAT_CALLBACK_BLINKING_GPIO_PDEV)	\
	}

#define	STATIC_DEV_IOCTL_FUNCTION		heartbeat_callback_ioctl
#define	STATIC_DEV_CALLBACK_FUNCTION	heartbeat_callback_callback
#include "add_static_dev.h"


#undef HEARTBEAT_CALLBACK_HEARTBEAT_PDEV
#undef HEARTBEAT_CALLBACK_BLINKING_GPIO_PDEV
