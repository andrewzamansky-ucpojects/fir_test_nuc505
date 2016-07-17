/*

*/

#include "_project_typedefs.h"
#include "_project_defines.h"
#include "_project_func_declarations.h"

#include "assert.h"
#include "os_wrapper.h"

/*-----------------------------------------------------------*/
extern void init( void );



/*-----------------------------------------------------------*/
int main( void )
{
	os_init();

	init();

    os_start();

	/* Will only get here if there was not enough heap space to create the
	idle task. */
	return 0;
}
