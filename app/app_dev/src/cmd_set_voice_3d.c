
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


#include "dsp_managment_api.h"
#include "voice_3D_api.h"
#include "os_wrapper.h"

extern dsp_descriptor_t voice_3d;
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
int do_set_voice_3d (cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{

	float gain;

	if(argc < 4)
	{
		SHELL_REPLY_STR("syntax err\n");
		return 1;
	}


	os_mutex_take_infinite_wait(control_mutex);

	gain = (float)atof(argv[1]);
	DSP_IOCTL_1_PARAMS(&voice_3d , IOCTL_VOICE_3D_SET_MEDIUM_GAIN , &gain );
	gain = (float)atof(argv[2]);
	DSP_IOCTL_1_PARAMS(&voice_3d , IOCTL_VOICE_3D_SET_SIDE_GAIN , &gain );
	gain =  (float)atof(argv[3]);
	DSP_IOCTL_1_PARAMS(&voice_3d , IOCTL_VOICE_3D_SET_3D_GAIN , &gain );

	os_mutex_give(control_mutex);

	return 0;
}

U_BOOT_CMD(
	set_voice_3d,     255,	0,	do_set_voice_3d,
	"set_voice_3d m-gain s-gain 3d-gain ",
	"info   - \n"
);
