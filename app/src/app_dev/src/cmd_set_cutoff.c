
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

#include "equalizer_api.h"
#include "common_dsp_api.h"


extern float cutoff_freq ;



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
int do_set_cutoff (cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{



	if(argc < 2)
	{
		SHELL_REPLY_STR("syntax err\n");
		return 1;
	}



	cutoff_freq = (float)atof(argv[1]);
	app_dev_set_cuttof();


	return 0;
}

U_BOOT_CMD(
	set_cutoff,     255,	0,	do_set_cutoff,
	"set_cutoff Fc ",
	"info   - \n"
);

