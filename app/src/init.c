

/* Standard includes. */
#include "_project_typedefs.h"
#include "_project_defines.h"
#include "_project_func_declarations.h"

//#include "assert.h"

#include "dev_managment_api.h"
#include "clocks_control_nuc505_api.h"


#include "included_modules.h"

#include "NUC505Series.h"

#include "NVIC_api.h"

#include "app_dev_api.h"

#include "gpio.h"

#include "cortexM_systick_api.h"
#include "heartbeat_api.h"

/*-----------------------------------------------------------*/




/*-----------------------------------------------------------*/

extern void vPortSVCHandler(void);
//extern void do_software_interrupt_asm(void);
extern void xPortPendSVHandler(void);
extern void xPortSysTickHandler(void);

#define	RX_BUFF_SIZE	256
uint8_t rx_buff[RX_BUFF_SIZE];


void heartbeat_callback(void);

CORTEXM_SYSTICK_API_CREATE_STATIC_DEV(systick_dev_inst,"systick",96000000 ,
		1000 , TIMER_API_PERIODIC_MODE , xPortSysTickHandler );

HEARTBEAT_API_CREATE_STATIC_DEV(heartbeat_dev_inst,"hrtbeat",heartbeat_callback , systick_dev_inst);

UART_NUC505_API_CREATE_STATIC_DEV(uart_dev_inst,"uart0",0,uart_wrap_dev_inst);

SW_UART_WRAPPER_API_CREATE_STATIC_DEV(uart_wrap_dev_inst,"swuart",
		uart_dev_inst,shell_dev_inst , rx_buff,RX_BUFF_SIZE);

//SW_UART_WRAPPER_API_CREATE_STATIC_DEV(uart_wrap_dev_inst,"swuart",
//		uart_dev_inst,dummy_dev , rx_buff,RX_BUFF_SIZE);

SHELL_API_CREATE_STATIC_DEV(shell_dev_inst,"shell",uart_wrap_dev_inst);

APP_API_CREATE_STATIC_DEV(app_dev_inst,"app"   );

I2S_NUC505_API_CREATE_STATIC_DEV(i2s_dev_inst,"i2s",app_dev_inst);

GPIO_NUC505_API_CREATE_STATIC_DEV(heartbeat_gpio_dev_inst,"ioHrt" , GPIO_NUC505_API_PORT_C ,
		0x00000008 ,GPIO_NUC505_API_MODE_OUT_PP);

//ARM_SH_API_CREATE_STATIC_DEV(sh_dev,"sh_arm");
//
//SW_UART_WRAPPER_API_CREATE_STATIC_DEV(sh_wrap_dev,"swu_sh",
//		sh_dev,dummy_dev , NULL,0);
//
//
//SPI_STM32F10X_API_CREATE_STATIC_DEV(spi_dev,"spi"  );
//
//GPIO_STM32F10X_API_CREATE_STATIC_DEV(spiFlsh_gpio_dev,"ioFlsh" ,GPIO_STM32F10X_API_PORT_A,
//		4 ,GPIO_STM32F10X_API_MODE_OUT_PP);
//
//
//SPI_FLASH_API_CREATE_STATIC_DEV(spi_flash_dev_inst,"spiFlsh"  ,spi_dev , spiFlsh_gpio_dev );
//
//
//SPI_FLASH_PARTITION_MANAGER_API_CREATE_STATIC_DEV(spi_flash_partition_manager_dev,"pmFlsh"  ,spi_flash_dev_inst );
//
//INIT_STATIC_DEVICES(
//		&spi_flash_dev_inst,
//		&spi_flash_partition_manager_dev
//		);
//
//pdev_descriptor_const semihosting_dev = &sh_dev;
//pdev_descriptor_const semihosting_dev_wrap = &sh_wrap_dev;


pdev_descriptor_const uart0_dev = &uart_dev_inst;
pdev_descriptor_const uart0_dev_wrap = &uart_wrap_dev_inst;
pdev_descriptor_const shell_dev = &shell_dev_inst;
pdev_descriptor_const i2s_dev = &i2s_dev_inst;
pdev_descriptor_const heartbeat_dev = &heartbeat_dev_inst;
pdev_descriptor_const systick_dev = &systick_dev_inst;
pdev_descriptor_const app_dev = &app_dev_inst;
pdev_descriptor_const heartbeat_gpio_dev = &heartbeat_gpio_dev_inst;

INIT_STATIC_DEVICES(&uart_wrap_dev_inst);

/**** end ofsetup of static devices and device managment ****/
/************************************************************/




/* function : NVIC_APP_Init
 *
 *
 *
 */
void	NVIC_APP_Init(void)
{
	NVIC_API_Init();
	NVIC_API_RegisterInt(SVCall_IRQn , vPortSVCHandler);
//	NVIC_API_RegisterInt(NVIC_API_Int_SVCall , do_software_interrupt_asm);
//#ifndef _NO_OS
	NVIC_API_RegisterInt(PendSV_IRQn , xPortPendSVHandler);
//	NVIC_API_RegisterInt(SysTick_IRQn , xPortSysTickHandler);
//#endif

}



/* function : prvSetupHardware
 *
 *
 *
 */
void prvSetupHardware( void )
{
	//pdev_descriptor dev;
    uint32_t baud;

	NVIC_APP_Init();


	clocks_control_nuc505_init();

	DEV_API_add_device((uint8_t*)"version",version_managment_api_init_dev_descriptor);


//	dev=DEV_API_add_device((uint8_t*)"serial",serial_number_stm32f10x_api_init_dev_descriptor);
//	DEV_IOCTL_0_PARAMS(dev , IOCTL_DEVICE_START   );
//
//	DEV_IOCTL(dev , IOCTL_SERIAL_NUM_GET , &pSerNum);


	NVIC_API_EnableAllInt();
	DEV_IOCTL_0_PARAMS(heartbeat_dev , IOCTL_DEVICE_START );
	NVIC_API_DisableAllInt();

	baud = 115200;
	DEV_IOCTL_1_PARAMS(uart0_dev , IOCTL_UART_SET_BAUD_RATE , &baud);

	DEV_IOCTL_0_PARAMS(uart0_dev , IOCTL_DEVICE_START );

	DEV_IOCTL_0_PARAMS(uart0_dev_wrap , IOCTL_DEVICE_START );

	DEV_IOCTL_0_PARAMS(shell_dev , IOCTL_DEVICE_START );

	DEV_IOCTL_0_PARAMS(heartbeat_gpio_dev , IOCTL_DEVICE_START );

	DEV_IOCTL_0_PARAMS(app_dev , IOCTL_DEVICE_START );

	DEV_IOCTL_0_PARAMS(i2s_dev , IOCTL_DEVICE_START );


	PRINTF_API_AddDebugOutput(uart0_dev_wrap);

}
















/*-----------------------------------------------------------*/


