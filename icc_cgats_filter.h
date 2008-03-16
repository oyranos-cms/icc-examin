/*
 * ICC Examin ist eine ICC Profil Betrachter
 * 
 * Copyright (C) 2005-2008  Kai-Uwe Behrmann 
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
#include "icc_list.h"

#include <string>

#define STD_CGATS_FIELDS 44

/**
 *  Arbeitsweise
 *
 *  Die Klasse CgatsFilter folgt dem Kontextmodell. Es k&ouml;nnen Eigenschaften
 *  eingestellt werden, Daten geladen und die Auswertung erfolgt mit Hilfe der
 *  zuvor eingestellten Optionen.
 */

class CgatsFilter
{
    // statische Hilfsobjekte
    static const char *cgats_alnum_;         //!< @brief non-integral type
    static const char *cgats_alpha_;
    static const char *cgats_numerisch_;
    static const char *cgats_ziffer_;
    static const char *leer_zeichen_;
      //! @brief Standard CGATS Schl&uuml;sselw&ouml;rter
    static const char ss_woerter_[STD_CGATS_FIELDS][16];
  public:
    CgatsFilter ()
    { DBG_PROG_START
        //! @brief Initialisierung
        typ_ = LCMS;
          //! @brief das Ersetzungswort f&uuml;r spektrale Feldbezeichner
        spektral = "SPECTRAL_";
        anfuehrungsstriche_setzen = false;
         //! @brief eine erste Messung wird ben&ouml;tigt
        messungen.resize(1);
        DBG_PROG_ENDE
    }
    ~CgatsFilter () {; }

  private:
    //! @brief Kopieren
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

    //! @brief Laden der CGATS ascii Daten
    void lade (char* text, size_t size) { clear();
                                          data_orig_.assign( text,0,size ); }
    void lade (std::string &text)       { clear(); data_orig_ = text; }
    //! @brief Zur&uuml;cksetzen der Datenstrukturen - keine neues Verhalten
    void clear()              { data_.resize(0); data_orig_.resize(0);
                                messungen.clear(); messungen.resize(1);
                                log.clear();
                                zeilen_.clear();
                                s_woerter_.resize(STD_CGATS_FIELDS);
                                for(unsigned i = 0; i < STD_CGATS_FIELDS; ++i)
                                    s_woerter_[i] = ss_woerter_[i];
                              }
    //! @brief Ausgeben
    std::string lcms_gefiltert() { typ_ = LCMS; cgats_korrigieren_();
                                   return data_; }
                //! @brief Modus
    std::string max_korrigieren(){ typ_ = MAX_KORRIGIEREN; cgats_korrigieren_();
                                   return data_; }

    // Optionen
                //! @brief frei w&auml;hlbarer Kopf ( standardgem&auml;&szlig; 7 Zeichen lang )
    std::string kopf;
                //! @brief frei w&auml;hlbarer Kommentar ( wird nach Kopfzeile eingef&uuml;gt )
    std::string kommentar;
                //! @brief frei w&auml;hlbarer Bezeichner f&uuml;r Spektraldaten (SPECTRAL_)
    std::string spektral;
                //! @brief f&uuml;r ausgegebene Worte im DATA Block
    bool        anfuehrungsstriche_setzen;

    //! @brief Messdaten
    struct Messung {
      ICClist<std::string> kommentare; //!< @brief KEYWORD ...
      ICClist<ICClist<std::string> > felder;     //!< @brief DATA_FIELD
      ICClist<ICClist<std::string> > block;      //!< @brief DATA
      ICClist<int> line;               //!< @brief CGATS DATA line
      int feld_spalten;                    //!< @brief NUMBER_OF_FIELDS
      int block_zeilen;                    //!< @brief NUMBER_OF_SETS
      Messung ()
      {
        feld_spalten = 0;
        block_zeilen = 0;
      }
    };
    ICClist<Messung> messungen;        //!< @brief teilweise strukturierte Messdaten
  private:
    void neuerAbschnitt_ ();
  public:
    /** @brief Die Liste von Mitteilungen und Auff&auml;lligkeiten
     *
     * Anm.: bei zusammengefassten Vorg&auml;ngen ist eventuell nur "meldung" g&uuml;ltig
     */
    struct Log {
      ICClist<std::string> eingabe; //!< @brief die bearbeiteten Zeilen (>=0)
      ICClist<std::string> ausgabe; //!< @brief die resultierenden Zeilen (>=0)
      std::string meldung;              //!< @brief eine Mitteilung f&uuml;r den Vorgang
      int original_zeile;               //!< @brief Zeilennummer der Eingabe
    };
    ICClist<Log> log;

  private:
    /** @brief --- Hauptfunktion ---

       o Konvertierung in Standard unix Dateiformat mit LF
       o Suchen und Ersetzen bekannnter Abweichungen (in data_)
       o zeilenweises lesen und editieren (in zeilen_)
       o verdecken der Kommentare (mit lokalem string gtext)
       o kontrollieren der Bl&ouml;ckanf&auml;nge und -enden
       o die Dateisignatur reparieren (7 / 14 byte lang)
       o zwischen den Bl&ouml;cken: Schl&uuml;sselworte erkennen und bearbeiten
       o die Zeilen wieder zusamenf&uuml;gen und als einen std::string zur&uuml;ckgeben
    */
    std::string cgats_korrigieren_               ();
    
    // - Hilfsfunktionen -

      /** @brief Ausz&auml;hlen der Formate(Farbkan&auml;le) im DATA_FORMAT Block
       *
       *  zeile     : zu bearbeitende kommentarfreie Zeile
       *  zeile_x   : Nummer der gew&auml;hlten Zeile
       */
    int sucheInDATA_FORMAT_( std::string &zeile, int &zeile_x );

      /** @brief klassifiziert CGATS Schl&uuml;sselworte; sinnvoll ausserhalb der Bl&ouml;cke
       *
       *  zeile     : zu bearbeitende kommentarfreie Zeile
       */
    int sucheSchluesselwort_( std::string zeile );

      /** @brief eine Zeile ausserhalb der beiden DATA und FORMAT Bl&ouml;cke nach
       * Klassifizierungsschl&uuml;ssel bearbeiten
       *
       *  zeilen    : Referenz auf alle Text Zeilen
       *  zeile_x   : Nummer der gew&auml;hlten Zeile
       *  editieren : Bearbeitungscode aus sucheSchluesselwort_()
       *  cmy       : Schalter f&uuml;r CB CMY Feldbezeichner
       */
    int editZeile_( ICClist<std::string> &zeilen,
                    int zeile_x, int editieren, bool cmy );

    // allgemeine Textbearbeitung
      /** @brief Textvektor Bearbeitung f&uuml;rs zeilenweise Editieren
       *
       *  zeilen    : Referenz auf alle Text Zeilen
       *  zeile_x   : Nummer der gew&auml;hlten Zeile
       */
    void suchenLoeschen_      ( ICClist<std::string> &zeilen,
                                std::string               text );

      /** @brief doppelte Zeilen l&ouml;schen
       *
       *  zeilen    : Referenz auf alle Text Zeilen
       */
    int  zeilenOhneDuplikate_ ( ICClist<std::string> &zeilen );

      /** @brief Buchstabenworte von Zahlen unterscheiden
       *
       *  zeilen    : Referenz auf alle Text Zeilen
       */
    ICClist<std::string> unterscheideZiffernWorte_ ( std::string &zeile );

      /** @brief Zeile von pos bis Ende in Anf&uuml;hrungszeichen setzen
       *
       *  zeilen    : Referenz auf alle Text Zeilen
       */
    void setzeWortInAnfuehrungszeichen_ ( std::string &zeile,
                                          std::string::size_type pos );

      /** @brief bequem einen Eintrag zu log hinzuf&uuml;gen
       *
       *  meldung   : Beschreibung
       *  zeile_x   : Zeile des Auftretens des Ereignisses
       *  zeile1    : Urspr&uuml;nglisch Zeilen
       *  zeile2    : ge&auml;nderte Zeilen
       */
    unsigned int logEintrag_ (std::string meldung, int zeile_x,
                              std::string zeile1,  std::string zeile2 );

      /** @brief angepasste Variante von suchenErsetzen()
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

    //! @brief Schl&uuml;sselwort -> passende Korrekturen
    enum {
      BELASSEN,
      KEYWORD,
      ANFUEHRUNGSSTRICHE,
      DATA_FORMAT_ZEILE,
      AUSKOMMENTIEREN,
      LINEARISIERUNG,
      CMY_DATEN,
      CMYK_DATEN,
      SAMPLE_KORRIGIEREN
    };

    // ben&ouml;tigte dynamische Hilfsobjekte
    std::string              data_;      //!< @brief der korrigierte CGATS Text
    std::string              data_orig_; //!< @brief eine Kopie vom Original
    ICClist<std::string> s_woerter_; //!< @brief Schl&uuml;sselw&ouml;rter
    ICClist<std::string> felder_;    //!< @brief die Feldnamen
    ICClist<std::string> zeilen_;    //!< @brief Arbeitsspeicher
    enum Typ_ {
      LCMS,
      MAX_KORRIGIEREN
    };
    enum Typ_                typ_;       //!< @brief Art des Filterns
    int zeile_letztes_NUMBER_OF_FIELDS;
};



//! @brief fertig zum Anwenden
std::string  cgats_korrigieren( char* data, size_t size );
//! @brief fertig zum Anwenden
std::string  cgats_max_korrigieren( char* data, size_t size );

#endif // ICC_CGATS_FILTER_H

