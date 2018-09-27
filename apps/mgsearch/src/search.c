/*******************************************************************
*
* Dieses Modul enth�lt die eigentliche Suche.
*
* min_xxx und max_xxx sind -1, wenn diese Kriterien nicht
* zu ber�cksichtigen sind.
*
****************************************************************/

#include <aes.h>
#include <tos.h>
#include <string.h>
#include <stdlib.h>
#include <tosdefs.h>
#include <portab.h>
#include <magx.h>
#include "gemutils.h"
#include "c:\pc\source\magxdesk.3\pattern.h"

static char _path[512];	/* f�r Rekursion */
static int depth;

static char _pattern[256];
static int  _min_date, _max_date;
static long _min_len, _max_len;
static int  (*_callback_ever)( void );
static void (*_callback_match)( char *path, char *fname );

static long _filesearch( void );

long filesearch( char *startpath, char *pattern,
			int min_date, int max_date,
			long min_len, long max_len,
			int (*callback_ever)( void ),
			void (*callback_match)(char *path, char *fname) )
{
	strcpy(_pattern, pattern);
	strcpy(_path, startpath);
	upperstring(_pattern);
	_min_date = min_date;
	_max_date = max_date;
	_min_len  = min_len;
	_max_len  = max_len;
	_callback_ever = callback_ever;
	_callback_match = callback_match;
	depth = 8;			/* Rekursionsz�hler */

	return(_filesearch());
}


static XATTR xa;

static long _filesearch( void )
{
	long dir;
	long ret;
	char fname[70];	/* Index (4 Bytes) + Dateiname */
	long err_xr;
	char *old;


	dir = Dopendir(_path, 0);		/* Modus: lange Namen */
	if	(dir < E_OK)
		return(dir);				/* Fehler */
	do	{

		/* Name einlesen */
		/* ------------- */

		ret = Dxreaddir(70, dir, fname, &xa, &err_xr);

		if	(_callback_ever())
			ret = EBREAK;

		if	(ret || err_xr)
			continue;
		if	(fname[4] == '.')
			{
			if	(!fname[5])
				continue;			/* "." */
			if	(fname[5] == '.' && !fname[6])
				continue;			/* ".." */
			}

		/* Auf Verzeichnis pr�fen, Rekursion */
		/* --------------------------------- */

		if	((xa.mode & S_IFMT) == 0040000)
			{
			old = _path + strlen(_path);
			strcpy(old, fname+4);
			strcat(old, "\\");
			if	(depth-- > 0)
				ret = _filesearch();
			else	ret = ERROR;
			depth++;
			*old = EOS;
			}
		else	{

		/* Kein Verzeichnis, also pattern match */
		/* ------------------------------------ */

			if	(pattern_match(_pattern, fname+4))
				{

				if	((_min_date != -1) &&
					(xa.mdate <= _min_date))
					continue;
				if	((_max_date != -1) &&
					(xa.mdate >= _max_date))
					continue;
				if	((_min_len != -1L) &&
					(xa.size <= _min_len))
					continue;
				if	((_max_len != -1L) &&
					(xa.size >= _max_len))
					continue;

				_callback_match(_path, fname+4);
				}
			}
		}
	while(!ret && !err_xr);
	return(Dclosedir(dir));
}
