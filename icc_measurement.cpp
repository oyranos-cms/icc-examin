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
  _sig    = tag._sig;
  _size   = tag._size - 8;
  if (_data != NULL) free (_data);
  _data = (char*) calloc ( _size , sizeof (char) );
  memcpy ( _data , &(tag._data)[8] , _size );

  init_meas();
  DBG
}

void
ICCmeasurement::load                ( ICCprofile *profil,
                                      char       *data,
                                      size_t      size )
{
  _profil = profil;
  _sig    = icMaxEnumTag;
  _size   = size;
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

  char **SampleNames;// = (char**)calloc ( sizeof (char**), 1);

  _nFelder = cmsIT8EnumDataFormat(_lcms_it8, &SampleNames);

  #ifdef DEBUG_ICCMEASUREMENT
  for (int i = 0; i < _nFelder; i++)
    cout << (char*)SampleNames[i] << endl;
  #endif

  switch (_sig) {
  case icSigAToB0Tag:
  case icSigBToA0Tag:
  case icSigPreview0Tag:
    break;
  case icSigAToB1Tag:
  case icSigBToA1Tag:
  case icSigPreview1Tag:
    break;
  case icSigAToB2Tag:
  case icSigBToA2Tag:
  case icSigPreview2Tag:
    break;
  default:
    break;
  }

  #ifdef DEBUG_ICCMEASUREMENT
  cout << "Anzahl Messfelder: " << getPatchCount() << " Samples " << getSampleCount() << " "; DBG
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



