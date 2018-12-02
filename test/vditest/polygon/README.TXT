Demonstration Bildschirmtreiberfehler bei gef�llten Polygonen
=============================================================

Das Programm POLYGON malt in verschiedenen Modi und F�llmustern Dreiecke
auf den Bildschirm. Es kann einfach ohne Parameter gestartet werden.

Die Verwendung eines benutzerdefinierten F�llmusters f�hrt offenbar in
einigen Konfigurationen zum Absturz oder zu unsch�nen Ergebnissen. Ich habe, da
ich das Programm in verschiedenen Aufl�sungen getestet habe, nur den Fall des
einfarbigen F�llmusters verwendet, aber die Variante mit 32 Bit f�hrt
im True-Colour-Modus auch zum Absturz.

Hier die Ergebnisse im einzelnen:

MagicMacX mit MagiC-VDI, 16M Farben			Emulator-Absturz
MagicMacX mit MagiC-VDI, 32k Farben			Emulator-Absturz
MagicMacX mit MagiC-VDI, 256 Farben			OK
MagicMac mit NVDI, 16M Farben				OK
MagicMac mit NVDI, 32k Farben				totaler Systemabsturz von OS 9
MagicMac mit NVDI, 256 Farben				vermatschte Ausgabe


Offenbar haben also verschiedene Bildschirmtreiber Probleme mit benutzerdefiniert
gef�llten Polygonen. K�nnt Ihr das zumindest f�r die MagiC-VDI-Treiber beheben?
