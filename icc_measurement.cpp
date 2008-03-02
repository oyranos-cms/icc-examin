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
{
  _profil = profil;

  if (_sig != icMaxEnumTag)
    _sig = icSigCharTargetTag;
  else
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
{
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
{
  // lcms liest ein
  _lcms_it8 = cmsIT8LoadFromMem ( _data, _size );

  char **SampleNames;

  _nFelder = (int)cmsIT8GetPropertyDbl(_lcms_it8, "NUMBER_OF_SETS");
  int _nKanaele = cmsIT8EnumDataFormat(_lcms_it8, &SampleNames);
  #ifdef DEBUG_ICCMEASUREMENT
  for (int i = 0; i < _nKanaele; i++)
    cout << (char*)SampleNames[i] << endl;
  #endif

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

  }

  int farben = 0;
  if (has_Lab) farben++;
  if (has_XYZ) {farben++; _XYZ_measurement = true; }
  if (has_RGB) {farben++; _RGB_measurement = true; }
  if (has_CMYK) {farben++; _CMYK_measurement = true; }
  if (has_xyY) farben++;


  const char* constr = (const char*) calloc (sizeof (char), 12);

  if (has_XYZ) { // keine Umrechnung nötig
    _XYZ_Satz.resize(_nFelder);
    _Feldnamen.resize(_nFelder);
    for (int i = 0; i < _nFelder; i++) { DBG
        constr = cmsIT8GetPatchName (_lcms_it8, i, NULL);
        _Feldnamen[i] = constr;
        _XYZ_Satz[i].X = cmsIT8GetDataDbl (_lcms_it8, constr, "XYZ_X")/100.0;
        _XYZ_Satz[i].Y = cmsIT8GetDataDbl (_lcms_it8, constr, "XYZ_Y")/100.0;
        _XYZ_Satz[i].Z = cmsIT8GetDataDbl (_lcms_it8, constr, "XYZ_Z")/100.0;
    }
  } DBG
  if (has_RGB) { // keine Umrechnung nötig
    _RGB_Satz.resize(_nFelder);
    for (int i = 0; i < _nFelder; i++) {
        _RGB_Satz[i].R = cmsIT8GetDataDbl (_lcms_it8, _Feldnamen[i].c_str(),
                                           "RGB_R")/256.0;
        _RGB_Satz[i].G = cmsIT8GetDataDbl (_lcms_it8, _Feldnamen[i].c_str(),
                                           "RGB_G")/256.0;
        _RGB_Satz[i].B = cmsIT8GetDataDbl (_lcms_it8, _Feldnamen[i].c_str(),
                                           "RGB_B")/256.0;
    }
  } DBG
  if (has_CMYK) { // keine Umrechnung nötig
    _CMYK_Satz.resize(_nFelder);
    for (int i = 0; i < _nFelder; i++) {
        _CMYK_Satz[i].C = cmsIT8GetDataDbl (_lcms_it8, _Feldnamen[i].c_str(),
                                           "CMYK_C")/256.0;
        _CMYK_Satz[i].M = cmsIT8GetDataDbl (_lcms_it8, _Feldnamen[i].c_str(),
                                           "CMYK_M")/256.0;
        _CMYK_Satz[i].Y = cmsIT8GetDataDbl (_lcms_it8, _Feldnamen[i].c_str(),
                                           "CMYK_Y")/256.0;
        _CMYK_Satz[i].K = cmsIT8GetDataDbl (_lcms_it8, _Feldnamen[i].c_str(),
                                           "CMYK_K")/256.0;
    }
  } DBG

  #ifdef DEBUG_ICCMEASUREMENT
  if (_RGB_measurement) {
    for (int i = 0; i < _nFelder; i++) {
      cout << _Feldnamen[i] << ": " << _XYZ_Satz[i].X << ", " << _XYZ_Satz[i].Y << ", " << _XYZ_Satz[i].Z << ", "  << _RGB_Satz[i].R << ", "  << _RGB_Satz[i].G << ", "  << _RGB_Satz[i].B << endl;
    } DBG
  } else if (_CMYK_measurement) {
    for (int i = 0; i < _nFelder; i++) {
      cout << _Feldnamen[i] << ": " << _XYZ_Satz[i].X << ", " << _XYZ_Satz[i].Y << ", " << _XYZ_Satz[i].Z << ", "  << _CMYK_Satz[i].C << ", "  << _CMYK_Satz[i].M << ", "  << _CMYK_Satz[i].Y << ", "  << _CMYK_Satz[i].K << endl;
    } DBG
  }

  cout << "Anzahl Messfelder: " << getPatchCount() << " Samples " << _nKanaele << " "; DBG

  if (farben > 1
   && ((has_CMYK || has_RGB)
       && (has_Lab || has_XYZ || has_xyY))) {
    cout << "sieht gut aus " ; DBG
  }
  #endif
}


std::vector<std::string>
ICCmeasurement::getText                     (void)
{ DBG
  std::vector<std::string> texte;
  std::string text = "Fehl";
  int count = 0;

  if (getTypName() == "meas") {
    if (_size < 36) return texte;
    std::stringstream s;
    icMeasurement meas;
    memcpy (&meas, &_data[8] , 28);
    s << _("Standard Betrachter") << ": " <<
    getStandardObserver( (icStandardObserver)icValue( meas.stdObserver) ) <<endl
      << _("Rückseite") << ": X = " << icValueSF(meas.backing.X)
                        << ", Y = " << icValueSF(meas.backing.Y)
                        << ", Z = " << icValueSF(meas.backing.Z) << endl
      << _("Geometrie") << ": "<< 
    getMeasurementGeometry ((icMeasurementGeometry)icValue(meas.geometry))<<endl
      << _("Flare")     << ": "<< 
    getMeasurementFlare ((icMeasurementFlare)icValue(meas.flare)) << endl
      << _("Beleuchtungstyp") << ": " <<
    getIlluminant ((icIlluminant)icValue(meas.illuminant)) <<endl;
    texte.push_back( s.str() );
  } else  if (getTypName() == "text"
           || getTypName() == "cprt?" ) { // text
    text = ""; DBG
  #if 1
    char* txt = (char*)calloc (_size-8, sizeof(char));
    memcpy (txt, &_data[8], _size - 8);
    char* pos = 0;
    #ifdef DEBUG_ICCMEASUREMENT
    cout << (int)strchr(txt, 13) << " "; DBG
    #endif
    while (strchr(txt, 13) > 0) { // \r 013 0x0d
      pos = strchr(txt, 13);
      #ifdef DEBUG_ICCMEASUREMENT
      cout << (int)pos << " "; DBG
      #endif
      if (pos > 0) {
        if (*(pos+1) == '\n')
          *pos = ' ';
        else
          *pos = '\n';
      }
      count++;
    };
    text = txt;
    free (txt);
  #else
    text.append (&_data[8], _size - 8);
    int pos = 0;
    #ifdef DEBUG_ICCMEASUREMENT
    cout << (int)text.find('\r') << " "; DBG
    #endif
    while ((int)text.find('\r') > 0) { // \r 013 0x0d
      pos = (int)text.find('\r');
      #ifdef DEBUG_ICCMEASUREMENT
      cout << pos << " "; DBG
      #endif
      if (pos > 0)
        //text.erase (pos);
        text.replace (pos, 1, ' ', 1); 
      count++;
    };
  #endif
    texte.push_back( text );

  } else {
    texte.push_back( getTypName() + " | <- iss'n das?" );
  }
    
  #ifdef DEBUG_ICCMEASUREMENT
  cout << count << " Ersetzungen "; DBG
  cout << " " << "" << "|" << getTypName() << "|" << text << " "; DBG
  #endif
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



