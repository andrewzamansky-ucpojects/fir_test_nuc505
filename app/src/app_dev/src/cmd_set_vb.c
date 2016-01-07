
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

#include "dsp_managment_api.h"

extern float vb_volume ;
extern float COMPR_ATTACK ;
extern float COMPR_REALESE ;
extern float HARMONICS_GAIN ;

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



	if(argc < 5)
	{
		SHELL_REPLY_STR("syntax err\n");
		return 1;
	}

	// should be transformed to ioctls !!

	vb_volume = (float)atof(argv[1]) /100 * 0.85;
	HARMONICS_GAIN = (float)atof(argv[2]);
	COMPR_ATTACK = (float)atof(argv[3]);
	COMPR_REALESE = (float)atof(argv[4]);

	return 0;
}

U_BOOT_CMD(
	set_vb,     255,	0,	do_set_vb,
	"set_vb  vol harmonic_vol attack release",
	"info   - \n"
);
