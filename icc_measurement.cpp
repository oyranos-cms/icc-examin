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

ICCmeasurement::ICCmeasurement ()
{
  _sig = icMaxEnumTag;
  _data = NULL;
  _profil = NULL;
  _lcms_it8 = NULL;
}

ICCmeasurement::ICCmeasurement (ICCprofile* profil, ICCtag &tag)
{
  ICCmeasurement::load (profil, tag); DBG
}

ICCmeasurement::~ICCmeasurement ()
{
  _sig = icMaxEnumTag;
  _size = 0;
  if (_lcms_it8 != NULL) cmsIT8Free (_lcms_it8);
  if (_data != NULL) free (_data);
  DBG
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

  init_meas();
  DBG
}

void
ICCmeasurement::clear               (void)
{
  if (_lcms_it8 != NULL) cmsIT8Free (_lcms_it8);
  if (_data != NULL) free (_data);
  _sig = icMaxEnumTag;
  _size = 0;
  _data = NULL;

  _lcms_it8 = NULL;
  _nFelder = 0;

  _profil = NULL;
  _XYZ_measurement = false;
  _RGB_measurement = false;
  _CMYK_measurement = false;
  _XYZ_Satz.clear();
  _RGB_Satz.clear();
  _CMYK_Satz.clear();
  _Feldnamen.clear();

  #ifdef DEBUG_ICCMEASUREMENT
  DBG
  #endif
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
  DBG

  init_meas();

  #ifdef DEBUG_ICCMEASUREMENT
  DBG
  #endif
}

void
ICCmeasurement::init_meas (void)
{ //DBG_S (_data)

  std::string data (_data, 0, _size);

  // reparieren: Sample_Name , SampleID
  std::string::size_type pos=0;
  while ((pos = data.find ("SampleID", pos)) != std::string::npos) {
      data.replace (pos, 8, "SAMPLE_ID"); DBG_S( "SampleID ersetzt" )
  }
  pos = 0;
  while ((pos = data.find ("Sample_Name", pos)) != std::string::npos) {
      data.replace (pos, strlen("Sample_Name"), "SAMPLE_NAME"); DBG_S( "Sample_Name ersetzt" )
  }
  if (getTagName() == "CIED") {
      data.insert (0, "ICCEXAM\n"); DBG_S( "Beschreibung eingeführt" )
  }

  //DBG_S (data)
  #if 0
  clear();
  return;
  #endif

  // lcms liest ein
  if (_lcms_it8 != NULL) cmsIT8Free (_lcms_it8);
  _lcms_it8 = cmsIT8Alloc();
  if (_data != NULL) free (_data);
  _data = (char*) calloc (sizeof(char), data.size());
  _size = data.size();
  memcpy (_data, data.c_str(), _size);
  _lcms_it8 = cmsIT8LoadFromMem ( _data, _size ); DBG

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
  if (farben > 0) {
    _Feldnamen.resize(_nFelder);
    const char* constr = (const char*) calloc (sizeof (char), 12);
    DBG
    for (int k = 0; k < _nFelder; k++) {
      if (_id_vor_name
       && (getTagName() != "DevD")) {// Name ignorieren
        char *text = (char*) calloc (sizeof(char), 12);
        sprintf (text, "%d", k+1);
        _Feldnamen[k] = text;
      } else {
        constr = cmsIT8GetPatchName (_lcms_it8, k, NULL);
        _Feldnamen[k] = constr;
      }
      //cout << _Feldnamen[k] << " " << k << " "; DBG
    }
  } DBG_S (_Feldnamen[0] << " bis " << _Feldnamen[_nFelder-1])
 
  // Farben auslesen
  if (has_XYZ) { DBG // keine Umrechnung nötig
    _XYZ_Satz.resize(_nFelder);
    for (int i = 0; i < _nFelder; i++) {
        _XYZ_Satz[i].X = cmsIT8GetDataDbl (_lcms_it8, _Feldnamen[i].c_str(),
                                           "XYZ_X")/100.0;
        _XYZ_Satz[i].Y = cmsIT8GetDataDbl (_lcms_it8, _Feldnamen[i].c_str(),
                                           "XYZ_Y")/100.0;
        _XYZ_Satz[i].Z = cmsIT8GetDataDbl (_lcms_it8, _Feldnamen[i].c_str(),
                                           "XYZ_Z")/100.0;
    }
  }
  if (has_RGB) { DBG // keine Umrechnung nötig
    _RGB_Satz.resize(_nFelder);
    for (int i = 0; i < _nFelder; i++) {
        _RGB_Satz[i].R = cmsIT8GetDataDbl (_lcms_it8, _Feldnamen[i].c_str(),
                                           "RGB_R")/256.0;
        _RGB_Satz[i].G = cmsIT8GetDataDbl (_lcms_it8, _Feldnamen[i].c_str(),
                                           "RGB_G")/256.0;
        _RGB_Satz[i].B = cmsIT8GetDataDbl (_lcms_it8, _Feldnamen[i].c_str(),
                                           "RGB_B")/256.0;
    }
  }
  if (has_CMYK) { DBG // keine Umrechnung nötig
    _CMYK_Satz.resize(_nFelder);
    for (int i = 0; i < _nFelder; i++) { //cout << _Feldnamen[i] << " " << i << " "; DBG
        _CMYK_Satz[i].C = cmsIT8GetDataDbl (_lcms_it8, _Feldnamen[i].c_str(),
                                           "CMYK_C")/100.0;
        _CMYK_Satz[i].M = cmsIT8GetDataDbl (_lcms_it8, _Feldnamen[i].c_str(),
                                           "CMYK_M")/100.0;
        _CMYK_Satz[i].Y = cmsIT8GetDataDbl (_lcms_it8, _Feldnamen[i].c_str(),
                                           "CMYK_Y")/100.0;
        _CMYK_Satz[i].K = cmsIT8GetDataDbl (_lcms_it8, _Feldnamen[i].c_str(),
                                           "CMYK_K")/100.0;
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

  #ifdef DEBUG_ICCMEASUREMENT
  // Infos für die Konsole
  DBG_S( "Anzahl Messfelder: " << getPatchCount() << " Samples " << _nKanaele )

  if (_RGB_measurement && _XYZ_measurement) {
    for (int i = 0; i < _nFelder; i++) {
      cout << _Feldnamen[i] << ": " << _XYZ_Satz[i].X << ", " << _XYZ_Satz[i].Y << ", " << _XYZ_Satz[i].Z << ", "  << _RGB_Satz[i].R << ", "  << _RGB_Satz[i].G << ", "  << _RGB_Satz[i].B << endl;
    } DBG
  } else if (_CMYK_measurement && _XYZ_measurement) {
    DBG_S( _Feldnamen.size() << " | " << _CMYK_Satz.size() )
    for (int i = 0; i < _nFelder; i++) {
      cout << _Feldnamen[i] << ": " << _XYZ_Satz[i].X << ", " << _XYZ_Satz[i].Y << ", " << _XYZ_Satz[i].Z << ", "  << _CMYK_Satz[i].C << ", "  << _CMYK_Satz[i].M << ", "  << _CMYK_Satz[i].Y << ", "  << _CMYK_Satz[i].K << endl;
    } DBG
  }

  if (farben > 1
   && ((has_CMYK || has_RGB)
       && (has_Lab || has_XYZ || has_xyY))) {
    DBG_S( "sieht gut aus" )
  }
  #endif
}


std::vector<std::string>
ICCmeasurement::getText                     (void)
{ DBG
  std::vector<std::string> texte;
  std::string text = "Fehl";

  return texte;
}

std::vector<std::string>
ICCmeasurement::getDescription              (void)
{ DBG
  std::vector<std::string> texte;
  std::string text =  "";
  icUInt32Number count = *(icUInt32Number*)(_data+8);

  text.append ((const char*)(_data+12), icValue(count));
  texte.push_back (text);
  #ifdef DEBUG_ICCMEASUREMENT
  cout << &_data[12] << "|" << "|" << text << " "; DBG
  #endif
  return texte;
}

std::vector<double>
ICCmeasurement::getCIEXYZ                   (int patch)
{
  std::vector<double> punkte;
  icTagBase *base  = (icTagBase*)(&_data[0]);

  if ((base->sig) == (icTagTypeSignature)icValue( icSigChromaticityType )) {
    int count = icValue(*(icUInt16Number*)&_data[8]);
    if (count == 0)
      count = 3;
    #ifdef DEBUG_ICCMEASUREMENT
    cout << count << " "; DBG
    #endif
    for (int i = 0; i < count ; i++) { // Table 35 -- chromaticityType encoding
      // TODO lcms braucht einen 16 Byte Offset (statt 12 Byte)
      icU16Fixed16Number* channel = (icU16Fixed16Number*)&_data[12+(4*i)];
      double xyz[3] = { icValueUF( channel[0] ),
                        icValueUF( channel[1] ),
                        1.0 - (icValueUF(channel[0]) + icValueUF(channel[1])) };
      punkte.push_back( xyz[0] );
      punkte.push_back( xyz[1] );
      punkte.push_back( xyz[2] );
      #ifdef DEBUG_ICCMEASUREMENT
      cout << xyz[0] << ", " << xyz[1] << ", " << xyz[2] << " "; DBG
      #endif
    }
  } else if (base->sig == (icTagTypeSignature)icValue( icSigXYZType )) {
    icXYZType *daten = (icXYZType*) &_data[0];
    punkte.push_back( icValueSF( (daten->data.data[0].X) ) );
    punkte.push_back( icValueSF( (daten->data.data[0].Y) ) );
    punkte.push_back( icValueSF( (daten->data.data[0].Z) ) );
  }

  return punkte;
}

std::vector<std::string>
ICCmeasurement::getText                     (int patch)
{ DBG
  std::vector<std::string> texte;

  return texte;
}



