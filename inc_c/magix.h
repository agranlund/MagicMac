/************************************************************************/
/*      MAGX.H      #defines for MAG!X Extensions                       */
/*              started 25/10/90 Andreas Kromke                         */
/*              last change 25/08/93 for Mag!X 2.00                     */
/*                                                                      */
/*     f�r TURBO C                                                      */
/************************************************************************/

#if  !defined( __AES__ )
#define OBJECT void
#endif

/* ProgramHeader, Programmkopf f�r ausf�hrbare Dateien                  */
/************************************************************************/

/* Kernel evnt_sem Modes (Mag!X 2.1) */

#define SEM_FREE    0
#define SEM_SET     1
#define SEM_TEST    2
#define SEM_CSET    3
#define SEM_GET     4
#define SEM_CREATE  5
#define SEM_DEL     6

typedef struct _appl {
     struct _appl *ap_next;   /* Verkettungszeiger                    */
     int       ap_id;         /* Application-Id                       */
     int       ap_parent;     /* ap_id der parent- Applikation        */
     OBJECT    *ap_menutree;  /* Men�leiste                           */
     OBJECT    *ap_desktree;  /* Desktop- Hintergrundbaum             */
     int       ap_1stob;      /*  dazu erstes Objekt                  */
     char      ap_dummy1[2];  /* zwei Leerzeichen vor ap_name         */
     char      ap_name[8];    /* Applikationsname                     */
     char      ap_dummy2[2];  /* Leerstelle und ggf. Ausblendzeichen  */
     char      ap_dummy3;     /* Nullbyte f�r EOS                     */
     char      ap_status;     /* 0=ready 1=waiting 2=outtime 3=zombie */
} APPL;

typedef struct {
     long xaesmagic;          /* Wert ist 'XAES'                      */
     APPL *act_appl;          /* aktive Applikation                   */
     int  ap_pd_offs;         /* Offset Basepage in APPL-Struktur     */
     int  appln;              /* Anzahl geladener Applikationen       */
     int  maxappln;           /* Anzahl ladbarer Applikationen        */
     APPL *applx[];           /* Applikationstabelle, L�nge maxappln  */
} DOSMAGIC;

/* Mag!X- Dateideskriptor (ab V2.01) */

typedef struct {
     void *fd_link;           /* 0x00: Zeiger auf FDs im selben Verzeichn. */
     int  fd_dirch;           /* 0x04: Bit0: "dirty"                       */
     int  fd_time;            /* 0x06: Zeit  (8086)                        */
     int  fd_date;            /* 0x08: Datum (8086)                        */
     int  res1;               /* 0x0a: Start- Cluster (68000)              */
     long fd_len;             /* 0x0c: Dateil�nge in Bytes                 */
     void *fd_dmd;            /* 0x10: Zeiger auf DMD                      */
     void *fd_dirdd;          /* 0x14: Zeiger auf DD des zug. Directories  */
     void *fd_dirfd;          /* 0x18: Zeiger auf FD des zug. Directories  */
     long fd_dirpos;          /* 0x1c: Pos. des zug. Eintrags im Directory */
     long fd_fpos;            /* 0x20: Position des Dateizeigers           */
     void *fd_xdata;
     int  usr1;               /* 0x28:  Offset zum Clusteranfang oder dev  */
     int  fd_xftype;          /* 0x2a: (wie dir_xftype)                    */
     void *fd_multi;          /* 0x2c: Zeiger auf FD derselben Datei       */
     int  fd_mode;            /* 0x30: Open- Modus (0,1,2)                 */
} MAGX_FD;

/* Mag!X Ger�tetreiber (ab 2.01) */

#define DEV_M_INSTALL 0xcd00

typedef struct _m_u {
     void (*unsel_unsel) (struct _m_u *unsel);
     long param;
} MAGX_UNSEL;

typedef struct {
     void *act_pd;
     APPL *act_appl;
     APPL *keyb_app;
     void (*appl_yield)       ( void );
     void (*appl_suspend)     ( void );
     long (*evnt_IO)          ( long ticks_50hz, void *unsel );
     void (*evnt_mIO)         ( long ticks_50hz, void *unsel, int cnt );
     void (*evnt_emIO)        ( APPL *ap );
     long (*evnt_sem)         ( int mode, void *sem, long timeout );
     void (*appl_IOcomplete)  ( APPL *ap );
     void (*Pfree)            ( void *pd );
} MAGX_KERNEL;

/* os_magic -> */

#ifndef _AESVARS
#define _AESVARS
typedef struct
     {
     /* Dies ist die Variable, auf die das Cookie und auch der   */
     /* osheader zeigen. Die ersten drei Variablen sind in jedem */
     /* TOS vorhanden.                                           */
     long magic;                   /* mu� $87654321 sein         */
     void *membot;                 /* Ende der AES- Variablen    */
     void *aes_start;              /* Startadresse               */
     /* KAOS */
     long magic2;                  /* ist 'MAGX' oder 'KAOS'     */
     long date;                    /* Erstelldatum, $ddmmyyyy    */
     /* Mit <res> und <txt> kann man die Angaben in magx.inf     */
     /* �berladen, <txt> ist die Fonth�he f�r den gro�en AES-    */
     /* Font, und zwar in Pixeln. <txt> = 0 �bernimmt den Wert   */
     /* aus magx.inf                                             */
     void (*chgres)(int res, int txt);  /* Aufl�sung �ndern      */
     /* Hier klinkt sich magxdesk.app ein.                       */
     long (**shel_vector)(void);   /* ROM- Desktop               */
     /* Dies ist das Bootlaufwerk, auf dem magx.inf liegt        */
     char *aes_bootdrv;            /* Hierhin kommt DESKTOP.INF  */
     int  *vdi_device;             /* vom AES benutzter Treiber  */
     void **nvdi_workstation;      /* vom AES benutzte Workst.   */
     /* Die folgenden beiden Variablen waren f�r KAOSDESK, um    */
     /* festzustellen, ob das gestartete Programm seinerseits    */
     /* einen shel_write() gemacht hatte.                        */
     int  *shelw_doex;             /* letztes <doex> f�r App #0  */
     int  *shelw_isgr;             /* letztes <isgr> f�r App #0  */
     /* MAG!X */
     int  version;                 /* etwa $0200 f�r V 2.0       */
     int  release;                 /* 0=� 1=� 2=� 3=release      */
     void *_basepage;              /* Basepage des AES           */
     /* Der Z�hler zeigt, ob AES die Maus aus- oder eingeschal-  */
     /* tet hat. Bei einem Wert von 0 ist die Maus sichtbar      */
     int  *moff_cnt;               /* globaler Mauszeigerz�hler  */
     /* den folgenden Wert erh�lt man auch mit shel_get(-1)      */
     long shel_buf_len;            /* L�nge des Shell- Puffers   */
     void *shel_buf;               /* Zeiger auf Shell- Puffer   */
     /* Liste der wartenden Applikationen. Die suspend_list,     */
     /* die erst ab Mag!X 2.0 existiert, ist leider noch nicht   */
     /* von au�en zug�nglich. Die suspend_list enth�lt die       */
     /* Programme, die ihre Rechenzeit verbraucht haben.         */
     APPL **notready_list;         /* wartende Applikationen     */
     APPL **menu_app;              /* men�besitzende Applikation */
     OBJECT **menutree;            /* aktiver Men�baum           */
     OBJECT **desktree;            /* aktiver Desktophintergrund */
     int  *desktree_1stob;         /*  dessen erstes Objekt      */
     DOSMAGIC *dos_magic;          /* �bergabestruktur f�rs DOS  */
     /* Zeiger auf folgende Struktur:                            */
     /*   int  maxwindn;      Anzahl m�glicher Fenster           */
     /*   int  topwhdl;       Handle des aktiven Fensters        */
     /*   int  whdlx[maxwindn];    Fensterliste, oben->unten     */
     /*        negative Handles geh�ren eingefrorenen Apps.      */
     /*   WINDOWS *windx;     Zeiger auf die Fenstertabelle      */
     void *windowinfo;             /* versch. Fensterinfos       */
     /* Installationsvektor f�r alternative Dateiauswahl,        */
     /* wird von Selectric verwendet. Kein TRAP n�tig!           */
     int  (**p_fsel)(char *pth, char *name, int *but, char *title);
     /* Umschaltung des pr�emptiven Multitaskings, Parameter:    */
     /*   d0.lo     Ticks f�r Scheibe                            */
     /*   d0.hi     Hintergrund- Priorit�t, z.B. 32 => 1:32      */
     /* d0 = -1L deaktiviert das pr�emptive Multitasking         */
     /* d0.lo <= -2 oder d0.hi <= -2 �ndert den Wert nicht       */
     /* Der alte Wert wird zur�ckgegeben.                        */
     long (*ctrl_timeslice) (long settings);
     APPL **topwind_app;           /* App. des obersten Fensters */
     APPL **mouse_app;             /* men�besitzende Applikation */
     APPL **keyb_app;              /* tastaturbesitzende Appl.   */
     long dummy;
     } AESVARS;
#endif
