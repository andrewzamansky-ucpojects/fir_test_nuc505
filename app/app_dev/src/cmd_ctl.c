
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

extern dsp_descriptor_t vb;
extern dsp_descriptor_t lpf_filter;
extern dsp_descriptor_t vb_final_filter;
extern dsp_descriptor_t hpf_filter_left;
extern dsp_descriptor_t hpf_filter_right;

extern dsp_descriptor_t stereo_to_mono;
extern dsp_descriptor_t limiter;
extern dsp_descriptor_t voice_3d;

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

	if(0 == strcmp(argv[1],"vb"))
	{
		if(0 == val)
		{
			dsp_management_api_set_module_control(&vb , DSP_MANAGEMENT_API_MODULE_CONTROL_BYPASS);
	//		dsp_management_api_set_module_control(&vb_final_filter , DSP_MANAGEMENT_API_MODULE_CONTROL_MUTE);
	//		dsp_management_api_set_module_control(&lpf_filter , DSP_MANAGEMENT_API_MODULE_CONTROL_BYPASS);
	//		dsp_management_api_set_module_control(&hpf_filter_left , DSP_MANAGEMENT_API_MODULE_CONTROL_BYPASS);
	//		dsp_management_api_set_module_control(&hpf_filter_right , DSP_MANAGEMENT_API_MODULE_CONTROL_BYPASS);
		}
		else
		{
			dsp_management_api_set_module_control(&vb , DSP_MANAGEMENT_API_MODULE_CONTROL_ON);
	//		dsp_management_api_set_module_control(&vb_final_filter , DSP_MANAGEMENT_API_MODULE_CONTROL_ON);
	//		dsp_management_api_set_module_control(&lpf_filter , DSP_MANAGEMENT_API_MODULE_CONTROL_ON);
	//		dsp_management_api_set_module_control(&hpf_filter_left , DSP_MANAGEMENT_API_MODULE_CONTROL_ON);
	//		dsp_management_api_set_module_control(&hpf_filter_right , DSP_MANAGEMENT_API_MODULE_CONTROL_ON);
		}
	}
	else if(0 == strcmp(argv[1],"loopback"))
	{
		loopback = val;
	}
	else if(0 == strcmp(argv[1],"lf_path"))
	{
		if(0 == val)
		{
			dsp_management_api_set_module_control(&stereo_to_mono , DSP_MANAGEMENT_API_MODULE_CONTROL_MUTE);
		}
		else
		{
			dsp_management_api_set_module_control(&stereo_to_mono , DSP_MANAGEMENT_API_MODULE_CONTROL_ON);
		}
	}
	else if(0 == strcmp(argv[1],"hf_path"))
	{
		if(0 == val)
		{
			dsp_management_api_set_module_control(&hpf_filter_left , DSP_MANAGEMENT_API_MODULE_CONTROL_MUTE);
			dsp_management_api_set_module_control(&hpf_filter_right , DSP_MANAGEMENT_API_MODULE_CONTROL_MUTE);
		}
		else
		{
			dsp_management_api_set_module_control(&hpf_filter_left , DSP_MANAGEMENT_API_MODULE_CONTROL_ON);
			dsp_management_api_set_module_control(&hpf_filter_right , DSP_MANAGEMENT_API_MODULE_CONTROL_ON);
		}
	}
	else if(0 == strcmp(argv[1],"limiter"))
	{
		if(0 == val)
		{
			dsp_management_api_set_module_control(&limiter , DSP_MANAGEMENT_API_MODULE_CONTROL_BYPASS);
		}
		else
		{
			dsp_management_api_set_module_control(&limiter , DSP_MANAGEMENT_API_MODULE_CONTROL_ON);
		}
	}
	else if(0 == strcmp(argv[1],"3d"))
	{
		if(0 == val)
		{
			dsp_management_api_set_module_control(&voice_3d , DSP_MANAGEMENT_API_MODULE_CONTROL_BYPASS);
		}
		else
		{
			dsp_management_api_set_module_control(&voice_3d , DSP_MANAGEMENT_API_MODULE_CONTROL_ON);
		}
	}
	else if (0 == strcmp(argv[1],"s2m"))
	{
		dsp = &stereo_to_mono;
		SHELL_REPLY_STR("stereo_to_mono -> ");
		do_ctl_aux(dsp, val);

	}
	else if (0 == strcmp(argv[1],"vb_m"))
	{
		dsp = &vb;
		SHELL_REPLY_STR("vb -> ");
		do_ctl_aux(dsp, val);
	}
	else if (0 == strcmp(argv[1],"vb_f"))
	{
		dsp = &vb_final_filter;
		SHELL_REPLY_STR("vb_final -> ");
		do_ctl_aux(dsp, val);
	}
	else if (0 == strcmp(argv[1],"lpf"))
	{
		dsp = &lpf_filter;
		SHELL_REPLY_STR("lpf_filter -> ");
		do_ctl_aux(dsp, val);
	}
	else if (0 == strcmp(argv[1],"hpf"))
	{
		dsp = &hpf_filter_left;
		SHELL_REPLY_STR("hpf_filter_left -> ");
		do_ctl_aux(dsp, val);

		dsp = &hpf_filter_right;
		SHELL_REPLY_STR("hpf_filter_right -> ");
		do_ctl_aux(dsp, val);
	}
	os_mutex_give(control_mutex);

	return 0;
}

U_BOOT_CMD(
	ctl,     255,	0,	do_ctl,
	"ctl vb/loopback/lf_path/hf_path/compressor/3d  0/1",
	"info   - \n"
);
