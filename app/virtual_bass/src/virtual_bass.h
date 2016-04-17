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

#include "src/_virtual_bass_prerequirements_check.h"


/***************   typedefs    *******************/



typedef struct
{
	float envelope_folower;
	float prev_harmonic_out;
	float prev_x;
} VIRTUAL_BASS_Instance_t;




#endif /* */
