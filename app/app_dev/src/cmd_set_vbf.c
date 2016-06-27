
/*
 *  cmd_set_comressor.c
 */
#include "_project.h"
#include <stdio.h>
#include <string.h>

#include <stdlib.h>

#include "u-boot/include/command.h"
#include "shell_api.h"

#include "equalizer_api.h"
#include "common_dsp_api.h"

#include "os_wrapper.h"

extern float cutoff_freq ;
extern os_mutex_t  control_mutex;



extern uint8_t app_dev_set_cuttof();

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
int do_set_vbf (cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{



	if(argc < 2)
	{
		SHELL_REPLY_STR("syntax err\n");
		return 1;
	}



	cutoff_freq = (float)atof(argv[1]);

	os_mutex_take_infinite_wait(control_mutex);

	app_dev_set_cuttof();

	os_mutex_give(control_mutex);

	return 0;
}

U_BOOT_CMD(
	set_vbf,     255,	0,	do_set_vbf,
	"set_vbf Fc ",
	"info   - \n"
);