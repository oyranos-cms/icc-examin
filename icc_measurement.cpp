/*
 * ICC Examin ist eine ICC Profil Betrachter
 * 
 * Copyright (C) 2004  Kai-Uwe Behrmann 
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
 * Qualtit�tspr�fung anhand von Messdaten welche im Profil vorhanden sind.
 * 
 */

// Date:      20. 08. 2004

#if 1
  #ifndef DEBUG
   #define DEBUG
  #endif
  #define DEBUG_ICCMEASUREMENT
#endif

#include <lcms.h> // f�r CGATS lesen
#include "icc_profile.h"
#include "icc_examin.h"
#include "icc_version.h"

#define _(text) text
#define g_message printf


/**
  *  @brief ICCmeasurement Funktionen
  */

ICCmeasurement::ICCmeasurement (ICCprofile* profil, ICCtag &tag)
{ DBG_PROG_START
  ICCmeasurement::load (profil, tag); 
  DBG_PROG_ENDE
}

void
ICCmeasurement::load                ( ICCprofile *profil,
                                      ICCtag&     tag )
{ DBG_PROG_START
  _profil = profil;

  _sig    = tag._sig;
  _size   = tag._size - 8;
  DBG_PROG_V( _size )
  // einfach austauschen
  if (_data != NULL) free (_data); 
  _data = (char*) calloc ( _size , sizeof (char) );
  memcpy ( _data , &(tag._data)[8] , _size );

  DBG_PROG_ENDE
}

void
ICCmeasurement::load                ( ICCprofile *profil,
                                      char       *data,
                                      size_t      size )
{ DBG_PROG_START
  _profil = profil;

  if (_sig != icMaxEnumTag)
    _sig = icSigCharTargetTag;

  _size   = size;
  // enfach austauschen
  if (!_data) free (_data);
  _data = (char*) calloc ( _size , sizeof (char) );
  memcpy ( _data , data , _size );

  DBG_PROG_ENDE
}

void
ICCmeasurement::leseTag (void)
{ DBG_PROG_START
  std::string data = ascii_korrigieren (); DBG_V( (int*)_data )

  // korrigierte CGATS Daten -> _data
  if (_data != NULL) free (_data);
  _data = (char*) calloc (sizeof(char), data.size());
  _size = data.size();
  memcpy (_data, data.c_str(), _size); DBG_V( (int*)_data )

  // lcms liest ein
  lcms_parse();
  DBG_PROG_ENDE
}

void
ICCmeasurement::init (void)
{ DBG_PROG_START DBG_MEM_V( (int*)_data )
  if (valid())
    return;

  if (_profil->hasTagName("targ")) {
    load (_profil, _profil->getTag(_profil->getTagByName("targ")));
    leseTag ();
  }
  else if (_profil->hasTagName("DevD") && (_profil->hasTagName("CIED"))) {
    load (_profil, _profil->getTag(_profil->getTagByName("DevD")));
    leseTag ();
    
    load (_profil, _profil->getTag(_profil->getTagByName("CIED")));
    leseTag ();
  }
  if (_RGB_MessFarben.size() != 0)
    DBG_NUM_V( _RGB_MessFarben.size() )

  if (_profil)
  {
    _channels = _profil->getColourChannelsCount();
    _isMatrix = !(_profil->hasCLUT());
  }

  init_umrechnen();
  DBG_PROG_ENDE
}

std::string
ICCmeasurement::ascii_korrigieren               (void)
{ DBG_PROG_START
  // Reparieren
  // LF FF
  char* ptr = 0; DBG_V( (int*) _data )
  while (strchr(_data, 13) > 0) { // \r 013 0x0d
      ptr = strchr(_data, 13);
      if (ptr > 0) {
        if (*(ptr+1) == '\n')
        {
          *ptr = '\n';
          *(ptr+1) = ' ';
        }
        else
          *ptr = '\n';
      }
  };

  // char* -> std::string
  std::string data (_data, 0, _size);

  // reparieren: Sample_Name , SampleID, ""
  std::string::size_type pos=0;
  std::string::size_type ende;
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
  pos = 0;
  while ((pos = data.find ("\"\"", pos)) != std::string::npos) {
      data.replace (pos, strlen("\"\""), "\""); DBG_NUM_S( "\"\" ersetzt" )
  }
  pos = 0;
  while ((pos = data.find ("Date:", pos)) != std::string::npos) {
      data.replace (pos, strlen("Date:"), "CREATED \"\" #"); DBG_NUM_S( "Date: ersetzt" )
  }
  pos = data.find ("BEGIN_DATA\n", 0); // Kommentarzeilen l�schen
  while ((pos = data.find ("\n#", pos)) != std::string::npos
       && (ende = data.find ("END_DATA\n", pos+2)) != std::string::npos) {
      ende = data.find ("\n", pos+2);
      data.erase (pos, ende - pos); DBG_NUM_S( "Kommentarzeile gel�scht" )
  }
  // fehlendes SAMPLE_ID einf�hren und jeder Zeile einen Z�hler voransetzen
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
    if (data.find ("SPECTRAL_NM", pos) != std::string::npos) count ++;
    if (data.find ("SPECTRAL_PCT", pos) != std::string::npos) count ++;
    if (data.find ("SPECTRAL_DEC", pos) != std::string::npos) count ++;
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
    data.insert (pos, &zahl[0]);
    DBG_NUM_S( "NUMBER_OF_FIELDS " << count << " eingef�gt" )
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
    DBG_S( "NUMBER_OF_SETS " << count << " eingef�gt" )
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
    if (count == 12) // 12 ist l�nger als die erlaubten 7 Zeichen
      diff = count;
    DBG_S (diff)
  }
  if (diff > 10) {
      data.insert (0, "ICCEXAM\n"); DBG_NUM_S( "Beschreibung eingef�hrt" )
  }

  DBG_NUM_S (data)
  DBG_PROG_ENDE
  return data;
}


void
ICCmeasurement::lcms_parse                   (void)
{ DBG_PROG_START
  LCMSHANDLE _lcms_it8 = cmsIT8LoadFromMem ( _data, _size ); DBG_MEM_V( (int*)_data)

  char **SampleNames; DBG_MEM

  // Messfeldanzahl
  if (_nFelder == 0
   || _nFelder == (int)cmsIT8GetPropertyDbl(_lcms_it8, "NUMBER_OF_SETS")) { DBG_NUM
    _nFelder = (int)cmsIT8GetPropertyDbl(_lcms_it8, "NUMBER_OF_SETS"); DBG_NUM
  } else {
    WARN_S( "Messfeldanzahl sollte schon �bereinstimmen! " << _nFelder << "|" << (int)cmsIT8GetPropertyDbl(_lcms_it8, "NUMBER_OF_SETS") )
    clear();
    return;
  }

  int _nKanaele = cmsIT8EnumDataFormat(_lcms_it8, &SampleNames);
  bool _sample_name = false;
  bool _sample_id = false;
  bool _id_vor_name = false;

  // Was ist alles da? Wollen wir sp�ter die Namen tauschen?
  for (int i = 0; i < _nKanaele; i++) {
    if (strstr((char*)SampleNames[i],"SAMPLE_ID") != 0)
      _sample_id = true;
    if (strstr((char*)SampleNames[i],"SAMPLE_NAME") != 0
     && _sample_id) {
      _sample_name = true;
      _id_vor_name = true;
    }
    #ifdef DEBUG_ICCMEASUREMENT
    DBG_NUM_S( (char*)SampleNames[i] << " _sample_name " << _sample_name <<
           " _sample_id" << _sample_id << " _id_vor_name " << _id_vor_name) 
    #endif
  }

  // Auslesen und Aufbereiten
  std::vector<std::string> farbkanaele;
  // m�ssen lokal bleiben !
  bool has_Lab = false;
  bool has_XYZ = false;
  bool has_CMYK = false;
  bool has_RGB = false;
  bool has_xyY = false;
  for (int i = 0; i < _nKanaele; i++) {

    if ((strstr (SampleNames[i], "LAB_L") != 0)
     || (strstr (SampleNames[i], "LAB_A") != 0)
     || (strstr (SampleNames[i], "LAB_B") != 0)) {
      cout << "Lab Daten ";
      has_Lab = true;
      farbkanaele.push_back(SampleNames[i]);
    } else if ((strstr (SampleNames[i], "XYZ_X") != 0)
            || (strstr (SampleNames[i], "XYZ_Y") != 0)
            || (strstr (SampleNames[i], "XYZ_Z") != 0)) {
      cout << "XYZ Daten ";
      has_XYZ = true;
      farbkanaele.push_back(SampleNames[i]);
    } else if ((strstr (SampleNames[i], "CMYK_C") != 0)
            || (strstr (SampleNames[i], "CMYK_M") != 0)
            || (strstr (SampleNames[i], "CMYK_Y") != 0)
            || (strstr (SampleNames[i], "CMYK_K") != 0)) {
      cout << "CMYK Daten ";
      has_CMYK = true;
      farbkanaele.push_back(SampleNames[i]);
    } else if ((strstr (SampleNames[i], "RGB_R") != 0)
            || (strstr (SampleNames[i], "RGB_G") != 0)
            || (strstr (SampleNames[i], "RGB_B") != 0)) {
      cout << "RGB Daten ";
      has_RGB = true;
      farbkanaele.push_back(SampleNames[i]);
    } else if ((strstr (SampleNames[i], "XYY_X") != 0)
            || (strstr (SampleNames[i], "XYY_Y") != 0)
            || (strstr (SampleNames[i], "XYY_CAPY") != 0)) {
      cout << "xyY Daten ";
      has_xyY = true;
      farbkanaele.push_back(SampleNames[i]);
    } else {
      farbkanaele.push_back(SampleNames[i]);
    }

  } DBG_PROG

  // Variablen
  int farben = 0;
  if (has_Lab) farben++;
  if (has_XYZ) {farben++; _XYZ_measurement = true; }
  if (has_RGB) {farben++; _RGB_measurement = true; }
  if (has_CMYK) {farben++; _CMYK_measurement = true; }
  if (has_xyY) farben++;


  // vorl�ufige lcms Farbnamen listen
    _Feldnamen.resize(_nFelder);
    DBG_PROG
    for (int k = 0; k < _nFelder; k++) {
      if (_id_vor_name
       && (getTagName() != "DevD")) {// Name ignorieren
        char *text = (char*) calloc (sizeof(char), 12);
        sprintf (text, "%d", k+1);
        _Feldnamen[k] = text;
        free(text);
      } else {
        const char *constr = cmsIT8GetPatchName (_lcms_it8, k, NULL);
        _Feldnamen[k] = constr;
      }
    }
  DBG_NUM_S (_Feldnamen[0] << " bis " << _Feldnamen[_nFelder-1])

  DBG_NUM_V( has_XYZ << has_RGB << has_CMYK )
 
  // Farben auslesen
  if (has_XYZ) { DBG_PROG // keine Umrechnung n�tig
    _XYZ_Satz.resize(_nFelder);
    for (int i = 0; i < _nFelder; i++) {
        _XYZ_Satz[i].X = cmsIT8GetDataDbl (_lcms_it8, _Feldnamen[i].c_str(),
                                           "XYZ_X") / 100.0;
        _XYZ_Satz[i].Y = cmsIT8GetDataDbl (_lcms_it8, _Feldnamen[i].c_str(),
                                           "XYZ_Y") / 100.0;
        _XYZ_Satz[i].Z = cmsIT8GetDataDbl (_lcms_it8, _Feldnamen[i].c_str(),
                                           "XYZ_Z") / 100.0;
    }
  }
  if (has_RGB) { DBG_PROG // keine Umrechnung n�tig
    _RGB_Satz.resize(_nFelder);
    for (int i = 0; i < _nFelder; i++) {
        _RGB_Satz[i].R = cmsIT8GetDataDbl (_lcms_it8, _Feldnamen[i].c_str(),
                                           "RGB_R") / 255.0;
        _RGB_Satz[i].G = cmsIT8GetDataDbl (_lcms_it8, _Feldnamen[i].c_str(),
                                           "RGB_G") / 255.0;
        _RGB_Satz[i].B = cmsIT8GetDataDbl (_lcms_it8, _Feldnamen[i].c_str(),
                                           "RGB_B") / 255.0;
    }
  }
  if (has_CMYK) { DBG_PROG // keine Umrechnung n�tig
    _CMYK_Satz.resize(_nFelder);
    for (int i = 0; i < _nFelder; i++) {
        _CMYK_Satz[i].C = cmsIT8GetDataDbl (_lcms_it8, _Feldnamen[i].c_str(),
                                           "CMYK_C") /100.0;
        _CMYK_Satz[i].M = cmsIT8GetDataDbl (_lcms_it8, _Feldnamen[i].c_str(),
                                           "CMYK_M") /100.0;
        _CMYK_Satz[i].Y = cmsIT8GetDataDbl (_lcms_it8, _Feldnamen[i].c_str(),
                                           "CMYK_Y") /100.0;
        _CMYK_Satz[i].K = cmsIT8GetDataDbl (_lcms_it8, _Feldnamen[i].c_str(),
                                           "CMYK_K") /100.0;
    } DBG_PROG
  }


  // Farbnamen nach Geschmack (unmittelbar vor cmsIT8Free !)
  if (_id_vor_name) {
    for (int i = 0; i < _nFelder; i++) {
        _Feldnamen[i] = cmsIT8GetData (_lcms_it8, _Feldnamen[i].c_str(),
                                       "SAMPLE_NAME");
    } DBG_NUM_S (_Feldnamen[0] <<" bis "<< _Feldnamen[_nFelder-1] <<" "<< _nFelder)
  }

  // lcms's cgats Leser wird nicht mehr gebraucht
  cmsIT8Free (_lcms_it8);
  _lcms_it8 = NULL;
  DBG_NUM_V( _XYZ_Satz.size() )
  DBG_NUM_V( _RGB_Satz.size() )
  DBG_NUM_V( _CMYK_Satz.size() )
  DBG_PROG_ENDE
}

void
ICCmeasurement::init_umrechnen                     (void)
{ DBG_PROG_START
  _Lab_Differenz_max = -1000.0;
  _Lab_Differenz_min = 1000.0;
  _Lab_Differenz_Durchschnitt = 0.0;
  _DE00_Differenz_max = -1000.0;
  _DE00_Differenz_min = 1000.0;
  _DE00_Differenz_Durchschnitt = 0.0;
  #define PRECALC cmsFLAGS_NOTPRECALC // No memory overhead, VERY
                                      // SLOW ON TRANSFORMING, very fast on creating transform.
                                      // Maximum accurancy.

  double max[3], min[3], WP[3];
  max[0] = max[1] = max[2] = 0;
  min[0] = min[1] = min[2] = 100;
  {
    std::vector<double> wp;
    if (_profil) wp = _profil->getWhitePkt();
    if (wp.size() == 3)
    { for (int i = 0; i < 3; i++)
        WP[i] = wp[i];
    } else
    { WP[0] = X_D50;
      WP[1] = Y_D50;
      WP[2] = Z_D50;
    }
  }
  { int maxFeld=0, minFeld=0;
    const char *maxFN=0, *minFN=0;
    if (_nFelder != (int)_XYZ_Satz.size()) DBG_PROG_S("Messfeldanzahl divergiert")
    for (int i = 0; i < _nFelder; i++)
    { 
      if (max[1] < _XYZ_Satz[i].Y)
      { max[0] = _XYZ_Satz[i].X;
        max[1] = _XYZ_Satz[i].Y;
        max[2] = _XYZ_Satz[i].Z;
        maxFeld = i;
        maxFN = _Feldnamen[i].c_str();
      }
      if (min[1] > _XYZ_Satz[i].Y)
      { min[0] = _XYZ_Satz[i].X;
        min[1] = _XYZ_Satz[i].Y;
        min[2] = _XYZ_Satz[i].Z;
        minFeld = i;
        minFN = _Feldnamen[i].c_str();
      }
    } 
    DBG_PROG_S( maxFN << " Nr. " << maxFeld << endl << " X_max = "<< max[0] <<" Y_max = "<< max[1] <<" Z_max = "<< max[2] )
    DBG_PROG_S( minFN << " Nr. " << minFeld << endl << " X_min = "<< min[0] <<" Y_min = "<< min[1] <<" Z_min = "<< min[2] )
  }


  if ((_RGB_measurement ||
       _CMYK_measurement) && _XYZ_measurement) {// keine Umrechnung n�tig
    cmsHTRANSFORM hCOLOURtoSRGB, hXYZtoSRGB, hCOLOURtoXYZ, hXYZtoLab, hCOLOURtoLab;
    cmsHPROFILE hCOLOUR, hsRGB, hLab, hXYZ;
    hCOLOUR = cmsOpenProfileFromMem (_profil->_data, _profil->_size);
    if (getColorSpaceName(_profil->header.colorSpace()) != "Rgb"
     || getColorSpaceName(_profil->header.colorSpace()) != "Cmyk") {
      WARN_S("unterschiedliche Messdaten und Profilfarbraum ") 
      //this->clear();
      //return;
    }

    hsRGB = cmsCreate_sRGBProfile ();
    hLab = cmsCreateLabProfile (cmsD50_xyY());
    hXYZ = cmsCreateXYZProfile ();
    #if 0
    #define BW_COMP cmsFLAGS_WHITEBLACKCOMPENSATION
    #else
    #define BW_COMP 0
    #endif
    #define TYPE_COLOUR_DBL (COLORSPACE_SH(PT_ANY)|CHANNELS_SH(_channels)|BYTES_SH(0))
    // Wie sieht das Profil die Messfarbe? -> XYZ
    hCOLOURtoXYZ =  cmsCreateTransform (hCOLOUR, TYPE_COLOUR_DBL,
                                    hXYZ, TYPE_XYZ_DBL,
                                    INTENT_ABSOLUTE_COLORIMETRIC,
                                    PRECALC|BW_COMP);
    // Wie sieht das Profil die Messfarbe? -> Lab
    hCOLOURtoLab =  cmsCreateTransform (hCOLOUR, TYPE_COLOUR_DBL,
                                    hLab, TYPE_Lab_DBL,
                                    INTENT_ABSOLUTE_COLORIMETRIC,
                                    PRECALC|BW_COMP);
    // Wie sieht das Messger�t die Messfarbe? -> Lab
    hXYZtoLab =  cmsCreateTransform (hXYZ, TYPE_XYZ_DBL,
                                    hLab, TYPE_Lab_DBL,
                                    INTENT_ABSOLUTE_COLORIMETRIC,
                                    PRECALC|BW_COMP);
    // Wie sieht das Profil die Messfarbe? -> Bildschirmdarstellung
    hCOLOURtoSRGB = cmsCreateTransform (hCOLOUR, TYPE_COLOUR_DBL,
                                    hsRGB, TYPE_RGB_DBL,
                                    INTENT_ABSOLUTE_COLORIMETRIC,
                                    PRECALC|BW_COMP);
    // Wie sieht die CMM die Messfarbe? -> Bildschirmdarstellung
    hXYZtoSRGB = cmsCreateTransform (hXYZ, TYPE_XYZ_DBL,
                                    hsRGB, TYPE_RGB_DBL,
                                    INTENT_ABSOLUTE_COLORIMETRIC,
                                    PRECALC|BW_COMP);
    double Farbe[_channels], RGB[3], XYZ[3], Lab[3];
    bool vcgt = false;
    std::vector<std::vector<double> > vcgt_kurven;
    //TODO
    if (_profil && _profil->hasTagName ("vcgt")) {
      vcgt = true;
      vcgt_kurven = _profil->getTagCurves( _profil->getTagByName("vcgt"),
                                                                ICCtag::MATRIX);
    }

    _RGB_MessFarben.resize(_nFelder);
    _RGB_ProfilFarben.resize(_nFelder);
    _XYZ_Ergebnis.resize(_nFelder);
    _Lab_Satz.resize(_nFelder);
    _Lab_Ergebnis.resize(_nFelder);
    _Lab_Differenz.resize(_nFelder); DBG_V( _Lab_Differenz.size() )
    _DE00_Differenz.resize(_nFelder);
    for (int i = 0; i < _nFelder; i++) {
        if (_isMatrix) {
          // Messfarben auf Weiss und Schwarz addaptiert
          XYZ[0] = (_XYZ_Satz[i].X-min[0])/(max[0]-min[0])*WP[0];
          XYZ[1] = (_XYZ_Satz[i].Y-min[1])/(max[1]-min[1])*WP[1];
          XYZ[2] = (_XYZ_Satz[i].Z-min[2])/(max[2]-min[2])*WP[2];
        } else {
          XYZ[0] = _XYZ_Satz[i].X;
          XYZ[1] = _XYZ_Satz[i].Y;
          XYZ[2] = _XYZ_Satz[i].Z;
        }
        cmsDoTransform (hXYZtoLab, &XYZ[0], &Lab[0], 1);
        _Lab_Satz[i].L = Lab[0];
        _Lab_Satz[i].a = Lab[1];
        _Lab_Satz[i].b = Lab[2];

        cmsDoTransform (hXYZtoSRGB, &XYZ[0], &RGB[0], 1);
        _RGB_MessFarben[i].R = RGB[0];
        _RGB_MessFarben[i].G = RGB[1];
        _RGB_MessFarben[i].B = RGB[2];

        // Profilfarben
        if (_RGB_measurement) {
          //for (int n = 0; n < _channels; n++)
          Farbe[0] = _RGB_Satz[i].R*100.0; DBG_PROG_V( _RGB_Satz[i].R )
          Farbe[1] = _RGB_Satz[i].G*100.0;
          Farbe[2] = _RGB_Satz[i].B*100.0;
        } else {
          Farbe[0] = _CMYK_Satz[i].C*100.0; DBG_PROG_V( _CMYK_Satz[i].C )
          Farbe[1] = _CMYK_Satz[i].M*100.0;
          Farbe[2] = _CMYK_Satz[i].Y*100.0;
          Farbe[3] = _CMYK_Satz[i].K*100.0;
        }

        cmsDoTransform (hCOLOURtoXYZ, &Farbe[0], &XYZ[0], 1);
        _XYZ_Ergebnis[i].X = XYZ[0]; DBG_PROG_V( _XYZ_Ergebnis[i].Y )
        _XYZ_Ergebnis[i].Y = XYZ[1];
        _XYZ_Ergebnis[i].Z = XYZ[2];

        cmsDoTransform (hCOLOURtoLab, &Farbe[0], &Lab[0], 1);
        _Lab_Ergebnis[i].L = Lab[0];
        _Lab_Ergebnis[i].a = Lab[1];
        _Lab_Ergebnis[i].b = Lab[2];
        
        cmsDoTransform (hCOLOURtoSRGB, &Farbe[0], &RGB[0], 1);
        _RGB_ProfilFarben[i].R = RGB[0];
        _RGB_ProfilFarben[i].G = RGB[1];
        _RGB_ProfilFarben[i].B = RGB[2];

        // geometrische Farbortdifferenz - dE CIE*Lab
        _Lab_Differenz[i] = HYP3( _Lab_Ergebnis[i].L - _Lab_Satz[i].L ,
                                  _Lab_Ergebnis[i].a - _Lab_Satz[i].a ,
                                  _Lab_Ergebnis[i].b - _Lab_Satz[i].b  );
        if (_Lab_Differenz_max < _Lab_Differenz[i])
          _Lab_Differenz_max = _Lab_Differenz[i];
        if (_Lab_Differenz_min > _Lab_Differenz[i])
          _Lab_Differenz_min = _Lab_Differenz[i];
        // dE2000
        _DE00_Differenz[i] = cmsCIE2000DeltaE( (cmsCIELab*)&_Lab_Ergebnis[i], (cmsCIELab*)&_Lab_Satz[i] , 1.0, 1.0, 1.0);
        if (_DE00_Differenz_max < _DE00_Differenz[i])
          _DE00_Differenz_max = _DE00_Differenz[i];
        if (_DE00_Differenz_min > _DE00_Differenz[i])
          _DE00_Differenz_min = _DE00_Differenz[i];
    }
    cmsDeleteTransform (hCOLOURtoSRGB);
    cmsDeleteTransform (hXYZtoSRGB);
    cmsDeleteTransform (hCOLOURtoXYZ);
    cmsDeleteTransform (hXYZtoLab);
    cmsDeleteTransform (hCOLOURtoLab);
    cmsCloseProfile (hCOLOUR);
    cmsCloseProfile (hsRGB);
    cmsCloseProfile (hLab);
    cmsCloseProfile (hXYZ);
  }/* else
  if (_CMYK_measurement && _XYZ_measurement) { // keine Umrechnung n�tig
    cmsHTRANSFORM hCMYKtoSRGB, hXYZtoSRGB, hCMYKtoXYZ, hXYZtoLab, hCMYKtoLab;
    cmsHPROFILE hCMYK, hsRGB, hLab, hXYZ;
    hCMYK = cmsOpenProfileFromMem (_profil->_data, _profil->_size);
    if (getColorSpaceName(_profil->header.colorSpace()) != "Cmyk") {
      WARN_S("unterschiedliche Messdaten und Profilfarbraum ")
      //return;
    }

    hsRGB = cmsCreate_sRGBProfile ();
    hLab = cmsCreateLabProfile (cmsD50_xyY());
    hXYZ = cmsCreateXYZProfile ();
    // Wie sieht das Profil die Messfarbe? -> XYZ
    hCMYKtoXYZ =  cmsCreateTransform (hCMYK, TYPE_CMYK_DBL,
                                    hXYZ, TYPE_XYZ_DBL,
                                    INTENT_ABSOLUTE_COLORIMETRIC,
                                    PRECALC|cmsFLAGS_WHITEBLACKCOMPENSATION);
    // Wie sieht das Profil die Messfarbe? -> Lab
    hCMYKtoLab =  cmsCreateTransform (hCMYK, TYPE_CMYK_DBL,
                                    hLab, TYPE_Lab_DBL,
                                    INTENT_ABSOLUTE_COLORIMETRIC,
                                    PRECALC|cmsFLAGS_WHITEBLACKCOMPENSATION);
    // Wie sieht das Messger�t die Messfarbe? -> Lab
    hXYZtoLab =  cmsCreateTransform (hXYZ, TYPE_XYZ_DBL,
                                    hLab, TYPE_Lab_DBL,
                                    INTENT_ABSOLUTE_COLORIMETRIC,
                                    PRECALC|cmsFLAGS_WHITEBLACKCOMPENSATION);
    // Wie sieht das Profil die Messfarbe? -> Bildschirmdarstellung
    hCMYKtoSRGB = cmsCreateTransform (hCMYK, TYPE_CMYK_DBL,
                                    hsRGB, TYPE_RGB_DBL,
                                    INTENT_ABSOLUTE_COLORIMETRIC,
                                    PRECALC|cmsFLAGS_WHITEBLACKCOMPENSATION);
    // Wie sieht die CMM die Messfarbe? -> Bildschirmdarstellung
    hXYZtoSRGB = cmsCreateTransform (hXYZ, TYPE_XYZ_DBL,
                                    hsRGB, TYPE_RGB_DBL,
                                    INTENT_ABSOLUTE_COLORIMETRIC,
                                    PRECALC|cmsFLAGS_WHITEBLACKCOMPENSATION);
    double CMYK[4], sRGB[3], Lab[3], XYZ[3];

    _RGB_MessFarben.resize(_nFelder);
    _RGB_ProfilFarben.resize(_nFelder);
    _XYZ_Ergebnis.resize(_nFelder);
    _Lab_Satz.resize(_nFelder); DBG_V(_nFelder)
    _Lab_Ergebnis.resize(_nFelder);
    _Lab_Differenz.resize(_nFelder);
    _DE00_Differenz.resize(_nFelder);
    for (int i = 0; i < _nFelder; i++) {
        // Messfarben
        XYZ[0] = _XYZ_Satz[i].X;
        XYZ[1] = _XYZ_Satz[i].Y;
        XYZ[2] = _XYZ_Satz[i].Z;
        cmsDoTransform (hXYZtoLab, &XYZ[0], &Lab[0], 1);
        _Lab_Satz[i].L = Lab[0];
        _Lab_Satz[i].a = Lab[1];
        _Lab_Satz[i].b = Lab[2];

        cmsDoTransform (hXYZtoSRGB, &XYZ[0], &sRGB[0], 1);
        _RGB_MessFarben[i].R = sRGB[0];
        _RGB_MessFarben[i].G = sRGB[1];
        _RGB_MessFarben[i].B = sRGB[2];

        // Profilfarben
        CMYK[0] = _CMYK_Satz[i].C*100.0;
        CMYK[1] = _CMYK_Satz[i].M*100.0;
        CMYK[2] = _CMYK_Satz[i].Y*100.0;
        CMYK[3] = _CMYK_Satz[i].K*100.0;
        cmsDoTransform (hCMYKtoXYZ, &CMYK[0], &XYZ[0], 1);
        _XYZ_Ergebnis[i].X = XYZ[0];
        _XYZ_Ergebnis[i].Y = XYZ[1];
        _XYZ_Ergebnis[i].Z = XYZ[2];

        cmsDoTransform (hCMYKtoLab, &CMYK[0], &Lab[0], 1);
        _Lab_Ergebnis[i].L = Lab[0];
        _Lab_Ergebnis[i].a = Lab[1];
        _Lab_Ergebnis[i].b = Lab[2];
        
        cmsDoTransform (hCMYKtoSRGB, &CMYK[0], &sRGB[0], 1);
        _RGB_ProfilFarben[i].R = sRGB[0];
        _RGB_ProfilFarben[i].G = sRGB[1];
        _RGB_ProfilFarben[i].B = sRGB[2];

        // geometrische Farbortdifferenz
        _Lab_Differenz[i] = HYP3( _Lab_Ergebnis[i].L - _Lab_Satz[i].L ,
                                  _Lab_Ergebnis[i].a - _Lab_Satz[i].a ,
                                  _Lab_Ergebnis[i].b - _Lab_Satz[i].b  );
        if (_Lab_Differenz_max < _Lab_Differenz[i])
          _Lab_Differenz_max = _Lab_Differenz[i];
        if (_Lab_Differenz_min > _Lab_Differenz[i])
          _Lab_Differenz_min = _Lab_Differenz[i];
        // dE2000
        _DE00_Differenz[i] = dE2000( _Lab_Ergebnis[i],_Lab_Satz[i] , 1.0, 1.0, 1.0);
        double de00 = cmsCIE2000DeltaE( (cmsCIELab*)&_Lab_Ergebnis[i], (cmsCIELab*)&_Lab_Satz[i] , 1.0, 1.0, 1.0);
        if (fabs(de00 - _DE00_Differenz[i]) > 0.1)
          DBG_V( i << ": " << de00 << "|" << _DE00_Differenz[i] );
        if (_DE00_Differenz_max < _DE00_Differenz[i])
          _DE00_Differenz_max = _DE00_Differenz[i];
        if (_DE00_Differenz_min > _DE00_Differenz[i])
          _DE00_Differenz_min = _DE00_Differenz[i];
    }
    cmsDeleteTransform (hCMYKtoSRGB);
    cmsDeleteTransform (hXYZtoSRGB);
    cmsDeleteTransform (hCMYKtoXYZ);
    cmsDeleteTransform (hXYZtoLab);
    cmsDeleteTransform (hCMYKtoLab);
    cmsCloseProfile (hCMYK);
    cmsCloseProfile (hsRGB);
    cmsCloseProfile (hLab);
    cmsCloseProfile (hXYZ);
  }*/ else
    WARN_S("keine RGB/CMYK und XYZ Messdaten gefunden")
  for (unsigned int i = 0; i < _Lab_Differenz.size(); i++) {
    _Lab_Differenz_Durchschnitt += _Lab_Differenz[i];
  }
  _Lab_Differenz_Durchschnitt /= (double)_Lab_Differenz.size();
  for (unsigned int i = 0; i < _DE00_Differenz.size(); i++) {
    _DE00_Differenz_Durchschnitt += _DE00_Differenz[i];
  }
  _DE00_Differenz_Durchschnitt /= (double)_DE00_Differenz.size();
  DBG_V( _Lab_Satz.size() )
  DBG_V( _Lab_Ergebnis.size() )
  DBG_V( _Lab_Differenz.size() )
  DBG_V( _RGB_Satz.size() )
  DBG_V( _CMYK_Satz.size() )
  DBG_V( _RGB_MessFarben.size() )
  DBG_PROG_ENDE
}

std::string
ICCmeasurement::getHtmlReport                     (void)
{ DBG_PROG_START
  char SF[] = "#cccccc";  // standard Hintergrundfarbe
  char HF[] = "#aaaaaa";  // hervorgehoben
  #define LAYOUTFARBE if (layout[l++] == true) \
                        html << HF; \
                      else \
                        html << SF; //Farbe nach Layoutoption ausw�hlen
  int l = 0;
  std::stringstream html; DBG_NUM_V( _RGB_MessFarben.size() )
  if (_RGB_MessFarben.size() == 0)
    init ();


  if (_reportTabelle.size() == 0)
    _reportTabelle = getText();
  std::vector<int> layout = getLayout(); DBG_PROG

  html << "<!DOCTYPE html PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\">";
  html << "<html><head>" << endl;
  html << "<title>" << _("Report zum Farbprofil") << "</title>\n";
  html << "<meta http-equiv=\"content-type\" content=\"text/html; charset=ISO-8859-1\">" << endl;
  html << "<meta name=\"description\" content=\"icc_examin ICC Profil Report\">\n";
  html << "<meta name=\"author\" content=\"automatic generated by icc_examin-" << ICC_EXAMIN_V << "\">\n";
  html << "</head><body bgcolor=\"" << SF << "\">" << endl << endl;

  int kopf = (int)_reportTabelle.size() - _nFelder;
  int tkopf = 1;
  if (_reportTabelle.size() <= 1)
    tkopf = 0;
  // Allgemeine Informationen
  for (int i = 0; i < kopf - tkopf ; i++) { DBG_S (_nFelder<<"|"<<kopf<<"|"<<i)
    //if (i == 0) html << "<h2>";
    html << _reportTabelle[i][0];
    //if (i == 0) html << "</h2>";
    html <<     "<br>\n\n";
  } DBG_NUM_V( _nFelder )
  if (!_nFelder)
    return html.str();

  html <<       "<table align=left cellpadding=\"2\" cellspacing=\"0\" border=\"0\" width=\"90%\" bgcolor=\"" << SF << "\">\n";
  html <<       "<thead> \n";
  html <<       "  <tr> \n";
  // Tabellenkopf
  int s = 0;           // Spalten
  int f = 0;           // Spalten f�r Farben
  if (_XYZ_Satz.size() && _RGB_MessFarben.size() == _XYZ_Satz.size()) {
    f = 2;
  } DBG_PROG
  l = 0;
  for (s = 0; s < (int)_reportTabelle  [kopf - tkopf].size() + f; s++) {
    if (s < f) {
      if (s == 0) {
        html <<   "    <th width=\"25\">" << _("Messfarbe") << "</th>";
      } else {
        html <<   "    <th width=\"25\">" << _("Profilfarbe") << "</th>";
      }
    } else {
      html <<   "    <th bgcolor=\""; LAYOUTFARBE
      html << "\">" << _reportTabelle [kopf - tkopf][s - f] << "</th>\n";
    }
  } DBG_PROG
  html <<       "  </tr>\n";
  html <<       "</thead>\n<tbody>\n";

  // Messfelder
  #define NACH_HTML(satz,kanal) \
          sprintf (farbe, "%x", (int)(satz[z].kanal*mult+0.5)); \
          if (strlen (farbe) == 1) \
            html << "0"; \
          else if (strstr (farbe, "100") != 0) \
            sprintf (farbe, "ff"); \
          html << farbe;

  char farbe[17];
  double mult = 256.0;
  for (int z = 0; z < _nFelder; z++) {
    html <<     "  <tr>\n";
    l = 0;
    for (s = 0; s < (int)_reportTabelle[kopf - tkopf].size() + f; s++) {
      if (s < f) { // Farbdarstellung
        html << "    <td width=\"20\" bgcolor=\"#"; 
        farbe[0] = 0;
        if (s == 0) {
          NACH_HTML (_RGB_MessFarben, R)
          NACH_HTML (_RGB_MessFarben, G)
          NACH_HTML (_RGB_MessFarben, B)
        } else {
          NACH_HTML (_RGB_ProfilFarben, R)
          NACH_HTML (_RGB_ProfilFarben, G)
          NACH_HTML (_RGB_ProfilFarben, B)
        }
        html << "\"></td>\n";
      } else {
        html << "    <td bgcolor=\""; LAYOUTFARBE
        html << "\">" << _reportTabelle [kopf + z][s - f] << "</td>\n";
      }
    }
    html <<     "  </tr>\n";
  }

  html <<       "</tbody>\n</table>\n\n<br>\n</body></html>\n";
  //DBG_NUM_S(html.str() )
  DBG_PROG_ENDE
  return html.str();
}

std::vector<std::vector<std::string> >
ICCmeasurement::getText                     (void)
{ DBG_PROG_START
  DBG_NUM_V( _RGB_MessFarben.size() )
  if (_RGB_MessFarben.size() == 0)
    init ();

  // push_back ist zu langsam
  std::vector<std::vector<std::string> > tabelle (1);
  std::stringstream s;
  int z = 0; // Zeilen

  tabelle[0].resize(1);
  tabelle[0][0] = _("keine Messdaten verf�gbar");

  if ((_CMYK_measurement || _RGB_measurement)
       && _XYZ_measurement) {
    tabelle.resize(_nFelder+4); // push_back ist zu langsam
    // Tabellen�berschrift
    tabelle[0].resize(1);
    tabelle[0][0] =    _("Mess- und Profilfarben aus <b>"); 
    if (_RGB_measurement)
      tabelle[0][0] += _("RGB");
    else
      tabelle[0][0] += _("CMYK");
    tabelle[0][0] +=   _("</b> Messdaten");
    z++; tabelle[z].resize(1);
    tabelle[z][0] = _("CMM: <b>lcms</b>");
    z++; tabelle[z].resize(1);
    s << _("Abweichungen (dE CIE*Lab) durchschnittlich:<b> ") << _Lab_Differenz_Durchschnitt << _("</b>  maximal: ") << _Lab_Differenz_max << _("  minimal: ") << _Lab_Differenz_min << _("  (dE CIE 2000) durchschnittlich: ") << _DE00_Differenz_Durchschnitt << _("  maximal: ") << _DE00_Differenz_max << _("  minimal: ") << _DE00_Differenz_min;
    tabelle[z][0] = s.str();
    z++;
    // Tabellenkopf
    int spalten, sp = 0, xyz_erg_sp = 0;
    int h = false;
    if (_XYZ_Ergebnis.size() == _XYZ_Satz.size())
      xyz_erg_sp = 3;
    #define HI (h == true) ? h-- : h++ // invertieren
    layout.clear();
    layout.push_back (HI); // Messfeld
    layout.push_back (HI); // dE Lab
    layout.push_back (HI); // dE2000
    layout.push_back (h); layout.push_back (h); layout.push_back (HI); // Lab
    layout.push_back (h); layout.push_back (h); layout.push_back (HI); // Lab'
    layout.push_back (h); layout.push_back (h); layout.push_back (HI); // XYZ
    if (xyz_erg_sp)
      layout.push_back (h); layout.push_back (h); layout.push_back (HI);//XYZ'
    if (_RGB_measurement) {
      layout.push_back (h); layout.push_back (h); layout.push_back (HI);//RGB
    } else {
      layout.push_back (h); layout.push_back (h); layout.push_back (h);
      layout.push_back (HI); // CMYK
    }
    spalten = (int) layout.size();
    tabelle[z].resize( spalten ); DBG_S( tabelle[z].size() )
    tabelle[z][sp++] = _("Messfeld");
    tabelle[z][sp++]=_("dE Lab");
    tabelle[z][sp++]=_("dE2000");
    tabelle[z][sp++]=_("L");
    tabelle[z][sp++]=_("a");
    tabelle[z][sp++]=_("b");
    tabelle[z][sp++]=_("L'");
    tabelle[z][sp++]=_("a'");
    tabelle[z][sp++]=_("b'");
    tabelle[z][sp++]=_("X");
    tabelle[z][sp++]=_("Y");
    tabelle[z][sp++]=_("Z");
    if (_XYZ_Ergebnis.size() == _XYZ_Satz.size()) {
      tabelle[z][sp++] = _("X'");
      tabelle[z][sp++] = _("Y'");
      tabelle[z][sp++] = _("Z'");
    }
    if (_RGB_measurement) {
      tabelle[z][sp++] = _("R");
      tabelle[z][sp++] = _("G");
      tabelle[z][sp++] = _("B");
    } else if (_CMYK_measurement) {
      tabelle[z][sp++] = _("C");
      tabelle[z][sp++] = _("M");
      tabelle[z][sp++] = _("Y");
      tabelle[z][sp++] = _("K");
    } DBG_PROG_V( z <<" "<< _nFelder <<" "<< tabelle.size() )
    z++;
    // Messwerte
    s.str("");
    #define DBG_TAB_V(txt)
    for (int i = 0; i < _nFelder; i++) { 
      sp = 0;
      tabelle[z+i].resize( spalten );
      tabelle[z+i][sp++] =  _Feldnamen[i]; DBG_TAB_V ( z <<" "<< sp <<" "<< _Lab_Differenz.size() )
      s << _Lab_Differenz[i]; tabelle[z+i][sp++] = s.str().c_str(); s.str("");
      s << _DE00_Differenz[i]; tabelle[z+i][sp++] = s.str().c_str(); s.str("");
      s << _Lab_Satz[i].L; tabelle[z+i][sp++] = s.str().c_str(); s.str("");
      s << _Lab_Satz[i].a; tabelle[z+i][sp++] = s.str().c_str(); s.str("");
      s << _Lab_Satz[i].b; tabelle[z+i][sp++] = s.str().c_str(); s.str("");
      s << _Lab_Ergebnis[i].L; tabelle[z+i][sp++] = s.str().c_str(); s.str("");
      s << _Lab_Ergebnis[i].a; tabelle[z+i][sp++] = s.str().c_str(); s.str("");
      s << _Lab_Ergebnis[i].b; tabelle[z+i][sp++] = s.str().c_str(); s.str("");
      s << _XYZ_Satz[i].X*100; tabelle[z+i][sp++] = s.str().c_str(); s.str("");
      s << _XYZ_Satz[i].Y*100; tabelle[z+i][sp++] = s.str().c_str(); s.str("");
      s << _XYZ_Satz[i].Z*100; tabelle[z+i][sp++] = s.str().c_str(); s.str("");
      if (xyz_erg_sp) {
      s << _XYZ_Ergebnis[i].X*100; tabelle[z+i][sp++]=s.str().c_str();s.str("");
      s << _XYZ_Ergebnis[i].Y*100; tabelle[z+i][sp++]=s.str().c_str();s.str("");
      s << _XYZ_Ergebnis[i].Z*100; tabelle[z+i][sp++]=s.str().c_str();s.str("");
      } DBG_TAB_V( i )
      if (_RGB_measurement) {
        s << _RGB_Satz[i].R*255; tabelle[z+i][sp++]= s.str().c_str(); s.str("");
        s << _RGB_Satz[i].G*255; tabelle[z+i][sp++]= s.str().c_str(); s.str("");
        s << _RGB_Satz[i].B*255; tabelle[z+i][sp++]= s.str().c_str(); s.str("");
      } else {
        s << _CMYK_Satz[i].C*100; tabelle[z+i][sp++]=s.str().c_str(); s.str("");
        s << _CMYK_Satz[i].M*100; tabelle[z+i][sp++]=s.str().c_str(); s.str("");
        s << _CMYK_Satz[i].Y*100; tabelle[z+i][sp++]=s.str().c_str(); s.str("");
        s << _CMYK_Satz[i].K*100; tabelle[z+i][sp++]=s.str().c_str(); s.str("");
      }
    }
  }

  DBG_PROG_ENDE
  return tabelle;
}

std::vector<std::string>
ICCmeasurement::getDescription              (void)
{ DBG_PROG_START
  std::vector<std::string> texte;
  std::string text =  "";

  #ifdef DEBUG_ICCMEASUREMENT
  #endif

  DBG_PROG_ENDE
  return texte;
}

std::vector<double>
ICCmeasurement::getMessRGB                  (int patch)
{ DBG_PROG_START
  std::vector<double> punkte(3);

  if (_RGB_MessFarben.size() == 0)
    init ();

  if (patch > _nFelder)
    return punkte;

  punkte[0] = _RGB_MessFarben[patch].R;
  punkte[1] = _RGB_MessFarben[patch].G;
  punkte[2] = _RGB_MessFarben[patch].B;

  DBG_PROG_ENDE
  return punkte;
}

std::vector<double>
ICCmeasurement::getCmmRGB                   (int patch)
{ DBG_PROG_START
  std::vector<double> punkte (3) ;

  if (_RGB_MessFarben.size() == 0)
    init ();

  if (patch > _nFelder)
    return punkte;

  punkte[0] = _RGB_ProfilFarben[patch].R;
  punkte[1] = _RGB_ProfilFarben[patch].G;
  punkte[2] = _RGB_ProfilFarben[patch].B;

  DBG_PROG_ENDE
  return punkte;
}


