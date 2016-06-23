
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
#include "os_wrapper.h"

extern float COMPR_ATTACK ;
extern float COMPR_REALESE ;
extern float HARMONICS_GAIN ;
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
int do_set_vb(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{



	if(argc < 4)
	{
		SHELL_REPLY_STR("syntax err\n");
		return 1;
	}

	// should be transformed to ioctls !!

	os_mutex_take_infinite_wait(control_mutex);

	HARMONICS_GAIN = (float)atof(argv[1]);
	COMPR_ATTACK = (float)atof(argv[2]);
	COMPR_REALESE = (float)atof(argv[3]);

	os_mutex_give(control_mutex);

	return 0;
}

U_BOOT_CMD(
	set_vb,     255,	0,	do_set_vb,
	"set_vb  harmonic_vol attack release",
	"info   - \n"
);
