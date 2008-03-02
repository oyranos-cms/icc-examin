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
 */

 /** @brief cgats Fehlerkorrekturen
 * 
 */



#include "icc_utils.h"
#include "icc_helfer.h"
#include "icc_cgats_filter.h"

#include <fstream>
#include <sstream>
#include <algorithm>

using namespace icc_parser;

//#define CGATS_DEBUG 2
#ifdef CGATS_DEBUG
#define DBG_CGATS DBG_PROG
#define DBG_CGATS_START DBG_PROG_START
#define DBG_CGATS_ENDE DBG_PROG_ENDE
#define DBG_CGATS_S( text ) DBG_PROG_S( text )
#define DBG_CGATS_V( text ) DBG_PROG_V( text )
#else
#define DBG_CGATS
#define DBG_CGATS_START
#define DBG_CGATS_ENDE
#define DBG_CGATS_S( text ) 
#define DBG_CGATS_V( text )
#endif

//! statische Initialisierungen

const char *CgatsFilter::cgats_alnum_ = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz_|/-+=()[]{}<>&?!:;,.0123456789";
const char *CgatsFilter::cgats_numerisch_ = "-+,.0123456789";
const char *CgatsFilter::cgats_ziffer_ = "0123456789";
const char *CgatsFilter::cgats_alpha_ = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz_|/=()[]{}<>&?!:;";
const char *CgatsFilter::leer_zeichen_ = " \t\n\v\f\r";
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

// Helfer

int
CgatsFilter::suchenUndErsetzen_ ( std::string           &text,
                                  const char*            suchen,
                                  const char*            ersetzen,
                                  std::string::size_type pos )
{
    int n = suchenErsetzen ( text, suchen, ersetzen, pos );
    if( n )
    {
      std::stringstream s;
      s << "Ersetzung: Suchwort, n mal, Ersatzwort \"" << suchen << "\", "
        << n << ", \"" << ersetzen << "\"";
      logEintrag_( s.str(), -1, "", "" );
    }
  return n;
}

// Implementationen

void
CgatsFilter::setzeWortInAnfuehrungszeichen_( std::string &zeile,
                                             std::string::size_type pos )
{
  DBG_CGATS_START
  {
    std::string::size_type ende, pos_orig = pos;

    bool in_anfuehrung = false;
    int letze_anfuehrungsstriche = -1;
    if( (ende = zeile.find_last_of( cgats_alnum_ ))
        == std::string::npos )
    {
      DBG_PROG_S( "Leere Zeile ? : " << zeile )
      DBG_CGATS_ENDE
      return;
    }

    suchenUndErsetzen_( zeile, "\"\"", "\"", 0 ); // GMB

    // Steht dieses Wort bereits in Anfuehrungszeichen
    if( zeile.find_last_of( "\"" ) != std::string::npos &&
        zeile.find_last_of( "\"" ) > ende )
    {
      ende = zeile.find_last_of( "\"" );
    }
    DBG_CGATS_V( pos <<" "<< ende <<" "<< zeile )
    DBG_CGATS_V( zeile.substr( pos, ende-pos +1 ) )
    // zeilenweise
    for( ; pos <= ende; ++pos )
    {
      if( zeile[pos] == '\"' )
      {
        DBG_CGATS_S( pos <<" "<< zeile.substr( pos, ende-pos+1 ) )
        if( in_anfuehrung ) // hinaus
        {
          in_anfuehrung = false;
          letze_anfuehrungsstriche = -1;
        } else {            // hinein
          in_anfuehrung = true;
          letze_anfuehrungsstriche = (int)pos;
        }
      } else if( strchr( cgats_alnum_, zeile[pos] ) &&// ein Zeichen
                 !in_anfuehrung )                     // .. ausserhalb
      {
        DBG_CGATS_V( zeile.substr( pos, ende-pos+1 ) )
        zeile.insert( pos, "\"" );
        ++ende;
        ++pos;
        in_anfuehrung = true;
        letze_anfuehrungsstriche = (int)pos-1;
        DBG_CGATS_S( zeile.substr( pos, ende-pos+1 ) )
      }
    }
    DBG_CGATS_V( in_anfuehrung )
    // Abschluss "
    if( in_anfuehrung )
      zeile.insert( ende+1, "\"" );
    // keine leere Zeile ohne ""
    std::string t = zeile.substr( pos_orig, ende-pos_orig+1 );
    if( t.find_last_of( cgats_alnum_ ) == std::string::npos &&
        t.find_last_of( "\"" ) == std::string::npos )
      zeile.insert( zeile.size(), "\"\"" );
  }
  DBG_CGATS_ENDE
}

std::vector<std::string>
CgatsFilter::unterscheideZiffernWorte_( std::string &zeile )
{
  DBG_CGATS_START
  std::string::size_type pos = 0, ende = 0, pos2, pos3;
  char text[64];
  bool in_anfuehrung = false;
  std::string txt;
  std::vector<std::string> ergebnis;

  suchenUndErsetzen_( zeile, ",", ".", 0 );

  // Worte Suchen und von Zahlen scheiden
  for( pos = 0; pos < zeile.size() ; ++pos )
  {
    DBG_CGATS_V( pos <<" "<< zeile.size() )
    in_anfuehrung = false;
    pos2 = pos;

    // erstes Zeichen suchen
    if( (pos = zeile.find_first_of( cgats_alnum_, pos )) != std::string::npos )
    {
      bool anf_zaehlen = true;
      // erstes Anfuehrungszeichen suchen
      if( zeile.find_first_of( "\"", pos2 ) != std::string::npos &&
          zeile.find_first_of( "\"", pos2 ) < pos )
        pos2 = zeile.find_first_of( "\"", pos2 );
      else
        anf_zaehlen = false;
      DBG_CGATS_V( pos2 )

      // Anfuehrungszeichen z&auml;hlen [ ""  " "  ABC ] - zeichenweise
      int letzes_anf_zeichen = -1;
      if( anf_zaehlen )
        for( pos3 = pos2; pos3 < pos; ++pos3)
          if( zeile[pos3] == '"' &&
              letzes_anf_zeichen >= 0 )
            letzes_anf_zeichen = -1;
          else
            letzes_anf_zeichen = (int)pos3;

      // falls ein Anfuehrungszeichen vor dem Wort ungerade sind // ["" " ABC ]
      if( letzes_anf_zeichen >= 0 )
      {
        in_anfuehrung = true;
        pos2 = pos;
        pos = letzes_anf_zeichen+1;
      }
      DBG_CGATS_V( pos )
      // das Ende des Wortes finden
      if( in_anfuehrung )
        ende = zeile.find_first_of( "\"", pos + 1 );
      else
        if( (ende = zeile.find_first_of( leer_zeichen_, pos2 + 1 ))
            == std::string::npos )
          ende = zeile.size();

      // bitte das erste Wort einbeziehen
      if( zeile.find_first_of( cgats_alnum_, pos ) > ende )
        ende = zeile.find_first_of( leer_zeichen_,
                                zeile.find_first_of( cgats_alnum_, pos ) + 1 );
      // das Wort extrahieren
      txt = zeile.substr( pos, ende-pos );
      ergebnis.push_back( txt );
      DBG_CGATS_V( pos <<" "<< ende )
#     if (CGATS_DEBUG >= 2)
      cout << zeile << endl;
      for(unsigned int j = 0; j < zeile.size();    ++j)
        if( j != pos && j != ende )
          cout << " ";
        else
          cout << "^";
      cout << "\n";
#     endif
      // das Wort untersuchen
      if( txt.find_first_of( cgats_numerisch_ ) != std::string::npos &&
          txt.find( "." ) != std::string::npos &&
          txt.find_first_of( cgats_alpha_ ) == std::string::npos )
      { // ist Flie&szlig;kommazahl
        pos += txt.size();
        sprintf( text, "%f", atof( zeile.substr( pos, ende-pos ).c_str() ) );
        DBG_CGATS_S( "Flie&szlig;kommazahl: " << txt )
      } else
      if( txt.find_first_of( cgats_ziffer_ ) != std::string::npos &&
          txt.find( "." ) == std::string::npos &&
          txt.find_first_of( cgats_alpha_ ) == std::string::npos )
      { // ist Ganzzahl
        pos += txt.size();
        sprintf( text, "%d", atoi( zeile.substr( pos, ende-pos ).c_str() ) );
        DBG_CGATS_S( "Ganzzahl: " << txt )
      } else
      if( txt.find_first_of( cgats_alnum_ ) != std::string::npos )
      { // ist Text
        bool neusetzen = false;
        int anf = 0;
        DBG_CGATS_S( "Text: " << txt )
        DBG_CGATS_V( in_anfuehrung )
        // Anfuehrungszeichen Beginn
        if( !in_anfuehrung && anfuehrungsstriche_setzen )
        {
          zeile.insert( pos, "\"" );
          in_anfuehrung = true;
          neusetzen = true;
          ++pos;
          ++anf;
        }
        if( anf_zaehlen )
          in_anfuehrung = false;
        // Ende des Wortes
        if( (pos2 = zeile.find_first_of( leer_zeichen_, pos ))
            == std::string::npos)
          pos2 = zeile.size();
        // Es folgt ein Anfuehrungszeichen
        if( pos2 > zeile.find_first_of( "\"", pos ) &&
            zeile.find_first_of( "\"", pos ) != std::string::npos &&
            !neusetzen )
          pos2 = zeile.find_first_of( "\"", pos );
        
        if( in_anfuehrung && anfuehrungsstriche_setzen )
        {
          DBG_CGATS_V( zeile )
          zeile.insert( pos2, "\"" );
          in_anfuehrung = false;
          ++anf;
        }
        ende += anf;
        pos = ende;
      }
      DBG_CGATS_V( pos << zeile.size() )
      // Schleife beenden
      if( zeile.find_first_of( cgats_alnum_, pos ) == std::string::npos )
      {
        DBG_CGATS_V( zeile.find_first_of( cgats_alnum_, pos ) )
        pos = zeile.size();
      }
      DBG_CGATS_V( pos << zeile )
    } else
      pos = zeile.size();
  }
  for( unsigned i = 0; i < ergebnis.size(); i++)
    DBG_CGATS_V( ergebnis[i] );
  DBG_CGATS_ENDE
  return ergebnis;
}

int
CgatsFilter::sucheInDATA_FORMAT_( std::string &zeile , int &zeile_n )
{
  DBG_CGATS_START
  int n = 0;
  bool anf_setzen = anfuehrungsstriche_setzen;
  anfuehrungsstriche_setzen = false;

  std::string::size_type pos=0;
  zeilenOhneDuplikate_( s_woerter_ );

  DBG_NUM_V( zeile )

  if("nm_" != spektral)
    suchenUndErsetzen_( zeile, "nm_", spektral.c_str(), 0 );
  if("nm" != spektral)
    suchenUndErsetzen_( zeile, "nm", spektral.c_str(), 0 );
  if("SPECTRUM_" != spektral)
    suchenUndErsetzen_( zeile, "SPECTRUM_", spektral.c_str(), 0 );
  if("R_" != spektral)
    suchenUndErsetzen_( zeile, "R_", spektral.c_str(), 0 );

  DBG_PROG_V( zeile )

  std::vector<std::string> test = unterscheideZiffernWorte_(zeile);
  for( unsigned int i = 0; i < test.size() ; ++i )
  { 
    pos = 0;
    bool gefunden = false;
    for( unsigned j = 0; j < s_woerter_.size(); ++j )
      if( test[i].find( s_woerter_[j], 0 ) != std::string::npos )
      {
        gefunden = true;
        DBG_NUM_S( s_woerter_[j] <<" "<< test[i] )
      }

    if(!gefunden)
    {
      DBG_PROG_V( zeile_letztes_NUMBER_OF_FIELDS )
      int zeile_i = 1;
      if(zeile_letztes_NUMBER_OF_FIELDS > 0)
        zeile_i = zeile_letztes_NUMBER_OF_FIELDS;
      zeilen_.insert( zeilen_.begin() + zeile_i, "KEYWORD \"" );
      zeilen_[zeile_i].append(test[i]);
      zeilen_[zeile_i].append("\"");
      ++zeile_n;
      ++zeile_letztes_NUMBER_OF_FIELDS;
      logEintrag_( "neues Schluesselwort hinzugefuegt: " + test[i] , zeile_n, "", zeilen_[zeile_letztes_NUMBER_OF_FIELDS] );
    }
  }

  n = (int)(unterscheideZiffernWorte_ (zeile)).size();

  anfuehrungsstriche_setzen = anf_setzen;

  DBG_CGATS_ENDE
  return n;
}

int
CgatsFilter::zeilenOhneDuplikate_ ( std::vector<std::string> &zeilen )
{
  int n = 0;
  DBG_CGATS_V( zeilen.size() )

  std::sort( zeilen.begin(), zeilen.end() );
  std::vector<std::string> ::iterator pos; 
  for( unsigned int i = 0; i < zeilen.size()-1; ++i)
    while ( (i < zeilen.size()-1) &&
            zeilen[i] == zeilen[i+1] )
    {
      DBG_NUM_S( zeilen[i] <<"="<< zeilen[i+1] << " geloescht" )
      zeilen.erase( zeilen.begin()+i+1 );
      if( i+1 < zeilen.size() )
        DBG_NUM_S( zeilen[i+1] );
      ++n;
    }
  DBG_CGATS_V( zeilen.size() )
  for( unsigned i = 0; i < zeilen.size(); i++)
    DBG_CGATS_V( zeilen[i] );
  return n;
}

int
CgatsFilter::sucheSchluesselwort_( std::string zeile )
{
  // Schluesselworte Klassifizieren
  std::string::size_type pos=0;

  {
#   define \
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
    KEY_Suche( "GRAY_BALANCE",       AUSKOMMENTIEREN ) //LINEARISIERUNG )
    KEY_Suche( "CMY Target",         CMY_DATEN )
    KEY_Suche( "CMY Daily Target",   CMY_DATEN )
    KEY_Suche( "CMYK Target",        CMYK_DATEN )
  }
# undef KEY_Suche

  return AUSKOMMENTIEREN;
}

int
CgatsFilter::editZeile_( std::vector<std::string> &zeilen,
                         int i, int editieren, bool cmy )
{
  // Zeilen mit Hilfe der Klassifizierung aus sucheSchluesselwort_() bearbeiten
  DBG_CGATS_START
  int zeilendifferenz = 0;
  std::string::size_type ende, pos;
  std::stringstream s;
  if( (ende = zeilen[i].find( "#", 0 )) == std::string::npos )
    ende = zeilen[i].size();
  pos = zeilen[i].find_first_not_of( cgats_alnum_ );

  DBG_CGATS_V( pos <<" "<< zeilen[i] )
  switch( editieren )
  {
    case BELASSEN: break;
    case KEYWORD:
            {
              editZeile_( zeilen, i, ANFUEHRUNGSSTRICHE, false );
              pos = zeilen[i].find_first_of('"');
              ende = zeilen[i].find_last_of('"');
              std::string txt = zeilen[i].substr( pos + 1, ende-pos - 1 );
              DBG_NUM_S( txt )
              // in Schluesselwortliste suchen
              bool noch_nicht = true;
              for(unsigned int t = 0; t < s_woerter_.size(); ++t)
                if( txt.find(s_woerter_[t]) != std::string::npos )
                  noch_nicht = false;
              if (noch_nicht)
              {
                s_woerter_.insert( s_woerter_.begin(), txt );
                s << "neues Schluesselwort gefunden: " << s_woerter_[0];
                logEintrag_( s.str(), i, zeilen[i], "" );
              }
            }  
            break;
    case ANFUEHRUNGSSTRICHE:
            {
              std::string gtext = zeilen[i].substr( 0, zeilen[i].find( "#" ) );
              DBG_CGATS_V( gtext )
              // das erste Zeichen nach dem Schluesselwort
              pos = gtext.find_first_of( cgats_alnum_ ,0 );
              DBG_CGATS_V( pos )
              pos = gtext.find_first_of( leer_zeichen_ ,pos );
              // Ende vor dem Kommentar
              ende = gtext.size()-1;
              DBG_CGATS_V( pos<<" "<<ende )
              DBG_CGATS_V( gtext.substr( pos, ende-pos +1 ) )
              int size = (int)gtext.size();
              zeilen[i].erase( 0, size );
              setzeWortInAnfuehrungszeichen_( gtext, pos );
              zeilen[i].insert( 0, gtext );
            }
            break;
    case DATA_FORMAT_ZEILE: // einige CGATS Dateien kennen nur:
            // SAMPLE_ID C M Y K       / auch  L A B C H ?  als Formatbezeichner
            {
              // C M Y und K suchen und ersetzen
              unsigned l = 
              logEintrag_( "DATA_FORMAT_ZEILE eigefuegt", i, zeilen[i], "" );
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
              int zaehler_FIELDS = sucheInDATA_FORMAT_( zeilen[i], i );
              s << "NUMBER_OF_FIELDS " << zaehler_FIELDS;
              zeilen.insert( zeilen.begin() + i, s.str() );
              DBG_CGATS_V( messungen.size() )
              log[l].ausgabe.push_back( s.str() );
              ++zeilendifferenz;
              zeilen.insert( zeilen.begin() + i + 1, "BEGIN_DATA_FORMAT" );
              log[l].ausgabe.push_back( "BEGIN_DATA_FORMAT" );
              ++zeilendifferenz;
              messungen[messungen.size()-1].felder.push_back(  
                                       unterscheideZiffernWorte_( zeilen[i] ) );
              zeilen.insert( zeilen.begin() + i + 3, "END_DATA_FORMAT" );
              log[l].ausgabe.push_back( "END_DATA_FORMAT" );
              ++zeilendifferenz;
              DBG_CGATS
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
            // &Uuml;bergehen
            DBG_PROG_V( typ_ )
            if( typ_ == MAX_KORRIGIEREN )
            {
              if( cmy ) {
                zeilendifferenz += 9;
              } else {
                zeilendifferenz += 12;
              }
            } else { // LCMS
              editZeile_( zeilen, i, AUSKOMMENTIEREN, false );
            }
            break;
  }
  DBG_CGATS_V( zeilen[i] )
  DBG_CGATS_ENDE
  return zeilendifferenz;
}

std::string
CgatsFilter::cgats_korrigieren_               ()
{
  DBG_PROG_START
  std::string::size_type pos=0;
  std::string::size_type ende;

  data_ = data_orig_;

  DBG_PROG_V( typ_ )

  // locale - Kommas unterscheiden
  doLocked_m( const char* loc_alt = setlocale(LC_NUMERIC, NULL), NULL )
  if(loc_alt) {
    DBG_NUM_V( loc_alt )
  } else {
    DBG_NUM_S( "keine LANG Variable gefunden" )
  }
  doLocked_m( setlocale(LC_NUMERIC,"C");, NULL )

  // Zeilenumbrueche reparieren
  // LF CR
  pos = 0;
  char CRLF[3];
  char LF[2];
  sprintf (CRLF , "\r\n");
  sprintf (LF , "\n");
  if(suchenErsetzen (data_,CRLF,LF,pos))
      logEintrag_( "LF CR ersetzt", -1, "", "" );

  char CR[2];
  sprintf (CR , "\r");
  if(suchenErsetzen (data_,CR,LF,0))
      logEintrag_( "CR ersetzt", -1, "", "" );

  if (icc_debug >= 2)
  {
    // testweises Speichern
    std::ofstream f ( "AtestCGATS.txt",  std::ios::out );
	f.write ( data_.c_str(), (std::streamsize)data_.size() );
    f.close();
  }


  // Reparieren:
    // FORMAT Bezeichner
  suchenUndErsetzen_( data_, "SampleID" , "SAMPLE_ID", pos ); // GMB
  suchenUndErsetzen_( data_, "Sample_ID" , "SAMPLE_ID", pos );
  suchenUndErsetzen_( data_, "SampleName" , "SAMPLE_NAME", pos );
  suchenUndErsetzen_( data_, "Sample_Name" , "SAMPLE_NAME", pos );
  suchenUndErsetzen_( data_, "Yxy_x", "XYY_X", pos );
  suchenUndErsetzen_( data_, "Yxy_y", "XYY_Y", pos );
  suchenUndErsetzen_( data_, "Yxy_Y", "XYY_CAPY", pos );
  suchenUndErsetzen_( data_, "Lab_L" , "LAB_L", pos );
  suchenUndErsetzen_( data_, "Lab_a" , "LAB_A", pos );
  suchenUndErsetzen_( data_, "Lab_b" , "LAB_B", pos );
    // Dateibezeichner
  suchenUndErsetzen_( data_, "CBTD" , "CBTD___", pos ); // CB
  suchenUndErsetzen_( data_, "CBPR" , "CBPR___", pos );
  suchenUndErsetzen_( data_, "CBTA" , "CBTA___", pos );
  suchenUndErsetzen_( data_, "CBRO" , "CBRO___", pos ); // wird nicht behandelt
    // Sonstiges
  suchenUndErsetzen_( data_, "Date:" , "CREATED \"\" #", pos ); // GMB

  // Zerlegen
  zeilen_ = zeilenNachVector(data_);

  // es gibt zwei Bl&ouml;cke  BEGIN_DATA / END_DATA und BEGIN_DATA_FORMAT / END_...
  bool im_data_format_block = false;
  bool im_data_block = false;
  int zeile_letztes_BEGIN_DATA = -1;
  int zeile_letztes_BEGIN_DATA_FORMAT = -1;
  int zeile_letztes_NUMBER_OF_SETS = -1;
  zeile_letztes_NUMBER_OF_FIELDS = -1;
  bool fehlendes_NUMBER_OF_SETS = true;
  bool fehlendes_NUMBER_OF_FIELDS = true;
  int zaehler_SETS = 0;
  int zaehler_FIELDS = 0;
  std::stringstream s;
  std::string gtext; // g&uuml;ltiger Text
  bool cmy_daten = false;

  // zeilenweises Bearbeiten
  for (int i = 0; i < (int)zeilen_.size(); ++i)
  {
    // Hole eine kommentarfreie Zeile -> gtext
    gtext = zeilen_[i].substr( 0, zeilen_[i].find( "#" ) );

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
      bool anf_setzen = anfuehrungsstriche_setzen;
      anfuehrungsstriche_setzen = false;
      //int wort_zahl = unterscheideZiffernWorte_(zeilen_[0]).size();
      anfuehrungsstriche_setzen = anf_setzen;
      if( ((ende - pos) != 7 &&
           (ende - pos) != 14 ) )
      {
        zeilen_.insert( zeilen_.begin(), kopf );
        logEintrag_( "Dateisignatur eingefuehrt", 0, "", kopf );
        ++i;
      }
      messungen[messungen.size()-1].kommentare.push_back( zeilen_[0] );
      // Kommentar einfuegen
      if( kommentar.size() ) {
        zeilen_.insert( zeilen_.begin()+1, kommentar );
        messungen[messungen.size()-1].kommentare.push_back( zeilen_[1] );
        ++i;
      }
    }

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

    if( im_data_block &&
        sucheWort (gtext, "END_DATA", 0 ) == std::string::npos )
    {
      // in gtext , durch . ersetzen und danach in zeilen_[i] zur&uuml;ckschreiben
      if(suchenUndErsetzen_ ( gtext, ",", ".", 0 ))
      {
        zeilen_[i].replace( 0, gtext.size(), gtext );
        DBG_NUM_S( ", ersetzt" )
      }

      // SETS z&auml;hlen
      if( gtext.find_first_of( cgats_alnum_ ) != std::string::npos )
      {
        ++zaehler_SETS;
        messungen[messungen.size()-1].block.push_back(  
                                      unterscheideZiffernWorte_( zeilen_[i] ) );
        messungen[messungen.size()-1].block_zeilen = zaehler_SETS;
      }
    }

    // FIELDS z&auml;hlen
    if( im_data_format_block &&
        sucheWort (gtext, "END_DATA_FORMAT", 0 ) == std::string::npos )
    {
      int size = (int)gtext.size();
      zeilen_[i].erase( 0, size );
      zaehler_FIELDS += sucheInDATA_FORMAT_( gtext, i );
      zeilen_[i].insert( 0, gtext );
      // Mitschreiben in felder
      messungen[messungen.size()-1].felder.push_back( 
                     unterscheideZiffernWorte_(zeilen_[i] ) );
      messungen[messungen.size()-1].feld_spalten = zaehler_FIELDS;
      DBG_NUM_S( "zaehler_FIELDS " << zaehler_FIELDS << " Zeile " << i )
    }

    bool block_ende = false;
    
    // DATA_FORMAT Block
    if( sucheWort (gtext, "BEGIN_DATA_FORMAT", 0 ) != std::string::npos )
    {
      DBG_NUM_V( messungen.size() )
      // das Protokol der Feldbezeichner startet
      if( messungen[messungen.size()-1].felder.size() >
          messungen[messungen.size()-1].block.size() )
      {
        logEintrag_( "Mehr Feldbloecke als Datenbloecke: loesche",
                     zeile_letztes_BEGIN_DATA_FORMAT, "", "" );
        messungen[messungen.size()-1].felder.resize(0);
        
      }
      for( unsigned j = 0; j < messungen[messungen.size()-1].felder.size(); ++j)
        DBG_NUM_S( j<<" "<<messungen[messungen.size()-1].felder[j][0] )
      // Markieren und Warnen
      if( im_data_format_block ) {
        DBG_PROG_S( "oops zwei mal BEGIN_DATA_FORMAT  Zeile: " << i )
      } else {
        im_data_format_block = true;
      }

      if( zeile_letztes_NUMBER_OF_FIELDS >= 0)
        fehlendes_NUMBER_OF_FIELDS = false;

      zeile_letztes_BEGIN_DATA_FORMAT = i;
      DBG_NUM_S( "BEGIN_DATA_FORMAT Zeile " << i )
    }

    if( sucheWort (gtext, "END_DATA_FORMAT", 0 ) != std::string::npos )
    {
      for( unsigned j = 0; j < messungen[messungen.size()-1].felder.size(); ++j )
        for( unsigned k = 0; k < messungen[messungen.size()-1].felder[j].size(); ++k )
          DBG_CGATS_S( "["j<<"]["<<k<<"] "<<messungen[messungen.size()-1].felder[j][k] );
      if( !im_data_format_block )
        DBG_PROG_S( "oops END_DATA_FORMAT ohne BEGIN_DATA_FORMAT  Zeile: " << i );

      s.str("");
      s << "NUMBER_OF_FIELDS " << zaehler_FIELDS;
      messungen[messungen.size()-1].feld_spalten = zaehler_FIELDS;
      // NUMBER_OF_FIELDS einf&uuml;gen
      if( im_data_format_block &&
          fehlendes_NUMBER_OF_FIELDS )
      {
        zeilen_.insert( zeilen_.begin() + zeile_letztes_BEGIN_DATA_FORMAT, s.str() );
        ++i;
        logEintrag_( "fehlendes NUMBER_OF_FIELDS eingefuegt",
                     zeile_letztes_BEGIN_DATA_FORMAT, "", s.str() );
        DBG_PROG_S( zeilen_[zeile_letztes_BEGIN_DATA_FORMAT] << " eingefuegt" )
      } else // NUMBER_OF_FIELDS Parsen und Vergleichen
      { DBG_PROG_V( zeile_letztes_NUMBER_OF_FIELDS )
        std::string t = zeilen_[zeile_letztes_NUMBER_OF_FIELDS].
               substr( 0, zeilen_[zeile_letztes_NUMBER_OF_FIELDS].find( "#" ) );
        DBG_CGATS_V( t )
        std::vector<std::string> v = unterscheideZiffernWorte_(t);
        int n_o_;
        if( v.size() >= 2 )
          n_o_ = (int)atof( v[1].c_str() );
        else
          n_o_ = 0;
        if( n_o_ != zaehler_FIELDS)
        {
          // Logbuch
          std::string alt = zeilen_[zeile_letztes_NUMBER_OF_FIELDS];
          // Hole eine kommentarfreie Zeile
          int size = (int)t.size();
          zeilen_[zeile_letztes_NUMBER_OF_FIELDS].erase( 0, size );
          t = s.str();
          zeilen_[zeile_letztes_NUMBER_OF_FIELDS].insert( 0, t );

          s.str("");
          s << "NUMBER_OF_FIELDS stimmt nicht ueberein: gefunden, berechnet: "
            << n_o_ << " <-> "<< zaehler_FIELDS; 
          logEintrag_( s.str(), zeile_letztes_NUMBER_OF_FIELDS,
                       alt, zeilen_[zeile_letztes_NUMBER_OF_FIELDS] );
        }
      }

      im_data_format_block = false;
      block_ende = true;
      zeile_letztes_NUMBER_OF_FIELDS = -1;
      zaehler_FIELDS = 0;
      DBG_NUM_S( "END_DATA_FORMAT Zeile " << i )
    }
    // ENDE DATA_FORMAT Block

    // DATA Block
    if( sucheWort (gtext, "BEGIN_DATA", 0 ) != std::string::npos )
    {
      // Markieren und Warnen
      if( im_data_block )
        logEintrag_( "oops zwei mal BEGIN_DATA  Zeile" , i, "", "" );
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
        DBG_PROG_S( "oops END_DATA ohne BEGIN_DATA  Zeile " << i );

      s.str("");
      s << "NUMBER_OF_SETS " << zaehler_SETS;
      messungen[messungen.size()-1].block_zeilen = zaehler_SETS;
      // NUMBER_OF_SETS einf&uuml;gen
      if( im_data_block &&
          fehlendes_NUMBER_OF_SETS )
      {
        zeilen_.insert( zeilen_.begin() + zeile_letztes_BEGIN_DATA, s.str() );
        ++i;
        DBG_PROG_S( zeilen_[zeile_letztes_BEGIN_DATA] << " eingefuegt" )
        logEintrag_( "fehlende NUMBER_OF_SETS eingefuegt",
                     zeile_letztes_BEGIN_DATA, "", s.str() );
      } else // NUMBER_OF_SETS Parsen und Vergleichen
      {
        std::string t = zeilen_[zeile_letztes_NUMBER_OF_SETS].
                 substr( 0, zeilen_[zeile_letztes_NUMBER_OF_SETS].find( "#" ) );
        DBG_PROG_V( t )
        std::vector<std::string> v = unterscheideZiffernWorte_(t);
        int n_o_;
        if( v.size() >= 2 )
          n_o_ = (int)atof( v[1].c_str() );
        else
          n_o_ = 0;
        if( n_o_ != zaehler_SETS)
        {
          // Logbuch
          std::string alt = zeilen_[zeile_letztes_NUMBER_OF_SETS];
          // Hole eine kommentarfreie Zeile
          int size = (int)t.size();
          zeilen_[zeile_letztes_NUMBER_OF_SETS].erase( 0, size );
          t = s.str();
          zeilen_[zeile_letztes_NUMBER_OF_SETS].insert( 0, t );
          s.str("");
          s << "NUMBER_OF_SETS stimmt nicht ueberein: gefunden, berechnet " << n_o_ << " <-> "<<
                  zaehler_SETS; 
          logEintrag_( s.str(), zeile_letztes_NUMBER_OF_SETS,
                       alt, zeilen_[zeile_letztes_NUMBER_OF_SETS] );
        }
      }


      DBG_NUM_S( "END_DATA Zeile " << i << " mit " << zaehler_SETS << " Messfeldern" )
      im_data_block = false;
      block_ende = true;
      zeile_letztes_NUMBER_OF_SETS = -1;
      zaehler_SETS = 0;
      neuerAbschnitt_();
    }
    // ENDE DATA Block

    // Schl&uuml;sselw&ouml;rter aufschl&uuml;sseln (Keywords)
    if( i != 0 )
    {
      if( !im_data_block && !im_data_format_block &&
          !block_ende )
      {
        int editieren = sucheSchluesselwort_(gtext);
        if( editieren == CMY_DATEN ) 
          cmy_daten = true;
        else if( editieren == CMYK_DATEN )
          cmy_daten = false;

        int i_alt = i;
        if( editieren )
        { // ... zuschl&uuml;sseln (bearbeiten ;) - dabei Zeilendifferenz hinzuz&auml;hlen
          i = i + editZeile_( zeilen_, i, editieren, cmy_daten );
        }
        if(editieren != BELASSEN &&
           editieren != CMY_DATEN && editieren != CMYK_DATEN)
          for(int k = i_alt; k <= i; ++k)
            messungen[messungen.size()-1].kommentare.push_back( zeilen_[k] );
      } else
        if( im_data_block && !im_data_format_block &&
            zeile_letztes_BEGIN_DATA != (int)i )
      {
        if( typ_ == MAX_KORRIGIEREN )
        {
          int size = (int)gtext.size();
          zeilen_[i].erase( 0, size );
          unterscheideZiffernWorte_( gtext );
          zeilen_[i].insert( 0, gtext );
        }
        DBG_CGATS_V( messungen[messungen.size()-1].block.size()-1 )
      }
    }
  }

  // Text neu aus Zeilen zusammenstellen
  data_.clear();
  for( unsigned int i = 0; i < zeilen_.size(); ++i)
  {
    data_.append( zeilen_[i] );
    data_.append( "\n" );
  }

  if(loc_alt)
    doLocked_m( setlocale(LC_NUMERIC,loc_alt) , NULL);

  //DBG_NUM_S (data_)
  DBG_PROG_ENDE
  return data_;
}

void
CgatsFilter::neuerAbschnitt_ ()
{
  DBG_PROG_START
  // alle Abschnitte synchronisieren
  if(messungen[messungen.size()-1]. felder.size() ||
     messungen[messungen.size()-1]. block.size())
    messungen.resize( messungen.size()+1 );
  messungen[messungen.size()-1]. feld_spalten = 0;
  messungen[messungen.size()-1]. block_zeilen = 0;
  DBG_PROG_ENDE
}

unsigned int
CgatsFilter::logEintrag_ (std::string meldung, int zeile_n,
                          std::string eingabe, std::string ausgabe )
{
  DBG_PROG_START
  // Logbuch
  unsigned int l = (unsigned int)log.size();
  log.resize(l+1);
  if( eingabe != "" )
    log[l].eingabe.push_back( eingabe );
  if( ausgabe != "" )
    log[l].ausgabe.push_back( ausgabe );
  log[l].meldung = meldung;
  log[l].original_zeile = zeile_n;
  DBG_PROG_S( zeile_n << ": " << meldung )
  DBG_PROG_ENDE
  return l;
}




std::string
cgats_korrigieren( char* data, size_t size )
{
  DBG_PROG_START
  CgatsFilter cgats;
  cgats.lade( data, size );
  std::string text = cgats.lcms_gefiltert ();
  DBG_PROG_ENDE
  return text;
}

std::string
cgats_max_korrigieren( char* data, size_t size )
{
  DBG_PROG_START
  CgatsFilter cgats;
  // Optionen setzen
  cgats.kommentar = "# Diese Datei wurde von ICC Examin geprueft bzw. neu zusammengestellt \n# bei Unvertraeglichkeiten bitte einen Hinweis\n# moeglichst mit Datenquelle(n), Ergebnis und Versionsnummer\n# an den Author email: ku.b @ gmx.de\n";
  // Laden
  cgats.lade( data, size );
  // Bearbeiten und Ausgeben
  std::string text = cgats.max_korrigieren ();

  if(icc_debug >= 2)
  {
    for( unsigned i = 0; i < cgats.messungen.size(); ++i )
    {
      for( unsigned j = 0; j < cgats.messungen[i]. kommentare.size(); ++j )
        DBG_PROG_S( i<<" "<<j<<" "<<cgats.messungen[i]. kommentare[j] );
      for( unsigned j = 0; j < cgats.messungen[i]. felder.size(); ++j )
        for( unsigned k = 0; k < cgats.messungen[i]. felder[j].size(); ++k )
          DBG_PROG_S( i<<" ["<<j<<"]["<<k<<"] "<<cgats.messungen[i]. felder[j][k] );
      for( unsigned j = 0; j < cgats.messungen[i]. block.size(); ++j )
        for( unsigned k = 0; k < cgats.messungen[i]. block[j].size(); ++k )
          DBG_PROG_S( i<<" ["<<j<<"]["<<k<<"] "<<cgats.messungen[i]. block[j][k] );
      DBG_PROG_V( cgats.messungen[i]. felder.size() <<" "<< cgats.messungen[i]. feld_spalten )
      DBG_PROG_V( cgats.messungen[i]. block.size() <<" "<< cgats.messungen[i]. block_zeilen )
    }
  }

  DBG_PROG_ENDE
  return text;
}

