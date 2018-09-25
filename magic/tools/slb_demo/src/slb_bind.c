/*
*
* Binding f�r die Benutzung einer "shared library"
*
* Andreas Kromke
* 22.10.97
*
*/

#include <mgx_dos.h>

/*****************************************************************
*
* �ffnet eine "shared lib".
*
* Eingabe:
*	name			Name der Bibliothek inkl. Extension.
*	path			Suchpfad mit '\', optional
*	min_ver		Minimale ben�tigte Versionsnummer
* R�ckgabe:
*	sl			Bibliotheks-Deskriptor
*	fn			Funktion zum Aufruf einer Bibliotheksfunktion
*	<ret>		tats�chliche Versionsnummer oder Fehlercode
*
*****************************************************************/

LONG Slbopen( char *name, char *path, LONG min_ver,
				SHARED_LIB *sl, SLB_EXEC *fn,
				LONG param )
{
	return(gemdos(0x16, name, path, min_ver, sl, fn, param));
}


/*****************************************************************
*
* Schlie�t eine "shared lib".
*
* R�ckgabe:
*	<ret>		EACCDN, falls Lib nicht ge�ffnet
*
*****************************************************************/

extern LONG Slbclose( SHARED_LIB sl )

{
	return(gemdos(0x17, sl));
}
