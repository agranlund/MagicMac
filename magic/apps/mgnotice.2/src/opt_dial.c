/*
*
* Enth�lt die spezifischen Routinen f�r den Dialog
* "Voreinstellungen"
*
*/

#include <mgx_dos.h>
#include <mt_aes.h>
#include <string.h>
#include <stdlib.h>
#include "gemut_mt.h"
#include "windows.h"
#include "globals.h"
#include "mgnotice.h"

static struct prefs localprefs;

/*********************************************************************
*
* Initialisierung
*
*********************************************************************/

void options_dial_init_rsc( void )
{
}


/*********************************************************************
*
* Behandelt die Exit- Objekte des Dialogs
* Beim Initialisieren wird in <data> ggf. ein Zeiger auf einen
* Text �bergeben.
*
*********************************************************************/

#pragma warn -par
WORD	cdecl hdl_options( DIALOG *d, EVNT *events, WORD exitbutton,
				WORD clicks, void *data )
{
	OBJECT *tree;
	int obj;
	long id,pt;
	int mono;



	/* 1. Fall: Dialog soll ge�ffnet werden */
	/* ------------------------------------ */

	tree = adr_options;

	if	(exitbutton == HNDL_INIT)
		{
		if	(d_options)			/* Dialog ist schon ge�ffnet ! */
			return(0);

		localprefs = prefs;
		itoa(localprefs.fontH,
			tree[OPTIONS_FONTSIZE].ob_spec.tedinfo->te_ptext,
			10);
		tree[OPTIONS_FONTNAME].ob_spec.tedinfo->te_ptext =
			localprefs.fontname;
		tree[OPTIONS_COLOUR].ob_spec.tedinfo->te_ptext
				= adr_colour[localprefs.colour+1].ob_spec.tedinfo->te_ptext;
		tree[OPTIONS_COLOUR].ob_spec.tedinfo->te_color
				= adr_colour[localprefs.colour+1].ob_spec.tedinfo->te_color;
		ob_dsel(tree, OPTIONS_OK);
		ob_dsel(tree, OPTIONS_CANCEL);

		return(1);
		}

	/* 2. Fall: Nachricht mit Code >= 1040 empfangen */
	/* --------------------------------------------- */

	if	(exitbutton == HNDL_MESG)	/* Wenn Nachricht empfangen... */
		{
		switch(events->msg[0])
			{
/*
			 case WM_ALLICONIFY:
	
			 case WM_ICONIFY:
			 	wind_update(BEG_UPDATE);
			 	wdlg_set_iconify(d, (GRECT *) (events->msg+4),
	 							" MGCOPY ",
	 							adr_beg_iconified, 1);
			 	is_iconified = TRUE;
			 	wind_update(END_UPDATE);
			 	break;
	
			 case WM_UNICONIFY:
			 	wind_update(BEG_UPDATE);
			 	wdlg_set_uniconify(d, (GRECT *) (events->msg+4),
		 							Rgetstring(STR_MAINTITLE, global),
		 							adr_beg);
			 	is_iconified = FALSE;
			 	wind_update(END_UPDATE);
				break;
*/	
			}
		return(1);		/* weiter */
		}

	/* 3. Fall: Dialog soll geschlossen werden */
	/* --------------------------------------- */

	if	(exitbutton == HNDL_CLSD)	/* Wenn Dialog geschlossen werden soll... */
		{
		close_dialog:
		return(0);		/* ...dann schlie�en wir ihn auch */
		}

	if	(exitbutton < 0)	/* unbekannte Unterfunktion */
		return(1);

	/* 4. Fall: Exitbutton wurde bet�tigt */
	/* ---------------------------------- */

	if	(clicks != 1)
		goto ende;

	if	(exitbutton == OPTIONS_COLOUR)
		{
		adr_colour->ob_x = tree->ob_x+tree[exitbutton].ob_x;
		adr_colour->ob_y = tree->ob_y+tree[exitbutton].ob_y;
		adr_colour->ob_y -= adr_colour[localprefs.colour+1].ob_y;
		obj = form_popup(adr_colour,0,0);
		if	(obj > 0)
			{
			tree[exitbutton].ob_spec.tedinfo->te_ptext
				= adr_colour[obj].ob_spec.tedinfo->te_ptext;
			tree[exitbutton].ob_spec.tedinfo->te_color
				= adr_colour[obj].ob_spec.tedinfo->te_color;
			localprefs.colour = obj-1;
			goto ende;
			}

		}

	if	((exitbutton == OPTIONS_FONTNAME) ||
		 (exitbutton == OPTIONS_FONTSIZE))
		{
		id = localprefs.fontID;
		pt = (((long) localprefs.fontH)<<16L);
		mono = !localprefs.fontprop;
		if	(dial_font( &id, &pt, &mono, localprefs.fontname ))
			{
			localprefs.fontID = (int) id;
			localprefs.fontH = (int) (pt >> 16L);
			localprefs.fontprop = !mono;
			itoa(localprefs.fontH,
				tree[OPTIONS_FONTSIZE].ob_spec.tedinfo->te_ptext,
				10);
			subobj_wdraw(d, OPTIONS_FONTNAME, OPTIONS_FONTNAME, 0);
			subobj_wdraw(d, OPTIONS_FONTSIZE, OPTIONS_FONTSIZE, 0);
			}
		}

	if	(exitbutton == OPTIONS_SAVE)
		{
		prefs = localprefs;
		save_options();
		goto ende;
		}

	if	(exitbutton == OPTIONS_CANCEL)		/* Abbruch */
		{
		goto close_dialog;
		}

	if	(exitbutton == OPTIONS_OK)			/* OK */
		{
		prefs = localprefs;
		return(0);
		}

	return(1);

	ende:
	ob_dsel(tree, exitbutton);
	subobj_wdraw(d, exitbutton, exitbutton, 0);
	return(1);		/* weiter */
}
#pragma warn +par
