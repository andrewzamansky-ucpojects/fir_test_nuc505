
/*
 *  cmd_set_comressor.c
 */
#include "_project.h"
#include <stdio.h>
#include <string.h>

#include <stdlib.h>

#include "u-boot/include/command.h"
#include "shell_api.h"



extern uint8_t cpu_stat_report_interval;
extern os_mutex_t  control_mutex;

/*
 * Subroutine:  do_set_cpu_stat_interval
 *
 * Description:
 *
 * Inputs:
 *
 * Return:      None
 *
 */

int do_set_cpu_stat_interval (cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{

	uint32_t interval;

	if(argc < 2)
	{
		SHELL_REPLY_STR("syntax err\n");
		return 1;
	}


	os_mutex_take_infinite_wait(control_mutex);

	interval = (uint32_t)atol(argv[1]);
	if (interval > 255)
	{
		SHELL_REPLY_STR("out of range\n");
		return 1;
	}

	cpu_stat_report_interval = (uint8_t) interval;
	os_mutex_give(control_mutex);

	return 0;
}

U_BOOT_CMD(
	set_cpu_stat_interval,     255,	0,	do_set_cpu_stat_interval,
	"set_cpu_stat_interval <seconds> (0 = mute)",
	"info   - \n"
);
