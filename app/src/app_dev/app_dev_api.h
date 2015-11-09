
#ifndef _app_dev_API_H_
#define _app_dev_API_H_

#include "app_dev_config.h"
#include "dev_managment_api.h" // for device manager defines and typedefs

/*****************  defines  **************/


/**********  define API  types ************/



/**********  define API  functions  ************/

typedef enum
{
	IOCTL_APP_DEV_SET_SERIAL_NUMBER = IOCTL_LAST_COMMON_IOCTL+1
}APP_DEV_API_ioctl_t;

#include "src/app_dev_static_dev_macros.h"

#define APP_API_CREATE_STATIC_DEV(dev,dev_name   ) \
			__APP_API_CREATE_STATIC_DEV(dev,dev_name   )

#endif
