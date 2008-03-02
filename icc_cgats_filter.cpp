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

2005-01-22
	* neu: NUMBER_OF_SETS zeilenweise korrigieren <- 7:00
	* neu: NUMBER_OF_FIELDS zeilenweise + Kommentar -> 10:00
2005-01-21
	* und: char Bearbeitung LF FF nach string umstellen <- 9:00
	* neu: zeilenorientierte Kommentarkorrektur -> 14:00
	* neu: sucheWort in icc_helfer.h/cpp <- 15:00
	* : -> 16:30
	* : <- 21:30
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

int
sucheInDATA_FORMAT_( std::string &zeile )
{
  std::string::size_type pos=0;
  std::string::size_type n_zeichen = zeile.size();
  int n = 0;

  // bis Kommentarbeginn
  if( zeile.find ("#", 0) != std::string::npos )
    n_zeichen = zeile.find( "#", 0 );

  while( pos < n_zeichen )
  {
    #define \
    DF_Suche(suche, pos) \
      if (zeile.find (suche, pos) != std::string::npos) \
      { \
        ++n; \
        DBG_NUM_S( pos << zeile[pos] << zeile[pos+1] << zeile[pos+2] << zeile[pos+3]) \
        pos = pos + strlen( suche ); \
      }
    DF_Suche( "SAMPLE_ID", pos )
    DF_Suche( "SAMPLE_NAME", pos )
    DF_Suche( "CMYK_C", pos )
    DF_Suche( "CMYK_M", pos )
    DF_Suche( "CMYK_Y", pos )
    DF_Suche( "CMYK_K", pos )
    DF_Suche( "RGB_R", pos )
    DF_Suche( "RGB_G", pos )
    DF_Suche( "RGB_B", pos )
    DF_Suche( "XYZ_X", pos )
    DF_Suche( "XYZ_Y", pos )
    DF_Suche( "XYZ_Z", pos )
    DF_Suche( "XYY_X", pos )
    DF_Suche( "XYY_Y", pos )
    DF_Suche( "XYY_CAPY", pos )
    DF_Suche( "LAB_L", pos )
    DF_Suche( "LAB_A", pos )
    DF_Suche( "LAB_B", pos )
    DF_Suche( "D_RED", pos )
    DF_Suche( "D_GREEN", pos )
    DF_Suche( "D_BLUE", pos )
    DF_Suche( "D_VIS", pos )
    DF_Suche( "D_MAJOR_FILTER", pos )
    DF_Suche( "SPECTRAL_", pos )
    DF_Suche( "SPECTRAL_PCT", pos )
    DF_Suche( "SPECTRAL_DEC", pos )
    DF_Suche( "nm_", pos )  // nicht standard
    DF_Suche( "SPECTRUM_", pos ) // nicht standard
    DF_Suche( "R_", pos ) // nicht standard
    DF_Suche( "XYY_CAPY", pos )
    DF_Suche( "LAB_C", pos )
    DF_Suche( "LAB_H", pos )
    DF_Suche( "LAB_DE", pos )
    DF_Suche( "LAB_DE_94", pos )
    DF_Suche( "LAB_DE_CMC", pos )
    DF_Suche( "LAB_DE_2000", pos )
    DF_Suche( "MEAN_DE", pos )
    DF_Suche( "STDEV_X", pos )
    DF_Suche( "STDEV_Y", pos )
    DF_Suche( "STDEV_Z", pos )
    DF_Suche( "STDEV_L", pos )
    DF_Suche( "STDEV_A", pos )
    DF_Suche( "STDEV_B", pos )
    DF_Suche( "STDEV_DE", pos )
    DF_Suche( "CHI_SQD_PAR", pos )
    pos++;
  }

  return n;
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

std::string
cgats_korrigieren               (char* _data, size_t _size)
{ DBG_PROG_START
  std::string::size_type pos=0;
  std::string::size_type ende;

  // Reparieren
  // LF CR
  // char* -> std::string
  std::string data (_data, 0, _size);

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
  SUCHENundERSETZEN( "SampleID" , "SAMPLE_ID" )
  SUCHENundERSETZEN( "Sample_ID" , "SAMPLE_ID" )
  SUCHENundERSETZEN( "SampleName" , "SAMPLE_NAME" )
  SUCHENundERSETZEN( "Sample_Name" , "SAMPLE_NAME" )
  SUCHENundERSETZEN( "Lab_L" , "LAB_L" )
  SUCHENundERSETZEN( "Lab_a" , "LAB_A" )
  SUCHENundERSETZEN( "Lab_b" , "LAB_B" )

  // zeilenweise
  std::vector<std::string> zeilen = zeilenNachVector(data);

  // Kommentare / Zahlen
  // es gibt zwei Blöcke  BEGIN_DATA / END_DATA und BEGIN_DATA_FORMAT / END_...
  bool im_data_format_block = false;
  bool im_data_block = false;
  int zeile_letztes_BEGIN_DATA = -1;
  int zeile_letztes_BEGIN_DATA_FORMAT = -1;
  int zeile_letztes_NUMBER_OF_SETS = -1;
  int zeile_letztes_NUMBER_OF_FIELDS = -1;
  bool fehlendes_NUMBER_OF_SETS = true;
  bool fehlendes_NUMBER_OF_FIELDS = true;
  int zaehler_SETS = -1; //TODO
  int zaehler_FIELDS = 0; //TODO
  std::stringstream s;

  // zeilenweises Bearbeiten
  for (unsigned i = 0; i < zeilen.size(); ++i)
  {
    // NUMBER_OF_FIELDS Position merken
    if( sucheWort (zeilen[i], "NUMBER_OF_FIELDS", 0 ) != std::string::npos )
    {
      zeile_letztes_NUMBER_OF_FIELDS = i;
      DBG_NUM_S( "NUMBER_OF_FIELDS Zeile " << i )
    }
    // NUMBER_OF_SETS Position merken
    if( sucheWort (zeilen[i], "NUMBER_OF_SETS", 0 ) != std::string::npos )
    {
      zeile_letztes_NUMBER_OF_SETS = i;
      DBG_NUM_S( "NUMBER_OF_SETS Zeile " << i )
    }
    // SETS zählen
    if( im_data_block )
    { // TODO Kommentare
      ++zaehler_SETS;
    }
    // FIELDS zählen
    if( im_data_format_block )
    {
      zaehler_FIELDS += sucheInDATA_FORMAT_( zeilen[i] );
      DBG_NUM_S( "zaehler_FIELDS " << zaehler_FIELDS << " Zeile " << i )
    }
    
    // DATA Block
    if( (pos = sucheWort (zeilen[i], "BEGIN_DATA", 0 )) != std::string::npos )
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

    if( (pos = sucheWort (zeilen[i], "END_DATA", 0 )) != std::string::npos )
    {
      if( !im_data_block )
        WARN_S( "oops END_DATA ohne BEGIN_DATA  Zeile " << i )

      // NUMBER_OF_SETS einfügen
      if( im_data_block && fehlendes_NUMBER_OF_SETS )
      {
        s.str("");
        // den neuen Text bauen
        s << "NUMBER_OF_SETS " << zaehler_SETS;
        // neue Zeile Einfügen
        zeileEinfuegen_( zeilen, zeile_letztes_BEGIN_DATA, s.str() );

        ++i;
        DBG_S( zeilen[zeile_letztes_BEGIN_DATA] << " eingefügt" )
      }

      im_data_format_block = false;
      zeile_letztes_NUMBER_OF_SETS = -1;
      zaehler_SETS = -1;
      DBG_NUM_S( "END_DATA Zeile " << i )
    }
    // ENDE DATA Block

    // DATA_FORMAT Block
    if( (pos = sucheWort (zeilen[i], "BEGIN_DATA_FORMAT", 0 )) != std::string::npos )
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

    if( (pos = sucheWort (zeilen[i], "END_DATA_FORMAT", 0 )) != std::string::npos )
    {
      if( !im_data_format_block )
        WARN_S( "oops END_DATA_FORMAT ohne BEGIN_DATA_FORMAT  Zeile " << i )

      // NUMBER_OF_FIELDS einfügen
      if( im_data_format_block && fehlendes_NUMBER_OF_FIELDS )
      {
        s.str("");
        // den neuen Text bauen
        s << "NUMBER_OF_FIELDS " << zaehler_FIELDS;
        // neue Zeile Einfügen
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

#if 0
    // TODO
    if( im_data_block &&
        sucheWort (zeilen[i], "END_DATA", 0 ) == std::string::npos )
      // Zählen
      if( keine Kommentarzeile )
        ++count;

    if( !im_data_block && !im_data_format_block )
      Schlüsselwörter suchen + Zeile danach in Anführungszeichen setzen bis Kommentar
    else
      if( keine Leerzeile )
        Zeile als Kommentar setzen

#endif
  }

  // Schlüsselwörter
  pos = 0;
  while ((pos = data.find ("\"\"", pos)) != std::string::npos) {
      data.replace (pos, strlen("\"\""), "\""); DBG_NUM_S( "\"\" ersetzt" )
  }
  pos = 0;
  while ((pos = data.find ("Date:", pos)) != std::string::npos) {
      data.replace (pos, strlen("Date:"), "CREATED \"\" #"); DBG_NUM_S( "Date: ersetzt" )
  }

  #if 0
  // Kommentarzeilen löschen
  pos = data.find ("BEGIN_DATA\n", 0);
  while ((pos = data.find ("\n#", pos)) != std::string::npos
       && (ende = data.find ("END_DATA\n", pos+2)) != std::string::npos) {
      ende = data.find ("\n", pos+2);
      data.erase (pos, ende - pos); DBG_NUM_S( "Kommentarzeile gelöscht" )
  }
  #endif

  // fehlendes SAMPLE_ID einführen und jeder Zeile einen Zähler voransetzen
  int count;
  if ((data.find ("SAMPLE_ID", 0)) == std::string::npos) {
    pos = data.find ("BEGIN_DATA_FORMAT\n", 0);
    data.insert (pos+strlen("BEGIN_DATA_FORMAT\n"), "SAMPLE_ID   ");
    pos = data.find ("BEGIN_DATA\n", 0); DBG_NUM_V(pos)
    count = 1; pos++;
    while ((pos = data.find ("\n", pos)) != std::string::npos
         && (ende = data.find ("END_DATA\n", pos+2)) != std::string::npos) {
      static char zahl[12];
      sprintf (&zahl[0], "%d   ", count); count++; //DBG_S(count << " " << pos)
      data.insert (pos+1, &zahl[0]);
      pos += strlen (&zahl[0]);
      //DBG_NUM_S( data )
    }
  }
  // NUMBER_OF_FIELDS reparieren
  if ((data.find ("NUMBER_OF_FIELDS", 0)) == std::string::npos)
  {
    pos = data.find ("BEGIN_DATA_FORMAT\n", 0); DBG_NUM_S (pos)
    count = 0; pos++;
    if (data.find ("SAMPLE_ID", pos) != std::string::npos) count ++;
    if (data.find ("SAMPLE_NAME", pos) != std::string::npos) count ++;
    if (data.find ("CMYK_C", pos) != std::string::npos) count ++;
    if (data.find ("CMYK_M", pos) != std::string::npos) count ++;
    if (data.find ("CMYK_Y", pos) != std::string::npos) count ++;
    if (data.find ("CMYK_K", pos) != std::string::npos) count ++;
    if (data.find ("RGB_R", pos) != std::string::npos) count ++;
    if (data.find ("RGB_G", pos) != std::string::npos) count ++;
    if (data.find ("RGB_B", pos) != std::string::npos) count ++;
    if (data.find ("XYZ_X", pos) != std::string::npos) count ++;
    if (data.find ("XYZ_Y", pos) != std::string::npos) count ++;
    if (data.find ("XYZ_Z", pos) != std::string::npos) count ++;
    if (data.find ("XYY_X", pos) != std::string::npos) count ++;
    if (data.find ("XYY_Y", pos) != std::string::npos) count ++;
    if (data.find ("XYY_CAPY", pos) != std::string::npos) count ++;
    if (data.find ("LAB_L", pos) != std::string::npos) count ++;
    if (data.find ("LAB_A", pos) != std::string::npos) count ++;
    if (data.find ("LAB_B", pos) != std::string::npos) count ++;
    if (data.find ("D_RED", pos) != std::string::npos) count ++;
    if (data.find ("D_GREEN", pos) != std::string::npos) count ++;
    if (data.find ("D_BLUE", pos) != std::string::npos) count ++;
    if (data.find ("D_VIS", pos) != std::string::npos) count ++;
    if (data.find ("D_MAJOR_FILTER", pos) != std::string::npos) count ++;
    if (data.find ("SPECTRAL_", pos) != std::string::npos) count ++;
    if (data.find ("SPECTRAL_PCT", pos) != std::string::npos) count ++;
    if (data.find ("SPECTRAL_DEC", pos) != std::string::npos) count ++;
    if (data.find ("nm_", pos) != std::string::npos) count ++; // nicht standard
    if (data.find ("SPECTRUM_", pos) != std::string::npos) count ++;
    if (data.find ("R_", pos) != std::string::npos) count ++;  // \ !standard
    if (data.find ("XYY_CAPY", pos) != std::string::npos) count ++;
    if (data.find ("LAB_C", pos) != std::string::npos) count ++;
    if (data.find ("LAB_H", pos) != std::string::npos) count ++;
    if (data.find ("LAB_DE", pos) != std::string::npos) count ++;
    if (data.find ("LAB_DE_94", pos) != std::string::npos) count ++;
    if (data.find ("LAB_DE_CMC", pos) != std::string::npos) count ++;
    if (data.find ("LAB_DE_2000", pos) != std::string::npos) count ++;
    if (data.find ("MEAN_DE", pos) != std::string::npos) count ++;
    if (data.find ("STDEV_X", pos) != std::string::npos) count ++;
    if (data.find ("STDEV_Y", pos) != std::string::npos) count ++;
    if (data.find ("STDEV_Z", pos) != std::string::npos) count ++;
    if (data.find ("STDEV_L", pos) != std::string::npos) count ++;
    if (data.find ("STDEV_A", pos) != std::string::npos) count ++;
    if (data.find ("STDEV_B", pos) != std::string::npos) count ++;
    if (data.find ("STDEV_DE", pos) != std::string::npos) count ++;
    if (data.find ("CHI_SQD_PAR", pos) != std::string::npos) count ++;
    static char zahl[64];
    pos = data.find ("BEGIN_DATA_FORMAT\n", 0);
    sprintf (&zahl[0], "NUMBER_OF_FIELDS %d\n", count); DBG_PROG_V( zahl )
    DBG_PROG_V( pos <<" "<< data.size() <<" "<< data.length() <<" "<< (int)data.npos)
    data.insert (pos, &zahl[0]);
    DBG_NUM_S( "NUMBER_OF_FIELDS " << count << " eingefügt" )
  }
  // Signatur reparieren
  pos = 0;
  int pos_alt = 0, diff = 0;
  count = 0;
  while (diff <= 1) {
    count ++;
    pos = data.find ("\n", pos);
    diff = pos - pos_alt;
    pos_alt = pos;
    if (count == 12) // 12 ist länger als die erlaubten 7 Zeichen
      diff = count;
    DBG_S (diff)
  }
  if (diff > 10) {
      data.insert (0, "ICCEXAM\n"); DBG_NUM_S( "Beschreibung eingeführt" )
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


