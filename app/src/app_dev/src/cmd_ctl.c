
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


extern uint8_t loopback ;
extern uint8_t vb_on;
extern uint8_t lf_path;
extern uint8_t hf_path;
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
int do_ctl (cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	uint8_t val;


	if(argc < 3)
	{
		SHELL_REPLY_STR("syntax err\n");
		return 1;
	}


	val = (float)atoi(argv[2]);

	if(0 == strcmp(argv[1],"vb"))
	{
		vb_on = val;
	}
	else if(0 == strcmp(argv[1],"loopback"))
	{
		loopback = val;
	}
	else if(0 == strcmp(argv[1],"lf_path"))
	{
		lf_path = val;
	}
	else if(0 == strcmp(argv[1],"hf_path"))
	{
		hf_path = val;
	}

	return 0;
}

U_BOOT_CMD(
	ctl,     255,	0,	do_ctl,
	"set_comressor band_num filter_type Fc QValue Gain",
	"info   - \n"
);

