/*
 * file : app_dev_config.h
 *
 *
 *
 *
 *
 *
 */

#ifndef _app_dev_config_H
#define _app_dev_config_H

#include "_project_typedefs.h"
#include "_project_defines.h"
#include "_project_func_declarations.h"

#define APP_DEV_CONFIG_MAX_QUEUE_LEN	2
#define APP_DEV_THREAD_PRIORITY				(tskIDLE_PRIORITY+4)
#define MAIN_STACK_SIZE_BYTES			DEFINE_STACK_SIZE( 360 )


#endif /* */
