/*
 * file : shell_config.h
 *
 *
 *
 *
 *
 *
 */

#ifndef _shell_config_H
#define _shell_config_H

#include "_project_typedefs.h"
#include "_project_defines.h"
#include "_project_func_declarations.h"

#include "PRINTF_api.h"

#include "os_wrapper.h"

#define SHELL_CONFIG_USE_AS_DYNAMIC_INSTANCE 	0

#define SHELL_TASK_PRIORITY				(tskIDLE_PRIORITY + 2)
#define SHELL_TASK_STACK_SIZE			DEFINE_STACK_SIZE( 1150 )
#define SHELL_MAX_QUEUE_LEN				( 3 )

#define SHELL_CONFIG_MAX_RX_BUFFER_SIZE	(1<<16)

extern int run_command(const char *cmd, int flag);


#endif /* */
