#include <tos.h>
#include <tosdefs.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>


int main( void )
{
	long ret;


	ret = Psetlimit(3, -1L);
	printf("\r\nHole  Limit: R�ckgabe: %ld\n", ret);
	ret = Psetlimit(3, 128000L);
	printf("\r\nSetze Limit: R�ckgabe: %ld\n", ret);
	ret = Psetlimit(3, -1L);
	printf("\r\nHole  Limit: R�ckgabe: %ld\n", ret);
	Pexec(0, "c:\\kaosdesk\\mcmd.tos", "", NULL);
	return((int)ret);
}
