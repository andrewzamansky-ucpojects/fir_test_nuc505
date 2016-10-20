
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


extern uint8_t loopback ;
extern os_mutex_t  control_mutex;

void do_ctl_aux(pdsp_descriptor dsp, float val)
{
	if(0 == val)
	{
		dsp_management_api_set_module_control(dsp , DSP_MANAGEMENT_API_MODULE_CONTROL_MUTE);
		SHELL_REPLY_STR("MUTE\n");
	}
	else if  (2 == val)
	{
		dsp_management_api_set_module_control(dsp , DSP_MANAGEMENT_API_MODULE_CONTROL_BYPASS);
		SHELL_REPLY_STR("BYPASS\n");
	}
	else
	{
		dsp_management_api_set_module_control(dsp , DSP_MANAGEMENT_API_MODULE_CONTROL_ON);
		SHELL_REPLY_STR("ON\n");
	}
}
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
	pdsp_descriptor dsp;

	if(argc < 3)
	{
		SHELL_REPLY_STR("syntax err\n");
		return 1;
	}


	val = (float)atoi(argv[2]);

	os_mutex_take_infinite_wait(control_mutex);

	if(0 == strcmp(argv[1],"loopback"))
	{
		loopback = val;
	}
	os_mutex_give(control_mutex);

	return 0;
}

U_BOOT_CMD(
	ctl,     255,	0,	do_ctl,
	"ctl vb/loopback/lf_path/hf_path/compressor/3d  0/1",
	"info   - \n"
);
