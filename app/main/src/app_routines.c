

/* Standard includes. */
#include "_project_typedefs.h"
#include "_project_defines.h"
#include "_project_func_declarations.h"

//#include "assert.h"

#include "dev_managment_api.h"

#include "included_modules.h"


#include "app_dev_api.h"

#include "gpio.h"
#include "heartbeat_api.h"

#define DEBUG
#include "PRINTF_api.h"
/*-----------------------------------------------------------*/



/*-----------------------------------------------------------*/

extern pdev_descriptor_const heartbeat_dev ;
extern pdev_descriptor_const heartbeat_gpio_dev ;




void busy_delay(uint32_t mSec)
{

	DEV_IOCTL_1_PARAMS(heartbeat_dev , HEARTBEAT_API_BUSY_WAIT_mS , (void*) mSec);

}

void app_tick_callback(void)
{
	DEV_IOCTL_0_PARAMS(heartbeat_dev , HEARTBEAT_API_EACH_1mS_CALL );
}




void app_idle_hook()
{
	DEV_IOCTL_0_PARAMS(heartbeat_dev , HEARTBEAT_API_CALL_FROM_IDLE_TASK );
}

void heartbeat_callback(void)
{
	static uint8_t tick=0;
	if(0 == tick)
	{
		DEV_IOCTL_0_PARAMS(heartbeat_gpio_dev , IOCTL_GPIO_PIN_CLEAR );
	}
	else
	{
		DEV_IOCTL_0_PARAMS(heartbeat_gpio_dev , IOCTL_GPIO_PIN_SET );
	}

	//PC3_DOUT = tick;
	tick = 1 - tick;

	// !!!! DONT USE PRINTF_DBG . IT CAN PUT IDLE TASK TO WAIT STATE . THIS IS WRONG !!
	// REMOVE THESE LINE AS SOON AS POSSIBLE
#if 0
	{
		uint8_t cpu_usage_int_part,cpu_usage_res_part;
		uint32_t cpu_usage;
		DEV_IOCTL_1_PARAMS(heartbeat_dev , HEARTBEAT_API_GET_CPU_USAGE , &cpu_usage );
		cpu_usage_int_part = cpu_usage / 1000;
		cpu_usage_res_part = cpu_usage - cpu_usage_int_part;
		PRINTF_DBG("cpu usage = %d.%03d%% \n", cpu_usage_int_part , cpu_usage_res_part);
	}
#endif

}



/*-----------------------------------------------------------*/
