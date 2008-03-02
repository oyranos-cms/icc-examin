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

2005-01-11
	* start: 6:00
	* neu: Kopie des vorhandenen Textes und Test <- 6:30
	* Brief schreiben <- 7:00
	* 

2005-01-21
	* und: char Bearbeitung LF FF nach string umstellen <- 9:00
	* neu: zeilenorientierte Kommentarkorrektur -> 14:00
	* neu: sucheWort <- 15:00
	* : -> 16:30
	* : <- 21:30
 */

#include "icc_utils.h"
#include "icc_helfer.h"

#include <fstream>

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
  // testweises Speichern
  #if 0
  std::ofstream f ( "AtestCGATS.vim",  std::ios::out );
  f.write ( data.c_str(), data.size() );
  f.close();
  #endif


  // reparieren: Sample_Name , SampleID, ""
  pos = 0;
  while ((pos = data.find ("SampleID", pos)) != std::string::npos) {
      data.replace (pos, 8, "SAMPLE_ID"); DBG_NUM_S( "SampleID ersetzt" )
  }
  pos = 0;
  while ((pos = data.find ("Sample_ID", pos)) != std::string::npos) {
      data.replace (pos, strlen("Sample_ID"), "SAMPLE_NAME"); DBG_NUM_S( "Sample_ID ersetzt" )
  }
  pos = 0;
  while ((pos = data.find ("SampleName", pos)) != std::string::npos) {
      data.replace (pos, strlen("SampleName"), "SAMPLE_NAME"); DBG_NUM_S( "SampleName ersetzt" )
  }
  pos = 0;
  while ((pos = data.find ("Sample_Name", pos)) != std::string::npos) {
      data.replace (pos, strlen("Sample_Name"), "SAMPLE_NAME"); DBG_NUM_S( "Sample_Name ersetzt" )
  }
  pos = 0;
  while ((pos = data.find ("Lab_L", pos)) != std::string::npos) {
      data.replace (pos, strlen("Lab_L"), "LAB_L"); DBG_NUM_S( "Lab_L ersetzt" )
  }
  pos = 0;
  while ((pos = data.find ("Lab_a", pos)) != std::string::npos) {
      data.replace (pos, strlen("Lab_a"), "LAB_A"); DBG_NUM_S( "Lab_a ersetzt" )
  }
  pos = 0;
  while ((pos = data.find ("Lab_b", pos)) != std::string::npos) {
      data.replace (pos, strlen("Lab_b"), "LAB_B"); DBG_NUM_S( "Lab_b ersetzt" )
  }
  // Kommentare
  // zeilenweise
  std::vector<std::string> zeilen = zeilenNachVector(data);
  // es gibt zwei Blöcke  BEGIN_DATA / END_DATA und BEGIN_DATA_FORMAT / END_...
  bool im_data_format_block = false;
  bool im_data_block = false;
  for (unsigned i = 0; i < zeilen.size(); ++i)
  {
    if( (pos = sucheWort (zeilen[i], "BEGIN_DATA", 0 )) != std::string::npos )
    {
      if( im_data_block )
        WARN_S( "oops zwei mal BEGIN_DATA  Zeile " << i )
      else
        im_data_block = true;

      DBG_NUM_S( "BEGIN_DATA Zeile " << i )
    }

    if( (pos = sucheWort (zeilen[i], "END_DATA", 0 )) != std::string::npos )
    {
      if( !im_data_block )
        WARN_S( "oops END_DATA ohne BEGIN_DATA  Zeile " << i )

      im_data_block = false;
      DBG_NUM_S( "END_DATA Zeile " << i )
    }

    if( (pos = sucheWort (zeilen[i], "BEGIN_DATA_FORMAT", 0 )) != std::string::npos )
    {
      if( im_data_format_block )
        WARN_S( "oops zwei mal BEGIN_DATA_FORMAT  Zeile " << i )
      else
        im_data_format_block = true;

      DBG_NUM_S( "BEGIN_DATA_FORMAT Zeile " << i )
    }

    if( (pos = sucheWort (zeilen[i], "END_DATA_FORMAT", 0 )) != std::string::npos )
    {
      if( !im_data_format_block )
        WARN_S( "oops END_DATA_FORMAT ohne BEGIN_DATA_FORMAT  Zeile " << i )

      im_data_format_block = false;
      DBG_NUM_S( "END_DATA_FORMAT Zeile " << i )
    }

    if( im_data_block )
      // Zählen
      ++count;




  }


  pos = 0;
  while ((pos = data.find ("\"\"", pos)) != std::string::npos) {
      data.replace (pos, strlen("\"\""), "\""); DBG_NUM_S( "\"\" ersetzt" )
  }
  pos = 0;
  while ((pos = data.find ("Date:", pos)) != std::string::npos) {
      data.replace (pos, strlen("Date:"), "CREATED \"\" #"); DBG_NUM_S( "Date: ersetzt" )
  }
  pos = data.find ("BEGIN_DATA\n", 0); // Kommentarzeilen löschen
  while ((pos = data.find ("\n#", pos)) != std::string::npos
       && (ende = data.find ("END_DATA\n", pos+2)) != std::string::npos) {
      ende = data.find ("\n", pos+2);
      data.erase (pos, ende - pos); DBG_NUM_S( "Kommentarzeile gelöscht" )
  }
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
  // NUMBER_OF_SETS reparieren
  if ((data.find ("NUMBER_OF_SETS", 0)) == std::string::npos) {
    pos = data.find ("BEGIN_DATA\n", 0); DBG_S (pos)
    count = 0; pos++;
    while ((pos = data.find ("\n", pos)) != std::string::npos
         && (ende = data.find ("END_DATA\n", pos+2)) != std::string::npos) {
      count ++; //DBG_S( pos << " " << count)
      pos++;
    }
    static char zahl[64];
    pos = data.find ("BEGIN_DATA\n", 0);
    sprintf (&zahl[0], "NUMBER_OF_SETS %d\n", count);
    data.insert (pos, &zahl[0]);
    DBG_S( "NUMBER_OF_SETS " << count << " eingefügt" )
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

  //DBG_NUM_S (data)
  DBG_PROG_ENDE
  return data;
}


