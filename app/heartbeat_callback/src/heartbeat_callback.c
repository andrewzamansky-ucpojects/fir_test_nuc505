/*
 *
 * file :   heartbeat_callback.c
 *
 *
 *
 *
 *
 */



/********  includes *********************/
#include "_project.h"
#include "dev_management_api.h" // for device manager defines and typedefs

#include "heartbeat_api.h"

#include "heartbeat_callback.h"

#define DEBUG
#include "PRINTF_api.h"

#include "os_wrapper.h"
#include "gpio_api.h"

#include "heartbeat_callback_add_component.h"
#include "I2S_mixer_api.h"


/********  defines *********************/

#define INSTANCE(hndl)	((heartbeat_callback_instance_t*)hndl)

#define HEARTBEAT_CONFIG_MAX_QUEUE_LEN					( 1 )

/********  types  *********************/

/********  externals *********************/

extern dsp_descriptor_t app_I2S_mixer;

/********  local defs *********************/



/********  types  *********************/


typedef struct
{

	uint8_t dummy;
} xMessage_t;


static os_queue_t xQueue=NULL ;

/* global to control report interval - controlled by cmd_set_cpu_stat_interval */
uint8_t cpu_stat_report_interval = 1;

/*---------------------------------------------------------------------------------------------------------*/
/* Function:		heartbeat_callback																		  */
/*																										 */
/* Parameters:																							 */
/*																						 */
/*																								  */
/* Returns:																					  */
/* Side effects:																						   */
/* Description:																							*/
/*																					 */
/*---------------------------------------------------------------------------------------------------------*/
uint8_t heartbeat_callback_callback(pdev_descriptor_t apdev ,
		const uint8_t aCallback_num , void * aCallback_param1, void * aCallback_param2)
{
	xMessage_t  queueMsg;

	if(NULL == xQueue) return 1;

	os_queue_send_immediate( xQueue, ( void * ) &queueMsg);


	return 0;
}


/**
 * \b main_thread_func
 *
 * Entry point for main application thread.
 *
 * This is the first thread that will be executed when the OS is started.
 *
 * @param[in] param Unused (optional thread entry parameter)
 *
 * @return None
 */
static void heartbeat_thread_func (void * apdev)
{
	xMessage_t xRxMessage;
	heartbeat_callback_instance_t *config_handle;
	pdev_descriptor_t l_heartbeat_blinking_gpio_dev ;
	pdev_descriptor_t l_heartbeat_dev ;
	float max_out_val;

	static uint8_t tick=0;
	xQueue = os_create_queue( HEARTBEAT_CONFIG_MAX_QUEUE_LEN , sizeof(xMessage_t ) );

	config_handle = DEV_GET_CONFIG_DATA_POINTER(apdev);
	l_heartbeat_blinking_gpio_dev = config_handle->heartbeat_blinking_gpio_dev;
	l_heartbeat_dev = config_handle->heartbeat_dev ;

	DSP_IOCTL_0_PARAMS(&app_I2S_mixer , IOCTL_I2S_MIXER_ENABLE_TEST_CLIPPING );

	while (1)
	{
		if( OS_QUEUE_RECEIVE_SUCCESS == os_queue_receive_infinite_wait( xQueue , &( xRxMessage )  ) )
		{

			if(0 == (tick & 0x1))
			{
				DEV_IOCTL_0_PARAMS(l_heartbeat_blinking_gpio_dev , IOCTL_GPIO_PIN_CLEAR );
			}
			else
			{
				DEV_IOCTL_0_PARAMS(l_heartbeat_blinking_gpio_dev , IOCTL_GPIO_PIN_SET );
			}
			tick++;
			if ((cpu_stat_report_interval != 0) && (cpu_stat_report_interval < tick))
			{
				tick = 0;
				uint8_t cpu_usage_int_part,cpu_usage_res_part;
				uint32_t cpu_usage;
//				uint32_t limiter_hits;
				DEV_IOCTL_1_PARAMS(l_heartbeat_dev , HEARTBEAT_API_GET_CPU_USAGE , &cpu_usage );
				cpu_usage_int_part = cpu_usage / 1000;
				cpu_usage_res_part = cpu_usage - cpu_usage_int_part;
				PRINTF_DBG("cpu usage = %d.%03d%% \n\r", cpu_usage_int_part , cpu_usage_res_part);

				DSP_IOCTL_1_PARAMS(&app_I2S_mixer , IOCTL_I2S_MIXER_GET_MAX_OUTPUT_VALUE , &max_out_val );
				PRINTF_DBG("max_out = %f\n\r",max_out_val);
//				DEV_IOCTL(&compressor_limiter, IOCTL_COMPRESSOR_GET_HIT_COUNTER ,&limiter_hits);
//				PRINTF_DBG("limiter = %d  \n", limiter_hits );
			}
		}

		os_stack_test();

	}
}




/*---------------------------------------------------------------------------------------------------------*/
/* Function:		heartbeat_ioctl																		  */
/*																										 */
/* Parameters:																							 */
/*																						 */
/*																								  */
/* Returns:																					  */
/* Side effects:																						   */
/* Description:																							*/
/*																					 */
/*---------------------------------------------------------------------------------------------------------*/
uint8_t heartbeat_callback_ioctl( pdev_descriptor_t apdev ,const uint8_t aIoctl_num
		, void * aIoctl_param1 , void * aIoctl_param2)
{
	heartbeat_callback_instance_t *config_handle;

	config_handle = DEV_GET_CONFIG_DATA_POINTER(apdev);
	switch(aIoctl_num)
	{
		case IOCTL_DEVICE_START :



			os_create_task("heartbeat" , heartbeat_thread_func, apdev, HEARTBEAT_STACK_SIZE_BYTES , HEARTBEAT_THREAD_PRIORITY);

			DEV_IOCTL_0_PARAMS(config_handle->heartbeat_blinking_gpio_dev , IOCTL_DEVICE_START );

			break;
		default :
			return 1;
	}
	return 0;
}
