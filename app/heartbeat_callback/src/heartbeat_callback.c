/*
 *
 * file :   heartbeat.c
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


/********  defines *********************/


#define HEARTBEAT_CONFIG_MAX_QUEUE_LEN					( 1 )

/********  types  *********************/

/********  externals *********************/



/********  local defs *********************/



/********  types  *********************/


typedef struct
{

	uint8_t dummy;
} xMessage_t;


static os_queue_t xQueue=NULL ;


extern pdev_descriptor_const heartbeat_dev ;
extern pdev_descriptor_const heartbeat_gpio_dev ;



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
uint8_t heartbeat_callback_callback(void * const aHandle ,
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
static void heartbeat_thread_func (void * param)
{
	xMessage_t xRxMessage;
	static uint8_t tick=0;
	xQueue = os_create_queue( HEARTBEAT_CONFIG_MAX_QUEUE_LEN , sizeof(xMessage_t ) );


	while (1)
	{
		if( OS_QUEUE_RECEIVE_SUCCESS == os_queue_receive_infinite_wait( xQueue , &( xRxMessage )  ) )
		{

			if(0 == (tick & 0x1))
			{
				DEV_IOCTL_0_PARAMS(heartbeat_gpio_dev , IOCTL_GPIO_PIN_CLEAR );
			}
			else
			{
				DEV_IOCTL_0_PARAMS(heartbeat_gpio_dev , IOCTL_GPIO_PIN_SET );
			}
			tick++;
			if (2 == tick)
			{
				tick = 0;
				uint8_t cpu_usage_int_part,cpu_usage_res_part;
				uint32_t cpu_usage;
//				uint32_t limiter_hits;
				DEV_IOCTL_1_PARAMS(heartbeat_dev , HEARTBEAT_API_GET_CPU_USAGE , &cpu_usage );
				cpu_usage_int_part = cpu_usage / 1000;
				cpu_usage_res_part = cpu_usage - cpu_usage_int_part;
				PRINTF_DBG("cpu usage = %d.%03d%% \n", cpu_usage_int_part , cpu_usage_res_part);
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
uint8_t heartbeat_callback_ioctl( void * const aHandle ,const uint8_t aIoctl_num
		, void * aIoctl_param1 , void * aIoctl_param2)
{

	switch(aIoctl_num)
	{
		case IOCTL_DEVICE_START :



			os_create_task("heartbeat" , heartbeat_thread_func, 0, HEARTBEAT_STACK_SIZE_BYTES , HEARTBEAT_THREAD_PRIORITY);

			DEV_IOCTL_0_PARAMS(heartbeat_gpio_dev , IOCTL_DEVICE_START );

			break;
		default :
			return 1;
	}
	return 0;
}