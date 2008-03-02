// Author:    Kai-Uwe Behrmann
// Copyright: Kai-Uwe Behrmann <ku.b@gmx.de>
// Date:      20. 08. 2004

#if 1
  #ifndef DEBUG
   #define DEBUG
  #endif
  #define DEBUG_ICCMEASUREMENT
#endif

#include "icc_profile.h"
#include "icc_examin.h"

#define _(text) text
#define g_message printf


/**
  *  @brief ICCmeasurement Funktionen
  */

ICCmeasurement::ICCmeasurement (ICCprofile* profil, ICCtag &tag)
{
  ICCmeasurement::load (profil, tag); DBG
}

void
ICCmeasurement::load                ( ICCprofile *profil,
                                      ICCtag&     tag )
{ DBG
  _profil = profil;

  _sig    = tag._sig;
  _size   = tag._size - 8;
  // einfach austauschen
  if (_data != NULL) free (_data); 
  _data = (char*) calloc ( _size , sizeof (char) );
  memcpy ( _data , &(tag._data)[8] , _size );

  DBG
}

void
ICCmeasurement::load                ( ICCprofile *profil,
                                      char       *data,
                                      size_t      size )
{ DBG
  _profil = profil;

  if (_sig != icMaxEnumTag)
    _sig = icSigCharTargetTag;

  _size   = size;
  // enfach austauschen
  if (!_data) free (_data);
  _data = (char*) calloc ( _size , sizeof (char) );
  memcpy ( _data , data , _size );

  #ifdef DEBUG_ICCMEASUREMENT
  DBG
  #endif
}

void
ICCmeasurement::leseTag (void)
{ DBG
  std::string data = ascii_korrigieren (); DBG_V( (int*)_data )

  // korrigierte CGATS Daten -> _data
  if (_data != NULL) free (_data);
  _data = (char*) calloc (sizeof(char), data.size());
  _size = data.size();
  memcpy (_data, data.c_str(), _size); DBG_V( (int*)_data )

  // lcms liest ein
  lcms_parse();
}

void
ICCmeasurement::init (void)
{ DBG_V( (int*)_data )
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
    DBG_V( _RGB_MessFarben.size() )

  init_umrechnen();
}

std::string
ICCmeasurement::ascii_korrigieren               (void)
{ DBG
  // Reparieren
  // LF FF
  char* ptr = 0; DBG_V( (int*) _data )
  while (strchr(_data, 13) > 0) { // \r 013 0x0d
      ptr = strchr(_data, 13);
      if (ptr > 0) {
        if (*(ptr+1) == '\n')
          *ptr = ' ';
        else
          *ptr = '\n';
      }
  };

  // char* -> std::string
  std::string data (_data, 0, _size);

  // reparieren: Sample_Name , SampleID, "" , LF FF
  std::string::size_type pos=0;
  std::string::size_type ende;
  while ((pos = data.find ("SampleID", pos)) != std::string::npos) {
      data.replace (pos, 8, "SAMPLE_ID"); DBG_S( "SampleID ersetzt" )
  }
  pos = 0;
  while ((pos = data.find ("Sample_Name", pos)) != std::string::npos) {
      data.replace (pos, strlen("Sample_Name"), "SAMPLE_NAME"); DBG_S( "Sample_Name ersetzt" )
  }
  pos = 0;
  while ((pos = data.find ("Lab_L", pos)) != std::string::npos) {
      data.replace (pos, strlen("Lab_L"), "LAB_L"); DBG_S( "Lab_L ersetzt" )
  }
  pos = 0;
  while ((pos = data.find ("Lab_a", pos)) != std::string::npos) {
      data.replace (pos, strlen("Lab_a"), "LAB_A"); DBG_S( "Lab_a ersetzt" )
  }
  pos = 0;
  while ((pos = data.find ("Lab_b", pos)) != std::string::npos) {
      data.replace (pos, strlen("Lab_b"), "LAB_B"); DBG_S( "Lab_b ersetzt" )
  }
  pos = 0;
  while ((pos = data.find ("\"\"", pos)) != std::string::npos) {
      data.replace (pos, strlen("\"\""), "\""); DBG_S( "\"\" ersetzt" )
  }
  pos = 0;
  while ((pos = data.find ("Date:", pos)) != std::string::npos) {
      data.replace (pos, strlen("Date:"), "CREATED \"\" #"); DBG_S( "Date: ersetzt" )
  }
  pos = data.find ("BEGIN_DATA\n", 0); // Kommentarzeilen löschen
  while ((pos = data.find ("\n#", pos)) != std::string::npos
       && (ende = data.find ("END_DATA\n", pos+2)) != std::string::npos) {
      ende = data.find ("\n", pos+2);
      data.erase (pos, ende - pos); DBG_S( "Kommentarzeile gelöscht" )
  }
  // fehlendes SAMPLE_ID einführen und jeder Zeile einen Zähler voransetzen
  int count;
  if ((data.find ("SAMPLE_ID", 0)) == std::string::npos) {
    pos = data.find ("BEGIN_DATA_FORMAT\n", 0);
    data.insert (pos+strlen("BEGIN_DATA_FORMAT\n"), "SAMPLE_ID   ");
    pos = data.find ("BEGIN_DATA\n", 0); DBG_S (pos)
    count = 1; pos++;
    while ((pos = data.find ("\n", pos)) != std::string::npos
         && (ende = data.find ("END_DATA\n", pos+2)) != std::string::npos) {
      static char zahl[12];
      sprintf (&zahl[0], "%d   ", count); count++; //DBG_S(count << " " << pos)
      data.insert (pos+1, &zahl[0]);
      pos += strlen (&zahl[0]);
      //DBG_S( data )
    }
  }
  // NUMBER_OF_FIELDS reparieren
  if ((data.find ("NUMBER_OF_FIELDS", 0)) == std::string::npos) {
    pos = data.find ("BEGIN_DATA_FORMAT\n", 0); DBG_S (pos)
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
    sprintf (&zahl[0], "NUMBER_OF_FIELDS %d\n", count);
    data.insert (pos, &zahl[0]);
    DBG_S( "NUMBER_OF_FIELDS " << count << " eingefügt" )
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
      data.insert (0, "ICCEXAM\n"); DBG_S( "Beschreibung eingeführt" )
  }

  //DBG_S (data)
  return data;
}


void
ICCmeasurement::lcms_parse                   (void)
{ DBG
  LCMSHANDLE _lcms_it8 = cmsIT8LoadFromMem ( _data, _size ); DBG_V( (int*)_data)

  char **SampleNames; DBG

  // Messfeldanzahl
  if (_nFelder == 0
   || _nFelder == (int)cmsIT8GetPropertyDbl(_lcms_it8, "NUMBER_OF_SETS")) { DBG
    _nFelder = (int)cmsIT8GetPropertyDbl(_lcms_it8, "NUMBER_OF_SETS"); DBG
  } else {
    DBG_S( "Messfeldanzahl sollte schon übereinstimmen! " << _nFelder << "|" << (int)cmsIT8GetPropertyDbl(_lcms_it8, "NUMBER_OF_SETS") )
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
    DBG_S( (char*)SampleNames[i] << " _sample_name " << _sample_name <<
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

  } DBG

  // Variablen
  int farben = 0;
  if (has_Lab) farben++;
  if (has_XYZ) {farben++; _XYZ_measurement = true; }
  if (has_RGB) {farben++; _RGB_measurement = true; }
  if (has_CMYK) {farben++; _CMYK_measurement = true; }
  if (has_xyY) farben++;


  // vorläufige lcms Farbnamen listen
    _Feldnamen.resize(_nFelder);
    DBG
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
  DBG_S (_Feldnamen[0] << " bis " << _Feldnamen[_nFelder-1])

  DBG_V( has_XYZ << has_RGB << has_CMYK )
 
  // Farben auslesen
  if (has_XYZ) { DBG // keine Umrechnung nötig
    _XYZ_Satz.resize(_nFelder);
    for (int i = 0; i < _nFelder; i++) {
        _XYZ_Satz[i].X = cmsIT8GetDataDbl (_lcms_it8, _Feldnamen[i].c_str(),
                                           "XYZ_X");
        _XYZ_Satz[i].Y = cmsIT8GetDataDbl (_lcms_it8, _Feldnamen[i].c_str(),
                                           "XYZ_Y");
        _XYZ_Satz[i].Z = cmsIT8GetDataDbl (_lcms_it8, _Feldnamen[i].c_str(),
                                           "XYZ_Z");
    }
  }
  if (has_RGB) { DBG // keine Umrechnung nötig
    _RGB_Satz.resize(_nFelder);
    for (int i = 0; i < _nFelder; i++) {
        _RGB_Satz[i].R = cmsIT8GetDataDbl (_lcms_it8, _Feldnamen[i].c_str(),
                                           "RGB_R");
        _RGB_Satz[i].G = cmsIT8GetDataDbl (_lcms_it8, _Feldnamen[i].c_str(),
                                           "RGB_G");
        _RGB_Satz[i].B = cmsIT8GetDataDbl (_lcms_it8, _Feldnamen[i].c_str(),
                                           "RGB_B");
    }
  }
  if (has_CMYK) { DBG // keine Umrechnung nötig
    _CMYK_Satz.resize(_nFelder);
    for (int i = 0; i < _nFelder; i++) { //cout << _Feldnamen[i] << " " << i << " "; DBG
        _CMYK_Satz[i].C = cmsIT8GetDataDbl (_lcms_it8, _Feldnamen[i].c_str(),
                                           "CMYK_C"); // /100.0;
        _CMYK_Satz[i].M = cmsIT8GetDataDbl (_lcms_it8, _Feldnamen[i].c_str(),
                                           "CMYK_M"); // /100.0;
        _CMYK_Satz[i].Y = cmsIT8GetDataDbl (_lcms_it8, _Feldnamen[i].c_str(),
                                           "CMYK_Y"); // /100.0;
        _CMYK_Satz[i].K = cmsIT8GetDataDbl (_lcms_it8, _Feldnamen[i].c_str(),
                                           "CMYK_K"); // /100.0;
    } DBG
  }


  // Farbnamen nach Geschmack (unmittelbar vor cmsIT8Free !)
  if (_id_vor_name) {
    for (int i = 0; i < _nFelder; i++) {
        _Feldnamen[i] = cmsIT8GetData (_lcms_it8, _Feldnamen[i].c_str(),
                                       "SAMPLE_NAME");
    } DBG_S (_Feldnamen[0] <<" bis "<< _Feldnamen[_nFelder-1] <<" "<< _nFelder)
  }

  // lcms's cgats Leser wird nicht mehr gebraucht
  cmsIT8Free (_lcms_it8);
  _lcms_it8 = NULL;
  DBG_V( _XYZ_Satz.size() )
  DBG_V( _RGB_Satz.size() )
  DBG_V( _CMYK_Satz.size() )
}

void
ICCmeasurement::init_umrechnen                     (void)
{ DBG
  _Lab_Differenz_max = -1000.0;
  _Lab_Differenz_min = 1000.0;
  _Lab_Differenz_Durchschnitt = 0.0;
  _DE00_Differenz_max = -1000.0;
  _DE00_Differenz_min = 1000.0;
  _DE00_Differenz_Durchschnitt = 0.0;
  #define PRECALC cmsFLAGS_NOTPRECALC // No memory overhead, VERY
                                      // SLOW ON TRANSFORMING, very fast on creating transform.
                                      // Maximum accurancy.

  if (_RGB_measurement && _XYZ_measurement) { DBG // keine Umrechnung nötig
    cmsHTRANSFORM hRGBtoSRGB, hXYZtoSRGB, hRGBtoXYZ, hXYZtoLab, hRGBtoLab;
    cmsHPROFILE hRGB, hsRGB, hLab, hXYZ;
    hRGB = cmsOpenProfileFromMem (_profil->_data, _profil->_size);
    if (getColorSpaceName(_profil->header.colorSpace()) != "Rgb") {
      cout << "unterschiedliche Messdaten und Profilfarbraum "; DBG
      this->clear();
      return;
    }

    hsRGB = cmsCreate_sRGBProfile ();
    hLab = cmsCreateLabProfile (cmsD50_xyY());
    hXYZ = cmsCreateXYZProfile ();
    // Wie sieht das Profil die Messfarbe? -> XYZ
    hRGBtoXYZ =  cmsCreateTransform (hRGB, TYPE_RGB_DBL,
                                    hXYZ, TYPE_XYZ_DBL,
                                    INTENT_ABSOLUTE_COLORIMETRIC,
                                    PRECALC|cmsFLAGS_WHITEBLACKCOMPENSATION);
    // Wie sieht das Profil die Messfarbe? -> Lab
    hRGBtoLab =  cmsCreateTransform (hRGB, TYPE_RGB_DBL,
                                    hLab, TYPE_Lab_DBL,
                                    INTENT_ABSOLUTE_COLORIMETRIC,
                                    PRECALC|cmsFLAGS_WHITEBLACKCOMPENSATION);
    // Wie sieht das Messgerät die Messfarbe? -> Lab
    hXYZtoLab =  cmsCreateTransform (hXYZ, TYPE_XYZ_DBL,
                                    hLab, TYPE_Lab_DBL,
                                    INTENT_ABSOLUTE_COLORIMETRIC,
                                    PRECALC|cmsFLAGS_WHITEBLACKCOMPENSATION);
    // Wie sieht das Profil die Messfarbe? -> Bildschirmdarstellung
    hRGBtoSRGB = cmsCreateTransform (hRGB, TYPE_RGB_DBL,
                                    hsRGB, TYPE_RGB_DBL,
                                    INTENT_ABSOLUTE_COLORIMETRIC,
                                    PRECALC|cmsFLAGS_WHITEBLACKCOMPENSATION);
    // Wie sieht die CMM die Messfarbe? -> Bildschirmdarstellung
    hXYZtoSRGB = cmsCreateTransform (hXYZ, TYPE_XYZ_DBL,
                                    hsRGB, TYPE_RGB_DBL,
                                    INTENT_ABSOLUTE_COLORIMETRIC,
                                    PRECALC|cmsFLAGS_WHITEBLACKCOMPENSATION);
    double RGB[3], sRGB[3], XYZ[3], Lab[3];

    _RGB_MessFarben.resize(_nFelder);
    _RGB_ProfilFarben.resize(_nFelder);
    _XYZ_Ergebnis.resize(_nFelder);
    _Lab_Satz.resize(_nFelder);
    _Lab_Ergebnis.resize(_nFelder);
    _Lab_Differenz.resize(_nFelder);
    _DE00_Differenz.resize(_nFelder);
    for (int i = 0; i < _nFelder; i++) {
        // Messfarben
        XYZ[0] = _XYZ_Satz[i].X / 100.0;
        XYZ[1] = _XYZ_Satz[i].Y / 100.0;
        XYZ[2] = _XYZ_Satz[i].Z / 100.0;
        cmsDoTransform (hXYZtoLab, &XYZ[0], &Lab[0], 1);
        _Lab_Satz[i].L = Lab[0];
        _Lab_Satz[i].a = Lab[1];
        _Lab_Satz[i].b = Lab[2];

        cmsDoTransform (hXYZtoSRGB, &XYZ[0], &sRGB[0], 1);
        _RGB_MessFarben[i].R = sRGB[0];
        _RGB_MessFarben[i].G = sRGB[1];
        _RGB_MessFarben[i].B = sRGB[2];

        // Profilfarben
        RGB[0] = _RGB_Satz[i].R/255.0;
        RGB[1] = _RGB_Satz[i].G/255.0;
        RGB[2] = _RGB_Satz[i].B/255.0;
        cmsDoTransform (hRGBtoXYZ, &RGB[0], &XYZ[0], 1);
        _XYZ_Ergebnis[i].X = XYZ[0] * 100.0;
        _XYZ_Ergebnis[i].Y = XYZ[1] * 100.0;
        _XYZ_Ergebnis[i].Z = XYZ[2] * 100.0;

        cmsDoTransform (hRGBtoLab, &RGB[0], &Lab[0], 1);
        _Lab_Ergebnis[i].L = Lab[0];
        _Lab_Ergebnis[i].a = Lab[1];
        _Lab_Ergebnis[i].b = Lab[2];
        
        cmsDoTransform (hRGBtoSRGB, &RGB[0], &sRGB[0], 1);
        _RGB_ProfilFarben[i].R = sRGB[0];
        _RGB_ProfilFarben[i].G = sRGB[1];
        _RGB_ProfilFarben[i].B = sRGB[2];

        // geometrische Farbortdifferenz - dE CIE*Lab
        _Lab_Differenz[i] = pow (    pow(_Lab_Ergebnis[i].L - _Lab_Satz[i].L,2)
                                   + pow(_Lab_Ergebnis[i].a - _Lab_Satz[i].a,2)
                                   + pow(_Lab_Ergebnis[i].b - _Lab_Satz[i].b,2)
                                 , 1.0/2.0);
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
    cmsDeleteTransform (hRGBtoSRGB);
    cmsDeleteTransform (hXYZtoSRGB);
    cmsDeleteTransform (hRGBtoXYZ);
    cmsDeleteTransform (hXYZtoLab);
    cmsDeleteTransform (hRGBtoLab);
    cmsCloseProfile (hRGB);
    cmsCloseProfile (hsRGB);
    cmsCloseProfile (hLab);
    cmsCloseProfile (hXYZ);
  } else
  if (_CMYK_measurement && _XYZ_measurement) { DBG // keine Umrechnung nötig
    cmsHTRANSFORM hCMYKtoSRGB, hXYZtoSRGB, hCMYKtoXYZ, hXYZtoLab, hCMYKtoLab;
    cmsHPROFILE hCMYK, hsRGB, hLab, hXYZ;
    hCMYK = cmsOpenProfileFromMem (_profil->_data, _profil->_size);
    if (getColorSpaceName(_profil->header.colorSpace()) != "Cmyk") {
      cout << "unterschiedliche Messdaten und Profilfarbraum "; DBG
      return;
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
    // Wie sieht das Messgerät die Messfarbe? -> Lab
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
    _Lab_Satz.resize(_nFelder);
    _Lab_Ergebnis.resize(_nFelder);
    _Lab_Differenz.resize(_nFelder);
    _DE00_Differenz.resize(_nFelder);
    for (int i = 0; i < _nFelder; i++) {
        // Messfarben
        XYZ[0] = _XYZ_Satz[i].X / 100.0;
        XYZ[1] = _XYZ_Satz[i].Y / 100.0;
        XYZ[2] = _XYZ_Satz[i].Z / 100.0;
        cmsDoTransform (hXYZtoLab, &XYZ[0], &Lab[0], 1);
        _Lab_Satz[i].L = Lab[0];
        _Lab_Satz[i].a = Lab[1];
        _Lab_Satz[i].b = Lab[2];

        cmsDoTransform (hXYZtoSRGB, &XYZ[0], &sRGB[0], 1);
        _RGB_MessFarben[i].R = sRGB[0];
        _RGB_MessFarben[i].G = sRGB[1];
        _RGB_MessFarben[i].B = sRGB[2];

        // Profilfarben
        CMYK[0] = _CMYK_Satz[i].C;
        CMYK[1] = _CMYK_Satz[i].M;
        CMYK[2] = _CMYK_Satz[i].Y;
        CMYK[3] = _CMYK_Satz[i].K;
        cmsDoTransform (hCMYKtoXYZ, &CMYK[0], &XYZ[0], 1);
        _XYZ_Ergebnis[i].X = XYZ[0] * 100.0;
        _XYZ_Ergebnis[i].Y = XYZ[1] * 100.0;
        _XYZ_Ergebnis[i].Z = XYZ[2] * 100.0;

        cmsDoTransform (hCMYKtoLab, &CMYK[0], &Lab[0], 1);
        _Lab_Ergebnis[i].L = Lab[0];
        _Lab_Ergebnis[i].a = Lab[1];
        _Lab_Ergebnis[i].b = Lab[2];
        
        cmsDoTransform (hCMYKtoSRGB, &CMYK[0], &sRGB[0], 1);
        _RGB_ProfilFarben[i].R = sRGB[0];
        _RGB_ProfilFarben[i].G = sRGB[1];
        _RGB_ProfilFarben[i].B = sRGB[2];

        // geometrische Farbortdifferenz
        _Lab_Differenz[i] = pow (    pow(_Lab_Ergebnis[i].L - _Lab_Satz[i].L,2)
                                   + pow(_Lab_Ergebnis[i].a - _Lab_Satz[i].a,2)
                                   + pow(_Lab_Ergebnis[i].b - _Lab_Satz[i].b,2)
                                 , 1.0/2.0);
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
  }
  for (unsigned int i = 0; i < _Lab_Differenz.size(); i++) {
    _Lab_Differenz_Durchschnitt += _Lab_Differenz[i];
  }
  _Lab_Differenz_Durchschnitt /= (double)_Lab_Differenz.size();
  for (unsigned int i = 0; i < _DE00_Differenz.size(); i++) {
    _DE00_Differenz_Durchschnitt += _DE00_Differenz[i];
  }
  _DE00_Differenz_Durchschnitt /= (double)_DE00_Differenz.size();
  DBG_V( _RGB_MessFarben.size() )
}

std::string
ICCmeasurement::getHtmlReport                     (void)
{ DBG
  char SF[] = "#cccccc";  // standard Hintergrundfarbe
  char HF[] = "#aaaaaa";  // hervorgehoben
  #define LAYOUTFARBE if (layout[l++] == true) \
                        html << HF; \
                      else \
                        html << SF; //Farbe nach Layoutoption auswählen
  int l = 0;
  std::stringstream html; DBG_V( _RGB_MessFarben.size() )
  if (_RGB_MessFarben.size() == 0)
    init ();

  if (_reportTabelle.size() == 0)
    _reportTabelle = getText();
  std::vector<int> layout = getLayout(); DBG

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
  } DBG_V( _nFelder )
  if (!_nFelder)
    return html.str();

  html <<       "<table align=left cellpadding=\"2\" cellspacing=\"0\" border=\"0\" width=\"90%\" bgcolor=\"" << SF << "\">\n";
  html <<       "<thead> \n";
  html <<       "  <tr> \n"; DBG
  // Tabellenkopf
  int s = 0;           // Spalten
  int f = 0;           // Spalten für Farben
  if (_XYZ_Satz.size() && _RGB_MessFarben.size() == _XYZ_Satz.size()) {
    f = 2;
  } DBG
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
  } DBG
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
        sprintf (farbe,"");
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
  DBG
  //DBG_S (html.str() )
  return html.str();
}

std::vector<std::vector<std::string> >
ICCmeasurement::getText                     (void)
{ DBG_V( _RGB_MessFarben.size() )
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
    } DBG
    z++;
    // Messwerte
    s.str("");
    for (int i = 0; i < _nFelder; i++) { 
      sp = 0;
      tabelle[z+i].resize( spalten );
      tabelle[z+i][sp++] =  _Feldnamen[i];
      s << _Lab_Differenz[i]; tabelle[z+i][sp++] = s.str().c_str(); s.str("");
      s << _DE00_Differenz[i]; tabelle[z+i][sp++] = s.str().c_str(); s.str("");
      s << _Lab_Satz[i].L; tabelle[z+i][sp++] = s.str().c_str(); s.str("");
      s << _Lab_Satz[i].a; tabelle[z+i][sp++] = s.str().c_str(); s.str("");
      s << _Lab_Satz[i].b; tabelle[z+i][sp++] = s.str().c_str(); s.str("");
      s << _Lab_Ergebnis[i].L; tabelle[z+i][sp++] = s.str().c_str(); s.str("");
      s << _Lab_Ergebnis[i].a; tabelle[z+i][sp++] = s.str().c_str(); s.str("");
      s << _Lab_Ergebnis[i].b; tabelle[z+i][sp++] = s.str().c_str(); s.str("");
      s << _XYZ_Satz[i].X; tabelle[z+i][sp++] = s.str().c_str(); s.str("");
      s << _XYZ_Satz[i].Y; tabelle[z+i][sp++] = s.str().c_str(); s.str("");
      s << _XYZ_Satz[i].Z; tabelle[z+i][sp++] = s.str().c_str(); s.str("");
      if (xyz_erg_sp) {
      s << _XYZ_Ergebnis[i].X; tabelle[z+i][sp++] = s.str().c_str(); s.str("");
      s << _XYZ_Ergebnis[i].Y; tabelle[z+i][sp++] = s.str().c_str(); s.str("");
      s << _XYZ_Ergebnis[i].Z; tabelle[z+i][sp++] = s.str().c_str(); s.str("");
      }
      if (_RGB_measurement) {
        s << _RGB_Satz[i].R; tabelle[z+i][sp++] = s.str().c_str(); s.str("");
        s << _RGB_Satz[i].G; tabelle[z+i][sp++] = s.str().c_str(); s.str("");
        s << _RGB_Satz[i].B; tabelle[z+i][sp++] = s.str().c_str(); s.str("");
      } else {
        s << _CMYK_Satz[i].C; tabelle[z+i][sp++] = s.str().c_str(); s.str("");
        s << _CMYK_Satz[i].M; tabelle[z+i][sp++] = s.str().c_str(); s.str("");
        s << _CMYK_Satz[i].Y; tabelle[z+i][sp++] = s.str().c_str(); s.str("");
        s << _CMYK_Satz[i].K; tabelle[z+i][sp++] = s.str().c_str(); s.str("");
      }
    }
  }

  return tabelle;
}

std::vector<std::string>
ICCmeasurement::getDescription              (void)
{ DBG
  std::vector<std::string> texte;
  std::string text =  "";

  #ifdef DEBUG_ICCMEASUREMENT
  #endif
  return texte;
}

std::vector<double>
ICCmeasurement::getMessRGB                  (int patch)
{ DBG
  std::vector<double> punkte(3);

  if (_RGB_MessFarben.size() == 0)
    init ();

  if (patch > _nFelder)
    return punkte;

  punkte[0] = _RGB_MessFarben[patch].R;
  punkte[1] = _RGB_MessFarben[patch].G;
  punkte[2] = _RGB_MessFarben[patch].B;

  return punkte;
}

std::vector<double>
ICCmeasurement::getCmmRGB                   (int patch)
{ DBG
  std::vector<double> punkte (3) ;

  if (_RGB_MessFarben.size() == 0)
    init ();

  if (patch > _nFelder)
    return punkte;

  punkte[0] = _RGB_ProfilFarben[patch].R;
  punkte[1] = _RGB_ProfilFarben[patch].G;
  punkte[2] = _RGB_ProfilFarben[patch].B;

  return punkte;
}


