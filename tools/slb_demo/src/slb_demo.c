/*
*
* Rumpf einer "shared library"
*
* Andreas Kromke
* 22.10.97
*
*/

#include <portab.h>
#include <tos.h>
#include <tosdefs.h>
#pragma warn -par

typedef void *PD;

char *mem;			/* hier globalen Speicher */

/*****************************************************************
*
* Die init-Funktion wird einmal beim Laden der Bibliothek
* aufgerufen. Dabei l�uft sie im Proze� der Bibliothek,
* d.h. es k�nnen Dateien ge�ffnet und Speicher angefordert
* werden, die jeweils der Bibliothek geh�ren.
* Achtung: Auf die dabei ge�ffneten Dateien darf durch die
*          Bibliotheksfunktionen _NICHT_ zugegriffen werden,
*          weil diese im Kontext des Aufrufers laufen.
*
* Achtung: Die init-Funktion l�uft im Supervisormode, da eine
*          Bibliothek i.a. keinen Userstack hat.
*          Daher darf sie nicht zuviel Stack benutzen (max. 1kB)
*          und nicht zu lange laufen (weil das Multitasking
*          im Supervisormode blockiert ist).
*          Ggf. kann aber ein Userstack alloziert und in den
*          Usermode gewechselt werden.
*
*****************************************************************/

extern LONG cdecl slb_init( void )
{
	mem = Malloc(4096L);
	if	(mem)
		return(E_OK);
	else	return(ENSMEM);
}

/*****************************************************************
*
* Die exit-Funktion wird einmal beim Freigeben der Bibliothek
* aufgerufen. Dabei l�uft sie im Proze� der Bibliothek,
* d.h. es k�nnen Dateien ge�ffnet und Speicher angefordert
* werden, die jeweils der Bibliothek geh�ren.
*
* Achtung: Die exit-Funktion l�uft im Supervisormode, da eine
*          Bibliothek i.a. keinen Userstack hat.
*          Daher darf sie nicht zuviel Stack benutzen (max. 1kB)
*          und nicht zu lange laufen (weil das Multitasking
*          im Supervisormode blockiert ist).
*          Ggf. kann aber ein Userstack alloziert und in den
*          Usermode gewechselt werden.
*
*****************************************************************/

extern void cdecl slb_exit( void )
{
	Mfree(mem);
}


/*****************************************************************
*
* Die open-Funktion wird einmal beim �ffnen der Bibliothek
* durch einen Anwenderproze� aufgerufen. Dabei l�uft sie im
* Proze� des Aufrufers, d.h. es k�nnen Dateien ge�ffnet und
* Speicher angefordert werden, die jeweils dem Aufrufer geh�ren.
*
* Durch den Kernel ist sichergestellt, da� jeder Proze� die
* Bibliothek nicht mehrmals �ffnet und da� die Bibliothek immer
* ordnungsgem�� geschlossen wird.
*
* Achtung: Die open-Funktion l�uft im Usermode, und zwar mit dem
*          Userstack des Aufrufers. Das hei�t, da� der Aufrufer,
*          auch wenn er im Supervisormode l�uft, immer einen
*          ausreichend gro�en usp zur Verf�gung stellen mu�.
*
*****************************************************************/

extern LONG cdecl slb_open( PD *pd )
{
	return(E_OK);
}


/*****************************************************************
*
* Die close-Funktion wird einmal beim Schlie�en der Bibliothek
* durch einen Anwenderproze� aufgerufen. Dabei l�uft sie im
* Proze� des Aufrufers, d.h. es k�nnen Dateien ge�ffnet bzw.
* geschlossen und Speicher angefordert und freigegeben  werden,
* die jeweils dem Aufrufer geh�ren.
*
* Achtung: Die close-Funktion l�uft im Usermode, und zwar mit dem
*          Userstack des Aufrufers. Das hei�t, da� der Aufrufer,
*          auch wenn er im Supervisormode l�uft, immer einen
*          ausreichend gro�en usp zur Verf�gung stellen mu�.
*
*****************************************************************/

extern void cdecl slb_close( PD *pd )
{
}


/*****************************************************************
*
* Eine Beispiel-Bibliotheksfunktion.
* Sie wird im Kontext des Aufrufers ausgef�hrt, und zwar mit dem
* Stack des Aufrufers (je nach Status usp oder ssp).
*
* Es wird dringend empfohlen, die Funktionen einer SLB nur im
* Usermode aufzurufen, um die Kompatibilit�t zu sp�teren
* Implementationen zu wahren.
*
*****************************************************************/

extern LONG cdecl slb_fn0( PD *pd, LONG fn, WORD nargs, char *s )
{
	Cconws(s);
	Cconws("\r\nTaste: ");
	Cconin();
	return(E_OK);
}
