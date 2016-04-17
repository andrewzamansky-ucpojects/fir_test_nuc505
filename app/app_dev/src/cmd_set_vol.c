
/*
 *  cmd_set_comressor.c
 */
#include "_project.h"
#include <stdio.h>
#include <string.h>

#include <stdlib.h>

#include <command.h>
#include "shell_api.h"

#include "dsp_managment_api.h"
#include "mixer_api.h"
#include "os_wrapper.h"

extern dsp_descriptor_t adder_bass_with_left_channel;
extern dsp_descriptor_t adder_bass_with_right_channel;
extern os_mutex_t  control_mutex;

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
int do_set_vol (cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	set_channel_weight_t ch_weight;

	float volume  ;

	if(argc < 2)
	{
		SHELL_REPLY_STR("syntax err\n");
		return 1;
	}


	volume  = (float)atof(argv[1]);

	os_mutex_take_infinite_wait(control_mutex);

	ch_weight.channel_num = 0;
	ch_weight.weight = volume;
	DSP_IOCTL_1_PARAMS(&adder_bass_with_left_channel , IOCTL_MIXER_SET_CHANNEL_WEIGHT , &ch_weight  );
	ch_weight.channel_num = 1;
	ch_weight.weight = volume;
	DSP_IOCTL_1_PARAMS(&adder_bass_with_left_channel , IOCTL_MIXER_SET_CHANNEL_WEIGHT , &ch_weight  );


	ch_weight.channel_num = 0;
	ch_weight.weight = volume;
	DSP_IOCTL_1_PARAMS(&adder_bass_with_right_channel , IOCTL_MIXER_SET_CHANNEL_WEIGHT , &ch_weight  );
	ch_weight.channel_num = 1;
	ch_weight.weight = volume;
	DSP_IOCTL_1_PARAMS(&adder_bass_with_right_channel , IOCTL_MIXER_SET_CHANNEL_WEIGHT , &ch_weight  );
	os_mutex_give(control_mutex);

	return 0;
}

U_BOOT_CMD(
	set_vol,     255,	0,	do_set_vol,
	"set_vol",
	"info   - \n"
);
