#include <tos.h>
#include <string.h>
#include <tosdefs.h>
#include <stdio.h>
#include <stdlib.h>

int main( void )
{
	int	i;
	char	path[128];


	for	(;;)
		{
		for	(i = 0; i < 20; i++)
			{
			sprintf(path, "%d.dir", i);
			Dcreate(path);
			}
		for	(i = 0; i < 20; i++)
			{
			sprintf(path, "%d.dir", i);
			Ddelete(path);
			}
		}

	return(0);
}
