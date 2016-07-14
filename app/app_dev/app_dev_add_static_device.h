#ifndef _add_static_device_step1
#define _add_static_device_step1

#include  "app_dev_api.h"

#undef _add_static_device_step2

#undef APP_DEV_DT_DEV_NAME


#elif !defined(_add_static_device_step2)
#define _add_static_device_step2

#undef _add_static_device_step1

#ifndef APP_DEV_DT_DEV_NAME
#error "APP_DEV_DT_DEV_NAME should be defined"
#endif


uint8_t app_dev_ioctl( void * const aHandle ,const uint8_t aIoctl_num
		, void * aIoctl_param1 , void * aIoctl_param2);
uint8_t app_dev_callback(void * const aHandle ,
		const uint8_t aCallback_num , void * aCallback_param1, void * aCallback_param2);


CREATE_STATIC_DEVICE(APP_DEV_DT_DEV_NAME , NULL , app_dev_ioctl ,
				DEV_API_dummy_pwrite_func , DEV_API_dummy_pread_func , app_dev_callback);



#undef CURRENT_DEV

#endif
