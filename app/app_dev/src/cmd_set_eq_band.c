
/*
 *  cmd_set_eq_band.c
 */
#include "_project.h"
#include <stdio.h>
#include <string.h>

#include <stdlib.h>

#include "u-boot/include/command.h"
#include "shell_api.h"
#include "dev_managment_api.h"
#include "dsp_managment_api.h"

#include "equalizer_api.h"
#include "common_dsp_api.h"
#include "os_wrapper.h"

extern dsp_descriptor_t leftChanelEQ,rightChanelEQ;
extern os_mutex_t  control_mutex;

/*
 * Subroutine:  force_output
 *
 * Description:
 *
 * Inputs:
 *
 * Return:      None
 *
 */
int do_set_eq_band (cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{

#if 0 // ffmpeg
	if(argc < 3) {
		SHELL_REPLY_STR("err . syntax : set_eq_band filter_name filter_params\n");
		return 1;
	}
	DSP_IOCTL_2_PARAMS(&leftChanelDsp , IOCTL_EQUALIZER_band_biquads_set_params, argv[1] , argv[2] );
#else
	equalizer_api_band_set_t band_set;
	equalizer_api_band_set_params_t  *p_band_set_params;

	if(argc < 5)  {
		SHELL_REPLY_STR("syntax err\n");
		return 1;
	}

	band_set.band_num = atoi(argv[2])-1;
	p_band_set_params = &band_set.band_set_params;
	DSP_IOCTL_1_PARAMS(&leftChanelEQ , IOCTL_EQUALIZER_GET_BAND_BIQUADS, &band_set );

	p_band_set_params->Fc = (float)atof(argv[3]);
	p_band_set_params->Gain = (float)atof(argv[4]);
	p_band_set_params->QValue = (float)atof(argv[5]);




	if (0 == strcmp(argv[6],"lpf")) {
		p_band_set_params->filter_mode = BIQUADS_LOWPASS_MODE_2_POLES;
	}
	else if (0 == strcmp(argv[6],"hpf")) {
		p_band_set_params->filter_mode = BIQUADS_HIGHPASS_MODE_2_POLES;
	} else if (0 == strcmp(argv[6],"peak")) {
		p_band_set_params->filter_mode = BIQUADS_PEAK_MODE;
	} else if (0 == strcmp(argv[6],"ls")) {
		p_band_set_params->filter_mode = BIQUADS_LOWSHELF_MODE;
	} else if (0 == strcmp(argv[6],"hs"))	{
		p_band_set_params->filter_mode = BIQUADS_HIGHSHELF_MODE;
	} else if (0 == strcmp(argv[6],"lpf_1_pole"))	{
		p_band_set_params->filter_mode = BIQUADS_LOWPASS_MODE_1_POLE;
	} else if (0 == strcmp(argv[6],"hpf_1_pole")) {
		p_band_set_params->filter_mode = BIQUADS_HIGHPASS_MODE_1_POLE;
	} else if (0 == strcmp(argv[6],"bypass"))	{
		p_band_set_params->filter_mode = BIQUADS_TRANSPARENT_MODE;
	}	else {
		SHELL_REPLY_STR("filter mode uknown\n");
		return 1;
	}

	os_mutex_take_infinite_wait(control_mutex);


	DSP_IOCTL_1_PARAMS(&leftChanelEQ , IOCTL_EQUALIZER_SET_BAND_BIQUADS, &band_set );
	DSP_IOCTL_1_PARAMS(&rightChanelEQ , IOCTL_EQUALIZER_SET_BAND_BIQUADS, &band_set );

	os_mutex_give(control_mutex);

#endif
	return 0;
}

U_BOOT_CMD(
	set_eq,     255,	0,	do_set_eq_band,
	"set_eq L/F/S band_num Fc Gain QValue filter_type",
	"info   - \n"
);
