#if  !defined( __VTSTRUCT__ )
#define __VTSTRUCT__

#define	TBUFSIZE	256			/* Gr��e des Tastaturbuffers	*/
#define	MAX_COLUMNS 256		/* maximale Spaltenanzahl	*/
#define	MIN_COLUMNS 40			/* minimale Spaltenanzahl	*/
#define	MAX_ROWS 256			/* maximale Zeilenanzahl	*/
#define	MIN_ROWS 10				/* minimale Zeilenanzahl	*/
#define	MAX_BUFFER 256			/* maximale Bufferzeilenanzahl	*/
#define	MIN_BUFFER 0			/* minimale Bufferzeilenanzahl	*/
#define	MAX_TIMER 999			/* maximaler Redraw-Abstand	*/
#define	MIN_TIMER 50			/* minimaler Redraw-Abstand	*/
#define	MAX_POINT 100			/* maximale Zeichenh�he	*/
#define	MIN_POINT 1				/* minimale Zeichenh�he	*/

typedef struct
{
	WORD	head;						/* erstes auszugebendes Zeichen in der Zeile oder -1, wenn nichts ausgegeben werden mu�	*/
	WORD	tail;						/* letztes auszugebendes Zeichen in der Zeile oder -1, wenn nichts ausgegeben werden mu�	*/
	ULONG *line;					/* Zeiger auf die langwortweisen Textdaten	*/
}TPTR;


/*
* Zu einem Terminalfenster gibt es i.a. mehrere Clients, z.B. Threads oder
* Signalhandler zu einem Proze�.
* F�r jede App-ID wird eine VTCLIENT_INFO-Struktur angelegt.
*/

typedef struct
{
	WINDOW	*w;					/* zugeh�riges Ausgabefenster */
	WORD		par_apid;			/* Parent-apid (wenn Proze�, sonst -1) */
} VTCLIENT_INFO;


/*
*
* Zu jedem Fenster WINDOW gibt es genau eine TSCREEN-Struktur.
* Man kommt �ber den Zeiger <interior> in WINDOW dran.
*
* Innerhalb dieser Struktur befindet sich eine VDIESC-Struktur,
* die kompatibel zu der zugeh�rigen Struktur in den LineA-Variablen
* ist:
*
* Struktur "VDIESC" (relativ zu LINEA):
*
* v_cel_mx	EQU -$2c
* v_cel_my	EQU -$2a
* v_cur_cx	EQU -$1c
* v_cur_cy	EQU -$1a
*
* Ans DOS wird ein Zeiger auf mx �bergeben, die anderen Variablen
* m�ssen im richtigen Abstand liegen.
*
*/

typedef struct
{
	WORD		refcnt;				/* Referenzz�hler (Tasks pro Terminalfenster) */
	WORD		handle;				/* VDI-Handle f�r die Workstation	*/
	WORD		child_id;			/* ID des TOS-Programmes	*/
	WORD		parent_id;			/* ID der Parent-Applikation	*/

	BYTE		name[256];			/* Pfad des Programms	*/

	WORD		input;				/* Semaphore f�r Eingaben	*/
	WORD		output;				/* Semaphore f�r Ausgaben	*/						
	
	WORD		term;					/* Flag f�r Beendigung	*/
	
	WORD		font_id;				/* Nummer des Zeichensatzes	*/
	WORD		point_size;			/* H�he in Punkten	*/
	WORD		char_width;			/* Zeichenbreite in Pixeln	*/
	WORD		char_height;		/* Zeichenh�he in Pixeln	*/

/* hier beginnt VDIESC */

	WORD		columns;				/* Anzahl der Spalten -1	*/
	WORD		visible_rows;		/* Anzahl der vom Benutzer vorgebenen Zeilen	*/

	WORD		rows;					/* Anzahl der Zeilen	-1		*/

	WORD		cur_cnt;				/* Cursor-Hide-Counter	*/
	WORD		cur_x;				/* letzte Spalte des Cursors	*/
	WORD		cur_y;				/* letzte Zeile des Cursors	*/
	WORD		save_x;				/* gespeicherte Cursorspalte oder -1	*/
	WORD		save_y;				/* gespeicherte Cursorzeile oder -1	*/

	WORD		x;						/* horizontale Cursorposition von 0 aus	*/
	WORD		term_y;				/* vertikale Cursorposition f�r BIOS */

/* hier endet VDIESC */

	WORD		y;						/* vertikale Cursorposition von 0 aus		*/

	WORD		wrap;					/* Zeilenumbruch aus (0) oder an (1)	*/
	WORD		revers;				/* Invertierung aus (0) oder an (1)	*/

	ULONG		colors;				/* Vordergrundfarbe | Hintergrundfarbe | 0 | 0 */ 

	
	void		(*vt_function)( struct _window *window, UWORD c );

	WORD		tcnt;					/* Anzahl der im Puffer vorhandenen Zeichen	*/
										/* von 0 aus gez�hlt, bei -1 ist kein Zeichen im Buffer	*/
	ULONG		tbuf[TBUFSIZE];	/* Tastaturpuffer, Zeichenformat wie bei Bconin()	*/

	TPTR		*first_line;		/*	Zeiger auf die erste Textzeile */

}TSCREEN;

#define	CUR_POS	0x8000
#endif	