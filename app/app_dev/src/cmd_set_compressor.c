
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
#include "compressor_api.h"
#include "os_wrapper.h"

extern dsp_descriptor_t compressor_limiter;
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
int do_set_compressor (cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{

	float compressor_ratio ;

	if(argc < 2)
	{
		SHELL_REPLY_STR("syntax err\n");
		return 1;
	}


	compressor_ratio = (float)atof(argv[1]);

	os_mutex_take_infinite_wait(control_mutex);

	DSP_IOCTL_1_PARAMS(&compressor_limiter , IOCTL_COMPRESSOR_SET_RATIO , &compressor_ratio );

	os_mutex_give(control_mutex);

	return 0;
}

U_BOOT_CMD(
	set_compressor,     255,	0,	do_set_compressor,
	"set_compressor ratio",
	"info   - \n"
);
