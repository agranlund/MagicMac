#include <tos.h>
#include <tosdefs.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>


int main( void )
{
	long ret;


	ret = Pusrval(11);
	printf("\r\nHole Usrval: R�ckgabe: %ld\n", ret);
	ret = Pusrval((int) ret);
	printf("\r\nSetze wieder altes: R�ckgabe: %ld\n", ret);
	return(0);
}
