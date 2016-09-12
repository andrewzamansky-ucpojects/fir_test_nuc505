/*

*/

#include "_project_typedefs.h"
#include "_project_defines.h"
#include "_project_func_declarations.h"

#include "assert.h"
#include "os_wrapper.h"

#include "heartbeat_api.h"

#define DEBUG
#include "PRINTF_api.h"


/*-----------------------------------------------------------*/

static pdev_descriptor_t l_heartbeat_dev;

pdev_descriptor_t debug_io0_dev, debug_io1_dev;

/*-----------------------------------------------------------*/


void busy_delay(uint32_t mSec)
{

	DEV_IOCTL_1_PARAMS(l_heartbeat_dev , HEARTBEAT_API_BUSY_WAIT_mS ,  mSec);

}



/*-----------------------------------------------------------*/
int main( void )
{
	pdev_descriptor_t dev;
	dev = DEV_OPEN("soc_clock_control_dev");
	if (NULL == dev) goto error;
	DEV_IOCTL_0_PARAMS(dev , IOCTL_DEVICE_START  );

	l_heartbeat_dev = DEV_OPEN("heartbeat_dev");
	if (NULL == l_heartbeat_dev) goto error;
	os_set_heartbeat_dev(l_heartbeat_dev);

	dev = DEV_OPEN("systick_dev");
	if (NULL == dev) goto error;
	os_set_tick_timer_dev(dev);
	os_init();

	dev = DEV_OPEN("u_boot_shell_dev");
	if (NULL == dev) goto error;
	DEV_IOCTL_0_PARAMS(dev , IOCTL_DEVICE_START  );

	dev = DEV_OPEN("heartbeat_callback_dev");
	if (NULL == dev) goto error;
	DEV_IOCTL_0_PARAMS(dev , IOCTL_DEVICE_START );

	dev = DEV_OPEN("app_dev");
	if (NULL == dev) goto error;
	DEV_IOCTL_0_PARAMS(dev , IOCTL_DEVICE_START );

	dev = DEV_OPEN("uart0_tx_wrap_dev");
	if (NULL == dev) goto error;
	PRINTF_API_AddDebugOutput(dev);


	debug_io0_dev = DEV_OPEN("debug_gpio0_dev");
	if (NULL == debug_io0_dev) goto error;
	DEV_IOCTL_0_PARAMS(debug_io0_dev , IOCTL_DEVICE_START );


	debug_io1_dev = DEV_OPEN("debug_gpio1_dev");
	if (NULL == debug_io1_dev) goto error;
	DEV_IOCTL_0_PARAMS(debug_io1_dev , IOCTL_DEVICE_START );

    os_start();

error :
	while (1);
	/* Will only get here if there was not enough heap space to create the
	idle task. */
	return 0;
}
