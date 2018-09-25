#include <stdio.h>
#include <malloc.h>
#include "fsort.h"


static FILE     *tmpfiles[MAXFILES];/* gerade zu mischende Dateien */
static int      eofiles[MAXFILES];  /* Flag f�r EOF derselbigen */
static int      akt_anzahl;         /* Anzahl derselbigen */
static char     *buffers[MAXFILES]; /* F�r jede genau ein Puffer */
static unsigned bufferlen;          /* L�nge desselbigen */
static unsigned *strings[MAXFILES]; /* Anfangsadressen der Strings relativ
                                       zum Anfang eines Puffers */
static unsigned stringslen;         /* max. m�gliche Anz. Strings im Speicher */
static unsigned anzahl[MAXFILES];   /* Aktuelle Anz. von Strings im Speicher */
static unsigned next_str[MAXFILES]; /* Index des n�chsten auszugebenden Strings */
static int      fileanz = 0;        /* Anzahl der insgesamt zu mischenden Dateien */
static char     *name[100];         /* Zeiger auf deren Namen */

extern long     memavail;           /* Freier Speicher in Bytes */
extern int      (*vergl)();         /* Zeiger auf benutzte Vergleichsfkt. */
extern int      (*gibaus)();        /* Zeiger auf benutzte Ausgabefunktion */
extern char     obuf[];             /* Ausgabepuffer */


/***********************************************************
*
* Merkt sich jeden �bergebenen Dateinamen zum sp�teren
*  Mischen der sortierten Dateien und z�hlt die Anzahl der
*  insgesamt zu mischenden Dateien in <fileanz> mit.
* Der Zeiger name[fileanz] wird auf den �bergebenen
*  Dateinamen gesetzt.
* Die Strings werden aufsteigend und direkt hintereinander
*  abgespeichert, dh.
*  a)       i > j  => name[i] > name[j]
*  b)       name[i+1] = name[i] + strlen(name[i]) + 1
*
***********************************************************/

merke(dateiname)
char dateiname[];
{
     static char filenames[1000];    /* Namen der zu mischenden Dateien */
            int  len;


     /* Bestimmung der Anfangsposition des neuen Strings */
     /* ------------------------------------------------ */

     if   (fileanz == 0)
          name[fileanz] = filenames;
     else name[fileanz] = name[fileanz - 1] + strlen(name[fileanz - 1]) + 1;


     /* Pr�fen, ob entweder <filenames[]> oder <name[]> �berl�uft */
     /* --------------------------------------------------------- */

     len = strlen(dateiname) + 1;    /* einschliesslich EOS */
     if   (fileanz >= 100 || (len + name[fileanz] > filenames + 1000)) {
          fprintf(stderr, "Zuviele zu mischende Dateien\n");
          exit(2);
          }

     /* String ins Feld <filenames[]> kopieren */
     /* -------------------------------------- */

     strcpy(name[fileanz], dateiname);
     fileanz++;
}


/***********************************************************
*
* L�scht einen Dateinamen aus der Liste, da er bereits
*  beim Mischen benutzt wurde. �bergeben wird die Nummer
*  der Datei (0..fileanz), name[nr] ist der Dateiname.
* Alle anderen Dateinamen r�cken auf, so dass immer die
*  Dateinamen name[0]..name[fileanz-1] noch zu mischen sind.
*
***********************************************************/

vergiss(nr)
int nr;
{
     register int i,len;

     len = strlen(name[nr]) + 1;           /* L�nge einschliesslich EOS */
     for  (i = nr + 1; i < fileanz; i++) { /* Andere Strings nachr�cken */
          strcpy(name[i] - len, name[i]);
          name[i - 1] = name[i] - len;
          }
     fileanz--;
}


/***********************************************************
*
* Initialisiert die <akt_anzahl>- vielen Puffer f�r die zu
* sortierenden Strings sowie f�r die Zeiger auf dieselben.
* Das Verh�ltnis Puffer/Zeiger ist 10:1
* R�ckgabewert TRUE: zuwenig Speicher
*
***********************************************************/

int init_buf2()
{
     register int i;
     long     all,hilf;
     extern   int  init_buf1();
     extern        free_buf1();


     /* Normalerweise stellt die erste Phase (Sortieren) den freien  */
     /* Hauptspeicher fest. Ist der Switch <merge_only> gesetzt, ist */
     /* dies noch nicht geschehen und muss nachgeholt werden         */
     /* ------------------------------------------------------------ */

     if   (memavail == 0L) {             /* noch kein Speicher reserviert */
          if   (init_buf1())             /* Speichergroesse feststellen   */
               return(TRUE);
          free_buf1();
          }

     all         = (memavail * 9L) / 10L;    /* Speicher - 10% */
     hilf        = all/11L;                  /* Speicher f�r Zeiger */
     hilf       /= (long) akt_anzahl;        /* F�r jede Datei ein Feld */
     stringslen  = (hilf < SEGMENT) ? ((unsigned) hilf) : (SEGMENT);
     hilf        = all - (long) stringslen * (long) akt_anzahl;
     hilf       /= (long) akt_anzahl;
     bufferlen   = (hilf < SEGMENT) ? ((unsigned) hilf) : (SEGMENT);
     stringslen /= sizeof(unsigned); /* Anzahl der Elemente statt Bytes */

     for  (i = 0; i < akt_anzahl; i++) {
          strings[i] = (unsigned *) malloc((unsigned) sizeof(unsigned) * stringslen);
          if   (strings[i] == NULL)
               return(TRUE);
          buffers[i] = malloc(bufferlen);
          if   (buffers[i] == NULL)
               return(TRUE);
          }

     bufferlen  -= MAXLEN + 1;      /* Sicherheitsbereich zwischen Puffern */
     return(FALSE);
}


/***********************************************************
*
* Gibt den durch init_buf2 bereitgestellten Speicherplatz
*  wieder an das Betriebssystem zur�ck.
* Dies ist notwendig, wenn z.B. insgesamt 24 Dateien gemischt
*  werden sollen, also (Datei 0..13 => tmp1),
*  (Datei 14..24,tmp1 =>ausgeben).
*  In diesem Fall ben�tigt man erst 14, dann 10, dann 2 Puffer.
*
***********************************************************/

free_buf2()
{
     register int i;


     for  (i = 0; i < akt_anzahl; i++) {
          free(strings[i]);
          free(buffers[i]);
          }
}


/***********************************************************
*
* Strings von <infile> einlesen, bis Puffer Nr.n voll ist.
* <anzahl>   = Anzahl gelesener Strings und
*  <strings> = Anf.adressen der Strings werden initialisiert.
* R�ckgabe TRUE, wenn Eingabedatei zuende.
*
***********************************************************/

int  lies(infile,n)
FILE *infile;
int  n;
{
     int  dateiende;
     unsigned totlen;    /* Gesamtlaenge aller bisher eingelesenen... */
                            /* ...Strings einschl. EOS */

     dateiende = FALSE;
     anzahl[n] = 0;
     next_str[n] = 0;       /* buffers[n] + strings[n][0] ist der n�chste, */
     totlen = 0;            /*  auszugebende String */

     while(!dateiende && (anzahl[n] < stringslen) && (totlen < bufferlen)) {
          if   (NULL == fgets(buffers[n]+totlen,MAXLEN,infile))
               dateiende = TRUE;
          else {
               strings[n][anzahl[n]] = totlen;             /* Anfangsadresse */
               totlen += strlen(buffers[n] + totlen) + 1;  /* einschl. EOS  */
               anzahl[n] += 1;
               }
          }
     return(dateiende);
}


/***********************************************************
*
* Schreibt die alphabetisch "kleinste" Zeichenkette
* aller erster Zeichenketten aller Eingabedateien
* nach <out_file>.
* R�ckgabe EOF: Fehler
*
***********************************************************/

int schreib_min(out_file)
FILE *out_file;
{
     register int  i, dateinr;
     register char *minstring, *string;

     /* Minimum suchen und merken */
     minstring = NULL;
     for  (i = 0; i < akt_anzahl; i++)
          if   (anzahl[i] > 0) {
               string = buffers[i] + strings[i][next_str[i]];
               if   ( (minstring == NULL) ||  ((*vergl)(minstring,string) > 0) ) {
                    minstring = string;
                    dateinr   = i;
                    }
               }
/*       if   (minstring == NULL) {
          fprintf(stderr, "-Interner Fehler-");
          return(EOF);
          }
*/
     /* Eintrag l�schen */
     (next_str[dateinr])++;     /* Zeiger f�r n�chsten String erh�hen  */
     (anzahl  [dateinr])--;     /* Anzahl der verbleibenden verringern */
                                /* (nicht "erniedrigen", denn das w�re eine */
     /* Ausgabe */              /*  Beleidigung, keine Operation) */
     return((*gibaus)(minstring,out_file));
}


/***********************************************************
*
* F�llt jeden Puffer auf, der leer ist. Bei Bedarf wird
* das EOF- Flag der jew. Datei gesetzt.
* R�ckgabe FALSE <=> alle Dateien fertig.
*
***********************************************************/

int fill_buffers()
{
     register int i, nicht_alle_leer;

     nicht_alle_leer = FALSE;
     for  (i = 0; i < akt_anzahl; i++) {
          if   (anzahl[i] == 0)
               if   (!eofiles[i])
                    eofiles[i] = lies(tmpfiles[i],i);
          if   (anzahl[i] > 0)
               nicht_alle_leer = TRUE;
          }
     return(nicht_alle_leer);
}


/***********************************************************
*
* Haupt- Unterprogramm zum Mergen von Dateien.
* Die tempor�ren Dateien tmpfiles[0]..tmpfiles[akt_anz-1]
* werden gemischt und nach <out_file> ausgegeben.
* R�ckgabewert = TRUE, wenn Fehler.
*
***********************************************************/

int mische(out_file)
FILE *out_file;
{
     register int  i;

               
     /* Speicher reservieren */
     /* -------------------- */

     if   (init_buf2()) {
          fprintf(stderr,"Zuwenig Speicher f�r Merge\n");
          return(TRUE);
          }

     /* F�r den Anfang alle Puffer f�llen */
     /* --------------------------------- */

     for  (i = 0; i < akt_anzahl; i++)
          eofiles[i] = lies(tmpfiles[i],i);

     /* Solange mischen, bis alle Dateien EOF und alle Puffer leer */
     /* ---------------------------------------------------------- */

     do   {
          if   (EOF == schreib_min(out_file)) {
               fprintf(stderr, "Schreibfehler\n");
               return(TRUE);
               }
          }
     while(fill_buffers());

     /* Speicher wieder ans Betriebssystem zur�ckgeben */
     /* ---------------------------------------------- */

     free_buf2();
     return(FALSE);
}


/***********************************************************
*
* Phase 2 des Sortiervorgangs:
* Die tempor�ren Dateien name[0]..name[fileanz-1]
*  werden zusammengemischt, wobei jeweils maximal MAXFILES
*  viele auf einmal gemischt werden.
*  Dateinamen, die mit '-' beginnen, werden als stdin
*  interpretiert und nat�rlich weder geschlossen noch
*  gel�scht.
* Falls das <nokill_flag> = TRUE ist, werden die Zwischen-
*  dateien nach dem Verwenden nicht gel�scht (wird f�r das
*  merge_only ben�tigt, denn die Ausgangsdateien sollen
*  ja erhalten bleiben).
* Falls fileanz > MAXFILES, m�ssen weitere Zwischendateien
*  erstellt werden, andernfalls wird sofort nach <out_file>
*  geschrieben.
* R�ckgabewert = TRUE, wenn Fehler.
*
***********************************************************/

int merge_to(ausg_name, nokill_flag, verbose)
char *ausg_name;
int  nokill_flag;
int  verbose;
{
              FILE  *tmp_out;
     register int   i;
     extern   char  *tmpnam();
              char  tmpname[15];       /* Namen f�r neue Zwischendateien */


     while(fileanz > 0) {      /* Solange noch Dateien zu mischen sind...  */

          if   (verbose) {
               fprintf(stderr, "Noch %3d Dateien zu mischen\15", fileanz);
               fflush(stderr);
               }

          for  (akt_anzahl = 0; (akt_anzahl < fileanz) && (akt_anzahl < MAXFILES); akt_anzahl++) {
               if   (name[akt_anzahl][0] == '-')
                    tmpfiles[akt_anzahl] = stdin;
               else tmpfiles[akt_anzahl] = fopen(name[akt_anzahl], "ra");
               if   (tmpfiles[akt_anzahl] == NULL) {
                    fprintf(stderr,"Kann tempor�re Datei %s nicht �ffnen\n",
                            name[akt_anzahl]);
                    return(TRUE);
                    }
               }

          /* Wenn die <akt_anzahl> der zu mischenden Dateien kleiner als */
          /* die Gesamtanzahl <fileanz> ist, muss eine neue Zwischen-    */
          /* datei erstellt und f�rs sp�tere Mischen ge-merke()-t werden */
          /* ----------------------------------------------------------- */

          if   (akt_anzahl < fileanz) {
               if   (NULL == tmpnam(tmpname)) {  /* Namen f�r tempor�re Datei holen */
                    fprintf(stderr, "Fehler bei 'tmpnam()'\n");
                    return(TRUE);
                    }
               tmp_out = fopen(tmpname,"wa");
               merke(tmpname);
               }

          /* Ist jedoch die <akt_anzahl> = <fileanz>, k�nnen alle noch */
          /* vorhandenen Dateien auf einmal in die endg�ltige Ausgabe- */
          /* datei <out_file> gemischt werden.                         */
          /* --------------------------------------------------------- */

          else tmp_out = (ausg_name == NULL) ? stdout : fopen(ausg_name,"wa");

          if   (tmp_out == NULL) {
               fprintf(stderr, "Kann Ausgabedatei nicht erstellen\n");
               return(TRUE);
               }

         /* tmp_out soll einen grossen Puffer bekommen */
         /* ------------------------------------------ */

          setvbuf (tmp_out, obuf, _IOFBF, BUF_LEN);

          if   (mische(tmp_out))
               return(TRUE);

          fflush (tmp_out);             /* In jedem Fall Buffer raus */
          if   (tmp_out != stdout)
               fclose(tmp_out);

          /* Die gerade durch mische() ausgegebenen Dateien sind nun */
          /* abgehakt und k�nnen "vergessen" werden.                 */
          /* Da alle Indizes jedesmal um einen nach links geschoben  */
          /* werden, muss immer name[0] vergessen werden.            */
          /* ------------------------------------------------------- */

          for  (i = 0; i < akt_anzahl; i++) {
               fflush (tmpfiles [i]);
               if   (tmpfiles[i] != stdin) {
                    fclose(tmpfiles[i]);
                    if   (!nokill_flag)
                          unlink(name[0]);
                    }
               vergiss(0);
               }

          } /* ENDWHILE */
       
     return(FALSE);      
}
