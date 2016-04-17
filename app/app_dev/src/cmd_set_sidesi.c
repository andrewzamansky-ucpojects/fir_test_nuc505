
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
int do_set_sidesi (cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{

	float gain;

	if(argc < 2)
	{
		SHELL_REPLY_STR("syntax err\n");
		return 1;
	}


	os_mutex_take_infinite_wait(control_mutex);

	gain = (float)atof(argv[1]);
	DSP_IOCTL_1_PARAMS(&voice_3d , IOCTL_VOICE_3D_SET_MEDIUM_GAIN , &gain );

	os_mutex_give(control_mutex);

	return 0;
}

U_BOOT_CMD(
	set_sidesi,     255,	0,	do_set_sidesi,
	"set_sidesi gain ",
	"info   - \n"
);
