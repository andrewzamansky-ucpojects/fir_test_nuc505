/*
 * file : VIRTUAL_BASS.h
 *
 *
 *
 *
 *
 *
 */

#ifndef _VIRTUAL_BASS_H
#define _VIRTUAL_BASS_H

#include "virtual_bass_config.h"
#include "dev_managment_api.h" // for device manager defines and typedefs
#include "src/_virtual_bass_prerequirements_check.h" // should be after {virtual_bass_config.h,dev_managment_api.h}


/***************   typedefs    *******************/



typedef struct
{
	float envelope_folower;
	float prev_harmonic_out;
	float prev_x;
} VIRTUAL_BASS_Instance_t;




#endif /* */
