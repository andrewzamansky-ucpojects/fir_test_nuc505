

/* Standard includes. */
#include "_project_typedefs.h"
#include "_project_defines.h"
#include "_project_func_declarations.h"

#include "clocks_api.h"
#include "included_modules.h"

#define DEBUG
#include "PRINTF_api.h"


/*-----------------------------------------------------------*/

extern pdev_descriptor_t systick_dev;
extern pdev_descriptor_const heartbeat_dev;
extern pdev_descriptor_const u_boot_shell_dev;
extern pdev_descriptor_const heartbeat_gpio_dev;
extern pdev_descriptor_const app_dev;
extern pdev_descriptor_const uart0_wrap_dev;




/**** end ofsetup of static devices and device management ****/
/************************************************************/

#define CORE_CLOCK_RATE		96000000
void app_tick_callback(void);
void app_idle_hook(void);



void app_routines_init(void)
{
	os_set_tick_timer_dev(systick_dev);
	os_set_tick_callback(app_tick_callback);
	os_set_idle_entrance_callback(app_idle_hook);
}

/* function : prvSetupHardware
 *
 *
 *
 */
void init( void )
{
	clocks_api_set_rate(CONFIG_DT_CORE_CLOCK  , CORE_CLOCK_RATE);


	DEV_IOCTL_0_PARAMS(heartbeat_dev , IOCTL_DEVICE_START );

	DEV_IOCTL_0_PARAMS(u_boot_shell_dev , IOCTL_DEVICE_START  );

	DEV_IOCTL_0_PARAMS(heartbeat_gpio_dev , IOCTL_DEVICE_START );

	DEV_IOCTL_0_PARAMS(app_dev , IOCTL_DEVICE_START );



	app_routines_init();

	PRINTF_API_AddDebugOutput(uart0_wrap_dev);


}


/*-----------------------------------------------------------*/
