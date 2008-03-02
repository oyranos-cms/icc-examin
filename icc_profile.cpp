// Author:    Kai-Uwe Behrmann
// Copyright: Kai-Uwe Behrmann <ku.b@gmx.de>
// Date:      04. 05. 2004

#if 1
  #ifndef DEBUG
   #define DEBUG
  #endif
  #define DEBUG_ICCFUNKT
  #define DEBUG_ICCHEADER
  #define DEBUG_ICCTAG
  #define DEBUG_ICCPROFILE
#endif

#include "icc_profile.h"
#include "icc_examin.h"
#include "icc_utils.h"

#define _(text) text
#define g_message printf


/**
  *  @brief ICCheader Funktionen
  */ 

ICCheader::ICCheader()
{ DBG_PROG_START
  valid = false;
  DBG_PROG_ENDE
}

void
ICCheader::load (void *data)
{ DBG_PROG_START
  if (data == NULL) {
    for (int i = 0; i < 128; i++)
      ((char*)&header)[i] = 0;
    valid = false;
    return;
  }

  memcpy ((void*)&header, data, sizeof (icHeader));
  #ifdef DEBUG_ICCHEADER
  cout << sizeof (icHeader) << " genommen" << " "; DBG
  #endif
  if (header.size > 0) {
    valid = true;
  #ifdef DEBUG_ICCHEADER
    cout << size() << " "; DBG
  #endif
  } else {
    valid = false;
  }
  DBG_PROG_ENDE
}


void
ICCheader::set_current_date (void)
{ DBG_PROG_START // TODO Zeit setzen
  header.date.day = icValue((icUInt16Number)1);
  header.date.month = icValue((icUInt16Number)9);
  header.date.year = icValue((icUInt16Number)2004);
  header.date.hours = icValue((icUInt16Number)12);
  header.date.minutes = icValue((icUInt16Number)0);
  header.date.seconds = icValue((icUInt16Number)0);
  DBG_PROG_ENDE
}

std::string
ICCheader::attributes (void)
{ DBG_PROG_START
  std::stringstream s;
  char* f = (char*) &(header.attributes);
  unsigned char         maske1 = 0x80;
  unsigned char         maske2 = 0x40;
  unsigned char         maske3 = 0x20;
  unsigned char         maske4 = 0x10;

  if (f[0] & maske1)
    s << _("Durchsicht, ");
  else
    s << _("Aufsicht, ");

  if (f[0] & maske2)  
    s << _("matt, ");
  else
    s << _("glänzend, ");

  if (f[0] & maske3)  
    s << _("negativ, ");
  else
    s << _("positiv, ");

  if (f[0] & maske4)  
    s << _("schwarz/weiss");
  else
    s << _("farbig");


  #ifdef DEBUG
  cout << (long)header.attributes; DBG
  char* ptr = (char*) &(header.attributes);
  for (int i = 0; i < 8 ; i++)
    cout << (int)ptr[i] << " ";
  DBG
  #endif
  DBG_PROG_ENDE
  return s.str();
}

std::string
ICCheader::flags (void)
{ DBG_PROG_START
  std::stringstream s;
  char* f = (char*) &(header.flags);
  unsigned char         maske1 = 0x80;
  unsigned char         maske2 = 0x40;

  if (f[0] & maske1)
    s << _("Farbprofil ist eingebettet und ");
  else
    s << _("Farbprofil ist nicht eingebettet und ");

  if (f[0] & maske2)  
    s << _("kann nicht unabhängig vom Bild verwendet werden.");
  else
    s << _("kann unabhängig vom Bild verwendet werden.");

  #ifdef DEBUG
  cout << (int)f[0] << " " << (long)header.flags; DBG
  char* ptr = (char*) &(header.flags);
  for (int i = 0; i < 8 ; i++)
    cout << (int)ptr[i] << " ";
  DBG
  #endif
  DBG_PROG_ENDE
  return s.str();
}

std::string
ICCheader::versionName (void)
{ DBG_PROG_START
  std::stringstream s;
  char* v = (char*)&(header.version);
  
  s << (int)v[0] << "." << (int)v[1]/16 << "." << (int)v[1]%16;

  DBG_PROG_ENDE
  return s.str();
}

std::string
ICCheader::print_long()
{ DBG_PROG_START
  #ifdef DEBUG_ICCHEADER
  cout << sizeof (icSignature) << " " << sizeof (icUInt32Number)<< endl;
  #endif
  std::string cm = cmmName();
  std::string mg = magicName();
  std::string ma = manufacturerName();
  std::string mo = modelName();
  std::string cr = creatorName();
  std::stringstream s; DBG
  s << "kein Dateikopf gefunden"; DBG
  
  //cout << "char icSignature ist " << sizeof (icSignature) << endl;
  if (valid) { DBG
    s.str("");
    s << "ICC Dateikopf:\n"<< endl \
      <<  "    " << _("Größe") << ":       " <<
                       size() << " " << _("bytes") << endl \
      <<  "    " << _("CMM") << ":         " << cm << endl \
      <<  "    " << _("Version") << ":     " << versionName() << endl \
      <<  "    " << _("Klasse") << ":      " <<
                       getDeviceClassName(deviceClass()) << endl \
      <<  "    " << _("Farbraum") << ":    " <<
                       getColorSpaceName(colorSpace()) << endl \
      <<  "    " << _("PCS") << ":         " <<
                       getColorSpaceName(pcs()) << endl \
      <<  "    " << printDatum(header.date) << endl \
      <<  "    " << _("Magic") << ":       " << mg << endl \
      <<  "    " << _("Plattform") << ":   " << platform() << endl \
      <<  "    " << _("Flags") << ":       " << flags() << endl \
      <<  "    " << _("Hersteller") << ":  " << ma << endl \
      <<  "    " << _("Model") << ":       " << mo << endl \
      <<  "    " << _("Attribute") << ":   " <<
                       attributes() << endl \
      <<  "    " << _("Übertragung") << ": " <<
                       renderingIntent() << endl \
      <<  "    " << _("Beleuchtung") << ": X=" <<
                       icSFValue(header.illuminant.X) << ", Y=" << \
                       icSFValue(header.illuminant.Y) << ", Z=" << \
                       icSFValue(header.illuminant.Z) << endl \
      <<  "    " << _("erzeugt von") << ": " << cr << endl ;
  } DBG_PROG_ENDE
  return s.str();
}

std::string
ICCheader::print()
{ DBG_PROG_START
  std::string s = _("Dateikopf ungültig");
  if (valid)
    s = _("    Kopf        icHeader      128 Dateikopf");
  DBG_PROG_ENDE
  return s;
}

/**
  *  @brief ICCtag Funktionen
  */

ICCtag::ICCtag                      (ICCprofile* profil, icTag* tag, char* data)
{ DBG_PROG_START
  DBG_PROG_S("ICCtag::ICCtag ICCprofile* , icTag* , char*  - beginn")
  ICCtag::load (profil, tag, data); DBG_S("ICCtag::ICCtag ICCprofile* , icTag* , char*  - fertig")
  DBG_PROG_ENDE
}

void
ICCtag::load                        ( ICCprofile *profil,
                                      icTag      *tag,
                                      char       *data )
{ DBG_PROG_START
  DBG_MEM_V( profil )
  DBG_MEM_V( tag )
  DBG_MEM_V( data )
  _profil = profil;
  _sig    = icValue(tag->sig); DBG_S( getSigTagName(_sig) )
  switch (_sig) {
  case icSigAToB0Tag:
  case icSigBToA0Tag:
  case icSigPreview0Tag:
    _intent = 0; break;
  case icSigAToB1Tag:
  case icSigBToA1Tag:
  case icSigPreview1Tag:
    _intent = 1; break;
  case icSigAToB2Tag:
  case icSigBToA2Tag:
  case icSigPreview2Tag:
    _intent = 2; break;
  default:
    _intent = -1;
    break;
  }

  switch (_sig) {
  case icSigAToB0Tag:
  case icSigAToB1Tag:
  case icSigAToB2Tag:
    _color_in = _profil->header.colorSpace();
    _color_out = _profil->header.pcs();
    _to_pcs = true;
    break;
  case icSigBToA0Tag:
  case icSigBToA1Tag:
  case icSigBToA2Tag:
    _color_in = _profil->header.pcs();
    _color_out = _profil->header.colorSpace();
    _to_pcs = false;
    break;
  case icSigGamutTag:
    _color_in = _profil->header.pcs(); _color_out = (icColorSpaceSignature)0;
    break;
  case icSigPreview0Tag:
  case icSigPreview1Tag:
  case icSigPreview2Tag:
    _color_in = _profil->header.pcs(); _color_out = _profil->header.pcs();
    _to_pcs = true;
    break;
  default:
    _color_in = (icColorSpaceSignature)0; _color_out = (icColorSpaceSignature)0;
    _to_pcs = true;
    break;
  }

  DBG_PROG

  if (_data != NULL && _size) { DBG_MEM
    DBG_MEM_S( "ICCtag wiederverwendet: " << (char*)tag->sig << " " )
    DBG_MEM
    free(_data); DBG_MEM // delete [] _data;
    DBG_MEM_S( "ICCtag wiederverwendet: " << (char*)tag->sig << " " )
  } DBG_PROG
  _size   = icValue(tag->size); DBG_MEM_V( _size )

  _data = (char*) calloc(sizeof(char),_size); // new char (_size);
  memcpy ( _data , data , _size );
  DBG_MEM_V((int*)_data)

  #ifdef DEBUG_ICCTAG_
  char* text = _data;
  cout << _sig << "=" << tag->sig << " offset " << icValue(tag->offset) << " size " << _size << " nächster tag " << _size + icValue(tag->offset) << " " << text << " "; DBG
  #endif
  DBG_PROG_ENDE
}

std::vector<std::string>
ICCtag::getText                     (void)
{ DBG_PROG_START
  std::vector<std::string> texte;
  std::string text = "Fehl";
  int count = 0;

  if (getTypName() == "sig") {

    if (_size < 12) return texte;
    icTechnologySignature tech;
    memcpy (&tech, &_data[8] , 4);
    text = getSigTechnology( (icTechnologySignature) icValue(tech) );
    texte.push_back( text );

  } else if (getTypName() == "dtim") {

    if (_size < 20) return texte;
    DBG
    icDateTimeNumber date;
    memcpy (&date, &_data[8] , 12);
    texte.push_back( printDatum(date) );

  } else if (getTypName() == "meas") {

    if (_size < 36) return texte;
    std::stringstream s;
    icMeasurement meas;
    memcpy (&meas, &_data[8] , 28);
    s << _("Standard Betrachter") << ": " <<
    getStandardObserver( (icStandardObserver)icValue( meas.stdObserver) ) <<endl
      << _("Rückseite") << ": X = " << icSFValue(meas.backing.X)
                        << ", Y = " << icSFValue(meas.backing.Y)
                        << ", Z = " << icSFValue(meas.backing.Z) << endl
      << _("Geometrie") << ": "<< 
    getMeasurementGeometry ((icMeasurementGeometry)icValue(meas.geometry))<<endl
      << _("Flare")     << ": "<< 
    getMeasurementFlare ((icMeasurementFlare)icValue(meas.flare)) << endl
      << _("Beleuchtungstyp") << ": " <<
    getIlluminant ((icIlluminant)icValue(meas.illuminant)) <<endl;
    texte.push_back( s.str() );

  } else if (getTypName() == "mft2") {

    icLut16* lut16 = (icLut16*) &_data[8];
    int inputChan, outputChan, clutPoints, inputEnt, outputEnt;

    inputChan = (int)lut16->inputChan;
    outputChan = (int)lut16->outputChan;
    clutPoints = (int)lut16->clutPoints;
    inputEnt = icValue(lut16->inputEnt);
    outputEnt = icValue(lut16->outputEnt);
    std::stringstream s;
    s << _("Konvertierungskette mit 16-bit Präzission:") << endl <<
         _("Intent:") << " " << renderingIntentName(_intent) << endl <<
         _("Eingangskanäle") << " (" << getColorSpaceName(_color_in)  << "): " << (int)inputChan << endl <<
         _("Ausgangskanäle") << " (" << getColorSpaceName(_color_out) << "): " << (int)outputChan << endl <<
         _("Matrix") << endl <<
         _("lineare Eingangkurve") << " " << _("mit") << " " << (int)inputEnt << " " << _("Stufungen") << endl <<
         _("3D Farbtabelle mit") << " " <<  (int)clutPoints << " " << _("Punkten Seitenlänge") << endl <<
         _("lineare Ausgangskurve") << " " << _("mit") << " " << (int)outputEnt << " " << _("Stufungen") << endl;
    texte.push_back( s.str() );

  } else if (getTypName() == "mft1") {

    icLut8* lut8 = (icLut8*) &_data[8];
    int inputChan, outputChan, clutPoints;//, inputEnt, outputEnt;
    inputChan = (int)lut8->inputChan;
    outputChan = (int)lut8->outputChan;
    clutPoints = (int)lut8->clutPoints;

    std::stringstream s;
    s << _("Konvertierungskette mit 8-bit Präzission:") << endl <<
         _("Intent:") << " " << renderingIntentName(_intent) << endl <<
         _("Eingangskanäle") << " (" << getColorSpaceName(_color_in)  << "): " << (int)inputChan << endl <<
         _("Ausgangskanäle") << " (" << getColorSpaceName(_color_out) << "): " << (int)outputChan << endl <<
         _("Matrix") << endl <<
         _("3D Farbtabelle mit") << " " <<  (int)clutPoints << " " << _("Punkten Seitenlänge") << endl;
    texte.push_back( s.str() );

  } else if (((icTagBase*)&_data[0])->sig == (icTagTypeSignature)icValue( icSigChromaticityType )) {

    int count = icValue(*(icUInt16Number*)&_data[8]);
    if (count == 0)
      count = 3;
    #ifdef DEBUG_ICCTAG
    cout << count << " "; DBG
    #endif
    for (int i = 0; i < count ; i++) { // Table 35 -- chromaticityType encoding
      std::stringstream s;
      s << _("Kanal ") << i;
      texte.push_back( s.str() );
      texte.push_back( "chrm" );
      #ifdef DEBUG_ICCTAG
      cout << s.str(); DBG
      #endif
    }

  } else  if (getTypName() == "text"
           || getTypName() == "cprt?" ) { // text

    text = ""; DBG_PROG
  #if 1
    char* txt = (char*)calloc (_size-8, sizeof(char));
    memcpy (txt, &_data[8], _size - 8);
    char* pos = 0;
    #ifdef DEBUG_ICCTAG
    cout << (int)strchr(txt, 13) << " "; DBG
    #endif
    while (strchr(txt, 13) > 0) { // \r 013 0x0d
      pos = strchr(txt, 13);
      #ifdef DEBUG_ICCTAG
      //cout << (int)pos << " "; DBG
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
    #ifdef DEBUG_ICCTAG
    cout << (int)text.find('\r') << " "; DBG
    #endif
    while ((int)text.find('\r') > 0) { // \r 013 0x0d
      pos = (int)text.find('\r');
      #ifdef DEBUG_ICCTAG
      cout << pos << " "; DBG
      #endif
      if (pos > 0)
        //text.erase (pos);
        text.replace (pos, 1, ' ', 1); 
      count++;
    };
  #endif
    texte.push_back( text );

  } else if ( getTypName() == "vcgt" ) {

    texte.push_back( _("Rot") );
    texte.push_back( _("Grün") );
    texte.push_back( _("Blau") );
    texte.push_back( "gamma_start_ende" );

  } else {

    texte.push_back( getTypName() + " | <- iss'n das?" );
  }
    
  #ifdef DEBUG_ICCTAG
  cout << count << " Ersetzungen "; DBG
  cout << " " << "" << "|" << getTypName() << "|" << text << " "; DBG
  #endif

  DBG_PROG_ENDE
  return texte;
}

std::vector<std::string>
ICCtag::getDescription              (void)
{ DBG_PROG_START
  std::vector<std::string> texte;
  std::string text =  "";
  icUInt32Number count = *(icUInt32Number*)(_data+8);

  text.append ((const char*)(_data+12), icValue(count));
  texte.push_back (text);
  #ifdef DEBUG_ICCTAG
  cout << &_data[12] << "|" << "|" << text << " "; DBG
  #endif
  DBG_PROG_ENDE
  return texte;
}

std::vector<double>
ICCtag::getCIEXYZ                                 (void)
{ DBG_PROG_START
  std::vector<double> punkte;
  icTagBase *base  = (icTagBase*)(&_data[0]);

  if ((base->sig) == (icTagTypeSignature)icValue( icSigChromaticityType )) {
    int count = icValue(*(icUInt16Number*)&_data[8]);
    if (count == 0)
      count = 3;
    #ifdef DEBUG_ICCTAG
    cout << count << " "; DBG
    #endif
    for (int i = 0; i < count ; i++) { // Table 35 -- chromaticityType encoding
      // TODO lcms braucht einen 16 Byte Offset (statt 12 Byte)
      icU16Fixed16Number* channel = (icU16Fixed16Number*)&_data[12+(4*i)];
      double xyz[3] = { icUFValue( channel[0] ),
                        icUFValue( channel[1] ),
                        1.0 - (icUFValue(channel[0]) + icUFValue(channel[1])) };
      punkte.push_back( xyz[0] );
      punkte.push_back( xyz[1] );
      punkte.push_back( xyz[2] );
      #ifdef DEBUG_ICCTAG
      cout << xyz[0] << ", " << xyz[1] << ", " << xyz[2] << " "; DBG
      #endif
    }
  } else if (base->sig == (icTagTypeSignature)icValue( icSigXYZType )) {
    icXYZType *daten = (icXYZType*) &_data[0];
    punkte.push_back( icSFValue( (daten->data.data[0].X) ) );
    punkte.push_back( icSFValue( (daten->data.data[0].Y) ) );
    punkte.push_back( icSFValue( (daten->data.data[0].Z) ) );
  }

  DBG_PROG_ENDE
  return punkte;
}

std::vector<double>
ICCtag::getCurve                                  (void)
{ DBG_PROG_START
  std::vector<double> punkte;
  icCurveType *daten = (icCurveType*) &_data[0];
  int count = icValue(daten->curve.count);

  if (count == 1) { // icU16Fixed16Number
    punkte.push_back (icValue(daten->curve.data[0])/256.0);
  } else {
    for (int i = 0; i < count; i++)
      punkte.push_back (icValue(daten->curve.data[i])/65536.0);
  }

  DBG_PROG_ENDE
  return punkte;
}

std::vector<std::vector<double> >
ICCtag::getCurves                                 (MftChain typ)
{ DBG_PROG_START
  std::vector<double> kurve;
  std::vector<std::vector<double> > kurven; DBG
  // Wer sind wir?
  if (getTypName() == "mft2") {
    icLut16* lut16 = (icLut16*) &_data[8];
    int inputChan, outputChan, clutPoints, inputEnt, outputEnt;
    inputChan = (int)lut16->inputChan;
    outputChan = (int)lut16->outputChan;
    clutPoints = (int)lut16->clutPoints;
    inputEnt = icValue(lut16->inputEnt);
    outputEnt = icValue(lut16->outputEnt);
    int feldPunkte = (int)pow((double)clutPoints, inputChan);
    #ifdef DEBUG_ICCTAG
    cout << feldPunkte << " Feldpunkte " << clutPoints << " clutPoints "; DBG
    #endif
    int start = 52,
        byte  = 2;
    double div   = 65536.0;
    DBG_PROG
    // Was wird verlangt?
    switch (typ) {
    case MATRIX:
         break;
    case CURVE_IN: DBG
         for (int j = 0; j < inputChan; j++)
         { kurve.clear();
           #ifdef DEBUG_ICCTAG
           cout << kurve.size() << " Start "; DBG
           #endif
           for (int i = inputEnt * j; i < inputEnt * (j+1); i++) {
             kurve.push_back( (double)icValue (*(icUInt16Number*)&_data[start + byte*i])
                              / div );
             #ifdef DEBUG_ICCTAG
             cout << icValue (*(icUInt16Number*)&_data[start + byte*i]) << " "; DBG
             #endif
           }
           kurven.push_back (kurve);
           #ifdef DEBUG_ICCTAG
           cout << kurve.size() << " Einträge "; DBG
           #endif
         } DBG_PROG
         break;
    case TABLE: DBG_PROG
         start += (inputChan * inputEnt) * byte;
         for (int i = 0; i < feldPunkte * outputChan; i++)
           kurve.push_back( (double)icValue (*(icUInt16Number*)&_data[start + byte*i])
                            / div );
         break;
    case CURVE_OUT: DBG_PROG
         start += (inputChan * inputEnt + feldPunkte * outputChan) * byte;
         for (int j = 0; j < outputChan; j++)
         { kurve.clear();
           for (int i = outputEnt * j; i < outputEnt * (j+1); i++)
             kurve.push_back( (double)icValue (*(icUInt16Number*)&_data[start + byte*i])
                              / div );
           kurven.push_back (kurve);
           #ifdef DEBUG_ICCTAG
           cout << kurve.size() << "|" << outputEnt << " Einträge "; DBG
           #endif
         }
         break;
    } 
  } else if (getTypName() == "mft1") {
    icLut8* lut8 = (icLut8*) &_data[8];
    int inputChan, outputChan, clutPoints, inputEnt=256, outputEnt=256;
    inputChan = (int)lut8->inputChan;
    outputChan = (int)lut8->outputChan;
    clutPoints = (int)lut8->clutPoints;
    int feldPunkte = (int)pow((double)clutPoints, inputChan);
    int start = 48,
        byte  = 1;
    double div   = 255.0;

    // Was wird verlangt?
    switch (typ) {
    case MATRIX:
         break;
    case CURVE_IN:
         for (int j = 0; j < inputChan; j++)
         { kurve.clear();
           #ifdef DEBUG_ICCTAG
           cout << kurve.size() << " Start "; DBG
           #endif
           for (int i = inputEnt * j; i < inputEnt * (j+1); i++)
             kurve.push_back( (double) *(icUInt8Number*)&_data[start + byte*i]
                              / div );
           kurven.push_back (kurve);
           #ifdef DEBUG_ICCTAG
           cout << kurve.size() << " Einträge "; DBG
           #endif
         }
         break;
    case TABLE:
         start += (inputChan * inputEnt) * byte;
         for (int i = 0; i < feldPunkte * outputChan; i++)
           kurve.push_back( (double) *(icUInt8Number*)&_data[start + byte*i]
                            / div );
         break;
    case CURVE_OUT:
         start += (inputChan * inputEnt + feldPunkte * outputChan) * byte;
         for (int j = 0; j < outputChan; j++)
         { kurve.clear();
           #ifdef DEBUG_ICCTAG
           cout << kurve.size() << " Start "; DBG
           #endif
           for (int i = outputEnt * j; i < outputEnt * (j+1); i++)
             kurve.push_back( (double) *(icUInt8Number*)&_data[start + byte*i]
                              / div );
           kurven.push_back (kurve);
           #ifdef DEBUG_ICCTAG
           cout << kurve.size() << " Einträge "; DBG
           #endif
         }
         break;
    } 
  } else if (getTypName() == "vcgt") {
    int parametrisch        = icValue(*(icUInt32Number*) &_data[8]);
    icUInt16Number nkurven  = icValue(*(icUInt16Number*) &_data[12]);
    icUInt16Number segmente = icValue(*(icUInt16Number*) &_data[14]);
    icUInt16Number byte     = icValue(*(icUInt16Number*) &_data[16]);
    
    #ifdef DEBUG_ICCTAG
    cout << _data << " parametrisch " << parametrisch << " nkurven " << nkurven << " segmente " << segmente << " byte " << byte << " "; DBG
    #endif

    if (parametrisch) { //icU16Fixed16Number
      double r_gamma = 1.0/icValue(*(icUInt32Number*)&_data[12])*65536.0;
      double start_r = icValue(*(icUInt32Number*)&_data[16])/65536.0;
      double ende_r = icValue(*(icUInt32Number*)&_data[20])/65536.0;
      double g_gamma = 1.0/icValue(*(icUInt32Number*)&_data[24])*65536.0;
      double start_g = icValue(*(icUInt32Number*)&_data[28])/65536.0;
      double ende_g = icValue(*(icUInt32Number*)&_data[32])/65536.0;
      double b_gamma = 1.0/icValue(*(icUInt32Number*)&_data[36])*65536.0;
      double start_b = icValue(*(icUInt32Number*)&_data[40])/65536.0;
      double ende_b = icValue(*(icUInt32Number*)&_data[44])/65536.0;
      kurve.clear();
      kurve.push_back(r_gamma);
      kurve.push_back(start_r);
      kurve.push_back(ende_r); kurven.push_back (kurve); kurve.clear();
      kurve.push_back(g_gamma);
      kurve.push_back(start_g);
      kurve.push_back(ende_g); kurven.push_back (kurve); kurve.clear();
      kurve.push_back(b_gamma);
      kurve.push_back(start_b);
      kurve.push_back(ende_b); kurven.push_back (kurve); kurve.clear();
    } else {
      int start = 18;
      double div   = 65536.0;
           for (int j = 0; j < nkurven; j++)
           { kurve.clear();
             #ifdef DEBUG_ICCTAG
             cout << kurve.size() << " Start "; DBG
             #endif
             for (int i = segmente * j; i < segmente * (j+1); i++)
               kurve.push_back( (double) icValue (*(icUInt16Number*)&_data[start + byte*i])
                                / div );
             kurven.push_back (kurve);
             #ifdef DEBUG_ICCTAG
             cout << kurve.size() << " Einträge "; DBG
             #endif
           }
    }
  }

  #ifdef DEBUG_ICCTAG
  cout << kurven.size() << " "; DBG
  #endif
  DBG_PROG_ENDE
  return kurven;
}

std::vector<std::vector<std::vector<std::vector<double> > > >
ICCtag::getTable                                 (MftChain typ)
{ DBG_PROG_START
  std::vector<std::vector<std::vector<std::vector<double> > > > Tabelle;
  std::vector<double> Farbe; DBG
  // Wer sind wir?
  if (getTypName() == "mft2") {
    icLut16* lut16 = (icLut16*) &_data[8];
    int inputChan, outputChan, clutPoints, inputEnt, outputEnt;
    inputChan = (int)lut16->inputChan;
    outputChan = (int)lut16->outputChan;
    clutPoints = (int)lut16->clutPoints;
    inputEnt = icValue(lut16->inputEnt);
    outputEnt = icValue(lut16->outputEnt);
    int feldPunkte = (int)pow((double)clutPoints, inputChan);
    #ifdef DEBUG_ICCTAG
    cout << feldPunkte << " Feldpunkte " << clutPoints << " clutPoints "; DBG
    #endif
    int start = 52,
        byte  = 2;
    double div   = 65536.0;
    DBG_PROG
    // Was wird verlangt?
    switch (typ) {
    case TABLE: { DBG_PROG
         start += (inputChan * inputEnt) * byte;
         Tabelle.resize(clutPoints);
         for (int i = 0; i < clutPoints; i++) {
           Tabelle[i].resize(clutPoints);
           for (int j = 0; j < clutPoints; j++) {
             Tabelle[i][j].resize(clutPoints);
             for (int k = 0; k < clutPoints; k++)
               Tabelle[i][j][k].resize(outputChan);
           }
         }
         int n = 0;
         for (int i = 0; i < clutPoints; i++) {
           Tabelle[i].resize(clutPoints);
           for (int j = 0; j < clutPoints; j++) {
             Tabelle[i][j].resize(clutPoints);
             for (int k = 0; k < clutPoints; k++) {
               Tabelle[i][j][k].resize(outputChan);
               for (int l = 0; l < outputChan; l++) {
                 Tabelle[i][j][k][l] = (double)icValue (*(icUInt16Number*)&_data[start + byte*n++])
                            / div;
               }
             }
           }
         }
         }
         break;
    case MATRIX:
    case CURVE_IN:
    case CURVE_OUT: DBG_PROG
         break;
    } 
  } else if (getTypName() == "mft1") {
    icLut8* lut8 = (icLut8*) &_data[8];
    int inputChan, outputChan, clutPoints, inputEnt=256, outputEnt=256;
    inputChan = (int)lut8->inputChan;
    outputChan = (int)lut8->outputChan;
    clutPoints = (int)lut8->clutPoints;
    int feldPunkte = (int)pow((double)clutPoints, inputChan);
    int start = 48,
        byte  = 1;
    double div   = 255.0;

    // Was wird verlangt?
    switch (typ) {
    case TABLE:
         start += (inputChan * inputEnt) * byte;
         for (int i = 0; i < feldPunkte * outputChan; i++)
           ;//Tabelle.push_back( (double) *(icUInt8Number*)&_data[start + byte*i]
              //              / div );
         break;
    case MATRIX:
    case CURVE_IN:
    case CURVE_OUT:
         break;
    } 
  }

  #ifdef DEBUG_ICCTAG
  cout << Tabelle.size() << " "; DBG
  #endif
  DBG_PROG_ENDE
  return Tabelle;
}

std::vector<double>
ICCtag::getNumbers                                 (MftChain typ)
{ DBG_PROG_START
  std::vector<double> nummern;
  // Wer sind wir?
  if (getTypName() == "mft2") {
    icLut16* lut16 = (icLut16*) &_data[8];
    int inputChan, outputChan, clutPoints, inputEnt, outputEnt;
    inputChan = (int)lut16->inputChan;
    outputChan = (int)lut16->outputChan;
    clutPoints = (int)lut16->clutPoints;
    inputEnt = icValue(lut16->inputEnt);
    outputEnt = icValue(lut16->outputEnt);

    // Was wird verlangt?
    switch (typ) {
    case MATRIX:
         for (int i = 0; i < 9; i++) {
           icS15Fixed16Number *n = (icS15Fixed16Number*)&_data[12 + 4*i];
           nummern.push_back( icSFValue (*n) );
         }
         break;
    case CURVE_IN:
    case TABLE:
    case CURVE_OUT:
         break;
    } 
  } else if (getTypName() == "mft1") {
    icLut8* lut8 = (icLut8*) &_data[8];
    int inputChan, outputChan, clutPoints;//, inputEnt, outputEnt;
    inputChan = (int)lut8->inputChan;
    outputChan = (int)lut8->outputChan;
    clutPoints = (int)lut8->clutPoints;

    // Was wird verlangt?
    switch (typ) {
    case MATRIX:
         for (int i = 0; i < 9; i++) {
           icS15Fixed16Number *n = (icS15Fixed16Number*)&_data[12 + 4*i];
           nummern.push_back( icSFValue (*n) );
         }
         break;
    case CURVE_IN:
    case TABLE:
    case CURVE_OUT:
         break;
    } 
  }

  #ifdef DEBUG_ICCTAG
  cout << nummern.size() << " "; DBG
  #endif
  DBG_PROG_ENDE
  return nummern;
}

std::vector<std::string>
ICCtag::getText                     (MftChain typ)
{ DBG_PROG_START
  std::vector<std::string> texte;

  // TODO: prüfen auf icColorSpaceSignature <-> Kanalanzahl
    // Was wird verlangt?
    switch (typ) {
    case MATRIX:
         for (int i = 0; i < 9; i++) {
           texte.push_back( "" );
         }
         break;
    case CURVE_IN:
         texte = getChannelNames (_color_in);
         break;
    case TABLE:
         if (_to_pcs)
           texte = getChannelNames (_color_in);
         else
           texte = getChannelNames (_color_out);
         break;
    case CURVE_OUT:
         texte = getChannelNames (_color_out);
         break;
    }

  DBG_PROG_ENDE
  return texte;
}


/**
  *  @brief ICCprofile Funktionen
  */

ICCprofile::ICCprofile (void)
{ DBG_PROG_START
  _data = NULL;
  _size = 0;
  DBG_PROG_ENDE
}

ICCprofile::ICCprofile (const char *filename)
  : _filename (filename)
{ DBG_PROG_START
  if (_data && _size) free(_data);//delete [] _data;
  _data = NULL;
  _size = 0;

  // delegieren
  _filename = filename;
  try {
    fload ();
  }
    catch (Ausnahme & a) {	// fängt alles von Ausnahme Abstammende
        printf ("Ausnahme aufgetreten: %s\n", a.what());
        a.report();
        _filename = "";
    }
    catch (std::exception & e) { // fängt alles von exception Abstammende
        printf ("Std-Ausnahme aufgetreten: %s\n", e.what());
        _filename = "";
    }
    catch (...) {		// fängt alles Übriggebliebene
        printf ("Huch, unbekannte Ausnahme\n");
        _filename = "";
    }
  DBG_PROG_ENDE
}

ICCprofile::~ICCprofile (void)
{ DBG_PROG_START
  this->clear();

  #ifdef DEBUG_PROFILE
  DBG_S ( "~ICCprofile beendet" )
  #endif
  DBG_PROG_ENDE
}
 
void
ICCprofile::clear (void)
{ DBG_PROG_START
  DBG_PROG_S( "Profil wird geleert" )

  if (_data && _size)
    free(_data);

  _data = NULL;
  _size = 0;
  _filename = "";
  header.load(NULL);

  tags.clear();
  if (_data != NULL && _size) free(_data);

  measurement.clear();

  #ifdef DEBUG_PROFILE
  cout << "_data, tags und measurement gelöscht"; DBG
  #endif
  DBG_PROG_ENDE
}

void
ICCprofile::load (std::string filename)
{ DBG_PROG_START
  // delegieren
  _filename = filename;
  try {
    fload ();
  }
    catch (Ausnahme & a) {	// fängt alles von Ausnahme Abstammende
        printf ("Ausnahme aufgetreten: %s\n", a.what());
        a.report();
        _filename = "";
    }
    catch (std::exception & e) { // fängt alles von exception Abstammende
        printf ("Std-Ausnahme aufgetreten: %s\n", e.what());
        _filename = "";
    }
    catch (...) {		// fängt alles Übriggebliebene
        printf ("Huch, unbekannte Ausnahme\n");
        _filename = "";
    }
  DBG_PROG_ENDE
}

void
ICCprofile::load (char* filename)
{ DBG_PROG_START
  // delegieren
  _filename = filename;
  try {
    fload ();
  }
    catch (Ausnahme & a) {	// fängt alles von Ausnahme Abstammende
        printf ("Ausnahme aufgetreten: %s\n", a.what());
        a.report();
        _filename = "";
    }
    catch (std::exception & e) { // fängt alles von exception Abstammende
        printf ("Std-Ausnahme aufgetreten: %s\n", e.what());
        _filename = "";
    }
    catch (...) {		// fängt alles Übriggebliebene
        printf ("Huch, unbekannte Ausnahme\n");
        _filename = "";
    }
  DBG_PROG_ENDE
}

void
ICCprofile::fload ()
{ DBG_PROG_START // ICC Profil laden
  std::string file;
  std::ifstream f ( _filename.c_str(), std::ios::binary | std::ios::ate );

  DBG_PROG
  if (_filename == "")
    throw ausn_file_io ("kein Dateiname angegeben");
  DBG_PROG
  if (!f) {
    throw ausn_file_io ("keine lesbare Datei gefunden");
    _filename = "";
  }

  if (_data != NULL && _size) {
    cout << "!!!! Profil wird wiederbenutzt !!!! "; DBG_PROG
    clear();
  }
  _size = (unsigned int)f.tellg();         f.seekg(0);
  _data = (char*)calloc (sizeof (char), _size);
  
  f.read ((char*)_data, _size);
  #ifdef DEBUG_ICCPROFILE
  cout << _size << "|" << f.tellg() << endl;
  #endif
  f.close();

  DBG_PROG
  //Kopf
  header.load ((void*)_data); DBG_PROG

  //Profilabschnitte
  // TagTabelle bei 132 abholen
  icTag *tagList = (icTag*)&((char*)_data)[132];
  //(icTag*) new char ( getTagCount() * sizeof (icTag));
  //memcpy (tagList , &((char*)_data)[132], sizeof (icTag) * getTagCount());
  DBG_PROG
  tags.resize(getTagCount()); DBG_PROG
  for (int i = 0 ; i < getTagCount() ; i++) { DBG_PROG
    tags[i].load( this, &tagList[i] ,
              &((char*)_data)[ icValue(tagList[i].offset) ]); DBG_PROG
    #ifdef DEBUG_ICCPROFILE
    cout << " sig: " << tags[i].getTagName() << " " << i << " "; DBG_PROG
    #endif

    // bekannte Tags mit Messdaten
    if (tags[i].getTagName() == ("targ")
     || tags[i].getTagName() == ("DevD")
     || tags[i].getTagName() == ("CIED")) {
      #ifdef DEBUG_ICCPROFILE
      cout << "Messdaten gefunden " << tags[i].getTagName() << " "; DBG_PROG
      #endif
      measurement.load( this, tags[i] );
    }
  }
  #ifdef DEBUG_ICCPROFILE
  cout << "TagCount: " << getTagCount() << " / " << tags.size() << " ";DBG
  #endif

  DBG_PROG_ENDE
}

std::vector<std::string>
ICCprofile::printTagInfo         (int item)
{ DBG_PROG_START
  std::vector<std::string> liste;

  liste.push_back( tags.at(item).getTagName() );
  liste.push_back( tags.at(item).getTypName() );

  DBG_PROG_ENDE
  return liste;
}

std::vector<std::string>
ICCprofile::printTags            ()
{ DBG_PROG_START
  std::vector<std::string> StringList;
  std::string text;
  std::stringstream s;

  if (!tags.size()) { DBG_MEM
    return StringList;
  } DBG_MEM

  int count = 0;
  for (std::vector<ICCtag>::iterator it = tags.begin(); it != tags.end(); it++){
    s.str("");
    s << count; count++;           StringList.push_back(s.str()); s.str("");
    s.str((*it).getTagName());     StringList.push_back(s.str()); s.str("");
    s.str((*it).getTypName());     StringList.push_back(s.str()); s.str("");
    s << (*it).getSize();          StringList.push_back(s.str()); s.str("");
    s.str((*it).getInfo()); StringList.push_back(s.str()); s.str("");
  #ifdef DEBUG_ICCPROFILE
    cout << (*it).getTagName() << " " << count << " "; DBG
  #endif
  }
  DBG_PROG_ENDE
  return StringList;
}

std::vector<std::string>
ICCprofile::getTagText                                  (int item)
{ DBG_PROG_START
  // Prüfen
  std::string name = tags[item].getTypName();
  std::string leer = name + " Typ - keine Textausgabe";
  std::vector<std::string> v;
  v.push_back( leer );

  if (name != "chrm"
   && name != "cprt?"
   && name != "dtim"
   && name != "meas"
   && name != "mft1"
   && name != "mft2"
   && name != "sig"
   && name != "text"
   && name != "vcgt"
   && name != "XYZ")
    return v;

  DBG_PROG_ENDE
  return tags.at(item).getText();
}

std::vector<std::string>
ICCprofile::getTagChannelNames                          (int item,
                                                         ICCtag::MftChain typ)
{ DBG_PROG_START
  // Prüfen
  std::string leer = tags[item].getTypName() + " Typ - keine Textausgabe";
  std::vector<std::string> v;
  v.push_back( leer );

  // Prüfen
  if (tags[item].getTypName() != "mft2"
   && tags[item].getTypName() != "mft1")
    return v;

  DBG_PROG_ENDE
  return tags.at(item).getText(typ);
}

std::vector<std::string>
ICCprofile::getTagDescription                    (int item)
{ DBG_PROG_START
  // Prüfen
  std::vector<std::string> leer;
  if (tags[item].getTypName() != "desc")
    return leer;

  DBG_PROG_ENDE
  return tags.at(item).getDescription();
}

std::vector<double>
ICCprofile::getTagCIEXYZ                         (int item)
{ DBG_PROG_START
  // Prüfen
  std::vector<double> XYZ;

  if ( tags[item].getTypName() == "XYZ"
    || tags[item].getTypName() == "chrm")
    XYZ = tags.at(item).getCIEXYZ();

  DBG_PROG_ENDE
  return XYZ;
}

std::vector<double>
ICCprofile::getTagCurve                          (int item)
{ DBG_PROG_START
  // Prüfen
  std::vector<double> leer;
  if (tags[item].getTypName() != "curv")
  {
    #ifdef DEBUG_ICCPROFILE
    cout << tags[item].getTypName() << " "; DBG
    #endif
    DBG_PROG_ENDE
    return leer;
  }

  DBG_PROG_ENDE
  return tags.at(item).getCurve();
}

std::vector<std::vector<double> >
ICCprofile::getTagCurves                         (int item,ICCtag::MftChain typ)
{ DBG_PROG_START
  // Prüfen
  std::vector<std::vector<double> > leer;
  if (tags[item].getTypName() != "mft2"
   && tags[item].getTypName() != "mft1"
   && tags[item].getTypName() != "vcgt")
  {
    #ifdef DEBUG_ICCPROFILE
    cout << "gibt nix für " << tags[item].getTypName() << " "; DBG
    #endif
    DBG_PROG_ENDE
    return leer;
  }

  DBG_PROG_ENDE
  return tags.at(item).getCurves(typ);
}

std::vector<std::vector<std::vector<std::vector<double> > > >
ICCprofile::getTagTable                         (int item,ICCtag::MftChain typ)
{ DBG_PROG_START
  // Prüfen
  std::vector<std::vector<std::vector<std::vector<double> > > > leer;
  if (tags[item].getTypName() != "mft2"
   && tags[item].getTypName() != "mft1")
  {
    #ifdef DEBUG_ICCPROFILE
    cout << "gibt nix für " << tags[item].getTypName() << " "; DBG
    #endif
    DBG_PROG_ENDE
    return leer;
  }

  DBG_PROG_ENDE
  return tags.at(item).getTable(typ);
}

std::vector<double>
ICCprofile::getTagNumbers                        (int item,ICCtag::MftChain typ)
{ DBG_PROG_START
  // Prüfen
  std::vector<double> leer;
  if (tags[item].getTypName() != "mft2"
   && tags[item].getTypName() != "mft1")
  {
    #ifdef DEBUG_ICCPROFILE
    cout << tags[item].getTypName() << " "; DBG
    #endif
    DBG_PROG_ENDE
    return leer;
  }
  DBG_PROG_ENDE
  return tags.at(item).getNumbers(typ);
}

int
ICCprofile::getTagByName            (std::string name)
{ DBG_PROG_START
  if (!tags.size()) { DBG_MEM
    return -1;
  } DBG_MEM

  int item = 0;
  for (std::vector<ICCtag>::iterator it = tags.begin(); it != tags.end(); it++){
    if ( (*it).getTagName() == name
      && (*it).getSize()            ) {
      #ifdef DEBUG_ICCPROFILE
      cout << item << "=" << (*it).getTagName() << " gefunden "; DBG
      #endif
      DBG_PROG_ENDE
      return item;
    }
    item++;
  }
  DBG_PROG_ENDE
  return -1;
}

bool
ICCprofile::hasTagName            (std::string name)
{ DBG_PROG_START
  if (!tags.size()) { DBG_PROG_ENDE
    return false;
  } DBG_PROG

  int item = 0;
  for (std::vector<ICCtag>::iterator it = tags.begin(); it != tags.end(); it++){
    if ( (*it).getTagName() == name
      && (*it).getSize()            ) {
      #ifdef DEBUG_ICCPROFILE
      cout << (*it).getTagName() << " gefunden "; DBG
      #endif
      return true;
    }
    item++;
  }
  DBG_PROG_ENDE
  return false;
}

std::vector<double>
ICCprofile::getWhitePkt           (void)
{ DBG_PROG_START
  std::vector<double> XYZ;
  if (hasTagName ("wtpt"))
    XYZ = getTagCIEXYZ (getTagByName ("wtpt"));
 
  DBG_PROG_ENDE
  return XYZ;
}

void
ICCprofile::saveProfileToFile  (char* filename)
{ DBG_PROG_START
  if (_data && _size) free(_data);//delete []_data;
  _size = sizeof (icHeader) + sizeof (icUInt32Number); DBG_MEM_V(_size <<" "<<sizeof (icProfile))
  _data = (char*)calloc (sizeof (char) , _size); //new char (sizeof(icHeader) );
  writeTagTable ();
  writeTags ();
  header.size(_size); DBG_V (_size )
  writeHeader ();

  std::ofstream f ( filename,  std::ios::out );
  f.write ( (char*)_data, _size );
  f.close();
  DBG_PROG_ENDE
}

int
ICCprofile::getProfileSize  ()
{ DBG_PROG_START
  if (_data && _size) free(_data);//delete []_data;
  _size = sizeof (icHeader) + sizeof (icUInt32Number); DBG_V(_size <<" "<<sizeof (icProfile))
  _data = (char*)calloc (sizeof (char) , _size); //new char (sizeof(icHeader) );
  writeTagTable ();
  writeTags ();
  header.size(_size); DBG_V (_size )
  writeHeader ();

  DBG_MEM_V( _size )
  DBG_PROG_ENDE
  return _size;
}

char*
ICCprofile::saveProfileToMem  (int *size)
{ DBG_PROG_START
  if (_data && _size) free(_data);//delete []_data;
  _size = sizeof (icHeader) + sizeof (icUInt32Number); DBG_V(_size <<" "<<sizeof (icProfile))
  _data = (char*)calloc (sizeof (char) , _size); //new char (sizeof(icHeader) );
  writeTagTable ();
  writeTags ();
  header.size(_size); DBG_V (_size )
  writeHeader ();

  char *block = (char*)calloc (sizeof (char) , _size);
  memcpy (block, _data, _size);
  *size = _size;
  DBG_MEM_V( _size )
  DBG_PROG_ENDE
  return block;
}

void
ICCprofile::writeTags (void)
{ DBG_PROG_START
  unsigned int i;
  for (i = 0; i < tags.size(); i++) {
    DBG_NUM_S ( i << ": " << tags[i].getTypName() )
    int size;
    const char* data = tags[i].write(&size);
    icTagList* list = (icTagList*)&((char*)_data)[128];
    
    list->tags[i].sig = icValue((icTagSignature)tags[i].getSignature());
    list->tags[i].offset = icValue((icUInt32Number)_size); DBG_MEM_V (icValue(list->tags[i].offset))
    list->tags[i].size = icValue((icUInt32Number)size); DBG_MEM_V(_size)
    char* temp = (char*) calloc (sizeof(char), _size + size + 
                                               (size%4 ? 4 - size%4 : 0));
    memcpy (temp, _data, _size); DBG_V( _size<<" "<< size<<" "<<size%4 )
    memcpy (&temp[_size], data, size);
    _size = _size + size + (size%4 ? 4 - size%4 : 0);
    free(_data);//delete [] _data;
    _data = temp;
    list = (icTagList*)&temp[128]; DBG_V (icValue(list->tags[i].offset))
    DBG_MEM_V (icValue(list->tags[i].sig))
    DBG_MEM_V (icValue(list->tags[i].size) << " " << (int)&_data[0])
  }
  DBG_MEM_V( _size )
  DBG_PROG_ENDE
}

void
ICCprofile::writeTagTable (void)
{ DBG_PROG_START
  icProfile* p = (icProfile*) _data;
  p->count = icValue((icUInt32Number)tags.size());
  int size = sizeof (icTag) * tags.size();
  char* data = (char*) calloc (sizeof(char), size + _size);
  memcpy (data, _data, _size);
  _size = _size + size;
  free(_data);//delete [] _data;
  _data = data;
  DBG_PROG_ENDE
}

void
ICCprofile::writeHeader (void)
{ DBG_PROG_START
  memcpy (_data, header.header_raw(), 128);
  DBG_PROG_ENDE
}



int
ICCprofile::checkProfileDevice (char* type, icProfileClassSignature deviceClass)
{ DBG_PROG_START
  int check = true;

  if ((strcmp(type, _("Work Space"))) == 0) {
      // test for device class
      switch (deviceClass)
        {
        case icSigInputClass:
        case icSigDisplayClass:
        case icSigOutputClass:
        case icSigLinkClass:
        case icSigAbstractClass:
        case icSigColorSpaceClass:
        case icSigNamedColorClass:
          // should be good
          break;
        default:
          g_message ("%s \"%s %s: %s",_("Your"), type,
                     _("Profile\" is designed for an other device"),
                     getDeviceClassName(deviceClass).c_str());
          check = false;
          break;
        }
  } else if ((strcmp(type, _("Separation"))) == 0) {
      switch (deviceClass)
        {
//        case icSigInputClass:
//        case icSigDisplayClass:
        case icSigOutputClass:
        case icSigLinkClass:
        case icSigAbstractClass:
        case icSigColorSpaceClass:
        case icSigNamedColorClass:
          // should be good
          break;
        default:
          g_message ("%s - %s - %s \"%s %s",_("Device class"),
                     getDeviceClassName(deviceClass).c_str(),
                     _("is not valid for an"),
                     type,
                     _("Profile\"."));
          check = false;
          break;
        if (icSigCmykData   != header.colorSpace())
          check = false;
        }
  } else if ((strcmp(type, _("Linearisation"))) == 0) {
      switch (deviceClass)
        {
//        case icSigInputClass:
//        case icSigDisplayClass:
//        case icSigOutputClass:
        case icSigLinkClass:
//        case icSigAbstractClass:
//        case icSigColorSpaceClass:
//        case icSigNamedColorClass:
          // should be good
          break;
        default:
          g_message ("%s - %s - %s \"%s %s",_("Device class"),
                     getDeviceClassName(deviceClass).c_str(),
                     _("is not valid for an"),
                     type,
                     _("Profile\"."));
          check = false;
          break;
        if (icSigCmykData   != header.colorSpace())
          g_message ("%s - %s - %s \"%s %s",_("Color space"),
                     getColorSpaceName(header.colorSpace()).c_str(),
                     _("is not valid for an"),
                     type,
                     _("Profile at the moment\"."));
          check = false;
        }
  }
  DBG_PROG_ENDE
  return check;
}

void
ICCprofile::saveMemToFile (char* filename, char *block, int size)
{ DBG_PROG_START
  FILE *fp=NULL;
  int   pt = 0;

  if ((fp=fopen(filename, "w")) != NULL) {
    do {
      fputc ( block[pt++] , fp);
    } while (--size);
  }

  fclose (fp);
  DBG_PROG_ENDE
}

void
ICCprofile::removeTag (int item)
{ DBG_PROG_START
  if (item >= (int)tags.size() )
    return;

  std::vector <ICCtag> t(tags.size()-1); DBG
  DBG_V (tags.size())
  int i = 0,
      zahl = 0; DBG
  for (; i < (int)tags.size(); i++)
    if (i != item) { DBG
      t[zahl] = tags[i]; DBG_S("i: " << i << " -> zahl: " << zahl)
      zahl++; DBG
    }

  DBG_PROG
  #if 0
  tags.resize(t.size());
  for (int i = 0; i < (int)t.size(); i++)
    tags[i].copy (t[i]);
  #else
  tags = t;
  #endif
  DBG_MEM_V( i << " " << tags.size())
  DBG_PROG_ENDE
}

/**
  *  allgemeine Funktionen
  */

void
lcms_error (int ErrorCode, const char* ErrorText)
{ DBG_PROG_START
   g_message ("LCMS error:%d %s", ErrorCode, ErrorText);
  DBG_PROG_ENDE
}

const char* cp_nchar (char* text, int n)
{ DBG_PROG_START
  static char string[1024];

  for (int i = 0; i < 1024 ; i++)
    string[i] = '\000';

  if (n < 1024)
    snprintf(&string[0], n, text);
  else
    return NULL;

  #ifdef DEBUG
  DBG_NUM_V( n << " Buchstaben kopieren " <<  (int)text << " " << string)
  #endif
  DBG_PROG_ENDE
  return &string[0];
}


