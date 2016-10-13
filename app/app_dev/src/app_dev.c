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
#include "biquad_filter_api.h"
#include "mixer_api.h"
#include "mixer2x1_api.h"
#include "mixer3x1_api.h"
#include "lookahead_compressor_api.h"
#include "standard_compressor_api.h"
#include "virtual_bass_api.h"
#include "voice_3D_api.h"
#include "I2S_nuc505_api.h"
#include "I2S_splitter_api.h"
#include "I2S_mixer_api.h"
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

dsp_descriptor_t lpf_filter;

dsp_descriptor_t vb;
dsp_descriptor_t vb_final_filter;


dsp_descriptor_t hpf_filter_left;
dsp_descriptor_t hpf_filter_right;

dsp_descriptor_t mpf_filter_left;
dsp_descriptor_t mpf_filter_right;

dsp_descriptor_t hpf_voice_3d;
dsp_descriptor_t mpf_voice_3d;

dsp_descriptor_t compressor_hf;
dsp_descriptor_t compressor_mf;

dsp_descriptor_t leftChanelEQ;
dsp_descriptor_t rightChanelEQ;

dsp_descriptor_t limiter;

dsp_descriptor_t stereo_to_mono;
dsp_descriptor_t adder_bass_with_left_channel;
dsp_descriptor_t adder_bass_with_right_channel;


void *dsp_buffers_pool;
float cutoff_freq_low = 100;
float cutoff_freq_high = 4000;
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
	/*
                                          -> hpf_filter_left  ->                              ------>     => adder_bass_with_left_channel -> leftChanelEQ --
                                        /                       \                            /          /                                                     \
                                       /                          => hpf_voice_3d   ===========>           /                                                        => limiter => app_I2S_mixer
                                      /                         /                            \        /                                                       /
                                        -> hpf_filter_right ->                                ------>/ => adder_bass_with_right_channel  -> rightChanelEQ  --
                                    /                                                               /
        source ->  app_I2S_spliter                                                                 /
                                   \                                                              /
                                     => stereo_to_mono -> lpf_filter -> virtual_bass ---------> /

	 */

	pMain_dsp_chain = DSP_CREATE_CHAIN(18);

	DSP_ADD_MODULE_TO_CHAIN(pMain_dsp_chain , I2S_SPLITTER_API_MODULE_NAME ,&app_I2S_spliter );

	/******** LF path modules**********/
	DSP_ADD_MODULE_TO_CHAIN(pMain_dsp_chain, MIXER2X1_API_MODULE_NAME, &stereo_to_mono);
	DSP_ADD_MODULE_TO_CHAIN(pMain_dsp_chain, BIQUAD_FILTER_API_MODULE_NAME, &lpf_filter);
	DSP_ADD_MODULE_TO_CHAIN(pMain_dsp_chain, VIRTUAL_BASS_API_MODULE_NAME, &vb);
	/********* end of LF path    **********/

	/******** HF path modules**********/
	DSP_ADD_MODULE_TO_CHAIN(pMain_dsp_chain, BIQUAD_FILTER_API_MODULE_NAME, &hpf_filter_left);
	DSP_ADD_MODULE_TO_CHAIN(pMain_dsp_chain, BIQUAD_FILTER_API_MODULE_NAME, &hpf_filter_right);
	DSP_ADD_MODULE_TO_CHAIN(pMain_dsp_chain, VOICE_3D_API_MODULE_NAME, &hpf_voice_3d);
	DSP_ADD_MODULE_TO_CHAIN(pMain_dsp_chain, STANDARD_COMPRESSOR_API_MODULE_NAME, &compressor_hf);
	/********* end of HF path    **********/

	/******** MF path modules**********/
	DSP_ADD_MODULE_TO_CHAIN(pMain_dsp_chain, BIQUAD_FILTER_API_MODULE_NAME, &mpf_filter_left);
	DSP_ADD_MODULE_TO_CHAIN(pMain_dsp_chain, BIQUAD_FILTER_API_MODULE_NAME, &mpf_filter_right);
	DSP_ADD_MODULE_TO_CHAIN(pMain_dsp_chain, VOICE_3D_API_MODULE_NAME, &mpf_voice_3d);
	DSP_ADD_MODULE_TO_CHAIN(pMain_dsp_chain, STANDARD_COMPRESSOR_API_MODULE_NAME, &compressor_mf);
	/********* end of MF path    **********/

	DSP_ADD_MODULE_TO_CHAIN(pMain_dsp_chain, MIXER3X1_API_MODULE_NAME, &adder_bass_with_left_channel);
	DSP_ADD_MODULE_TO_CHAIN(pMain_dsp_chain, MIXER3X1_API_MODULE_NAME, &adder_bass_with_right_channel);
	DSP_ADD_MODULE_TO_CHAIN(pMain_dsp_chain, BIQUAD_FILTER_API_MODULE_NAME, &leftChanelEQ);
	DSP_ADD_MODULE_TO_CHAIN(pMain_dsp_chain, BIQUAD_FILTER_API_MODULE_NAME, &rightChanelEQ);
	DSP_ADD_MODULE_TO_CHAIN(pMain_dsp_chain , LOOKAHEAD_COMPRESSOR_API_MODULE_NAME , &limiter);
	DSP_ADD_MODULE_TO_CHAIN(pMain_dsp_chain , I2S_MIXER_API_MODULE_NAME , &app_I2S_mixer);


	//******* distribute L-R **********/
	DSP_CREATE_CHAIN_INPUT_TO_MODULE_LINK(pMain_dsp_chain,DSP_INPUT_PAD_0,&app_I2S_spliter,DSP_INPUT_PAD_0);
#if 1
	/******** LF path **********/
	DSP_CREATE_INTER_MODULES_LINK(&app_I2S_spliter,DSP_OUTPUT_PAD_0,&stereo_to_mono,DSP_INPUT_PAD_0);
	DSP_CREATE_INTER_MODULES_LINK(&app_I2S_spliter,DSP_OUTPUT_PAD_1,&stereo_to_mono,DSP_INPUT_PAD_1);

	DSP_CREATE_INTER_MODULES_LINK(&stereo_to_mono,DSP_OUTPUT_PAD_0,&lpf_filter,DSP_INPUT_PAD_0);

	/********  VB  ************/
	DSP_CREATE_INTER_MODULES_LINK(&lpf_filter,DSP_OUTPUT_PAD_0,&vb,DSP_INPUT_PAD_0);

	/******* end of VB   *********/

	/********* end of LF path    **********/


	/********** HF path *********/

	DSP_CREATE_INTER_MODULES_LINK(&app_I2S_spliter,DSP_OUTPUT_PAD_0,&hpf_filter_left,DSP_INPUT_PAD_0);
	DSP_CREATE_INTER_MODULES_LINK(&app_I2S_spliter,DSP_OUTPUT_PAD_1,&hpf_filter_right,DSP_INPUT_PAD_0);

	DSP_CREATE_INTER_MODULES_LINK(&hpf_filter_left,DSP_OUTPUT_PAD_0,&hpf_voice_3d,DSP_INPUT_PAD_0);
	DSP_CREATE_INTER_MODULES_LINK(&hpf_filter_right,DSP_OUTPUT_PAD_0,&hpf_voice_3d,DSP_INPUT_PAD_1);

	DSP_CREATE_INTER_MODULES_LINK(&hpf_voice_3d,DSP_OUTPUT_PAD_0,&compressor_hf,DSP_INPUT_PAD_0);
	DSP_CREATE_INTER_MODULES_LINK(&hpf_voice_3d,DSP_OUTPUT_PAD_1,&compressor_hf,DSP_INPUT_PAD_1);

	/********* end of HF path  *********/

	/********** MF path *********/

	DSP_CREATE_INTER_MODULES_LINK(&app_I2S_spliter,DSP_OUTPUT_PAD_0,&mpf_filter_left,DSP_INPUT_PAD_0);
	DSP_CREATE_INTER_MODULES_LINK(&app_I2S_spliter,DSP_OUTPUT_PAD_1,&mpf_filter_right,DSP_INPUT_PAD_0);

	DSP_CREATE_INTER_MODULES_LINK(&mpf_filter_left,DSP_OUTPUT_PAD_0,&mpf_voice_3d,DSP_INPUT_PAD_0);
	DSP_CREATE_INTER_MODULES_LINK(&mpf_filter_right,DSP_OUTPUT_PAD_0,&mpf_voice_3d,DSP_INPUT_PAD_1);

	DSP_CREATE_INTER_MODULES_LINK(&mpf_voice_3d,DSP_OUTPUT_PAD_0,&compressor_mf,DSP_INPUT_PAD_0);
	DSP_CREATE_INTER_MODULES_LINK(&mpf_voice_3d,DSP_OUTPUT_PAD_1,&compressor_mf,DSP_INPUT_PAD_1);

	/********* end of MF path  *********/

	/********** collecting LF ,MF and HF pathes together with phase change on HF *********/
	DSP_CREATE_INTER_MODULES_LINK(&vb,DSP_OUTPUT_PAD_0,&adder_bass_with_left_channel,DSP_INPUT_PAD_0);
	DSP_CREATE_INTER_MODULES_LINK(&compressor_hf,DSP_OUTPUT_PAD_0,&adder_bass_with_left_channel,DSP_INPUT_PAD_1);
	DSP_CREATE_INTER_MODULES_LINK(&compressor_mf,DSP_OUTPUT_PAD_0,&adder_bass_with_left_channel,DSP_INPUT_PAD_2);

	DSP_CREATE_INTER_MODULES_LINK(&vb,DSP_OUTPUT_PAD_0,&adder_bass_with_right_channel,DSP_INPUT_PAD_0);
	DSP_CREATE_INTER_MODULES_LINK(&compressor_hf,DSP_OUTPUT_PAD_1,&adder_bass_with_right_channel,DSP_INPUT_PAD_1);
	DSP_CREATE_INTER_MODULES_LINK(&compressor_mf,DSP_OUTPUT_PAD_1,&adder_bass_with_right_channel,DSP_INPUT_PAD_2);

	/********** end of collecting LF ,MF and HF pathes together  *********/

	/********** EQ+Compressor  *********/

	DSP_CREATE_INTER_MODULES_LINK(&adder_bass_with_left_channel,DSP_OUTPUT_PAD_0,&leftChanelEQ,DSP_INPUT_PAD_0);

	DSP_CREATE_INTER_MODULES_LINK(&adder_bass_with_right_channel,DSP_OUTPUT_PAD_0,&rightChanelEQ,DSP_INPUT_PAD_0);

	DSP_CREATE_INTER_MODULES_LINK(&leftChanelEQ,DSP_OUTPUT_PAD_0,&limiter,DSP_INPUT_PAD_0);
	DSP_CREATE_INTER_MODULES_LINK(&rightChanelEQ,DSP_OUTPUT_PAD_0,&limiter,DSP_INPUT_PAD_1);

	/********** end ofEQ+Compressor  *********/

	/********** mix 2 channels to I2S bus  *********/
	DSP_CREATE_INTER_MODULES_LINK(&limiter,DSP_OUTPUT_PAD_0,&app_I2S_mixer,DSP_INPUT_PAD_0);
	DSP_CREATE_INTER_MODULES_LINK(&limiter,DSP_OUTPUT_PAD_1,&app_I2S_mixer,DSP_INPUT_PAD_1);
#else
	DSP_CREATE_INTER_MODULES_LINK(&app_I2S_spliter,DSP_OUTPUT_PAD_0,&app_I2S_mixer,DSP_INPUT_PAD_0);
	DSP_CREATE_INTER_MODULES_LINK(&app_I2S_spliter,DSP_OUTPUT_PAD_1,&app_I2S_mixer,DSP_INPUT_PAD_1);
#endif
	DSP_CREATE_MODULE_TO_CHAIN_OUTPUT_LINK(pMain_dsp_chain, DSP_OUTPUT_PAD_0, &app_I2S_mixer, DSP_OUTPUT_PAD_0);

	return 0;
}


/*---------------------------------------------------------------------------------------------------------*/
/* Function:        app_dev_set_cuttof                                                                          */
/*                                                                                                         */
/* Parameters:                                                                                             */
/*                                                                                         */
/*                                                                                                  */
/* Returns:                                                                                      */
/* Side effects:                                                                                           */
/* Description:                                                                                            */
/*                                                            						 */
/*---------------------------------------------------------------------------------------------------------*/
uint8_t app_dev_set_cuttof()
{
	biquad_filter_api_band_set_t band_set;
	biquad_filter_api_band_set_params_t  *p_band_set_params;
	float freq;

	p_band_set_params = &band_set.band_set_params;

	p_band_set_params->Gain = 1;

	p_band_set_params->Fc = cutoff_freq_low;
	p_band_set_params->QValue = 1;//0.836;//0.707;
	p_band_set_params->filter_mode = BIQUADS_LOWPASS_MODE_1_POLE;
	band_set.band_num = 0;
	DSP_IOCTL_1_PARAMS(&lpf_filter , IOCTL_BIQUAD_FILTER_SET_BAND_BIQUADS, &band_set );

	p_band_set_params->Fc = cutoff_freq_low;
	p_band_set_params->QValue = 1;//0.836;//0.707;
	p_band_set_params->filter_mode = BIQUADS_HIGHPASS_MODE_2_POLES;
	band_set.band_num = 0;
	DSP_IOCTL_1_PARAMS(&mpf_filter_left , IOCTL_BIQUAD_FILTER_SET_BAND_BIQUADS, &band_set );
	DSP_IOCTL_1_PARAMS(&mpf_filter_right, IOCTL_BIQUAD_FILTER_SET_BAND_BIQUADS, &band_set );
	p_band_set_params->filter_mode = BIQUADS_HIGHPASS_MODE_1_POLE;
	band_set.band_num = 1;
	DSP_IOCTL_1_PARAMS(&mpf_filter_left , IOCTL_BIQUAD_FILTER_SET_BAND_BIQUADS, &band_set );
	DSP_IOCTL_1_PARAMS(&mpf_filter_right , IOCTL_BIQUAD_FILTER_SET_BAND_BIQUADS, &band_set );
	p_band_set_params->Fc = cutoff_freq_high;
	p_band_set_params->filter_mode = BIQUADS_LOWPASS_MODE_2_POLES;
	p_band_set_params->QValue = 0.707;//0.836;//0.707;
	band_set.band_num = 2;
	DSP_IOCTL_1_PARAMS(&mpf_filter_left , IOCTL_BIQUAD_FILTER_SET_BAND_BIQUADS, &band_set );
	DSP_IOCTL_1_PARAMS(&mpf_filter_right, IOCTL_BIQUAD_FILTER_SET_BAND_BIQUADS, &band_set );
	p_band_set_params->filter_mode = BIQUADS_LOWPASS_MODE_2_POLES;
	band_set.band_num = 3;
	DSP_IOCTL_1_PARAMS(&mpf_filter_left , IOCTL_BIQUAD_FILTER_SET_BAND_BIQUADS, &band_set );
	DSP_IOCTL_1_PARAMS(&mpf_filter_right , IOCTL_BIQUAD_FILTER_SET_BAND_BIQUADS, &band_set );


	p_band_set_params->Fc = cutoff_freq_high;
	p_band_set_params->QValue = 0.707;//0.836;//0.707;
	p_band_set_params->filter_mode = BIQUADS_HIGHPASS_MODE_2_POLES;
	band_set.band_num = 0;
	DSP_IOCTL_1_PARAMS(&hpf_filter_left , IOCTL_BIQUAD_FILTER_SET_BAND_BIQUADS, &band_set );
	DSP_IOCTL_1_PARAMS(&hpf_filter_right, IOCTL_BIQUAD_FILTER_SET_BAND_BIQUADS, &band_set );
	band_set.band_num = 1;
	DSP_IOCTL_1_PARAMS(&hpf_filter_left , IOCTL_BIQUAD_FILTER_SET_BAND_BIQUADS, &band_set );
	DSP_IOCTL_1_PARAMS(&hpf_filter_right , IOCTL_BIQUAD_FILTER_SET_BAND_BIQUADS, &band_set );

	freq = 100;
	DSP_IOCTL_1_PARAMS(&vb , IOCTL_VIRTUAL_BASS_SET_FIRST_HPF, &cutoff_freq_low  );
	DSP_IOCTL_1_PARAMS(&vb , IOCTL_VIRTUAL_BASS_SET_SECOND_HPF, &freq);

//	p_band_set_params->QValue = 1;//0.836;//0.707;
//	p_band_set_params->Gain = 1;
//	p_band_set_params->Fc = cutoff_freq_low * 3.4;
//	p_band_set_params->filter_mode = BIQUADS_LOWPASS_MODE_2_POLES;
//	band_set.band_num = 0;
//	DSP_IOCTL_1_PARAMS(&vb_final_filter , IOCTL_BIQUAD_FILTER_SET_BAND_BIQUADS, &band_set );
//	p_band_set_params->Fc = cutoff_freq_low * 0.2;
//	p_band_set_params->filter_mode = BIQUADS_HIGHPASS_MODE_1_POLE;
//	band_set.band_num = 1;
//	DSP_IOCTL_1_PARAMS(&vb_final_filter , IOCTL_BIQUAD_FILTER_SET_BAND_BIQUADS, &band_set );
//	p_band_set_params->QValue = 1;//0.836;//0.707;
//	p_band_set_params->filter_mode = BIQUADS_HIGHPASS_MODE_2_POLES;
//	band_set.band_num = 2;
//	DSP_IOCTL_1_PARAMS(&vb_final_filter , IOCTL_BIQUAD_FILTER_SET_BAND_BIQUADS, &band_set );


	return 0;
}


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
	float threshold ;
	float gain ;
	set_channel_weight_t ch_weight;

	config_handle = DEV_GET_CONFIG_DATA_POINTER(apdev);

	switch(aIoctl_num)
	{
		case IOCTL_DEVICE_START :

			/* Create an application thread */

			dsp_buffers_pool = memory_pool_init(7 , I2S_BUFF_LEN * sizeof(float));
			dsp_management_api_set_buffers_pool(dsp_buffers_pool);

			app_dev_create_signal_flow();


#if 1
			/**************   LPF path  *************/
			ch_weight.weight = 0.5;
			ch_weight.channel_num = 0;
			DSP_IOCTL_1_PARAMS(&stereo_to_mono , IOCTL_MIXER_SET_CHANNEL_WEIGHT , &ch_weight  );
			ch_weight.channel_num = 1;
			DSP_IOCTL_1_PARAMS(&stereo_to_mono , IOCTL_MIXER_SET_CHANNEL_WEIGHT , &ch_weight  );
			//dsp_management_api_set_module_control(&stereo_to_mono , DSP_MANAGEMENT_API_MODULE_CONTROL_BYPASS);

			DSP_IOCTL_1_PARAMS(&lpf_filter , IOCTL_BIQUAD_FILTER_SET_NUM_OF_BANDS , 1 );
			dsp_management_api_set_module_control(&lpf_filter , DSP_MANAGEMENT_API_MODULE_CONTROL_BYPASS);
			//dsp_management_api_set_module_control(&lpf_filter , DSP_MANAGEMENT_API_MODULE_CONTROL_BYPASS);

			//dsp_management_api_set_module_control(&vb , DSP_MANAGEMENT_API_MODULE_CONTROL_BYPASS);



			/**************   HPF path  *************/
			DSP_IOCTL_1_PARAMS(&hpf_filter_left , IOCTL_BIQUAD_FILTER_SET_NUM_OF_BANDS , 2 );
			DSP_IOCTL_1_PARAMS(&hpf_filter_right , IOCTL_BIQUAD_FILTER_SET_NUM_OF_BANDS , 2 );
//			dsp_management_api_set_module_control(&hpf_filter_left , DSP_MANAGEMENT_API_MODULE_CONTROL_BYPASS);
//			dsp_management_api_set_module_control(&hpf_filter_right , DSP_MANAGEMENT_API_MODULE_CONTROL_BYPASS);

			gain=0.5;
			DSP_IOCTL_1_PARAMS(&hpf_voice_3d , IOCTL_VOICE_3D_SET_MEDIUM_GAIN , &gain );
			gain = 0.5;
			DSP_IOCTL_1_PARAMS(&hpf_voice_3d , IOCTL_VOICE_3D_SET_SIDE_GAIN , &gain );
			gain = 0 ;
			DSP_IOCTL_1_PARAMS(&hpf_voice_3d , IOCTL_VOICE_3D_SET_3D_GAIN , &gain );
			//dsp_management_api_set_module_control(&hpf_voice_3d , DSP_MANAGEMENT_API_MODULE_CONTROL_BYPASS);
			//dsp_management_api_set_module_control(&compressor_hf , DSP_MANAGEMENT_API_MODULE_CONTROL_BYPASS);

			/**************   MPF path  *************/
			DSP_IOCTL_1_PARAMS(&mpf_filter_left , IOCTL_BIQUAD_FILTER_SET_NUM_OF_BANDS , 4 );
			DSP_IOCTL_1_PARAMS(&mpf_filter_right , IOCTL_BIQUAD_FILTER_SET_NUM_OF_BANDS , 4 );
//			dsp_management_api_set_module_control(&mpf_filter_left , DSP_MANAGEMENT_API_MODULE_CONTROL_BYPASS);
//			dsp_management_api_set_module_control(&mpf_filter_right , DSP_MANAGEMENT_API_MODULE_CONTROL_BYPASS);

			gain=0.5;
			DSP_IOCTL_1_PARAMS(&mpf_voice_3d , IOCTL_VOICE_3D_SET_MEDIUM_GAIN , &gain );
			gain = 0.5;
			DSP_IOCTL_1_PARAMS(&mpf_voice_3d , IOCTL_VOICE_3D_SET_SIDE_GAIN , &gain );
			gain = 0 ;
			DSP_IOCTL_1_PARAMS(&mpf_voice_3d , IOCTL_VOICE_3D_SET_3D_GAIN , &gain );
			//dsp_management_api_set_module_control(&mpf_voice_3d , DSP_MANAGEMENT_API_MODULE_CONTROL_BYPASS);
			//dsp_management_api_set_module_control(&compressor_mf , DSP_MANAGEMENT_API_MODULE_CONTROL_BYPASS);

			/**************  final mixed LPF+HPF path  *************/
			ch_weight.channel_num = 0;
			ch_weight.weight = 1;
			DSP_IOCTL_1_PARAMS(&adder_bass_with_left_channel , IOCTL_MIXER_SET_CHANNEL_WEIGHT , &ch_weight  );
			DSP_IOCTL_1_PARAMS(&adder_bass_with_right_channel , IOCTL_MIXER_SET_CHANNEL_WEIGHT , &ch_weight  );
			ch_weight.channel_num = 1;
			ch_weight.weight = -1;
			DSP_IOCTL_1_PARAMS(&adder_bass_with_left_channel , IOCTL_MIXER_SET_CHANNEL_WEIGHT , &ch_weight  );
			DSP_IOCTL_1_PARAMS(&adder_bass_with_right_channel , IOCTL_MIXER_SET_CHANNEL_WEIGHT , &ch_weight  );
			ch_weight.channel_num = 2;
			ch_weight.weight = -1;
			DSP_IOCTL_1_PARAMS(&adder_bass_with_left_channel , IOCTL_MIXER_SET_CHANNEL_WEIGHT , &ch_weight  );
			DSP_IOCTL_1_PARAMS(&adder_bass_with_right_channel , IOCTL_MIXER_SET_CHANNEL_WEIGHT , &ch_weight  );
//			dsp_management_api_set_module_control(&adder_bass_with_left_channel , DSP_MANAGEMENT_API_MODULE_CONTROL_BYPASS);
//			dsp_management_api_set_module_control(&adder_bass_with_right_channel , DSP_MANAGEMENT_API_MODULE_CONTROL_BYPASS);



			     /*    eq filters   */
			DSP_IOCTL_1_PARAMS(&leftChanelEQ , IOCTL_BIQUAD_FILTER_SET_NUM_OF_BANDS ,7 );
			DSP_IOCTL_1_PARAMS(&rightChanelEQ , IOCTL_BIQUAD_FILTER_SET_NUM_OF_BANDS , 7);
			//dsp_management_api_set_module_control(&leftChanelEQ , DSP_MANAGEMENT_API_MODULE_CONTROL_BYPASS);
			//dsp_management_api_set_module_control(&rightChanelEQ , DSP_MANAGEMENT_API_MODULE_CONTROL_BYPASS);

			DSP_IOCTL_1_PARAMS(&limiter , IOCTL_LOOKAHEAD_COMPRESSOR_SET_TYPE , LOOKAHEAD_COMPRESSOR_API_TYPE_LIMITER);
			threshold = 0.9999;
			DSP_IOCTL_1_PARAMS(&limiter , IOCTL_LOOKAHEAD_COMPRESSOR_SET_HIGH_THRESHOLD , &threshold );
			DSP_IOCTL_1_PARAMS(&limiter , IOCTL_LOOKAHEAD_COMPRESSOR_SET_LOOK_AHEAD_SIZE , LATENCY_LENGTH );
			//dsp_management_api_set_module_control(&limiter , DSP_MANAGEMENT_API_MODULE_CONTROL_BYPASS);

			app_dev_set_cuttof();
#endif

			os_create_task("main" , main_thread_func, config_handle , MAIN_STACK_SIZE_BYTES , APP_DEV_THREAD_PRIORITY);

			DEV_IOCTL_0_PARAMS(config_handle->i2s_dev , IOCTL_DEVICE_START );

			break;
		default :
			return 1;
	}
	return 0;
}
