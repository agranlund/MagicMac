/* Fehlercodes */

#define EDITERR_BUFFER_FULL	-200L

/* Struktur f�r ein erweitertes EDIT-Feld */

typedef struct {

/* �ffentlicher Teil */

	LONG	magic;			/* Kennung */
	unsigned char *buf;		/* Text */
	unsigned char *tmplt;	/* Maske, z.B. "________.___" */
	unsigned char *valid;	/* z.B. "xxxxxxxxxxx" */
	LONG buflen;			/* Pufferl�nge OHNE (!) EOS */
	LONG curr_tlen;		/* aktuelle Textl�nge ohne EOS */
	LONG	nlines;			/* Anzahl Zeilen im Text */
	LONG yscroll;			/* oben dargestellte Zeile */
	WORD	ob_w;
	WORD ob_h;			/* Objektgr��en */
	WORD autowrap;			/* Automatischer Zeilenumbruch in Pixeln */
	WORD max_linew;		/* Max. Zeilenbreite */
	WORD	tabwidth;			/* Tabulatorweite in Pixeln */
	WORD bcolour;			/* Hintergrundfarbe */
	WORD tcolour;			/* Textfarbe */
	WORD	fontID;
	WORD fontH;
	WORD fontPix;			/* 1:vst_height() 0:vst_point() */
	WORD mono;			/* "monospaced" font */
	WORD dirty;			/* Text wurde ge�ndert */

/* privater Teil */

	WORD charW;			/* => Zeichenbreite */
	WORD charH;			/* Zeilenh�he */
	WORD lvis;			/* Anzahl sichtbarer Zeilen */
	WORD leff;			/*   davon g�ltige */
	struct _xed_li
		{
		unsigned char *line;	/* sichtbare Zeile */
		LONG len;				/* zugeh�rige L�nge */
		WORD lextra;			/* L�nge der Zeilenendekennung */
		} *lines;
	WORD xscroll;			/* horiz. Scroll-Offset in Pixeln */
	/* Infos f�r Cursor */
	unsigned char *tcurs;	/* gew�nschte Cursorpos. im Text */
	WORD	curs_hidecnt;		/* Cursor abgeschaltet? */
	WORD ccurs_x;			/* Spalte (in Zeichen) des Cursors */
	WORD ccurs_y;			/* (sichtbare) Zeile des Cursors */
	WORD pcurs_x;			/*  Cursorpos. in Pixeln */
	WORD pcurs_y;
	WORD	pcursth_x;		/* f�r Cursor hoch/runter */
	/* Infos f�r Selektion */
	unsigned char *bsel,*esel;	/* selektierter Bereich */
} XEDITINFO;

extern WORD cdecl xeditob_userdef( PARMBLK *pb );
extern XEDITINFO *edit_create( void );
extern WORD edit_open( OBJECT *ob, XEDITINFO *xi );
extern void edit_close( XEDITINFO *xi );
extern void edit_delete( XEDITINFO *xi );
extern WORD edit_evnt( OBJECT *tree, WORD obj, WORD whdl, EVNT *ev,
				XEDITINFO *xi, LONG *errcode );
extern WORD edit_cursor(OBJECT *tree, WORD obj, WORD whdl,
			WORD show, XEDITINFO *xi);
extern void edit_set_buf( XEDITINFO *xi,
			unsigned char *buf, LONG buflen );
extern void edit_set_format( XEDITINFO *xi,
			WORD tabwidth, WORD autowrap );
extern WORD edit_set_font( XEDITINFO *xi, WORD fontID,
						WORD fontH, WORD fontPix, WORD mono );
extern unsigned char *edit_get_cursor( XEDITINFO *xi );
extern WORD edit_set_cursor( OBJECT *tree, WORD obj, WORD whdl,
					XEDITINFO *xi, unsigned char *s );
extern void edit_set_scroll_and_cpos(
			XEDITINFO *xi,
			WORD xscroll,
			LONG yscroll,
			unsigned char *cyscroll,
			unsigned char *curpos,
			WORD cx, WORD cy);
extern WORD edit_scroll(OBJECT *tree, WORD obj,
				WORD whdl, XEDITINFO *xi,
				LONG yscroll, WORD xscroll);
extern WORD edit_resized( OBJECT *ob, XEDITINFO *xi,
					WORD *oldrh, WORD *newrh );


/* F�r FTEXT */

extern WORD init_ftextob( OBJECT *tree, WORD obj, USERBLK *ub,
			TEDINFO *te);
