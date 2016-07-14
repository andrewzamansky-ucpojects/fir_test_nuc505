
#ifndef _app_dev_API_H_
#define _app_dev_API_H_

#include "_project.h"
#include "dev_management_api.h" // for device manager defines and typedefs

/*****************  defines  **************/


/**********  define API  types ************/



/**********  define API  functions  ************/

typedef enum
{
	IOCTL_APP_DEV_SET_SERIAL_NUMBER = IOCTL_LAST_COMMON_IOCTL+1
}APP_DEV_API_ioctl_t;


#endif
