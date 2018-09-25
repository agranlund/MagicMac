#include <tos.h>
#include <tosdefs.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>


int main( int argc, char *argv[] )
{
	long ret;
	char *s = "";
	unsigned long attr = 0;
	int wr_flag = 0;


	if	(argc == 3)
		{
		wr_flag = 1;
		attr = strtoul(argv[2], &s, 0);
		}

	if	((argc < 2) || (argc > 3) || (*s) || (attr > 65535L))
		{
		Cconws("Syntax: FATTRIB path [[0x]new_attr]\r\n");
		return(1);
		}

	ret = Fattrib(argv[1], wr_flag, (int) attr);
	printf("\r\nR�ckgabe: %ld\n", ret);
	if	(ret >= 0)
		{
		if	(ret & 0x40     ) Cconws("Symlink ");
		if	(ret & F_ARCHIVE) Cconws("Archiv ");
		if	(ret & F_SUBDIR ) Cconws("Subdir ");
		if	(ret & F_VOLUME ) Cconws("Label ");
		if	(ret & F_SYSTEM ) Cconws("System ");
		if	(ret & F_HIDDEN ) Cconws("Hidden ");
		if	(ret & F_RDONLY ) Cconws("Rdonly ");
		Cconws("\r\n");
		ret = 0;
		}
	return((int)ret);
}
