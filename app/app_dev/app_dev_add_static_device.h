/* !! DONT PUT HEADER FILE PROTECTIONS IN THIS FILE !! */

#include  "app_dev_api.h"

#ifndef APP_DT_I2S_DEV
	#error "APP_DT_I2S_DEV should be defined"
#endif

uint8_t app_dev_ioctl( void * const aHandle ,const uint8_t aIoctl_num
		, void * aIoctl_param1 , void * aIoctl_param2);
uint8_t app_dev_callback(void * const aHandle ,
		const uint8_t aCallback_num , void * aCallback_param1, void * aCallback_param2);

#include "src/app_dev.h"

EXTERN_DECLARATION_TO_STATIC_DEVICE_INST(APP_DT_I2S_DEV) ;
#define STATIC_DEV_DATA_STRUCT_TYPE		app_instance_t
#define STATIC_DEV_DATA_STRUCT						\
	{												\
		P_TO_STATIC_DEVICE_INST(APP_DT_I2S_DEV),	\
	}

#define	STATIC_DEV_IOCTL_FUNCTION		app_dev_ioctl
#define	STATIC_DEV_CALLBACK_FUNCTION	app_dev_callback
#include "add_static_dev.h"

#undef	APP_DT_I2S_DEV
