
/*
 *  cmd_set_comressor.c
 */
#include "dev_managment_config.h"
#include "src/_dev_managment_prerequirements_check.h"// should be after dev_managment_config.h
#include <stdio.h>
#include <string.h>

#include <stdlib.h>

#include <command.h>
#include "shell_api.h"

#include "equalizer_api.h"
#include "common_dsp_api.h"

extern dsp_descriptor_t lpf_filter;
//extern dsp_descriptor_t vb_hpf_filter;
extern dsp_descriptor_t vb_final_filter;


extern dsp_descriptor_t hpf_filter_left;
extern dsp_descriptor_t hpf_filter_right;

/*
 * Subroutine:  do_set_comressor
 *
 * Description:
 *
 * Inputs:
 *
 * Return:      None
 *
 */
int do_set_cutoff (cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{


	set_band_biquads_t  set_band_biquads;

	if(argc < 2)
	{
		SHELL_REPLY_STR("syntax err\n");
		return 1;
	}



	set_band_biquads.Fc = (float)atof(argv[1]);
	set_band_biquads.QValue = 1;
	set_band_biquads.Gain = 1;

	set_band_biquads.filter_mode = BIQUADS_LOWPASS_MODE_2_POLES;
	set_band_biquads.band_num = 0;
	DSP_IOCTL_1_PARAMS(&lpf_filter , IOCTL_EQUALIZER_SET_BAND_BIQUADS, &set_band_biquads );
	set_band_biquads.filter_mode = BIQUADS_LOWPASS_MODE_1_POLE;
	set_band_biquads.band_num = 1;
	DSP_IOCTL_1_PARAMS(&lpf_filter , IOCTL_EQUALIZER_SET_BAND_BIQUADS, &set_band_biquads );

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

	set_band_biquads.Fc = set_band_biquads.Fc * 0.8;
	set_band_biquads.filter_mode = BIQUADS_HIGHPASS_MODE_2_POLES;
	set_band_biquads.band_num = 0;
	DSP_IOCTL_1_PARAMS(&vb_final_filter , IOCTL_EQUALIZER_SET_BAND_BIQUADS, &set_band_biquads );
	set_band_biquads.filter_mode = BIQUADS_HIGHPASS_MODE_1_POLE;
	set_band_biquads.band_num = 1;
	DSP_IOCTL_1_PARAMS(&vb_final_filter , IOCTL_EQUALIZER_SET_BAND_BIQUADS, &set_band_biquads );

	return 0;
}

U_BOOT_CMD(
	set_cutoff,     255,	0,	do_set_cutoff,
	"set_cutoff Fc ",
	"info   - \n"
);

