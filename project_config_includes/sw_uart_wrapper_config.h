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

#define SW_UART_WRAPPER_CONFIG_USE_MALLOC			 		1
#define SW_UART_WRAPPER_CONFIG_NUM_OF_DYNAMIC_INSTANCES 	4
#define SW_UART_WRAPPER_CONFIG_MAX_RX_BUFFER_SIZE 			((1<<16)-1)
#define SW_UART_WRAPPER_CONFIG_MAX_TX_BUFFER_SIZE 			(255)

#define SW_UART_WRAPPER_CONFIG_TASK_PRIORITY				(tskIDLE_PRIORITY+2)
#define SW_UART_WRAPPER_CONFIG_TASK_STACK_SIZE			DEFINE_STACK_SIZE( 460 )
#define SW_UART_WRAPPER_CONFIG_MAX_QUEUE_LEN			( 10 )

extern pdev_descriptor_const systick_dev;
//#define busy_delay(mSec) DEV_IOCTL_1_PARAMS(systick_dev , IOCTL_TIMER_BUSY_WAIT_mS ,  mSec)


#endif /* */
