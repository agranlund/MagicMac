
typedef int FD;			/* wirkliche Form egal */
typedef int DD;


/* DriveMediaDescriptor,  wird f�r jedes angemeldete Laufwerk angelegt     */
/***************************************************************************/

typedef struct {
     int  d_roff[3];          /* 0x00: Sekt.nr.offsets f�r FAT/DIR/DATA    */
     int  d_drive;            /* 0x06: Laufwerksnummer 0..15               */
     int  d_fatrec;           /* 0x08: erste Sektornr. der 2. FAT          */
     int  d_fsiz;             /* 0x0a: FAT- Gr��e in Sektoren              */
     int  d_clsiz;            /* 0x0c: Cluster- Gr��e in Sektoren          */
     int  d_clsizb;           /* 0x0e: Cluster- Gr��e in Bytes             */
     int  d_recsiz;           /* 0x10: Sektor- Gr��e in Bytes              */
     int  d_numcl;            /* 0x12: Anzahl der Datencluster             */
     int  d_lclsiz;           /* 0x14: 2er- Logarithmus von clsiz          */
     int  d_mclsiz;           /* 0x16: Bit- Maske f�r clsiz                */
     int  d_lrecsiz;          /* 0x18: 2er- Logarithmus von recsiz         */
     int  d_mrecsiz;          /* 0x1a: Bit- Maske f�r recsiz               */
     int  d_lclsizb;          /* 0x1c: 2er- Logarithmus von clsizb         */
     FD   *d_fatfd;           /* 0x1e: Zeiger auf FD der FAT               */
     long d_dummy;
     DD   *d_rdd;             /* 0x26: Zeiger auf DD der Root              */
     int  d_flag;             /* 0x2a: FAT- Typ: 0: 12-Bit, 1: 16-Bit      */
} DMD;


/* Directory- Eintrag im Verzeichnis auf einem Datentr�ger                 */
/***************************************************************************/

typedef struct {
     char dir_name[11];       /* 0x00: Dateiname                           */
     char dir_attr;           /* 0x0b: Attribut                            */
     char dir_dummy[10];
     int  dir_time;           /* 0x16: Zeit  der letzten �nderung          */
     int  dir_date;           /* 0x18: Datum der letzten �nderung          */
     int  dir_stcl;           /* 0x1a: erster Cluster                      */
     long dir_flen;           /* 0x1c: Dateil�nge                          */
} DIRENTRY;


/* BufferControlBlock, verkettete Liste der DOS- Sektorpuffer              */
/***************************************************************************/

typedef struct ____bcb {
  struct ____bcb *b_link;     /* 0x00: Zeiger auf n�chsten BCB             */
     int  b_bufdrv;           /* 0x04: Laufwersnummer, -1 f�r ung�ltig     */
     int  b_buftyp;           /* 0x06: FAT=0, DIR=1, DATA=2                */
     int  b_bufrec;           /* 0x08: Sektornummer (GEMDOS- Code)         */
     int  b_dirty;            /* 0x0a: Pufferinhalt ge�ndert               */
     DMD  *b_dmd;             /* 0x0c: Zeiger auf DMD von b_bufdrv         */
     char *b_bufr;            /* 0x10: Zeiger auf Sektorpuffer             */
} BCB;


