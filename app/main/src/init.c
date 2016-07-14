

/* Standard includes. */
#include "_project_typedefs.h"
#include "_project_defines.h"
#include "_project_func_declarations.h"

#include "clocks_api.h"
#include "included_modules.h"

#define DEBUG
#include "PRINTF_api.h"
#include "gpio.h"


/*-----------------------------------------------------------*/



#define	RX_BUFF_SIZE	256
uint8_t rx_buff[RX_BUFF_SIZE];

#define CORE_CLOCK_RATE		96000000
#define OS_TICK_IN_MICRO_SEC		1000
void heartbeat_callback(void);
void app_tick_callback(void);
void app_idle_hook(void);

/***********************************/
/********** systick_dev ********/
#define CURRENT_DEV		cortexM_systick
#include INIT_CURRENT_DEV()

#define CORTEXM_SYSTICK_DT_DEV_NAME			systick_dev
#define CORTEXM_SYSTICK_DT_INITIAL_RATE		OS_TICK_IN_MICRO_SEC
#define CORTEXM_SYSTICK_DT_MODE				TIMER_API_PERIODIC_MODE
#define CORTEXM_SYSTICK_DT_CALLBACK			NULL

#include ADD_CURRENT_DEV()



/***********************************/
/********** heartbeat_dev ********/
#define CURRENT_DEV		heartbeat
#include INIT_CURRENT_DEV()

#define HEARTBEAT_DT_DEV_NAME			heartbeat_dev
#define HEARTBEAT_DT_CALLBACK_FUNC		heartbeat_callback
#define HEARTBEAT_DT_OS_TIMER_PDEV		systick_dev

#include ADD_CURRENT_DEV()

//extern dev_descriptor_t inst_uart0_wrap_dev;

/***********************************/
/********** uart0_dev ********/
#define CURRENT_DEV		uart_nuc505
#include INIT_CURRENT_DEV()

#define UART_NUC505_DT_DEV_NAME			uart0_dev
#define UART_NUC505_DT_UART_NUMBER		UART_NUC505_API_UART_ID_0
#define UART_NUC505_DT_CALLBACK_PDEV	uart0_wrap_dev
#define UART_NUC505_DT_BAUD_RATE		115200

#include ADD_CURRENT_DEV()


/***********************************/
/********** uart0_wrap_dev ********/
#define CURRENT_DEV		sw_uart_wrapper
#include INIT_CURRENT_DEV()

#define SW_UART_WRAPPER_DT_DEV_NAME			uart0_wrap_dev
#define SW_UART_WRAPPER_DT_SERVER_PDEV		uart0_dev
#define SW_UART_WRAPPER_DT_CLIENT_PDEV		shell_dev
#define SW_UART_WRAPPER_DT_RX_BUFFER		rx_buff
#define SW_UART_WRAPPER_DT_RX_BUFFER_SIZE	RX_BUFF_SIZE

#include ADD_CURRENT_DEV()


/***********************************/
/********** shell_dev ********/
#define CURRENT_DEV		shell
#include INIT_CURRENT_DEV()

#define SHELL_DT_DEV_NAME		shell_dev
#define SHELL_DT_SERVER_PDEV	uart0_wrap_dev
#define SHELL_DT_CALLBACK_PDEV	u_boot_shell_dev

#include ADD_CURRENT_DEV()


/***********************************/
/********** u_boot_shell_dev ********/
#define CURRENT_DEV		u_boot_shell
#include INIT_CURRENT_DEV()

#define U_BOOT_SHELL_DT_DEV_NAME		u_boot_shell_dev
#define U_BOOT_SHELL_DT_SERVER_PDEV		shell_dev

#include ADD_CURRENT_DEV()


/***********************************/
/********** app_dev ********/
#define CURRENT_DEV		app_dev
#include INIT_CURRENT_DEV()

#define APP_DEV_DT_DEV_NAME		app_dev

#include ADD_CURRENT_DEV()


/***********************************/
/********** i2s_dev ********/
#define CURRENT_DEV		I2S_nuc505
#include INIT_CURRENT_DEV()

#define I2S_NUC505_DT_DEV_NAME					i2s_dev
#define I2S_NUC505_DT_NUM_OF_WORDS_IN_BUFFER	I2S_BUFF_LEN
#define I2S_NUC505_DT_NUM_OF_BYTES_IN_WORD		NUM_OF_BYTES_PER_AUDIO_WORD
#define I2S_NUC505_DT_CALLBACK_PDEV				app_dev

#include ADD_CURRENT_DEV()

/***********************************/
/********** heartbeat_gpio_dev ********/
#define CURRENT_DEV		gpio_nuc505
#include INIT_CURRENT_DEV()

#define GPIO_NUC505_DT_DEV_NAME		heartbeat_gpio_dev
#define GPIO_NUC505_DT_PORT			GPIO_NUC505_API_PORT_C
#define GPIO_NUC505_DT_PIN			0x00000008
#define GPIO_NUC505_DT_MODE			GPIO_NUC505_API_MODE_OUT_PP

#include ADD_CURRENT_DEV()



/**** end ofsetup of static devices and device management ****/
/************************************************************/



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

	DEV_IOCTL_0_PARAMS(uart0_dev , IOCTL_DEVICE_START );

	DEV_IOCTL_0_PARAMS(uart0_wrap_dev , IOCTL_DEVICE_START );

	DEV_IOCTL_0_PARAMS(shell_dev , IOCTL_DEVICE_START );

	DEV_IOCTL_0_PARAMS(u_boot_shell_dev , IOCTL_DEVICE_START  );

	DEV_IOCTL_0_PARAMS(heartbeat_gpio_dev , IOCTL_DEVICE_START );

	DEV_IOCTL_0_PARAMS(app_dev , IOCTL_DEVICE_START );


	DEV_IOCTL_0_PARAMS(i2s_dev , IOCTL_DEVICE_START );

	app_routines_init();

	PRINTF_API_AddDebugOutput(uart0_wrap_dev);


}


/*-----------------------------------------------------------*/
