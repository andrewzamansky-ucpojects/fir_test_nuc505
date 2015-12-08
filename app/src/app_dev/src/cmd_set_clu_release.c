
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

extern float COMPR_REALESE ;


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
int do_set_clu_release (cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{



	if(argc < 2)
	{
		SHELL_REPLY_STR("syntax err\n");
		return 1;
	}



	COMPR_REALESE = (float)atof(argv[1]);

	return 0;
}

U_BOOT_CMD(
	set_clu_release,     255,	0,	do_set_clu_release,
	"set_clu_release val ",
	"info   - \n"
);
