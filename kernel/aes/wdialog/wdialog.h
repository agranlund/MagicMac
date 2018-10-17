#if	CALL_MAGIC_KERNEL

#define	wdlg_get_tree \
			wdlg_gtree

#define	wdlg_get_edit \
			wdlg_gedit

#define	wdlg_get_udata \
			wdlg_gudata

#define	wdlg_get_handle \
			wdlg_ghandle

#define	wdlg_set_edit \
			wdlg_sedit

#define	wdlg_set_tree \
			wdlg_stree

#define	wdlg_set_size \
			wdlg_ssize

#define	wdlg_set_iconify \
			wdlg_sic

#define	wdlg_set_uniconify \
			wdlg_sui

#endif

typedef struct
{
	WORD	mwhich;
	WORD	mx;
	WORD	my;
	WORD	mbutton;
	WORD	kstate;
	WORD	key;
	WORD	mclicks;
	WORD	reserved[9];
	WORD	msg[16];
} EVNT;

typedef	WORD	(cdecl *HNDL_OBJ)( struct _dialog *dialog, EVNT *events, WORD obj, WORD clicks, void *data );

/*----------------------------------------------------------------------------------------*/ 
/* Bei �nderungen an der DIALOG-Struktur mu� auch die Assembler-Definition angepa�t 		*/
/*	werden!																											*/
/*----------------------------------------------------------------------------------------*/ 

typedef struct _dialog
{
	LONG			magic1;													/* Magic: 'wdlg' */
	LONG			version;													/* Versionsnummer: 0x10000L */
	
	WORD			flags;
	
	void			*user_data;												/* benutzerinterne Daten */

	OBJECT 		*tree;													/* Objektbaum */
	GRECT			rect;														/* Dialogposition und -ausma�e (nicht mit border geschnitten) */

	WORD			whdl;														/* Fensterhandle */
	WORD			kind;														/* Fensterattribute */
	GRECT			border;													/* Fensterposition und -ausma�e (auf den Rand bezogen) */

	HNDL_OBJ		handle_exit;											/* Zeiger auf die Service-Funktion */

	WORD			act_editob;												/* aktuelles Edit- Objekt */
	WORD			cursorpos;

	WORD			root_ob_state;											/* beim Aufruf von wdlg_open gesicherter ob_state des ROOT-Objekts */
	OBSPEC		root_ob_spec;											/* beim Aufruf von wdlg_open gesicherter ob_spec des ROOT-Objekts */
} DIALOG;

/* Definitionen f�r <flags> */
#define	WDLG_BKGD	1													/* Hintergrundbedienung zulassen */

/* Funktionsnummern f�r <obj> bei handle_exit(...) */
#define	HNDL_INIT	-1													/* Dialog initialisieren */
#define	HNDL_MESG	-2													/* Message bearbeiten */
#define	HNDL_CLSD	-3													/* Dialogfenster wurde geschlossen */
#define	HNDL_OPEN	-5													/* Dialog-Initialisierung abschlie�en (zweiter Aufruf am Ende von wdlg_init) */
#define	HNDL_EDIT	-6													/* Zeichen f�r ein Edit-Feld �berpr�fen */
#define	HNDL_EDDN	-7													/* Zeichen wurde ins Edit-Feld eingetragen */
#define	HNDL_EDCH	-8													/* Edit-Feld wurde gewechselt */
#define	HNDL_MOVE	-9													/* Dialog wurde verschoben */
#define	HNDL_TOPW	-10												/* Dialog-Fenster ist nach oben gekommen */
#define	HNDL_UNTP	-11												/* Dialog-Fenster ist nicht aktiv */

/* Parameterbeschreibung f�r die Service-Routine handle_exit():

	WORD	handle_exit( struct _dialog *d, WORD obj, WORD clicks, void *data );
	
	d:			Zeiger auf eine Dialog-Struktur. Auf die Struktur sollte
				nicht direkt zugegriffen werden. Die wdlg_xx-Funktionen
				sollten benutzt werden!

	obj:		>= 0: Objektnummer
				< 0:	Funktionsnummer (siehe unten)
			
	clicks:	Anzahl der Mausklicks (falls es sich bei <obj>
				um eine Objektnummer handelt)
	
	data:		der Inhalt h�ngt von <obj> ab
					
	Bedeutung von <data> abh�ngig von <obj>:

	Falls <obj> eine (positive) Objektnummer ist, wird in <data>
	die Variable <user_data> �bergeben (siehe wdlg_create).
	<clicks> enth�lt die Anzahl der Mausklicks auf dieses Objekt.
	
	HNDL_INIT: <data> ist die bei wdlg_init �bergebene Variable.
					Wenn handle_exit() 0 zur�ckliefert, legt
					wdlg_create() keine Dialog-Struktur an (Fehler).
					Die Variable <code> wird in <clicks> �bergeben.
					
	HNDL_OPEN: <data> ist die bei wdlg_open �bergebene Variable.
					Die Variable <code> wird in <clicks> �bergeben.
					
	HNDL_CLSD: <data> ist <user_data>. Wenn handle_exit() 0 
					zur�ckliefert, wird der Dialog geschlossen -
					wdlg_evnt() liefert 0 zur�ck.

	HNDL_MOVE: <data> ist <user_data>. Wenn handle_exit() 0 
					zur�ckliefert, wird der Dialog geschlossen -
					wdlg_evnt() liefert 0 zur�ck.

	HNDL_TOPW: <data> ist <user_data>. Wenn handle_exit() 0 
					zur�ckliefert, wird der Dialog geschlossen -
					wdlg_evnt() liefert 0 zur�ck.

	HNDL_UNTP: <data> ist <user_data>. Wenn handle_exit() 0 
					zur�ckliefert, wird der Dialog geschlossen -
					wdlg_evnt() liefert 0 zur�ck.

	HNDL_UMSG: <data> ist ein Zeiger auf den Message-Buffer.
					Wenn handle_exit() 0 zur�ckliefert, wird der
					Dialog geschlossen -	wdlg_evnt() liefert 0 zur�ck.

	HNDL_EDIT:	<data> zeigt auf ein Wort mit dem Tastencode.
					Wenn handle_exit() 1 zur�ckliefert, wird der
					Tastendruck verarbeitet, bei 0 ignoriert.
	
	HNDL_EDDN:	<data> zeigt auf ein Wort mit dem Tastencode.

	HNDL_EDCH:	<data> zeigt auf ein Wort mit der Objektnummer
					des neuen Edit-Felds.

*/

extern	DIALOG	*wdlg_create( HNDL_OBJ handle_exit, OBJECT *tree, void *user_data, WORD code, void *data, WORD flags );
extern	WORD	wdlg_open( DIALOG *d, BYTE *title, WORD kind, WORD x, WORD y, WORD code, void *data );
extern	WORD	wdlg_close( DIALOG *d, WORD *x, WORD *y );
extern	WORD	wdlg_delete( DIALOG *d );

extern	WORD	wdlg_get_tree( DIALOG *d, OBJECT **tree, GRECT *r );
extern	WORD	wdlg_get_edit( DIALOG *d, WORD *cursor );
extern	void	*wdlg_get_udata( DIALOG *d );
extern	WORD	wdlg_get_handle( DIALOG *d );

extern	WORD	wdlg_set_edit( DIALOG *d, WORD obj );
extern	WORD	wdlg_set_tree( DIALOG *d, OBJECT *tree );
extern	WORD	wdlg_set_size( DIALOG *d, GRECT *size );
extern	WORD	wdlg_set_iconify( DIALOG *d, GRECT *g, char *title, OBJECT *tree, WORD obj );
extern	WORD	wdlg_set_uniconify( DIALOG *d, GRECT *g, char *title, OBJECT *tree );

extern 	WORD	wdlg_evnt( DIALOG *d, EVNT *events );
extern	void	wdlg_redraw( DIALOG *d, GRECT *rect, WORD obj, WORD depth );
