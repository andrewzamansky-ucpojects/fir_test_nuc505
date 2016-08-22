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

#include "_virtual_bass_prerequirements_check.h"

#include "virtual_bass_api.h" //place first to test that header file is self-contained
#include "virtual_bass.h"
#include "common_dsp_api.h"

#include "auto_init_api.h"

#include "math.h"

#ifdef CONFIG_USE_HW_DSP
  #include "cpu_config.h"
  #include "arm_math.h"
#endif

/********  defines *********************/


/********  types  *********************/

/********  externals *********************/


/********  exported variables *********************/

char virtual_bass_module_name[] = "virtual_bass";


/**********   external variables    **************/



/***********   local variables    **************/





/* ----------------------------------------------------------------------

** Fast approximation to the log2() function.  It uses a two step

** process.  First, it decomposes the floating-point number into

** a fractional component F and an exponent E.  The fraction component

** is used in a polynomial approximation and then the exponent added

** to the result.  A 3rd order polynomial is used and the result

** when computing db20() is accurate to 7.984884e-003 dB.

** ------------------------------------------------------------------- */


float log2f_approx_coeff[4] = {1.23149591368684f, -4.11852516267426f, 6.02197014179219f, -3.13396450166353f};


float log2f_approx(float X)
{
  float *C = &log2f_approx_coeff[0];
  float Y;
  float F;
  int E;

  // This is the approximation to log2()
  F = frexpf(fabsf(X), &E);

  //  Y = C[0]*F*F*F + C[1]*F*F + C[2]*F + C[3] + E;

  Y = *C++;
  Y *= F;
  Y += (*C++);
  Y *= F;
  Y += (*C++);
  Y *= F;
  Y += (*C++);
  Y += E;

  return(Y);
}


#define LOG_COEFF	(20.0f/3.321928095f)
#define THRESHOLD	-65.0f

#define COMPR_ATTACK  	0.999896f
#define ONE_MINUS_COMPR_ATTACK  	(1.0f - COMPR_ATTACK)
#define COMPR_REALESE	0.900842f
#define  ONE_MINUS_COMPR_REALESE	(1.0f - COMPR_REALESE)
#define RATIO			0.2f
float vb_volume = 0.3315;
float volatile mon ;
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
void virtual_bass_dsp(pdsp_descriptor apdsp , size_t data_len ,
		dsp_pad_t *in_pads[MAX_NUM_OF_OUTPUT_PADS] , dsp_pad_t out_pads[MAX_NUM_OF_OUTPUT_PADS])
{

	float *apCh1In ;
	float *apCh1Out  ;
	float curr_ratio ;
	VIRTUAL_BASS_Instance_t *handle;



//	uint8_t envelope_folower_sample_count =ENVELOP_FOLLOWER_SAMPLE_RATE;
	float envelope_folower;
	float curr_y,curr_x , curr_x_log;


	handle = apdsp->handle;
	envelope_folower = handle->envelope_folower ;

	apCh1In = in_pads[0]->buff;
	apCh1Out = out_pads[0].buff;





//	arm_scale_f32(apCh1In , (-2.2f) , apCh1Out , data_len);
//	arm_abs_f32( apCh1In , apCh1Out , data_len);

	while(data_len--)
	{
		float delta , tmp ;


		curr_x = *apCh1In++;

		mon = curr_x;
		// calculate 20*log(x)
		curr_x_log = log2f_approx(curr_x);
		curr_x_log *= LOG_COEFF;
		mon = curr_x_log;
		delta = 0;
		if(curr_x_log > THRESHOLD)
		{
			delta = curr_x_log - THRESHOLD;
		}
		mon = delta;

		if(delta > envelope_folower)
		{
			envelope_folower *= COMPR_ATTACK ;
			tmp = delta * ONE_MINUS_COMPR_ATTACK ;
		}
		else
		{
			envelope_folower *= COMPR_REALESE ;
			tmp = delta * ONE_MINUS_COMPR_REALESE ;
		}
		envelope_folower += tmp ;
		mon = envelope_folower;


		tmp = delta / (-160.0f); // to change sign for log2lin
		tmp += RATIO ;
		tmp *= envelope_folower ;
		mon = tmp;


		if(curr_x > 0)
		{
			tmp *= 1.2f ;
		}
		else
		{
			tmp *= 0.8f ;
		}
		mon = tmp;

		tmp /= 20;
		curr_ratio = fast_pow(10 , tmp);
		mon = curr_ratio;

		curr_y = curr_x * curr_ratio;
		curr_y *= vb_volume;
		*apCh1Out++ = curr_y;
	}



	handle->envelope_folower =envelope_folower ;


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
uint8_t virtual_bass_ioctl(pdsp_descriptor apdsp ,const uint8_t aIoctl_num , void * aIoctl_param1 , void * aIoctl_param2)
{
	VIRTUAL_BASS_Instance_t *handle = apdsp->handle;;

	switch(aIoctl_num)
	{

		case IOCTL_DSP_INIT :
			handle->envelope_folower = 0 ;

			break;

		default :
			return 1;
	}
	return 0;
}



/*---------------------------------------------------------------------------------------------------------*/
/* Function:        virtual_bass_init                                                                          */
/*                                                                                                         */
/* Parameters:                                                                                             */
/*                                                                                         */
/*                                                                                                  */
/* Returns:                                                                                      */
/* Side effects:                                                                                           */
/* Description:                                                                                            */
/*                                                            						 */
/*---------------------------------------------------------------------------------------------------------*/
void  virtual_bass_init(void)
{
	DSP_REGISTER_NEW_MODULE("virtual_bass",virtual_bass_ioctl , virtual_bass_dsp , VIRTUAL_BASS_Instance_t);
}

AUTO_INIT_FUNCTION(virtual_bass_init);
