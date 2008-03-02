// Author:    Kai-Uwe Behrmann
// Copyright: Kai-Uwe Behrmann <ku.b@gmx.de>
// Date:      04. 05. 2004


#include "icc_profile.h"
#include "icc_examin.h"
#include "icc_utils.h"

#define _(text) text
#define g_message printf

int
icValue (icUInt32Number val)
{
  unsigned char        *temp = (unsigned char*) &val;

  //temp = (unsigned char*) &val;

#if BYTE_ORDER == LITTLE_ENDIAN
  static unsigned char  uint32[4];

  uint32[0] = temp[3];
  uint32[1] = temp[2];
  uint32[2] = temp[1];
  uint32[3] = temp[0];

  DBG
  cout << (int)*temp << " Größe nach Wandlung " << (int)temp[0] << " "
       << (int)temp[1] << " " << (int)temp[2] << " " <<(int)temp[3]
       << endl;

  return (int) &uint32[0];
#else
  cout << (int)val << " Größe ohne Wandlung " << (int)temp[0] << " "
       << (int)temp[1] << " " << (int)temp[2] << " " <<(int)temp[3]
       << endl;
  return (int)val;
#endif
}

long
icValue (icUInt64Number val)
{
  unsigned char        *temp  = (unsigned char*) &val;

#if BYTE_ORDER == LITTLE_ENDIAN
  static unsigned char  uint64[8];
  int little = 0,
      big    = 8;

  for (; little < 8 ; i++ ) {
    uint64[little++] = temp[big--];
  }

  DBG
  cout << (int)*temp << " Größe nach Wandlung " << (int)temp[0] << " "
       << (int)temp[1] << " " << (int)temp[2] << " " <<(int)temp[3]
       << endl;

  return (int) &uint64[0];
#else
  cout << (int)val << " Größe ohne Wandlung " << (int)temp[0] << " "
       << (int)temp[1] << " " << (int)temp[2] << " " <<(int)temp[3]
       << endl;
  return (long)val;
#endif
}

double
icValue (icS15Fixed16Number val)
{
  signed int    temp  = (signed int) val;
  static double d;

  d = (double)temp / 65536;

  DBG

  return d;
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
  memcpy ((void*)&header, data,/* (void*)&header,*/ sizeof (icHeader));
  cout << sizeof (icHeader) << " genommen" << endl;
  if (header.size > 0) {
    valid = true;
    cout << size() << endl;
  } else {
    valid = false;
  }
  DBG
}

/*void
ICCheader::size (int i)
{
  return size;
}*/

std::string
ICCheader::print_long()
{
  cout << sizeof (icSignature) << " " << sizeof (icUInt32Number)<< endl;
  std::stringstream s; DBG
  s << "kein Dateikopf gefunden"; DBG
  //cout << "char icSignature ist " << sizeof (icSignature) << endl;
  if (valid) { DBG
    s.str("");
    s << "ICC Dateikopf:\n"<< endl \
      <<  "    " << _("Größe") << ":       " <<
                       size() << " " << _("bytes") << endl \
      <<  "    " << _("CMM") << ":         " << CmmName() << endl \
      <<  "    " << _("Version") << ":     " << version() << endl \
      <<  "    " << _("Klasse") << ":      " <<
                       getDeviceClassName(deviceClass()) << endl \
      <<  "    " << _("Farbraum") << ":    " <<
                       getColorSpaceName(colorSpace()) << endl \
      <<  "    " << _("PCS") << ":         " <<
                       getColorSpaceName(pcs()) << endl \
      <<  "    " << _("Datum") << ":       " <<
                       header.date.day     << "/" <<
                       header.date.month   << "/" <<
                       header.date.year    << " " <<
                       header.date.hours   << ":" <<
                       header.date.minutes << " " << _("Uhr")      << " " <<
                       header.date.seconds << " " << _("Sekunden") << endl \
      <<  "    " << _("Magic") << ":       " << icValue(header.magic) << endl \
      <<  "    " << _("Plattform") << ":   " << platform() << endl \
      <<  "    " << _("Flags") << ":       " << icValue(header.flags) << endl \
      <<  "    " << _("Hersteller") << ":  " <<
                       icValue(header.manufacturer) << endl \
      <<  "    " << _("Model") << ":       " << icValue(header.model) << endl \
      <<  "    " << _("Attribute") << ":   " <<
                       icValue(header.attributes) << endl \
      <<  "    " << _("Übertragung") << ": " <<
                       icValue(header.renderingIntent) << endl \
      <<  "    " << _("Beleuchtung") << ": X=" <<
                       icValue(header.illuminant.X) << ", Y=" << \
                       icValue(header.illuminant.Y) << ", Z=" << \
                       icValue(header.illuminant.Z) << endl \
      <<  "    " << _("von") << ":         " << icValue(header.flags) << endl ;
  } DBG
  return s.str();
}

std::string
ICCheader::print()
{
  string s = _("Dateikopf ungültig");
  if (valid)
    s = _("  1 Kopf        icHeader      128 Dateikopf");
  return s;
}


const char*
ICCheader::getColorSpaceName (icColorSpaceSignature color)
{
  const char* name;

  switch (color) {
    case icSigXYZData: name = cp_char (_("XYZ")); break;
    case icSigLabData: name = cp_char (_("Lab")); break;
    case icSigLuvData: name = cp_char (_("Luv")); break;
    case icSigYCbCrData: name = cp_char (_("YCbCr")); break;
    case icSigYxyData: name = cp_char (_("Yxy")); break;
    case icSigRgbData: name = cp_char (_("Rgb")); break;
    case icSigGrayData: name = cp_char (_("Gray")); break;
    case icSigHsvData: name = cp_char (_("Hsv")); break;
    case icSigHlsData: name = cp_char (_("Hls")); break;
    case icSigCmykData: name = cp_char (_("Cmyk")); break;
    case icSigCmyData: name = cp_char (_("Cmy")); break;
    case icSig2colorData: name = cp_char (_("2color")); break;
    case icSig3colorData: name = cp_char (_("3color")); break;
    case icSig4colorData: name = cp_char (_("4color")); break;
    case icSig5colorData: name = cp_char (_("5color")); break;
    case icSig6colorData: name = cp_char (_("6color")); break;
    case icSig7colorData: name = cp_char (_("7color")); break;
    case icSig8colorData: name = cp_char (_("8color")); break;
    case icSig9colorData: name = cp_char (_("9color")); break;
    case icSig10colorData: name = cp_char (_("10color")); break;
    case icSig11colorData: name = cp_char (_("11color")); break;
    case icSig12colorData: name = cp_char (_("12color")); break;
    case icSig13colorData: name = cp_char (_("13color")); break;
    case icSig14colorData: name = cp_char (_("14color")); break;
    case icSig15colorData: name = cp_char (_("15color")); break;
    default: name = cp_char (_("")); break;
  }
  return name;
}

const char*
ICCheader::getDeviceClassName (icProfileClassSignature deviceClass)
{
  const char* name;

  switch (deviceClass)
  {
    case icSigInputClass: name = cp_char (_("Eingabe")); break;
    case icSigDisplayClass: name = cp_char (_("Monitor")); break;
    case icSigOutputClass: name = cp_char (_("Ausgabe")); break;
    case icSigLinkClass: name = cp_char (_("Verknüpfung")); break;
    case icSigAbstractClass: name = cp_char (_("Abstrakter Farbraum")); break;
    case icSigColorSpaceClass: name = cp_char (_("Farbraum")); break;
    case icSigNamedColorClass: name = cp_char (_("Schmuckfarben")); break;
    default: name = cp_char (_("")); break;
  }
  return name;
}

const char*
ICCheader::getPlatformName (icPlatformSignature platform)
{
  const char* name;

  switch (platform)
  {
    case icSigMacintosh: name = cp_char (_("Macintosh")); break;
    case icSigMicrosoft: name = cp_char (_("Microsoft")); break;
    case icSigSolaris: name = cp_char (_("Solaris")); break;
    case icSigSGI: name = cp_char (_("SGI")); break;
    case icSigTaligent: name = cp_char (_("Taligent")); break;
    default: name = cp_char (_("")); break;
  }
  return name;
}

/*int
ICCheader::getLcmsGetColorSpace ( cmsHPROFILE   hProfile)
{
  int lcmsColorSpace;

  #if (LCMS_VERSION >= 113)
    lcmsColorSpace = _cmsLCMScolorSpace ( cmsGetColorSpace (hProfile) );
  #else
  switch (cmsGetColorSpace (hProfile))
  {
    case icSigXYZData: lcmsColorSpace = PT_XYZ; break;
    case icSigLabData: lcmsColorSpace = PT_Lab; break;
    case icSigLuvData: lcmsColorSpace = PT_YUV; break;
    case icSigLuvKData: lcmsColorSpace = PT_YUVK; break;
    case icSigYCbCrData: lcmsColorSpace = PT_YCbCr; break;
    case icSigYxyData: lcmsColorSpace = PT_Yxy; break;
    case icSigRgbData: lcmsColorSpace = PT_RGB; break;
    case icSigGrayData: lcmsColorSpace = PT_GRAY; break;
    case icSigHsvData: lcmsColorSpace = PT_HSV; break;
    case icSigHlsData: lcmsColorSpace = PT_HLS; break;
    case icSigCmykData: lcmsColorSpace = PT_CMYK; break;
    case icSigCmyData: lcmsColorSpace = PT_CMY; break;
    case icSig2colorData: lcmsColorSpace = PT_ANY; break;
    case icSig3colorData: lcmsColorSpace = PT_ANY; break;
    case icSig4colorData: lcmsColorSpace = PT_ANY; break;
    case icSig5colorData: lcmsColorSpace = PT_ANY; break;
    case icSig6colorData: lcmsColorSpace = PT_ANY; break;
    case icSig7colorData: lcmsColorSpace = PT_ANY; break;
    case icSig8colorData: lcmsColorSpace = PT_ANY; break;
    case icSig9colorData: lcmsColorSpace = PT_ANY; break;
    case icSig10colorData: lcmsColorSpace = PT_ANY; break;
    case icSig11colorData: lcmsColorSpace = PT_ANY; break;
    case icSig12colorData: lcmsColorSpace = PT_ANY; break;
    case icSig13colorData: lcmsColorSpace = PT_ANY; break;
    case icSig14colorData: lcmsColorSpace = PT_ANY; break;
    case icSig15colorData: lcmsColorSpace = PT_ANY; break;
    case icSigHexachromeData: lcmsColorSpace = PT_HiFi; break; // speculation
    default: lcmsColorSpace = PT_ANY; break;
  }
  #endif
  return lcmsColorSpace;
}*/


/**
  *  @brief ICCtag Funktionen
  */

ICCtag::ICCtag ()
{
  _tag.sig = icMaxEnumTag;
  _data = 0; DBG
}

ICCtag::ICCtag                      (icTag* tag, char* data)
{
  ICCtag::load (tag, data); DBG
}

ICCtag::~ICCtag ()
{
  _tag.sig = icMaxEnumTag;
  _tag.offset = 0;
  _tag.size = 0;
  if (!_data) free (_data);
  DBG
}

void
ICCtag::load                        ( icTag *tag, char* data )
{
  _tag.sig = tag->sig;
  _tag.offset = tag->offset;
  _tag.size = tag->size;

  cout << _tag.sig << tag->sig << endl;

  if (!_data) free (_data);
  _data = (char*) calloc ( _tag.size , sizeof (char) );
  memcpy ( _data , data , _tag.size );
  DBG
}

std::string
ICCtag::getSigTagName               ( icTagSignature  sig )
{
  const char* name;
  std::string string;

  switch (sig) {
    case icSigAToB0Tag: name = cp_char (_("A2B0")); break;
    case icSigAToB1Tag: name = cp_char (_("A2B1")); break;
    case icSigAToB2Tag: name = cp_char (_("A2B2")); break;
    case icSigBlueColorantTag: name = cp_char (_("bXYZ")); break;
    case icSigBlueTRCTag: name = cp_char (_("bTRC")); break;
    case icSigBToA0Tag: name = cp_char (_("B2A0")); break;
    case icSigBToA1Tag: name = cp_char (_("B2A1")); break;
    case icSigBToA2Tag: name = cp_char (_("B2A2")); break;
    case icSigCalibrationDateTimeTag: name = cp_char (_("calt")); break;
    case icSigCharTargetTag: name = cp_char (_("targ")); break;
    case icSigCopyrightTag: name = cp_char (_("cprt")); break;
    case icSigCrdInfoTag: name = cp_char (_("crdi")); break;
    case icSigDeviceMfgDescTag: name = cp_char (_("dmnd")); break;
    case icSigDeviceModelDescTag: name = cp_char (_("dmdd")); break;
    case icSigGamutTag: name = cp_char (_("gamt")); break;
    case icSigGrayTRCTag: name = cp_char (_("kTRC")); break;
    case icSigGreenColorantTag: name = cp_char (_("gXYZ")); break;
    case icSigGreenTRCTag: name = cp_char (_("gTRC")); break;
    case icSigLuminanceTag: name = cp_char (_("lumi")); break;
    case icSigMeasurementTag: name = cp_char (_("meas")); break;
    case icSigMediaBlackPointTag: name = cp_char (_("bkpt")); break;
    case icSigMediaWhitePointTag: name = cp_char (_("wtpt")); break;
    case icSigNamedColorTag: name = cp_char (_("'ncol")); break;
    case icSigNamedColor2Tag: name = cp_char (_("ncl2")); break;
    case icSigPreview0Tag: name = cp_char (_("pre0")); break;
    case icSigPreview1Tag: name = cp_char (_("pre1")); break;
    case icSigPreview2Tag: name = cp_char (_("pre2")); break;
    case icSigProfileDescriptionTag: name = cp_char (_("desc")); break;
    case icSigProfileSequenceDescTag: name = cp_char (_("pseq")); break;
    case icSigPs2CRD0Tag: name = cp_char (_("psd0")); break;
    case icSigPs2CRD1Tag: name = cp_char (_("psd1")); break;
    case icSigPs2CRD2Tag: name = cp_char (_("psd2")); break;
    case icSigPs2CRD3Tag: name = cp_char (_("psd3")); break;
    case icSigPs2CSATag: name = cp_char (_("ps2s")); break;
    case icSigPs2RenderingIntentTag: name = cp_char (_("ps2i")); break;
    case icSigRedColorantTag: name = cp_char (_("rXYZ")); break;
    case icSigRedTRCTag: name = cp_char (_("rTRC")); break;
    case icSigScreeningDescTag: name = cp_char (_("scrd")); break;
    case icSigScreeningTag: name = cp_char (_("scrn")); break;
    case icSigTechnologyTag: name = cp_char (_("tech")); break;
    case icSigUcrBgTag: name = cp_char (_("bfd")); break;
    case icSigViewingCondDescTag: name = cp_char (_("vued")); break;
    case icSigViewingConditionsTag: name = cp_char (_("view")); break;
    default: name = cp_char (_("???")); break;
  }
  string = name;
  return string;
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
  //header = new ICCheader();
  _data = NULL;
  _size = 0;
  
}

ICCprofile::ICCprofile (const char *filename)
  : _filename (filename)
{
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
  ;
}

void
ICCprofile::load (std::string filename)
{
  // delegieren
  _filename = filename;
  try {
    fload();
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
    fload();
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
  int i = 0;

  DBG
  if (_filename == "")
    throw ausn_file_io ("kein Dateiname angegeben");
  DBG
  if (!f)
    throw ausn_file_io ("keine lesbare Datei gefunden");

  if (_data!=NULL) { free (_data); } DBG
  _size = (unsigned int)f.tellg();         f.seekg(0);
  _data = (icProfile*) calloc ( _size, sizeof (char)); DBG
  
  f.read ((char*)_data, _size); DBG
  cout << _size << "|" << i << "|" << f.tellg() << endl;
  f.close(); DBG

  DBG
  //Kopf
  header.load ((void*)_data); DBG

  //Profilabschnitte
  tags.clear(); DBG
  cout << "TagCount: " << getTagCount() << " / " << tags.size() << endl;
  icTag *tagList = (icTag*)&((char*)_data)[132];
  //(icTag*) calloc ( getTagCount() , sizeof (icTag));
  //memcpy (tagList , &((char*)_data)[132], sizeof (icTag) * getTagCount());
  for (int i = 0 ; i < getTagCount() ; i++) {
    ICCtag tag;//( tagList[i] , &((char*)_data)[ tagList[i].offset ] );
    tag.load( &tagList[i] , &((char*)_data)[ tagList[i].offset ] );
    cout << " sig: " << tag.getTagName() << endl; 
    tags.push_back(tag);
  }
  cout << "tags: " << tags.size() << endl;
  DBG
}

std::string
ICCprofile::printLongTag         (int item)
{
  std::string text = "Hier kommt die lange Beschreibung";

  DBG
  return text;
}

std::list<std::string>
ICCprofile::printTags            ()
{
  DBG
  std::list<std::string> StringList;

  if (!tags.size()) {
    std::string text = "Keine Tags vorhanden.";
    StringList.push_back(text);
    return StringList;
  }

  for (std::list<ICCtag>::iterator i = tags.begin() ;
         i != tags.end() ; i++) {
    std::string text;

    text = (*i).getTagName();
    StringList.push_back(text);
  }

  return StringList;
}

char*
ICCprofile::getProfileInfo                   ( )
{
  static char text[128];
  static char profile_info[2048];
  icXYZNumber WhitePt;
  int       first = false,
            min_len = 24,
            len, i, pos=17;

    /* formatting */
/*    if (cmsIsTag(hProfile, icSigCopyrightTag)) {
        cmsReadICCText(hProfile, icSigCopyrightTag, &text[0]);
        len = strlen (text) + 16;
        if (len > min_len)
            min_len = len + 1;
    }
    profile_info[0] = '\000';

    if (cmsIsTag(hProfile, icSigProfileDescriptionTag)) {
        sprintf (text,_("Description:     "));
        cmsReadICCText(hProfile, icSigProfileDescriptionTag, &text[pos]);
        sprintf (profile_info,"%s%s",profile_info,text);
        if (!first) { // formatting for tooltips 
            len = min_len - strlen (profile_info);

            for (i=0; i < len * 2.2; i++) {
                sprintf (&profile_info[strlen (&profile_info[0])] ," ");
                //sprintf (profile_info,"%s ",profile_info);
            }
        }
        sprintf (profile_info,"%s\n",profile_info);
    }
    if (cmsIsTag(hProfile, icSigDeviceMfgDescTag)) {
        sprintf (text,_("Manufacturer:    "));
        cmsReadICCText(hProfile, icSigDeviceMfgDescTag, &text[pos]);
        sprintf (profile_info,"%s%s",profile_info,text);
    }
    if (cmsIsTag(hProfile, icSigDeviceModelDescTag)) {
        sprintf (text,_("Model:           "));
        cmsReadICCText(hProfile, icSigDeviceModelDescTag, &text[pos]);
        sprintf (profile_info,"%s%s",profile_info,text);
    }
    if (cmsIsTag(hProfile, icSigCopyrightTag)) {
        sprintf (text,_("Copyright:       "));
        cmsReadICCText(hProfile, icSigCopyrightTag, &text[pos]);
        sprintf (profile_info,"%s%s",profile_info,text);
    }

    sprintf (profile_info,"%s\n",profile_info);

    cmsTakeMediaWhitePoint (&WhitePt, hProfile);
    _cmsIdentifyWhitePoint (text, &WhitePt);
    sprintf (profile_info, "%s%s\n", profile_info, text);
*/
    sprintf (text,_("Device Class:   "));
    sprintf (profile_info,"%s%s %s\n",profile_info,text,
                              header.getDeviceClassName(header.deviceClass()));
    sprintf (text,_("Color Space:    "));
    sprintf (profile_info,"%s%s %s\n",profile_info,text,
                              header.getColorSpaceName(header.colorSpace()));
    sprintf (text,_("PCS Space:      "));
    sprintf (profile_info,"%s%s %s",profile_info,text,
                              header.getColorSpaceName(header.pcs()));

 
  return profile_info;
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
                     header.getDeviceClassName(deviceClass));
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
                     header.getDeviceClassName(deviceClass),
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
                     header.getDeviceClassName(deviceClass),
                     _("is not valid for an"),
                     type,
                     _("Profile\"."));
          check = false;
          break;
        if (icSigCmykData   != header.colorSpace())
          g_message ("%s - %s - %s \"%s %s",_("Color space"),
                     header.getColorSpaceName(header.colorSpace()),
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



const char* cp_char (char* text)
{
  static char string[128];

  sprintf(string, text);

  return &string[0];
}

const char* cp_nchar (char* text, int n)
{
  static char string[1024];

  if (n < 1024)
    snprintf(&string[0], n, text);
  else
    return NULL;

  return &string[0];
}


