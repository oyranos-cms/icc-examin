// Author:    Kai-Uwe Behrmann
// Copyright: Kai-Uwe Behrmann <ku.b@gmx.de>
// Date:      04. 05. 2004


#include "icc_profile.h"
#include "icc_examin.h"
#include "icc_utils.h"

#define _(text) text
#define g_message printf

// interne Funktionen

static char* cp_char (char* text);


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
  if (header.size > 0)
    valid = true;
  else
    valid = false;
  DBG
}

std::string
ICCheader::print_long()
{
  std::stringstream s; DBG
  s << "kein Dateikopf gefunden"; DBG
  if (valid) { DBG
    s.str("");
    s << "ICC Dateikopf:\n"<< endl \
      <<  "    Größe:     " << header.size() << endl \
      <<  "    CMM:       " << header.cmmId /*getCmmName()*/ << endl \
      <<  "    Version:   " << header.version << endl;
  } DBG
  return s.str();
}

std::string
ICCheader::print()
{
  string s = "Dateikopf ungültig";
  if (valid)
    s = "  1 Kopf        icHeader      128 Dateikopf";
  return s;
}


char*
ICCheader::getColorSpaceName (icColorSpaceSignature color)
{
  char* name;

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

char*
ICCheader::getDeviceClassName (icProfileClassSignature deviceClass)
{
  char* name;

  switch (deviceClass)
  {
    case icSigInputClass: name = cp_char (_("Input")); break;
    case icSigDisplayClass: name = cp_char (_("Display")); break;
    case icSigOutputClass: name = cp_char (_("Output")); break;
    case icSigLinkClass: name = cp_char (_("Link")); break;
    case icSigAbstractClass: name = cp_char (_("Abstract")); break;
    case icSigColorSpaceClass: name = cp_char (_("ColorSpace")); break;
    case icSigNamedColorClass: name = cp_char (_("NamedColor")); break;
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

char*
ICCtag::getSigTagName               (   icTagSignature  sig )
{
  char* name;

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
    default: name = cp_char (_("")); break;
  }
  return name;
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
//#include <lcms.h>
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

  if (_data!=NULL) {
    free (_data);
  } DBG
  _size = (unsigned int)f.tellg();         f.seekg(0);
  _data = (icProfile*) calloc ( _size, sizeof (char)); DBG
  
  f.read ((char*)_data, _size); DBG
  cout << _size << "|" << i << "|" << f.tellg() << endl;
  f.close(); DBG

  //cmsHPROFILE p = cmsOpenProfileFromMem ((char*)_data, (int)_size);
  //printf (cmsTakeProductDesc(p));
  DBG
  header.load ((void*)&_data); DBG
}

void
ICCprofile::printProfileTags            ()
{
#ifdef DEBUG
  #if LCMS_VERSION >= 113
  LPLCMSICCPROFILE p = (LPLCMSICCPROFILE) hProfile;
  int              i;

  printf ("TagCount = %d \n",p->TagCount);
  for (i=0; i < p->TagCount ; i++) {
    char *name = getSigTagName(p->TagNames[i]);
    long   len = strlen (name);

    if (len <= 1) {
      name = calloc (sizeof (char), 256);
      sprintf (name, "%x", p->TagNames[i]);
    }
    printf ("%d: TagNames %s TagSizes %d TagOffsets %d TagPtrs %d\n",
             i, name, p->TagSizes[i], p->TagOffsets[i], (int)p->TagPtrs[i]);
    free (name);
  }
  #endif
#endif
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
                              header.getDeviceClassName(header.header.deviceClass));
    sprintf (text,_("Color Space:    "));
    sprintf (profile_info,"%s%s %s\n",profile_info,text,
                              header.getColorSpaceName(header.header.colorSpace));
    sprintf (text,_("PCS Space:      "));
    sprintf (profile_info,"%s%s %s",profile_info,text,
                              header.getColorSpaceName(header.header.pcs));

 
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
        if (icSigCmykData   != header.header.colorSpace)
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
        if (icSigCmykData   != header.header.colorSpace)
          g_message ("%s - %s - %s \"%s %s",_("Color space"),
                     header.getColorSpaceName(header.header.colorSpace),
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

static char* cp_char (char* text)
{
  static char string[128];

  sprintf(string, text);

  return &string[0];
}


