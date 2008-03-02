/*
 * ICC Examin ist eine ICC Profil Betrachter
 * 
 * Copyright (C) 2005  Kai-Uwe Behrmann 
 *
 * Autor: Kai-Uwe Behrmann <ku.b@gmx.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 * 
 * -----------------------------------------------------------------------------
 *
 * cgats Fehlerkorrekturen
 * 
 */

#ifndef ICC_CGATS_FILTER_H
#define ICC_CGATS_FILTER_H

#include "icc_utils.h"
#include "icc_cgats_filter.h"

#include <vector>
#include <string>

#define STD_CGATS_FIELDS 44

class CgatsFilter
{
    const static double pi = M_PI;           // integral type

    // statische Hilfsobjekte
    const static char *cgats_alnum_;         // non-integral type
    const static char *cgats_alpha_;
    const static char *cgats_numerisch_;
    const static char *cgats_ziffer_;
    const static char *leer_zeichen_;
    const static char ss_woerter_[STD_CGATS_FIELDS][16];// Standard Schlüsselwö.
  public:
    CgatsFilter ()
    { DBG_PROG_START
        // Initialisierung konstanter Typen
        typ_ = LCMS;
        kopf = "ICCEXAM";
        spektral = "SPECTRAL_";
        anfuehrungsstriche_setzen = false;
        DBG_PROG_ENDE
    }
    ~CgatsFilter () {; }
    void lade (char* data, size_t size) { clear();
                                          data_orig_.assign( data,0,size ); }
    void lade (std::string &data)       { clear(); data_orig_ = data; }
    void clear()                        { data_.resize(0); data_orig_.resize(0);
           s_woerter_.resize(STD_CGATS_FIELDS);
           for(unsigned i = 0; i < STD_CGATS_FIELDS; ++i)
             s_woerter_[i] = ss_woerter_[i]; }
    // Ausgeben
    std::string lcms_gefiltert() { typ_ = LCMS; cgats_korrigieren();
                                   return data_; }
    // basICColor Modus
    std::string max_korrigieren(){ typ_ = MAX_KORRIGIEREN; cgats_korrigieren(); 
                                   return data_; }

    
    // frei wählbarer Kopf ( standardgemäß 7 Zeichen lang )
    std::string kopf;
    // frei wählbarer Kommentar ( wird nach Kopfzeile eingefügt )
    std::string kommentar;
    // frei wählbarer Bezeichner für Spektraldaten (standard ist SPECTRAL_)
    std::string spektral;
    bool        anfuehrungsstriche_setzen; // für ausgegebene Worte

    // Liste von Blöcken + Feldbezeichnern
    //   v- Feld/Block v- Zeilen   v- Inhalt
    std::vector<  std::vector<std::string> > felder;
    std::vector<  std::vector<std::string> > bloecke;

private:
    // --- Hauptfunktion ---
    // Konvertierung in Standard unix Dateiformat mit LF
    // Suchen und Ersetzen bekannnter Abweichungen (in einem std::string)
    // zeilenweises lesen und editieren (in einem vector aus strings)
    // verdecken der Kommentare
    // kontrollieren der Blöckanfänge und -enden
    // die Dateisignatur reparieren (7 / 14 byte lang)
    // zwischen den Blöcken die Keywords erkennen und entsprechend bearbeiten
    // die Zeilen wieder zusamenfügen und als einen std::string zurückgeben
    std::string cgats_korrigieren               ();

    // - Hilfsfunktionen -
    // Auszählen der Formate(Farbkanäle) im DATA_FORMAT Block
    int sucheInDATA_FORMAT_( std::string &zeile );
    // klassifiziert CGATS Keywords; sinnvoll ausserhalb der Blöcke
    int sucheSchluesselwort_( std::string zeile );
    // eine Zeile ausserhalb der beiden DATA und FORMAT Blöcke nach
    //  Klassifizierungsangabe bearbeiten
    int editZeile_( std::vector<std::string> &zeilen, int zeile_n, int editieren,
                bool cmy );
    // vector Bearbeitung fürs zeilenweise Editieren
    void suchenLoeschen_      ( std::vector<std::string> &zeilen,
                                std::string               text );
    // doppelte Zeilen löschen
    int  zeilenOhneDuplikate_ ( std::vector<std::string> &zeilen );
    // Buchstabenworte von Zahlen unterscheiden
    std::vector<std::string> unterscheideZiffernWorte_ ( std::string &zeile );
    // Zeile von pos bis Ende in Anführungszeichen setzen
    void setzeWortInAnfuehrungszeichen_ ( std::string &zeile,
                                std::string::size_type pos );

    // Schlüsselwort -> passende Korrekturen
    enum {
    BELASSEN,
    KEYWORD,
    ANFUEHRUNGSSTRICHE,
    DATA_FORMAT_ZEILE,
    AUSKOMMENTIEREN,
    LINEARISIERUNG,
    CMY_DATEN,
    CMYK_DATEN
    };

    // benötigte dynamische Hilfsobjekte
    std::string              data_;      // der korrigierte CGATS Text
    std::string              data_orig_; // eine Kopie vom Original
    std::vector<std::string> s_woerter_; // Schlüsselwörter
    int                      typ_;       // Art des Filterns
    enum {                   // enum passend zu typ_
      LCMS,
      MAX_KORRIGIEREN
    };
};


// fertig zum Anwenden
std::string  cgats_korrigieren( char* data, size_t size );
std::string  cgats_max_korrigieren( char* data, size_t size );

#endif // ICC_CGATS_FILTER_H

