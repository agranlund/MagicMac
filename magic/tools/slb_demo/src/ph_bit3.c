/*
*
* Programm zur Manipulation des Bit 3 im Programmheader.
* MagiC ben�tigt dieses Bit, das i.a. Null ist, daf�r, um
* dem Programm nur den minimal notwendigen Speicher zuzuweisen,
* d.h. nur Basepage+Text+Daten+Symbol+Bss.
* Wird insbesondere f�r alle SharedLibraries ben�tigt.
*
* Andreas Kromke
* 25.10.97
*
*/

#include <portab.h>
#include <tos.h>
#include <tosdefs.h>
#include <stdio.h>

/* ProgramHeader, Programmkopf f�r ausf�hrbare Dateien                  */
/************************************************************************/

typedef struct {
 WORD ph_branch;		/* 0x00: mu� 0x601a sein */
 LONG ph_tlen;			/* 0x02: L�nge  des TEXT - Segments */
 LONG ph_dlen;			/* 0x06: L�nge  des DATA - Segments */
 LONG ph_blen;			/* 0x0a: L�nge  des BSS  - Segments */
 LONG ph_slen;			/* 0x0e: L�nge  der Symboltabelle */
 LONG ph_res1;			/* 0x12: von PureC ben�tigt */
 LONG ph_flags;		/* 0x16:	Bit 0: Heap nicht l�schen */
					/*		Bit 1: Laden ins FastRAM	 */
					/*		Bit 2: Malloc aus FastRAM */
					/*		Bit 3: nur t+d+b+s (MagiC 5.20) */
					/*		Bit 4,5,6,7: Speicherschutz (MiNT) */
					/*		Bit 8: unbenutzt		 */
					/*		Bit 9: unbenutzt		 */
					/*		Bit 10: unbenutzt		 */
					/*		Bit 11: SharedText (MiNT) */
					/*		Bit 12: unbenutzt		 */
					/*		Bit 13: unbenutzt		 */
					/*		Bit 14: unbenutzt		 */
					/*		Bit 15: unbenutzt		 */
					/*		Bits 31..28: TPA-Size	 */
					/*		 (mal 128k + 128k: Mindestgr. Heap */
 WORD ph_reloflag;		/* 0x1a: ungleich Null => nicht relozieren */
} PH;



WORD main( WORD argc, char *argv[] )
{
	PH ph;
	WORD ret = 0;
	LONG err;
	WORD f;

	for	(argc--,argv++; argc; argc--,argv++)
		{
		Cconws("Datei: ");
		Cconws(*argv);
		err = Fopen(*argv, RMODE_RW);	/* zum Lesen+Schreiben �ffnen */
		f = (WORD) err;	/* Datei-Handle */
		err = Fread(f, sizeof(PH), &ph);
		if	((err != sizeof(PH)) || (ph.ph_branch != 0x601a))
			{
			err = EPLFMT;
			goto nextone;
			}
		Fseek(0L, f, 0);		/* Dateizeiger an den Anfang */
		ph.ph_flags |= 8;		/* Bit 3 setzen */
		err = Fwrite(f, sizeof(PH), &ph);

	 nextone:
		if	(f > 0)
			Fclose(f);
		if	(err < 0)
			{
			printf(" => Fehler %ld", err);
			ret = (WORD) err;
			}
		Cconws("\r\n");
		}
	return(ret);
}