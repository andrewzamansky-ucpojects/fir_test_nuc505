
/*
 *  cmd_set_comressor.c
 */
#include "_project.h"
#include <stdio.h>
#include <string.h>

#include <stdlib.h>

#include "u-boot/include/command.h"
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
int do_set_limiter (cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{

	uint32_t chunk_size ;
	float release ;

	if(argc < 3)
	{
		SHELL_REPLY_STR("syntax err\n");
		return 1;
	}
	os_mutex_take_infinite_wait(control_mutex);

	chunk_size =  atoi(argv[1]);
	DSP_IOCTL_1_PARAMS(&compressor_limiter , IOCTL_COMPRESSOR_SET_LOOK_AHEAD_SIZE , (void*)chunk_size );


	release = (float)atof(argv[2]);
	DSP_IOCTL_1_PARAMS(&compressor_limiter , IOCTL_COMPRESSOR_SET_RELEASE , &release );
	os_mutex_give(control_mutex);

	return 0;
}

U_BOOT_CMD(
	set_limiter,     255,	0,	do_set_limiter,
	"set_limiter chunk_size release",
	"info   - \n"
);
