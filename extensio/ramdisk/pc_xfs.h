/*
 * pc_xfs.h vom 23.06.1996
 *
 * Autor:
 * Thomas Binder
 * (binder@rbg.informatik.th-darmstadt.de)
 *
 * Zweck:
 * Headerdatei f�r die Pure-C-MagiC-3-XFS-Schnittstelle. Die genau
 * Beschreibung der Funktionen entnehme man der MagiC-3-Doku, die f�r
 * registrierte Benutzer bei ASH erh�ltlich ist. Hier finden sich nur
 * Erkl�rungen f�r die Unterschiede zwischen der Beschreibung in der
 * Doku und den benutzten Funktionspointern, die sich aus technischen
 * Gr�nden leider nicht vermeiden lie�en.
 *
 * History:
 * 04.11.-
 * 05.11.1995: Erstellung
 * 06.11.1995: MX_KERNEL in THE_MX_KERNEL umbenannt, da sich gezeigt
 *             hat, da� die darin erreichbaren Funktionen Register A2
 *             ver�ndern, was Pure C gewaltig in's Schleudern bringt.
 *             Daher mu�te auch diese Struktur auf eine eigene
 *             interne abgebildet werden, in der die Funktionen die
 *             Register vorher retten und dann die eigentlich
 *             gew�nschte Routine anspringen (siehe pc_xfs.s).
 * 07.11.1995: XATTR-Struktur kommentiert und mit Konstanten versehen
 * 10.11.1995: mode bei xfs_dcreate entfernt, da es in der neuen
 *             MagiC-3-Doku nicht mehr auftaucht.
 * 11.11.1995: _sprintf in der Kernel-Struktur ist nicht mehr cdecl,
 *             da ja sowieso eine vorgeschaltete Routine angesprungen
 *             wird, die dann die Parameter auf den Stack legt.
 * 12.11.1995: xfs_fopen, xfs_xattr und xfs_attrib m�ssen bei Bedarf
 *             auch einen Zeiger auf einen symbolischen Link liefern,
 *             daher wurde bei den Prototypen ein solcher eingebaut.
 * 23.11.1995: dta_drive war versehentlich WORD statt char.
 * 11.12.1995: Erweiterung der THE_MX_KERNEL-Struktur um die neuen
 *             Elemente von Kernelversion 1 und 2.
 * 26.12.1995: KER_INSTXFS und KER_GETINFO aufgenommen.
 * 28.12.1995: Auch xfs_chmod, xfs_chown und xfs_dcntl m�ssen u.U.
 *             einen Zeiger auf einen symbolischen Link liefern,
 *             daher wurden die Prototypen entsprechend erweitert.
 * 31.12.1995: path2DD hat jetzt kein unhandliches returns-Array
 *             mehr, sondern die neuen Einzelparameter lastpath,
 *             linkdir und symlink.
 * 02.01.1996: Bei dev_getline waren mode und size vertauscht.
 * 13.02.1996: Der zweite Parameter f�r install_xfs war unn�tig und
 *             ist deswegen jetzt 'rausgeflogen, ebenso wie der
 *             Zeiger real_dev.
 * 16.06.1996: d_devcode in der Struktur DMD war vom falschen Typ.
 * 23.06.1996: Neue Elemente mxalloc, mfree und mshrink in die
 *             Kernelstruktur aufgenommen.
 */

#ifndef __PC_XFS__
#define __PC_XFS__

#include <tos.h>
#include <portab.h>
#include <sys/stat.h>
#include "toserror.h"
#define PD BASEPAGE
typedef void APPL;
#include "mgx_xfs.h"

/*
 * Zeiger auf die tats�chlich von MagiC angesprochenen Strukturen.
 * Sie werden u.a. bei xfs_drv_open und bei xfs_fopen ben�tigt, denn
 * es darf nat�rlich nie der Zeiger auf die C-Struktur eingetragen
 * werden, wenn es sich um MagiC-Strukturen wie DMD, DD, FD, oder DHD
 * handelt.
 */
extern void	*real_xfs;


/*
 * Die XFS-Schnittstelle, wie sie das C-Programm sieht. In dieser
 * Struktur fehlen die Anteile der XFS-Schnittstelle, die f�r
 * (externe) Treiber ohne Belang sind.
 */
typedef struct
{
	char	xfs_name[8];
	LONG	(*xfs_sync)(MX_DMD *d);
	void	(*xfs_pterm)(PD *pd);
/*
 * F�r xfs_garbcoll mu� ein Funktionspointer angegeben werden, auch
 * wenn das Filesystem die interne Speicherverwaltung von MagiC 3
 * nicht benutzt. In diesem Fall einfach eine Funktion einbinden, die
 * 0L zur�ckliefert.
 */
	LONG	(*xfs_garbcoll)(MX_DMD *d);
	void	(*xfs_freeDD)(void *dd);
	LONG	(*xfs_drv_open)(MX_DMD *d);
	LONG	(*xfs_drv_close)(MX_DMD *d, WORD mode);
/*
 * Da xfs_path2DD normalerweise bis zu vier R�ckgabewerte hat, werden
 * drei davon in Zeigerparametern zur�ckgegeben. Die Zuordnung der
 * Register (siehe MagiC-3-Doku):
 * d0: R�ckgabewert der C-Funktion
 * d1: *lastpath
 * a0: *linkdir
 * a1: *symlink
 */
	LONG	(*xfs_path2DD)(void *reldir, char *pathname, WORD mode,
		char **lastpath, LONG *linkdir, char **symlink);
/*
 * Auch xfs_sfirst liefert zwei R�ckgabeparameter. Da a0 aber nur
 * einen Zeiger auf einen Symbolischen Link enthalten kann, ist der
 * Parameter entsprechend als char ** deklariert, d.h. die C-Funktion
 * mu� ggf. hier den Zeiger auf den symbolischen Link ablegen (dabei
 * nicht vergessen, da� die ersten beiden "Buchstaben" die L�nge des
 * Links (als Wort) angeben).
 */
	LONG	(*xfs_sfirst)(void *srchdir, char *name, DTA *dta,
		WORD attrib, char **symlink);
/* Entsprechendes gilt nat�rlich auch f�r xfs_snext */
	LONG	(*xfs_snext)(DTA *dta, MX_DMD *dmd, char **symlink);
#define OM_RPERM	1
#define OM_WPERM	2
#define OM_EXEC		4
#define OM_APPEND	8
#define OM_RDENY	16
#define OM_WDENY	32
#define OM_NOCHECK	64
/*
 * Dont get fooled by Pure-C header files;
 * we need the MiNT values here
 */
#undef O_CREAT
#undef O_TRUNC
#undef O_EXCL
#define O_CREAT		0x200
#define O_TRUNC		0x400
#define O_EXCL		0x800
/*
 * Auch xfs_fopen liefert unter Umst�nden einen Zeiger auf einen
 * symbolischen Link...
 */
	LONG	(*xfs_fopen)(void *dir, char *name, WORD omode,
		WORD attrib, char **symlink);
	LONG	(*xfs_fdelete)(void *dir, char *name);
	LONG	(*xfs_link)(void *olddir, void *newdir, char *oldname,
		char *newname, WORD flag_link);
/* Ebenfalls zus�tzlich mit Platzhalter f�r symbolischen Link */
	LONG	(*xfs_xattr)(void *dir, char *name, XATTR *xattr,
		WORD mode, char **symlink);
/* Und noch dreimal... */
	LONG	(*xfs_attrib)(void *dir, char *name, WORD rwflag,
		WORD attrib, char **symlink);
	LONG	(*xfs_chown)(void *dir, char *name, UWORD uid,
		UWORD gid, char **symlink);
	LONG	(*xfs_chmod)(void *dir, char *name, UWORD mode,
		char **symlink);
	LONG	(*xfs_dcreate)(void *dir, char *name);
	LONG	(*xfs_ddelete)(void *dir);
	LONG	(*xfs_DD2name)(void *dir, char *name, WORD bufsize);
	LONG	(*xfs_dopendir)(void *dir, WORD tosflag);
	LONG	(*xfs_dreaddir)(void *dhd, WORD size, char *buf,
		XATTR *xattr, LONG *xr);
	LONG	(*xfs_drewinddir)(void *dhd);
	LONG	(*xfs_dclosedir)(void *dhd);
	LONG	(*xfs_dpathconf)(void *dir, WORD which);
	LONG	(*xfs_dfree)(void *dd, DISKINFO *free);
	LONG	(*xfs_wlabel)(void *dir, char *name);
	LONG	(*xfs_rlabel)(void *dir, char *name, char *buf,
		WORD len);
	LONG	(*xfs_symlink)(void *dir, char *name, char *to);
	LONG	(*xfs_readlink)(void *dir, char *name, char *buf,
		WORD size);
/* Nochmal mit Platzhalter f�r symbolischen Link */
	LONG	(*xfs_dcntl)(void *dir, char *name, WORD cmd, LONG arg,
		char **symlink);
} THE_MGX_XFS;

/*
 * Devicetreiber, wieder aus C-Sicht, trotzdem identisch mit der
 * "echten" Struktur, d.h. hier sind keine vorgeschalteten
 * Assemblerroutinen n�tig
 */
typedef struct
{
	LONG	(*dev_close)(void *file);
	LONG	(*dev_read)(void *file, LONG count, char *buffer);
	LONG	(*dev_write)(void *file, LONG count, char *buffer);
	LONG	(*dev_stat)(void *file, MAGX_UNSEL *unselect,
		WORD rwflag, LONG apcode);
	LONG	(*dev_seek)(void *file, LONG where, WORD mode);
	LONG	(*dev_datime)(void *file, WORD *d, WORD setflag);
	LONG	(*dev_ioctl)(void *file, WORD cmd, void *buf);
#define CMODE_RAW		0
#define CMODE_COOKED	1
#define CMODE_ECHO		2
	LONG	(*dev_getc)(void *file, WORD mode);
	LONG	(*dev_getline)(void *file, char *buf, WORD mode,
		LONG size);
	LONG	(*dev_putc)(void *file, WORD mode, LONG value);
} THE_MGX_DEV;

/*
 * Die Kernel-Struktur, deren Funktionen leider auch nicht direkt von
 * Pure C aus aufgerufen werden k�nnen, da sie unter Umst�nden das
 * Register A2 ver�ndern, was Pure C �berhaupt nicht mag. Also wird
 * diese Struktur ebenfalls nachgebildet... Um die Variablen aus der
 * Struktur anzusprechen empfiehlt es sich, den Zeiger real_kernel
 * zu benutzen, da die Kopie nicht aktualisiert wird und es durchaus
 * denkbar ist, da� einer der Variablenzeiger nachtr�glich ge�ndert
 * wird. Die vorgeschalteten Funktionen benutzen selbstverst�ndlich
 * auch real_kernel, um die tats�chlich gew�nschten Routinen
 * aufzurufen.
 */
typedef struct
{
     WORD version;
     void (*fast_clrmem)      ( void *von, void *bis );
     char (*toupper)          ( char c );
     void (*_sprintf)         ( char *dest, const char *source, LONG *p );
     PD	**act_pd;
     APPL *act_appl;
     APPL *keyb_app;
     WORD *pe_slice;
     WORD *pe_timer;
     void (*appl_yield)       ( void );
     void (*appl_suspend)     ( void );
     void (*appl_begcritic)   ( void );
     void (*appl_endcritic)   ( void );
     long (*evnt_IO)          ( LONG ticks_50hz, MAGX_UNSEL *unsel );
     void (*evnt_mIO)         ( LONG ticks_50hz, MAGX_UNSEL *unsel, WORD cnt );
     void (*evnt_emIO)        ( APPL *ap );
     void (*appl_IOcomplete)  ( APPL *ap );
     long (*evnt_sem)         ( WORD mode, void *sem, LONG timeout );
     void (*Pfree)            ( PD *pd );
     WORD int_msize;
     LONG (*int_malloc)       ( void );
     void (*int_mfree)        ( void *memblk );
     void (*resv_intmem)      ( void *mem, LONG bytes );
     LONG (*diskchange)       ( WORD drv );
/* Ab Kernelversion 1: */
     LONG (*DMD_rdevinit)     ( MX_DMD *dmd );
/* Ab Kernelversion 2: */
     LONG (*proc_info)        ( WORD code, PD *pd );
/* Ab Kernelversion 4: */
     LONG (*mxalloc)          ( LONG amount, WORD mode, PD *pd );
     LONG (*mfree)            ( void *block );
     LONG (*mshrink)          ( void *block, LONG newlen );
} THE_MX_KERNEL;

/*
 * Zeiger auf die tats�chlich von Dcntl(KER_INSTXFS, ...) gelieferte
 * Kernelstruktur. Die Funktionen sollten nicht angesprochen werden,
 * f�r das Auslesen der Variablen ist es jedoch ratsam, immer �ber
 * diese Struktur zu gehen, da die Kopie nicht aktualisiert wird.
 */
extern THE_MX_KERNEL *real_kernel;
extern THE_MX_KERNEL *kernel;
extern THE_MX_KERNEL *install_kernel(THE_MX_KERNEL *);

/*
 * Routine zur Installation des XFS. Ihr �bergibt man den Zeiger auf
 * das zu installierende XFS. Zur�ck erh�lt man einen Zeiger auf die
 * Kernelstruktur von MagiC 3 oder NULL, wenn ein Fehler aufgetreten
 * ist.
 */
LONG install_xfs(THE_MGX_XFS *xfs);

#endif

/* EOF */
