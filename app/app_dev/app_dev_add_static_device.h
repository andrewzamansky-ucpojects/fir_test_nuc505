/* !! DONT PUT HEADER FILE PROTECTIONS IN THIS FILE !! */

#include  "app_dev_api.h"


uint8_t app_dev_ioctl( void * const aHandle ,const uint8_t aIoctl_num
		, void * aIoctl_param1 , void * aIoctl_param2);
uint8_t app_dev_callback(void * const aHandle ,
		const uint8_t aCallback_num , void * aCallback_param1, void * aCallback_param2);


#define	STATIC_DEV_IOCTL_FUNCTION		app_dev_ioctl
#define	STATIC_DEV_CALLBACK_FUNCTION	app_dev_callback
#include "add_static_dev.h"
