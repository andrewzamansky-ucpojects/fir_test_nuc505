/*
 * file : project_defines.h
 *
 *
 *
 *
 *
 *
 */

#ifndef _project_defines_H
#define _project_defines_H


#include <stddef.h> // include for NULL


#ifdef __cplusplus
	#define  EXTERN_C_FUNCTION    extern "C"
#else
	#define  EXTERN_C_FUNCTION
#endif


/* Configuration of the Cortex-M# Processor and Core Peripherals */
#define __CM4_REV                 0x0201    /*!< Core Revision r2p1                               */
#define __NVIC_PRIO_BITS          4         /*!< Number of Bits used for Priority Levels          */
#define __Vendor_SysTickConfig    1         /*!< Set to 1 if different SysTick Config is used     */
#define __MPU_PRESENT             1         /*!< MPU present or not                               */
#define __FPU_PRESENT             1         /*!< FPU present or not                               */
#define __FPU_USED				 1

#define ARM_MATH_CM4	1

//#include "NUC505Series.h"

/**** freertos defines ****/
#define ADDITIONAL_STACK_SAFATY_MARGIN	 20
#define DEFINE_STACK_SIZE(n)    (n + ADDITIONAL_STACK_SAFATY_MARGIN)


#define INTERRUPT_LOWEST_PRIORITY    15

#define I2S_BUFF_LEN 		512
#define LATENCY_LENGTH		64
#define	NUM_OF_BYTES_PER_AUDIO_WORD		2// 2- 16bits , 4- 32bits

#define 	_USE_DSP_

#endif /* */
