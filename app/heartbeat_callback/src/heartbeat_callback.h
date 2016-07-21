/*
 * file : heartbeat_callback.h
 *
 *
 *
 *
 *
 *
 */

#ifndef _heartbeat_callback_H
#define _heartbeat_callback_H

#include "_project.h"
#include "dev_management_api.h" // for device manager defines and typedefs


/***************   typedefs    *******************/


typedef struct
{
	pdev_descriptor_t   heartbeat_dev;
	pdev_descriptor_t   heartbeat_blinking_gpio_dev;
} heartbeat_callback_instance_t;



#endif /* */
