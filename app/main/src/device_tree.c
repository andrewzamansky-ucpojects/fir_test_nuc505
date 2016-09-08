#include "_project.h"

#include "dev_management_api.h"


/***********************************/
/********** soc_clock_control_dev ********/
#define DT_DEV_NAME							soc_clock_control_dev
#define DT_DEV_MODULE						clock_control_nuc505

#define CLOCK_CONTROL_NUC505_DT_XTAL_RATE		CONFIG_CRYSTAL_CLOCK
#define CLOCK_CONTROL_NUC505_DT_CORE_RATE		96000000

#include ADD_CURRENT_DEV



/***********************************/
/********** systick_dev ********/
#define DT_DEV_NAME							systick_dev
#define DT_DEV_MODULE						cortexM_systick

#define	CORTEXM_SYSTICK_DT_CLOCK_PDEV		soc_clock_control_dev
#define	CORTEXM_SYSTICK_DT_CLOCK_INDEX		NUC505_CORE_CLOCK
#define CORTEXM_SYSTICK_DT_INITIAL_RATE		OS_TICK_IN_MICRO_SEC
#define CORTEXM_SYSTICK_DT_MODE				TIMER_API_PERIODIC_MODE

#include ADD_CURRENT_DEV



/***********************************/
/********** heartbeat_callback_dev ********/
#define DT_DEV_NAME							heartbeat_callback_dev
#define DT_DEV_MODULE						heartbeat_callback

#define HEARTBEAT_CALLBACK_HEARTBEAT_PDEV		heartbeat_dev
#define HEARTBEAT_CALLBACK_BLINKING_GPIO_PDEV	heartbeat_gpio_dev

#include ADD_CURRENT_DEV

/***********************************/
/********** heartbeat_dev ********/
#define DT_DEV_NAME							heartbeat_dev
#define DT_DEV_MODULE						heartbeat

#define HEARTBEAT_DT_CALLBACK_PDEV			heartbeat_callback_dev
#define HEARTBEAT_DT_OS_TIMER_PDEV			systick_dev

#include ADD_CURRENT_DEV



/***********************************/
/********** uart0_dev ********/
#define DT_DEV_NAME							uart0_dev
#define DT_DEV_MODULE						uart_nuc505

#define UART_NUC505_DT_BASE_ADDRESS			UART_NUC505_API_BASE_ADDRESS_UART0
#define UART_NUC505_DT_TX_CALLBACK_PDEV		uart0_tx_wrap_dev
#define UART_NUC505_DT_RX_CALLBACK_PDEV		uart0_rx_wrap_dev
#define UART_NUC505_DT_BAUD_RATE			115200

#include ADD_CURRENT_DEV


/***********************************/
/********** uart0_tx_wrap_dev ********/
#define DT_DEV_NAME							uart0_tx_wrap_dev
#define DT_DEV_MODULE						async_tx_wrapper

#define ASYNC_TX_WRAPPER_DT_SERVER_PDEV		uart0_dev

#include ADD_CURRENT_DEV

/***********************************/
/********** uart0_rx_wrap_dev ********/
#define DT_DEV_NAME							uart0_rx_wrap_dev
#define DT_DEV_MODULE						async_rx_wrapper

#define ASYNC_RX_WRAPPER_DT_SERVER_PDEV		uart0_dev
#define ASYNC_RX_WRAPPER_DT_CLIENT_PDEV		shell_dev
#define ASYNC_RX_WRAPPER_DT_RX_BUFFER_SIZE	255

#include ADD_CURRENT_DEV


/***********************************/
/********** shell_dev ********/
#define DT_DEV_NAME							shell_dev
#define DT_DEV_MODULE						shell

#define SHELL_DT_RX_SERVER_PDEV				uart0_rx_wrap_dev
#define SHELL_DT_TX_SERVER_PDEV				uart0_tx_wrap_dev
#define SHELL_DT_CALLBACK_PDEV				u_boot_shell_dev

#include ADD_CURRENT_DEV


/***********************************/
/********** u_boot_shell_dev ********/
#define DT_DEV_NAME							u_boot_shell_dev
#define DT_DEV_MODULE						u_boot_shell

#define U_BOOT_SHELL_DT_SERVER_PDEV			shell_dev

#include ADD_CURRENT_DEV


/***********************************/
/********** i2s_dev ********/
#define DT_DEV_NAME							i2s_dev
#define DT_DEV_MODULE						I2S_nuc505

#define I2S_NUC505_DT_NUM_OF_WORDS_IN_BUFFER	I2S_BUFF_LEN
#define I2S_NUC505_DT_NUM_OF_BYTES_IN_WORD		NUM_OF_BYTES_PER_AUDIO_WORD
#define I2S_NUC505_DT_CALLBACK_PDEV				app_dev

#include ADD_CURRENT_DEV


/***********************************/
/********** app_dev ********/
#define DT_DEV_NAME							app_dev
#define DT_DEV_MODULE						app_dev

#define	APP_DT_I2S_DEV				i2s_dev

#include ADD_CURRENT_DEV



/***********************************/
/********** heartbeat_gpio_dev ********/
#define DT_DEV_NAME							heartbeat_gpio_dev
#define DT_DEV_MODULE						gpio_nuc505

#define GPIO_NUC505_DT_PORT					GPIO_NUC505_API_PORT_C
#define GPIO_NUC505_DT_PIN					GPIO_NUC505_API_PIN_3
#define GPIO_NUC505_DT_MODE					GPIO_NUC505_API_MODE_OUT_PP

#include ADD_CURRENT_DEV
