/*********************************************************************
*
*  FLP_PAR1              Andreas Kromke, 27.8.94
*  ========
*
* schaltet die parallele Floppy�bertragung ein.
*
*********************************************************************/

#include <tos.h>
#include <magx.h>

int main( void )
{
	long mode;

	mode = Sconfig(0, 0L);
	Sconfig(1, mode | SCB_FLPAR);		/* Parallelbetrieb ein */
	return(0);
}
