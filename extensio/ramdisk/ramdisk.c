/*
 * ramdisk.c vom 21.07.1996
 *
 * Autor:
 * Thomas Binder
 * (binder@rbg.informatik.th-darmstadt.de)
 *
 * Zweck:
 * C-Quellcode des Ramdisk-Filesystems f�r MagiC 3, das auf der
 * Pure-C-Schnittstelle aufbaut. Hier finden sich die Routinen, die
 * vom Kernel (indirekt �ber die Schnittstelle) aufgerufen werden,
 * also die Elemente der THE_MGX_XFS- und THE_MGX_DEV-Strukturen
 * darstellen.
 * Anmerkung: Dieser Quellcode stellt keinen Ersatz f�r die MagiC-
 * Doku dar; nur zusammen mit ihr wird man alles nachvollziehen
 * k�nnen (hoffentlich). Deshalb haben nur die Funktionen in
 * ramutil.c Header, die die genaue Funktionsweise erkl�ren. Hier
 * h�tte ich prinzipiell die Doku abtippen m�ssen, was ich aus
 * verst�ndlichen Gr�nden nicht gemacht habe; es wird nur erkl�rt,
 * was allgemein beachtenswert und was f�r die Ramdisk n�tig ist.
 * Man bekommt die Doku �brigens als registrierter MagiC-Benutzer
 * direkt von ASH (wohl auch in der Mailbox).
 *
 * History:
 * 05.11.-
 * 25.11.1995: Erstellung (mit Unterbrechungen)
 * 27.11.1995: Fehler in dfree-Funktion behoben und Cconws bei
 *             check_name wieder in TRACE ge�ndert.
 * 03.12.1995: datime eingebaut, chmod und attrib jetzt auch f�r .
 *             und .., TRACE f�r restliche dev-Funktionen eingebaut,
 *             "not supported"-Meldung bei TRACE von (noch) nicht
 *             unterst�tzten Funktionen.
 * 04.12.1995: ramdisk_seek hat bei Bereichs�berschreitungen bisher
 *             "geclippt", jetzt wird korrekt ERANGE geliefert und
 *             der Positionszeiger nicht ver�ndert.
 *             Beim Erweitern eines Verzeichnisses wird jetzt korrekt
 *             um DEFAULTDIR Eintr�ge aufgestockt.
 * 04.12.-
 * 05.12.1995: Dateien werden jetzt anders verwaltet: Sie bestehen
 *             jeweils aus einzelnen Bl�cken a DEFAULTFILE Bytes, die
 *             miteinander verkettet sind (sie sind also eigentlich
 *             noch sizeof(char *) Bytes l�nger). Dadurch sollte sich
 *             die Speicherfragmentierung vor allem bei Dateien, die
 *             in kleinen Schritten geschrieben werden, deutlich
 *             verringern. Au�erdem wird es dadurch erst m�glich,
 *             Dateien zu erweitern, die l�nger als der im Moment
 *             noch (an einem St�ck) freie Speicher sind. Der gro�e
 *             Nachteil soll aber nicht verschwiegen werden: Die
 *             Geschwindigkeit beim Lesen und insbesondere beim
 *             Schreiben nimmt ab; trotzdem ist die Ramdisk beim
 *             Lesen noch schneller als eine "konventionelle" Ramdisk
 *             (gemessen mit How-Fast).
 *             An einigen Stellen Tests auf x-Bit eingebaut, falls
 *             sie noch gefehlt haben.
 * 06.12.1995: In link wurde der neue Name in TOS-Schreibweise
 *             gewandelt, wenn die TOS-Domain aktiv war. Da neue
 *             Namen aber eigentlich nie gek�rzt werden, ist dies
 *             jetzt entfernt worden. Dabei wurde gleichzeitig noch
 *             ein weiterer Fehler innerhalb von link entfernt: Wenn
 *             die TOS-Domain aktiv war, wurde beim Verschieben der
 *             Datei der neue Name nicht in Kleinbuchstaben
 *             gewandelt (diese �nderung an Filenamen in der
 *             TOS-Domain ist sinnvoll, da dort in der Regel komplett
 *             gro� geschriebene Namen geliefert werden).
 * 09.12.1995: readlabel liefert jetzt EFILNF zur�ck, um korrekt
 *             anzuzeigen, da� kein Label vorhanden ist bzw. sein
 *             kann.
 * 11.12.1995: writelabel liefert jetzt EACCDN statt EINVFN, um
 *             anzuzeigen, da� keine Labels unterst�tzt werden.
 *             In attrib wurde beim �ndern des Attributs FA_CHANGED
 *             versehentlich xattr.mode statt xattr.attr ver�ndert.
 *             fd_refcnt des Root-dd wird jetzt auf 1 gesetzt, da
 *             MagiC dies ab Kernelversion 3 voraussetzt.
 *             In ddelete wird nur noch dann der refcnt selbst
 *             gepr�ft und freigegeben, wenn die Kernelversion
 *             kleiner als 3 ist.
 *             Bei Neuanlage oder "Truncation" eines Files wird jetzt
 *             automatisch das FA_CHANGED-Attribut gesetzt.
 * 26.12.1995: Neuprogrammierung von path2DD, mit der Hoffnung auf
 *             weniger F�lle, in denen sich ein Verzeichnis angeblich
 *             nicht l�schen l��t.
 * 27.12.1995: Einige dumme Fehler im neuen path2DD ausgebaut.
 *             trace gibt jetzt in ein File aus. Wird beim Start des
 *             XFS eine Umschalttaste gedr�ckt, erscheinen die Debug-
 *             ausgaben wie vorher direkt auf dem Bildschirm.
 *             findfile um Parameter maybe_dir erweitert, mit dem
 *             festgelegt wird, ob bei leerem Suchnamen das aktuelle
 *             Verzeichnis geliefert werden soll.
 *             findfile und die Aufrufe leicht ge�ndert: tostrunc
 *             hei�t jetzt s_or_e und legt fest, ob eine Datei zum
 *             �ffnen gesucht wird (== FF_SEARCH), oder ob nur
 *             getestet werden soll, ob die Datei existiert (dann ist
 *             s_or_e == FF_EXIST, was beim Neuanlegen von Files
 *             wichtig ist).
 *             link arbeitet jetzt endlich ganz richtig (hoffe ich
 *             zumindest...)
 *             Kmalloc ist jetzt eine Funktion, die darauf achtet,
 *             da� der gr��te freie Block mindestens leave_free Bytes
 *             lang ist. Soll Kmalloc den gr��ten freien Block
 *             ermitteln, wird dessen L�nge abz�glich leave_free
 *             geliefert, ggf. 0L.
 * 28.12.1995: Bei ddelete fehlte der Test, ob Schreibrechte f�r das
 *             aktuelle Verzeichnis vorhanden sind.
 *             Neue Funktion work_entry, die einen Verzeichniseintrag
 *             bearbeitet. Sie erh�lt dazu eine Funktion als
 *             Parameter, die die gew�nschten �nderungen vornimmt.
 *             Das besondere an der Funktion ist, da� sie bei
 *             Verzeichnissen daf�r sorgt, da� alle Eintr�ge, die
 *             dieses Directory repr�sentieren, angepa�t werden (also
 *             der Eintrag selbst, . innerhalb des dazugeh�rigen
 *             Verzeichnisses und .. von allen Unterverzeichnissen).
 *             Damit werden hoffentlich bald atime/adate und mtime/
 *             mdate von Verzeichnissen gesetzt.
 *             Unterst�tzung des Dcntl-Kommandos FUTIME sowie der
 *             Fcntl-Kommandos FSTAT, FIONREAD, FIONWRITE und FUTIME.
 * 29.12.1995: Angefangen, atime/adate und mtime/mdate auch bei
 *             Verzeichnissen zu verwalten.
 *             work_entry verfolgt symbolische Links nicht mehr, wenn
 *             f�r symlink ein Nullzeiger �bergeben wird.
 *             Bei write wird nicht mehr die Zugriffszeit ge�ndert,
 *             sondern nur noch die Modifikationszeit.
 * 30.12.1995: Vervollst�ndigung von Zugriffs/Modifikationszeit von
 *             Verzeichnissen.
 *             Bei Debug in Datei wird jetzt die alte Datei umbenannt
 *             und nicht mehr gel�scht.
 *             Bei fopen fehlte der Test, ob es sich �berhaupt um
 *             eine Datei und nicht etwa um ein Verzeichnis handelt.
 *             Das war hoffentlich der Grund f�r die sporadischen
 *             EACCDNs bei ddelete...
 *             In readlink wird jetzt die Zugriffszeit angepa�t.
 *             Bessere (richtige) �berpr�fung des x-Flags von
 *             Verzeichnissen (jetzt in findfile und new_file).
 *             Tracemeldungen erweitert.
 *             Aufsplittung in mehrere Dateien.
 *             Test auf x-Bit in xattr entfernt.
 *             Modus 7 von Dpathconf liefert jetzt korrekt die
 *             unterst�tzten GEMDOS-Attribute.
 *             Beginn der Kommentierung.
 * 31.12.1995: DD-Verwaltung umgestellt. Durch das neue Element
 *             fd_is_parent wird jetzt beim Zur�ckliefern eines DD
 *             in path2DD vermerkt, wie oft die Eltern-DDs schon
 *             Eltern sind. Dabei werden ihre is_parent-Z�hler
 *             jeweils um eins erh�ht, wenn der neue DD bisher noch
 *             nicht benutzt war. Dies wird durch die neue Funktion
 *             increase_refcnts in ramutil.c erledigt, die auch den
 *             refcnt des neuen DDs erh�ht.
 *             Dementsprechend ist auch die Freigabe von DDs durch
 *             freeDD abge�ndert worden. Das neue Verfahren sollte
 *             wesentlich zuverl�ssiger sein und keine DDs freigeben,
 *             die noch in Benutzung sind. Umgekehrt sollten auch
 *             keine "Leichen" mehr dauerhaft bestehen bleiben.
 *             path2DD an neues Parameterlayout angepa�t.
 *             sfirst verlangt jetzt nur noch dann Leserechte f�r das
 *             Verzeichnis, wenn die Maske Wildcards enth�lt.
 *             fopen liefert jetzt bei FA_LABEL im Attribut EACCDN
 *             statt EINVFN.
 * 01.01.1996: Nein, ich habe nicht in's neue Jahr 'reinprogrammiert,
 *             sowas �berlasse ich gerne anderen ;)
 *             Weiterf�hrung der ausf�hrlichen Kommentierung.
 *             In dcreate wurde prepare_dir mit einem falschen Wert
 *             f�r parent aufgerufen. Nichts weltbewegendes, es war
 *             aber ein Fehler...
 *             In link kann jetzt (wieder) ein Verzeichnis innerhalb
 *             der Ordnerhierarchie verschoben werden. Jetzt wird
 *             aber auch der de_faddr-Eintrag von ".." des
 *             betroffenen Verzeichnisses angepa�t.
 *             An einigen Stellen unn�tige Tests entfernt, die schon
 *             erfolgt sind bzw. noch bei anderer Gelegenheit
 *             erfolgen (im Programmablauf, versteht sich).
 *             In chmod wird jetzt work_entry so aufgerufen, da�
 *             symoblische Links erkannt werden, sonst aber EINVFN
 *             geliefert wird.
 *             In fopen fehlten die Tests, ob der neue Name "." oder
 *             ".." ist und ob das aktuelle Verzeichnis noch ge�ffnet
 *             ist.
 *             Bei fdelete fehlte ebenfalls der Test, ob das aktuelle
 *             Verzeichnis noch offen ist.
 *             snext liefert jetzt EFILNF statt ENMFIL, wenn es der
 *             erste Aufruf mit dieser DTA ist, wenn also eigentlich
 *             ein Fsfirst bedient wird.
 * 02.01.1996: Immer noch Kommentierung *st�hn*
 *             get_size hatte noch einen kleinen Fehler, die
 *             bisherige Blockzahl wurde doppelt gez�hlt. Da diese
 *             aber sowieso v�llig �berfl�ssig ist, wurde der
 *             Parameter weggelassen.
 *             Nochmaliges Studium der Doku hat ergeben, da� der
 *             Kernel Fcreate und Fsfirst mit Attribut FA_LABEL schon
 *             selbst auf rlabel und wlabel zur�ckf�hrt, daher sind
 *             die entsprechenden Abfragen in fopen und sfirst
 *             entfernt worden.
 *             Eine sinnlose Gr��enbeschr�nkung von symbolischen
 *             Links entfernt (was hatte ich mir dabei blo� gedacht?)
 *             readlink etwas verbessert (kaum der Rede wert).
 *             In read und write werden jetzt bei falschen Zugriffen
 *             echte Fehler gemeldet und nicht nur, da� nur 0 Bytes
 *             gelesen bzw. geschrieben wurden.
 *             Die do...while-Schleifen in read und write durch
 *             while-Schleifen ersetzt, weil ja denkbar ist, da�
 *             tats�chlich nur 0 Bytes gelesen bzw. geschrieben
 *             werden sollen.
 *             datime ist jetzt FUTIME-kompatibel, d.h. ctime und
 *             cdate werden auf die aktuelle Zeit/das aktuelle Datum
 *             gesetzt.
 *             Fcntl mit FIONWRITE liefert jetzt einen zuverl�ssigen
 *             Wert zur�ck.
 * 03.01.1996: getline funktioniert jetzt richtig.
 *             Sind bei fopen O_CREAT und O_EXCL gesetzt, darf die
 *             Datei nicht existieren; daher wird in diesem Fall
 *             jetzt EACCDN geliefert.
 * 20.01.1996: Die restlichen static-chars durch entsprechende
 *             int_malloc-Aufrufe ersetzt. Die Ramdisk ist dadurch
 *             jetzt voll reentrant.
 * 29.01.1996: Wird sfirst ohne Wildcards aufgerufen, wird direkt
 *             nach der Datei gesucht, ohne den Umweg �ber snext zu
 *             gehen. Somit klappt dann auch das Umbennenen bereits
 *             existierender Files beim Kopieren mit Gemini, das
 *             (zumindest bei dieser Gelegenheit) intelligenterweise
 *             Fsfirst statt Fxattr benutzt, um auf Existenz einer
 *             Datei zu pr�fen. Die �nderung hilft auch bei �lteren
 *             Programmen, die eine als Argumente �bergebene Datei
 *             per Fsfirst suchen.
 *             In link wurde bisher nicht gepr�ft, ob der neue Name
 *             nicht erlaubte Zeichen enth�lt.
 * 30.01.1996: Wird eine Datei mit O_CREAT, O_TRUNC und OM_WPERM
 *             ge�ffnet, versucht die Ramdisk in TOS-Domain-Prozessen
 *             nicht mehr, das File mit 8+3-Vergleich zu finden.
 * 12.02.1996: path2DD �berliest jetzt mehrfache Backslashes.
 * 18.02.1996: In link kann es jetzt nicht mehr passieren, da� ein
 *             Verzeichnis in eines seiner "Nachfahren" verschoben
 *             wird.
 * 26.02.1996: Unterst�tzung von Volume Labels.
 * 02.05.1996: Leider befand sich in der Neufassung von path2DD noch
 *             ein saudummer Fehler, durch den symbolische Links
 *             nicht (mehr) funktionierten...
 * 16.06.1996: Bei drv_open werden jetzt auch d_driver und d_devcode
 *             initialisiert.
 * 21.07.1996: Kleinere Optimierungen in ramdisk_write, die leider
 *             keine sp�rbare Verbesserung der Schreibgeschwindigkeit
 *             bringen :(
 */

#include <string.h>
#include "ramdisk.h"

#pragma warn -par

/*
 * Dieses vielleicht etwas ungeschickte Makro erm�glicht es, den
 * R�ckgabewert jeder Funktion automatisch zu protokollieren
 */
#ifdef DEBUG
#define return(x)	{LONG abccba = (LONG)(x);\
	TRACE(("return(%L), Zeile %L\r\n", abccba,\
		(LONG)__LINE__));\
	return(abccba);}
#endif

/*
 * Um das Syncen brauchen wir uns keine Sorgen zu machen, da die
 * Ramdisk ja keinerlei Caches benutzt (im Prinzip ist sie ein
 * einziger, gro�er Cache ;)
 */
void ramdisk_sync(MX_DMD *d)
{
	TRACE(("sync\r\n"));
}

/*
 * Wird ein Programm beendet, mu� gepr�ft werden, ob es offene
 * Directories "hinterlassen" hat. Wenn ja, sind diese nat�rlich
 * ung�ltig und die zugeh�rige Struktur mu� freigegeben werden.
 * Hatte man bei dopendir Speicher angefordert, mu� dieser nat�rlich
 * dem System zur�ckgegeben werden.
 */
void ramdisk_pterm(MX_DMD *dmd, PD *pd)
{
	WORD	i;

	TRACE(("pterm\r\n"));
	for (i = 0; i < MAX_DHD; i++)
	{
		if (dhd[i].dhd_owner == pd)
			dhd[i].dhd_dir = NULL;
	}
}

/*
 * Eine Garbage Collection brauchen wir auch nicht, weil die Ramdisk
 * nichts dauerhaft per int_malloc anfordert. Zwar k�nnte man in der
 * echten XFS-Struktur (also in *real_xfs) f�r garbcoll einen NULL-
 * Zeiger eintragen, allerdings halte ich es nicht f�r so schlimm,
 * eine Leerfunktion einzubauen. An *real_xfs sollte man �brigens
 * besser nichts ver�ndern, wenn einem die Funktionsf�higkeit seines
 * XFS am Herzen liegt...
 */
LONG ramdisk_garbcoll(MX_DMD *d)
{
	TRACE(("garbcoll\r\n"));
	return(E_OK);
}

/*
 * Die DDs des Ramdisk-XFS sind r�ckw�rts �ber den fd_parent-Zeiger
 * verkettet. Intern wird dabei �ber fd_is_parent gez�hlt, von wie
 * vielen DDs ein anderer der Parent ist. Wenn der Kernel freeDD
 * aufruft, darf der DD also nur dann wirklich freigegeben werden,
 * wenn der is_parent-Z�hler 0 ist. Ebenso werden alle weiter hinten
 * liegenden DDs freigegeben, wenn ihr is_parent-Z�hler nach
 * Erniedrigung um Eins und der refcnt-Z�hler gleich Null sind.
 * Au�erdem werden hier alle DDs freigegeben, deren fd_file nicht
 * NULL ist, obwohl refcnt und is_parent 0 sind. Dies sind Leichen,
 * die bei path2DD auftreten k�nnen, wenn beim weiteren Parsen ein
 * Fehler aufgetreten ist.
 */
void ramdisk_freeDD(MX_DD *dd)
{
	RAMDISK_FD	*i;
	WORD		j;

	TRACE(("freeDD - DD = %L\r\n", dd));
	i = (RAMDISK_FD *)dd;
/* Sicherstellen, da� der DD auch wirklich freigegeben werden soll */
	if (i->fd_refcnt != 0)
	{
		TRACE(("freeDD: fd_refcnt == %L!\r\n", (LONG)i->fd_refcnt));
		return;
	}
/*
 * Den aktuellen is_parent-Z�hler um Eins erh�hen, da er in der
 * Schleife vermindert wird. Auf diese Weise wird nur eine Schleife
 * gebraucht und trotzdem korrekt �berpr�ft, ob der vom Kernel
 * freizugebende DD kein Eltern-DD eines anderen mehr ist.
 */
	i->fd_is_parent++;
	while (i != NULL)
	{
		if (i->fd_is_parent)
			i->fd_is_parent--;
/* Nur freigeben, wenn is_parent und refcnt Null sind */
		if (!i->fd_is_parent && !i->fd_refcnt)
		{
			TRACE(("freeDD: Gebe DD %L frei!\r\n", i));
			i->fd_file = NULL;
		}
/*
 * Den n�chsten DD in der r�ckw�rts verketteten Liste w�hlen. Der
 * parent des Root-DDs, der auf jeden Fall erreicht wird, ist NULL,
 * was die Abbruchbedingung der Schleife ist.
 */
		i = i->fd_parent;
	}
/*
 * Die "Leichen" freigeben; also alle DDs, die nicht frei sind,
 * obwohl refcnt und is_parent beide Null sind
 */
	for (j = ROOT + 1; j < MAX_FD; j++)
	{
		if ((fd[j].fd_file != NULL) && !fd[j].fd_refcnt &&
			!fd[j].fd_is_parent)
		{
			TRACE(("freeDD: Gebe \"Leichen\"-DD %L frei!\r\n",
				&fd[j]));
			fd[j].fd_file = NULL;
		}
	}
}

/*
 * F�r drv_open mu� eine statische Variable gef�hrt werden, die
 * angibt, ob die Ramdisk bereits erfolgreich ge�ffnet wurde (bei
 * Diskwechseln ist d_xfs ja NULL, der Inhalt der Ramdisk hat sich
 * aber nicht ge�ndert). Wichtig ist auch, da� f�r d_xfs der Zeiger
 * real_xfs aus pc_xfs.h eingetragen wird, da ramdisk_xfs nicht die
 * Struktur ist, die vom Kernel angesprochen werden soll.
 */
LONG ramdisk_drv_open(MX_DMD *d)
{
	static WORD	opened_once = 0;

	TRACE(("drv_open - drive %L\r\n", (LONG)d->d_drive));
	if (d->d_xfs == NULL)
	{
/* Pr�fen, ob sich drv_open auf unser Ramdisk-Laufwerk bezieht */
		if (d->d_drive == ramdisk_drive)
		{
/* Wie gesagt: Unbedingt real_xfs f�r d_xfs eintragen! */
			d->d_xfs = real_xfs;
			d->d_root = (MX_DD *)&fd[ROOT];
			d->d_biosdev = -1;
			d->d_driver = 0;
			d->d_devcode = 0;
			if (!opened_once)
			{
/*
 * Wurde die Ramdisk das erste Mal ge�ffnet, m�ssen FDs, DHDs und das
 * Wurzelverzeichnis gel�scht sowie einige wichtige Strukturen
 * vorbereitet werden
 */
				opened_once = 1;
				(kernel->fast_clrmem)(root, &root[ROOTSIZE]);
				prepare_dir(root, ROOTSIZE, ROOT_DE);
				(kernel->fast_clrmem)(dhd, &dhd[MAX_DHD]);
				(kernel->fast_clrmem)(fd, &fd[MAX_FD]);
				fd[ROOT].fd_dmd = ramdisk_dmd = d;
				fd[ROOT].fd_file = &root_de;
				fd[ROOT].fd_parent = NULL;
				fd[ROOT].fd_refcnt = 1;
				root_de = root[1];
				strcpy(root_de.de_fname, "");
				root_de.de_faddr = (char *)root;
			}
/*
 * Diskwechsel k�nnen ja nicht vorkommen (das ist ja der Zweck von
 * drv_open nach dem ersten �ffnen), also wird immer E_OK geliefert
 */
			return(E_OK);
		}
		else
/* War es das falsche Laufwerk, EDRIVE liefern */
			return(EDRIVE);
	}
/*
 * Gleiches gilt, wenn - aus welchem Grund auch immer -, d_xfs nicht
 * mehr auf den richtigen Wert (real_xfs, nicht &ramdisk_xfs!) zeigt.
 * Das kann eigentlich nur passieren, wenn die Ramdisk auf einem
 * Laufwerk angemeldet wurde, das jetzt von einem anderen XFS
 * beansprucht wird.
 */
	if (d->d_xfs != real_xfs)
		return(EDRIVE);
	return(E_OK);
}

/*
 * Bei drv_close m�ssen nur noch die Directory-Handles �berpr�ft
 * werden, da DDs schon vom Kernel �berpr�ft werden. Wird ein noch
 * offenes Directory gefunden, wird bei mode == 0 EACCDN geliefert,
 * ansonsten wird das Handle freigegeben.
 */
LONG ramdisk_drv_close(MX_DMD *d, WORD mode)
{
	WORD	i;

	TRACE(("drv_close - %S\r\n", mode ? "forced" : "requested"));
/*
 * Auch hier sicherheitshalber eine Pr�fung, ob noch das richtige
 * XFS eingetragen ist
 */
	if (d->d_xfs != real_xfs)
		return(EDRIVE);
	for (i = 0; i < MAX_DHD; i++)
	{
/* Pr�fen, ob das Handle Nr. i belegt ist, also noch benutzt wird */
		if (dhd[i].dhd_dir != NULL)
		{
/* Je nach mode entsprechend reagieren */
			if (mode)
				dhd[i].dhd_dir = NULL;
			else
				return(EACCDN);
		}
	}
	return(E_OK);
}

/*
 * path2DD geh�rt zu den XFS-Funktionen, die einem am meisten
 * Kopfzerbrechen bereiten. Die Gr�nde:
 * - Der Pfad mu� komplett selbst bearbeitet, also in einzelne
 *   Komponenten zerlegt werden
 * - Die Zugriffsrechte m�ssen, soweit vorhanden, an den richtigen
 *   Stellen selbst gepr�ft werden
 * - Spezialf�lle wie "." und ".." im Pfad m�ssen beachtet werden
 * - Der Pfad darf nicht dauerhaft ver�ndert werden, da man direkt
 *   den Pfad des Anwenderprogramms geliefert bekommt; andernfalls
 *   kann es schwere Komplikationen geben
 * - Je nach Modus mu� die Bearbeitung an einer anderen Stelle
 *   beendet werden.
 * - Ein DD mu� so geschaffen sein, da� man aus ihm den kompletten
 *   Zugriffspfad mit m�glichst wenig Aufwand rekonstruieren kann
 *   (Stichwort DD2name)
 * - Soll path2DD den DD zu einem Verzeichnis liefern, in dem die
 *   letzte Komponente liegt (mode == 0), darf man sich nicht um
 *   die letzte Komponente k�mmern; es ist also egal, ob es nun
 *   eine Datei, ein Verzeichnis, oder sonstwas ist, die letzte
 *   Komponente mu� noch nicht einmal existieren!
 * An dieser Stelle hat man es mit MiNT wesentlich einfacher, da man
 * hier nur eine Lookup-Funktion f�r einzelne Dateien/Verzeichnisse
 * zur Verf�gung stellen mu�, den Rest erledigt der Kernel. Daf�r ist
 * die MagiC-L�sung in den meisten F�llen deutlich schneller.
 */
LONG ramdisk_path2DD(MX_DD *reldir, const char *pathname, WORD mode,
	const char **lastpath, MX_DD **linkdir, void **symlink)
{
	const char *next;
	const char *current;
	const char *nullbyte;
	char *temp;
	RAMDISK_FD	*new,
				*dd;
	DIRENTRY	*found;
	WORD		dirlookup = mode;

/*
 * Da temp per int_malloc angefordert wird, mu� der Speicher vor
 * Verlassen der Funktion wieder freigegeben werden. Das erreicht man
 * in einer Funktion wie dieser, die an vielen Stellen verlassen
 * wird, am besten, in dem man return so umdefiniert, da� dies
 * automatisch gemacht wird. Nebenbei l��t sich dabei auch noch
 * wundersch�n eine TRACE-Ausgabe der Returnwerte realisieren...
 * Nat�rlich mu� das return-Makro am Ende der Funktion wieder
 * gel�scht werden.
 */
#undef return
#define return(x)	{(kernel->int_mfree)(temp);\
	TRACE(("-> %L, %S, %L, %S; %L\r\n", (LONG)(x), *lastpath,\
		*linkdir, *symlink, (LONG)__LINE__));\
	return(x);}

#ifdef DEBUG
	*lastpath = *symlink = "";
	*linkdir = 0L;
#endif
	TRACE(("path2DD - %L, %S (%L), %S; root = %L\r\n", reldir,
		pathname, pathname, dirlookup ? "Verzeichnis" : "Datei",
		&fd[ROOT]));
/* Speicher f�r eine Pfadkomponente anfordern (bei der
 * Initialisierung wurde ja sichergestellt, da� int_malloc daf�r ein
 * gen�gend gro�es Speicherst�ck alloziert). Tempor�rer Speicher
 * ist n�tig, da der gelieferte Pfad nicht ver�ndert werden darf
 * und eine Komponente auch l�nger als 32 Zeichen sein k�nnte und
 * daher "beschnitten" werden mu�.
 */
	temp = (void *)(kernel->int_malloc)();
	temp[32] = 0;
/* Pr�fen, ob der gelieferte DD �berhaupt OK ist */
	dd = (RAMDISK_FD *)reldir;
	if (check_dd(dd) < 0)
		return(check_dd(dd));
/* Wird ein Nullzeiger geliefert, lehnt path2DD das ab */
	if (pathname == NULL)
		return(dirlookup ? EPTHNF : EFILNF);
/* Den Zeiger auf das Nullbyte des Pfadnamens merken */
	nullbyte = strchr(pathname, 0);
/*
 * In der folgenden Schleife wird jeweils die erste Komponente des
 * Pfades extrahiert (next zeigt auf sie) und dann entsprechend
 * reagiert. Ebenso zeigt dd immer auf den DD des Vorg�ngers, zu
 * Beginn also auf das gelieferte relative Verzeichnis.
 */
	for (next = pathname;;)
	{
/* Eventuell f�hrende Backslashes �berlesen */
		for (; *next == '\\'; next++);
/*
 * Wenn noch weitere Komponenten folgen, mu� der aktuelle DD ein
 * Verzeichnis mit x-Zugriff sein (das x-Bit bei Verzeichnissen sagt
 * in etwa "darf �berschritten werden" aus). Wenn es die letzte
 * Komponente ist, findet keine �berpr�fung statt. Dies wird dann
 * in findfile bzw. new_file nachgeholt.
 */
		if (*next && !xaccess(dd->fd_file))
			return(EACCDN);
/*
 * Folgt keine Komponente mehr, ist der aktuelle DD das Ergebnis der
 * Funktion, er mu� also zur�ckgeliefert werden. Vorher werden noch
 * fd_refcnt und fd_is_parent durch increase_refcnts angepa�t.
 * Dazu noch eine Anmerkung: path2DD wird vom Kernel auch mit leerem
 * Pfadnamen aufgerufen, womit diese Bedingung gleich zu Anfang
 * erf�llt ist. Das passiert beispielsweise, wenn ein Programm
 * Dopendir vom Wurzelverzeichnis aufruft.
 */
		if (!*next)
		{
			increase_refcnts(dd);
			*lastpath = next;
			return((LONG)dd);
		}
/*
 * current ist ein Zeiger auf die aktuelle Komponente, w�hrend
 * pathname auf den kompletten Restpfad zeigt
 */
		pathname = current = next;
/*
 * Nach dem n�chsten Backslash suchen. Wird einer gefunden, mu� die
 * aktuelle Komponente nach temp umkopiert und current auf temp
 * gesetzt werden.
 */
		if ((next = strchr(pathname, '\\')) != NULL)
		{
			size_t len = next - pathname;
			if (len > 32)
				len = 32;
			strncpy(temp, pathname, len);
			temp[len] = '\0';
			current = temp;
/* Backslash(es) �berlesen */
			for (; *next == '\\'; next++);
		}
		else
		{
/*
 * Wurde kein Backslash mehr gefunden, ist zu pr�fen, ob der DD einer
 * Datei gesucht wurde. Falls ja, ist die Suche beendet und der
 * aktuelle DD wird zur�ckgeliefert (nach Erh�hung von fd_refcnt und
 * fd_parent).
 */
			if (!dirlookup)
			{
				increase_refcnts(dd);
				*lastpath = pathname;
				return((LONG)dd);
			}
/*
 * Ist jedoch der DD eines Verzeichnisses gesucht, wird next auf
 * das Nullbyte umgesetzt, damit wird der noch zu belegende neue
 * DD zu Beginn des n�chsten Schleifendurchgangs zur�ckgeliefert.
 */
			next = nullbyte;
		}
/*
 * Ist die aktuelle Komponente "..", wird gepr�ft, ob das aktuelle
 * Verzeichnis das Wurzelverzeichnis ist. Wenn ja, wird dies dem
 * Kernel signalisiert (�hnlich EMOUNT in MiNT). Andernfalls wird nur
 * der DD auf seinen "Vater" umgesetzt und der n�chste Durchgang der
 * Schleife gestartet.
 */
		if (!strcmp(current, ".."))
		{
			if (dd == &fd[ROOT])
			{
				*lastpath = next;
				*linkdir = (MX_DD *)dd;
				*symlink = NULL;
				return(ELINK);
			}
			dd = dd->fd_parent;
			continue;
		}
/* "." wird komplett �bersprungen */
		if (!strcmp(current, "."))
			continue;
/*
 * In allen anderen F�llen mu� die aktuelle Komponente jetzt gesucht
 * werden, sie ist ein Bestandteil des Pfades. Wird sie nicht
 * gefunden, ist der Pfad ung�ltig und es mu� EPTHNF geliefert
 * werden.
 */
		if ((found = findfile(dd, current, 2, FF_SEARCH, 0)) == NULL)
			return(EPTHNF);
/*
 * Ist die Komponente ein symbolischer Link, wird dieser ausgelesen
 * und dem Kernel mit einer entsprechenden Meldung geliefert
 */
		if (is_link(found->de_xattr.st_mode))
		{
			TRACE(("path2DD: Folge symbolischem Link auf %S!\r\n",
				&found->de_faddr[2]));
			increase_refcnts(dd);
			*lastpath = next;
			*linkdir = (MX_DD *)dd;
			*symlink = found->de_faddr;
			return(ELINK);
		}
/* Ist es kein Verzeichnis, ist der Pfad ung�ltig, also EPTHNF */
		if (!is_dir(found->de_xattr.st_mode))
			return(EPTHNF);
/*
 * Einen DD f�r das Verzeichnis anfordern, der auch schon vom
 * gleichen Verzeichnis belegt sein kann (der fd_refcnt wird ja
 * erh�ht). Auf diese Weise ist sichergestellt, da� ein Verzeichnis
 * bei ddelete z.B. nicht mehrere zu �berpr�fende DDs hat. Au�erdem
 * wird dadurch die DD-Ausnutzung effizienter.
 * War allerdings kein DD mehr frei, mu� abgebrochen werden.
 */
		if ((new = findfd(found)) == NULL)
			return(ENSMEM);
		new->fd_dmd = ramdisk_dmd;
		new->fd_file = found;
/*
 * Den aktuellen DD als "Vater" des neuen eintragen und danach den
 * neuen DD zum aktuellen machen
 */
		new->fd_parent = dd;
		dd = new;
	}
/* Wichtig: Das oben definierte return-Makro mu� gel�scht werden! */
#undef return
}

/*
 * Im Debug-Modus wird jetzt wieder return umdefiniert, um jedes
 * return samt R�ckgabewert protokollieren zu k�nnen
 */
#ifdef DEBUG
#define return(x)	{LONG abccba = (LONG)(x);\
	TRACE(("return(%L), Zeile %L\r\n", abccba,\
		(LONG)__LINE__));\
	return(abccba);}
#endif

/*
 * sfirst/snext geh�ren auch zu den gemeineren Zeitgenossen,
 * insbesondere f�r Filesysteme, die nicht mit TOS-Filenamen und
 * -Attributen arbeiten. F�r die Ramdisk wird das Problem durch
 * ein simuliertes Dreaddir im TOS-kompatiblen Modus gel�st, die
 * Suchmaske wird dabei zusammen mit dem Directory, dem Suchattribut
 * und der aktuellen Suchposition im f�r das Filesystem verf�gbaren
 * Bereich der DTA gespeichert (zum Gl�ck reicht der Platz daf�r
 * gerade aus).
 * Die Suchmaske wird vorher in ein spezielles Format gebracht, und
 * zwar immer exakt 8 Zeichen TOS-Filename, Punkt, exakt 3 Zeichen
 * Extension, ggf. wird mit Leerzeichen aufgef�llt. Ein '*' in einem
 * der beiden Namensteile wird durch '?' bis zum Ende des jeweiligen
 * Teils ersetzt. Beispiele:
 * "*.*" -> "????????.???"
 * "AB*.C" -> "AB??????.C  "
 * "Zu langer Name mit Leerzeichen.txt" -> "ZUXLANGE.TXT"
 * Diese Vorgehensweise erleichert sp�ter den Vergleich mit dem
 * aktuellen Filenamen, der auf die gleiche Weise behandelt wird.
 * Es kann zwar zu Problemen kommen, wenn der aktuelle Name '?' oder
 * '*' enth�lt (das Ramdisk-XFS akzeptiert das), doch viel schlimmes
 * wird dabei nicht passieren.
 */
LONG ramdisk_sfirst(MX_DD *srchdir, const char *name, DTA *dta,
	WORD attrib, void **symlink)
{
	RAMDISK_FD	*dd;
	RAMDISK_DTA	*the_dta;
	DIRENTRY	*found;
	char		*temp;

	TRACE(("sfirst - %L\\%S, %L\r\n", srchdir, name, (LONG)attrib));
	dd = (RAMDISK_FD *)srchdir;
	if (check_dd(dd) < 0)
		return(check_dd(dd));
	temp = (void *)(kernel->int_malloc)();
	the_dta = (RAMDISK_DTA *)dta;
	tostrunc(temp, name, 1);
	fill_tosname(the_dta->dta_mask, temp);
	(kernel->int_mfree)(temp);
	the_dta->dta_pos = 0;
/*
 * F�r sfirst ist Leseberechtigung n�tig, sobald die fertige Maske
 * ein '?' enth�lt. In allen anderen F�llen soll nur die Existenz
 * eines bestimmten Files gepr�ft werden, wozu nur x-Rechte n�tig
 * sind, die aber schon von path2DD gepr�ft wurden.
 */
	if ((strchr(the_dta->dta_mask, '?') != NULL))
	{
		if (!raccess(dd->fd_file))
			return(EACCDN);
	}
	else
	{
/*
 * Suche ohne Wildcards wird direkt auf findfile zur�ckgef�hrt (mit
 * Originalmaske!), damit Gemini beim Kopieren bereits existierende
 * Files korrekt umbennenen kann. Auch bei manchen �lteren Programmen
 * kann dies hilfreich sein, wenn sie �bergebene Dateien per Fsfirst
 * suchen: Wenn es ein langer Filename ist, k�nnte der alleinige
 * Vergleich auf 8+3-Ebene das falsche File finden, falls sich die
 * Namen im TOS-Format nicht unterscheiden (z.B. dateiname1 und
 * dateiname2).
 */
		if ((found = findfile(dd, name, 0, FF_SEARCH, 1)) == NULL)
			return(EFILNF);
		the_dta->dta_pos = -found->de_nr;
	}
	the_dta->dta_drive = (char)ramdisk_drive;
	the_dta->dta_attr = (char)attrib;
	the_dta->dta_dir = (DIRENTRY *)dd->fd_file->de_faddr;
/*
 * sfirst selbst liest keinen Verzeichniseintrag, sondern ruft snext
 * auf und liefert dessen Ergebnis
 */
	return(ramdisk_snext((DTA *)the_dta, ramdisk_dmd, symlink));
}

LONG ramdisk_snext(DTA *dta, MX_DMD *dmd, void **symlink)
{
	RAMDISK_DHD	handle;
	RAMDISK_DTA	*the_dta;
	XATTR		xattr;
	WORD		first_call,
				matched;
	LONG		dummy,
				r;
	char		*name;

	TRACE(("snext - %S\r\n", ((RAMDISK_DTA *)dta)->dta_mask));
	if (dmd != ramdisk_dmd)
		return(EDRIVE);
	the_dta = (RAMDISK_DTA *)dta;
/*
 * Wenn dta_pos noch <= 0 ist, stammt der Aufruf von direkt von
 * sfirst, was in first_call gespeichert wird
 */
	first_call = (the_dta->dta_pos <= 0);
	if (the_dta->dta_pos < 0)
		the_dta->dta_pos = -the_dta->dta_pos;
/*
 * Wenn es nicht der erste Aufruf ist und die Maske keine Wildcards
 * enth�lt, mu� gleich ENMFIL geliefert werden
 */
	if (!first_call && (strchr(the_dta->dta_mask, '?') == NULL))
		return(ENMFIL);
/*
 * Es wird ein Pseudo-Directory-Handle eingerichtet, das dann f�r
 * dreaddir benutzt wird. Dabei mu� nat�rlich der TOS-Modus benutzt
 * werden, damit schon passende Namen geliefert werden.
 */
	handle.dhd_dmd = dmd;
	handle.dhd_dir = the_dta->dta_dir;
	handle.dhd_pos = the_dta->dta_pos;
	handle.dhd_tosmode = 1;
/*
 * Zum Auslesen aus der Kernelstruktur immer den Zeiger real_kernel
 * benutzen, kernel soll und mu� nur f�r Funktionsaufrufe genutzt
 * werden
 */
	handle.dhd_owner = *real_kernel->act_pd;
/*
 * In der folgenden Schleife wird solange dreaddir aufgerufen, bis
 * entweder ein Fehler aufgetreten oder ein passender Name mit
 * passendem Attribut gefunden wurde
 */
	for (;;)
	{
		r = ramdisk_dreaddir((MX_DHD *)&handle, 13, the_dta->dta_name, &xattr,
			&dummy);
		the_dta->dta_pos = handle.dhd_pos;
		if (r)
		{
/*
 * Ist die Fehlermeldung ENMFIL und dieses snext ist eigentlich
 * sfirst (sfirst ruft ja am Ende snext auf), mu� stattdessen EFILNF
 * geliefert werden
 */
			if (first_call && (r == ENMFIL))
				r = EFILNF;
			return(r);
		}
		name = (void *)(kernel->int_malloc)();
		fill_tosname(name, the_dta->dta_name);
		matched = match_tosname(name, the_dta->dta_mask);
		(kernel->int_mfree)(name);
		if (matched)
		{
/*
 * Ist der gefundene Name ein symbolischer Link, mu� das dem Kernel
 * signalisiert werden
 */
			if (is_link(xattr.st_mode))
			{
				TRACE(("snext: Folge symbolischem Link auf %S!"
					"\r\n", &((char *)xattr.st_ino)[2]));
				*symlink = (char *)xattr.st_ino;
				return(ELINK);
			}
/*
 * Ansonsten werden Suchattribut und Dateiattribut verglichen. Dabei
 * entspricht die Abfrage dem Codest�ck, das im Profibuch bei Fsfirst
 * angegeben ist.
 */
			TRACE(("Suchattribut: %L, Dateiattribut: %L\r\n",
				(LONG)the_dta->dta_attr, (LONG)xattr.st_attr));
			TRACE(("Mode: %L\r\n", (LONG)xattr.st_mode));
			if (xattr.st_attr == 0)
				break;
			if (xattr.st_attr & (FA_CHANGED|FA_RDONLY))
				break;
			if (the_dta->dta_attr & xattr.st_attr)
				break;
		}
	}
/*
 * Wurde ein passender Eintrag gefunden, mu� die DTA entsprechend
 * gef�llt werden. Als Zeit wird dabei immer die der letzten
 * Modifikation geliefert, was am ehesten zutrifft.
 */
	the_dta->dta_attribute = xattr.st_attr;
	the_dta->dta_time = xattr.st_mtim.u.d.time;
	the_dta->dta_date = xattr.st_mtim.u.d.date;
	the_dta->dta_len = xattr.st_size;
	return(E_OK);
}

/*
 * fopen beim Ramdisk-XFS ist bislang unvollst�ndig, da eine Datei
 * nur ein einziges Mal ge�ffnet werden kann. Filesharing ist so
 * nicht m�glich, selbst ein und derselbe Proze� kann eine Datei
 * nicht mehrmals �ffnen.
 * Bei fopen sind auch wieder eine ganze Menge von Dingen zu
 * beachten:
 * - Wenn die Datei nicht gefunden wird, mu� sie angelegt werden,
 *   wenn das O_CREAT-Flag in omode gesetzt ist
 * - Sind O_CREAT und O_EXCL gesetzt, darf die Datei noch nicht
 *   existieren
 * - Ist die zu �ffnende Datei ein symbolischer Link, mu� das dem
 *   Kernel gemeldet werden
 * - Directories d�rfen nicht per fopen ge�ffnet werden
 * - Darf die Datei beschrieben bzw. ausgelesen werden?
 * - Die Datei mu� ggf. gek�rzt werden (O_TRUNC)
 * - Bei den bekannten Extensionen von ausf�hrbaren Programmen sollte
 *   beim Anlegen automatisch das x-Flag gesetzt werden
 * - Wird Filesharing unterst�tzt, mu� nat�rlich gepr�ft werden, ob
 *   der neue Modus mit den bisherigen harmoniert
 */
LONG ramdisk_fopen(MX_DD *dir, const char *name, WORD omode, WORD attrib,
	void **symlink)
{
	RAMDISK_FD	*dd,
				*new_fd;
	DIRENTRY	*found;
	FILEBLOCK	*file,
				*next;

	TRACE(("fopen - %L\\%S, %L, %L\r\n", dir, name, (LONG)omode,
		(LONG)attrib));
	dd = (RAMDISK_FD *)dir;
	if (check_dd(dd) < 0)
		return(check_dd(dd));
/*
 * Diese �berpr�fung ist eigentlich nicht n�tig, da findfile auch das
 * x-Flag testet, aber so wird EACCDN statt EFILNF geliefert
 */
	if (!xaccess(dd->fd_file))
	{
		TRACE(("fopen: x-Bit fehlt!\r\n"));
		return(EACCDN);
	}
/*
 * Wenn O_CREAT oder O_TRUNC gesetzt ist, mu� auch Schreibzugriff
 * gew�nscht sein, sonst stimmt etwas nicht
 */
	if (((omode & O_CREAT) || (omode & O_TRUNC)) &&
		((omode & OM_WPERM) != OM_WPERM))
	{
		TRACE(("fopen: O_CREAT bzw. O_TRUNC ohne OM_WPERM!\r\n"));
		return(EACCDN);
	}
/*
 * File suchen, ohne 8+3-Vergleich, wenn die Datei neu angelegt
 * werden soll (OM_WPERM, O_CREAT und O_TRUNC gesetzt), sonst mit
 */
	if ((omode & (OM_WPERM | O_CREAT | O_TRUNC)) ==
		(OM_WPERM | O_CREAT | O_TRUNC))
	{
		found = findfile(dd, name, 2, FF_EXIST, 0);
	}
	else
		found = findfile(dd, name, 2, FF_SEARCH, 0);
	if (found != NULL)
	{
/*
 * Wird das File gefunden und es ist ein symbolischer Link, mu� das
 * dem Kernel signalisiert werden
 */
		if (is_link(found->de_xattr.st_mode))
		{
			TRACE(("fopen: Folge symbolischem Link auf %S!\r\n",
				&found->de_faddr[2]));
			*symlink = found->de_faddr;
			return(ELINK);
		}
/* Verzeichnisse k�nnen nicht als Datei ge�ffnet werden */
		if (is_dir(found->de_xattr.st_mode))
			return(EFILNF);
/* Schreiben in schreibgesch�tzte Dateien geht auch nicht */
		if ((omode & OM_WPERM) && !waccess(found))
		{
			TRACE(("fopen: OM_WPERM auf schreibgesch�tzte Datei!\r\n"));
			return(EACCDN);
		}
/*
 * Sollten die Modi O_CREAT und O_EXCL gesetzt sein, mu� EACCDN
 * geliefert werden, da gew�nscht wurde, eine neue Datei anzulegen,
 * die bislang nicht existiert
 */
		if ((omode & (O_CREAT | O_EXCL)) == (O_CREAT | O_EXCL))
		{
			TRACE(("fopen: Datei existiert, O_CREAT und O_EXCL sind "
				"aber gew�nscht!\r\n"));
			return(EACCDN);
		}
	}
/*
 * Einen FD f�r die Datei anfordern, Fehler melden, wenn das nicht
 * klappt
 */
	if ((new_fd = findfd(found)) == NULL)
		return(ENSMEM);
/*
 * Gibt es schon einen belegten FD f�r diese Datei, wird mit EACCDN
 * abgebrochen. Genaugenommen m��te jetzt hier eine �berpr�fung
 * stattfinden, ob die Zugriffsmodi kompatibel sind. Das habe ich mir
 * aber bisher gespart...
 */
	if (new_fd->fd_parent != NULL)
	{
		TRACE(("fopen: Datei schon ge�ffnet!\r\n"));
		TRACE(("rootDD = %L, new_fd = %L, new_fd->fd_parent = %L\r\n",
			&fd[ROOT], new_fd, new_fd->fd_parent));
		return(EACCDN);
	}
	if (found == NULL)
	{
/*
 * Die Datei war noch nicht vorhanden, also mu� sie angelegt werden.
 * Dazu mu� aber O_CREAT gesetzt sein, sonst wurde nur �ffnen einer
 * bereits existenten Datei gew�nscht und es mu� EFILNF geliefert
 * werden.
 */
		if ((omode & O_CREAT) != O_CREAT)
		{
			TRACE(("fopen: File nicht gefunden, kein O_CREAT!\r\n"));
			return(EFILNF);
		}
#ifdef CHECK_OPEN
/* Pr�fen, ob das aktuelle Verzeichnis nicht noch ge�ffnet ist */
	if (dir_is_open((DIRENTRY *)dd->fd_file->de_faddr))
	{
		TRACE(("fopen: Dir offen!\r\n"));
		return(EACCDN);
	}
#endif
/* Nat�rlich darf keine Datei namens "." oder ".." angelegt werden */
		if (!strcmp(name, ".") || !strcmp(name, ".."))
		{
			TRACE(("fopen: Name ist \".\" oder \"..\"!\r\n"));
			return(EACCDN);
		}
/*
 * Neuen Verzeichniseintrag anlegen, ggf. Fehler melden. Der Filename
 * wird bei TOS-Domain-Prozessen nur in Kleinbuchstaben gewandelt,
 * nicht in's 8+3-Format. Das ist eine Kompromi�l�sung, weil viele
 * Programme zwar mit langen Dateinamen zurecht kommen, unter MagiC
 * aber nicht in die MiNT-Domain schalten, weil sie das Vorhandensein
 * von Pdomain vom MiNT-Cookie abh�ngig machen, anstatt die Funktion
 * einfach aufzurufen.
 */
		if ((found = new_file(dd, name)) == NULL)
		{
			TRACE(("fopen: File konnte nicht angelegt werden!\r\n"));
			return(EACCDN);
		}
/* Speicher anfordern, ggf. Fehler melden */
		if ((file = Kmalloc(sizeof(FILEBLOCK))) == NULL)
			return(ENSMEM);
		file->next = NULL;
/*
 * Jetzt ist sichergestellt, da� tats�chlich ein neuer Eintrag
 * entsteht, also mu� die Modifikationszeit des Verzeichnisses
 * angepa�t werden
 */
		work_entry(dd, ".", NULL, 1, 0L, 0L, set_amtime);
/* Die noch leeren Elemente im neuen Eintrag f�llen */
		found->de_faddr = (char *)file;
		found->de_xattr.st_mode = S_IFREG;
/* Bei passender Endung das x-Flag setzen */
		if ((omode & OM_EXEC) || has_xext(name))
			found->de_xattr.st_mode |= 0777;
		else
			found->de_xattr.st_mode |= 0666;
/*
 * Wird ein schreibgesch�tztes File neu angelegt, m�ssen die
 * Zugriffsrechte beschnitten werden. FA_CHANGED wird �brigens immer
 * gesetzt, GEMDOS macht das beim Anlegen einer leeren Datei nicht,
 * dadurch gehen beim Backup u.U. Eintr�ge verloren.
 */
		if (attrib & FA_RDONLY)
		{
			found->de_xattr.st_mode &= ~(S_IWUSR | S_IWGRP | S_IWOTH);
			attrib = FA_RDONLY | FA_CHANGED;
		}
		else
			attrib = FA_CHANGED;
		found->de_xattr.st_ino = (LONG)file;
		found->de_xattr.st_size = 0;
		found->de_xattr.st_blocks = 1;
		found->de_xattr.st_attr = attrib;
	}
	else
	{
		if (omode & O_TRUNC)
		{
/*
 * Falls die Datei schon existierte und O_TRUNC gesetzt ist, mu� das
 * File jetzt auf L�nge Null gek�rzt werden. Tats�chlich bleibt dabei
 * ein Fileblock vorhanden, eine Datei mit L�nge 0 belegt auf der
 * Ramdisk trotzdem einen Fileblock. Das ist zwar nicht so toll,
 * erleichert aber einige Dinge.
 */
			file = (FILEBLOCK *)found->de_faddr;
			next = file->next;
			file->next = NULL;
			file = next;
			while (file != NULL)
			{
				next = file->next;
				Kfree(file);
				file = next;
			}
			found->de_xattr.st_size = 0;
			found->de_xattr.st_blocks = 1;
			found->de_xattr.st_attr |= FA_CHANGED;
		}
	}
/* Den neuen FD auff�llen und zur�ckliefern */
	new_fd->fd_dmd = ramdisk_dmd;
	new_fd->fd_refcnt = 1;
	new_fd->fd_mode = omode;
/*
 * Hier darf die Ramdisk-Struktur eingetragen werden, da die
 * Parameterformate und die Registerbenutzung im Gegensatz zur
 * THE_MGX_XFS-Struktur kompatibel sind
 */
	new_fd->fd_dev = &ramdisk_dev;
	new_fd->fd_fpos = 0L;
	new_fd->fd_file = found;
	new_fd->fd_parent = dd;
/*
 * Zum Auslesen aus der Kernelstruktur immer den Zeiger real_kernel
 * benutzen, kernel soll und mu� nur f�r Funktionsaufrufe genutzt
 * werden
 */
	new_fd->fd_owner = *real_kernel->act_pd;
	return((LONG)new_fd);
}

/*
 * Eine Datei soll gel�scht werden. Es ist zwar sch�n, da� der Kernel
 * die �berpr�fung, ob die Datei noch offen ist, dem XFS �berl��t,
 * aber laut Doku mu� diese �berpr�fung ergeben, da� die Datei eben
 * nicht mehr offen ist. Dadurch ist es z.B. nicht m�glich, die
 * Datei zu l�schen, solange nur lesend auf sie zugegriffen wird.
 * Physikalisch gel�scht w�rde sie dann erst, wenn der letzte Proze�
 * die Datei schlie�t.
 */
LONG ramdisk_fdelete(MX_DD *dir, const char *name)
{
	RAMDISK_FD	*dd,
				*fd;
	DIRENTRY	*found;
	FILEBLOCK	*file,
				*next;

	TRACE(("fdelete - %L\\%S\r\n", dir, name));
	dd = (RAMDISK_FD *)dir;
	if (check_dd(dd) < 0)
		return(check_dd(dd));
/* Die Datei suchen; existiert sie nicht, Fehler melden */
	if ((found = findfile(dd, name, 2, FF_SEARCH, 0)) == NULL)
		return(EFILNF);
#ifdef CHECK_OPEN
/* Pr�fen, ob das aktuelle Verzeichnis nicht noch ge�ffnet ist */
	if (dir_is_open((DIRENTRY *)dd->fd_file->de_faddr))
	{
		TRACE(("fdelete: Dir offen!\r\n"));
		return(EACCDN);
	}
#endif
/*
 * F�r das L�schen von Dateien sind Schreib- und Zugriffsrechte f�r
 * das betroffene Verzeichnis n�tig
 */
	if (!waccess(dd->fd_file) || !xaccess(dd->fd_file))
	{
		TRACE(("fdelete: Kein Schreibrecht f�r Verzeichnis!\r\n"));
		return(EACCDN);
	}
/* Verzeichnisse k�nnen nicht per fdelete gel�scht werden */
	if (is_dir(found->de_xattr.st_mode))
	{
		TRACE(("fdelete: Datei ist Verzeichnis!\r\n"));
		return(EACCDN);
	}
/* Pr�fen, ob die Datei noch offen ist und entsprechend reagieren */
	if (((fd = findfd(found)) != NULL) && (fd->fd_parent != NULL))
	{
		TRACE(("fdelete: Datei noch ge�ffnet!\r\n"));
		return(EACCDN);
	}
/* Auch f�r die Datei selbst m�ssen Schreibrechte vorhanden sein */
	if (!waccess(found))
	{
		TRACE(("fdelete: Datei ist schreibgesch�tzt!\r\n"));
		return(EACCDN);
	}
/*
 * Jetzt den durch die Datei (oder den Link) belegten Speicher
 * freigeben und den Eintrag im Verzeichnis als leer kennzeichnen
 */
	if (is_link(found->de_xattr.st_mode))
		Kfree(found->de_faddr);
	else
	{
		file = (FILEBLOCK *)found->de_faddr;
		do
		{
			next = file->next;
			Kfree(file);
			file = next;
		} while (file != NULL);
	}
	found->de_faddr = NULL;
/*
 * Schlie�lich noch die Modifikationszeit des betroffenen
 * Verzeichnisses anpassen
 */
	work_entry(dd, ".", NULL, 1, 0L, 0L, set_amtime);
	return(E_OK);
}

/*
 * Das Ramdisk-XFS unterst�tzt keine Hardlinks, daher werden in
 * dieser Funktion nur das Umbennenen bzw. Verschieben von Dateien,
 * Links und Verzeichnissen erm�glicht.
 * Wird dabei ein Verzeichnis echt verschoben, mu� die Adresse von
 * ".." angepa�t werden, da sich ja das Elternverzeichnis ge�ndert
 * hat. Ebenso mu� dann sichergestellt sein, da� das Zielverzeichnis
 * nicht zu den "Nachfahren" des Quellverzeichnis geh�rt (man darf
 * z.B. \usr nicht nach \usr\local verschieben).
 */
LONG ramdisk_link(MX_DD *olddir, MX_DD *newdir, const char *oldname,
	const char *newname, WORD flag_link)
{
	RAMDISK_FD	*old,
				*new,
				*i;
	DIRENTRY	*e_old,
				*e_new;
	char		*temp;

	if (flag_link)
	{
		TRACE(("link - hardlinks not supported!\r\n"));
		return(EINVFN);
	}
	TRACE(("link - rename %L\\%S to %L\\%S\r\n", olddir, oldname,
		newdir, newname));
	old = (RAMDISK_FD *)olddir;
	new = (RAMDISK_FD *)newdir;
	if (check_dd(old) < 0)
		return(check_dd(old));
	if (check_dd(new) < 0)
		return(check_dd(new));
/*
 * F�r beide betroffenen Verzeichnisse m�ssen die entsprechenden
 * Rechte vorhanden sein
 */
	if (!waccess(old->fd_file) || !waccess(new->fd_file) ||
		!xaccess(old->fd_file) || !xaccess(new->fd_file))
	{
		return(EACCDN);
	}
/* Pr�fen, ob der gew�nschte neue Name zul�ssig ist */
	if (!check_name(newname))
		return(EACCDN);
/*
 * Den neuen Namen auf die maximale L�nge stutzen und, wenn der
 * aufrufende Proze� in der TOS-Domain l�uft, in Kleinbuchstaben
 * umwandeln (diese Umwandlung ist sinnvoll, weil TOS-Domain-
 * Prozesse in der Regel komplett gro� geschriebene Filenamen
 * liefern, was auf einem case-sensitiven Filesystem unpraktisch
 * ist)
 */
	temp = (void *)(kernel->int_malloc)();
	temp[32] = 0;
	strncpy(temp, newname, 32L);
	if (p_Pdomain(-1) == 0)
		strlwr(temp);
	newname = temp;
/*
 * Das umzubennende File mu� gefunden werden, notfalls, je nach
 * MagiC-Version und aktueller Domain, auch mit 8+3-Vergleichen
 */
	if ((e_old = findfile(old, oldname, 2, FF_SEARCH, 0)) == NULL)
	{
		(kernel->int_mfree)(temp);
		return(EFILNF);
	}
/*
 * Im Gegensatz dazu mu� beim Zielfilenamen nur sichergestellt sein,
 * da� nicht schon ein Eintrag mit exakt dem selben Namen existiert;
 * daher wird nur mit FF_EXIST gesucht
 */
	if ((e_new = findfile(new, newname, 0, FF_EXIST, 0)) != NULL)
	{
		(kernel->int_mfree)(temp);
		return(EACCDN);
	}
/*
 * Ist der Quelleintrag ein Verzeichnis, mu� sichergestellt werden,
 * da� das Zielverzeichnis nicht unterhalb liegt. Dazu reicht es aus,
 * die DD-Kette dies Zielverzeichnis r�ckw�rts nach Vorkommen des
 * Quelleintrags abzusuchen. Wird er gefunden, ist das Verschieben
 * nicht m�glich.
 */
	if (is_dir(e_old->de_xattr.st_mode))
	{
		for (i = new; i->fd_parent != NULL; i = i->fd_parent)
		{
			if (i->fd_file == e_old)
			{
				(kernel->int_mfree)(temp);
				return(EACCDN);
			}
		}
	}
/*
 * Der nachfolgende Test ist defaultm��ig nicht aktiv, weil zumindest
 * Thing beim Verschieben von Dateien in Verzeichnissen Frename
 * direkt nach D(x)readdir aufruft. Ein solches Vorgehen ist nicht
 * empfehlenswert, es ist besser, erst alle Dateinamen in eine Liste
 * einzulesen und sie erst nach dem Dclosedir der Reihe nach
 * zu verschieben.
 */
#ifdef CHECK_OPEN
/*
 * Sicherstellen, da� die beiden betroffenen Verzeichnisse nicht
 * noch per Dopendir ge�ffnet sind
 */
	if (dir_is_open((DIRENTRY *)old->fd_file->de_faddr))
	{
		(kernel->int_mfree)(temp);
		return(EACCDN);
	}
	if (dir_is_open((DIRENTRY *)new->fd_file->de_faddr))
	{
		(kernel->int_mfree)(temp);
		return(EACCDN);
	}
#endif
/* Wenn die beiden DDs gleich sind, soll nur umbenannt werden */
	if (old == new)
	{
		strcpy(e_old->de_fname, newname);
/* Die Modifikationszeit des aktuellen Verzeichnisses anpassen */
		work_entry(old, ".", NULL, 1, 0L, 0L, set_amtime);
		(kernel->int_mfree)(temp);
		return(E_OK);
	}
/*
 * Sonst versuchen, einen neuen Eintrag im Zielverzeichnis anzulegen
 */
	if ((e_new = new_file(new, newname)) == NULL)
	{
		(kernel->int_mfree)(temp);
		return(EACCDN);
	}
/*
 * Klappte das, die Modifikationszeiten der beiden Verzeichnisse
 * anpassen, den alten Eintrag in den neuen kopieren, den neuen Namen
 * eintrage und den alten Eintrag freigeben
 */
	work_entry(old, ".", NULL, 1, 0L, 0L, set_amtime);
	work_entry(new, ".", NULL, 1, 0L, 0L, set_amtime);
	*e_new = *e_old;
	strcpy(e_new->de_fname, newname);
	e_old->de_faddr = NULL;
/*
 * Wurde ein Unterverzeichnis auf diese Weise in ein neues Directory
 * verschoben, mu� noch der ".."-Eintrag angepa�t werden
 */
	if (is_dir(e_new->de_xattr.st_mode))
	{
		((DIRENTRY *)e_new->de_faddr)[1].de_faddr =
			new->fd_file->de_faddr;
		((DIRENTRY *)e_new->de_faddr)[1].de_xattr.st_ino =
			(LONG)new->fd_file->de_faddr;
	}
	(kernel->int_mfree)(temp);
	return(E_OK);
}

/*
 * F�r die Ramdisk ist xattr keine allzu gro�e Schwierigkeit, weil
 * ein Verzeichniseintrag schon die komplette XATTR-Struktur
 * enth�lt. "Echte" Filesysteme m�ssen hier unter Umst�nden stark
 * tricksen, um einige der Felder korrekt zu belegen. �brigens sollte
 * man nicht vergessen, im R�ckgabewert von Dpathconf mit Modus 8
 * anzugeben, welche Felder der XATTR-Struktur mit verl��lichen
 * Werten gef�llt werden.
 */
LONG ramdisk_xattr(MX_DD *dir, const char *name, XATTR *xattr, WORD mode,
	void **symlink)
{
	RAMDISK_FD	*dd;
	DIRENTRY	*found;

	TRACE(("xattr - %L\\%S (%L)\r\n", dir, name, name));
	dd = (RAMDISK_FD *)dir;
	if (check_dd(dd) < 0)
	{
		TRACE(("xattr: check_dd fehlgeschlagen!\r\n"));
		return(check_dd(dd));
	}
	TRACE(("xattr: %S\r\n", name));
/* Das angeforderte File suchen, ggf. Fehler melden */
	if ((found = findfile(dd, name, 0, FF_SEARCH, 1)) == NULL)
	{
		TRACE(("xattr: %S nicht gefunden!\r\n", name));
		return(EFILNF);
	}
/*
 * Ist das betroffene File ein symbolischer Link und sollen solche
 * verfolgt werden, mu� das dem Kernel gemeldet werden
 */
	if (!mode && is_link(found->de_xattr.st_mode))
	{
		TRACE(("xattr: Folge symbolischem Link auf %S!\r\n",
			&found->de_faddr[2]));
		*symlink = found->de_faddr;
		return(ELINK);
	}
/* In allen anderen F�llen die Zielstruktur auff�llen */
	*xattr = found->de_xattr;
	return(E_OK);
}

/*
 * Das Ermitteln bzw. Setzen der GEMDOS-Attribute kann, je nach
 * Filesystem, eine haarige Angelegenheit sein, da nicht alle
 * von ihnen vorhanden sind oder nur aufwendig simuliert werden
 * k�nnen. Nicht vorhandene Attribute werden beim Setzen einfach
 * ignoriert, dabei mu� nat�rlich darauf geachtet werden, da� man
 * sie auch im R�ckgabewert ausmaskiert hat.
 * Das Ramdisk-XFS benutzt f�r attrib die Hilfsfunktions work_entry
 * (aus ramutil.c), die bei einem Eintrag Modifikationen vorzunehmen,
 * die durch eine �bergebene Funktion realisiert werden (hier:
 * attrib_action). Das besondere an work_entry ist, da� es alle
 * n�tigen Tests selbst durchf�hrt und bei �nderungen an ".", "..",
 * oder einem normalen Verzeichniseintrag automatisch daf�r sorgt,
 * da� die beiden jeweils anderen mitbetroffenen Eintr�ge ebenfalls
 * ge�ndert werden.
 */
LONG ramdisk_attrib(MX_DD *dir, const char *name, WORD rwflag, WORD attrib,
	void **symlink)
{
	TRACE(("attrib - %L\\%S, %L, %L\r\n", dir, name, (LONG)rwflag,
		(LONG)attrib));
	return(work_entry((RAMDISK_FD *)dir, name, symlink, rwflag,
		rwflag, attrib, attrib_action));
}

LONG attrib_action(DIRENTRY *entry, LONG rwflag, LONG attrib)
{
	if (rwflag)
	{
/*
 * Die Ramdisk unterst�tzt, neben dem Verzeichnis-Attribut, das
 * nat�rlich nicht ver�ndert werden darf, nur FA_RDONLY und
 * FA_CHANGED. Im Falle von FA_RDONLY werden entweder allen
 * drei Userklassen (Owner/Group/Others) die Schreibrechte entzogen
 * oder gew�hrt.
 */
		if (attrib & FA_RDONLY)
		{
			entry->de_xattr.st_mode &= ~(S_IWUSR | S_IWGRP | S_IWOTH);
			entry->de_xattr.st_attr |= FA_RDONLY;
		}
		else
		{
			entry->de_xattr.st_mode |= S_IWUSR | S_IWGRP | S_IWOTH;
			entry->de_xattr.st_attr &= ~FA_RDONLY;
		}
/* FA_CHANGED kann bei Verzeichnissen nicht gesetzt werden */
		if ((attrib & FA_CHANGED) && is_file(entry->de_xattr.st_mode))
			entry->de_xattr.st_attr |= FA_CHANGED;
		else
			entry->de_xattr.st_attr &= ~FA_CHANGED;
	}
/*
 * Am Schlu� immer das momentan g�ltige Attribut liefern, damit ist
 * auch die Bedingung erf�llt, da� bei �nderungen nur die tats�chlich
 * gesetzten Attribute als Returncode benutzt werden d�rfen.
 */
	return(entry->de_xattr.st_attr);
}

/*
 * chown wird bislang nicht unterst�tzt, aber trotzdem mu� die
 * Funktion zumindest soweit ausprogrammiert werden, da� symbolische
 * Links erkannt werden (Fchown bezieht sich, ebenso wie Fchmod,
 * immer auf das Ziel eines symbolischen Links, nicht auf den Link
 * selbst!)
 */
LONG ramdisk_chown(MX_DD *dir, const char *name, UWORD uid, UWORD gid,
	void **symlink)
{
	TRACE(("chown - not supported\r\n"));
/*
 * Wird work_entry mit NULL als action aufgerufen, wird EINVFN
 * geliefert, wenn name kein symbolischer Link ist. Ansonsten wird
 * wie �blich *symlink belegt und ELINK gemeldet.
 */
	return(work_entry((RAMDISK_FD *)dir, name, symlink, 0, uid, gid,
		0));
}

/*
 * chmod �ndert die Zugriffsrechte f�r einen Verzeichniseintrag
 * (symbolische Links m�ssen verfolgt werden). F�r Filesysteme, die
 * nicht direkt mit den Unix-Zugriffsrechten arbeiten, mu� hier eine
 * Art "Workaround" realisiert oder die Funktion wie chown ohne echte
 * Funktionalit�t ausgestatt werden. Solche Probleme gibt es �brigens
 * auch im Unix-Bereich, so verwaltet beispielsweise das AFS (ein
 * Netzwerkfilesystem) andere, weitergehende Zugriffsrechte, die dann
 * f�r chmod soweit m�glich "umgemappt" werden.
 * Beim Ramdisk-XFS ist chmod, wie attrib und chown, �ber work_entry
 * realisiert, damit auch die Zugriffsrechte von Verzeichnissen
 * korrekt ge�ndert werden k�nnen.
 */
LONG ramdisk_chmod(MX_DD *dir, const char *name, UWORD mode, void **symlink)
{
	TRACE(("chmod - %L\\%S, %L\r\n", dir, name, (LONG)mode));
	return(work_entry((RAMDISK_FD *)dir, name, symlink, 1, mode, 0L,
		chmod_action));
}

LONG chmod_action(DIRENTRY *entry, LONG _mode, LONG dummy)
{
	UWORD	mode;

/*
 * Der neue Zugriffsmodus wird direkt in die XATTR-Struktur des
 * Verzeichniseintrags kopiert; je nach neuem Zustand der Rechte
 * f�r das Schreiben wird das FA_RDONLY-Bit im GEMDOS-Attribut
 * gesetzt oder gel�scht
 */
	mode = (UWORD)_mode;
	entry->de_xattr.st_mode &= S_IFMT;
	entry->de_xattr.st_mode |= mode;
	if (!waccess(entry))
		entry->de_xattr.st_attr |= FA_RDONLY;
	else
		entry->de_xattr.st_attr &= ~FA_RDONLY;
	return(E_OK);
}

/*
 * Das Anlegen eines neuen Unterverzeichnisses ist nat�rlich recht
 * unspektak�lar...
 */
LONG ramdisk_dcreate(MX_DD *dir, const char *name, UWORD mode)
{
	RAMDISK_FD	*dd;
	DIRENTRY	*entry,
				*new;

	TRACE(("dcreate - %L\\%S, rootDD = %L\r\n", dir, name,
		&fd[ROOT]));
	dd = (RAMDISK_FD *)dir;
	if (check_dd(dd) < 0)
	{
		TRACE(("dcreate: dd fehlerhaft!\r\n"));
		return(check_dd(dd));
	}
#ifdef CHECK_OPEN
/* Pr�fen, ob das aktuelle Verzeichnis nicht noch ge�ffnet ist */
	if (dir_is_open((DIRENTRY *)dd->fd_file->de_faddr))
	{
		TRACE(("dcreate: Dir offen!\r\n"));
		return(EACCDN);
	}
#endif
/*
 * Es darf nat�rlich noch keinen Verzeichniseintrag gleichen Namens
 * geben
 */
	if (findfile((RAMDISK_FD *)dir, name, 0, FF_EXIST, 0) != NULL)
	{
		TRACE(("dcreate: Datei existiert bereits!\r\n"));
		return(EACCDN);
	}
/* Neuen Eintrag anfordern, ggf. Fehler melden */
	if ((entry = new_file(dd, name)) == NULL)
	{
		TRACE(("dcreate: Kein Platz mehr!\r\n"));
		return(EACCDN);
	}
/* Speicher f�r neues Verzeichnis anfordern, ggf. Fehler melden */
	if ((new = Kmalloc(DEFAULTDIR * sizeof(DIRENTRY))) == NULL)
		return(ENSMEM);
/*
 * Erst jetzt ist sichergestellt, da� der neue Eintrag auch von Dauer
 * ist, also kann die Modifikationszeit des aktuellen Verzeichnisses
 * angepa�t werden
 */
	work_entry(dd, ".", NULL, 1, 0L, 0L, set_amtime);
/*
 * Das neue Verzeichnis l�schen, die Eintr�ge "." und ".." anlegen
 * und den neuen Eintrag fertig ausf�llen
 */
	(kernel->fast_clrmem)(new, &new[DEFAULTDIR]);
	prepare_dir(new, (WORD)DEFAULTDIR,
		(DIRENTRY *)dd->fd_file->de_faddr);
	entry->de_faddr = (char *)new;
	entry->de_xattr.st_mode = S_IFDIR | 0777;
	entry->de_xattr.st_ino = (LONG)new;
	entry->de_xattr.st_size = 0;
	entry->de_xattr.st_blocks = 1;
	entry->de_xattr.st_attr = FA_DIR;
	return(E_OK);
}

/*
 * Das L�schen eines Verzeichnisses ist etwas schwieriger, was leider
 * am MagiC-Kernel liegt: Bis Kernelversion 2 (einschlie�lich) mu�
 * man _auf jeden Fall_ selbst den refcnt erniedrigen und ggf. den DD
 * freigegeben (und nur dann darf das Verzeichnis auch gel�scht
 * werden), sonst kommt die Verwaltung ganz gewaltig in's Straucheln.
 * Leider ist bzw. war das noch nicht einmal konkret in der Doku
 * erw�hnt, es hei�t bzw. hie� dort nur, da� der refcnt �berpr�ft
 * werden mu�. Ab Kernelversion 3 regelt der Kernel (voraussichtlich)
 * diesen Teil selbst, was auch wesentlich sinnvoller ist.
 * Wer eine DD-Verwaltung hat, die ein- und demselben Verzeichnis
 * bei path2DD jedesmal einen neuen DD zuordnet, mu� hier nat�rlich
 * noch testen, ob es nicht noch einen anderen DD gibt, der das
 * selbe Verzeichnis referenziert. Da das Ramdisk-XFS DDs mehrfach
 * nutzt, er�brigt sich das Problem hier.
 */
LONG ramdisk_ddelete(MX_DD *dir)
{
	RAMDISK_FD	*dd,
				parent,
				copy;
	DIRENTRY	*the_dir;
	WORD		i,
				cnt,
				max;

	TRACE(("ddelete - %L\r\n", dir));
	dd = (RAMDISK_FD *)dir;
	if (check_dd(dd) < 0)
		return(check_dd(dd));
	TRACE(("ddelete: %L entspricht %L\\%S\r\n", dir, dd->fd_parent,
		dd->fd_file->de_fname));
	if (real_kernel->version < 3)
	{
/*
 * Vor Kernelversion 3 mu� man, wie bereits erw�hnt, den refcnt des
 * DDs selbst erniedrigen und dann pr�fen, ob er Null ist. Wenn nein,
 * darf das Verzeichnis nicht gel�scht werden, weil es vom Kernel
 * noch gebraucht wird.
 */
		if (--dd->fd_refcnt > 0)
		{
			TRACE(("ddelete: refcnt == %L!\r\n",
				(LONG)dd->fd_refcnt));
			return(EACCDN);
		}
	}
	else
	{
/*
 * Ab Kernelversion 3 erledigt der Kernel diese Aufgabe, daher kann
 * hier der die Verringerung und Pr�fung des refcnts entfallen. Der
 * TRACE ist nur aus Sicherheitsgr�nden eingebaut, falls es wider
 * Erwarten doch nicht funktionieren sollte (bis dato habe ich noch
 * nicht die Kernelversion 3).
 */
		TRACE(("ddelete: Kernelversion > 2, kein fd_refcnt-Check!"
			"\r\n"));
	}
/*
 * Vom aktuellen DD eine Kopie machen, weil er u.U. freigegeben wird,
 * womit sein Inhalt nat�rlich verloren geht. Gleiches gilt f�r den
 * DD des "Vaters".
 */
	copy = *dd;
	parent = *(dd->fd_parent);
/* Vor Kernelversion 3 mu� der DD jetzt freigegeben werden */
	if (real_kernel->version < 3)
		ramdisk_freeDD((MX_DD *)dd);
/*
 * Zum L�schen eines Verzeichnisses mu� das Vaterverzeichnis
 * beschreibbar sein
 */
	if (!waccess(parent.fd_file))
	{
		TRACE(("ddelete: Kein Schreibzugriff auf Elternverzeichnis!"
			"\r\n"));
		return(EACCDN);
	}
/* Das Verzeichnis selbst mu� ebenfalls beschreibbar sein */
	if (!waccess(copy.fd_file))
	{
		TRACE(("ddelete: Verzeichnis ist schreibgesch�tzt!\r\n"));
		return(EACCDN);
	}
/* Zum Lesen ge�ffnet darf das Verzeichnis ebenfalls nicht sein */
	the_dir = (DIRENTRY *)copy.fd_file->de_faddr;
	if (dir_is_open(the_dir))
	{
		TRACE(("ddelete: Verzeichnis offen!\r\n"));
		return(EACCDN);
	}
/*
 * Der Check, ob das Vaterverzeichnis noch offen ist, wird auch wegen
 * Problemen mit Thing defaultm��ig nicht eingebunden, weil Thing
 * beim rekursiven L�schen Ddelete aufruft, wenn das Vaterverzeichnis
 * noch ge�ffnet ist :(
 */
#ifdef CHECK_PARENT
/* Gleiches gilt f�r das Vaterverzeichnis */
	if (dir_is_open((DIRENTRY *)parent.fd_file->de_faddr))
	{
		TRACE(("ddelete: Elternverzeichnis offen!\r\n"));
		return(EACCDN);
	}
#endif
/*
 * Jetzt mu� gepr�ft werden, ob das Verzeichnis leer ist, also keine
 * Eintr�ge au�er "." und ".." mehr existieren
 */
	max = the_dir->de_maxnr;
	for (cnt = i = 0; i < max; i++)
	{
		if (the_dir[i].de_faddr != NULL)
		{
			if (++cnt > 2)
			{
				TRACE(("ddelete: Verzeichnis nicht leer!\r\n"));
				return(EACCDN);
			}
		}
	}
/*
 * Ging alles glatt, den Speicher, den das Verzeichnis belegt hat,
 * freigeben, die Modifikationszeit des Elternverzeichnisses anpassen
 * und den Eintrag im Vaterverzeichnis freigeben
 */
	Kfree(the_dir);
	work_entry(&parent, ".", NULL, 1, 0L, 0L, set_amtime);
	copy.fd_file->de_faddr = NULL;
	return(E_OK);
}

/*
 * DD2name soll zu einem gegebenen DD den Zugriffspfad liefern; hier
 * wird deutlich, wozu das Ramdisk-XFS eine r�ckw�rts verkettete DD-
 * Struktur benutzt, da man so ohne gro�e Sucherei den Pfad, der bis
 * zu einer bestimmten Stelle f�hrt, zusammenstellen kann.
 * Unter MagiC 3 wird (zumindest bei mir mit diesem XFS) DD2name nie
 * aufgerufen, wenn man das Laufwerk �ber U: anspricht. Setzt man
 * also auf Laufwerk U: beispielsweise den Pfad \m\testdir und M ist
 * das Laufwerk des XFS, so liefert Dgetpath immer nur \m, DD2name
 * wird �berhaupt nicht aufgerufen. Beim DOS-XFS passiert das
 * interessanterweise nicht... Zum Gl�ck funktioniert mit MagiC 4
 * alles bestens, das Problem besteht also wirklich nur mit MagiC 3.
 */
LONG ramdisk_DD2name(MX_DD *dir, char *name, WORD bufsize)
{
	RAMDISK_FD	*dd;
	char		*temp;

	TRACE(("DD2name - %L\r\n", dir));
/* Wie �blich erstmal pr�fen, ob der dd g�ltig ist */
	dd = (RAMDISK_FD *)dir;
	if (check_dd(dd) < 0)
		return(check_dd(dd));
/*
 * Wenn nicht mindestens ein Byte Platz hat, gleich einen Fehler
 * melden (wegen des abschlie�enden Nullbytes).
 */
	if (bufsize < 1)
		return(ERANGE);
	*name = 0;
	temp = (void *)(kernel->int_malloc)();
/*
 * Jetzt vom aktuellen dd r�ckw�rts bis zu dem Verzeichnis im Pfad
 * gehen, das �ber dem Wurzelverzeichnis liegt. Damit am Ende der
 * Pfad in der richtigen Reihenfolge herauskommt, wird der aktuelle
 * aktuelle Ordnername umgedreht angeh�ngt und das Ergebnis vor der
 * R�ckgabe ebenfalls komplett gedreht. Beim Anh�ngen jedes
 * Zwischenpfades mu� nat�rlich gepr�ft werden, ob im Puffer noch
 * genug Platz ist.
 */
	for (; dd->fd_parent != NULL; dd = dd->fd_parent)
	{
/* F�r TOS-Domain-Prozesse einen verkr�ppelten Pfad liefern */
		if (p_Pdomain(-1) == 0)
			tostrunc(temp, dd->fd_file->de_fname, 0);
		else
			strcpy(temp, dd->fd_file->de_fname);
		if ((WORD)(strlen(name) + strlen(temp) + 1) >= bufsize)
		{
			(kernel->int_mfree)(temp);
			return(ERANGE);
		}
		strrev(temp);
		strcat(name, temp);
		strcat(name, "\\");
	}
/*
 * Zu guter Letzt den erzeugten Pfad umdrehen, damit er die richtige
 * Reihenfolge hat
 */
	strrev(name);
	(kernel->int_mfree)(temp);
	TRACE(("DD2name liefert: %S\r\n", name));
	return(E_OK);
}

/*
 * Bei dopendir mu� man, �hnlich wie bei path2DD, die zu liefernde
 * Struktur selbst zur Verf�gung stellen. In ihr sollte man, neben
 * dem vom Kernel vorgeschriebenen DMD-Zeiger, alles speichern, damit
 * dreaddir m�glichst schnell arbeiten kann. Man sollte ebenfalls den
 * Proze�, der dopendir aufgerufen hat, in der Struktur ablegen,
 * damit man bei dreaddir/dclosedir pr�fen kann, ob der richtige
 * Proze� den Aufruf t�tigt.
 * Man darf bzw. sollte in der Struktur keinen Zeiger auf den
 * DD des zu lesenden Verzeichnisses eintragen, da der Kernel nach
 * Dopendir den DD freigibt. Das erste dreaddir des XFS w�rde dann
 * also einen DD benutzen, der nicht mehr g�ltig und u.U. sogar schon
 * wieder anderweitig vergeben ist.
 */
LONG ramdisk_dopendir(MX_DD *dir, WORD tosflag)
{
	WORD		i;
	RAMDISK_FD	*dd;

	TRACE(("dopendir %L\r\n", dir));
	dd = (RAMDISK_FD *)dir;
	if (check_dd(dd) < 0)
		return(check_dd(dd));
/*
 * Zum Lesen eines Verzeichnisses sind nur Leserechte n�tig, das x-
 * Flag mu� nur dann gesetzt sein, wenn man einen Eintrag innerhalb
 * des Verzeichnisses ansprechen will (es bedeutet, wie bereits
 * erw�hnt, soviel wie "Verzeichnis darf �berschritten werden")
 */
	if (!raccess(dd->fd_file))
		return(EACCDN);
/* Eine freie Directory-Handle-Struktur suchen */
	for (i = 0; i < MAX_DHD; i++)
	{
		if (dhd[i].dhd_dir == NULL)
		{
/*
 * Wurde eine gefunden, diese f�llen. Dabei wird der Zeiger auf den
 * Beginn des zu lesenden Verzeichnisses, das tosflag und der
 * aufrufende Proze� abgelegt. dhd_pos gibt f�r dreaddir an, der
 * wievielte Eintrag der n�chste zu lesende ist und mu� daher zu
 * Beginn auf Null gesetzt werden.
 */
			dhd[i].dhd_dmd = ramdisk_dmd;
			dhd[i].dhd_dir = (DIRENTRY *)dd->fd_file->de_faddr;
			dhd[i].dhd_pos = 0;
			dhd[i].dhd_tosmode = tosflag;
			dhd[i].dhd_owner = *real_kernel->act_pd;
			return((LONG)&dhd[i]);
		}
	}
/* War keine Struktur mehr frei, einen Fehler melden */
	return(ENHNDL);
}

/*
 * In dreaddir hat man wieder zwei Probleme: Der Name mu� unter
 * Umst�nden in's 8+3-Format gewandelt werden und man mu� u.U. die
 * XATTR-Struktur belegen. F�r letzteres kann man nat�rlich die
 * xattr-Funktion benutzen; dazu mu� man nur einen Pseudo-DD
 * anlegen, da xattr ja einen DD f�r das aktuelle Verzeichnis
 * erwartet. Man kann nat�rlich ebenso eine Unterfunktion zum F�llen
 * der XATTR-Struktur schreiben, die sowohl von xattr als auch von
 * dreaddir benutzt wird. F�r das Wandeln in's 8+3-Format sollte von
 * sfirst her auch schon eine Funktion zur Verf�gung stehen, also
 * ist es letztlich nicht so furchtbar schwer, dreaddir zu
 * realisieren.
 * Da ein Verzeichniseintrag der Ramdisk die XATTR-Struktur enth�lt,
 * er�brigt sich das zweite genannte Problem hier ohnehin...
 */
LONG ramdisk_dreaddir(MX_DHD *dhd, WORD size, char *buf, XATTR *xattr,
	LONG *xr)
{
	RAMDISK_DHD	*handle;
	RAMDISK_FD	help;
	DIRENTRY	*dir;
	WORD		pos;

	TRACE(("%S\r\n", (xattr == NULL) ? "dreaddir" : "dxreaddir"));
	handle = (RAMDISK_DHD *)dhd;
/*
 * Zun�chst einmal das Handle pr�fen, dabei auch auf den Proze� 
 * achtenn. Der Test auf NULL ist zwar prinzipiell unn�tig, weil der
 * Kernel ja schon den DMD ermittelt haben mu�, aber sicher ist
 * sicher.
 */
	if ((handle == NULL) || (handle->dhd_dmd != ramdisk_dmd) ||
		(handle->dhd_owner != *real_kernel->act_pd))
	{
		return(EIHNDL);
	}
/*
 * Da bei einem Lesezugriff auf ein Verzeichnis dessen Zugriffszeit
 * ge�ndert werden mu�, wird work_entry aufgerufen. Diese Funktion
 * erwartet allerdings einen DD, daher mu� ein solcher generiert
 * werden (�hnlich wie oben f�r den Aufruf von xattr vorgeschlagen).
 */
	help.fd_dmd = ramdisk_dmd;
	help.fd_refcnt = 0;
	help.fd_file = handle->dhd_dir;
	help.fd_parent = NULL;
	work_entry(&help, ".", NULL, 1, 1L, 0L, set_amtime);
	dir = handle->dhd_dir;
	pos = handle->dhd_pos;
/* Den n�chsten nicht-leeren Verzeichniseintrag suchen */
	for (; (pos < dir->de_maxnr) && (dir[pos].de_faddr == NULL);
		pos++);
/* Gibt es keinen mehr, das Ende des Lesevorgangs signalisieren */
	if (pos >= dir->de_maxnr)
		return(ENMFIL);
/*
 * Ansonsten je nach Modus den Namen in's 8+3-Format quetschen oder
 * ihn unver�ndert samt Index (nicht vergessen!) ablegen. Dabei mu�
 * immer darauf geachtet werden, da� der Zielpuffer gen�gend Platz
 * bietet!
 */
	if (handle->dhd_tosmode)
	{
		if (size < 13)
			return(ERANGE);
		tostrunc(buf, dir[pos].de_fname, 0);
	}
	else
	{
		if (((WORD)strlen(dir[pos].de_fname) + 4) >=
			size)
		{
			TRACE(("%S pa�t nicht in den Puffer!",
				(LONG)dir[pos].de_fname));
			return(ERANGE);
		}
		*(LONG *)buf = (LONG)dir[pos].de_xattr.st_ino;
		strcpy(&buf[4], dir[pos].de_fname);
	}
/*
 * Ggf. auch die XATTR-Struktur gef�llt werden (also im Falle eines
 * Dxreaddir-Aufrufs). Dabei d�rfen symbolische Links nicht verfolgt
 * werden!
 */
	if (xattr != NULL)
	{
		*xattr = dir[pos].de_xattr;
		*xr = 0L;
	}
/*
 * Zum Schlu� noch den Lesezeiger f�r den n�chsten dreaddir-Aufruf
 * setzen
 */
	handle->dhd_pos = pos + 1;
	return(E_OK);
}

/*
 * F�r das "Zur�ckspulen" eines Verzeichnisses mu� nicht viel
 * beachtet werden, das betroffene Filesystem mu� nat�rlich dazu in
 * der Lage sein. Wenn nicht, ist EINVFN zu liefern.
 * Beim Ramdisk-XFS gen�gt es, den Lesezeiger wieder auf Null zu
 * setzen.
 */
LONG ramdisk_drewinddir(MX_DHD *dhd)
{
	RAMDISK_DHD	*handle;

	TRACE(("drewinddir\r\n"));
	handle = (RAMDISK_DHD *)dhd;
/* Wieder das Handle �berpr�fen */
	if ((handle == NULL) || (handle->dhd_dmd != ramdisk_dmd) ||
		(handle->dhd_owner != *real_kernel->act_pd))
	{
		return(EIHNDL);
	}
/* Lesezeiger zur�cksetzen */
	handle->dhd_pos = 0;
	return(E_OK);
}

/*
 * Bei dclosedir hat man die M�glichkeit, Puffer, die man u.U. bei
 * dopendir zur Beschleunigung des Lesens angefordert hat, wieder
 * freizugeben
 */
LONG ramdisk_dclosedir(MX_DHD *dhd)
{
	RAMDISK_DHD	*handle;

	TRACE(("dclosedir\r\n"));
	handle = (RAMDISK_DHD *)dhd;
/* Handle checken */
	if ((handle == NULL) || (handle->dhd_dmd != ramdisk_dmd) ||
		(handle->dhd_owner != *real_kernel->act_pd))
	{
		return(EIHNDL);
	}
/* Handle freigeben */
	handle->dhd_dir = NULL;
	return(E_OK);
}

/*
 * dpathconf ist eine sehr wichtige Funktion, damit Programme
 * Informationen �ber das Filesystem ermitteln k�nnen. Bislang sind
 * 8 Modi definiert, die genaue Bedeutung kann man vollst�ndig im
 * ersten Teil meines Artikels "Alternative Filesysteme im Griff" in
 * der ST-Computer 11/95, Seite 44ff nachlesen.
 * Wenn man einen bestimmten Modus nicht unterst�tzt bzw. nicht
 * unterst�tzen kann, mu� daf�r EINVFN geliefert werden.
 * Die Ergebnisse von dpathconf sind in der Regel vom Verzeichnis
 * unabh�ngig, es ist allerdings bei speziellen Filesystemen durchaus
 * anders denkbar.
 */
LONG ramdisk_dpathconf(MX_DD *dir, WORD which)
{
	TRACE(("dpathconf - %L, %L\r\n", dir, (LONG)which));
	if (check_dd((RAMDISK_FD *)dir) < 0)
		return(check_dd((RAMDISK_FD *)dir));
	switch (which)
	{
		case -1:
/* Maximal Modus 8 */
			return(8);
		case 0:
/*
 * Es k�nnen allerh�chstens soviel Dateien ge�ffnet werden wie FDs
 * vorhanden sind (minus 1 f�r den Root-DD)
 */
			return(MAX_FD - 1);
		case 1:
/* Keine Hardlinks, also maximal 1 Link pro File */
			return(1);
		case 2:
/*
 * Pfadnamen k�nnen unendlich lang werden (genaugenommen zwar nicht,
 * weil ja die Anzahl an DDs begrenzt ist, aber das macht letztlich
 * keinen gro�en Unterschied)
 */
			return(0x7fffffffL);
		case 3:
/* Maximal 32 Zeichen Filename */
			return(32L);
		case 4:
/* "Am St�ck" k�nnen maximal DEFAULTFILE Bytes geschrieben werden */
			return(DEFAULTFILE);
		case 5:
/* Die Ramdisk schneidet zu lange Filenamen automatisch ab */
			return(1L);
		case 6:
/* Volle Unterscheidung von Gro�- und Kleinschreibung */
			return(0L);
		case 7:
/*
 * M�gliche Filetypen: Directories, symbolische Links, normale Files.
 * Alle Unix-Filemodi bis auf Setuid, Setgid und das "Sticky-Bit".
 * TOS-Attribute: Verzeichnis, Nur Lesen, Ver�ndert, symbolischer
 * Link (wie es MagiC benutzt)
 */
			return(0x01900000L | (0777L << 8L) |
				FA_RDONLY | FA_DIR | FA_CHANGED | FA_SYMLINK);
		case 8:
/* Alle Elemente der XATTR-Struktur echt vorhanden */
			return(0x0fffL);
		default:
/* Andere Dpathconf-Modi kennt das Filesystem nicht */
			return(EINVFN);
	}
}

/*
 * Bei dfree ist es mir nach wie vor ein R�tsel, wozu ein DD
 * �bergeben wird, ein DMD h�tte gereicht (schlie�lich wird der freie
 * Platz auf einem Laufwerk nicht pfadabh�ngig sein, au�erdem gibt es
 * nur die GEMDOS-Funktion Dfree, die ein Laufwerk als Parameter
 * erh�lt, keinen Pfad). Bei MiNT ist es �brigens �hnlich, komisch.
 */
LONG ramdisk_dfree(MX_DD *dd, DISKINFO *free)
{
	LONG	freeblocks,
			usedblocks;

	TRACE(("dfree\r\n"));
/*
 * Im Debug-Modus wird protokolliert, welche DDs durch welches
 * Verzeichnis belegt sind. Auf diese Weise kann bei Bedarf gepr�ft
 * werden, ob DDs falsch oder unn�tig belegt sind oder versehentlich
 * freigegeben wurden
 */
#ifdef DEBUG
	{
		WORD	i;

		for (i = 0; i < MAX_FD; i++)
		{
			if (fd[i].fd_file != NULL)
			{	
				TRACE(("fd %L ist belegt durch %S!\r\n", &fd[i],
					((DIRENTRY *)fd[i].fd_file)->de_fname));
			}
		}
	}
#endif
	if (check_dd((RAMDISK_FD *)dd) < 0)
		return(check_dd((RAMDISK_FD *)dd));
/*
 * Die freien Blocks errechnen sich aus dem (f�r die Ramdisk) noch
 * freien Speicher geteilt durch die Gr��e eines Fileblocks. Das
 * ergibt zwar nie einen 100%ig verl��lichen Wert, aber besser wird
 * man es bei einer Ramdisk auch kaum machen k�nnen, da sich der
 * freie Speicher st�ndig �ndern kann.
 */
	freeblocks = ((LONG)Kmalloc(-1) + DEFAULTFILE - 1) / DEFAULTFILE;
/*
 * Die belegten Bl�cke werden rekursiv vom Wurzelverzeichnis aus
 * gez�hlt. Das Wurzelverzeichnis selbst belegt 0 Bl�cke, damit die
 * Ramdisk auch wirklich als leer angesehen wird, wenn keine Dateien
 * oder Verzeichnisse vorhanden sind.
 */
	usedblocks = get_size(root);
/* Die Zielstruktur belegen */
	free->b_free = freeblocks;
	free->b_total = freeblocks + usedblocks;
	free->b_secsiz = DEFAULTFILE;
	free->b_clsiz = 1L;
	return(E_OK);
}

/*
 * Diese direkte Hilfsfunktion f�r dfree ermittelt rekursiv die
 * belegten Blocks relativ zum Directory search
 */
LONG get_size(DIRENTRY *search)
{
	WORD		i;
	LONG		newsize;

	TRACE(("get_size - Verzeichnis %L\r\n", search));
/* Zun�chst die Gr��e des aktuellen Directories selbst ermitteln */
	newsize = search[0].de_xattr.st_blocks;
/*
 * Dann alle Eintr�ge au�er "." und ".." durchgehen und ihre Gr��e
 * addieren, wenn es Dateien oder symbolische Links sind. Bei
 * Verzeichnissen wird get_size rekursiv aufgerufen und das
 * Ergebnis addiert.
 */
	for (i = 2; i < search[0].de_maxnr; i++)
	{
		if (search[i].de_faddr != NULL)
		{
			if (is_dir(search[i].de_xattr.st_mode))
				newsize += get_size((DIRENTRY *)search[i].de_faddr);
			else
				newsize += search[i].de_xattr.st_blocks;
		}
	}
/* Am Ende die neu ermittelte Blockzahl zur�ckliefern */
	return(newsize);
}

/*
 * Hier mu� ein Label angelegt/ge�ndert werden, n�heres siehe
 * MagiC-Doku. Werden keine Labels unterst�tzt, mu� EACCDN geliefert
 * werden.
 */
LONG ramdisk_wlabel(MX_DD *dir, const char *name)
{
	TRACE(("wlabel - %S\r\n", name));
/* dir wird nur �berpr�ft, sonst aber ignoriert */
	if (check_dd((RAMDISK_FD *)dir) < 0)
		return(check_dd((RAMDISK_FD *)dir));
/*
 * Bei Bedarf Volume Label l�schen, sonst die ersten 32 Zeichen des
 * gew�nschten Labels �bernehmen
 */
	if (*name == '\xe5')
		strcpy(volume_label, "");
	else
	{
		volume_label[32] = 0;
		strncpy(volume_label, name, 32L);
	}
	return(E_OK);
}

/*
 * Zum Ermitteln des Volume Labels, n�heres siehe MagiC-Doku.
 * Filesysteme ohne Volume Label sollten hier EFILNF liefern.
 * Der Parameter name wird von der Ramdisk ignoriert, was laut
 * Doku auch zul�ssig ist.
 */
LONG ramdisk_rlabel(MX_DD *dir, const char *name, char *buf, WORD len)
{
	TRACE(("rlabel - %S %L\r\n", name, (LONG)len));
/* dir wird zwar �berpr�ft, sonst aber ignoriert */
	if (check_dd((RAMDISK_FD *)dir) < 0)
		return(check_dd((RAMDISK_FD *)dir));
/*
 * Ist das Label leer, wird EFILNF geliefert, weil genaugenommen
 * keines existiert
 */
	if (!*volume_label)
		return(EFILNF);
/* Pr�fen, ob der Zielpuffer genug Platz bietet */
	if ((WORD)strlen(volume_label) >= len)
		return(ERANGE);
/* Aktuelles Label in Zielpuffer kopieren */
	strcpy(buf, volume_label);
	return(E_OK);
}

/*
 * Ein symbolischer Link soll angelegt werden, werden keine solchen
 * unterst�tzt, ist EINVFN zu liefern. Beim Ramdisk-XFS werden die
 * Links als ein Speicherblock abgelegt (der Inhalt wird ja sowieso
 * nicht mehr ver�ndert) und haben genau das Format, da� MagiC f�r
 * das Zur�ckliefern von Linkzielen bei ELINK vorschreibt. Bis heute
 * ist mir noch nicht so ganz klar, wieso Andreas ein so eigenartiges
 * Format gew�hlt hat, das weder ein Pascal-, noch ein C-String ist,
 * sondern eher ein Gemisch aus beidem... Vielleicht ein GFA-Basic-
 * String? ;>
 */
LONG ramdisk_symlink(MX_DD *dir, const char *name, const char *to)
{
	RAMDISK_FD	*dd;
	DIRENTRY	*entry;
	char		*link;
	LONG		len;

	TRACE(("symlink - %S to %L\\%S\r\n", to, dir, name));
	dd = (RAMDISK_FD *)dir;
	if (check_dd(dd) < 0)
		return(check_dd(dd));
#ifdef CHECK_OPEN
/* Pr�fen, ob das Verzeichnis nicht noch ge�ffnet ist */
	if (dir_is_open((DIRENTRY *)dd->fd_file->de_faddr))
		return(EACCDN);
#endif
/* Herausfinden, ob eine Datei gleichen Namens schon existiert */
	if (findfile(dd, name, 0, FF_EXIST, 0) != NULL)
		return(EACCDN);
/* Versuchen, einen neuen Eintrag zu erhalten */
	if ((entry = new_file((RAMDISK_FD *)dir, name)) == NULL)
		return(EACCDN);
/*
 * Berechnen, wieviel Speicher der Link im MagiC-Format braucht und
 * diesen anfordern
 */
	len = strlen(to) + 1L;
	if (len & 1)
		len++;
	if ((link = Kmalloc(len + 2L)) == NULL)
		return(ENSMEM);
/*
 * Ging alles glatt, jetzt die Modifikationszeit des Verzeichnisses
 * anpassen, das Ziel des Links in den angeforderten Speicherbereich
 * kopieren und den neuen Verzeichniseintrag auff�llen. Dabei wird
 * als GEMDOS-Attribut das von MagiC benutzte FA_SYMLINK eingetragen.
 */
	work_entry(dd, ".", NULL, 1, 0L, 0L, set_amtime);
	*(WORD *)link = (WORD)len;
	strcpy(&link[2], to);
	entry->de_faddr = link;
	entry->de_xattr.st_mode = S_IFLNK | 0777;
	entry->de_xattr.st_ino = (LONG)link;
	entry->de_xattr.st_size = len + 2L;
	entry->de_xattr.st_blocks = (len + 1L + DEFAULTFILE) / DEFAULTFILE;
	entry->de_xattr.st_attr = FA_SYMLINK; /* MagiC-TOSFS-like */
	return(E_OK);
}

/*
 * Beim Lesen eine symbolischen Links mu� darauf geachtet werden, da�
 * size gro� genug ist, den Zugriffspfad samt abschlie�endem Nullbyte
 * aufzunehmen. Unterst�tzt das XFS keine symbolischen Links, mu�
 * auch hier EINVFN geliefert werden.
 */
LONG ramdisk_readlink(MX_DD *dir, const char *name, char *buf, WORD size)
{
	RAMDISK_FD	*dd;
	DIRENTRY	*found;

	TRACE(("readlink - %L\\%S\r\n", dir, name));
	dd = (RAMDISK_FD *)dir;
	if (check_dd(dd) < 0)
		return(check_dd(dd));
/* Den Verzeichniseintrag suchen */
	if ((found = findfile(dd, name, 2, FF_SEARCH, 0)) == NULL)
		return(EFILNF);
/* Wenn es kein symbolischer Link ist, einen Fehler melden */
	if (!is_link(found->de_xattr.st_mode))
		return(EACCDN);
/* Pr�fen, ob der Zielpuffer gro� genug ist */
	if (size < (strlen(&found->de_faddr[2]) + 1L))
		return(ERANGE);
/*
 * Wenn ja, wird die letzte Zugriffszeit des Links gesetzt und der
 * Zielpfad in den Puffer kopiert
 */
	found->de_xattr.st_atim.u.d.time = Tgettime();
	found->de_xattr.st_atim.u.d.date = Tgetdate();
	strcpy(buf, &found->de_faddr[2]);
	return(E_OK);
}

/*
 * dcntl f�hrt bestimmte Aktionen f�r Verzeichniseintr�ge durch. Der
 * Inhalt von arg h�ngt dabei von cmd ab. Nicht unterst�tzte Modi
 * sind mit EINVFN abzulehnen, gleiches gilt nat�rlich auch f�r den
 * Fall, da� dcntl �berhaupt nicht unterst�tzt wird. Auf jeden Fall
 * aber sollten symbolische Links verfolgt werden.
 * Im Ramdisk-XFS ist Dcntl wieder �ber work_entry realisiert, um
 * auch Verzeichnisse bearbeiten zu k�nnen. Bislang wird nur FUTIME
 * (Zeiten ver�ndern) unterst�tzt.
 */
LONG ramdisk_dcntl(MX_DD *dir, const char *name, WORD cmd, LONG arg,
	void **symlink)
{
	RAMDISK_FD	*dd;

	TRACE(("dcntl - %L\\%S, %L, %L\r\n", dir, name, (LONG)cmd,
		arg));
	dd = (RAMDISK_FD *)dir;
	return(work_entry(dd, name, symlink, 1, cmd, arg, dcntl_action));
}

LONG dcntl_action(DIRENTRY *entry, LONG cmd, LONG arg)
{
	WORD	*timebuf;

	switch ((WORD)cmd)
	{
/*
 * Zu FUTIME habe ich bisher keine brauchbare Doku gefunden, daher
 * hier eine kurze Beschreibung (die Funktionsweise habe ich in den
 * MinixFS-Sourcen entnommen):
 * F�r das Kommando FUTIME zeigt arg auf ein WORD-Array mit vier
 * Elementen. Die Belegung:
 * arg[0] - Uhrzeit des letzten Zugriffs
 * arg[1] - Datum des letzten Zugriffs
 * arg[2] - Uhrzeit der letzten Modifikation
 * arg[3] - Datum der letzten Modifikation
 * Wenn arg ein Nullzeiger ist, sollen alle drei Zeiten (also auch
 * die Erstellungszeit) auf das aktuelle Datum gesetzt werden, sonst
 * atime/adate auf arg[0/1], mtime/mdate auf arg[2/3] und ctime/cdate
 * auf die aktuelle Uhrzeit/das aktuelle Datum. Die Erstellungszeit
 * (also ctime/cdate) wird immer auf die aktuelle Zeit gesetzt, damit
 * man eine Datei nicht zur�ckdatieren kann (das kann durchaus
 * wichtig sein).
 */
		case FUTIME:
/* F�r FUTIME braucht man Schreibzugriff auf die Datei */
			if (!waccess(entry))
				return(EACCDN);
			timebuf = (WORD *)arg;
			entry->de_xattr.st_ctim.u.d.time = Tgettime();
			entry->de_xattr.st_ctim.u.d.date = Tgetdate();
			if (timebuf != NULL)
			{
				entry->de_xattr.st_atim.u.d.time = timebuf[0];
				entry->de_xattr.st_atim.u.d.date = timebuf[1];
				entry->de_xattr.st_mtim.u.d.time = timebuf[2];
				entry->de_xattr.st_mtim.u.d.date = timebuf[3];
			}
			else
			{
				entry->de_xattr.st_atim.u.d.time = entry->de_xattr.st_mtim.u.d.time =
					entry->de_xattr.st_ctim.u.d.time;
				entry->de_xattr.st_atim.u.d.date = entry->de_xattr.st_mtim.u.d.date =
					entry->de_xattr.st_ctim.u.d.date;
			}
			return(E_OK);
		default:
			return(EINVFN);
	}
}

/* Ab hier folgen die Funktionen des Device-Treibers */

/*
 * Wenn eine Datei geschlossen werden soll, mu� man sicherstellen,
 * da� der refcnt auch wirklich Null ist, bevor man ihn freigibt
 */
LONG ramdisk_close(MX_FD *file)
{
	RAMDISK_FD	*fd;

	TRACE(("close - %L\r\n", file));
	fd = (RAMDISK_FD *)file;
	if (check_fd(fd) < 0)
	{
		TRACE(("close: check_fd fehlgeschlagen!\r\n"));
		return(check_fd(fd));
	}
	TRACE(("close: fd_refcnt vorher: %L", (LONG)fd->fd_refcnt));
	if (fd->fd_refcnt)
		fd->fd_refcnt--;
	TRACE(("close: fd_refcnt nachher: %L", (LONG)fd->fd_refcnt));
	if (!fd->fd_refcnt)
		fd->fd_file = NULL;
	return(E_OK);
}

/*
 * Beim Lesen mu� man immer darauf achten, da� man nicht �ber das
 * Ende der Datei hinausliest, ggf. mu� man weniger lesen. Man darf
 * dabei auch nicht vergessen, da� read nach erfolgreichem Lesen
 * die Anzahl der Bytes zur�ckliefert. Ist man also am Ende der Datei
 * angelangt, wird kein Fehler gemeldet, sondern lediglich 0 Bytes
 * gelesen.
 * Beim Ramdisk-XFS kann beim Lesen nat�rlich kein Fehler auftreten
 * (zumindest kann er nicht bemerkt werden), allerdings ist das Lesen
 * insgesamt etwas trickreich, da man sich ja an der verketteten
 * Liste von Filebl�cken entlanghangeln mu�. Erschwert wird das noch
 * durch die Tatsache, da� die Startposition des Lesevorgangs selten
 * direkt am Anfang eines Blocks liegt.
 */
LONG ramdisk_read(MX_FD *file, LONG count, void *_buffer)
{
	RAMDISK_FD	*fd;
	FILEBLOCK	*the_file;
	LONG		pos,
				read,
				readable;
	char *buffer = _buffer;

	TRACE(("read - %L, %L\r\n", file, count));
	fd = (RAMDISK_FD *)file;
	if (check_fd(fd) < 0)
		return(check_fd(fd));
/*
 * Wenn das File nicht zum Lesen oder Ausf�hren ge�ffnet war, einen
 * Fehler melden
 */
	if ((fd->fd_mode & (OM_RPERM | OM_EXEC)) == 0)
		return(EACCDN);
/* Ggf. die Anzahl der zu lesenden Bytes verringern */
	if ((fd->fd_fpos + count) > fd->fd_file->de_xattr.st_size)
		count = fd->fd_file->de_xattr.st_size - fd->fd_fpos;
/*
 * Den Fileblock und die Position in ihm ermitteln, an der das Lesen
 * beginnen mu�
 */
	pos = 0L;
	the_file = (FILEBLOCK *)fd->fd_file->de_faddr;
	while ((pos + DEFAULTFILE) < fd->fd_fpos)
	{
		the_file = the_file->next;
		pos += DEFAULTFILE;
	}
	pos = fd->fd_fpos - pos;
/*
 * In der folgenden Schleife werden so lange aufeinanderfolgende
 * Filebl�cke in den Zielpuffer kopiert, bis alle lesbaren Bytes
 * bearbeitet wurden. Da Anfang und Ende des Lesens mitten in einem
 * Block liegen k�nnen, wird mit der Variable readable angegeben,
 * wieviele Bytes im aktuellen Durchgang gelesen werden k�nnen.
 */
	readable = DEFAULTFILE - pos;
	read = 0L;
	while (count > 0L)
	{
		readable = (readable > count) ? count : readable;
		memcpy(buffer, &(the_file->data)[pos], readable);
		count -= readable;
		read += readable;
		buffer = &buffer[readable];
		readable = DEFAULTFILE;
		pos = 0L;
		the_file = the_file->next;
	}
/*
 * Am Ende die Position des innerhalb der Datei auf den aktuellen
 * Stand bringen und die letzte Zugriffszeit setzen
 */
	fd->fd_fpos += read;
	fd->fd_file->de_xattr.st_atim.u.d.time = Tgettime();
	fd->fd_file->de_xattr.st_atim.u.d.date = Tgetdate();
/* Zur�ckgeben, wieviele Bytes tats�chlich gelesen wurden */
	return(read);
}

/*
 * F�r das Schreiben gilt �hnliches wie f�r das Lesen, allerdings mu�
 * hier die Datei - soweit noch m�glich - erweitert werden, wenn �ber
 * ihr bisheriges Ende hinaus geschrieben werden soll. K�nnen nicht
 * alle Bytes geschrieben werden, mu� auch hier die Zahl entsprechend
 * verk�rzt werden.
 * F�r die Ramdisk m�ssen, wenn die Datei durch den Schreibzugriff
 * l�nger wird, entsprechend viele Filebl�cke neu angefordert und an
 * die bisherigen angeh�ngt werden. Sollte kein Speicher mehr frei
 * sein, k�nnen eben nicht alle Bytes geschrieben werden.
 */
LONG ramdisk_write(MX_FD *file, LONG count, void *_buffer)
{
	char *buffer = _buffer;
	RAMDISK_FD	*fd;
	FILEBLOCK	*the_file,
				*add,
				*j,
				*new;
	LONG		new_blocks,
				i,
				writeable,
				written,
				maxcount,
				pos;

	TRACE(("write - %L, %L\r\n", file, count));
	fd = (RAMDISK_FD *)file;
	if (check_fd(fd) < 0)
		return(check_fd(fd));
/* Fehler melden, wenn das File nicht zum Schreiben ge�ffnet ist */
	if ((fd->fd_mode & OM_WPERM) == 0)
		return(EACCDN);
/*
 * Den Fileblock, in dem das Schreiben beginnt und den, an den ggf.
 * neue Bl�cke angeh�ngt werden, ermitteln.
 */
	pos = 0L;
	j = the_file = (FILEBLOCK *)fd->fd_file->de_faddr;
	while (j != NULL)
	{
		if ((pos + DEFAULTFILE) < fd->fd_fpos)
		{
			the_file = j->next;
			pos += DEFAULTFILE;
		}
		add = j;
		j = j->next;
	}
/*
 * Berechnen, wieviele Bytes maximal in die bisher vorhandenen
 * Filebl�cke passen. Sollte das nicht reichen, um alle gew�nschten
 * Bytes zu schreiben, mu� die Datei erweitert werden.
 */
	maxcount = fd->fd_file->de_xattr.st_blocks * DEFAULTFILE;
	if ((fd->fd_fpos + count) > maxcount)
	{
/*
 * Zum Erweitern berechnen, wieviele neue Filebl�cke ben�tigt werden
 */
		new_blocks = (fd->fd_fpos + count + DEFAULTFILE - 1) /
			DEFAULTFILE;
		new_blocks -= fd->fd_file->de_xattr.st_blocks;
/*
 * Entsprechend viele Bl�cke der Reihe nach anfordern, an das
 * Fileende anh�ngen und die Zahl der schreibbaren Bytes entsprechend
 * erh�hen. Sollte f�r einen Block kein Speicher mehr verf�gbar sein,
 * mu� die Schleife vorzeitig abgebrochen werden.
 */
		for (i = 0; i < new_blocks; i++)
		{
			if ((new = Kmalloc(sizeof(FILEBLOCK))) == NULL)
				break;
			fd->fd_file->de_xattr.st_blocks++;
			maxcount += DEFAULTFILE;
			add->next = new;
			new->next = NULL;
			add = new;
		}
/*
 * Jetzt bestimmen, wieviele Bytes tats�chlich geschrieben werden
 * k�nnen
 */
		if ((fd->fd_fpos + count) > maxcount)
			count = maxcount - fd->fd_fpos;
	}
/*
 * Die Vorgehensweise zum Schreiben entspricht exakt der zum Lesen,
 * nur da� Quelle und Ziel vertauscht sind
 */
	pos = fd->fd_fpos - pos;
	writeable = DEFAULTFILE - pos;
	written = 0L;
	while (count > 0L)
	{
		writeable = (writeable > count) ? count : writeable;
		memcpy(&(the_file->data)[pos], buffer, writeable);
		count -= writeable;
		written += writeable;
		buffer = &buffer[writeable];
		writeable = DEFAULTFILE;
		pos = 0L;
		the_file = the_file->next;
	}
/*
 * Auch hier nach dem Schreiben die Position innerhalb der Datei auf
 * den aktuellen Stand bringen. Da sich die Datei durch das Schreiben
 * vergr��ert haben kann, mu� ggf. die Dateigr��e im Directoryeintrag
 * angepa�t werden. Beim Lesen des Eintrags erh�lt man also immer die
 * gerade aktuelle L�nge, selbst wenn das File noch beschrieben wird.
 */
	fd->fd_fpos += written;
	if (fd->fd_fpos > fd->fd_file->de_xattr.st_size)
		fd->fd_file->de_xattr.st_size = fd->fd_fpos;
/*
 * Die Modifikationszeit der Datei setzen und anzeigen, da� die Datei
 * ver�ndert wurde. Am Schlu� dann die Zahl der geschriebenen Bytes
 * zur�ckliefern.
 */
	fd->fd_file->de_xattr.st_mtim.u.d.time = Tgettime();
	fd->fd_file->de_xattr.st_mtim.u.d.date = Tgetdate();
	fd->fd_file->de_xattr.st_attr |= FA_CHANGED;
	return(written);
}

/*
 * Mit stat soll festgestellt werden, ob Bytes gelesen bzw.
 * geschrieben werden k�nnen. Da die Ramdisk ohnehin immer bereit
 * ist, wird hier auch keine Interruptroutine ben�tigt. Damit habe
 * ich mich auch noch nicht auseinandergesetzt, zumal das Problem
 * in der Regel auch nur f�r "echte" Devices akut ist.
 */
LONG ramdisk_stat(MX_FD *file, MAGX_UNSEL *unselect, WORD rwflag,
	LONG apcode)
{
	RAMDISK_FD	*fd;
	LONG		retcode;

	TRACE(("stat - %L, %L, %L, %L\r\n", file, unselect,
		(LONG)rwflag, apcode));
	fd = (RAMDISK_FD *)file;
/*
 * Man m�ge mir die Verwendung von goto verzeihen, aber hier ist es
 * wirklich �u�erst praktisch
 */
	if (check_fd(fd) < 0)
	{
		retcode = check_fd(fd);
		goto rs_exit;
	}
/*
 * Wenn Lesebereitschaft bei einem File getestet werden soll, da�
 * nicht zum Lesen ge�ffnet ist, mu� ein Fehler gemeldet werden
 */
	if (!rwflag && ((fd->fd_mode & (OM_RPERM | OM_EXEC)) == 0))
	{
		retcode = EACCDN;
		goto rs_exit;
	}
/* Gleiches gilt nat�rlich auch f�r den umgekehrten Fall */
	if (rwflag && ((fd->fd_mode & OM_WPERM) == 0))
	{
		retcode = EACCDN;
		goto rs_exit;
	}
/* Ansonsten kann getrost "Bereit" gemeldet werden */
	retcode = 1L;
/*
 * Bei der Ergebnisr�ckgabe mu�, wenn unselect kein Nullpointer war,
 * der Returnwert auch in unsel.status abgelegt werden (daher auch
 * das goto, um unn�tige Tipparbeit und if-Verrenkungen zu vermeiden)
 */
rs_exit:
	if (unselect != NULL)
		unselect->unsel.status = retcode;
	return(retcode);
}

/*
 * Wenn der Schreib-/Lesezeiger einer Datei verschoben werden soll,
 * mu� man nat�rlich auf Bereichs�berschreitungen achten und ggf.
 * ERANGE melden. Ansonsten ist seek, zumindest beim Ramdisk-XFS,
 * kein Problem.
 */
LONG ramdisk_seek(MX_FD *file, LONG where, WORD mode)
{
	RAMDISK_FD	*fd;
	LONG		new_pos;

	TRACE(("seek - %L, %L, %L\r\n", file, where, (LONG)mode));
	fd = (RAMDISK_FD *)file;
	if (check_fd(fd) < 0)
		return(check_fd(fd));
/* Je nach Modus die Bezugsposition f�r das seek ermitteln */
	switch (mode)
	{
		case 0:
			new_pos = 0L;
			break;
		case 1:
			new_pos = fd->fd_fpos;
			break;
		case 2:
			new_pos = fd->fd_file->de_xattr.st_size;
			break;
		default:
/*
 * Bei einem ung�ltigen Seek-Modus gebe ich einfach die aktuelle
 * Position zur�ck. Ob das so OK ist, wei� ich nicht, aber wenn ein
 * Programm Fseek falsch aufruft, mu� es auch mit falschen
 * Ergebnissen rechnen...
 */
			return(fd->fd_fpos);
	}
/*
 * Den Offset addieren (er gibt immer an, wieviele Bytes �bersprungen
 * werden sollen, also mu� er f�r Modus 2 einen Wert <= 0 haben).
 * W�rden dadurch die Grenzen �berschritten, den Zeiger nicht
 * ver�ndern und einen Fehler melden.
 */
	new_pos += where;
	if ((new_pos < 0L) || (new_pos > fd->fd_file->de_xattr.st_size))
		return(ERANGE);
/*
 * Ging alles glatt, den Zeiger auf die neue Position setzen und
 * diese zur�ckliefern
 */
	fd->fd_fpos = new_pos;
	return(new_pos);
}

/*
 * Soll das Datum einer ge�ffneten Datei ver�ndert werden, mu� man
 * vorher unter Umst�nden kl�ren, welche der drei Zeiten, die eine
 * Datei haben kann (Erstellung, letzter Zugriff, letzte �nderung),
 * man �ndern bzw. auslesen will. F�r das Ramdisk-XFS funktioniert
 * datime beim �ndern �hnlich wie das FUTIME-Kommando von
 * Dcntl/Fcntl, d.h. ctime/cdate werden auf die aktuelle Zeit/das
 * aktuelle Datum gesetzt, atime/adate und mtime/mdate erhalten
 * die Werte, die datime �bergeben wurden. Beim Auslesen wird immer
 * das Datum der letzten �nderung genommen, wie es auch in der DTA
 * von snext geliefert wird.
 */
LONG ramdisk_datime(MX_FD *file, WORD *d, WORD setflag)
{
	RAMDISK_FD	*fd;

	TRACE(("datime - %L, %L\r\n", file, (LONG)setflag));
	fd = (RAMDISK_FD *)file;
	if (check_fd(fd) < 0)
		return(check_fd(fd));
	switch(setflag)
	{
		case 0:
/* Zeit und Datum der letzten Modifikation auslesen */
			d[0] = fd->fd_file->de_xattr.st_mtim.u.d.time;
			d[1] = fd->fd_file->de_xattr.st_mtim.u.d.date;
			break;
		case 1:
/* Zum �ndern m�ssen Schreibrechte f�r die Datei vorhanden sein */
			if (!waccess(fd->fd_file))
				return(EACCDN);
			fd->fd_file->de_xattr.st_ctim.u.d.time = Tgettime();
			fd->fd_file->de_xattr.st_ctim.u.d.date = Tgetdate();
			fd->fd_file->de_xattr.st_atim.u.d.time =
				fd->fd_file->de_xattr.st_mtim.u.d.time = d[0];
			fd->fd_file->de_xattr.st_atim.u.d.date =
				fd->fd_file->de_xattr.st_mtim.u.d.date = d[1];
			break;
		default:
/* Ung�ltige Werte f�r setflag werden mit Fehler quittiert */
			return(EACCDN);
	}
	return(E_OK);
}

/*
 * ioctl ist der Bruder von dcntl und ist f�r offene Dateien
 * zust�ndig. Hier gibt es eine Reihe von Kommandos, die man
 * unterst�tzen kann (siehe auch die MagiC- und die MiNT-Doku).
 * Das Ramdisk-XFS unterst�tzt FSTAT (XATTR-Struktur zur Datei
 * liefern), FIONREAD (wieviele Bytes k�nnen sicher gelesen werden),
 * FIONWRITE (wieviele Bytes k�nnen sicher geschrieben werden) und
 * FUTIME (Zugriffs- und Modifikationszeit �ndern).
 */
LONG ramdisk_ioctl(MX_FD *file, WORD cmd, void *buf)
{
	RAMDISK_FD	*fd;
	WORD		*timebuf;
	LONG		*avail;
	XATTR		*xattr;

	TRACE(("ioctl - %L, %L, %L\r\n", file, (LONG)cmd,  buf));
	fd = (RAMDISK_FD *)file;
	if (check_fd(fd) < 0)
		return(check_fd(fd));
	avail = (LONG *)buf;
	switch (cmd)
	{
/* Zu FUTIME siehe dcntl */
		case FUTIME:
			if (!waccess(fd->fd_file))
				return(EACCDN);
			timebuf = (WORD *)buf;
			fd->fd_file->de_xattr.st_ctim.u.d.time = Tgettime();
			fd->fd_file->de_xattr.st_ctim.u.d.date = Tgetdate();
			if (timebuf != NULL)
			{
				fd->fd_file->de_xattr.st_atim.u.d.time = timebuf[0];
				fd->fd_file->de_xattr.st_atim.u.d.date = timebuf[1];
				fd->fd_file->de_xattr.st_mtim.u.d.time = timebuf[2];
				fd->fd_file->de_xattr.st_mtim.u.d.date = timebuf[3];
			}
			else
			{
				fd->fd_file->de_xattr.st_atim.u.d.time =
					fd->fd_file->de_xattr.st_mtim.u.d.time =
					fd->fd_file->de_xattr.st_ctim.u.d.time;
				fd->fd_file->de_xattr.st_atim.u.d.date =
					fd->fd_file->de_xattr.st_mtim.u.d.date =
					fd->fd_file->de_xattr.st_ctim.u.d.date;
			}
			return(E_OK);
		case FIONREAD:
/*
 * Es kann nat�rlich nur f�r Dateien, die zum Lesen ge�ffnet sind,
 * die Anzahl der lesbaren Bytes ermittelt werden
 */
			if ((fd->fd_mode & (OM_RPERM | OM_EXEC)) == 0)
				return(EACCDN);
/*
 * Bei der Ramdisk k�nnen immer soviele Bytes gelesen werden, wie
 * noch zwischen Lesezeigerposition und Dateiende vorhanden sind
 */
			*avail = fd->fd_file->de_xattr.st_size - fd->fd_fpos;
			return(E_OK);
		case FIONWRITE:
/*
 * F�r FIONWRITE gilt nat�rlich analog zu FIONREAD, da� nur f�r
 * Dateien, die zum Schreiben offen sind, die Anzahl der schreibbaren
 * Bytes ermittelt werden kann
 */
			if ((fd->fd_mode & OM_WPERM) == 0)
				return(EACCDN);
/*
 * Es k�nnen auf jeden Fall soviele Bytes geschrieben werden, wie
 * zwischen Ende des letzten Fileblocks und der Position des
 * Schreibzeigers noch vorhanden sind
 */
			*avail = fd->fd_file->de_xattr.st_blocks * DEFAULTFILE -
				fd->fd_fpos;
			if (*avail < 0L)
				*avail = 0L;
			return(E_OK);
		case FSTAT:
/*
 * F�r FSTAT mu� einfach die XATTR-Struktur der Datei in den
 * Zielbereich kopiert werden. F�r andere Filesysteme ist das u.U.
 * wesentlich kompliziert, weil die XATTR-Struktur erst "gebastelt"
 * werden mu�.
 */
			xattr = (XATTR *)buf;
			*xattr = fd->fd_file->de_xattr;
			return(E_OK);
		default:
/* Nicht unterst�tzte Kommandos m�ssen mit EINVFN abgelehnt werden */
			return(EINVFN);
	}
}

/*
 * Ein Byte aus einer Datei auslesen. Bei Filesystemen f�hrt man
 * diese Funktion am zweckm��igsten auf read mit L�nge 1 zur�ck.
 * Erh�lt man dabei einen Wert ungleich 1, ist entweder ein Fehler
 * aufgetreten oder das Dateiende erreicht. Im Ramdisk-XFS wird in
 * beiden F�llen "Dateiende" signalisiert, weil der n�chste Aufruf
 * sicherlich ebensowenig erfolgreich sein wird. mode wird nicht
 * beachtet!
 */
LONG ramdisk_getc(MX_FD *file, WORD mode)
{
	RAMDISK_FD	*fd;
	UBYTE		dummy;

	TRACE(("getchar - %L, %L\r\n", file, (LONG)mode));
	fd = (RAMDISK_FD *)file;
	if (check_fd(fd) < 0)
		return(check_fd(fd));
	if (ramdisk_read(file, 1L, (char *)&dummy) != 1L)
		return(0xff1aL);
/*
 * Konnte ein Byte gelesen werden, dieses auf ULONG erweitern und
 * zur�ckliefern
 */
	return((ULONG)dummy);
}

/*
 * getline ist etwas vertrackter, weil man prinzipiell auch auf
 * Zeilenende reagieren mu�. Man kann es sich nat�rlich auch einfach
 * machen, und getline direkt auf read zur�ckf�hren (size Bytes
 * lesen), dabei werden aber unter Umst�nden mehrere Zeilen auf
 * einmal gelesen (so macht es z.B. auch MiNT). Auf jeden Fall mu�
 * man zum Ermitteln der Zahl der gelesenen Zeichen, die ja als
 * Returnwert geliefert werden sollen, nach dem ersten CR bzw. LF
 * suchen und darf dieses dann _nicht_ dazurechnen.
 * Das Ramdisk-XFS versucht, via getc bis zum Dateiende oder einem
 * Zeilenende zu lesen, maximal nat�rlich size Bytes. Auch hier wird
 * mode nicht beachtet, was aber sowieso eher f�r Terminal-Devices
 * wichtig ist.
 */
LONG ramdisk_getline(MX_FD *file, char *buf, WORD mode, LONG size)
{
	RAMDISK_FD	*fd;
	LONG		dummy,
				count;

	TRACE(("getline - %L, %L, %L\r\n", file, size, (LONG)mode));
	fd = (RAMDISK_FD *)file;
	if (check_fd(fd) < 0)
		return(0L);
/*
 * Die Schleife hat nur interne Abbruchsbedingungen, count z�hlt
 * dabei, wieviele Zeichen bisher eingelesen wurden
 */
	for (count = 0L;; count++)
	{
/* Abbrechen, wenn schon die Maximalzahl an Zeichen gelesen ist */
		if (count == size)
			return(count);
/* Das n�chste Zeichen via getline einlesen */
		dummy = ramdisk_getc(file, 0);
		TRACE(("getline: count = %L, gelesenes Byte: %L\r\n",
			count, dummy));
/*
 * Ist es das Zeichen f�r Dateiende, den Puffer mit einem Nullbyte
 * abschlie�en und die Zahl der gelesen Zeichen liefern (das mit dem
 * Nullbyte ist nicht unbedingt n�tig, sieht allerdings besser aus)
 */
		if (dummy == 0xff1aL)
		{
			buf[count] = 0;
			return(count);
		}
/*
 * Wurde ein CR oder ein LF gelesen, ist der Lesevorgang ebenfalls
 * beendet, das jeweilige Zeichen darf aber nicht mitgez�hlt werden.
 * Im Falle von CR mu� noch das n�chste Zeichen �berlesen werden, da
 * Zeilenenden entweder CRLF oder LF sind und das LF im ersten Fall
 * eben �bersprungen werden mu� (sonst w�rde der n�chste Aufruf von
 * getline eine Leerzeile liefern).
 */
		if ((dummy == 0xdL) || (dummy == 0xaL))
		{
			if (dummy == 0xdL)
				ramdisk_getc(file, 0);
			buf[count] = 0;
			return(count);
		}
/*
 * Bei allen anderen Zeichen dieses jetzt im Puffer plazieren und den
 * n�chsten Schleifendurchlauf beginnen
 */
		buf[count] = dummy;
	}
}

/*
 * Auch das Ausgeben f�hrt man am geschicktesten auf write zur�ck,
 * zumal als R�ckgabewert ohnehin die Zahl der geschriebenen Zeichen
 * geliefert werden mu� (bei Devicetreiben von Filesystemen entweder
 * 0 oder 1).
 * Das Ramdisk-XFS beachtet auch hier mode nicht, weil das eigentlich
 * nur f�r Terminal-Devicetreiber sinnvoll ist.
 */
LONG ramdisk_putc(MX_FD *file, WORD mode, LONG value)
{
	RAMDISK_FD	*fd;
	char		dummy;

	TRACE(("putc - %L, %L, %L\r\n", file, (LONG)mode, value));
	fd = (RAMDISK_FD *)file;
	if (check_fd(fd) < 0)
		return(check_fd(fd));
	dummy = (char)value;
	return(ramdisk_write(file, 1L, &dummy));
}
#pragma warn .par

THE_MGX_XFS ramdisk_xfs = {
	"Ramdisk",
	ramdisk_sync,
	ramdisk_pterm,
	ramdisk_garbcoll,
	ramdisk_freeDD,
	ramdisk_drv_open,
	ramdisk_drv_close,
	ramdisk_path2DD,
	ramdisk_sfirst,
	ramdisk_snext,
	ramdisk_fopen,
	ramdisk_fdelete,
	ramdisk_link,
	ramdisk_xattr,
	ramdisk_attrib,
	ramdisk_chown,
	ramdisk_chmod,
	ramdisk_dcreate,
	ramdisk_ddelete,
	ramdisk_DD2name,
	ramdisk_dopendir,
	ramdisk_dreaddir,
	ramdisk_drewinddir,
	ramdisk_dclosedir,
	ramdisk_dpathconf,
	ramdisk_dfree,
	ramdisk_wlabel,
	ramdisk_rlabel,
	ramdisk_symlink,
	ramdisk_readlink,
	ramdisk_dcntl
};

THE_MGX_DEV ramdisk_dev = {
	ramdisk_close,
	ramdisk_read,
	ramdisk_write,
	ramdisk_stat,
	ramdisk_seek,
	ramdisk_datime,
	ramdisk_ioctl,
	ramdisk_getc,
	ramdisk_getline,
	ramdisk_putc
};
