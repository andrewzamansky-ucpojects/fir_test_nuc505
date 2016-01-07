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
#include "app_dev_config.h"
#include "dev_managment_api.h" // for device manager defines and typedefs

#include "app_dev_api.h"

#include "app_dev.h"


#include "PRINTF_api.h"

#include "os_wrapper.h"

#include "dsp_managment_api.h"
#include "equalizer_api.h"
#include "mixer_api.h"
#include "compressor_api.h"
#include "virtual_bass_api.h"
#include "voice_3D_api.h"
#include "I2S_nuc505_api.h"
#include "I2S_splitter_api.h"
#include "I2S_mixer_api.h"
#include "timer_api.h"
#include "heartbeat_api.h"
#include "math.h"
#include "memory_pool_api.h"

 #include "arm_math.h"

/********  defines *********************/
#if (2 == NUM_OF_BYTES_PER_AUDIO_WORD)
	typedef int16_t	buffer_type_t	;
#endif
#if (4 == NUM_OF_BYTES_PER_AUDIO_WORD)
	typedef int32_t	buffer_type_t	;
#endif


/********  types  *********************/

/********  externals *********************/



/********  local defs *********************/



/********  types  *********************/


typedef struct
{

	void *rxBuffer;
	void *txBuffer;

} xMessage_t;


os_queue_t xQueue=NULL ;


dsp_descriptor_t app_I2S_spliter;
dsp_descriptor_t app_I2S_mixer;

dsp_descriptor_t lpf_filter;

dsp_descriptor_t vb;
dsp_descriptor_t vb_hpf_filter;
dsp_descriptor_t vb_final_filter;


dsp_descriptor_t hpf_filter_left;
dsp_descriptor_t hpf_filter_right;

dsp_descriptor_t voice_3d;

dsp_descriptor_t leftChanelEQ;
dsp_descriptor_t rightChanelEQ;

dsp_descriptor_t compressor_limiter;
dsp_descriptor_t vb_limiter;

dsp_descriptor_t stereo_to_mono;
dsp_descriptor_t adder_bass_with_left_channel;
dsp_descriptor_t adder_bass_with_right_channel;

extern pdev_descriptor_const i2s_dev;
extern pdev_descriptor_const heartbeat_dev ;

void *dsp_buffers_pool;

float cutoff_freq = 100;

TIMER_API_CREATE_STATIC_DEV(app_timer_inst,"apptim" ,systick_dev_inst );
static pdev_descriptor_const  timer_device = &app_timer_inst;

uint8_t loopback = 0 ;



static uint8_t s_flag=0;


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
uint8_t app_dev_callback(void * const aHandle ,
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
static void main_thread_func (void * param)
{
	xMessage_t xRxMessage;
	uint16_t i;//,j;
	buffer_type_t *pRxBuf;
	buffer_type_t *pTxBuf;
	uint8_t is_timer_elapsed=1;
	uint32_t retVal;
	xQueue = os_create_queue( APP_DEV_CONFIG_MAX_QUEUE_LEN , sizeof(xMessage_t ) );
	uint64_t cpu_usage_print_timeout = 1000;

    while (1)
    {
    	retVal = os_queue_receive_with_timeout( xQueue , &( xRxMessage ) , 1000 );
		if( OS_QUEUE_RECEIVE_SUCCESS ==  retVal)
		{

			if ( s_flag < 2 )
			{
				if(1 == s_flag)
				{
					DEV_IOCTL_0_PARAMS(i2s_dev , I2S_ENABLE_OUTPUT_IOCTL );
				}
				s_flag++;
			}

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
				//******* distribute L-R
				DSP_FUNC_1CH_IN_2CH_OUT(&app_I2S_spliter,(float*)pRxBuf, I2S_BUFF_LEN );



				/******** LF path **********/

				DSP_FUNC_2CH_IN_1CH_OUT(&stereo_to_mono,
						app_I2S_spliter.out_pads[0],app_I2S_spliter.out_pads[1],  I2S_BUFF_LEN );

				DSP_FUNC_1CH_IN_1CH_OUT(&lpf_filter,stereo_to_mono.out_pads[0] ,I2S_BUFF_LEN );
				memory_pool_free(dsp_buffers_pool,stereo_to_mono.out_pads[0]);

				/********  VB  ************/
				DSP_FUNC_1CH_IN_1CH_OUT(&vb,lpf_filter.out_pads[0] ,I2S_BUFF_LEN );
				memory_pool_free(dsp_buffers_pool,lpf_filter.out_pads[0]);

				DSP_FUNC_1CH_IN_1CH_OUT(&vb_final_filter,vb.out_pads[0],I2S_BUFF_LEN );
				memory_pool_free(dsp_buffers_pool,vb.out_pads[0]);

				/******* end of VB   *********/

				/********* end of LF path    **********/





				/********** HF path *********/

				DSP_FUNC_1CH_IN_1CH_OUT(&hpf_filter_left, app_I2S_spliter.out_pads[0] ,I2S_BUFF_LEN );
				memory_pool_free(dsp_buffers_pool,app_I2S_spliter.out_pads[0]);

				DSP_FUNC_1CH_IN_1CH_OUT(&hpf_filter_right, app_I2S_spliter.out_pads[1] ,I2S_BUFF_LEN );
				memory_pool_free(dsp_buffers_pool,app_I2S_spliter.out_pads[1]);

				DSP_FUNC_2CH_IN_2CH_OUT(&voice_3d,
						hpf_filter_left.out_pads[0],hpf_filter_right.out_pads[0], I2S_BUFF_LEN );
				memory_pool_free(dsp_buffers_pool,hpf_filter_left.out_pads[0]);
				memory_pool_free(dsp_buffers_pool,hpf_filter_right.out_pads[0]);

				/********* end of HF path  *********/


				/********** collecting LF and HF pathes together with phase change on HF *********/
				DSP_FUNC_2CH_IN_1CH_OUT(&adder_bass_with_left_channel,
						voice_3d.out_pads[0], vb_final_filter.out_pads[0], I2S_BUFF_LEN );
				memory_pool_free(dsp_buffers_pool,voice_3d.out_pads[0]);

				DSP_FUNC_2CH_IN_1CH_OUT(&adder_bass_with_right_channel,
						voice_3d.out_pads[1],vb_final_filter.out_pads[0], I2S_BUFF_LEN );
				memory_pool_free(dsp_buffers_pool,voice_3d.out_pads[1]);

				memory_pool_free(dsp_buffers_pool,vb_final_filter.out_pads[0]);


				/********** end of collecting LF and HF pathes together  *********/


				/********** EQ+Compressor  *********/

				DSP_FUNC_1CH_IN_1CH_OUT(&leftChanelEQ, adder_bass_with_left_channel.out_pads[0] ,I2S_BUFF_LEN );
				memory_pool_free(dsp_buffers_pool,adder_bass_with_left_channel.out_pads[0]);

				DSP_FUNC_1CH_IN_1CH_OUT(&rightChanelEQ, adder_bass_with_right_channel.out_pads[0] ,I2S_BUFF_LEN );
				memory_pool_free(dsp_buffers_pool,adder_bass_with_right_channel.out_pads[0]);

				DSP_FUNC_2CH_IN_2CH_OUT(&compressor_limiter,
						leftChanelEQ.out_pads[0],rightChanelEQ.out_pads[0], I2S_BUFF_LEN );
				memory_pool_free(dsp_buffers_pool,leftChanelEQ.out_pads[0]);
				memory_pool_free(dsp_buffers_pool,rightChanelEQ.out_pads[0]);

				/********** end ofEQ+Compressor  *********/


				/********** mix 2 channels to I2S bus  *********/
				DSP_FUNC_2CH_IN_1CH_OUT_NO_OUTPUT_ALLOCATION(&app_I2S_mixer,
						compressor_limiter.out_pads[0],compressor_limiter.out_pads[1] ,
						(float*)pTxBuf,  	I2S_BUFF_LEN );

				memory_pool_free(dsp_buffers_pool,compressor_limiter.out_pads[0] );
				memory_pool_free(dsp_buffers_pool,compressor_limiter.out_pads[1] );

			}//if (1==loopback )


		}

		if (is_timer_elapsed)
		{
			uint8_t cpu_usage_int_part,cpu_usage_res_part;
			uint32_t cpu_usage;
			DEV_IOCTL_1_PARAMS(heartbeat_dev , HEARTBEAT_API_GET_CPU_USAGE , &cpu_usage );
			cpu_usage_int_part = cpu_usage / 1000;
			cpu_usage_res_part = cpu_usage - cpu_usage_int_part;
			PRINTF_DBG("cpu usage = %d.%03d%% \n", cpu_usage_int_part , cpu_usage_res_part);
			DEV_IOCTL(timer_device,TIMER_API_SET_COUNTDOWN_VALUE_AND_REST,&cpu_usage_print_timeout);
		}
		DEV_IOCTL(timer_device, TIMER_API_CHECK_IF_COUNTDOWN_ELAPSED ,  &is_timer_elapsed );

#if ((1==INCLUDE_uxTaskGetStackHighWaterMark ) && (1==OS_FREE_RTOS))
		{
			static  size_t stackLeft,minStackLeft=0xffffffff;

			stackLeft = uxTaskGetStackHighWaterMark( NULL );
			if(minStackLeft > stackLeft)
			{
				minStackLeft = stackLeft;
				PRINTF_DBG("%s   stack left = %d  \r\n" ,
						__FUNCTION__   ,minStackLeft);
			}
		}
#endif
    }
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
	set_band_biquads_t  set_band_biquads;

	set_band_biquads.Gain = 1;

	set_band_biquads.Fc = cutoff_freq;
	set_band_biquads.QValue = 1;//0.836;//0.707;
	set_band_biquads.filter_mode = BIQUADS_BANDPASS_MODE;
	set_band_biquads.band_num = 0;
	DSP_IOCTL_1_PARAMS(&lpf_filter , IOCTL_EQUALIZER_SET_BAND_BIQUADS, &set_band_biquads );
	set_band_biquads.filter_mode = BIQUADS_BANDPASS_MODE;
	set_band_biquads.band_num = 1;
	DSP_IOCTL_1_PARAMS(&lpf_filter , IOCTL_EQUALIZER_SET_BAND_BIQUADS, &set_band_biquads );

	set_band_biquads.Fc = cutoff_freq;
	set_band_biquads.QValue = 1;//0.836;//0.707;
	set_band_biquads.filter_mode = BIQUADS_HIGHPASS_MODE_2_POLES;
	set_band_biquads.band_num = 0;
	DSP_IOCTL_1_PARAMS(&hpf_filter_left , IOCTL_EQUALIZER_SET_BAND_BIQUADS, &set_band_biquads );
	set_band_biquads.filter_mode = BIQUADS_HIGHPASS_MODE_1_POLE;
	set_band_biquads.band_num = 1;
	DSP_IOCTL_1_PARAMS(&hpf_filter_left , IOCTL_EQUALIZER_SET_BAND_BIQUADS, &set_band_biquads );

	set_band_biquads.filter_mode = BIQUADS_HIGHPASS_MODE_2_POLES;
	set_band_biquads.band_num = 0;
	DSP_IOCTL_1_PARAMS(&hpf_filter_right , IOCTL_EQUALIZER_SET_BAND_BIQUADS, &set_band_biquads );
	set_band_biquads.filter_mode = BIQUADS_HIGHPASS_MODE_1_POLE;
	set_band_biquads.band_num = 1;
	DSP_IOCTL_1_PARAMS(&hpf_filter_right , IOCTL_EQUALIZER_SET_BAND_BIQUADS, &set_band_biquads );

	set_band_biquads.QValue = 1;//0.836;//0.707;
	set_band_biquads.Gain = 1;
	set_band_biquads.Fc = cutoff_freq * 3.4;
	set_band_biquads.filter_mode = BIQUADS_LOWPASS_MODE_2_POLES;
	set_band_biquads.band_num = 0;
	DSP_IOCTL_1_PARAMS(&vb_final_filter , IOCTL_EQUALIZER_SET_BAND_BIQUADS, &set_band_biquads );
	set_band_biquads.Fc = cutoff_freq * 0.2;
	set_band_biquads.filter_mode = BIQUADS_HIGHPASS_MODE_1_POLE;
	set_band_biquads.band_num = 1;
	DSP_IOCTL_1_PARAMS(&vb_final_filter , IOCTL_EQUALIZER_SET_BAND_BIQUADS, &set_band_biquads );
	set_band_biquads.QValue = 1;//0.836;//0.707;
	set_band_biquads.filter_mode = BIQUADS_HIGHPASS_MODE_2_POLES;
	set_band_biquads.band_num = 2;
	DSP_IOCTL_1_PARAMS(&vb_final_filter , IOCTL_EQUALIZER_SET_BAND_BIQUADS, &set_band_biquads );


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
uint8_t app_dev_ioctl( void * const aHandle ,const uint8_t aIoctl_num
		, void * aIoctl_param1 , void * aIoctl_param2)
{
	float threshold ;
	float gain ;
	set_channel_weight_t ch_weight;
	uint8_t retVal;
	switch(aIoctl_num)
	{
		case IOCTL_DEVICE_START :

			/* Create an application thread */

			dsp_buffers_pool = memory_pool_init(5 , I2S_BUFF_LEN * sizeof(float));
			dsp_managment_api_set_buffers_pool(dsp_buffers_pool);

			/**************  I2S splitter and mixer  *************/
			retVal = I2S_splitter_api_init_dsp_descriptor(&app_I2S_spliter);
			if(retVal) while(1);
			DSP_IOCTL_0_PARAMS(&app_I2S_spliter , IOCTL_DEVICE_START );

			retVal = I2S_mixer_api_init_dsp_descriptor(&app_I2S_mixer);
			if(retVal) while(1);
			DSP_IOCTL_0_PARAMS(&app_I2S_mixer , IOCTL_DEVICE_START );



			/**************   mixers  *************/
			retVal = mixer_api_init_dsp_descriptor(&stereo_to_mono);
			if(retVal) while(1);
			DSP_IOCTL_1_PARAMS(&stereo_to_mono , IOCTL_MIXER_SET_NUM_OF_CHANNELS , ((void*)2) );
			ch_weight.channel_num = 0;
			ch_weight.weight = 0.5;
			DSP_IOCTL_1_PARAMS(&stereo_to_mono , IOCTL_MIXER_SET_CHANNEL_WEIGHT , &ch_weight  );
			ch_weight.channel_num = 1;
			ch_weight.weight = 0.5;
			DSP_IOCTL_1_PARAMS(&stereo_to_mono , IOCTL_MIXER_SET_CHANNEL_WEIGHT , &ch_weight  );
			DSP_IOCTL_0_PARAMS(&stereo_to_mono , IOCTL_DEVICE_START );

			retVal = mixer_api_init_dsp_descriptor(&adder_bass_with_left_channel);
			if(retVal) while(1);
			DSP_IOCTL_1_PARAMS(&adder_bass_with_left_channel , IOCTL_MIXER_SET_NUM_OF_CHANNELS , ((void*)2) );
			ch_weight.channel_num = 0;
			ch_weight.weight = 1;
			DSP_IOCTL_1_PARAMS(&adder_bass_with_left_channel , IOCTL_MIXER_SET_CHANNEL_WEIGHT , &ch_weight  );
			ch_weight.channel_num = 1;
			ch_weight.weight = 1;
			DSP_IOCTL_1_PARAMS(&adder_bass_with_left_channel , IOCTL_MIXER_SET_CHANNEL_WEIGHT , &ch_weight  );
			DSP_IOCTL_0_PARAMS(&adder_bass_with_left_channel , IOCTL_DEVICE_START );

			retVal = mixer_api_init_dsp_descriptor(&adder_bass_with_right_channel);
			if(retVal) while(1);
			DSP_IOCTL_1_PARAMS(&adder_bass_with_right_channel , IOCTL_MIXER_SET_NUM_OF_CHANNELS , ((void*)2) );
			ch_weight.channel_num = 0;
			ch_weight.weight = 1;
			DSP_IOCTL_1_PARAMS(&adder_bass_with_right_channel , IOCTL_MIXER_SET_CHANNEL_WEIGHT , &ch_weight  );
			ch_weight.channel_num = 1;
			ch_weight.weight = 1;
			DSP_IOCTL_1_PARAMS(&adder_bass_with_right_channel , IOCTL_MIXER_SET_CHANNEL_WEIGHT , &ch_weight  );
			DSP_IOCTL_0_PARAMS(&adder_bass_with_right_channel , IOCTL_DEVICE_START );


			retVal = equalizer_api_init_dsp_descriptor(&lpf_filter);
			if(retVal) while(1);
			DSP_IOCTL_1_PARAMS(&lpf_filter , IOCTL_EQUALIZER_SET_NUM_OF_BANDS , ((void*)(size_t)2) );
			DSP_IOCTL_0_PARAMS(&lpf_filter , IOCTL_DEVICE_START );


			retVal = equalizer_api_init_dsp_descriptor(&hpf_filter_left);
			if(retVal) while(1);
			DSP_IOCTL_1_PARAMS(&hpf_filter_left , IOCTL_EQUALIZER_SET_NUM_OF_BANDS , ((void*)(size_t)2) );
			DSP_IOCTL_0_PARAMS(&hpf_filter_left , IOCTL_DEVICE_START );

			retVal = equalizer_api_init_dsp_descriptor(&hpf_filter_right);
			if(retVal) while(1);
			DSP_IOCTL_1_PARAMS(&hpf_filter_right , IOCTL_EQUALIZER_SET_NUM_OF_BANDS , ((void*)(size_t)2) );
			DSP_IOCTL_0_PARAMS(&hpf_filter_right , IOCTL_DEVICE_START );



			/************  vb filters ***********/
//			set_band_biquads.Fc = 78.5;
//			set_band_biquads.QValue = 0.707;//0.836;//0.7;
//			set_band_biquads.Gain = 1;
//			retVal = equalizer_api_init_dsp_descriptor(&vb_hpf_filter);
//			if(retVal) while(1);
//			DSP_IOCTL_1_PARAMS(&vb_hpf_filter , IOCTL_EQUALIZER_SET_NUM_OF_BANDS , ((void*)(size_t)4) );
//			set_band_biquads.filter_mode = BIQUADS_HIGHPASS_MODE_2_POLES;
//			set_band_biquads.band_num = 0;
//			DSP_IOCTL_1_PARAMS(&vb_hpf_filter , IOCTL_EQUALIZER_SET_BAND_BIQUADS, &set_band_biquads );
//			set_band_biquads.band_num = 1;
//			DSP_IOCTL_1_PARAMS(&vb_hpf_filter , IOCTL_EQUALIZER_SET_BAND_BIQUADS, &set_band_biquads );
//			set_band_biquads.band_num = 2;
//			DSP_IOCTL_1_PARAMS(&vb_hpf_filter , IOCTL_EQUALIZER_SET_BAND_BIQUADS, &set_band_biquads );
//			set_band_biquads.band_num = 3;
//			DSP_IOCTL_1_PARAMS(&vb_hpf_filter , IOCTL_EQUALIZER_SET_BAND_BIQUADS, &set_band_biquads );
//			DSP_IOCTL_0_PARAMS(&vb_hpf_filter , IOCTL_DEVICE_START );



			retVal = virtual_bass_api_init_dsp_descriptor(&vb);
			if(retVal) while(1);
			DSP_IOCTL_0_PARAMS(&vb , IOCTL_DEVICE_START );
			dsp_managment_api_set_module_control(&vb , DSP_MANAGMENT_API_MODULE_CONTROL_BYPASS);

			retVal = equalizer_api_init_dsp_descriptor(&vb_final_filter);
			if(retVal) while(1);
			DSP_IOCTL_1_PARAMS(&vb_final_filter , IOCTL_EQUALIZER_SET_NUM_OF_BANDS , ((void*)(size_t)3) );
			DSP_IOCTL_0_PARAMS(&vb_final_filter , IOCTL_DEVICE_START );
			dsp_managment_api_set_module_control(&vb_final_filter , DSP_MANAGMENT_API_MODULE_CONTROL_BYPASS);
			app_dev_set_cuttof();

			/**************   eq filters  *************/
			retVal = equalizer_api_init_dsp_descriptor(&leftChanelEQ);
			if(retVal) while(1);
			DSP_IOCTL_1_PARAMS(&leftChanelEQ , IOCTL_EQUALIZER_SET_NUM_OF_BANDS , ((void*)(size_t)7) );
			DSP_IOCTL_0_PARAMS(&leftChanelEQ , IOCTL_DEVICE_START );
			equalizer_api_init_dsp_descriptor(&rightChanelEQ);
			DSP_IOCTL_1_PARAMS(&rightChanelEQ , IOCTL_EQUALIZER_SET_NUM_OF_BANDS , ((void*)(size_t)7) );
			DSP_IOCTL_0_PARAMS(&rightChanelEQ , IOCTL_DEVICE_START );
#if 0// look_ahead_compressor
			retVal = compressor_api_init_dsp_descriptor(&compressor_limiter);
			if(retVal) while(1);
			DSP_IOCTL_1_PARAMS(&compressor_limiter , IOCTL_COMPRESSOR_SET_TYPE , ((void*)(size_t)COMPRESSOR_API_TYPE_LOOKAHEAD) );
			threshold=0.8;
			DSP_IOCTL_1_PARAMS(&compressor_limiter , IOCTL_COMPRESSOR_SET_HIGH_THRESHOLD , &threshold );
			DSP_IOCTL_1_PARAMS(&compressor_limiter , IOCTL_COMPRESSOR_SET_LATENCY , ((void*)(size_t)LATENCY_LENGTH) );
#else
			{
				float attack,release,ratio;
				retVal = compressor_api_init_dsp_descriptor(&compressor_limiter);
				if(retVal) while(1);
				DSP_IOCTL_1_PARAMS(&compressor_limiter , IOCTL_COMPRESSOR_SET_TYPE , ((void*)(size_t)COMPRESSOR_API_TYPE_REGULAR) );
				threshold=0.125;
				DSP_IOCTL_1_PARAMS(&compressor_limiter , IOCTL_COMPRESSOR_SET_HIGH_THRESHOLD , &threshold );
				attack = 0.5;
				DSP_IOCTL_1_PARAMS(&compressor_limiter , IOCTL_COMPRESSOR_SET_ATTACK , &attack );
				release = 0.5;
				DSP_IOCTL_1_PARAMS(&compressor_limiter , IOCTL_COMPRESSOR_SET_RELEASE , &release );
				ratio = 2;//[1,inf]
				DSP_IOCTL_1_PARAMS(&compressor_limiter , IOCTL_COMPRESSOR_SET_RATIO , &ratio );
			}
#endif
			DSP_IOCTL_0_PARAMS(&compressor_limiter , IOCTL_DEVICE_START );
			dsp_managment_api_set_module_control(&compressor_limiter , DSP_MANAGMENT_API_MODULE_CONTROL_BYPASS);

			retVal = voice_3D_api_init_dsp_descriptor(&voice_3d);
			if(retVal) while(1);
			gain=0.5;
			DSP_IOCTL_1_PARAMS(&voice_3d , IOCTL_VOICE_3D_SET_MEDIUM_GAIN , &gain );
			gain = 0.5;
			DSP_IOCTL_1_PARAMS(&voice_3d , IOCTL_VOICE_3D_SET_SIDE_GAIN , &gain );
			gain = 0 ;
			DSP_IOCTL_1_PARAMS(&voice_3d , IOCTL_VOICE_3D_SET_3D_GAIN , &gain );

			DSP_IOCTL_0_PARAMS(&voice_3d , IOCTL_DEVICE_START );

//			retVal = compressor_api_init_dsp_descriptor(&vb_limiter);
//			if(retVal) while(1);
//			DSP_IOCTL_1_PARAMS(&vb_limiter , IOCTL_COMPRESSOR_SET_HIGH_THRESHOLD , ((void*)(size_t)(0.8)) );
//			DSP_IOCTL_0_PARAMS(&vb_limiter , IOCTL_DEVICE_START );

			os_create_task("main" , main_thread_func, 0, MAIN_STACK_SIZE_BYTES , APP_DEV_THREAD_PRIORITY);

			break;
		default :
			return 1;
	}
	return 0;
}
