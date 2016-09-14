
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
#include "I2S_nuc505_api.h"
#include "os_wrapper.h"

extern pdev_descriptor_t i2s_dev;
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

	int volume  ;
	int8_t int8_volume  ;

	if(argc < 2)
	{
		SHELL_REPLY_STR("syntax err\n");
		return 1;
	}


	volume  = (int)atof(argv[1]);

	if( (volume < -60) || (volume > 0))
	{
		SHELL_REPLY_STR("volume should be in [0..-60]  db n");
		return 1;
	}
	os_mutex_take_infinite_wait(control_mutex);

	int8_volume = volume ;
	DEV_IOCTL_1_PARAMS(i2s_dev , I2S_SET_OUT_VOLUME_LEVEL_DB ,&int8_volume);

	os_mutex_give(control_mutex);

	return 0;
}

U_BOOT_CMD(
	set_vol,     255,	0,	do_set_vol,
	"set_vol volume",
	"info   - \n"
);
