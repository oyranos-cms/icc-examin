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

unsigned int
icValue (icUInt16Number val)
{
#if BYTE_ORDER == LITTLE_ENDIAN
  #define BYTES 2
  #define KORB  4
  unsigned char        *temp  = (unsigned char*) &val;
  static unsigned char  korb[KORB];
  for (int i = 0; i < KORB ; i++ )
    korb[i] = (int) 0;

  int klein = 0,
      gross = BYTES - 1;
  for (; klein < BYTES ; klein++ )
    korb[klein] = temp[gross--];

  unsigned int *erg = (unsigned int*) &korb[0];

  #ifdef DEBUG_ICCFUNKT
  cout << *erg << " Größe nach Wandlung " << (int)korb[0] << " "
       << (int)korb[1] << " " << (int)korb[2] << " " <<(int)korb[3]
       << " "; DBG
  #endif
  return (long)*erg;
#else
  return (long)val;
#endif
}

unsigned int
icValue (icUInt32Number val)
{
#if BYTE_ORDER == LITTLE_ENDIAN
  unsigned char        *temp = (unsigned char*) &val;

  static unsigned char  uint32[4];

  uint32[0] = temp[3];
  uint32[1] = temp[2];
  uint32[2] = temp[1];
  uint32[3] = temp[0];

  unsigned int *erg = (unsigned int*) &uint32[0];

  #ifdef DEBUG_ICCFUNKT
  cout << *erg << " Größe nach Wandlung " << (int)temp[0] << " "
       << (int)temp[1] << " " << (int)temp[2] << " " <<(int)temp[3]
       << " "; DBG
  #endif

  return (int) *erg;
#else
  #ifdef DEBUG_ICCFUNKT
  cout << "BIG_ENDIAN" << " "; DBG
  #endif
  return (int)val;
#endif
}

unsigned long
icValue (icUInt64Number val)
{
#if BYTE_ORDER == LITTLE_ENDIAN
  unsigned char        *temp  = (unsigned char*) &val;

  static unsigned char  uint64[8];
  int little = 0,
      big    = 8;

  for (; little < 8 ; little++ ) {
    uint64[little] = temp[big--];
  }

  unsigned long *erg = (unsigned long*) &uint64[0];

  #ifdef DEBUG_ICCFUNKT
  cout << *erg << " Größe nach Wandlung " << (int)temp[0] << " "
       << (int)temp[1] << " " << (int)temp[2] << " " <<(int)temp[3]
       << " "; DBG
  #endif
  return (long)*erg;
#else
  return (long)val;
#endif
}

signed int
icValue (icInt32Number val)
{
#if BYTE_ORDER == LITTLE_ENDIAN
  #define BYTES 4
  #define KORB  4
  unsigned char        *temp  = (unsigned char*) &val;
  static unsigned char  korb[KORB];
  for (int i = 0; i < KORB ; i++ )
    korb[i] = (int) 0;

  int klein = 0,
      gross = BYTES - 1;
  for (; klein < BYTES ; klein++ )
      korb[klein] = temp[gross--];

  signed int *erg = (signed int*) &korb[0];

  #ifdef DEBUG_ICCFUNKT
  cout << *erg << " Größe nach Wandlung " << (int)korb[0] << " "
       << (int)korb[1] << " " << (int)korb[2] << " " <<(int)korb[3]
       << " "; DBG
  #endif
  return (signed int)*erg;
#else
  return (signed int)val;
#endif
}

signed int
icValue (icInt16Number val)
{
#if BYTE_ORDER == LITTLE_ENDIAN
  #define BYTES 2
  #define KORB  4
  unsigned char        *temp  = (unsigned char*) &val;
  static unsigned char  korb[KORB];
  for (int i = 0; i < KORB ; i++ )
    korb[i] = (int) 0;

  int klein = 0,
      gross = BYTES - 1;
  for (; klein < BYTES ; klein++ )
      korb[klein] = temp[gross--];

  signed int *erg = (signed int*) &korb[0];
  #ifdef DEBUG_ICCFUNKT
  cout << *erg << " Größe nach Wandlung " << (int)korb[0] << " "
       << (int)korb[1] << " " << (int)korb[2] << " " <<(int)korb[3]
       << " "; DBG
  #endif
  return (signed int)*erg;
#else
  return (signed int)val;
#endif
}

double
icValueSF (icS15Fixed16Number val)
{
  return icValue(val) / 65536.0;
}

icColorSpaceSignature
icValue (icColorSpaceSignature val)
{
  icUInt32Number i = val;
  return (icColorSpaceSignature) icValue (i);
}

#define icValue_to_icUInt32Number(typ) \
typ \
icValue (typ val) \
{ \
  icUInt32Number i = val; \
  return (typ) icValue (i); \
}

icValue_to_icUInt32Number(icPlatformSignature)
icValue_to_icUInt32Number(icProfileClassSignature)
icValue_to_icUInt32Number(icTagSignature)
icValue_to_icUInt32Number(icTagTypeSignature)

// Farbkonvertierungen

double*
XYZto_xyY (double* XYZ)
{
  static double xyY[3];
  double summe = (XYZ[0] + XYZ[1] + XYZ[2]) + 0.0000001;

  xyY[0] = XYZ[0] / summe;
  xyY[1] = XYZ[1] / summe;
  xyY[2] = XYZ[2] / summe;

  return &xyY[0];
}

double*
XYZto_xyY (std::vector<double> XYZ)
{
  static double xyY[3];
  for (int i = 0 ; i < 3 ; i++)
    xyY[i] = 0.0;

  if (XYZ.size() == 3) {
    double summe = (XYZ[0] + XYZ[1] + XYZ[2]) + 0.0000001;

    xyY[0] = XYZ[0] / summe;
    xyY[1] = XYZ[1] / summe;
    xyY[2] = XYZ[2] / summe;
  }

  return &xyY[0];
}


/**
  *  @brief ICCheader Funktionen
  */ 

ICCheader::ICCheader()
{
  valid = false;
}

void
ICCheader::load (void *data)
{
  DBG
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
  DBG
}

std::string
ICCheader::renderingIntent (void)
{
  std::stringstream s;

  switch (header.renderingIntent) 
    {
    case 0:
      s << _("Fotographisch");
      break;
    case 1:
      s << _("relativ Farbmetrisch");
      break;
    case 2:
      s << _("Gesättigt");
      break;
    case 3:
      s << _("absolut Farbmetrisch");
      break;
    }
  return s.str();
}

std::string
ICCheader::attributes (void)
{
  std::stringstream s;
  char* f = (char*) &(header.attributes);
  unsigned char         maske1 = 0xf0;
  unsigned char         maske2 = 0x08;
  unsigned char         maske3 = 0x04;
  unsigned char         maske4 = 0x02;

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
  cout << f[0] << " " << (long)header.attributes; DBG
  char* ptr = (char*) &(header.attributes);
  for (int i = 0; i < 8 ; i++)
    cout << (int)ptr[i] << " ";
  DBG
  #endif
  return s.str();
}

std::string
ICCheader::flags (void)
{
  std::stringstream s;
  char* f = (char*) &(header.flags);
  unsigned char         maske1 = 0xf0;
  unsigned char         maske2 = 0x08;

  if (f[0] & maske1)
    s << _("Farbprofil ist eingebettet und ");
  else
    s << _("Farbprofil ist nicht eingebettet und ");

  if (f[0] & maske2)  
    s << _("kann nicht unabhängig vom Bild verwendet werden.");
  else
    s << _("kann unabhängig vom Bild verwendet werden.");

  #ifdef DEBUG
  cout << f[0] << " " << (long)header.flags; DBG
  char* ptr = (char*) &(header.flags);
  for (int i = 0; i < 8 ; i++)
    cout << (int)ptr[i] << " ";
  DBG
  #endif
  return s.str();
}

std::string
ICCheader::versionName (void)
{
  std::stringstream s;
  char* v = (char*)&(header.version);
  
  s << (int)v[0] << "." << (int)v[1]/16 << "." << (int)v[1]%16;

  return s.str();
}

std::string
ICCheader::print_long()
{
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
      <<  "    " << _("Datum") << ":       " <<
                       icValue(header.date.day)     << "/" <<
                       icValue(header.date.month)   << "/" <<
                       icValue(header.date.year)    << " " <<
                       icValue(header.date.hours)   << ":" <<
                       icValue(header.date.minutes) << " " << _("Uhr") << " " <<
                       icValue(header.date.seconds) << " " << _("Sekunden") << endl \
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
                       icValueSF(header.illuminant.X) << ", Y=" << \
                       icValueSF(header.illuminant.Y) << ", Z=" << \
                       icValueSF(header.illuminant.Z) << endl \
      <<  "    " << _("erzeugt von") << ": " << cr << endl ;
  } DBG
  return s.str();
}

std::string
ICCheader::print()
{
  string s = _("Dateikopf ungültig");
  if (valid)
    s = _("    Kopf        icHeader      128 Dateikopf");
  return s;
}


std::string
ICCheader::getColorSpaceName (icColorSpaceSignature color)
{
  std::string text;

  switch (color) {
    case icSigXYZData: text =_("XYZ"); break;
    case icSigLabData: text =_("Lab"); break;
    case icSigLuvData: text =_("Luv"); break;
    case icSigYCbCrData: text =_("YCbCr"); break;
    case icSigYxyData: text =_("Yxy"); break;
    case icSigRgbData: text =_("Rgb"); break;
    case icSigGrayData: text =_("Gray"); break;
    case icSigHsvData: text =_("Hsv"); break;
    case icSigHlsData: text =_("Hls"); break;
    case icSigCmykData: text =_("Cmyk"); break;
    case icSigCmyData: text =_("Cmy"); break;
    case icSig2colorData: text =_("2color"); break;
    case icSig3colorData: text =_("3color"); break;
    case icSig4colorData: text =_("4color"); break;
    case icSig5colorData: text =_("5color"); break;
    case icSig6colorData: text =_("6color"); break;
    case icSig7colorData: text =_("7color"); break;
    case icSig8colorData: text =_("8color"); break;
    case icSig9colorData: text =_("9color"); break;
    case icSig10colorData: text =_("10color"); break;
    case icSig11colorData: text =_("11color"); break;
    case icSig12colorData: text =_("12color"); break;
    case icSig13colorData: text =_("13color"); break;
    case icSig14colorData: text =_("14color"); break;
    case icSig15colorData: text =_("15color"); break;
    default: text =_(""); break;
  }
  return text;
}

std::string
ICCheader::getDeviceClassName (icProfileClassSignature deviceClass)
{
  std::string text;

  switch (deviceClass)
  {
    case icSigInputClass: text =_("Eingabe"); break;
    case icSigDisplayClass: text =_("Monitor"); break;
    case icSigOutputClass: text =_("Ausgabe"); break;
    case icSigLinkClass: text =_("Verknüpfung"); break;
    case icSigAbstractClass: text =_("Abstrakter Farbraum"); break;
    case icSigColorSpaceClass: text =_("Farbraum"); break;
    case icSigNamedColorClass: text =_("Schmuckfarben"); break;
    default: text =_(""); break;
  }
  return text;
}

std::string
ICCheader::getPlatformName (icPlatformSignature platform)
{
  std::string text;

  switch (platform)
  {
    case icSigMacintosh: text =_("Macintosh"); break;
    case icSigMicrosoft: text =_("Microsoft"); break;
    case icSigSolaris: text =_("Solaris"); break;
    case icSigSGI: text =_("SGI"); break;
    case icSigTaligent: text =_("Taligent"); break;
    default: text =_(""); break;
  }
  return text;
}

/**
  *  @brief ICCtag Funktionen
  */

ICCtag::ICCtag ()
{
  _sig = icMaxEnumTag;
  _data = 0; DBG
}

ICCtag::ICCtag                      (icTag* tag, char* data)
{
  ICCtag::load (tag, data); DBG
}

ICCtag::~ICCtag ()
{
  _sig = icMaxEnumTag;
  _size = 0;
  if (!_data) free (_data);
  DBG
}

void
ICCtag::load                        ( icTag *tag, char* data )
{
  _sig    = icValue(tag->sig);
  _size   = icValue(tag->size);

  if (!_data) free (_data);
  _data = (char*) calloc ( _size , sizeof (char) );
  memcpy ( _data , data , _size );
  DBG

  #ifdef DEBUG_ICCTAG
  char* text = _data;
  cout << _sig << "=" << tag->sig << " offset " << icValue(tag->offset) << " size " << _size << " nächster tag " << _size + icValue(tag->offset) << " " << text << " "; DBG
  #endif
}

std::string
ICCtag::getText                     (void)
{ DBG
  std::string text = "Fehl";

  if (getTypName() == "sig") {
    if (_size < 12) return text;
    icTechnologySignature tech;
    memcpy (&tech, &_data[8] , 4);
    text = getSigTechnology( (icTechnologySignature) icValue(tech) );
  } else if (getTypName() == "dtim") {
    if (_size < 20) return text;
    DBG
    std::stringstream s;
    icDateTimeNumber date;
    memcpy (&date, &_data[8] , 12);
    s << _("Datum") << ": " <<
                       icValue(date.day)     << "/" <<
                       icValue(date.month)   << "/" <<
                       icValue(date.year)    << " " <<
                       icValue(date.hours)   << ":" <<
                       icValue(date.minutes) << " " << _("Uhr") << " " <<
                       icValue(date.seconds) << " " << _("Sekunden");
    text = s.str();
  } else if (getTypName() == "meas") {
    if (_size < 36) return text;
    std::stringstream s;
    icMeasurement meas;
    memcpy (&meas, &_data[8] , 28);
    s << 
    getStandardObserver( (icStandardObserver)icValue( meas.stdObserver) ) <<endl
      << _("Rückseite") << ": X= " << icValueSF(meas.backing.X)
                        << ", Y= " << icValueSF(meas.backing.Y)
                        << ", Z= " << icValueSF(meas.backing.Z) << endl
      << _("Geometrie") << ": "<< 
    getMeasurementGeometry ((icMeasurementGeometry)icValue(meas.geometry))<<endl
      << _("Flare")     << ": "<< 
    getMeasurementFlare ((icMeasurementFlare)icValue(meas.flare)) << endl
      << _("Beleuchtungstyp") << ": " <<
    getIlluminant ((icIlluminant)icValue(meas.illuminant)) <<endl;
    text = s.str();
  } else {
    text = "";
    text.append (&_data[8], _size - 8);
  }
    
  #ifdef DEBUG_ICCTAG
  cout << " " << "" << "|" << getTypName() << "|" << text << " "; DBG
  #endif
  return text;
}

std::vector<std::string>
ICCtag::getDescription              (void)
{ DBG
  std::vector<std::string> texte;
  std::string text =  "";
  icUInt32Number count = *(icUInt32Number*)(_data+8);

  text.append ((const char*)(_data+12), icValue(count));
  texte.push_back (text);
  #ifdef DEBUG_ICCTAG
  cout << &_data[12] << "|" << "|" << text << " "; DBG
  #endif
  return texte;
}

std::vector<double>
ICCtag::getCIExy                                  (void)
{
  std::vector<double> punkte;
  icXYZType *daten = (icXYZType*) &_data[0];

  punkte.push_back( icValueSF( (daten->data.data[0].X) ) );
  punkte.push_back( icValueSF( (daten->data.data[0].Y) ) );
  punkte.push_back( icValueSF( (daten->data.data[0].Z) ) );

  return punkte;
}

std::vector<double>
ICCtag::getCurve                                  (void)
{
  std::vector<double> punkte;
  icCurveType *daten = (icCurveType*) &_data[0];
  int count = icValue(daten->curve.count);

  for (int i = 0; i < count; i++)
    punkte.push_back (icValue(daten->curve.data[i])/65536.0);

  return punkte;
}

std::string
ICCtag::getMore                                   ( void )
{
  std::string text = "Beschreibung";

  switch (_sig) {
    case icSigAToB0Tag: text = _("Farbtabelle, Gerät an Kontaktfarbraum, Anpassung 0"); break;
    case icSigAToB1Tag: text = _("Farbtabelle, Gerät an Kontaktfarbraum, Anpassung 1"); break;
    case icSigAToB2Tag: text = _("Farbtabelle, Gerät an Kontaktfarbraum, Anpassung 2"); break;
    case icSigBlueColorantTag: text = _("blaue Grundfarbe"); break;
    case icSigBlueTRCTag: text = _("blaue Farbwiedergabekurve"); break;
    case icSigBToA0Tag: text = _("Farbtabelle, Kontaktfarbraum an Gerät, Anpassung 0"); break;
    case icSigBToA1Tag: text = _("Farbtabelle, Kontaktfarbraum an Gerät, Anpassung 1"); break;
    case icSigBToA2Tag: text = _("Farbtabelle, Kontaktfarbraum an Gerät, Anpassung 2"); break;
    case icSigCalibrationDateTimeTag: text = _("Kalibrationsdatum"); break;
    case icSigCharTargetTag: text = _("targ"); break;
    case icSigCopyrightTag: text = _("Kopierrecht"); break;
    case icSigCrdInfoTag: text = _("crdi"); break;
    case icSigDeviceMfgDescTag: text = _("Herstellerbeschreibung"); break;
    case icSigDeviceModelDescTag: text = _("Gerätebeschreibung"); break;
    case icSigGamutTag: text = _("Farbumfang"); break;
    case icSigGrayTRCTag: text = _("schwarze Wiedergabekurve"); break;
    case icSigGreenColorantTag: text = _("grüne Grundfarbe"); break;
    case icSigGreenTRCTag: text = _("grüne Farbwiedergabekurve"); break;
    case icSigLuminanceTag: text = _("lumi"); break;
    case icSigMeasurementTag: text = _("Messart"); break;
    case icSigMediaBlackPointTag: text = _("Medienschwarzpunkt"); break;
    case icSigMediaWhitePointTag: text = _("Medienweißpunkt"); break;
    case icSigNamedColorTag: text = _("'ncol"); break;
    case icSigNamedColor2Tag: text = _("ncl2"); break;
    case icSigPreview0Tag: text = _("pre0"); break;
    case icSigPreview1Tag: text = _("pre1"); break;
    case icSigPreview2Tag: text = _("pre2"); break;
    case icSigProfileDescriptionTag: text = _("Profilbeschreibung"); break;
    case icSigProfileSequenceDescTag: text = _("Beschreibung der Profilverknüpfung"); break;
    case icSigPs2CRD0Tag: text = _("psd0"); break;
    case icSigPs2CRD1Tag: text = _("psd1"); break;
    case icSigPs2CRD2Tag: text = _("psd2"); break;
    case icSigPs2CRD3Tag: text = _("psd3"); break;
    case icSigPs2CSATag: text = _("ps2s"); break;
    case icSigPs2RenderingIntentTag: text = _("ps2i"); break;
    case icSigRedColorantTag: text = _("rote Grundfarbe"); break;
    case icSigRedTRCTag: text = _("rote Farbwiedergabekurve"); break;
    case icSigScreeningDescTag: text = _("scrd"); break;
    case icSigScreeningTag: text = _("scrn"); break;
    case icSigTechnologyTag: text = _("tech"); break;
    case icSigUcrBgTag: text = _("bfd"); break;
    case icSigViewingCondDescTag: text = _("Beschreibung der Betrachtungbedingungen"); break;
    case icSigViewingConditionsTag: text = _("Betrachtungsbedingungen"); break;
    case 1147500100: text = _("Farbmessflächen"); break;
    case 1128875332: text = _("Farbmessergebnisse"); break;
    case 1349350514: text = _("Profilierungsparameter"); break;
    case 1986226036: text = _("Apple Monitor-Grafikkartentabelle"); break;
    case 1667785060: text = _("Farbanpassungsmatrix"); break;
    case 1667789421: text = _("Primärfarben"); break;
    case 1668051567: text = _("Schmuckfarbordnung"); break;
    case 1668051572: text = _("Schmuckfarbnamen"); break;
    case 0: text = _("----"); break;
    default: text = _("???"); break;
  }
  return text;
}

std::string
ICCtag::getSigTagName               ( icTagSignature  sig )
{
  std::string text;

  switch (sig) {
    case icSigAToB0Tag: text = _("A2B0"); break;
    case icSigAToB1Tag: text = _("A2B1"); break;
    case icSigAToB2Tag: text = _("A2B2"); break;
    case icSigBlueColorantTag: text = _("bXYZ"); break;
    case icSigBlueTRCTag: text = _("bTRC"); break;
    case icSigBToA0Tag: text = _("B2A0"); break;
    case icSigBToA1Tag: text = _("B2A1"); break;
    case icSigBToA2Tag: text = _("B2A2"); break;
    case icSigCalibrationDateTimeTag: text = _("calt"); break;
    case icSigCharTargetTag: text = _("targ"); break;
    case icSigCopyrightTag: text = _("cprt"); break;
    case icSigCrdInfoTag: text = _("crdi"); break;
    case icSigDeviceMfgDescTag: text = _("dmnd"); break;
    case icSigDeviceModelDescTag: text = _("dmdd"); break;
    case icSigGamutTag: text = _("gamt"); break;
    case icSigGrayTRCTag: text = _("kTRC"); break;
    case icSigGreenColorantTag: text = _("gXYZ"); break;
    case icSigGreenTRCTag: text = _("gTRC"); break;
    case icSigLuminanceTag: text = _("lumi"); break;
    case icSigMeasurementTag: text = _("meas"); break;
    case icSigMediaBlackPointTag: text = _("bkpt"); break;
    case icSigMediaWhitePointTag: text = _("wtpt"); break;
    case icSigNamedColorTag: text = _("'ncol"); break;
    case icSigNamedColor2Tag: text = _("ncl2"); break;
    case icSigPreview0Tag: text = _("pre0"); break;
    case icSigPreview1Tag: text = _("pre1"); break;
    case icSigPreview2Tag: text = _("pre2"); break;
    case icSigProfileDescriptionTag: text = _("desc"); break;
    case icSigProfileSequenceDescTag: text = _("pseq"); break;
    case icSigPs2CRD0Tag: text = _("psd0"); break;
    case icSigPs2CRD1Tag: text = _("psd1"); break;
    case icSigPs2CRD2Tag: text = _("psd2"); break;
    case icSigPs2CRD3Tag: text = _("psd3"); break;
    case icSigPs2CSATag: text = _("ps2s"); break;
    case icSigPs2RenderingIntentTag: text = _("ps2i"); break;
    case icSigRedColorantTag: text = _("rXYZ"); break;
    case icSigRedTRCTag: text = _("rTRC"); break;
    case icSigScreeningDescTag: text = _("scrd"); break;
    case icSigScreeningTag: text = _("scrn"); break;
    case icSigTechnologyTag: text = _("tech"); break;
    case icSigUcrBgTag: text = _("bfd"); break;
    case icSigViewingCondDescTag: text = _("vued"); break;
    case icSigViewingConditionsTag: text = _("view"); break;
    case 1147500100: text = _("DevD"); break;
    case 1128875332: text = _("CIED"); break;
    case 1349350514: text = _("Pmtr"); break;
    case 1986226036: text = _("vcgt"); break;
    case 1667785060: text = _("chad"); break;
    case 1667789421: text = _("chrm"); break;
    case 1668051567: text = _("clro"); break;
    case 1668051572: text = _("clrt"); break;
    //case : text = _(""); break;
    case 0: text = _("----"); break;
    default: text = _("???"); break;
  }
  #ifdef DEBUG_ICCTAG_
  char c[5] = "clrt";
  long* l = (long*) &c[0];
  cout << *l << ": " << (long)"clrt" << " "; DBG
  #endif
  return text;
}

std::string
ICCtag::getSigTypeName               ( icTagTypeSignature  sig )
{
  std::string text;

  switch (sig) {
    case icSigCurveType: text = _("curv"); break;
    case icSigDataType: text = _("data"); break;
    case icSigDateTimeType: text = _("dtim"); break;
    case icSigLut16Type: text = _("mft2"); break;
    case icSigLut8Type: text = _("mft1"); break;
    case icSigMeasurementType: text = _("meas"); break;
    case icSigNamedColorType: text = _("ncol"); break;
    case icSigProfileSequenceDescType: text = _("pseq"); break;
    case icSigS15Fixed16ArrayType: text = _("sf32"); break;
    case icSigScreeningType: text = _("scrn"); break;
    case icSigSignatureType: text = _("sig"); break;
    case icSigTextType: text = _("text"); break;
    case icSigTextDescriptionType: text = _("desc"); break;
    case icSigU16Fixed16ArrayType: text = _("uf32"); break;
    case icSigUcrBgType: text = _("bfd"); break;
    case icSigUInt16ArrayType: text = _("ui16"); break;
    case icSigUInt32ArrayType: text = _("ui32"); break;
    case icSigUInt64ArrayType: text = _("ui64"); break;
    case icSigUInt8ArrayType: text = _("ui08"); break;
    case icSigViewingConditionsType: text = _("view"); break;
    case icSigXYZType: text = _("XYZ"); break;
    //case icSigXYZArrayType: text = _("XYZ"); break;
    case icSigNamedColor2Type: text = _("ncl2"); break;
    case icSigCrdInfoType: text = _("crdi"); break;
    case 1986226036: text = _("vcgt"); break;
    case icSigCopyrightTag: text = _("cprt?"); break; //??? (Imacon)
    default: text = _("???"); break;
  }
  return text;
}

std::string
ICCtag::getSigTechnology             ( icTechnologySignature sig )
{
  std::string text;
  switch (sig) {
    case icSigDigitalCamera: text = _("Digitale Kamera"); break; //dcam
    case icSigFilmScanner: text = _("Filmscanner"); break; //fscn
    case icSigReflectiveScanner: text = _("Reflectiver Scanner"); break; //rscn
    case icSigInkJetPrinter: text = _("Tintenstrahldrucker"); break; //ijet
    case icSigThermalWaxPrinter: text = _("Thermischer Wachsdrucker"); break; //twax
    case icSigElectrophotographicPrinter: text = _("Electrophotograph Drucker"); break; //epho
    case icSigElectrostaticPrinter: text = _("Electrostatischer Drucker"); break; //esta
    case icSigDyeSublimationPrinter: text = _("Thermosublimationsdrucker"); break; //dsub
    case icSigPhotographicPaperPrinter: text = _("Photographischer Papierdrucker"); break; //rpho
    case icSigFilmWriter: text = _("Filmbelichter"); break; //fprn
    case icSigVideoMonitor: text = _("Video Monitor"); break; //vidm
    case icSigVideoCamera: text = _("Video Kamera"); break; //vidc
    case icSigProjectionTelevision: text = _("Projection Television"); break; //pjtv
    case icSigCRTDisplay: text = _("Kathodenstrahlmonitor"); break; //CRT
    case icSigPMDisplay: text = _("Passivmatrixmonitor"); break; //PMD
    case icSigAMDisplay: text = _("Aktivmatrixmonitor"); break; //AMD
    case icSigPhotoCD: text = _("Photo CD"); break; //KPCD
    case icSigPhotoImageSetter: text = _("PhotoImageSetter"); break; //imgs
    case icSigGravure: text = _("Gravure"); break; //grav
    case icSigOffsetLithography: text = _("Offset Lithography"); break; //offs
    case icSigSilkscreen: text = _("Silkscreen"); break; //silk
    case icSigFlexography: text = _("Flexography"); break; //flex
    case icMaxEnumTechnology: text = _("----"); break;   
    default: text = _("???"); break;
  }
  return text;
}

std::string
ICCtag::getIlluminant             ( icIlluminant sig )
{
  std::string text;
  switch (sig) {
    case icIlluminantUnknown: text = _("Illuminant unbekannt"); break;
    case icIlluminantD50: text = _("Illuminant D50"); break;
    case icIlluminantD65: text = _("Illuminant D65"); break;
    case icIlluminantD93: text = _("Illuminant D93"); break;
    case icIlluminantF2: text = _("Illuminant F2"); break;
    case icIlluminantD55: text = _("Illuminant D55"); break;
    case icIlluminantA: text = _("Illuminant A"); break;
    case icIlluminantEquiPowerE: text = _("Illuminant ausgeglichene Energie E"); break;
    case icIlluminantF8: text = _("Illuminant F8"); break;
    case icMaxEnumIluminant: text = _("Illuminant ---"); break;

    default: text = _("???"); break;
  }
  return text;
}

std::string
ICCtag::getStandardObserver             ( icStandardObserver sig )
{
  std::string text;
  switch (sig) {
    case icStdObsUnknown: text = _("unbekannt"); break;
    case icStdObs1931TwoDegrees: text = _("2 Grad"); break;
    case icStdObs1964TenDegrees: text = _("10 Grad"); break;
    case icMaxStdObs: text = _("---"); break;

    default: text = _("???"); break;
  }
  return text;
}

std::string
ICCtag::getMeasurementGeometry             ( icMeasurementGeometry sig )
{
  std::string text;
  switch (sig) {
    case icGeometryUnknown: text = _("unbekannt"); break;
    case icGeometry045or450: text = _("0/45, 45/0"); break;
    case icGeometry0dord0: text = _("0/d or d/0"); break;
    case icMaxGeometry: text = _("---"); break;

    default: text = _("???"); break;
  }
  return text;
}

std::string
ICCtag::getMeasurementFlare             ( icMeasurementFlare sig )
{
  std::string text;
  switch (sig) {
    case icFlare0: text = _("0"); break;
    case icFlare100: text = _("100"); break;
    case icMaxFlare: text = _("---"); break;

    default: text = _("???"); break;
  }
  return text;
}

/*
void
ICCtag::printLut            (   LPLUT           Lut,
                                int             sig)
{
  unsigned int      i;
  int               channels=0;

  printf ("Channels In/Out = %d/%d Nr: %d/%d\n",Lut->InputChan, Lut->OutputChan,
                                        Lut->InputEntries, Lut->OutputEntries);

  switch (sig)
  {
  case icSigAToB0Tag:
  case icSigAToB1Tag:
  case icSigAToB2Tag:
    channels = Lut ->InputChan;
    break;
  case icSigBToA0Tag:
  case icSigBToA1Tag:
  case icSigBToA2Tag:
    channels = Lut ->OutputChan;
    break;
  case icSigGamutTag:
    channels = 1;
    break;
  }
  channels = Lut->OutputChan;

  for (i=0; i < Lut ->InputEntries; i++)  {
    if (channels > 0)
      printf ("C=%d ", Lut ->L2[0][i]);
    if (channels > 1)
      printf ("M=%d ", Lut ->L2[1][i]);
    if (channels > 2)
      printf ("Y=%d ", Lut ->L2[2][i]);
    if (channels > 3)
      printf ("K=%d ", Lut ->L2[3][i]);
  }
  printf ("\n");
}*/


/**
  *  @brief ICCprofile Funktionen
  */

ICCprofile::ICCprofile (void)
{
  _data = NULL;
  _size = 0;
  
}

ICCprofile::ICCprofile (const char *filename)
  : _filename (filename)
{
  if (_data) free (_data);
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
}

ICCprofile::~ICCprofile (void)
{
  if (_data) free (_data);
  tags.clear();

  #ifdef DEBUG_PROFILE
  cout << "_data und tags gelöscht"; DBG
  #endif
}

void
ICCprofile::load (std::string filename)
{
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
}

void
ICCprofile::load (char* filename)
{
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
}

void
ICCprofile::fload ()
{ // ICC Profil laden
  //FilePtr fp ( _filename.c_str(), "rb");
  string file;
  ifstream f ( _filename.c_str(), ios::binary | ios::ate );

  DBG
  if (_filename == "")
    throw ausn_file_io ("kein Dateiname angegeben");
  DBG
  if (!f) {
    throw ausn_file_io ("keine lesbare Datei gefunden");
    _filename = "";
  }

  if (_data!=NULL) { free (_data); DBG }
  _size = (unsigned int)f.tellg();         f.seekg(0);
  _data = (icProfile*) calloc ( _size, sizeof (char)); DBG
  
  f.read ((char*)_data, _size); DBG
  #ifdef DEBUG_ICCPROFILE
  cout << _size << "|" << f.tellg() << endl;
  #endif
  f.close(); DBG

  DBG
  //Kopf
  header.load ((void*)_data); DBG

  //Profilabschnitte
  tags.clear(); DBG
  #ifdef DEBUG_ICCPROFILE
  cout << "TagCount: " << getTagCount() << " / " << tags.size() << " ";DBG
  #endif
  // TagTabelle bei 132 abholen
  icTag *tagList = (icTag*)&((char*)_data)[132];
  //(icTag*) calloc ( getTagCount() , sizeof (icTag));
  //memcpy (tagList , &((char*)_data)[132], sizeof (icTag) * getTagCount());
  for (int i = 0 ; i < getTagCount() ; i++) {
    ICCtag tag;
    DBG
    tag.load( &tagList[i] ,
              &((char*)_data)[ icValue(tagList[i].offset) ]); DBG
  #ifdef DEBUG_ICCPROFILE
    cout << " sig: " << tag.getTagName() << " "; DBG
  #endif
    tags.push_back(tag); DBG
  }
  #ifdef DEBUG_ICCPROFILE
  cout << "tags: " << tags.size() << " "; DBG
  #endif
  DBG
}

std::vector<std::string>
ICCprofile::printTagInfo         (int item)
{
  std::vector<std::string> liste;

  liste.push_back( tags.at(item).getTagName() );
  liste.push_back( tags.at(item).getTypName() );

  DBG
  return liste;
}

std::vector<std::string>
ICCprofile::printTags            ()
{
  DBG
  std::vector<std::string> StringList;
  std::string text;
  std::stringstream s;

  if (!tags.size()) { DBG
    return StringList;
  } DBG

  int count = 0;
  for (std::vector<ICCtag>::iterator it = tags.begin(); it != tags.end(); it++){
    s.str("");
    s << count; count++;           StringList.push_back(s.str()); s.str("");
    s.str((*it).getTagName());     StringList.push_back(s.str()); s.str("");
    s.str((*it).getTypName());     StringList.push_back(s.str()); s.str("");
    s << (*it).getSize();          StringList.push_back(s.str()); s.str("");
    s.str((*it).getMore()); StringList.push_back(s.str()); s.str("");
  #ifdef DEBUG_ICCPROFILE
    cout << (*it).getTagName() << " "; DBG
  #endif
  }

  return StringList;
}

std::string
ICCprofile::getTagText                                  (int item)
{
  // Prüfen
  std::string leer = tags[item].getTypName() + " Typ - keine Textausgabe";
  if (tags[item].getTypName() != "text"
   && tags[item].getTypName() != "cprt?"
   && tags[item].getTypName() != "meas"
   && tags[item].getTypName() != "sig"
   && tags[item].getTypName() != "dtim")
    return leer;

  return tags.at(item).getText();
}

std::vector<std::string>
ICCprofile::getTagDescription                           (int item)
{
  // Prüfen
  std::vector<std::string> leer;
  if (tags[item].getTypName() != "desc")
    return leer;

  return tags.at(item).getDescription();
}

std::vector<double>
ICCprofile::getTagCIExy                                 (int item)
{
  // Prüfen
  std::vector<double> leer;
  if (tags[item].getTypName() != "XYZ")
    return leer;

  return tags.at(item).getCIExy();
}

std::vector<double>
ICCprofile::getTagCurve                                 (int item)
{
  // Prüfen
  std::vector<double> leer;
  if (tags[item].getTypName() != "curv"
   && tags[item].getTypName() != "vcgt")
    return leer;

  return tags.at(item).getCurve();
}

int
ICCprofile::getTagByName            (std::string name)
{
  if (!tags.size()) { DBG
    return -1;
  } DBG

  int item = 0;
  for (std::vector<ICCtag>::iterator it = tags.begin(); it != tags.end(); it++){
    if ( (*it).getTagName() == name
      && (*it).getSize()            ) {
      #ifdef DEBUG_ICCPROFILE
      cout << item << "=" << (*it).getTagName() << " gefunden "; DBG
      #endif
      return item;
    }
    item++;
  }

  return -1;
}

bool
ICCprofile::hasTagName            (std::string name)
{
  if (!tags.size()) { DBG
    return false;
  } DBG

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

  return false;
}

std::vector<double>
ICCprofile::getWhitePkt           (void)
{
  std::vector<double> xyY;
  if (hasTagName ("wtpt"))
    xyY = getTagCIExy (getTagByName ("wtpt"));

  return xyY;
}




int
ICCprofile::checkProfileDevice (char* type, icProfileClassSignature deviceClass)
{
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
                     header.getDeviceClassName(deviceClass).c_str());
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
                     header.getDeviceClassName(deviceClass).c_str(),
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
                     header.getDeviceClassName(deviceClass).c_str(),
                     _("is not valid for an"),
                     type,
                     _("Profile\"."));
          check = false;
          break;
        if (icSigCmykData   != header.colorSpace())
          g_message ("%s - %s - %s \"%s %s",_("Color space"),
                     header.getColorSpaceName(header.colorSpace()).c_str(),
                     _("is not valid for an"),
                     type,
                     _("Profile at the moment\"."));
          check = false;
        }
  }
  return check;
}

void
ICCprofile::saveProfileToFile (char* filename, char *profile, int size)
{
  FILE *fp=NULL;
  int   pt = 0;

  if ((fp=fopen(filename, "w")) != NULL) {
    do {
      fputc ( profile[pt++] , fp);
    } while (--size);
  }

  fclose (fp);
}


/**
  *  allgemeine Funktionen
  */

void
lcms_error (int ErrorCode, const char* ErrorText)
{
   g_message ("LCMS error:%d %s", ErrorCode, ErrorText);
}

const char* cp_nchar (char* text, int n)
{
  static char string[1024];

  for (int i = 0; i < 1024 ; i++)
    string[i] = '\000';

  if (n < 1024)
    snprintf(&string[0], n, text);
  else
    return NULL;

  #ifdef DEBUG
  cout << n << " Buchstaben kopieren " <<  (int)text << " " << string <<" "; DBG
  #endif

  return &string[0];
}


