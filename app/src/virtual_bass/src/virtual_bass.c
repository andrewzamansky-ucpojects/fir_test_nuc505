/*
 *
 * file :   virtual_bass.c
 *
 *
 *
 *
 *
 */



/********  includes *********************/

#include "virtual_bass_config.h"
#include "dev_managment_api.h" // for device manager defines and typedefs
#include "dsp_managment_api.h" // for device manager defines and typedefs
#include "_virtual_bass_prerequirements_check.h" // should be after {virtual_bass_config.h,dev_managment_api.h}

#include "virtual_bass_api.h" //place first to test that header file is self-contained
#include "virtual_bass.h"
#include "common_dsp_api.h"

#include "math.h"

#ifdef _USE_DSP_
  #include "arm_math.h"
#endif

/********  defines *********************/


/********  types  *********************/

/********  externals *********************/


/********  local defs *********************/

#define INSTANCE(hndl)	((VIRTUAL_BASS_Instance_t*)hndl)


/**********   external variables    **************/



/***********   local variables    **************/
#if VIRTUAL_BASS_CONFIG_NUM_OF_DYNAMIC_INSTANCES>0

	static VIRTUAL_BASS_Instance_t VIRTUAL_BASS_InstanceParams[VIRTUAL_BASS_CONFIG_NUM_OF_DYNAMIC_INSTANCES] = { {0} };
	static uint16_t usedInstances =0 ;


#endif // for VIRTUAL_BASS_CONFIG_NUM_OF_DYNAMIC_INSTANCES>0

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


#define HARMONIC_CREATOR_1

/*---------------------------------------------------------------------------------------------------------*/
/* Function:        virtual_bass_dsp                                                                          */
/*                                                                                                         */
/* Parameters:                                                                                             */
/*                                                                                         */
/*                                                                                                  */
/* Returns:                                                                                      */
/* Side effects:                                                                                           */
/* Description:                                                                                            */
/*                                                            						 */
/*---------------------------------------------------------------------------------------------------------*/
void virtual_bass_dsp(const void * const aHandle , size_t data_len ,
		float *in_pads[MAX_NUM_OF_OUTPUT_PADS] , float *out_pads[MAX_NUM_OF_OUTPUT_PADS])
{

	float *apCh1In ;
	float *apCh1Out  ;
	float curr_ratio ;



	uint8_t envelope_folower_sample_count =ENVELOP_FOLLOWER_SAMPLE_RATE;
	float envelope_folower;
	float curr_y,curr_x ;

#ifdef	HARMONIC_CREATOR_1
	float harmonic_out;
#else
	float xn1;
#endif


	envelope_folower = INSTANCE(aHandle)->envelope_folower ;

	apCh1In = in_pads[0];
	apCh1Out = out_pads[0];



#ifdef	HARMONIC_CREATOR_1
	harmonic_out = INSTANCE(aHandle)->prev_harmonic_out ;
#else
	xn1 = INSTANCE(aHandle)->prev_x ;
#endif

//	arm_scale_f32(apCh1In , (-2.2f) , apCh1Out , data_len);
//	arm_abs_f32( apCh1In , apCh1Out , data_len);

	while(data_len--)
	{
		float tmp , abs_result;


		curr_x = *apCh1In++;
//		curr_x = PRE_CLU_GAIN * curr_x;
		curr_x = (-2.2f) * curr_x;

//		if(ENVELOP_FOLLOWER_SAMPLE_RATE == envelope_folower_sample_count )
//		{
			envelope_folower_sample_count = 0;
//			abs_result = *apCh1Out;
			abs_result = fabs(curr_x);


			if (abs_result > envelope_folower)
			{
				abs_result *=COMPR_ATTACK_ADJUSTED;
				envelope_folower *= (1-COMPR_ATTACK_ADJUSTED);
			}
			else
			{
				abs_result *=COMPR_REALESE_ADJUSTED;
				envelope_folower *= (1 - COMPR_REALESE_ADJUSTED);
			}
			envelope_folower += abs_result;


			tmp = (B + envelope_folower);
			curr_ratio = A/tmp;
			envelope_folower_sample_count = 1;

			// harmonic creations
#ifdef HARMONIC_CREATOR_1
			if(curr_x < harmonic_out  )
			{
				tmp = HU1_B_ADJUSTED;
			}
			else
			{
				tmp = HU1_A_ADJUSTED;
			}
			harmonic_out = (1-tmp)*harmonic_out;
			tmp *=  curr_x ;
			harmonic_out += tmp ;
#else
			if(curr_x > xn1)
			{
				harmonic_out = curr_x;
			}
			xn1 = curr_x;
#endif
//		}
		envelope_folower_sample_count++;

		curr_y = curr_ratio * curr_x ;
//		curr_y = curr_y * POST_CLU_GAIN;
		tmp = HARMONICS_GAIN * harmonic_out;
		curr_y += tmp ;
		curr_y = curr_y * vb_volume ;
		*apCh1Out++ =   curr_y;


	}


#ifdef	HARMONIC_CREATOR_1
	INSTANCE(aHandle)->prev_harmonic_out = harmonic_out;
#else
	INSTANCE(aHandle)->prev_x = xn1;
#endif

	INSTANCE(aHandle)->envelope_folower =envelope_folower ;


}




/*---------------------------------------------------------------------------------------------------------*/
/* Function:        virtual_bass_ioctl                                                                          */
/*                                                                                                         */
/* Parameters:                                                                                             */
/*                                                                                         */
/*                                                                                                  */
/* Returns:                                                                                      */
/* Side effects:                                                                                           */
/* Description:                                                                                            */
/*                                                            						 */
/*---------------------------------------------------------------------------------------------------------*/
uint8_t virtual_bass_ioctl(void * const aHandle ,const uint8_t aIoctl_num , void * aIoctl_param1 , void * aIoctl_param2)
{

	switch(aIoctl_num)
	{
//#if VIRTUAL_BASS_CONFIG_NUM_OF_DYNAMIC_INSTANCES > 0
//		case IOCTL_GET_PARAMS_ARRAY_FUNC :
//			*(const dev_param_t**)aIoctl_param1  = VIRTUAL_BASS_Dev_Params;
//			*(uint8_t*)aIoctl_param2 = sizeof(VIRTUAL_BASS_Dev_Params)/sizeof(dev_param_t); //size
//			break;
//#endif // for VIRTUAL_BASS_CONFIG_NUM_OF_DYNAMIC_INSTANCES > 0


		case IOCTL_DEVICE_START :

			break;

		default :
			return 1;
	}
	return 0;
}





#if VIRTUAL_BASS_CONFIG_NUM_OF_DYNAMIC_INSTANCES>0

/*---------------------------------------------------------------------------------------------------------*/
/* Function:        VIRTUAL_BASS_API_Init_Dev_Descriptor                                                                          */
/*                                                                                                         */
/* Parameters:                                                                                             */
/*                                                                                         */
/*                                                                                                  */
/* Returns:                                                                                      */
/* Side effects:                                                                                           */
/* Description:                                                                                            */
/*                                                            						 */
/*---------------------------------------------------------------------------------------------------------*/
uint8_t  virtual_bass_api_init_dsp_descriptor(pdsp_descriptor aDspDescriptor)
{
	VIRTUAL_BASS_Instance_t *pInstance;
	if(NULL == aDspDescriptor) return 1;
	if (usedInstances >= VIRTUAL_BASS_CONFIG_NUM_OF_DYNAMIC_INSTANCES) return 1;

	pInstance = &VIRTUAL_BASS_InstanceParams[usedInstances ];

	aDspDescriptor->handle = pInstance;
	aDspDescriptor->ioctl = virtual_bass_ioctl;
	aDspDescriptor->dsp_func = virtual_bass_dsp;
	pInstance->envelope_folower = 0 ;
	pInstance->prev_x =  0 ;
	pInstance->prev_harmonic_out = 0;
	usedInstances++;

	return 0 ;

}
#endif  // for VIRTUAL_BASS_CONFIG_NUM_OF_DYNAMIC_INSTANCES>0
