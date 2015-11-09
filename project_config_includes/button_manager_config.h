/*
 * file : sw_uart_wrapper_config.h
 *
 *
 *
 *
 *
 *
 */

#ifndef _sw_uart_wrapper_config_H
#define _sw_uart_wrapper_config_H

#include "_project_typedefs.h"
#include "_project_defines.h"
#include "_project_func_declarations.h"

#include "PRINTF_api.h"

#include "os_wrapper.h"

#define BUTTON_MANAGER_CONFIG_NUM_OF_DYNAMIC_BUTTONS_GROUPS  	1
#define BUTTON_MANAGER_CONFIG_TASK_PRIORITY				(TASK_NORMAL_PRIORITY+1)
#define BUTTON_MANAGER_CONFIG_TASK_STACK_SIZE				DEFINE_STACK_SIZE( 160 )

#endif /* */
