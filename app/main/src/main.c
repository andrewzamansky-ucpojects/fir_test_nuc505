/*

*/

#include "_project_typedefs.h"
#include "_project_defines.h"
#include "_project_func_declarations.h"

#include "assert.h"
#include "os_wrapper.h"

#include "clocks_api.h"
#include "included_modules.h"
#include "app_dev_api.h"

#include "gpio.h"
#include "heartbeat_api.h"

#define DEBUG
#include "PRINTF_api.h"


/*-----------------------------------------------------------*/

extern pdev_descriptor_t systick_dev;
extern pdev_descriptor_const u_boot_shell_dev;
extern pdev_descriptor_t heartbeat_dev;
extern pdev_descriptor_const heartbeat_gpio_dev;
extern pdev_descriptor_const app_dev;
extern pdev_descriptor_const uart0_wrap_dev;

/*-----------------------------------------------------------*/
#define CORE_CLOCK_RATE		96000000


void busy_delay(uint32_t mSec)
{

	DEV_IOCTL_1_PARAMS(heartbeat_dev , HEARTBEAT_API_BUSY_WAIT_mS , (void*) mSec);

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
int main( void )
{

	clocks_api_set_rate(CONFIG_DT_CORE_CLOCK  , CORE_CLOCK_RATE);

	os_set_heartbeat_dev(heartbeat_dev);
	os_set_tick_timer_dev(systick_dev);
	os_init();

	DEV_IOCTL_0_PARAMS(u_boot_shell_dev , IOCTL_DEVICE_START  );

	DEV_IOCTL_0_PARAMS(heartbeat_gpio_dev , IOCTL_DEVICE_START );

	DEV_IOCTL_0_PARAMS(app_dev , IOCTL_DEVICE_START );


	PRINTF_API_AddDebugOutput(uart0_wrap_dev);

    os_start();

	/* Will only get here if there was not enough heap space to create the
	idle task. */
	return 0;
}
