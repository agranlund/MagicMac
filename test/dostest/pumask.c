#include <tos.h>
#include <tosdefs.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>


int main( void )
{
	long ret;


	ret = Pumask(11);
	printf("\r\nHole  Umask: R�ckgabe: %ld\n", ret);
	ret = Pumask((int) ret);
	printf("\r\nSetze wieder altes: R�ckgabe: %ld\n", ret);
	return(0);
}
