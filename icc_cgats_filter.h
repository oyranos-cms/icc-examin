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

#include <vector>
#include <string>

#define STD_CGATS_FIELDS 44

/*
 *  Arbeitsweise
 *
 *  Die Klasse CgatsFilter folgt dem Kontextmodell. Es können Eigenschaften
 *  eingestellt werden, Daten geladen und die Auswertung erfolgt mit Hilfe der
 *  zuvor eingestellten Optionen.
 */

class CgatsFilter
{
    // statische Hilfsobjekte
    static const char *cgats_alnum_;         // non-integral type
    static const char *cgats_alpha_;
    static const char *cgats_numerisch_;
    static const char *cgats_ziffer_;
    static const char *leer_zeichen_;
      // Standard CGATS Schlüsselwörter
    static const char ss_woerter_[STD_CGATS_FIELDS][16];
  public:
    CgatsFilter ()
    { DBG_PROG_START
        // Initialisierung
        typ_ = LCMS;
          // die Dateisignatur
        kopf = "ICCEXAM";
          // das Ersetzungswort für spektrale Feldbezeichner
        spektral = "SPECTRAL_";
        anfuehrungsstriche_setzen = false;
         // eine erste Messung wird benötigt
        messungen.resize(1);
        DBG_PROG_ENDE
    }
    ~CgatsFilter () {; }

    // Kopieren
  private:
    CgatsFilter& copy (const CgatsFilter & o)
                              {
                                typ_ = o.typ_;
                                kopf = o.kopf;
                                kommentar = o.kommentar;
                                spektral = o.spektral;
                                messungen = o.messungen;
                                anfuehrungsstriche_setzen = o.anfuehrungsstriche_setzen;
                                log = o.log;
                                return *this;
                              }
  public:
    CgatsFilter             (const CgatsFilter& o) { copy(o); }
    CgatsFilter& operator = (const CgatsFilter& o) { return copy (o); }

    // Laden der CGATS ascii Daten
    void lade (char* text, size_t size) { clear();
                                          data_orig_.assign( text,0,size ); }
    void lade (std::string &text)       { clear(); data_orig_ = text; }
    // Zurücksetzen der Datenstrukturen - keine neues Verhalten
    void clear()              { data_.resize(0); data_orig_.resize(0);
                                messungen.resize(0); messungen.resize(1);
                                log.clear();
                                zeilen_.clear();
                                s_woerter_.resize(STD_CGATS_FIELDS);
                                for(unsigned i = 0; i < STD_CGATS_FIELDS; ++i)
                                    s_woerter_[i] = ss_woerter_[i];
                              }
    // Ausgeben
    std::string lcms_gefiltert() { typ_ = LCMS; cgats_korrigieren_();
                                   return data_; }
                // basICColor Modus
    std::string max_korrigieren(){ typ_ = MAX_KORRIGIEREN; cgats_korrigieren_();
                                   return data_; }

    // Optionen
                // frei wählbarer Kopf ( standardgemäß 7 Zeichen lang )
    std::string kopf;
                // frei wählbarer Kommentar ( wird nach Kopfzeile eingefügt )
    std::string kommentar;
                // frei wählbarer Bezeichner für Spektraldaten (SPECTRAL_)
    std::string spektral;
                // für ausgegebene Worte im DATA Block
    bool        anfuehrungsstriche_setzen;

    // Messdaten
    struct Messung {
      std::vector<std::string> kommentare; // KEYWORD ...
      std::vector<std::string> felder;     // DATA_FIELD
      std::vector<std::string> block;      // DATA
      int feld_spalten;                    // NUMBER_OF_FIELDS
      int block_zeilen;                    // NUMBER_OF_SETS
    };
    std::vector<Messung> messungen;        // teilweise strukturierte Messdaten
  private:
    void neuerAbschnitt_ ();
  public:
    // Die Liste von Mitteilungen und Auffälligkeiten
    // Anm.: bei zusammengefassten Vorgängen ist eventuell nur "meldung" gültig
    struct Log {
      std::vector<std::string> eingabe; // die bearbeiteten Zeilen (>=0)
      std::vector<std::string> ausgabe; // die resultierenden Zeilen (>=0)
      std::string meldung;              // eine Mitteilung für den Vorgang
      int original_zeile;               // Zeilennummer der Eingabe
    };
    std::vector<Log> log;

  private:
    /* --- Hauptfunktion ---
       o Konvertierung in Standard unix Dateiformat mit LF
       o Suchen und Ersetzen bekannnter Abweichungen (in data_)
       o zeilenweises lesen und editieren (in zeilen_)
       o verdecken der Kommentare (mit lokalem string gtext)
       o kontrollieren der Blöckanfänge und -enden
       o die Dateisignatur reparieren (7 / 14 byte lang)
       o zwischen den Blöcken: Schlüsselworte erkennen und bearbeiten
       o die Zeilen wieder zusamenfügen und als einen std::string zurückgeben
    */
    std::string cgats_korrigieren_               ();
    
    // - Hilfsfunktionen -

      /* Auszählen der Formate(Farbkanäle) im DATA_FORMAT Block
       *
       *  zeile     : zu bearbeitende kommentarfreie Zeile
       *  zeile_x   : Nummer der gewählten Zeile
       */
    int sucheInDATA_FORMAT_( std::string &zeile, int &zeile_x );

      /* klassifiziert CGATS Schlüsselworte; sinnvoll ausserhalb der Blöcke
       *
       *  zeile     : zu bearbeitende kommentarfreie Zeile
       */
    int sucheSchluesselwort_( std::string zeile );

      /* eine Zeile ausserhalb der beiden DATA und FORMAT Blöcke nach
       * Klassifizierungsschlüssel bearbeiten
       *
       *  zeilen    : Referenz auf alle Text Zeilen
       *  zeile_x   : Nummer der gewählten Zeile
       *  editieren : Bearbeitungscode aus sucheSchluesselwort_()
       *  cmy       : Schalter für CB CMY Feldbezeichner
       */
    int editZeile_( std::vector<std::string> &zeilen,
                    int zeile_x, int editieren, bool cmy );

    // allgemeine Textbearbeitung
      /* Textvektor Bearbeitung fürs zeilenweise Editieren
       *
       *  zeilen    : Referenz auf alle Text Zeilen
       *  zeile_x   : Nummer der gewählten Zeile
       */
    void suchenLoeschen_      ( std::vector<std::string> &zeilen,
                                std::string               text );

      /* doppelte Zeilen löschen
       *
       *  zeilen    : Referenz auf alle Text Zeilen
       */
    int  zeilenOhneDuplikate_ ( std::vector<std::string> &zeilen );

      /* Buchstabenworte von Zahlen unterscheiden
       *
       *  zeilen    : Referenz auf alle Text Zeilen
       */
    std::vector<std::string> unterscheideZiffernWorte_ ( std::string &zeile );

      /* Zeile von pos bis Ende in Anführungszeichen setzen
       *
       *  zeilen    : Referenz auf alle Text Zeilen
       */
    void setzeWortInAnfuehrungszeichen_ ( std::string &zeile,
                                          std::string::size_type pos );

      /* bequem einen Eintrag zu log hinzufügen
       *
       *  meldung   : Beschreibung
       *  zeile_x   : Zeile des Auftretens des Ereignisses
       *  zeile1    : Ursprünglisch Zeilen
       *  zeile2    : geänderte Zeilen
       */
    unsigned int logEintrag_ (std::string meldung, int zeile_x,
                              std::string zeile1,  std::string zeile2 );

      /* angepasste Variante von suchenErsetzen()
       *
       *  text      : Text Zeile
       *  suchen    : zu suchendes Wort
       *  ersetzen  : ersetzen durch
       *  pos       : Startposition in "text"
       */
    int  suchenUndErsetzen_     ( std::string           &text,
                                  const char*            suchen,
                                  const char*            ersetzen,
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
    std::vector<std::string> zeilen_;    // Arbeitsspeicher
    enum Typ_ {
      LCMS,
      MAX_KORRIGIEREN
    };
    enum Typ_                typ_;       // Art des Filterns
    int zeile_letztes_NUMBER_OF_FIELDS;
};



// fertig zum Anwenden
std::string  cgats_korrigieren( char* data, size_t size );
std::string  cgats_max_korrigieren( char* data, size_t size );

#endif // ICC_CGATS_FILTER_H

