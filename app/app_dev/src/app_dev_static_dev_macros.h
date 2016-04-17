
#ifndef _app_dev_static_dev_macros_h_
#define _app_dev_static_dev_macros_h_

#include "_project.h"
#include "dev_managment_api.h" // for device manager defines and typedefs

uint8_t app_dev_ioctl( void * const aHandle ,const uint8_t aIoctl_num
		, void * aIoctl_param1 , void * aIoctl_param2);
uint8_t app_dev_callback(void * const aHandle ,
		const uint8_t aCallback_num , void * aCallback_param1, void * aCallback_param2);


#define __APP_API_CREATE_STATIC_DEV(dev,dev_name  )\
		extern const dev_descriptor_t dev ;						\
		const dev_descriptor_t dev =								\
			{											\
				dev_name,								\
				NULL,									\
				app_dev_ioctl,							\
				DEV_API_dummy_pwrite_func,				\
				DEV_API_dummy_pread_func,				\
				app_dev_callback						\
			}


#endif
