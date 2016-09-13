
/*
 *  cmd_set_comressor.c
 */
#include "_project.h"
#include <stdio.h>
#include <string.h>

#include <stdlib.h>

#include "u-boot/include/command.h"
#include "shell_api.h"

#include "dsp_management_api.h"
#include "I2S_mixer_api.h"
#include "os_wrapper.h"

extern dsp_descriptor_t app_I2S_mixer;
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

	float volume  ;

	if(argc < 2)
	{
		SHELL_REPLY_STR("syntax err\n");
		return 1;
	}


	volume  = (float)atof(argv[1]);

	os_mutex_take_infinite_wait(control_mutex);

	DSP_IOCTL_1_PARAMS(&app_I2S_mixer , IOCTL_I2S_MIXER_SET_OUT_LEVEL , &volume  );

	os_mutex_give(control_mutex);

	return 0;
}

U_BOOT_CMD(
	set_vol,     255,	0,	do_set_vol,
	"set_vol",
	"info   - \n"
);
