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


/*
   ChangeLog

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
	* 

 */

#include "icc_utils.h"
#include "icc_helfer.h"

#include <fstream>
#include <sstream>



// --- Hauptfunktion ---
// Konvertierung in Standard unix Dateiformat mit LF
// Suchen und Ersetzen bekannnter Abweichungen (in einem std::string)
// zeilenweises lesen und editieren (in einem vector aus strings)
// verdecken der Kommentare
// kontrollieren der Blöckanfänge und -enden
// die Dateisignatur reparieren (7 / 14 byte lang)
// zwischen den Blöcken die Keywords erkennen und entsprechend bearbeiten
// die Zeilen wieder zusamenfügen und als einen std::string zurückgeben
std::string cgats_korrigieren               (char* _data, size_t _size);

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
void zeileEinfuegen_( std::vector<std::string> &zeilen,
                      int                       pos_n,
                      std::string               neue_zeile );


// TODO in Klasse einbauen
  char cgats_alnum[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz_|/-+=()[]{}<>&?!:;,.0123456789";
  char leer_zeichen[] = { 0, 32, '\t', '\n', '\v', '\f', '\r' };

int
sucheInDATA_FORMAT_( std::string &zeile )
{
  std::string::size_type pos=0;
  int n = 0;

  {
    #define \
    DF_Suche(suche, pos_) /* pos spielt nur bei SPECTRAL_ eine Rolle */ \
    { pos = pos_; \
      if ((pos = zeile.find (suche, pos)) != std::string::npos) \
      { \
        ++n; \
        /*DBG_NUM_S( pos << zeile.substr(pos,3) )*/ \
        pos = pos + strlen( suche ); \
      } \
    }
    DF_Suche( "SAMPLE_ID", 0 )
    DF_Suche( "SAMPLE_NAME", 0 )
    DF_Suche( "CMYK_C", 0 )
    DF_Suche( "CMYK_M", 0 )
    DF_Suche( "CMYK_Y", 0 )
    pos = 0;
    while( pos < zeile.size() )
      DF_Suche( "CMYK_K", pos )
    DF_Suche( "RGB_R", 0 )
    DF_Suche( "RGB_G", 0 )
    DF_Suche( "RGB_B", 0 )
    DF_Suche( "XYZ_X", 0 )
    DF_Suche( "XYZ_Y", 0 )
    DF_Suche( "XYZ_Z", 0 )
    DF_Suche( "XYY_X", 0 )
    DF_Suche( "XYY_Y", 0 )
    DF_Suche( "XYY_CAPY", 0 )
    DF_Suche( "LAB_L", 0 )
    DF_Suche( "LAB_A", 0 )
    DF_Suche( "LAB_B", 0 )
    DF_Suche( "D_RED", 0 )
    DF_Suche( "D_GREEN", 0 )
    DF_Suche( "D_BLUE", 0 )
    DF_Suche( "D_VIS", 0 )
    DF_Suche( "D_MAJOR_FILTER", 0 )
    DF_Suche( "SPECTRAL_PCT", pos )
    DF_Suche( "SPECTRAL_DEC", pos )
    // sich wiederholende Zeichen
    pos = 0;
    while( pos < zeile.size() )
      DF_Suche( "SPECTRAL_", pos )
    pos = 0;
    while( pos < zeile.size() )
      DF_Suche( "nm", pos )  // nicht standard
    pos = 0;
    while( pos < zeile.size() )
      DF_Suche( "SPECTRUM_", pos ) // nicht standard
    pos = 0;
    while( pos < zeile.size() )
      DF_Suche( "R_", pos ) // nicht standard
    DF_Suche( "LAB_C", 0 )
    DF_Suche( "LAB_H", 0 )
    DF_Suche( "LAB_DE", 0 )
    DF_Suche( "LAB_DE_94", 0 )
    DF_Suche( "LAB_DE_CMC", 0 )
    DF_Suche( "LAB_DE_2000", 0 )
    DF_Suche( "MEAN_DE", 0 )
    DF_Suche( "STDEV_X", 0 )
    DF_Suche( "STDEV_Y", 0 )
    DF_Suche( "STDEV_Z", 0 )
    DF_Suche( "STDEV_L", 0 )
    DF_Suche( "STDEV_A", 0 )
    DF_Suche( "STDEV_B", 0 )
    DF_Suche( "STDEV_DE", 0 )
    DF_Suche( "CHI_SQD_PAR", 0 )
  }

  return n;
}

int
sucheSchluesselwort_( std::string zeile )
{
  std::string::size_type pos=0;

  enum {
    BELASSEN,
    ANFUEHRUNGSSTRICHE,
    DATA_FORMAT_ZEILE,
    AUSKOMMENTIEREN,
    LINEARISIERUNG,
    CMY_DATEN,
    CMYK_DATEN
  };

  {
    #define \
    KEY_Suche(suche, ret) \
      if ((pos = zeile.find (suche, 0)) != std::string::npos) \
      { \
        DBG_NUM_S( ret <<" "<< zeile.substr(pos,3) ) \
        return ret; \
      }
    KEY_Suche( "KEYWORD",            ANFUEHRUNGSSTRICHE )
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

void
zeileEinfuegen_( std::vector<std::string> &zeilen,
                int                       pos_n,
                std::string               neue_zeile )
{
        // Position im Zeilenvektor suchen
        std::vector<std::string>::iterator v_pos = zeilen.begin();
        int n = 0;
        while( n < pos_n)
        {
          ++n;
          ++v_pos;
        }
        // neue Zeile Einfügen
        zeilen.insert(v_pos, neue_zeile );
}

int
editZeile_( std::vector<std::string> &zeilen, int i, int editieren, bool cmy )
{
  DBG_PROG_START
  int zeilendifferenz = 0;
  std::string::size_type ende, pos;
  if( (ende = zeilen[i].find( "#", 0 )) == std::string::npos )
    ende = zeilen[i].size();
  pos = zeilen[i].find_first_not_of( cgats_alnum );
  
  DBG_NUM_V( pos <<" "<< zeilen[i] )
  switch( editieren )
  {
    case 0: break; //BELASSEN
    case 1: { //ANFUEHRUNGSSTRICHE,
              bool in_anfuehrung = false;
              int letze_anfuehrungsstriche = -1;
              // das erste Zeichen nach dem Schlüsselwort
              //while ( strchr( cgats_alnum, zeilen[i][pos] )
              pos = zeilen[i].find_first_not_of( cgats_alnum ,pos, ende );
              DBG_NUM_V( pos )
              for( ; pos < ende; ++pos )
              { DBG_NUM_V( pos )
                if( zeilen[i][pos] == '\"' )
                { DBG_NUM_V( zeilen[i].substr( pos, ende-pos ) )
                  if( in_anfuehrung ) // hinaus
                  {
                    in_anfuehrung = false;
                    letze_anfuehrungsstriche = -1;
                  } else {            // hinein
                    in_anfuehrung = true;
                    letze_anfuehrungsstriche = pos;
                  }
                } else if( strchr( cgats_alnum, zeilen[i][pos] ) &&//ein Zeichen
                           !in_anfuehrung )                     // .. ausserhalb
                { DBG_NUM_V( zeilen[i].substr( pos, ende-pos ) )
                  zeilen[i].insert( pos, "\"" );
                  ++ende;
                  ++pos;
                  in_anfuehrung = true;
                  letze_anfuehrungsstriche = pos-1;
                  DBG_NUM_S( zeilen[i].substr( pos, ende-pos ) )
                }
#if 0
                if(!( zeilen[i].find( "\"" ) <
                      zeilen[i].find_first_of( cgats_alnum ,pos, ende ) ))
                  zeilen[i].insert( pos+1, "\"" );
                if( zeilen[i].find_last_of( cgats_alnum ) < zeile.find( "\"" ) )
                {
                  zeilen[i].insert( ende-1, "\"" );
                }
#endif
              }
              if( in_anfuehrung )
                zeilen[i].insert( ende-1, "\"" );
            }
            break;
    case 2: //DATA_FORMAT_ZEILE, einige CGATS Dateien kennen nur:
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
                suchenErsetzen ( zeilen[i], "\tO", "\tD_RED", 0 );
                suchenErsetzen ( zeilen[i], "\tG", "\tD_GREEN", 0 );
                suchenErsetzen ( zeilen[i], "\tB", "\tD_BLUE", 0 );
                suchenErsetzen ( zeilen[i], "\tK", "\tCMYK_K", 0 );
              }
              int zaehler_FIELDS = sucheInDATA_FORMAT_( zeilen[i] );
              std::stringstream s;
              s << "NUMBER_OF_FIELDS " << zaehler_FIELDS;
              zeileEinfuegen_( zeilen, i, s.str() );
              ++zeilendifferenz;
              zeileEinfuegen_( zeilen, i+1, "BEGIN_DATA_FORMAT" );
              ++zeilendifferenz;
              zeileEinfuegen_( zeilen, i+3, "END_DATA_FORMAT" );
              ++zeilendifferenz;
            }
            break;
    case 5: // CMY Target
    case 6: // CMYK Target
    case 3: //AUSKOMMENTIEREN,
            if( zeilen[i].size() &&
                zeilen[i][0] != '#' )
              zeilen[i].insert( 0, "# " );
            break;
    case 4: //LINEARISIERUNG
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
cgats_korrigieren               (char* _data, size_t _size)
{ DBG_PROG_START
  std::string::size_type pos=0;
  std::string::size_type ende;

  // char* -> std::string
  std::string data (_data, 0, _size);

  // Zeilenumbrüche reparieren
  // LF CR
  pos = 0;
  char CRLF[3];
  char LF[2];
  sprintf (CRLF , "\r\n");
  sprintf (LF , "\n");
  if(suchenErsetzen (data,CRLF,LF,pos) != std::string::npos)
  {
      DBG_NUM_S( "LF CR ersetzt" )
  }
  char CR[2];
  sprintf (CR , "\r");
  if(suchenErsetzen (data,CR,LF,pos) != std::string::npos)
  {
      DBG_NUM_S( "CR ersetzt" )
  }

  #if 0
  // testweises Speichern
  std::ofstream f ( "AtestCGATS.vim",  std::ios::out );
  f.write ( data.c_str(), data.size() );
  f.close();
  #endif


  // reparieren: Sample_Name , SampleID, ""
  #define SUCHENundERSETZEN( suchen, ersetzen ) \
  pos = 0; \
  if(suchenErsetzen ( data, suchen, ersetzen, pos ) != std::string::npos) \
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
  std::vector<std::string> zeilen = zeilenNachVector(data);

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
      if( gtext.find_first_of( cgats_alnum ) != std::string::npos )
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
        zeileEinfuegen_( zeilen, zeile_letztes_BEGIN_DATA, s.str() );

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
        zeileEinfuegen_( zeilen, zeile_letztes_BEGIN_DATA_FORMAT, s.str() );
        
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
      pos  = gtext.find_first_of( cgats_alnum );
      if((ende = gtext.find_first_not_of( cgats_alnum )) == std::string::npos ||
          gtext.find_first_not_of( cgats_alnum ) > ende )
      {
        ende = gtext.size();
      }
      DBG_NUM_V( pos <<" "<< ende <<" "<< gtext )
      if( ((ende - pos) != 7 &&
           (ende - pos) != 14 ) ||
          sucheSchluesselwort_( gtext ) != 3 )
      {
        zeileEinfuegen_( zeilen, 0, "ICCEXAM" );
        DBG_NUM_S( "Beschreibung eingeführt" )
      }
    } else
    // Schlüsselwörter aufschlüsseln (Keywords)
      if( !im_data_block && !im_data_format_block )
    {
      int editieren = sucheSchluesselwort_(gtext);
      if( editieren == 5 ) // CMY_DATEN 
        cmy_daten = true;
      else if( editieren == 6 ) // CMYK_DATEN
        cmy_daten = false;

      if( editieren )
      { // ... zuschlüsseln (bearbeiten ;) - dabei Zeilendifferenz hinzuzählen
        i = i + editZeile_( zeilen, i, editieren, cmy_daten );
      }
    }
  }

  // Text neu aus Zeilen zusammenstellen
  data.clear();
  for( unsigned int i = 0; i < zeilen.size(); ++i)
  {
    data.append( zeilen[i] );
    data.append( "\n" );
  }

  //DBG_NUM_S (data)
  DBG_PROG_ENDE
  return data;
}


