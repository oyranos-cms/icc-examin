/*
 * ICC Examin ist eine ICC Profil Betrachter
 * 
 * Copyright (C) 2004-2005  Kai-Uwe Behrmann 
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


/* TODO
	o verschmelzen
	o einzelne Blöcke herausschreiben
    o Number_of_FIELDS immer korrigieren falls differiert
	o über return Wert zurückgeben struct zeiennummer + 2 geänderte Zeilen
    o Header Keywort parsen -> Liste von legalen + neuen Schlüsselworten 
    o NUMBER_OF_FIELDS: unbekannte FIELDS als KEYWORD deklarieren und mitzählen 
      für Exportmodus
    o im DATA_BLOCK nach Ziffer und Worten unterscheiden und Worte in
      Anführungszeichen, auf Flieskommazahl/Ganzzahl prüfen, abschaltbar
*/

/*
   ChangeLog

2005-01-25
	* und: insert umstellen <- 9:00
	* neu: KEYWORD in Schlüsselwortliste aufnehmen -> 12:00
	* neu: ss_woerter_ const static <- 20:00
	* neu: unterscheideZiffernWorte_ <- 21:30
	* neu: setzeWortInAnfuehrungszeichen_ <- 22:30
	* -> 23:30
2005-01-24
	* <- 10:30
	* Telefon Ulm 11:30 - 12:30
	* neu: Klasse <- 12:30
	* -> 14:00
	* und: Funktionalität wiederherstellen <- 19:00
	* neu: zeilenOhneDuplikate_
	* und: Umschreiben zu Liste von Schlüsselworten
	* und: vector/string Studium -> 22:00
2005-01-23
	* neu: CB Anpassungen <- 8:00
	* neu: Schlüsselworte suchen <- 9:30
	* fix: Spektraldaten NUMBER_OF_FIELDS korrekt gesetzt -> 10:30
	* und: Schlüsselwörter <- 11:30
	* neu: Schlüsselwörter korrigieren - editZeile AUSKOMMENTIEREN -> 13:00
	* neu: exakteres Editieren der Kopfzeile <- 17:00
	* neu: editZeile - in Anführungszeichen setzen -> 19:15
	* neu: editZeile - SAMPLE_ID -> BLOCK DATA_FORMAT -> 21:00
	* neu: editZeile - LINEARISIERUNG -> 21:15
	* Sortieren und Dokumentieren -> 22:00
	* Testen
	* fix: SAMPLE_ID mehrfach korrigiert -> 22:30
	* fix: Blockende nicht erfasst -> sucheWort
	* fix: CMY <-> CMYK <-> CMYKOGBK - cmy, cmy_daten, CMY_DATEN -> 24:00
2005-01-22
	* neu: NUMBER_OF_SETS zeilenweise korrigieren <- 7:00
	* neu: NUMBER_OF_FIELDS zeilenweise + Kommentar -> 10:00
	* neu: generelle Kommentarbehandlung <- 20:00
	* fix: sucheInDATA_FORMAT_()
	* fix: Signatur -> 22:30
2005-01-21
	* und: char Bearbeitung LF FF nach string umstellen <- 9:00
	* neu: zeilenorientierte Kommentarkorrektur -> 14:00
	* neu: sucheWort in icc_helfer.h/cpp <- 15:00
	* : -> 16:30
	* : <- 21:30
	* : -> 24:00
2005-01-11
	* start: 6:00
	* neu: Kopie des vorhandenen Textes und Test <- 6:30
	* Brief schreiben <- 7:00

*/

#include "icc_utils.h"
#include "icc_helfer.h"
#include "icc_cgats_filter.h"

#include <fstream>
#include <sstream>

// Initialisierungen

const char *CgatsFilter::cgats_alnum_ = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz_|/-+=()[]{}<>&?!:;,.0123456789";
const char *CgatsFilter::cgats_numerisch_ = "-+,.0123456789";
const char *CgatsFilter::cgats_alpha_ = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz_|/=()[]{}<>&?!:;";
const char *CgatsFilter::leer_zeichen_ = "\t\n\v\f\r";
const char CgatsFilter::ss_woerter_[STD_CGATS_FIELDS][16] =
  { "SAMPLE_ID",
    "SAMPLE_NAME",
    "CMYK_C",
    "CMYK_M",
    "CMYK_Y",
    "CMYK_K",
    "RGB_R",
    "RGB_G",
    "RGB_B",
    "XYZ_X",
    "XYZ_Y",
    "XYZ_Z",
    "XYY_X",
    "XYY_Y",
    "XYY_CAPY",
    "LAB_L",
    "LAB_A",
    "LAB_B",
    "D_RED",
    "D_GREEN",
    "D_BLUE",
    "D_VIS",
    "D_MAJOR_FILTER",
    "SPECTRAL_PCT",
    "SPECTRAL_DEC",
    "SPECTRAL_",
    "nm",  // nicht standard
    "SPECTRUM_", // nicht standard
    "R_", // nicht standard
    "LAB_C",
    "LAB_H",
    "LAB_DE",
    "LAB_DE_94",
    "LAB_DE_CMC",
    "LAB_DE_2000",
    "MEAN_DE",
    "STDEV_X",
    "STDEV_Y",
    "STDEV_Z",
    "STDEV_L",
    "STDEV_A",
    "STDEV_B",
    "STDEV_DE",
    "CHI_SQD_PAR" };

// Implementationen

void
CgatsFilter::setzeWortInAnfuehrungszeichen_( std::string &zeile,
                                        std::string::size_type pos )
{ DBG_PROG_START
  std::string::size_type ende = zeile.size();
              bool in_anfuehrung = false;
              int letze_anfuehrungsstriche = -1;
              // das erste Zeichen nach dem Schlüsselwort
              pos = zeile.find_first_not_of( cgats_alnum_ ,pos, ende );
              DBG_NUM_V( pos )
              for( ; pos < ende; ++pos )
              {
                if( zeile[pos] == '\"' )
                { DBG_NUM_S( pos <<" "<< zeile.substr( pos, ende-pos ) )
                  if( in_anfuehrung ) // hinaus
                  {
                    in_anfuehrung = false;
                    letze_anfuehrungsstriche = -1;
                  } else {            // hinein
                    in_anfuehrung = true;
                    letze_anfuehrungsstriche = pos;
                  }
                } else if( strchr( cgats_alnum_, zeile[pos] ) &&// ein Zeichen
                           !in_anfuehrung )                     // .. ausserhalb
                { DBG_NUM_V( zeile.substr( pos, ende-pos ) )
                  zeile.insert( pos, "\"" );
                  ++ende;
                  ++pos;
                  in_anfuehrung = true;
                  letze_anfuehrungsstriche = pos-1;
                  DBG_NUM_S( zeile.substr( pos, ende-pos ) )
                }
              }
              if( in_anfuehrung )
                if( ende == zeile.size() )
                  zeile.insert( ende, "\"" );
                else
                  zeile.insert( ende-1, "\"" );

  DBG_PROG_ENDE
}

void
CgatsFilter::unterscheideZiffernWorte_( std::string &zeile )
{ DBG_PROG_START
  std::string::size_type pos = 0, ende = 0;
  static char text[64];

  suchenErsetzen ( zeile, ",", ".", 0 );

  // Worte Suchen und von Zahlen scheiden
  for( unsigned int i = 0; i < zeile.size() ; ++i )
  { 
    pos = 0;
    if( (pos = zeile.find_first_of( cgats_alnum_ )) != std::string::npos )
    {
      if( (ende = zeile.find_first_not_of( cgats_alnum_ )) == std::string::npos)
        ende = zeile.size();
      sprintf( text, "%f", atof( zeile.substr( pos, ende-pos ).c_str() ) );
      
      if( strlen( text ) ) // ist Fließkommazahl
      {
        i += pos;
      } else {
        sprintf( text, "%d", atoi( zeile.substr( pos, ende-pos ).c_str() ) );
        if( strlen( text ) ) // ist Ganzzahl
        {
          i += pos;
        } else { // Text
          setzeWortInAnfuehrungszeichen_( zeile, pos );
          i += pos;
        }
      }
    }
  }
  DBG_PROG_ENDE
}

int
CgatsFilter::sucheInDATA_FORMAT_( std::string &zeile )
{
  std::string::size_type pos=0;
  int n = 0;

  zeilenOhneDuplikate_( s_woerter_ );

  for( unsigned int i = 0; i < s_woerter_.size() ; ++i )
  { 
    pos = 0;
    while( (pos = sucheWort( zeile, s_woerter_[i], pos )) != std::string::npos )
    {
      ++pos;
      ++n;
    }
  }

  return n;
}

int
CgatsFilter::zeilenOhneDuplikate_ ( std::vector<std::string> &zeilen )
{
  int n = 0;

  sort( zeilen.begin(), zeilen.end() );
  std::vector<std::string> ::iterator pos; 
  for( unsigned int i = 0; i < zeilen.size()-1; ++i)
    while ( i < zeilen.size()-1 &&
            zeilen[i] == zeilen[i+1] )
    {
      DBG_NUM_S( zeilen[i] <<"="<< zeilen[i+1] << " gelöscht" )
      zeilen.erase( zeilen.begin()+i+1 );
      if( i+1 < zeilen.size() )
        DBG_NUM_S( zeilen[i+1] )
      ++n;
    }
  return n;
}

int
CgatsFilter::sucheSchluesselwort_( std::string zeile )
{
  std::string::size_type pos=0;

  {
    #define \
    KEY_Suche(suche, ret) \
      if ((pos = zeile.find (suche, 0)) != std::string::npos) \
      { \
        DBG_NUM_S( ret <<" "<< zeile.substr(pos,3) ) \
        return ret; \
      }
    KEY_Suche( "KEYWORD",            KEYWORD )
    KEY_Suche( "SAMPLE_ID",          DATA_FORMAT_ZEILE )
    KEY_Suche( "SAMPLE_NAME",        DATA_FORMAT_ZEILE )
    KEY_Suche( "ORIGINATOR",         ANFUEHRUNGSSTRICHE )
    KEY_Suche( "DESCRIPTOR",         ANFUEHRUNGSSTRICHE )
    KEY_Suche( "CREATED",            ANFUEHRUNGSSTRICHE )
    KEY_Suche( "MANUFACTURER",       ANFUEHRUNGSSTRICHE )
    KEY_Suche( "PROD_DATE",          ANFUEHRUNGSSTRICHE )
    KEY_Suche( "SERIAL",             ANFUEHRUNGSSTRICHE )
    KEY_Suche( "MATERIAL",           ANFUEHRUNGSSTRICHE )
    KEY_Suche( "INSTRUMENTATION",    ANFUEHRUNGSSTRICHE )
    KEY_Suche( "MEASUREMENT_SOURCE", ANFUEHRUNGSSTRICHE )
    KEY_Suche( "PRINT_CONDITIONS",   ANFUEHRUNGSSTRICHE )
    KEY_Suche( "NUMBER_OF_FIELDS",   BELASSEN )
    KEY_Suche( "BEGIN_DATA_FORMAT",  BELASSEN )
    KEY_Suche( "END_DATA_FORMAT",    BELASSEN )
    KEY_Suche( "NUMBER_OF_SETS",     BELASSEN )
    KEY_Suche( "BEGIN_DATA",         BELASSEN )
    KEY_Suche( "END_DATA",           BELASSEN )
    KEY_Suche( "GRAY_BALANCE",       LINEARISIERUNG )
    KEY_Suche( "CMY Target",         CMY_DATEN )
    KEY_Suche( "CMY Daily Target",   CMY_DATEN )
    KEY_Suche( "CMYK Target",        CMYK_DATEN )
  }

  return AUSKOMMENTIEREN;
}

int
CgatsFilter::editZeile_( std::vector<std::string> &zeilen,
                         int i, int editieren, bool cmy )
{
  DBG_PROG_START
  int zeilendifferenz = 0;
  std::string::size_type ende, pos;
  if( (ende = zeilen[i].find( "#", 0 )) == std::string::npos )
    ende = zeilen[i].size();
  pos = zeilen[i].find_first_not_of( cgats_alnum_ );

  DBG_NUM_V( pos <<" "<< zeilen[i] )
  switch( editieren )
  {
    case BELASSEN: break;
    case KEYWORD:  editZeile_( zeilen, i, ANFUEHRUNGSSTRICHE, false );
                   pos = zeilen[i].find_first_of('"');
                   ende = zeilen[i].find_last_of('"');
                   DBG_NUM_S( zeilen[i].substr( pos + 1, ende - pos - 1 ) )
                   s_woerter_.insert( s_woerter_.begin(), zeilen[i].substr(
                                         pos + 1, ende - pos - 1) );
                   DBG_NUM_S( "neues Schlüsselwort: " << s_woerter_[0] )
                   break;
    case ANFUEHRUNGSSTRICHE: {
              std::string gtext = zeilen[i].substr( 0, zeilen[i].find( "#" ) );
              // das erste Zeichen nach dem Schlüsselwort
              //while ( strchr( cgats_alnum_, zeilen[i][pos] )
              pos = gtext.find_first_not_of( cgats_alnum_ ,pos, ende );
              DBG_NUM_V( pos )
              if( (ende = gtext.find( "#", 0 )) == std::string::npos )
                ende = gtext.size();

              int size = gtext.size();
              zeilen[i].erase( 0, size );
              setzeWortInAnfuehrungszeichen_( gtext, pos );
              zeilen[i].insert( 0, gtext );
            }
            break;
    case DATA_FORMAT_ZEILE: // einige CGATS Dateien kennen nur:
            // SAMPLE_ID C M Y K         / auch  L A B C H ?
            {
              // C M Y und K suchen und ersetzen
              if( 0 ) // cmy
              {
                suchenErsetzen ( zeilen[i], "\tC\t", "\tCMY_C\t", 0 );
                suchenErsetzen ( zeilen[i], "\tM\t", "\tCMY_M\t", 0 );
                suchenErsetzen ( zeilen[i], "\tY", "\tCMY_Y", 0 );
              } else {
                suchenErsetzen ( zeilen[i], "\tC\t", "\tCMYK_C\t", 0 );
                suchenErsetzen ( zeilen[i], "\tM\t", "\tCMYK_M\t", 0 );
                suchenErsetzen ( zeilen[i], "\tY", "\tCMYK_Y", 0 );
                suchenErsetzen ( zeilen[i], "\tO", "\tRGB_R", 0 );
                suchenErsetzen ( zeilen[i], "\tG", "\tRGB_G", 0 );
                suchenErsetzen ( zeilen[i], "\tB", "\tD_BLUE", 0 );
                suchenErsetzen ( zeilen[i], "\tK", "\tCMYK_K", 0 );
              }
              int zaehler_FIELDS = sucheInDATA_FORMAT_( zeilen[i] );
              std::stringstream s;
              s << "NUMBER_OF_FIELDS " << zaehler_FIELDS;
              zeilen.insert( zeilen.begin() + i, s.str() );
              ++zeilendifferenz;
              zeilen.insert( zeilen.begin() + i + 1, "BEGIN_DATA_FORMAT" );
              ++zeilendifferenz;
              zeilen.insert( zeilen.begin() + i + 3, "END_DATA_FORMAT" );
              ++zeilendifferenz;
            }
            break;
    case CMY_DATEN:
    case CMYK_DATEN:
    case AUSKOMMENTIEREN:
            if( zeilen[i].size() &&
                zeilen[i][0] != '#' )
              zeilen[i].insert( 0, "# " );
            break;
    case LINEARISIERUNG:
            if( cmy )
              zeilendifferenz += 9;
            else
              zeilendifferenz += 12;
            break;
  }
  DBG_NUM_V( zeilen[i] )
  DBG_PROG_ENDE
  return zeilendifferenz;
}

std::string
CgatsFilter::cgats_korrigieren               ()
{ DBG_PROG_START
  std::string::size_type pos=0;
  std::string::size_type ende;

  data_ = data_orig_;

  // Zeilenumbrüche reparieren
  // LF CR
  pos = 0;
  char CRLF[3];
  char LF[2];
  sprintf (CRLF , "\r\n");
  sprintf (LF , "\n");
  if(suchenErsetzen (data_,CRLF,LF,pos) != std::string::npos)
  {
      DBG_NUM_S( "LF CR ersetzt" )
  }
  char CR[2];
  sprintf (CR , "\r");
  if(suchenErsetzen (data_,CR,LF,pos) != std::string::npos)
  {
      DBG_NUM_S( "CR ersetzt" )
  }

  #if 0
  // testweises Speichern
  std::ofstream f ( "AtestCGATS.vim",  std::ios::out );
  f.write ( data_.c_str(), data_.size() );
  f.close();
  #endif


  // reparieren: Sample_Name , SampleID, ""
  #define SUCHENundERSETZEN( suchen, ersetzen ) \
  pos = 0; \
  if(suchenErsetzen ( data_, suchen, ersetzen, pos ) != std::string::npos) \
  { \
      DBG_NUM_S( suchen " ersetzt" ) \
  }
  // FORMAT Bezeichner
  SUCHENundERSETZEN( "SampleID" , "SAMPLE_ID" )
  SUCHENundERSETZEN( "Sample_ID" , "SAMPLE_ID" )
  SUCHENundERSETZEN( "SampleName" , "SAMPLE_NAME" )
  SUCHENundERSETZEN( "Sample_Name" , "SAMPLE_NAME" )
  SUCHENundERSETZEN( "Lab_L" , "LAB_L" )
  SUCHENundERSETZEN( "Lab_a" , "LAB_A" )
  SUCHENundERSETZEN( "Lab_b" , "LAB_B" )
  // Dateibezeichner
  SUCHENundERSETZEN( "CBTD" , "CBTD___" )
  SUCHENundERSETZEN( "CBPR" , "CBPR___" )
  SUCHENundERSETZEN( "CBTA" , "CBTA___" )
  SUCHENundERSETZEN( "CBRO" , "CBRO___" ) // wird nicht behandelt
  // Sonstiges
  SUCHENundERSETZEN( "Date:" , "CREATED \"\" #" ) // nicht ganz korrekt; für GMB


  // zeilenweise
  std::vector<std::string> zeilen = zeilenNachVector(data_);

  // Zahlen
  // es gibt zwei Blöcke  BEGIN_DATA / END_DATA und BEGIN_DATA_FORMAT / END_...
  bool im_data_format_block = false;
  bool im_data_block = false;
  int zeile_letztes_BEGIN_DATA = -1;
  int zeile_letztes_BEGIN_DATA_FORMAT = -1;
  int zeile_letztes_NUMBER_OF_SETS = -1;
  int zeile_letztes_NUMBER_OF_FIELDS = -1;
  bool fehlendes_NUMBER_OF_SETS = true;
  bool fehlendes_NUMBER_OF_FIELDS = true;
  int zaehler_SETS = -1;
  int zaehler_FIELDS = 0;
  std::stringstream s;
  std::string gtext; // gültiger Text
  bool cmy_daten = false;

  // zeilenweises Bearbeiten
  for (unsigned i = 0; i < zeilen.size(); ++i)
  {
    // Hole eine kommentarfreie Zeile
    gtext = zeilen[i].substr( 0, zeilen[i].find( "#" ) );

    // NUMBER_OF_FIELDS Position merken
    if( sucheWort (gtext, "NUMBER_OF_FIELDS", 0 ) != std::string::npos )
    {
      zeile_letztes_NUMBER_OF_FIELDS = i;
      DBG_NUM_S( "NUMBER_OF_FIELDS Zeile " << i )
    }
    // NUMBER_OF_SETS Position merken
    if( sucheWort (gtext, "NUMBER_OF_SETS", 0 ) != std::string::npos )
    {
      zeile_letztes_NUMBER_OF_SETS = i;
      DBG_NUM_S( "NUMBER_OF_SETS Zeile " << i )
    }

    if( im_data_block )
    {
      // SETS zählen
      if( gtext.find_first_of( cgats_alnum_ ) != std::string::npos )
        ++zaehler_SETS;

      // in gtext , durch . ersetzen und danach in zeilen[i] zurückschreiben
      if(suchenErsetzen ( gtext, ",", ".", 0 ) != std::string::npos)
      {
        zeilen[i].replace( 0, gtext.size(), gtext );
        DBG_NUM_S( ", ersetzt" )
      }
    }

    // FIELDS zählen
    if( im_data_format_block )
    {
      zaehler_FIELDS += sucheInDATA_FORMAT_( gtext );
      DBG_NUM_S( "zaehler_FIELDS " << zaehler_FIELDS << " Zeile " << i )
    }
    
    // DATA Block
    if( sucheWort (gtext, "BEGIN_DATA", 0 ) != std::string::npos )
    {
      // Markieren und Warnen
      if( im_data_block )
        WARN_S( "oops zwei mal BEGIN_DATA  Zeile " << i )
      else
        im_data_block = true;

      if( zeile_letztes_NUMBER_OF_SETS >= 0)
        fehlendes_NUMBER_OF_SETS = false;

      zeile_letztes_BEGIN_DATA = i;
      DBG_NUM_S( "BEGIN_DATA Zeile " << i )
    }

    if( sucheWort (gtext, "END_DATA", 0 ) != std::string::npos )
    {
      if( !im_data_block )
        WARN_S( "oops END_DATA ohne BEGIN_DATA  Zeile " << i )

      // NUMBER_OF_SETS einfügen
      if( im_data_block && fehlendes_NUMBER_OF_SETS )
      {
        s.str("");
        s << "NUMBER_OF_SETS " << zaehler_SETS;
        zeilen.insert( zeilen.begin() + zeile_letztes_BEGIN_DATA, s.str() );

        ++i;
        DBG_S( zeilen[zeile_letztes_BEGIN_DATA] << " eingefügt" )
      }

      DBG_NUM_S( "END_DATA Zeile " << i << " mit " << zaehler_SETS << " Messfeldern" )
      im_data_format_block = false;
      zeile_letztes_NUMBER_OF_SETS = -1;
      zaehler_SETS = -1;
    }
    // ENDE DATA Block

    // DATA_FORMAT Block
    if( sucheWort (gtext, "BEGIN_DATA_FORMAT", 0 ) != std::string::npos )
    {
      // Markieren und Warnen
      if( im_data_format_block )
        WARN_S( "oops zwei mal BEGIN_DATA_FORMAT  Zeile " << i )
      else
        im_data_format_block = true;

      if( zeile_letztes_NUMBER_OF_FIELDS >= 0)
        fehlendes_NUMBER_OF_FIELDS = false;

      zeile_letztes_BEGIN_DATA_FORMAT = i;
      DBG_NUM_S( "BEGIN_DATA_FORMAT Zeile " << i )
    }

    if( sucheWort (gtext, "END_DATA_FORMAT", 0 ) != std::string::npos )
    {
      if( !im_data_format_block )
        WARN_S( "oops END_DATA_FORMAT ohne BEGIN_DATA_FORMAT  Zeile " << i )

      // NUMBER_OF_FIELDS einfügen
      if( im_data_format_block &&
          fehlendes_NUMBER_OF_FIELDS )
      {
        s.str("");
        s << "NUMBER_OF_FIELDS " << zaehler_FIELDS;
        zeilen.insert( zeilen.begin() + zeile_letztes_BEGIN_DATA_FORMAT, s.str() );
        
        ++i;
        DBG_S( zeilen[zeile_letztes_BEGIN_DATA_FORMAT] << " eingefügt" )
      }

      im_data_format_block = false;
      zeile_letztes_NUMBER_OF_FIELDS = -1;
      zaehler_FIELDS = -1;
      DBG_NUM_S( "END_DATA_FORMAT Zeile " << i )
    }
    // ENDE DATA_FORMAT Block

    // Datei Signatur reparieren
    if( i == 0 )
    {
      pos  = gtext.find_first_of( cgats_alnum_ );
      if((ende = gtext.find_first_not_of( cgats_alnum_ )) == std::string::npos||
          gtext.find_first_not_of( cgats_alnum_ ) > ende )
      {
        ende = gtext.size();
      }
      DBG_NUM_V( pos <<" "<< ende <<" "<< gtext )
      if( ((ende - pos) != 7 &&
           (ende - pos) != 14 ) ||
          sucheSchluesselwort_( gtext ) != 3 )
      {
        zeilen.insert( zeilen.begin(), "ICCEXAM" );
        DBG_NUM_S( "Beschreibung eingeführt" )
      }
    } else
    // Schlüsselwörter aufschlüsseln (Keywords)
      if( !im_data_block && !im_data_format_block )
    {
      int editieren = sucheSchluesselwort_(gtext);
      if( editieren == CMY_DATEN ) 
        cmy_daten = true;
      else if( editieren == CMYK_DATEN )
        cmy_daten = false;

      if( editieren )
      { // ... zuschlüsseln (bearbeiten ;) - dabei Zeilendifferenz hinzuzählen
        i = i + editZeile_( zeilen, i, editieren, cmy_daten );
      }
    }
  }

  // Text neu aus Zeilen zusammenstellen
  data_.clear();
  for( unsigned int i = 0; i < zeilen.size(); ++i)
  {
    data_.append( zeilen[i] );
    data_.append( "\n" );
  }

  //DBG_NUM_S (data_)
  DBG_PROG_ENDE
  return data_;
}


std::string
cgats_korrigieren( char* data, size_t size )
{ DBG_PROG_START
  CgatsFilter cgats;
  cgats.lade( data, size );
  std::string text = cgats.lcms_gefiltert ();
  DBG_PROG_ENDE
  return text;
}

