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
 * Qualtitätsprüfung anhand von Messdaten welche im Profil vorhanden sind.
 * 
 */

// Date:      20. 08. 2004

#if 0
  #ifndef DEBUG
   #define DEBUG
  #endif
  #define DEBUG_ICCMEASUREMENT
  #define DBG_MESS_START DBG_PROG_START
  #define DBG_MESS_ENDE DBG_PROG_ENDE
  #define DBG_MESS_V(t) DBG_NUM_V(t)
#else
  #define DBG_MESS_START
  #define DBG_MESS_ENDE
  #define DBG_MESS_V(t)
#endif


#include <lcms.h> // für CGATS lesen
#include "icc_profile.h"
#include "icc_examin.h"
#include "icc_version.h"
#include "icc_helfer.h"

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
  // einfach austauschen
  if (!_data) free (_data);
  _data = (char*) calloc ( _size , sizeof (char) );
  memcpy ( _data , data , _size );

  DBG_PROG_ENDE
}

void
ICCmeasurement::leseTag (void)
{ DBG_PROG_START
  CgatsFilter cgats;
  cgats.lade( _data, _size );
  std::string data = cgats.lcms_gefiltert (); DBG_V( (int*)_data )

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
    WARN_S( "Messfeldanzahl sollte schon übereinstimmen! " << _nFelder << "|" << (int)cmsIT8GetPropertyDbl(_lcms_it8, "NUMBER_OF_SETS") )
    clear();
    return;
  }

  int _nKanaele = cmsIT8EnumDataFormat(_lcms_it8, &SampleNames);
  bool _sample_name = false;
  bool _sample_id = false;
  bool _id_vor_name = false;

  // Was ist alles da? Wollen wir später die Namen tauschen?
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
  // müssen lokal bleiben !
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


  // vorläufige lcms Farbnamen listen
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
  if (has_XYZ) { DBG_PROG // keine Umrechnung nötig
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
  if (has_RGB) { DBG_PROG // keine Umrechnung nötig
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
  if (has_CMYK) { DBG_PROG // keine Umrechnung nötig
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
    int m = _nFelder < (int)_XYZ_Satz.size() ? _nFelder : (int)_XYZ_Satz.size();
    DBG_PROG_S( "Felder: " << m )
    for (int i = 0; i < m; i++)
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
    if( maxFN )
      DBG_PROG_S( maxFN << " Nr. " << maxFeld << endl << " X_max = "<< max[0] <<" Y_max = "<< max[1] <<" Z_max = "<< max[2] )
    if( minFN )
    DBG_PROG_S( minFN << " Nr. " << minFeld << endl << " X_min = "<< min[0] <<" Y_min = "<< min[1] <<" Z_min = "<< min[2] )
  }


  if ((_RGB_measurement ||
       _CMYK_measurement) || _XYZ_measurement)
  {
    cmsHTRANSFORM hCOLOURtoRGB, hXYZtoRGB, hCOLOURtoXYZ, hXYZtoLab,hCOLOURtoLab;
    cmsHPROFILE hCOLOUR, hsRGB, hLab, hXYZ;


    if (getColorSpaceName(_profil->header.colorSpace()) != "Rgb"
     || getColorSpaceName(_profil->header.colorSpace()) != "Cmyk")
    {
      WARN_S("unterschiedliche Messdaten und Profilfarbraum ")
      DBG_PROG_V( getColorSpaceName(_profil->header.colorSpace()) )
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
    if ((_RGB_measurement ||
         _CMYK_measurement))
    {
      if( _profil->size() )
        hCOLOUR = cmsOpenProfileFromMem (_profil->_data, _profil->_size);
      else { // Alternative
        size_t groesse = 0;
        char* block = 0;
        if( _CMYK_measurement )
          block = oyranos.cmyk(groesse);
        else
        if( _RGB_measurement )
          block = oyranos.rgb(groesse);
        DBG_PROG_V( groesse )

        hCOLOUR = cmsOpenProfileFromMem (block, groesse);
      }
      if( !hCOLOUR )
        WARN_S(_("hCOLOUR ist leer"))

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
      // Wie sieht das Profil die Messfarbe? -> Bildschirmdarstellung
      hCOLOURtoRGB = cmsCreateTransform (hCOLOUR, TYPE_COLOUR_DBL,
                                    hsRGB, TYPE_RGB_DBL,
                                    INTENT_ABSOLUTE_COLORIMETRIC,
                                    PRECALC|BW_COMP);
    }
    if (_XYZ_measurement) {
      if( !hsRGB )
        WARN_S(_("hsRGB ist leer"))

      // Wie sieht das Messgerät die Messfarbe? -> Lab
      hXYZtoLab = cmsCreateTransform (hXYZ, TYPE_XYZ_DBL,
                                    hLab, TYPE_Lab_DBL,
                                    INTENT_ABSOLUTE_COLORIMETRIC,
                                    PRECALC|BW_COMP);
      // Wie sieht die CMM die Messfarbe? -> Bildschirmdarstellung
      hXYZtoRGB = cmsCreateTransform (hXYZ, TYPE_XYZ_DBL,
                                    hsRGB, TYPE_RGB_DBL,
                                    INTENT_ABSOLUTE_COLORIMETRIC,
                                    PRECALC|BW_COMP);
    }
    double Farbe[_channels], RGB[3], XYZ[3], Lab[3];
    bool vcgt = false;
    std::vector<std::vector<double> > vcgt_kurven;
    //TODO
    if (_profil && _profil->hasTagName ("vcgt")) {
      vcgt = true;
      vcgt_kurven = _profil->getTagCurves( _profil->getTagByName("vcgt"),
                                                                ICCtag::MATRIX);
    }

    if (_XYZ_measurement)
    {
      _RGB_MessFarben.resize(_nFelder);
      _RGB_ProfilFarben.resize(_nFelder);
      _XYZ_Ergebnis.resize(_nFelder);
      _Lab_Satz.resize(_nFelder);
      _Lab_Ergebnis.resize(_nFelder);
      _Lab_Differenz.resize(_nFelder); DBG_V( _Lab_Differenz.size() )
      _DE00_Differenz.resize(_nFelder);

      if( (int)_XYZ_Satz.size() != _nFelder )
        WARN_S(_("_XYZ_Satz.size() und _nFelder sind ungleich"))
      if( (int)_RGB_Satz.size() != _nFelder )
        WARN_S(_("_RGB_Satz.size() und _nFelder sind ungleich"))
      if( (int)_CMYK_Satz.size() != _nFelder )
        WARN_S(_("_CMYK_Satz.size() und _nFelder sind ungleich"))
      for (int i = 0; i < _nFelder; i++)
      {
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
        _Lab_Satz[i].L = Lab[0]; DBG_MESS_V( _Lab_Satz[i].L )
        _Lab_Satz[i].a = Lab[1]; DBG_MESS_V( _Lab_Satz[i].a )
        _Lab_Satz[i].b = Lab[2]; DBG_MESS_V( _Lab_Satz[i].b )

        cmsDoTransform (hXYZtoRGB, &XYZ[0], &RGB[0], 1);
        _RGB_MessFarben[i].R = RGB[0]; DBG_MESS_V( _RGB_MessFarben[i].R )
        _RGB_MessFarben[i].G = RGB[1]; DBG_MESS_V( _RGB_MessFarben[i].G )
        _RGB_MessFarben[i].B = RGB[2]; DBG_MESS_V( _RGB_MessFarben[i].B )

        
        if ((_RGB_measurement ||
             _CMYK_measurement))
        {

          // Profilfarben
          if (_RGB_measurement) {
            //for (int n = 0; n < _channels; n++)
            Farbe[0] = _RGB_Satz[i].R*100.0; DBG_MESS_V( _RGB_Satz[i].R )
            Farbe[1] = _RGB_Satz[i].G*100.0;
            Farbe[2] = _RGB_Satz[i].B*100.0;
          } else {
            Farbe[0] = _CMYK_Satz[i].C*100.0; DBG_MESS_V( _CMYK_Satz[i].C )
            Farbe[1] = _CMYK_Satz[i].M*100.0;
            Farbe[2] = _CMYK_Satz[i].Y*100.0;
            Farbe[3] = _CMYK_Satz[i].K*100.0;
          }

          cmsDoTransform (hCOLOURtoXYZ, &Farbe[0], &XYZ[0], 1);
          _XYZ_Ergebnis[i].X = XYZ[0]; DBG_MESS_V( _XYZ_Ergebnis[i].X )
          _XYZ_Ergebnis[i].Y = XYZ[1]; DBG_MESS_V( _XYZ_Ergebnis[i].Y )
          _XYZ_Ergebnis[i].Z = XYZ[2]; DBG_MESS_V( _XYZ_Ergebnis[i].Z )

          cmsDoTransform (hCOLOURtoLab, &Farbe[0], &Lab[0], 1);
          _Lab_Ergebnis[i].L = Lab[0]; DBG_MESS_V( _Lab_Ergebnis[i].L )
          _Lab_Ergebnis[i].a = Lab[1]; DBG_MESS_V( _Lab_Ergebnis[i].a )
          _Lab_Ergebnis[i].b = Lab[2]; DBG_MESS_V( _Lab_Ergebnis[i].b )
        
          cmsDoTransform (hCOLOURtoRGB, &Farbe[0], &RGB[0], 1);
          _RGB_ProfilFarben[i].R = RGB[0]; DBG_MESS_V( _RGB_ProfilFarben[i].R )
          _RGB_ProfilFarben[i].G = RGB[1]; DBG_MESS_V( _RGB_ProfilFarben[i].G )
          _RGB_ProfilFarben[i].B = RGB[2]; DBG_MESS_V( _RGB_ProfilFarben[i].B )

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
      }
    }
    if (_XYZ_measurement) {
      cmsDeleteTransform (hXYZtoRGB);
      cmsDeleteTransform (hXYZtoLab);
    }
    if ((_RGB_measurement ||
         _CMYK_measurement)) {
      cmsDeleteTransform (hCOLOURtoXYZ);
      cmsDeleteTransform (hCOLOURtoLab);
      cmsDeleteTransform (hCOLOURtoRGB);
      cmsCloseProfile (hCOLOUR);
    }
    cmsCloseProfile (hsRGB);
    cmsCloseProfile (hLab);
    cmsCloseProfile (hXYZ);
  } else
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
                        html << SF; //Farbe nach Layoutoption auswählen
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
  int f = 0;           // Spalten für Farben
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
  tabelle[0][0] = _("keine Messdaten verfügbar");

  if ((_CMYK_measurement || _RGB_measurement)
       && _XYZ_measurement) {
    tabelle.resize(_nFelder+4); // push_back ist zu langsam
    // Tabellenüberschrift
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
{ DBG_MESS_START
  std::vector<double> punkte(3);

  if (_RGB_MessFarben.size() == 0)
    init ();

  if (patch > _nFelder) {
    WARN_S( "Patch Nr: " << patch << " ausserhalb des Messfarbsatzes" )
    DBG_MESS_ENDE
    return punkte;
  }

  punkte[0] = _RGB_MessFarben[patch].R; DBG_MESS_V( _RGB_MessFarben[patch].R <<  punkte[0] )
  punkte[1] = _RGB_MessFarben[patch].G; DBG_MESS_V( _RGB_MessFarben[patch].G )
  punkte[2] = _RGB_MessFarben[patch].B; DBG_MESS_V( _RGB_MessFarben[patch].B )

  DBG_MESS_ENDE
  return punkte;
}

std::vector<double>
ICCmeasurement::getCmmRGB                   (int patch)
{ DBG_MESS_START
  std::vector<double> punkte (3) ;

  if (_RGB_MessFarben.size() == 0)
    init ();

  if (patch > _nFelder) {
    WARN_S( "Patch Nr: " << patch << " ausserhalb des Messfarbsatzes" )
    DBG_MESS_ENDE
    return punkte;
  }

  punkte[0] = _RGB_ProfilFarben[patch].R;
  punkte[1] = _RGB_ProfilFarben[patch].G;
  punkte[2] = _RGB_ProfilFarben[patch].B;

  DBG_MESS_ENDE
  return punkte;
}

std::vector<double>
ICCmeasurement::getMessLab                  (int patch)
{ DBG_MESS_START
  std::vector<double> punkte (3) ;

  if (_Lab_Satz.size() == 0)
    init ();

  if (patch > _nFelder) {
    WARN_S( "Patch Nr: " << patch << " ausserhalb des Messfarbsatzes" )
    DBG_MESS_ENDE
    return punkte;
  }

  punkte[0] =  _Lab_Satz[patch].L          / 100.0;
  punkte[1] = (_Lab_Satz[patch].a + 128.0) / 255.0;
  punkte[2] = (_Lab_Satz[patch].b + 128.0) / 255.0;

  DBG_MESS_ENDE
  return punkte;
}

std::vector<double>
ICCmeasurement::getCmmLab                   (int patch)
{ DBG_MESS_START
  std::vector<double> punkte (3) ;

  if (_Lab_Ergebnis.size() == 0)
    init ();

  if (patch > _nFelder) {
    WARN_S( "Patch Nr: " << patch << " ausserhalb des Messfarbsatzes" )
    DBG_MESS_ENDE
    return punkte;
  }

  punkte[0] =  _Lab_Ergebnis[patch].L          / 100.0;
  punkte[1] = (_Lab_Ergebnis[patch].a + 128.0) / 255.0;
  punkte[2] = (_Lab_Ergebnis[patch].b + 128.0) / 255.0;

  DBG_MESS_ENDE
  return punkte;
}


