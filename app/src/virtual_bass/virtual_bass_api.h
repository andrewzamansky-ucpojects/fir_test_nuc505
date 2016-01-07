
#ifndef _virtual_bass_API_H_
#define _virtual_bass_API_H_

#include "virtual_bass_config.h"
#include "dev_managment_api.h" // for device manager defines and typedefs
#include "dsp_managment_api.h" // for device manager defines and typedefs
#include "src/_virtual_bass_prerequirements_check.h" // should be after {virtual_bass_config.h,dev_managment_api.h}

#include "common_dsp_api.h"

/*****************  defines  **************/


/**********  define API  types ************/





/**********  define API  functions  ************/


uint8_t  virtual_bass_api_init_dsp_descriptor(pdsp_descriptor aDspDescriptor);

//#include "src/_virtual_bass_static_dev_macros.h"
//
//#define VIRTUAL_BASS_API_CREATE_STATIC_DEV(dev,dev_name)  __VIRTUAL_BASS_API_CREATE_STATIC_DEV(dev,dev_name )
//

#endif
