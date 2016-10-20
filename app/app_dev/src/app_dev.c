/*
 *
 * file :   app_dev.c
 *
 *
 *
 *
 *
 */



/********  includes *********************/
#include "_project.h"
#include "dev_management_api.h" // for device manager defines and typedefs

#include "app_dev_api.h"

#include "app_dev.h"

#define DEBUG
#include "PRINTF_api.h"

#include "os_wrapper.h"

#include "dsp_management_api.h"
#include "fir_filter_api.h"
#include "I2S_nuc505_api.h"
#include "I2S_splitter_api.h"
#include "I2S_mixer_api.h"
#include "fir_filter_api.h"
#include "math.h"
#include "memory_pool_api.h"

#include "cpu_config.h"
#include "arm_math.h"

#include "app_dev_add_component.h"

/********  defines *********************/
#if (2 == NUM_OF_BYTES_PER_AUDIO_WORD)
	typedef int16_t	buffer_type_t	;
#endif
#if (4 == NUM_OF_BYTES_PER_AUDIO_WORD)
	typedef int32_t	buffer_type_t	;
#endif


#define APP_DEV_CONFIG_MAX_QUEUE_LEN					( 2 )

#define INSTANCE(hndl)	((app_instance_t*)hndl)

/********  types  *********************/

/********  externals *********************/



/********  local defs *********************/



/********  types  *********************/


typedef struct
{

	void *rxBuffer;
	void *txBuffer;

} xMessage_t;


static os_queue_t xQueue=NULL ;


dsp_descriptor_t app_I2S_spliter = {0};
dsp_descriptor_t app_I2S_mixer;

dsp_descriptor_t fir_filter_dsp;

void *dsp_buffers_pool;

uint8_t loopback = 0 ;
os_mutex_t  control_mutex;
static dsp_chain_t *pMain_dsp_chain;

/*---------------------------------------------------------------------------------------------------------*/
/* Function:        app_dev_callback                                                                          */
/*                                                                                                         */
/* Parameters:                                                                                             */
/*                                                                                         */
/*                                                                                                  */
/* Returns:                                                                                      */
/* Side effects:                                                                                           */
/* Description:                                                                                            */
/*                                                            						 */
/*---------------------------------------------------------------------------------------------------------*/
uint8_t app_dev_callback(pdev_descriptor_t apdev ,
		const uint8_t aCallback_num , void * aCallback_param1, void * aCallback_param2)
{
	xMessage_t  queueMsg;

	if(NULL == xQueue) return 1;

	queueMsg.rxBuffer = aCallback_param1;
	queueMsg.txBuffer = aCallback_param2;
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
static void main_thread_func (void * aHandle)
{
	xMessage_t xRxMessage;
	uint16_t i;//,j;
	buffer_type_t *pRxBuf;
	buffer_type_t *pTxBuf;

	xQueue = os_create_queue( APP_DEV_CONFIG_MAX_QUEUE_LEN , sizeof(xMessage_t ) );

	control_mutex = os_create_mutex();

	DEV_IOCTL_0_PARAMS(INSTANCE(aHandle)->i2s_dev , I2S_ENABLE_OUTPUT_IOCTL );

    while (1)
    {
		if( OS_QUEUE_RECEIVE_SUCCESS ==  os_queue_receive_infinite_wait( xQueue , &( xRxMessage )  ))
		{

			pRxBuf =  (buffer_type_t*)xRxMessage.rxBuffer;
			pTxBuf =  (buffer_type_t*)xRxMessage.txBuffer;

			if(1 == loopback)//transparent mode
			{
				for(i = 0 ; i < I2S_BUFF_LEN ; i++)
				{
					pTxBuf[2*i] =  pRxBuf[2*i ];
					pTxBuf[2*i + 1] =  pRxBuf[2*i + 1];

				}
			}
			else
			{
				os_mutex_take_infinite_wait(control_mutex);

				DSP_SET_CHAIN_INPUT_BUFFER(pMain_dsp_chain, DSP_INPUT_PAD_0, (float*)pRxBuf );
				DSP_SET_CHAIN_OUTPUT_BUFFER(pMain_dsp_chain, DSP_OUTPUT_PAD_0, (float*)pTxBuf );
				DSP_PROCESS_CHAIN(pMain_dsp_chain , I2S_BUFF_LEN );

				os_mutex_give(control_mutex);

			}//if (1==loopback )
		}

		os_stack_test();

    }
}

/*---------------------------------------------------------------------------------------------------------*/
/* Function:        app_dev_create_signal_flow                                                                          */
/*                                                                                                         */
/* Parameters:                                                                                             */
/*                                                                                         */
/*                                                                                                  */
/* Returns:                                                                                      */
/* Side effects:                                                                                           */
/* Description:                                                                                            */
/*                                                            						 */
/*---------------------------------------------------------------------------------------------------------*/
static uint8_t app_dev_create_signal_flow()
{

	pMain_dsp_chain = DSP_CREATE_CHAIN(18);

	DSP_ADD_MODULE_TO_CHAIN(pMain_dsp_chain , I2S_SPLITTER_API_MODULE_NAME ,&app_I2S_spliter );

	DSP_ADD_MODULE_TO_CHAIN(pMain_dsp_chain, FIR_FILTER_API_MODULE_NAME, &fir_filter_dsp);

	DSP_ADD_MODULE_TO_CHAIN(pMain_dsp_chain , I2S_MIXER_API_MODULE_NAME , &app_I2S_mixer);


	//******* distribute L-R **********/
	DSP_CREATE_CHAIN_INPUT_TO_MODULE_LINK(pMain_dsp_chain,DSP_INPUT_PAD_0,&app_I2S_spliter,DSP_INPUT_PAD_0);

	DSP_CREATE_INTER_MODULES_LINK(&app_I2S_spliter,DSP_OUTPUT_PAD_0,&fir_filter_dsp,DSP_INPUT_PAD_0);

	DSP_CREATE_INTER_MODULES_LINK(&fir_filter_dsp,DSP_OUTPUT_PAD_0,&app_I2S_mixer,DSP_INPUT_PAD_0);
	DSP_CREATE_INTER_MODULES_LINK(&app_I2S_spliter,DSP_OUTPUT_PAD_1,&app_I2S_mixer,DSP_INPUT_PAD_1);

	DSP_CREATE_MODULE_TO_CHAIN_OUTPUT_LINK(pMain_dsp_chain, DSP_OUTPUT_PAD_0, &app_I2S_mixer, DSP_OUTPUT_PAD_0);

	return 0;
}

#define NUM_OF_COEFF	101
float coeff[NUM_OF_COEFF] = { 1.3410e-020, -2.2724e-006, -4.3747e-006, 1.5253e-005, 3.6023e-005, -8.9013e-006, -9.5324e-005, -6.5367e-005, 1.2828e-004 ,
  2.2355e-004, -3.9006e-005, -3.9737e-004, -2.4790e-004, 4.2167e-004, 6.9403e-004, -9.8499e-005, -1.0789e-003, -6.6021e-004 ,
  1.0299e-003, 1.6709e-003, -1.9532e-004, -2.4102e-003, -1.4769e-003, 2.1509e-003, 3.4926e-003, -3.3116e-004, -4.7880e-003 ,
  -2.9659e-003, 4.0713e-003, 6.6751e-003, -4.9700e-004, -8.8406e-003, -5.5773e-003, 7.2772e-003, 1.2155e-002, -6.7238e-004 ,
  -1.5849e-002, -1.0289e-002, 1.2922e-002, 2.2310e-002, -8.2897e-004, -2.9723e-002, -2.0352e-002, 2.5403e-002, 4.7294e-002,
  -9.3764e-004, -7.4031e-002, -6.0681e-002, 9.3727e-002, 3.0194e-001, 3.9902e-001, 3.0194e-001, 9.3727e-002, -6.0681e-002 ,
  -7.4031e-002, -9.3764e-004, 4.7294e-002, 2.5403e-002, -2.0352e-002, -2.9723e-002, -8.2897e-004, 2.2310e-002, 1.2922e-002 ,
  -1.0289e-002, -1.5849e-002, -6.7238e-004, 1.2155e-002, 7.2772e-003, -5.5773e-003, -8.8406e-003, -4.9700e-004, 6.6751e-003 ,
  4.0713e-003, -2.9659e-003, -4.7880e-003, -3.3116e-004, 3.4926e-003, 2.1509e-003, -1.4769e-003, -2.4102e-003, -1.9532e-004,
  1.6709e-003, 1.0299e-003, -6.6021e-004, -1.0789e-003, -9.8499e-005, 6.9403e-004, 4.2167e-004, -2.4790e-004, -3.9737e-004,
  -3.9006e-005, 2.2355e-004, 1.2828e-004, -6.5367e-005, -9.5324e-005, -8.9013e-006, 3.6023e-005, 1.5253e-005, -4.3747e-006 ,
  -2.2724e-006, 1.3410e-020 };

#if 0

#define NUM_OF_COEFF	21
float coeff[NUM_OF_COEFF] = {1.3542e-020, -3.1174e-004, -9.0803e-004, 2.7882e-003, 1.0064e-002, -3.3188e-004, -3.8719e-002, -4.2420e-002, 8.0087e-002 ,
  2.9031e-001, 3.9889e-001, 2.9031e-001, 8.0087e-002, -4.2420e-002, -3.8719e-002, -3.3188e-004, 1.0064e-002, 2.7882e-003 ,
  -9.0803e-004, -3.1174e-004, 1.3542e-020 };

#endif

#if 0

#define NUM_OF_COEFF	6
float coeff[NUM_OF_COEFF] = { -1.8944e-020, 5.6725e-002, 4.4327e-001, 4.4327e-001, 5.6725e-002, -1.8944e-020 };

#endif

/*---------------------------------------------------------------------------------------------------------*/
/* Function:        app_dev_ioctl                                                                          */
/*                                                                                                         */
/* Parameters:                                                                                             */
/*                                                                                         */
/*                                                                                                  */
/* Returns:                                                                                      */
/* Side effects:                                                                                           */
/* Description:                                                                                            */
/*                                                            						 */
/*---------------------------------------------------------------------------------------------------------*/
uint8_t app_dev_ioctl( pdev_descriptor_t apdev ,const uint8_t aIoctl_num
		, void * aIoctl_param1 , void * aIoctl_param2)
{
	app_instance_t *config_handle;
	fir_filter_api_set_params_t	fir_filter_api_set;
	config_handle = DEV_GET_CONFIG_DATA_POINTER(apdev);

	switch(aIoctl_num)
	{
		case IOCTL_DEVICE_START :

			/* Create an application thread */

			dsp_buffers_pool = memory_pool_init(7 , I2S_BUFF_LEN * sizeof(float));
			dsp_management_api_set_buffers_pool(dsp_buffers_pool);

			app_dev_create_signal_flow();

			fir_filter_api_set.number_of_filter_coefficients = NUM_OF_COEFF;
			fir_filter_api_set.p_coefficients = coeff;
			fir_filter_api_set.predefined_data_block_size = I2S_BUFF_LEN;
			DSP_IOCTL_1_PARAMS(&fir_filter_dsp , IOCTL_FIR_FILTER_SET_FIR_COEFFICIENTS , &fir_filter_api_set);

			os_create_task("main" , main_thread_func, config_handle , MAIN_STACK_SIZE_BYTES , APP_DEV_THREAD_PRIORITY);

			DEV_IOCTL_0_PARAMS(config_handle->i2s_dev , IOCTL_DEVICE_START );

			break;
		default :
			return 1;
	}
	return 0;
}
