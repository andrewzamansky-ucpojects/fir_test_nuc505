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
#include "compressor_api.h"
#include "I2S_nuc505_api.h"
#include "timer_api.h"
#include "heartbeat_api.h"

/********  defines *********************/



/********  types  *********************/

/********  externals *********************/



/********  local defs *********************/



/********  types  *********************/


typedef struct
{

	int16_t *rxBuffer;
	int16_t *txBuffer;

} xMessage_t;


os_queue_t xQueue=NULL ;

dsp_descriptor_t lpf_filter;
dsp_descriptor_t vb_hpf_filter;
dsp_descriptor_t vb_final_filter;


dsp_descriptor_t hpf_filter_left;
dsp_descriptor_t hpf_filter_right;

dsp_descriptor_t leftChanelEQ;
dsp_descriptor_t rightChanelEQ;

dsp_descriptor_t compressor_limiter;
dsp_descriptor_t vb_limiter;

extern pdev_descriptor_const i2s_dev;
extern pdev_descriptor_const heartbeat_dev ;
float leftChData_step_0[I2S_BUFF_LEN + LATENCY_LENGTH]  ={0};
float rightChData_step_0[I2S_BUFF_LEN + LATENCY_LENGTH]	={0};
float leftChData_step_1[I2S_BUFF_LEN + LATENCY_LENGTH]	={0};
float rightChData_step_1[I2S_BUFF_LEN + LATENCY_LENGTH]	={0};
float leftChData_step_2[I2S_BUFF_LEN + LATENCY_LENGTH]	={0};
float rightChData_step_2[I2S_BUFF_LEN + LATENCY_LENGTH]	={0};

float VB_data_out[I2S_BUFF_LEN + LATENCY_LENGTH] ={0}	;



float leftChData_orig_prev[LATENCY_LENGTH]	={0};
float rightChData_orig_prev[LATENCY_LENGTH] 	={0};

float leftChData_hf_prev[LATENCY_LENGTH]	={0};
float rightChData_hf_prev[LATENCY_LENGTH] ={0}	;

float compressor_ratio = 0;

TIMER_API_CREATE_STATIC_DEV(app_timer_inst,"httpt" ,systick_dev_inst );
static pdev_descriptor_const  timer_device = &app_timer_inst;

uint8_t vb_on = 0;
uint8_t loopback = 0 ;
uint8_t lf_path = 1 ;
uint8_t hf_path = 1 ;
/*---------------------------------------------------------------------------------------------------------*/
/* Function:        my_float_memcpy                                                                          */
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
/*---------------------------------------------------------------------------------------------------------*/
void my_float_memset(float *dest ,float val , size_t len)
{
	for( ; len ;len--)
	{
		*dest++ = val;
	}
}



#define COMPR_ATTACK	0.998609f
#define COMPR_REALESE	 0.987032f

#define g1	15.0f
#define g2	1.0f
#define b1	(g2/g1)
#define B	(b1/(1-b1))
#define A	(g1*B)

#define ENVELOP_FOLLOWER_SAMPLE_RATE	16
#define COMPR_ATTACK_ADJUSTED	(0.0624130625)
#define COMPR_REALESE_ADJUSTED	(0.0616895)
//#define COMPR_ATTACK_ADJUSTED	(COMPR_ATTACK  / ENVELOP_FOLLOWER_SAMPLE_RATE)
//#define COMPR_REALESE_ADJUSTED	(COMPR_REALESE / ENVELOP_FOLLOWER_SAMPLE_RATE)

#define POST_CLU_GAIN	1
#define HARMONICS_GAIN	2.0

float vb_volume = -1;

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

	static int print_count=0;
	float max_val ;
//	static float prev_ratio = 1;
	float curr_ratio = 1;
//	static int threshold_detectd = 0;
//	uint32_t accomulator=0;
	uint8_t usePreviousRatio;
	uint16_t i,j,k;
//	float threshold = INSTANCE(aHandle)->threshold;
//	float reverse_ratio = INSTANCE(aHandle)->reverse_ratio;
	float threshold = 0.01;// * ((uint16_t)0x7fff) ;
	float reverse_ratio = 0.5;
	uint8_t envelope_folower_sample_count =ENVELOP_FOLLOWER_SAMPLE_RATE;

	static float saved_yn1=0,saved_y1n1=0;
	float yn1,y1n1;
	float y1;

	static float saved_prev_y =0;
	float prev_y;
	float curr_y ;

	yn1 = saved_yn1;
	y1n1=saved_y1n1;
	prev_y = saved_prev_y;
	max_val = threshold ;
	j = 0;
	usePreviousRatio = 1;
	for(i = 0 ; i < data_len ; i++)
	{
		float tmp,tmp1,abs_result;


		curr_y = apCh1In[i]/ 0x7fff ;
//		curr_y = 6 * curr_y;

		abs_result = fabs(curr_y);
		if(ENVELOP_FOLLOWER_SAMPLE_RATE == envelope_folower_sample_count )
		{
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
			curr_ratio = A/(B+tmp);
			envelope_folower_sample_count = 1;
		}
		envelope_folower_sample_count++;

		curr_y = curr_ratio * curr_y ;
//		curr_y = curr_y * POST_CLU_GAIN;
		abs_result = HARMONICS_GAIN * abs_result;
		curr_y += abs_result ;
		curr_y = curr_y * vb_volume;
		apCh1Out[ i ] =   curr_y * 0x7fff;


	}

	saved_prev_y = prev_y ;
	saved_yn1 =yn1 ;
	saved_y1n1 = y1n1;


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


#define HIGH_THRESHOLD_VALUE	0x4000


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
	uint16_t i,j;
	int16_t *pRxBuf;
	int16_t *pTxBuf;
	float *pTmpBuf1,*pTmpBuf2,*pTmpBuf3;
	uint8_t is_timer_elapsed=1;
	uint32_t retVal;
	xQueue = os_create_queue( APP_DEV_CONFIG_MAX_QUEUE_LEN , sizeof(xMessage_t ) );
	uint64_t cpu_usage_print_timeout = 1000;

    while (1)
    {
    	retVal = os_queue_receive_with_timeout( xQueue , &( xRxMessage ) , 1000 );
		if( OS_QUEUE_RECEIVE_SUCCESS ==  retVal)
		{


			pRxBuf = xRxMessage.rxBuffer;
			pTxBuf = xRxMessage.txBuffer;

			if(loopback)//transparent mode
			{
				for(i = 0 ; i < I2S_BUFF_LEN ; i++)
				{
					pTxBuf[2*i] =  pRxBuf[2*i ];
					pTxBuf[2*i + 1] =  pRxBuf[2*i + 1];

				}

			}
			else
			{
				// on gcc cortex-m3 MEMCPY() is slower then direct copy !!!
				//******* restore previous last chunk as current first chunk
				my_float_memcpy_2_buffers( leftChData_step_0 , leftChData_orig_prev,
						rightChData_step_0 , rightChData_orig_prev , LATENCY_LENGTH);

				//******* distribute L-R
				pTmpBuf1 = &leftChData_step_0[LATENCY_LENGTH];
				pTmpBuf2 = &rightChData_step_0[LATENCY_LENGTH];
				for(i = LATENCY_LENGTH ; i < (I2S_BUFF_LEN + LATENCY_LENGTH) ; i++)
				{
					*pTmpBuf1++ = (float) (*pRxBuf++);//pRxBuf[2*j ];
					*pTmpBuf2++ = (float) (*pRxBuf++);//pRxBuf[2*j + 1];
	//				leftChData_step_0[i] = (float)pRxBuf[2*j ];//(*pRxBuf++);//pRxBuf[2*j ];
	//				rightChData_step_0[i] = (float)pRxBuf[2*j + 1];//(*pRxBuf++);//pRxBuf[2*j + 1];
				}

				//******* store last chunk for latency
				my_float_memcpy_2_buffers( leftChData_orig_prev , &leftChData_step_0[I2S_BUFF_LEN],
						rightChData_orig_prev , &rightChData_step_0[I2S_BUFF_LEN] , LATENCY_LENGTH);


	#if 1 // with X.O



				/******** LF path **********/
				if(lf_path)
				{
					pTmpBuf1 = leftChData_step_1 ;
					pTmpBuf2 = leftChData_step_0 ;
					pTmpBuf3 = rightChData_step_0 ;
					for(i = 0 ; i < (I2S_BUFF_LEN  + LATENCY_LENGTH); i++)
					{
						*pTmpBuf1++ = ((*pTmpBuf2++) +(*pTmpBuf3++))/2;
					}

					DSP_FUNC_1CH_IN_1CH_OUT(&lpf_filter,&leftChData_step_1[LATENCY_LENGTH],&leftChData_step_2[LATENCY_LENGTH],I2S_BUFF_LEN );

					/********  VB  ************/
					if (vb_on)
					{

						vb_dsp(NULL ,  1, 1 , I2S_BUFF_LEN ,
								&leftChData_step_2[LATENCY_LENGTH],NULL,
								&leftChData_step_1[LATENCY_LENGTH] ,NULL);

	//					DSP_FUNC_1CH_IN_1CH_OUT(&vb_limiter,leftChData_step_1,leftChData_step_2,
	//							 I2S_BUFF_LEN + LATENCY_LENGTH);

	//					DSP_FUNC_1CH_IN_1CH_OUT(&vb_hpf_filter,&leftChData_step_2[LATENCY_LENGTH],&leftChData_step_1[LATENCY_LENGTH],I2S_BUFF_LEN );

						DSP_FUNC_1CH_IN_1CH_OUT(&vb_final_filter,&leftChData_step_1[LATENCY_LENGTH],&leftChData_step_2[LATENCY_LENGTH],I2S_BUFF_LEN );
					}
					else
					{
						pTmpBuf1 = leftChData_step_2 ;
						pTmpBuf2 = leftChData_step_2 ;
						for(i = 0 ; i < (I2S_BUFF_LEN  + LATENCY_LENGTH); i++)
						{
							*pTmpBuf1++ = (*pTmpBuf2++) ;
						}
					}
					/******* end of VB   *********/


					my_float_memcpy(VB_data_out,&VB_data_out[I2S_BUFF_LEN], LATENCY_LENGTH );
					my_float_memcpy(&VB_data_out[LATENCY_LENGTH],&leftChData_step_2[LATENCY_LENGTH], I2S_BUFF_LEN );
				}
				else
				{
					my_float_memset(VB_data_out , 0 , I2S_BUFF_LEN + LATENCY_LENGTH);
				}
				/********* end of LF path    **********/

				/********** HF path *********/

				if(hf_path)
				{
					DSP_FUNC_1CH_IN_1CH_OUT(&hpf_filter_left,&leftChData_step_0[LATENCY_LENGTH],&leftChData_step_1[LATENCY_LENGTH],I2S_BUFF_LEN );
					DSP_FUNC_1CH_IN_1CH_OUT(&hpf_filter_right,&rightChData_step_0[LATENCY_LENGTH],&rightChData_step_1[LATENCY_LENGTH],I2S_BUFF_LEN );

				}
				else
				{
					my_float_memset(leftChData_step_1 , 0 , I2S_BUFF_LEN + LATENCY_LENGTH);
					my_float_memset(rightChData_step_1 , 0 , I2S_BUFF_LEN + LATENCY_LENGTH);
				}


				DSP_FUNC_1CH_IN_1CH_OUT(&leftChanelEQ,&leftChData_step_1[LATENCY_LENGTH],&leftChData_step_2[LATENCY_LENGTH],I2S_BUFF_LEN );
				DSP_FUNC_1CH_IN_1CH_OUT(&rightChanelEQ,&rightChData_step_1[LATENCY_LENGTH],&rightChData_step_2[LATENCY_LENGTH],I2S_BUFF_LEN );

				my_float_memcpy_2_buffers( leftChData_step_2 , leftChData_hf_prev,
						rightChData_step_2 , rightChData_hf_prev , LATENCY_LENGTH);
				my_float_memcpy_2_buffers( leftChData_hf_prev , &leftChData_step_2[I2S_BUFF_LEN],
						rightChData_hf_prev , &rightChData_step_2[I2S_BUFF_LEN] , LATENCY_LENGTH);


				/********* end of HF path  *********/


				/********** collecting LF and HF pathes together with phase change on HF *********/
				pTmpBuf1 = leftChData_step_2 ;
				pTmpBuf2 = rightChData_step_2 ;
				pTmpBuf3 = VB_data_out ;
				for(i = 0 ; i < (I2S_BUFF_LEN + LATENCY_LENGTH) ; i++)
				{
//					*pTmpBuf1 = -*pTmpBuf1;
//					*pTmpBuf2 = -*pTmpBuf2;
					(*pTmpBuf1++) += (*pTmpBuf3);
					(*pTmpBuf2++) += (*pTmpBuf3++) ;
				}
				/********** end of collecting LF and HF pathes together  *********/



	#else  // without X.O
				DSP_FUNC_1CH_IN_1CH_OUT(&leftChanelEQ,leftChData_step_0,leftChData_step_2,I2S_BUFF_LEN);
				DSP_FUNC_1CH_IN_1CH_OUT(&rightChanelEQ,rightChData_step_0,rightChData_step_2,I2S_BUFF_LEN);
	#endif //   X.O

				if(0 != compressor_ratio)
				{
					DSP_FUNC_2CH_IN_2CH_OUT(&compressor_limiter,leftChData_step_2,rightChData_step_2,
							leftChData_step_1,rightChData_step_1,
							I2S_BUFF_LEN + LATENCY_LENGTH);

				}
				else
				{
					my_float_memcpy_2_buffers( leftChData_step_1 , leftChData_step_2,
							rightChData_step_1 , rightChData_step_2 , I2S_BUFF_LEN);
				}


				pTmpBuf1 = leftChData_step_1 ;
				pTmpBuf2 = rightChData_step_1 ;
				for(i = 0 ; i < (I2S_BUFF_LEN); i++)
				{
					*pTxBuf++ = (int16_t)(*pTmpBuf1++)		;// pTxBuf[2*i]
					*pTxBuf++ = (int16_t)(*pTmpBuf2++); // pTxBuf[2*i + 1]
				}




	//			if ( s_flag1 < 2 )
	//			{
	//				s_flag1 ++;
	//				if(2 == s_flag1)
	//				{
	//					DEV_IOCTL_0_PARAMS(i2s_dev , I2S_ENABLE_OUTPUT_IOCTL );
	//				}
	//			}
			}//if loopback
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
	set_band_biquads_t  set_band_biquads;
	switch(aIoctl_num)
	{
		case IOCTL_DEVICE_START :

			/* Create an application thread */

			set_band_biquads.Fc = 300;
			set_band_biquads.QValue = 1;//0.836;//0.707;
			set_band_biquads.Gain = 1;

			equalizer_api_init_dsp_descriptor(&lpf_filter);
			DSP_IOCTL_1_PARAMS(&lpf_filter , IOCTL_EQUALIZER_SET_NUM_OF_BANDS , ((void*)(size_t)2) );
			set_band_biquads.filter_mode = BIQUADS_LOWPASS_MODE_2_POLES;
			set_band_biquads.band_num = 0;
			DSP_IOCTL_1_PARAMS(&lpf_filter , IOCTL_EQUALIZER_SET_BAND_BIQUADS, &set_band_biquads );
			set_band_biquads.filter_mode = BIQUADS_LOWPASS_MODE_1_POLE;
			set_band_biquads.band_num = 1;
			DSP_IOCTL_1_PARAMS(&lpf_filter , IOCTL_EQUALIZER_SET_BAND_BIQUADS, &set_band_biquads );
			DSP_IOCTL_0_PARAMS(&lpf_filter , IOCTL_DEVICE_START );

			equalizer_api_init_dsp_descriptor(&hpf_filter_left);
			DSP_IOCTL_1_PARAMS(&hpf_filter_left , IOCTL_EQUALIZER_SET_NUM_OF_BANDS , ((void*)(size_t)2) );
			set_band_biquads.filter_mode = BIQUADS_HIGHPASS_MODE_2_POLES;
			set_band_biquads.band_num = 0;
			DSP_IOCTL_1_PARAMS(&hpf_filter_left , IOCTL_EQUALIZER_SET_BAND_BIQUADS, &set_band_biquads );
			set_band_biquads.filter_mode = BIQUADS_HIGHPASS_MODE_1_POLE;
			set_band_biquads.band_num = 1;
			DSP_IOCTL_1_PARAMS(&hpf_filter_left , IOCTL_EQUALIZER_SET_BAND_BIQUADS, &set_band_biquads );
			DSP_IOCTL_0_PARAMS(&hpf_filter_left , IOCTL_DEVICE_START );

			equalizer_api_init_dsp_descriptor(&hpf_filter_right);
			DSP_IOCTL_1_PARAMS(&hpf_filter_right , IOCTL_EQUALIZER_SET_NUM_OF_BANDS , ((void*)(size_t)2) );
			set_band_biquads.filter_mode = BIQUADS_HIGHPASS_MODE_2_POLES;
			set_band_biquads.band_num = 0;
			DSP_IOCTL_1_PARAMS(&hpf_filter_right , IOCTL_EQUALIZER_SET_BAND_BIQUADS, &set_band_biquads );
			set_band_biquads.filter_mode = BIQUADS_HIGHPASS_MODE_1_POLE;
			set_band_biquads.band_num = 1;
			DSP_IOCTL_1_PARAMS(&hpf_filter_right , IOCTL_EQUALIZER_SET_BAND_BIQUADS, &set_band_biquads );
			DSP_IOCTL_0_PARAMS(&hpf_filter_right , IOCTL_DEVICE_START );



			/************  vb filters ***********/
			set_band_biquads.Fc = 78.5;
			set_band_biquads.QValue = 0.707;//0.836;//0.7;
			set_band_biquads.Gain = 1;
			equalizer_api_init_dsp_descriptor(&vb_hpf_filter);
			DSP_IOCTL_1_PARAMS(&vb_hpf_filter , IOCTL_EQUALIZER_SET_NUM_OF_BANDS , ((void*)(size_t)4) );
			set_band_biquads.filter_mode = BIQUADS_HIGHPASS_MODE_2_POLES;
			set_band_biquads.band_num = 0;
			DSP_IOCTL_1_PARAMS(&vb_hpf_filter , IOCTL_EQUALIZER_SET_BAND_BIQUADS, &set_band_biquads );
			set_band_biquads.band_num = 1;
			DSP_IOCTL_1_PARAMS(&vb_hpf_filter , IOCTL_EQUALIZER_SET_BAND_BIQUADS, &set_band_biquads );
			set_band_biquads.band_num = 2;
			DSP_IOCTL_1_PARAMS(&vb_hpf_filter , IOCTL_EQUALIZER_SET_BAND_BIQUADS, &set_band_biquads );
			set_band_biquads.band_num = 3;
			DSP_IOCTL_1_PARAMS(&vb_hpf_filter , IOCTL_EQUALIZER_SET_BAND_BIQUADS, &set_band_biquads );
			DSP_IOCTL_0_PARAMS(&vb_hpf_filter , IOCTL_DEVICE_START );

#if 0
			set_band_biquads.QValue = 0.707;//0.836;//0.7;
			set_band_biquads.Gain = 1;
			equalizer_api_init_dsp_descriptor(&vb_final_filter);
			DSP_IOCTL_1_PARAMS(&vb_final_filter , IOCTL_EQUALIZER_SET_NUM_OF_BANDS , ((void*)(size_t)3) );
			set_band_biquads.Fc = 100;
			set_band_biquads.filter_mode = BIQUADS_HIGHPASS_MODE_2_POLES;
			set_band_biquads.band_num = 0;
			DSP_IOCTL_1_PARAMS(&vb_final_filter , IOCTL_EQUALIZER_SET_BAND_BIQUADS, &set_band_biquads );
			set_band_biquads.filter_mode = BIQUADS_HIGHPASS_MODE_2_POLES;
			set_band_biquads.band_num = 1;
			DSP_IOCTL_1_PARAMS(&vb_final_filter , IOCTL_EQUALIZER_SET_BAND_BIQUADS, &set_band_biquads );
			set_band_biquads.Fc = 300;
			set_band_biquads.filter_mode = BIQUADS_LOWPASS_MODE_2_POLES;
			set_band_biquads.band_num = 2;
			DSP_IOCTL_1_PARAMS(&vb_final_filter , IOCTL_EQUALIZER_SET_BAND_BIQUADS, &set_band_biquads );
			DSP_IOCTL_0_PARAMS(&vb_final_filter , IOCTL_DEVICE_START );
#else
			set_band_biquads.QValue = 1;//0.836;//0.707;
			set_band_biquads.Gain = 1;
			equalizer_api_init_dsp_descriptor(&vb_final_filter);
			DSP_IOCTL_1_PARAMS(&vb_final_filter , IOCTL_EQUALIZER_SET_NUM_OF_BANDS , ((void*)(size_t)7) );
			set_band_biquads.Fc = 80;
			set_band_biquads.filter_mode = BIQUADS_HIGHPASS_MODE_2_POLES;
			set_band_biquads.band_num = 0;
			DSP_IOCTL_1_PARAMS(&vb_final_filter , IOCTL_EQUALIZER_SET_BAND_BIQUADS, &set_band_biquads );
			set_band_biquads.filter_mode = BIQUADS_HIGHPASS_MODE_1_POLE;
			set_band_biquads.band_num = 1;
			DSP_IOCTL_1_PARAMS(&vb_final_filter , IOCTL_EQUALIZER_SET_BAND_BIQUADS, &set_band_biquads );
			set_band_biquads.QValue = 0.707;//0.836;//0.707;
			set_band_biquads.Fc = 30;
			set_band_biquads.filter_mode = BIQUADS_HIGHPASS_MODE_2_POLES;
			set_band_biquads.band_num = 2;
			DSP_IOCTL_1_PARAMS(&vb_final_filter , IOCTL_EQUALIZER_SET_BAND_BIQUADS, &set_band_biquads );
			set_band_biquads.filter_mode = BIQUADS_HIGHPASS_MODE_1_POLE;
			set_band_biquads.band_num = 3;
			DSP_IOCTL_1_PARAMS(&vb_final_filter , IOCTL_EQUALIZER_SET_BAND_BIQUADS, &set_band_biquads );
			set_band_biquads.Fc = 500;
			set_band_biquads.filter_mode = BIQUADS_LOWPASS_MODE_2_POLES;
			set_band_biquads.band_num = 4;
			DSP_IOCTL_1_PARAMS(&vb_final_filter , IOCTL_EQUALIZER_SET_BAND_BIQUADS, &set_band_biquads );
			set_band_biquads.band_num = 5;
			DSP_IOCTL_1_PARAMS(&vb_final_filter , IOCTL_EQUALIZER_SET_BAND_BIQUADS, &set_band_biquads );
			set_band_biquads.band_num = 6;
			DSP_IOCTL_1_PARAMS(&vb_final_filter , IOCTL_EQUALIZER_SET_BAND_BIQUADS, &set_band_biquads );			DSP_IOCTL_0_PARAMS(&vb_final_filter , IOCTL_DEVICE_START );
#endif
			/**************   eq filters  *************/
			equalizer_api_init_dsp_descriptor(&leftChanelEQ);
			DSP_IOCTL_1_PARAMS(&leftChanelEQ , IOCTL_EQUALIZER_SET_NUM_OF_BANDS , ((void*)(size_t)7) );
			DSP_IOCTL_0_PARAMS(&leftChanelEQ , IOCTL_DEVICE_START );
			equalizer_api_init_dsp_descriptor(&rightChanelEQ);
			DSP_IOCTL_1_PARAMS(&rightChanelEQ , IOCTL_EQUALIZER_SET_NUM_OF_BANDS , ((void*)(size_t)7) );
			DSP_IOCTL_0_PARAMS(&rightChanelEQ , IOCTL_DEVICE_START );

			compressor_api_init_dsp_descriptor(&compressor_limiter);
			DSP_IOCTL_1_PARAMS(&compressor_limiter , IOCTL_COMPRESSOR_SET_HIGH_THRESHOLD , ((void*)(size_t)HIGH_THRESHOLD_VALUE) );
			DSP_IOCTL_0_PARAMS(&compressor_limiter , IOCTL_DEVICE_START );

			compressor_api_init_dsp_descriptor(&vb_limiter);
			DSP_IOCTL_1_PARAMS(&vb_limiter , IOCTL_COMPRESSOR_SET_HIGH_THRESHOLD , ((void*)(size_t)(0.2 * 0x8000)) );
			DSP_IOCTL_0_PARAMS(&vb_limiter , IOCTL_DEVICE_START );

			os_create_task("main" , main_thread_func, 0, MAIN_STACK_SIZE_BYTES , APP_DEV_THREAD_PRIORITY);

			break;
		default :
			return 1;
	}
	return 0;
}
