
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
#include "os_wrapper.h"

extern float vb_volume ;

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
int do_set_vbi(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{



	if(argc < 2)
	{
		SHELL_REPLY_STR("syntax err\n");
		return 1;
	}

	// should be transformed to ioctls !!

	os_mutex_take_infinite_wait(control_mutex);

	vb_volume = (float)atof(argv[1]) /100 * 0.85;
	PRINTF_DBG("vb_volume = %f\n",vb_volume);
	os_mutex_give(control_mutex);

	return 0;
}

U_BOOT_CMD(
	set_vbi,     255,	0,	do_set_vbi,
	"set_vbi  vol",
	"info   - \n"
);
