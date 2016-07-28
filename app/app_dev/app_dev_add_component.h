/* !! DONT PUT HEADER FILE PROTECTIONS IN THIS FILE !! */

#include  "app_dev_api.h"
#include "src/app_dev.h"

uint8_t app_dev_ioctl( pdev_descriptor_t apdev ,const uint8_t aIoctl_num
		, void * aIoctl_param1 , void * aIoctl_param2);
uint8_t app_dev_callback(pdev_descriptor_t apdev ,
		const uint8_t aCallback_num , void * aCallback_param1, void * aCallback_param2);

#define	MODULE_NAME					app_dev
#define	MODULE_IOCTL_FUNCTION		app_dev_ioctl
#define	MODULE_CALLBACK_FUNCTION	app_dev_callback
#define MODULE_DATA_STRUCT_TYPE		app_instance_t

#ifdef DT_DEV_MODULE

	#ifndef APP_DT_I2S_DEV
		#error "APP_DT_I2S_DEV should be defined"
	#endif

	EXTERN_DECLARATION_TO_STATIC_DEVICE_INST(APP_DT_I2S_DEV) ;
	#define STATIC_DEV_DATA_STRUCT						\
		{												\
			P_TO_STATIC_DEVICE_INST(APP_DT_I2S_DEV),	\
		}

#endif

#include "add_component.h"

#undef	APP_DT_I2S_DEV
