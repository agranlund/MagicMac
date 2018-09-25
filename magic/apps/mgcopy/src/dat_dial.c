/*
*
* Enth�lt die spezifischen Routinen f�r den Dialog
* "Datei existiert schon"
*
*/

#include <mgx_dos.h>
#include <mt_aes.h>
#include <string.h>
#include <stdlib.h>
#include "gemut_mt.h"
#include "mgcopy.h"
#include "dat_dial.h"
#include "globals.h"


#define TRUE   1
#define FALSE  0
#define EOS    '\0'
#ifndef NULL
#define NULL        ( ( void * ) 0L )
#endif
#define USA	0
#define FRG	1

static TEDINFO *t1,*t2;
static XTED xt1,xt2;
static char alt[66],neu[66],tmplt[66];
static char *tmplt_8_3 = "________.___";


/*********************************************************************
*
* Initialisierung
*
*********************************************************************/

void dat_dial_init_rsc( void )
{
	rsrc_gaddr(0, T_DATEXI, &adr_dat);
	t1 = (adr_dat+DATEXI_O)->ob_spec.tedinfo;
	t2 = (adr_dat+DATEXI_N)->ob_spec.tedinfo;
	t1->te_ptext = alt;
	t2->te_ptext = neu;
	t1->te_just = t2->te_just = TE_LEFT;

	memset(tmplt, '_', 64);	/* neue Schablone */
	tmplt[65] = '\0';
	xt1.xte_ptmplt = xt2.xte_ptmplt = tmplt;
}


/*********************************************************************
*
* Dialog schlie�en
*
*********************************************************************/

void close_dat_dialog( void )
{
	if	(d_dat)
		{
		wdlg_close(d_dat, NULL, NULL);
		wdlg_delete(d_dat);
		d_dat = NULL;
		}
}


/*********************************************************************
*
* Behandelt die Exit- Objekte des Dialogs
* Das Exit-Objekt <objnr> wurde mit <clicks> Klicks angew�hlt.
*
* objnr = -1:	Initialisierung.
*			d->user_data und d->dialog_tree initialisieren!
*		-2:	Nachricht int data[8] wurde �bergeben
* 		-3:	Fenster wurde durch Closebutton geschlossen.
*		-4:	Programm wurde beendet.
*
* R�ckgabe:	0	Dialog schlie�en
*			< 0	Fehlercode
*
*********************************************************************/

#pragma warn -par
WORD cdecl hdl_dat( DIALOG *d, EVNT *events, WORD exitbutton, WORD clicks, void *data )
{
	static int is_8_3;
	OBJECT *tree;
	FILEDESCR *fd;


	/* 1. Fall: Dialog soll ge�ffnet werden */
	/* ------------------------------------ */

	tree = adr_dat;
	fd = (FILEDESCR *) wdlg_get_udata(d);

	if	(exitbutton == HNDL_INIT)
		{
		filetype ftype;
		char *titel;
		char *name;


		if	(d_dat)			/* Dialog ist schon ge�ffnet ! */
			return(0);

		fd -> answ = WAITING;
		d_dat = d;

		ob_dsel(adr_dat, EX_OK);
		ob_dsel(adr_dat, EX_AB);
		ob_dsel(adr_dat, EX_SKIP);
		ob_dsel(adr_dat, EX_USE);

		titel = (adr_dat+DATEXI_T)->ob_spec.free_string;
		ftype = fd -> ftype;
		name = fd -> fname;

		is_8_3 = fd -> is_8_3;

		if	(is_8_3)
			{
			t1->te_ptmplt = t2->te_ptmplt = tmplt_8_3;
			t1->te_pvalid = t2->te_pvalid = "f";
			(adr_dat+DATEXI_O)->ob_width =
			(adr_dat+DATEXI_N)->ob_width = 13*gl_hwchar;
			t1->te_txtlen = t2->te_txtlen = 12;
			t1->te_tmplen = t2->te_tmplen = 13;
			}
		else	{
			xt1.xte_pvalid = xt2.xte_pvalid = "m";
			xt1.xte_scroll = xt2.xte_scroll = 0;
			t1->te_ptmplt = t2->te_ptmplt = NULL;
			t1->te_pvalid = (void *) &xt1;
			t2->te_pvalid = (void *) &xt2;
			t1->te_tmplen = t2->te_tmplen =
			t1->te_txtlen = t2->te_txtlen =
					fd->maxnamelen + 1;
			xt1.xte_vislen = fd->maxnamelen;
			if	(xt1.xte_vislen > 20)
				xt1.xte_vislen = 20;
			xt2.xte_vislen = xt1.xte_vislen;
			(adr_dat+DATEXI_O)->ob_width =
			(adr_dat+DATEXI_N)->ob_width = 
				xt1.xte_vislen*gl_hwchar;
			}

		if	(ftype == FOLDER)
			{
			strcpy(titel, Rgetstring(STR_FOLDER, NULL));
			objs_hide(adr_dat, EX_SKIP, 0);
			objs_unhide(adr_dat, EX_USE, 0);
			*neu = EOS;
			}
		else	{
			if	(is_8_3)
				fname_int(name, neu);
			else	strcpy(neu, name);
			objs_unhide(adr_dat, EX_SKIP, 0);
			objs_hide(adr_dat, EX_USE, 0);
			strcpy(titel, Rgetstring(
				(ftype == ORDINARYFILE) ? STR_FILE : STR_ALIAS,
				NULL));
			}
		strcat(titel, Rgetstring((clicks) ? STR_GIVENAME : STR_EXISTS,
							NULL));

		if	(is_8_3)
			fname_int(name, alt);
		else	strcpy(alt, name);

		return(1);
		}

	/* 2. Fall: Nachricht mit Code >= 1040 empfangen */
	/* --------------------------------------------- */

	if	(exitbutton == HNDL_MESG)	/* Wenn Nachricht empfangen... */
		{
		return(1);		/* weiter */
		}

	/* 3. Fall: Dialog soll geschlossen werden */
	/* --------------------------------------- */

	if	(exitbutton == HNDL_CLSD)	/* Wenn Dialog geschlossen werden soll... */
		{
		fd -> answ = CANCEL;
		close_dialog:
		return(0);		/* ...dann schlie�en wir ihn auch */
		}

	if	(exitbutton < 0)	/* unbekannte Unterfunktion */
		return(1);

	/* 4. Fall: Exitbutton wurde bet�tigt */
	/* ---------------------------------- */

	if	(clicks != 1)
		goto ende;

	if	(exitbutton == EX_AB)			/* Abbruch */
		{
		fd -> answ = CANCEL;
		goto close_dialog;
		}

	if	(exitbutton == EX_OK)			/* OK */
		{
		if	(!*neu)
			goto ende;			/* Name ung�ltig */
		fd -> answ = OK;
		if	(is_8_3)
			fname_ext(neu, fd -> fname);
		else	strcpy(fd -> fname, neu);
		goto close_dialog;
		}

	if	((exitbutton == EX_SKIP) ||
		 (exitbutton == EX_USE))			/* �berspringen */
		{
		fd -> answ = SKIP;
		goto close_dialog;
		}

	return(1);

	ende:
	ob_dsel(tree, exitbutton);
	subobj_wdraw(d, exitbutton, exitbutton, 0);
	return(1);		/* weiter */
}
