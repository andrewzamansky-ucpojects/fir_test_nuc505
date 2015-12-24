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
#include "I2S_nuc505_api.h"
#include "I2S_splitter_api.h"
#include "I2S_mixer_api.h"
#include "timer_api.h"
#include "heartbeat_api.h"
#include "math.h"
#include "memory_pool_api.h"

/********  defines *********************/
#if (2 == NUM_OF_BYTES_PER_AUDIO_WORD)
	#define	FLOAT_NORMALIZER	0x7fff
	typedef int16_t	buffer_type_t	;
#endif
#if (4 == NUM_OF_BYTES_PER_AUDIO_WORD)
	#define	FLOAT_NORMALIZER	0x7fffffff
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
dsp_descriptor_t vb_hpf_filter;
dsp_descriptor_t vb_final_filter;


dsp_descriptor_t hpf_filter_left;
dsp_descriptor_t hpf_filter_right;

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

float compressor_ratio = 0;
float cutoff_freq = 100;

TIMER_API_CREATE_STATIC_DEV(app_timer_inst,"apptim" ,systick_dev_inst );
static pdev_descriptor_const  timer_device = &app_timer_inst;

uint8_t vb_on = 0;
uint8_t loopback = 0 ;
uint8_t lf_path = 1 ;
uint8_t hf_path = 1 ;



//#define COMPR_ATTACK	0.998609f
//#define COMPR_REALESE	 0.987032f
float COMPR_ATTACK =	0.0001f;
float COMPR_REALESE	= 0.9999f;

#define g1	15.0f
#define g2	1.0f
#define b1	(g2/g1)
#define B	(b1/(1-b1))
#define A	(g1*B)

#define ENVELOP_FOLLOWER_SAMPLE_RATE	1
#define ENVELOP_FOLLOWER_SAMPLE_RATE_FLOAT	1.0f
//#define COMPR_ATTACK_ADJUSTED	(0.0624130625)
//#define COMPR_REALESE_ADJUSTED	(0.0616895)
#define COMPR_ATTACK_ADJUSTED	(COMPR_ATTACK  / ENVELOP_FOLLOWER_SAMPLE_RATE_FLOAT)
#define COMPR_REALESE_ADJUSTED	(COMPR_REALESE / ENVELOP_FOLLOWER_SAMPLE_RATE_FLOAT)

#define PRE_CLU_GAIN	0.01f
#define POST_CLU_GAIN	1.2f
//#define HARMONICS_GAIN	2.0
#define HU1_A	0.0125f
#define HU1_B	0.08215f
#define HU1_A_ADJUSTED	(HU1_A  / ENVELOP_FOLLOWER_SAMPLE_RATE_FLOAT)
#define HU1_B_ADJUSTED	(HU1_B / ENVELOP_FOLLOWER_SAMPLE_RATE_FLOAT)

float vb_volume = 0.3315;// 30 * 0.85*1.3
float HARMONICS_GAIN = 1.1 ;

static uint8_t s_flag=0;

/*---------------------------------------------------------------------------------------------------------*/
/* Function:        vb_dsp                                                                          */
/*                                                                                                         */
/* Parameters:                                                                                             */
/*                                                                                         */
/*                                                                                                  */
/* Returns:                                                                                      */
/* Side effects:                                                                                           */
/* Description:                                                                                            */
/*                                                            						 */
/*---------------------------------------------------------------------------------------------------------*/
void vb_dsp(const void * const aHandle ,
		uint8_t num_of_inputs,uint8_t num_of_ouputs, size_t data_len ,
		float *apCh1In , float *apCh2In,
		float *apCh1Out , float *apCh2Out)
{

	float curr_ratio = 1;
	uint16_t i;
//	float threshold = 0.01;// * ((uint16_t)0x7fff) ;
//	float reverse_ratio = 0.5;
	uint8_t envelope_folower_sample_count =ENVELOP_FOLLOWER_SAMPLE_RATE;
	static float saved_yn1=0,saved_y1n1=0,saved_xn1=0;
	static float saved_harmonic_outn1=0;
	float yn1,y1n1;
	float xn1;
	float harmonic_outn1;


	float curr_y,curr_x ;

	yn1 = saved_yn1;
	xn1 = saved_xn1;
	y1n1 = saved_y1n1;
	harmonic_outn1 = saved_harmonic_outn1;


	for(i = 0 ; i < data_len ; i++)
	{
		float tmp,tmp1,abs_result;


		curr_x = apCh1In[i];//  / 0x7fff ;
//		curr_x = PRE_CLU_GAIN * curr_x;
		curr_x = (-2.2f) * curr_x;

//		if(ENVELOP_FOLLOWER_SAMPLE_RATE == envelope_folower_sample_count )
//		{
			abs_result = fabs(curr_x);
			tmp = abs_result;
			tmp1 = COMPR_REALESE_ADJUSTED * y1n1 + (1 - COMPR_REALESE_ADJUSTED) * tmp;
			if(tmp < tmp1)
			{
				tmp = tmp1;
			}
			y1n1 = tmp;

			tmp1 = (1 - COMPR_ATTACK_ADJUSTED) * tmp;
			tmp =( COMPR_ATTACK_ADJUSTED * yn1) ;//+ ((1 - COMPR_ATTACK) * tmp);
			tmp = tmp + tmp1;

			yn1=tmp;

			tmp = tmp;// / 0x7fff;
			curr_ratio = A/(B+tmp);
			envelope_folower_sample_count = 1;

			// harmonic creations
#if 1
			if(curr_x < harmonic_outn1  )
			{
				tmp = HU1_B_ADJUSTED;
			}
			else
			{
				tmp = HU1_A_ADJUSTED;
			}
			harmonic_outn1 = (1-tmp)*harmonic_outn1;
			harmonic_outn1 += tmp * curr_x ;
#else
			if(curr_x > xn1)
			{
				harmonic_outn1 = curr_x;
			}
			xn1 = curr_x;
#endif
//		}
//		envelope_folower_sample_count++;

		curr_y = curr_ratio * curr_x ;
//		curr_y = curr_y * POST_CLU_GAIN;
		tmp = HARMONICS_GAIN * harmonic_outn1;
		curr_y += tmp ;
		curr_y = curr_y * vb_volume ;
		apCh1Out[ i ] =   curr_y;//  * 0x7fff;


	}

	saved_harmonic_outn1 = harmonic_outn1 ;
	saved_xn1 =xn1 ;
	saved_yn1 =yn1 ;
	saved_y1n1 =y1n1 ;


}


/*---------------------------------------------------------------------------------------------------------*/
/* Function:        my_float_memcpy                                                                          */
// on gcc cortex-m3 MEMCPY() is slower then direct copy !!!
/*---------------------------------------------------------------------------------------------------------*/
void my_float_memcpy(float *dest ,float *src , size_t len)
{
	for( ; len ;len--)
	{
		*dest++ = *src++;
	}
}

/*---------------------------------------------------------------------------------------------------------*/
/* Function:        my_float_memcpy                                                                          */
// on gcc cortex-m3 MEMCPY() is slower then direct copy !!!
/*---------------------------------------------------------------------------------------------------------*/
void my_float_memcpy_2_buffers(float *dest1 ,float *src1 ,float *dest2 ,float *src2, size_t len)
{
	for( ; len ;len--)
	{
		*dest1++ = *src1++;
		*dest2++ = *src2++;
	}
}

/*---------------------------------------------------------------------------------------------------------*/
/* Function:        my_float_memset                                                                          */
// on gcc cortex-m3 MEMCPY() is slower then direct copy !!!
/*---------------------------------------------------------------------------------------------------------*/
void my_float_memset(float *dest ,float val , size_t len)
{
	for( ; len ;len--)
	{
		*dest++ = val;
	}
}



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


#define HIGH_THRESHOLD_VALUE	0.8

static float *vb_dsp_Pad_Out_0;


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

				if(2 == loopback)//transparent mode
				{
					 adder_bass_with_left_channel.out_pads[0] = app_I2S_spliter.out_pads[0];
					 adder_bass_with_right_channel.out_pads[0] = app_I2S_spliter.out_pads[1];
				}
				else
				{

	#if 1 // with X.O



					/******** LF path **********/
					if(lf_path)
					{
						DSP_FUNC_2CH_IN_1CH_OUT(&stereo_to_mono,
								app_I2S_spliter.out_pads[0],app_I2S_spliter.out_pads[1],  I2S_BUFF_LEN );

						DSP_FUNC_1CH_IN_1CH_OUT(&lpf_filter,stereo_to_mono.out_pads[0] ,I2S_BUFF_LEN );
						memory_pool_free(dsp_buffers_pool,stereo_to_mono.out_pads[0]);

						/********  VB  ************/
						if (vb_on)
						{
							vb_dsp_Pad_Out_0 = (float*)memory_pool_malloc(dsp_buffers_pool);
	#if 1
							vb_dsp(NULL ,  1, 1 , I2S_BUFF_LEN ,
									lpf_filter.out_pads[0] ,NULL,
									vb_dsp_Pad_Out_0  ,NULL);

							memory_pool_free(dsp_buffers_pool,lpf_filter.out_pads[0]);
	#else
							vb_dsp_Pad_Out_0 = stereo_to_mono.out_pads[0] ;
	#endif

							DSP_FUNC_1CH_IN_1CH_OUT(&vb_final_filter,vb_dsp_Pad_Out_0,I2S_BUFF_LEN );
							memory_pool_free(dsp_buffers_pool,vb_dsp_Pad_Out_0);
						}
						else
						{
							vb_final_filter.out_pads[0] = lpf_filter.out_pads[0];
						}
						/******* end of VB   *********/
					}
					else
					{
						vb_final_filter.out_pads[0] = (float*)memory_pool_malloc(dsp_buffers_pool);
						my_float_memset(vb_final_filter.out_pads[0] , 0 , I2S_BUFF_LEN );
					}
					/********* end of LF path    **********/

					/********** HF path *********/

					if(hf_path)
					{
						DSP_FUNC_1CH_IN_1CH_OUT(&hpf_filter_left, app_I2S_spliter.out_pads[0] ,I2S_BUFF_LEN );
						memory_pool_free(dsp_buffers_pool,app_I2S_spliter.out_pads[0]);

						DSP_FUNC_1CH_IN_1CH_OUT(&hpf_filter_right, app_I2S_spliter.out_pads[1] ,I2S_BUFF_LEN );
						memory_pool_free(dsp_buffers_pool,app_I2S_spliter.out_pads[1]);

					}
					else
					{
						hpf_filter_left.out_pads[0] = app_I2S_spliter.out_pads[0];
						my_float_memset(hpf_filter_left.out_pads[0] , 0 , I2S_BUFF_LEN  );

						hpf_filter_right.out_pads[0] = app_I2S_spliter.out_pads[1];
						my_float_memset(hpf_filter_right.out_pads[0] , 0 , I2S_BUFF_LEN  );
					}


					/********* end of HF path  *********/


					/********** collecting LF and HF pathes together with phase change on HF *********/
					DSP_FUNC_2CH_IN_1CH_OUT(&adder_bass_with_left_channel,
							hpf_filter_left.out_pads[0], vb_final_filter.out_pads[0], I2S_BUFF_LEN );
					memory_pool_free(dsp_buffers_pool,hpf_filter_left.out_pads[0]);

					DSP_FUNC_2CH_IN_1CH_OUT(&adder_bass_with_right_channel,
							hpf_filter_right.out_pads[0],vb_final_filter.out_pads[0], I2S_BUFF_LEN );
					memory_pool_free(dsp_buffers_pool,hpf_filter_right.out_pads[0]);

					memory_pool_free(dsp_buffers_pool,vb_final_filter.out_pads[0]);


					/********** end of collecting LF and HF pathes together  *********/



		#else  // without X.O
					adder_bass_with_left_channel_Pad_Out_0 = app_I2S_spliter.out_pads[0];
					adder_bass_with_right_channel_Pad_Out_0 = app_I2S_spliter.out_pads[1];
		#endif //   X.O

				}//if (2==loopback )

				DSP_FUNC_1CH_IN_1CH_OUT(&leftChanelEQ, adder_bass_with_left_channel.out_pads[0] ,I2S_BUFF_LEN );
				memory_pool_free(dsp_buffers_pool,adder_bass_with_left_channel.out_pads[0]);

				DSP_FUNC_1CH_IN_1CH_OUT(&rightChanelEQ, adder_bass_with_right_channel.out_pads[0] ,I2S_BUFF_LEN );
				memory_pool_free(dsp_buffers_pool,adder_bass_with_right_channel.out_pads[0]);

				if((0 != compressor_ratio) /*&& (0==loopback)*/)
				{
					DSP_FUNC_2CH_IN_2CH_OUT(&compressor_limiter,
							leftChanelEQ.out_pads[0],rightChanelEQ.out_pads[0], I2S_BUFF_LEN );
					memory_pool_free(dsp_buffers_pool,leftChanelEQ.out_pads[0]);
					memory_pool_free(dsp_buffers_pool,rightChanelEQ.out_pads[0]);


				}
				else
				{
					compressor_limiter.out_pads[0] = leftChanelEQ.out_pads[0] ;
					compressor_limiter.out_pads[1] = rightChanelEQ.out_pads[0];
				}



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
	float threshold = HIGH_THRESHOLD_VALUE;
	set_channel_weight_t ch_weight;
	uint8_t retVal;
	switch(aIoctl_num)
	{
		case IOCTL_DEVICE_START :

			/* Create an application thread */

			dsp_buffers_pool = memory_pool_init(4 , I2S_BUFF_LEN * sizeof(float));

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




			retVal = equalizer_api_init_dsp_descriptor(&vb_final_filter);
			if(retVal) while(1);
			DSP_IOCTL_1_PARAMS(&vb_final_filter , IOCTL_EQUALIZER_SET_NUM_OF_BANDS , ((void*)(size_t)3) );
			DSP_IOCTL_0_PARAMS(&vb_final_filter , IOCTL_DEVICE_START );
			app_dev_set_cuttof();

			/**************   eq filters  *************/
			retVal = equalizer_api_init_dsp_descriptor(&leftChanelEQ);
			if(retVal) while(1);
			DSP_IOCTL_1_PARAMS(&leftChanelEQ , IOCTL_EQUALIZER_SET_NUM_OF_BANDS , ((void*)(size_t)7) );
			DSP_IOCTL_0_PARAMS(&leftChanelEQ , IOCTL_DEVICE_START );
			equalizer_api_init_dsp_descriptor(&rightChanelEQ);
			DSP_IOCTL_1_PARAMS(&rightChanelEQ , IOCTL_EQUALIZER_SET_NUM_OF_BANDS , ((void*)(size_t)7) );
			DSP_IOCTL_0_PARAMS(&rightChanelEQ , IOCTL_DEVICE_START );

			retVal = compressor_api_init_dsp_descriptor(&compressor_limiter);
			if(retVal) while(1);
			DSP_IOCTL_1_PARAMS(&compressor_limiter , IOCTL_COMPRESSOR_SET_HIGH_THRESHOLD , &threshold );
			DSP_IOCTL_1_PARAMS(&compressor_limiter , IOCTL_COMPRESSOR_SET_LATENCY , ((void*)(size_t)LATENCY_LENGTH) );
			DSP_IOCTL_0_PARAMS(&compressor_limiter , IOCTL_DEVICE_START );

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
