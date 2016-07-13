

/* Standard includes. */
#include "_project_typedefs.h"
#include "_project_defines.h"
#include "_project_func_declarations.h"

//#include "assert.h"

#include "dev_managment_api.h"
#include "clocks_api.h"


#include "included_modules.h"



#define DEBUG
#include "PRINTF_api.h"

#include "app_dev_api.h"

#include "gpio.h"

#include "cortexM_systick_api.h"
#include "heartbeat_api.h"
#include "u_boot_shell_api.h"


/*-----------------------------------------------------------*/



#define	RX_BUFF_SIZE	256
uint8_t rx_buff[RX_BUFF_SIZE];

#define CORE_CLOCK_RATE		96000000
#define OS_TICK_IN_MICRO_SEC		1000
void heartbeat_callback(void);
void app_tick_callback(void);
void app_idle_hook(void);


#define CURRENT_DEV()		cortexM_systick
#include INIT_CURRENT_DEV()

#define CORTEXM_SYSTICK_DT_DEV_NAME			systick_dev
#define CORTEXM_SYSTICK_DT_INITIAL_RATE		OS_TICK_IN_MICRO_SEC
#define CORTEXM_SYSTICK_DT_MODE				TIMER_API_PERIODIC_MODE
#define CORTEXM_SYSTICK_DT_CALLBACK			NULL

#include ADD_CURRENT_DEV()


//CORTEXM_SYSTICK_API_CREATE_STATIC_DEV(systick_dev ,
//		OS_TICK_IN_MICRO_SEC , TIMER_API_PERIODIC_MODE , NULL );

HEARTBEAT_API_CREATE_STATIC_DEV(heartbeat_dev ,heartbeat_callback , systick_dev);

UART_NUC505_API_CREATE_STATIC_DEV(uart0_dev , UART_NUC505_API_UART_ID_0 , uart0_wrap_dev , 115200);

SW_UART_WRAPPER_API_CREATE_STATIC_DEV(uart0_wrap_dev , uart0_dev ,shell_dev , rx_buff,RX_BUFF_SIZE);

SHELL_API_CREATE_STATIC_DEV(shell_dev , uart0_wrap_dev , u_boot_shell_dev);

U_BOOT_SHELL_API_CREATE_STATIC_DEV(u_boot_shell_dev , shell_dev );

APP_API_CREATE_STATIC_DEV(app_dev);

I2S_NUC505_API_CREATE_STATIC_DEV(i2s_dev , app_dev);

GPIO_NUC505_API_CREATE_STATIC_DEV(heartbeat_gpio_dev , GPIO_NUC505_API_PORT_C ,
		0x00000008 ,GPIO_NUC505_API_MODE_OUT_PP);



/**** end ofsetup of static devices and device managment ****/
/************************************************************/

//
//inline void init_i2s()
//{
//    I2S_API_set_params_t I2S_API_set_params;
//
//	I2S_API_set_params.num_of_words_in_buffer_per_chenel = I2S_BUFF_LEN;
//	I2S_API_set_params.num_of_bytes_in_word = NUM_OF_BYTES_PER_AUDIO_WORD;
//	DEV_IOCTL_1_PARAMS(i2s_dev , I2S_SET_PARAMS, &I2S_API_set_params);
//	DEV_IOCTL_0_PARAMS(i2s_dev , IOCTL_DEVICE_START );
//
//}

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


//	DEV_IOCTL_0_PARAMS(heartbeat_dev , IOCTL_DEVICE_START );
//
//	DEV_IOCTL_0_PARAMS(uart0_dev , IOCTL_DEVICE_START );
//
//	DEV_IOCTL_0_PARAMS(uart0_wrap_dev , IOCTL_DEVICE_START );
//
//	DEV_IOCTL_0_PARAMS(shell_dev , IOCTL_DEVICE_START );
//
//	DEV_IOCTL_0_PARAMS(u_boot_shell_dev , IOCTL_DEVICE_START  );
//
//	DEV_IOCTL_0_PARAMS(heartbeat_gpio_dev , IOCTL_DEVICE_START );
//
//	DEV_IOCTL_0_PARAMS(app_dev , IOCTL_DEVICE_START );


//	init_i2s();

	app_routines_init();

	PRINTF_API_AddDebugOutput(uart0_wrap_dev);


}


/*-----------------------------------------------------------*/
