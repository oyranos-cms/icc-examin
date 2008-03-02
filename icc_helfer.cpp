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
 * Aufbereitung von ICC internen Informationen
 * 
 */

// Date:      04. 05. 2004

#if 0
# ifndef DEBUG
#  define DEBUG
# endif
# define DEBUG_ICCFUNKT
#endif

#include <lcms.h>
#include <icc34.h>
#include "icc_utils.h"
#include "icc_formeln.h"
#include "icc_helfer.h"

#define g_message printf

/** @brief MSB<->LSB */
icUInt16Number
icValue (icUInt16Number val)
{
#if BYTE_ORDER == LITTLE_ENDIAN
# define BYTES 2
# define KORB  4
  unsigned char        *temp  = (unsigned char*) &val;
  static unsigned char  korb[KORB];
  for (int i = 0; i < KORB ; i++ )
    korb[i] = (int) 0;  // leeren

  int klein = 0,
      gross = BYTES - 1;
  for (; klein < BYTES ; klein++ ) {
    korb[klein] = temp[gross--];
#   ifdef DEBUG_ICCFUNKT
    cout << klein << " "; DBG_PROG
#   endif
  }

  unsigned int *erg = (unsigned int*) &korb[0];

# ifdef DEBUG_ICCFUNKT
# if 0
  cout << *erg << " Groesse nach Wandlung " << (int)korb[0] << " "
       << (int)korb[1] << " " << (int)korb[2] << " " <<(int)korb[3]
       << " "; DBG_PROG
# else
  cout << *erg << " Groesse nach Wandlung " << (int)temp[0] << " " << (int)temp[1]
       << " "; DBG_PROG
# endif
# endif
# undef BYTES
# undef KORB
  return (long)*erg;
#else
  return (long)val;
#endif
}

icUInt32Number
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

# ifdef DEBUG_ICCFUNKT
  cout << *erg << " Groesse nach Wandlung " << (int)temp[0] << " "
       << (int)temp[1] << " " << (int)temp[2] << " " <<(int)temp[3]
       << " "; DBG_PROG
# endif

  return (int) *erg;
#else
# ifdef DEBUG_ICCFUNKT
  cout << "BIG_ENDIAN" << " "; DBG_PROG
# endif
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

# ifdef DEBUG_ICCFUNKT
  cout << *erg << " Groesse nach Wandlung " << (int)temp[0] << " "
       << (int)temp[1] << " " << (int)temp[2] << " " <<(int)temp[3]
       << " "; DBG_PROG
# endif
  return (long)*erg;
#else
  return (long)val;
#endif
}

icInt32Number
icValue (icInt32Number val)
{
#if BYTE_ORDER == LITTLE_ENDIAN
# define BYTES 4
# define KORB  4
  unsigned char        *temp  = (unsigned char*) &val;
  static unsigned char  korb[KORB];
  for (int i = 0; i < KORB ; i++ )
    korb[i] = (int) 0;

  int klein = 0,
      gross = BYTES - 1;
  for (; klein < BYTES ; klein++ )
      korb[klein] = temp[gross--];

  signed int *erg = (signed int*) &korb[0];

# ifdef DEBUG_ICCFUNKT
  cout << *erg << " Groesse nach Wandlung " << (int)korb[0] << " "
       << (int)korb[1] << " " << (int)korb[2] << " " <<(int)korb[3]
       << " "; DBG_PROG
# endif
# undef BYTES
# undef KORB
  return (signed int)*erg;
#else
  return (signed int)val;
#endif
}

icInt16Number
icValue (icInt16Number val)
{
#if BYTE_ORDER == LITTLE_ENDIAN
# define BYTES 2
# define KORB  4
  unsigned char        *temp  = (unsigned char*) &val;
  static unsigned char  korb[KORB];
  for (int i = 0; i < KORB ; i++ )
    korb[i] = (int) 0;

  int klein = 0,
      gross = BYTES - 1;
  for (; klein < BYTES ; klein++ )
      korb[klein] = temp[gross--];

  signed int *erg = (signed int*) &korb[0];
# ifdef DEBUG_ICCFUNKT
  cout << *erg << " Groesse nach Wandlung " << (int)korb[0] << " "
       << (int)korb[1] << " " << (int)korb[2] << " " <<(int)korb[3]
       << " "; DBG_PROG
# endif
# undef BYTES
# undef KORB
  return (signed int)*erg;
#else
  return (signed int)val;
#endif
}

icS15Fixed16Number
icValueSF (double val)
{
  return icValue((icS15Fixed16Number)(val * 65536.0 + 0.5));
}

icU16Fixed16Number
icValueUF (double val)
{
  return icValue((icU16Fixed16Number)(val * 65536.0 + 0.5));
}

icUInt16Number
icValueUI16 (double val)
{
  return icValue((icUInt16Number)(val * 65536.0 + 0.5));
}

double
icSFValue (icS15Fixed16Number val)
{
  return icValue(val) / 65536.0;
}

double
icUFValue (icU16Fixed16Number val)
{
  return icValue(val) / 65536.0;
}

double
icUI16Value (icUInt16Number val)
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

void
icValueXYZ (icXYZNumber* ic_xyz,double X, double Y, double Z)
{
  ic_xyz->X = icValueSF(X);
  ic_xyz->Y = icValueSF(Y);
  ic_xyz->Z = icValueSF(Z);
}

// Farbkonvertierungen

void
xyYto_XYZ (std::vector<double> & Y)
{
  if( (Y.size()%3) || Y.size() == 0 ) {
    WARN_S( "xyY ist kein Vielfaches von 3" )
    return;
  }
  for(unsigned int i = 0; i < Y.size()/3; ++i)
  {
    double xyY[3] = {Y[i*3+0],  Y[i*3+1], Y[i*3+2] };
    Y[i*3+0] = xyY[0]/*x*/ * xyY[2]/*Y*/ / xyY[1]/*y*/;
    Y[i*3+1] = xyY[2];
    Y[i*3+2] = (1-xyY[0]-xyY[1]) * xyY[2] / xyY[1];
    DBG_PROG_S( Y[i*3+0] << ", " << Y[i*3+1] << ", " << Y[i*3+2] )
  }
}

void
XYZto_xyY (std::vector<double> & Y)
{
  if( (Y.size()%3) || Y.size() == 0 ) {
    WARN_S( "XYZ ist kein Vielfaches von 3" )
    return;
  }
  for(unsigned int i = 0; i < Y.size()/3; ++i)
  {
    double XYZ[3] = {Y[i*3+0],  Y[i*3+1], Y[i*3+2] };
    double summe = (XYZ[0] + XYZ[1] + XYZ[2]) + 0.0000001;
    Y[i*3+0] = XYZ[0] / summe;
    Y[i*3+1] = XYZ[1] / summe;
    Y[i*3+2] = XYZ[1];
    DBG_PROG_S( Y[i*3+0] << ", " << Y[i*3+1] << ", " << Y[i*3+2] )
  }
}

const double*
XYZto_xyY (double* XYZ)
{
  static double xyY[3];
  double summe = (XYZ[0] + XYZ[1] + XYZ[2]) + 0.0000001;

  xyY[0] = XYZ[0] / summe;
  xyY[1] = XYZ[1] / summe;
  xyY[2] = XYZ[1];

  return &xyY[0];
}


// Namen

std::string
renderingIntentName (int intent)
{
  std::stringstream s;

  switch (intent) 
    {
    case 0:
      s << _("Photometric");
      break;
    case 1:
      s << _("Relative Colorimetric");
      break;
    case 2:
      s << _("Saturation");
      break;
    case 3:
      s << _("Absolute Colorimetric");
      break;
    }
  return s.str();
}

int
getColorSpaceChannels (icColorSpaceSignature color)
{
  int n;

  switch (color) {
    case icSigXYZData: n = 3; break;
    case icSigLabData: n = 3; break;
    case icSigLuvData: n = 3; break;
    case icSigYCbCrData: n = 3; break;
    case icSigYxyData: n = 3; break;
    case icSigRgbData: n = 3; break;
    case icSigGrayData: n = 1; break;
    case icSigHsvData: n = 3; break;
    case icSigHlsData: n = 3; break;
    case icSigCmykData: n = 4; break;
    case icSigCmyData: n = 3; break;
    case icSig2colorData: n = 2; break;
    case icSig3colorData: n = 3; break;
    case icSig4colorData: n = 4; break;
    case icSig5colorData: n = 5; break;
    case icSig6colorData: n = 6; break;
    case icSig7colorData: n = 7; break;
    case icSig8colorData: n = 8; break;
    case icSig9colorData: n = 9; break;
    case icSig10colorData: n = 10; break;
    case icSig11colorData: n = 11; break;
    case icSig12colorData: n = 12; break;
    case icSig13colorData: n = 13; break;
    case icSig14colorData: n = 14; break;
    case icSig15colorData: n = 15; break;
    default: n = 0; break;
  }
  return n;
}

std::string
getColorSpaceName (icColorSpaceSignature color)
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
    default: { icUInt32Number i = icValue(color);
               char t[5];
               memcpy (t,(char*)&i, 4);
               t[4] = 0;
               text = t;
               text += "?";
               break;
             }
  }
  return text;
}

std::vector<std::string>
getChannelNames (icColorSpaceSignature color)
{
  std::vector<std::string> texte;
  std::stringstream s;
# define nFARBEN(n) for (int i = 0; i < n; i++) \
                       { s << i << ". " << _("Colour"); \
                         texte.push_back (s.str()); \
                       }

  switch (color) {
    case icSigXYZData: texte.push_back (_("CIE X"));
                       texte.push_back (_("CIE Y (Luminance)"));
                       texte.push_back (_("CIE Z")); break;
    case icSigLabData: texte.push_back (_("CIE *L"));
                       texte.push_back (_("CIE *a"));
                       texte.push_back (_("CIE *b")); break;
    case icSigLuvData: texte.push_back (_("CIE *L"));
                       texte.push_back (_("CIE *u"));
                       texte.push_back (_("CIE *v")); break;
    case icSigYCbCrData: texte.push_back (_("Luminance Y"));
                       texte.push_back (_("Colour b"));
                       texte.push_back (_("Colour r")); break;
    case icSigYxyData: texte.push_back (_("CIE Y (Luminance)"));
                       texte.push_back (_("CIE x"));
                       texte.push_back (_("CIE y")); break;
    case icSigRgbData: texte.push_back (_("Red"));
                       texte.push_back (_("Green"));
                       texte.push_back (_("Blue")); break;
    case icSigGrayData: texte.push_back (_("Black")); break;
    case icSigHsvData: texte.push_back (_("Hue"));
                       texte.push_back (_("Saturation"));
                       texte.push_back (_("Value")); break;
    case icSigHlsData: texte.push_back (_("Hue"));
                       texte.push_back (_("Lightness"));
                       texte.push_back (_("Saturation")); break;
    case icSigCmykData: texte.push_back (_("Cyan"));
                       texte.push_back (_("Magenta"));
                       texte.push_back (_("Yellow"));
                       texte.push_back (_("Black")); break;
    case icSigCmyData: texte.push_back (_("Cyan"));
                       texte.push_back (_("Magenta"));
                       texte.push_back (_("Yellow")); break;
    case icSig2colorData: nFARBEN(2) break;
    case icSig3colorData: nFARBEN(3) break;
    case icSig4colorData: nFARBEN(4) break;
    case icSig5colorData: nFARBEN(5) break;
    case icSig6colorData: nFARBEN(6) break;
    case icSig7colorData: nFARBEN(7) break;
    case icSig8colorData: nFARBEN(8) break;
    case icSig9colorData: nFARBEN(9) break;
    case icSig10colorData: nFARBEN(10) break;
    case icSig11colorData: nFARBEN(11) break;
    case icSig12colorData: nFARBEN(12) break;
    case icSig13colorData: nFARBEN(13) break;
    case icSig14colorData: nFARBEN(14) break;
    case icSig15colorData: nFARBEN(15) break;
    default: texte.push_back (_("No Colour")); break;
  }
  return texte;
}

#undef nFARBEN

std::string
getDeviceClassName (icProfileClassSignature deviceClass)
{
  std::string text;

  switch (deviceClass)
  {
    case icSigInputClass: text =_("Input"); break;
    case icSigDisplayClass: text =_("Display"); break;
    case icSigOutputClass: text =_("Output"); break;
    case icSigLinkClass: text =_("Link"); break;
    case icSigAbstractClass: text =_("Abstract"); break;
    case icSigColorSpaceClass: text =_("Colour Space"); break;
    case icSigNamedColorClass: text =_("Named Colour"); break;
    default: { icUInt32Number i = icValue(deviceClass);
               char t[5];
               memcpy (t,(char*)&i, 4);
               t[4] = 0;
               text = t;
               text += "?";
               break;
             }
  }
  return text;
}

std::string
getPlatformName (icPlatformSignature platform)
{
  std::string text;

  switch (platform)
  {
    case icSigMacintosh: text =_("Macintosh"); break;
    case icSigMicrosoft: text =_("Microsoft"); break;
    case icSigSolaris: text =_("Solaris"); break;
    case icSigSGI: text =_("SGI"); break;
    case icSigTaligent: text =_("Taligent"); break;
    default: { icUInt32Number i = icValue(platform);
               char t[5];
               memcpy (t,(char*)&i, 4);
               t[4] = 0;
               text = t;
               text += "?";
               break;
             }
  }
  return text;
}

std::string
getSigTagName               ( icTagSignature  sig )
{
  std::string text;

  switch (sig) {
    case icSigAToB0Tag: text = "A2B0"; break;
    case icSigAToB1Tag: text = "A2B1"; break;
    case icSigAToB2Tag: text = "A2B2"; break;
    case icSigBlueColorantTag: text = "bXYZ"; break;
    case icSigBlueTRCTag: text = "bTRC"; break;
    case icSigBToA0Tag: text = "B2A0"; break;
    case icSigBToA1Tag: text = "B2A1"; break;
    case icSigBToA2Tag: text = "B2A2"; break;
    case icSigCalibrationDateTimeTag: text = "calt"; break;
    case icSigCharTargetTag: text = "targ"; break;
    case icSigCopyrightTag: text = "cprt"; break;
    case icSigCrdInfoTag: text = "crdi"; break;
    case icSigDeviceMfgDescTag: text = "dmnd"; break;
    case icSigDeviceModelDescTag: text = "dmdd"; break;
    case icSigGamutTag: text = "gamt"; break;
    case icSigGrayTRCTag: text = "kTRC"; break;
    case icSigGreenColorantTag: text = "gXYZ"; break;
    case icSigGreenTRCTag: text = "gTRC"; break;
    case icSigLuminanceTag: text = "lumi"; break;
    case icSigMeasurementTag: text = "meas"; break;
    case icSigMediaBlackPointTag: text = "bkpt"; break;
    case icSigMediaWhitePointTag: text = "wtpt"; break;
    case icSigNamedColorTag: text = "'ncol"; break;
    case icSigNamedColor2Tag: text = "ncl2"; break;
    case icSigPreview0Tag: text = "pre0"; break;
    case icSigPreview1Tag: text = "pre1"; break;
    case icSigPreview2Tag: text = "pre2"; break;
    case icSigProfileDescriptionTag: text = "desc"; break;
    case 1685283693: text = "dscm"; break;
    case icSigProfileSequenceDescTag: text = "pseq"; break;
    case icSigPs2CRD0Tag: text = "psd0"; break;
    case icSigPs2CRD1Tag: text = "psd1"; break;
    case icSigPs2CRD2Tag: text = "psd2"; break;
    case icSigPs2CRD3Tag: text = "psd3"; break;
    case icSigPs2CSATag: text = "ps2s"; break;
    case icSigPs2RenderingIntentTag: text = "ps2i"; break;
    case icSigRedColorantTag: text = "rXYZ"; break;
    case icSigRedTRCTag: text = "rTRC"; break;
    case icSigScreeningDescTag: text = "scrd"; break;
    case icSigScreeningTag: text = "scrn"; break;
    case icSigTechnologyTag: text = "tech"; break;
    case icSigUcrBgTag: text = "bfd"; break;
    case icSigViewingCondDescTag: text = "vued"; break;
    case icSigViewingConditionsTag: text = "view"; break;
    case 1147500100: text = "DevD"; break;
    case 1128875332: text = "CIED"; break;
    case 1349350514: text = "Pmtr"; break;
    case 1986226036: text = "vcgt"; break;
    case 1667785060: text = "chad"; break;
    case icSigChromaticityType: text = "chrm"; break;
    case 1668051567: text = "clro"; break;
    case 1668051572: text = "clrt"; break;
    case 0: text = "----"; break;
    default: { icUInt32Number i = icValue(sig);
               char t[5];
               memcpy (t,(char*)&i, 4);
               t[4] = 0;
               text = t;
               text += "?";
               break;
             }
  }
# ifdef DEBUG_ICCTAG_
  char c[5] = "clrt";
  long* l = (long*) &c[0];
  cout << *l << ": " << (long)"clrt" << " "; DBG_PROG
# endif
  return text;
}

std::string
getSigTagDescription                            ( icTagSignature  sig )
{
  std::string text = _("Description");

  switch (sig) {
    case icSigAToB0Tag: text = _("Lockup table, device to PCS, intent photometric"); break;
    case icSigAToB1Tag: text = _("Lockup table, device to PCS, intent relative colorimetric"); break;
    case icSigAToB2Tag: text = _("Lockup table, device to PCS, intent saturation"); break;
    case icSigBlueColorantTag: text = _("Blue Colorant"); break;
    case icSigBlueTRCTag: text = _("Blue tone reproduction curve"); break;
    case icSigBToA0Tag: text = _("Lockup table, PCS to device, intent photometric"); break;
    case icSigBToA1Tag: text = _("Lockup table, PCS to device, intent relative colorimetric"); break;
    case icSigBToA2Tag: text = _("Lockup table, PCS to device, intent saturation"); break;
    case icSigCalibrationDateTimeTag: text = _("Calibration date"); break;
    case icSigCharTargetTag: text = _("Colour measurement data"); break;
    case icSigCopyrightTag: text = _("Copyright"); break;
    case icSigCrdInfoTag: text = _("crdi"); break;
    case icSigDeviceMfgDescTag: text = _("Device manufacturerer description"); break;
    case icSigDeviceModelDescTag: text = _("Device model description"); break;
    case icSigGamutTag: text = _("gamut"); break;
    case icSigGrayTRCTag: text = _("Gray tone reproduction curve"); break;
    case icSigGreenColorantTag: text = _("Green Colorant"); break;
    case icSigGreenTRCTag: text = _("Green tone reproduction curve"); break;
    case icSigLuminanceTag: text = _("Luminance"); break;
    case icSigMeasurementTag: text = _("Measurement"); break;
    case icSigMediaBlackPointTag: text = _("Media black point"); break;
    case icSigMediaWhitePointTag: text = _("Media white point"); break;
    case icSigNamedColorTag: text = _("Named Colour"); break;
    case icSigNamedColor2Tag: text = _("Named Colour 2"); break;
    case icSigPreview0Tag: text = _("Preview, photografic"); break;
    case icSigPreview1Tag: text = _("Preview, relative colorimetric"); break;
    case icSigPreview2Tag: text = _("Preview, saturated"); break;
    case icSigProfileDescriptionTag: text = _("Profile description"); break;
    case 1685283693: text = _("Profile description, multilingual"); break;//dscm
    case icSigProfileSequenceDescTag: text = _("Profile sequence description"); break;
    case icSigPs2CRD0Tag: text = _("psd0"); break;
    case icSigPs2CRD1Tag: text = _("psd1"); break;
    case icSigPs2CRD2Tag: text = _("psd2"); break;
    case icSigPs2CRD3Tag: text = _("psd3"); break;
    case icSigPs2CSATag: text = _("ps2s"); break;
    case icSigPs2RenderingIntentTag: text = _("ps2i"); break;
    case icSigRedColorantTag: text = _("Red Colorant"); break;
    case icSigRedTRCTag: text = _("Red tone reproduction curve"); break;
    case icSigScreeningDescTag: text = _("scrd"); break;
    case icSigScreeningTag: text = _("scrn"); break;
    case icSigTechnologyTag: text = _("Technologie"); break;
    case icSigUcrBgTag: text = _("bfd"); break;
    case icSigViewingCondDescTag: text = _("Viewing conditions description"); break;
    case icSigViewingConditionsTag: text = _("Viewing Conditions"); break;
    case 1147500100: text = _("Device colours"); break;//DevD
    case 1128875332: text = _("Measured colours"); break;//CIED
    case 1349350514: text = _("Profiling parameters"); break;//Pmtr
    case 1986226036: text = _("VideoCardGammaTable"); break;//vcgt
    case 1667785060: text = _("Colour adaption matrix"); break; //chad
    case icSigChromaticityType: text = _("Chromaticity"); break; //chrm
    case 1668051567: text = _("Named colour order"); break;//clro
    case 1668051572: text = _("Named colour names"); break;//clrt
    case 0: text = _("----"); break;
    default: { icUInt32Number i = icValue(sig);
               char t[5];
               memcpy (t,(char*)&i, 4);
               t[4] = 0;
               text = t;
               text += "?";
               break;
             }
  }
  return text;
}

std::string
getSigTypeName               ( icTagTypeSignature  sig )
{
  std::string text;

  switch (sig) {
    case icSigCurveType: text = "curv"; break;
    case icSigDataType: text = "data"; break;
    case icSigDateTimeType: text = "dtim"; break;
    case icSigLut16Type: text = "mft2"; break;
    case icSigLut8Type: text = "mft1"; break;
    case icSigMeasurementType: text = "meas"; break;
    case icSigNamedColorType: text = "ncol"; break;
    case icSigProfileSequenceDescType: text = "pseq"; break;
    case icSigS15Fixed16ArrayType: text = "sf32"; break;
    case icSigScreeningType: text = "scrn"; break;
    case icSigSignatureType: text = "sig"; break;
    case icSigTextType: text = "text"; break;
    case icSigTextDescriptionType: text = "desc"; break;
    case icSigU16Fixed16ArrayType: text = "uf32"; break;
    case icSigUcrBgType: text = "bfd"; break;
    case icSigUInt16ArrayType: text = "ui16"; break;
    case icSigUInt32ArrayType: text = "ui32"; break;
    case icSigUInt64ArrayType: text = "ui64"; break;
    case icSigUInt8ArrayType: text = "ui08"; break;
    case icSigViewingConditionsType: text = "view"; break;
    case icSigXYZType: text = "XYZ"; break;
    //case icSigXYZArrayType: text = "XYZ"; break;
    case icSigNamedColor2Type: text = "ncl2"; break;
    case icSigCrdInfoType: text = "crdi"; break;
    case icSigChromaticityType: text = "chrm"; break;
    case 1986226036: text = "vcgt"; break;
    case icSigCopyrightTag: text = "cprt?"; break; //??? (Imacon)
    case 1835824483: text = "mluc"; break;
    default: { icUInt32Number i = icValue(sig);
               char t[5];
               memcpy (t,(char*)&i, 4);
               t[4] = 0;
               text = t;
               text += "?";
               break;
             }
  }
  return text;
}

std::string
getSigTechnology             ( icTechnologySignature sig )
{
  std::string text;
  switch (sig) {
    case icSigDigitalCamera: text = _("Digital camera"); break; //dcam
    case icSigFilmScanner: text = _("Film scanner"); break; //fscn
    case icSigReflectiveScanner: text = _("Reflective scanner"); break; //rscn
    case icSigInkJetPrinter: text = _("InkJet printer"); break; //ijet
    case icSigThermalWaxPrinter: text = _("Thermal wax printer"); break; //twax
    case icSigElectrophotographicPrinter: text = _("Electrophotographic printer"); break; //epho
    case icSigElectrostaticPrinter: text = _("Electrostatic printer"); break; //esta
    case icSigDyeSublimationPrinter: text = _("Dye sublimation printer"); break; //dsub
    case icSigPhotographicPaperPrinter: text = _("Photographic paper printer"); break; //rpho
    case icSigFilmWriter: text = _("Film writer"); break; //fprn
    case icSigVideoMonitor: text = _("Video Monitor"); break; //vidm
    case icSigVideoCamera: text = _("Video camera"); break; //vidc
    case icSigProjectionTelevision: text = _("Projection Television"); break; //pjtv
    case icSigCRTDisplay: text = _("Cathode ray tube display"); break; //CRT
    case icSigPMDisplay: text = _("Passive matrix monitor"); break; //PMD
    case icSigAMDisplay: text = _("Active matrix monitor"); break; //AMD
    case icSigPhotoCD: text = _("Photo CD"); break; //KPCD
    case icSigPhotoImageSetter: text = _("PhotoImageSetter"); break; //imgs
    case icSigGravure: text = _("Gravure"); break; //grav
    case icSigOffsetLithography: text = _("Offset Lithography"); break; //offs
    case icSigSilkscreen: text = _("Silkscreen"); break; //silk
    case icSigFlexography: text = _("Flexography"); break; //flex
    case icMaxEnumTechnology: text = _("----"); break;   
    default: { icUInt32Number i = icValue(sig);
               char t[5];
               memcpy (t,(char*)&i, 4);
               t[4] = 0;
               text = t;
               text += "?";
               break;
             }
  }
  return text;
}

std::string
getChromaticityColorantType( int type )
{
  std::string text;
  switch (type) {
    case 0: text = ""; break;
    case 1: text = _("ITU-R BT.709"); break;
    case 2: text = _("SMPTE RP145-1994"); break;
    case 3: text = _("EBU Tech.3213-E"); break;
    case 4: text = _("P22"); break;

    default: DBG text = _("???"); break;
  }
  return text;
}

std::string
getIlluminant             ( icIlluminant sig )
{
  std::string text;
  switch (sig) {
    case icIlluminantUnknown: text = _("Illuminant unknown"); break;
    case icIlluminantD50: text = _("Illuminant D50"); break;
    case icIlluminantD65: text = _("Illuminant D65"); break;
    case icIlluminantD93: text = _("Illuminant D93"); break;
    case icIlluminantF2: text = _("Illuminant F2"); break;
    case icIlluminantD55: text = _("Illuminant D55"); break;
    case icIlluminantA: text = _("Illuminant A"); break;
    case icIlluminantEquiPowerE: text = _("Illuminant with equal energy E"); break;
    case icIlluminantF8: text = _("Illuminant F8"); break;
    case icMaxEnumIluminant: text = _("Illuminant ---"); break;

    default: text = _("???"); break;
  }
  return text;
}

std::string
getStandardObserver             ( icStandardObserver sig )
{
  std::string text;
  switch (sig) {
    case icStdObsUnknown: text = _("unknown"); break;
    case icStdObs1931TwoDegrees: text = _("2 degree (1931)");
         break;
    case icStdObs1964TenDegrees: text = _("10 degree (1964)");
         break;
    case icMaxStdObs: text = _("---"); break;

    default: text = _("???"); break;
  }
  return text;
}

std::string
getMeasurementGeometry             ( icMeasurementGeometry sig )
{
  std::string text;
  switch (sig) {
    case icGeometryUnknown: text = _("unknown"); break;
    case icGeometry045or450: text = _("0/45, 45/0"); break;
    case icGeometry0dord0: text = _("0/d or d/0"); break;
    case icMaxGeometry: text = _("---"); break;

    default: text = _("???"); break;
  }
  return text;
}

std::string
getMeasurementFlare             ( icMeasurementFlare sig )
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

std::string
printDatum                      (icDateTimeNumber date)
{ DBG_PROG
  std::stringstream s;
    s <<
                       icValue(date.day)     << "/" <<
                       icValue(date.month)   << "/" <<
                       icValue(date.year)    << " " <<
                       icValue(date.hours)   << ":";
    if (icValue(date.minutes) < 10)
      s << "0";
                  s << icValue(date.minutes) << " " << _("o'clock") << " " <<
                       icValue(date.seconds) << " " << _("seconds");
  return s.str();
}

namespace icc_examin_ns {

#          if defined(LINUX) || defined(APPLE) || defined(SOLARIS)
# define   ZEIT_TEILER 10000
#          else // WINDOWS TODO
# define   ZEIT_TEILER CLOCKS_PER_SEC;
#          endif

# include <sys/time.h>
#include <time.h>
#include <unistd.h>
#include <math.h>
  double zeitSekunden()
  {
           time_t zeit_ = zeit();
           double teiler = ZEIT_TEILER;
           double dzeit = zeit_ / teiler;
    return dzeit;
  }
  time_t zeit()
  {
           time_t zeit_;
           double teiler = ZEIT_TEILER;
#          if defined(__GNUC__) || defined(APPLE) || defined(SOLARIS) || defined(BSD)
           struct timeval tv;
           gettimeofday( &tv, NULL );
           double tmp_d;
           zeit_ = tv.tv_usec/(1000000/(time_t)teiler)
                   + (time_t)(modf( (double)tv.tv_sec / teiler,&tmp_d )
                     * teiler*teiler);
           //DBG_THREAD_V( modf(tv.tv_sec/teiler,&tmp_d)*teiler*teiler<<","<<
             //            tv.tv_usec/(1000000/teiler) )
#          else // WINDOWS TODO
           zeit_ = clock();
#          endif
    return zeit_;
  }
  time_t zeitProSekunde()
  {
    return ZEIT_TEILER;
  }
  void sleep(double sekunden)
  {
#            if defined(__GCC__) || defined(__APPLE__)
             timespec ts;
             double ganz;
             double rest = modf(sekunden, &ganz);
             ts.tv_sec = (time_t)ganz;
             ts.tv_nsec = (time_t)(rest * 1000000000);
             //DBG_PROG_V( sekunden<<" "<<ts.tv_sec<<" "<<ganz<<" "<<rest )
             nanosleep(&ts, 0);
#            else
             usleep((time_t)(sekunden/(double)CLOCKS_PER_SEC));
#            endif
  }
  void wait(double sekunden, int aktualisieren)
  {
    icc_examin_ns::sleep(sekunden);
    if(aktualisieren && waitFunc)
      waitFunc();
  }
}

std::string
zeig_bits_bin(const void* speicher, int groesse)
{
  std::string text;
  int byte_zahl;
  char txt[12];

  for (int k = 0; k < groesse; k++)
  {   for (int i = 8-1; i >= 0; i--)
      {
              unsigned char* u8 = (unsigned char*)speicher;
              byte_zahl = (u8[k] >> i) & 1;
              sprintf (&txt[7-i], "%d", byte_zahl);
      }
      text.append( txt, strlen (txt));
      text.append( " ", 1); /*aller 8 bit ein leerzeichen*/
  }

  return text;
}

// aus oyranos.c
#include <errno.h>
#include <sys/stat.h>
int
isFileFull (const char* fullFileName)
{ DBG_PROG_START
  struct stat status;
  int r = 0;
  const char* name = fullFileName;

  DBG_PROG_S("fullFileName = \"" << fullFileName << "\"" )
  status.st_mode = 0;
  r = stat (name, &status);

  DBG_MEM_S("status.st_mode = " << (int)((status.st_mode&S_IFMT)&S_IFDIR) ) 
  DBG_MEM_S("status.st_mode = " << (int)status.st_mode )
  DBG_MEM_S("name = " << name )
  DBG_MEM_V( r )
  switch (r)
  {
    case EACCES:       WARN_S("EACCES = " << r); break;
    case EIO:          WARN_S("EIO = " << r); break;
    case ELOOP:        WARN_S("ELOOP = " << r); break;
    case ENAMETOOLONG: WARN_S("ENAMETOOLONG = " << r); break;
    case ENOENT:       WARN_S("ENOENT = " << r); break;
    case ENOTDIR:      WARN_S("ENOTDIR = " << r); break;
    case EOVERFLOW:    WARN_S("EOVERFLOW = " << r); break;
  }

  r = !r &&
       (   ((status.st_mode & S_IFMT) & S_IFREG)
        || ((status.st_mode & S_IFMT) & S_IFLNK));

  DBG_MEM_V( r )
  if (r)
  {
    FILE* fp = fopen (name, "r");
    if (!fp) {
      r = 0;
    } else {
      fclose (fp);
    }
  } 

  DBG_PROG_ENDE
  return r;
}


char*
ladeDatei ( std::string dateiname, size_t *size )
{ DBG_PROG_START

    char* data = 0;
    *size = 0;

    std::ifstream f ( dateiname.c_str(), std::ios::binary | std::ios::ate );

    DBG_PROG_V( dateiname )
    if (dateiname == "" || !isFileFull(dateiname.c_str()) || !f)
    { DBG_MEM
      if(dateiname != _("Monitor Profile")) 
      {
        if(dateiname == "") WARN_S( "kein Dateiname" );
        if(!isFileFull(dateiname.c_str())) WARN_S( "keine Datei: " << dateiname );
        if(!f) WARN_S( "fehlerhafte Datei: " << dateiname );
#       if HAVE_EXCEPTION
        DBG_PROG_ENDE
        throw ausn_file_io (dateiname.c_str());
#       endif
        dateiname = "";
      }
#     if !HAVE_EXCEPTION
      goto ERROR;
#     endif
    }
    DBG_MEM

    *size = (unsigned int)f.tellg();
    DBG_MEM_V ( *size << "|" << f.tellg() )
    f.seekg(0);
    if(*size) {
      data = (char*)calloc (sizeof (char), *size+1);
      f.read ((char*)data, *size);
      DBG_MEM_V ( *size << "|" << f.tellg() <<" "<< (int*)data <<" "<< strlen(data) )
      f.close();
    } else {
      data = 0;
      WARN_S( _("Dateigroesse 0 fuer ") << dateiname )
    }

  ERROR:

  DBG_PROG_ENDE
  return data;
}

void
saveMemToFile (const char* filename, const char *block, int size)
{ DBG_PROG_START
  FILE *fp=NULL;
  int   pt = 0;

  if ((fp=fopen(filename, "w")) != NULL &&
      block && size) {
    do {
      fputc ( block[pt++] , fp);
    } while (--size);
  }

  if (fp)
    fclose (fp);
  DBG_PROG_ENDE
}

Speicher
dateiNachSpeicher (const std::string & dateiname)
{ DBG_PROG_START
  size_t groesse;
  char *block = ladeDatei(dateiname, &groesse);
  Speicher s (block,groesse);
  s = dateiname;
  s.zeit ((time_t)holeDateiModifikationsZeit( dateiname.c_str() ));
  DBG_PROG_V( s.zeit() )
  if(block) free(block);
  DBG_PROG_ENDE
  return s;
}

void
dateiNachSpeicher (Speicher & s, const std::string & dateiname)
{ DBG_PROG_START
  size_t groesse;
  char *block = ladeDatei(dateiname, &groesse);
  s.lade (block,groesse);
  s = dateiname;
  s.zeit ((time_t)holeDateiModifikationsZeit( dateiname.c_str() ));
  if(block) free(block);
  DBG_PROG_V( s.zeit() )
  DBG_PROG_ENDE
}

void
speicherNachDatei (std::string & dateiname, Speicher & s)
{ DBG_PROG_START
  const char  *block = s;
  size_t groesse = s.size();
  saveMemToFile( dateiname.c_str(), block, groesse );
  DBG_PROG_ENDE
}


#include <sys/stat.h>

double
holeDateiModifikationsZeit (const char* fullFileName)
{ DBG_MEM_START
  struct stat status;
  int r = 0;
  const char* name = fullFileName;

  status.st_mode = 0;
  r = stat (name, &status);
  r = !r &&
       (   ((status.st_mode & S_IFMT) & S_IFREG)
        || ((status.st_mode & S_IFMT) & S_IFLNK));

  double m_zeit = 0.0;
  if (r)
  {
#   if defined(APPLE) || defined(BSD)
    m_zeit = status.st_mtime ;
    m_zeit += status.st_mtimespec.tv_nsec/1000000. ;
#   else
    m_zeit = status.st_mtim.tv_sec ;
    DBG_MEM_V( status.st_mtim.tv_sec )
    m_zeit += status.st_mtim.tv_nsec/1000000. ;
    DBG_MEM_V( status.st_mtim.tv_nsec )
#   endif
  }

  DBG_MEM_ENDE
  return m_zeit;
}


#ifdef WIN32
#define DIR_SEPARATOR_C '\\'
#define DIR_SEPARATOR "\\"
#else
#define DIR_SEPARATOR_C '/'
#define DIR_SEPARATOR "/" 
#endif

char*
getExecPath(const char *filename)
{
  DBG_PROG_START
  char *exec_path = NULL;

  if (filename)
  {
    int len = strlen(filename) * 2 + 1024;
    char *text = (char*) calloc( sizeof(char), len );
    text[0] = 0;
    /* whats the path for the executeable ? */
    snprintf (text, len-1, filename);

    if(text[0] == '~')
    {
      /* home directory */
      if(getenv("HOME"))
        sprintf( text, "%s%s", getenv("HOME"), &filename[0]+1 );
    }

    /* relative names - where the first sign is no directory separator */
    if (text[0] != DIR_SEPARATOR_C)
    {
      FILE *pp = NULL;
  
      if (text) free (text);
      text = (char*) malloc( 1024 );

      /* Suche das ausfuehrbare Programm
         TODO symbolische Verknuepfungen */
      snprintf( text, 1024, "which %s", filename);
      pp = popen( text, "r" );
      if (pp) {
        if (fscanf (pp, "%s", text) != 1)
        {
          pclose (pp);
          printf( "no executeable path found\n" );
        }
      } else { 
        printf( "could not ask for executeable path\n" );
      }
    
      if(text[0] != DIR_SEPARATOR_C)
      {
        char* cn = (char*) calloc(2048, sizeof(char));
        sprintf (cn, "%s%s%s", getenv("PWD"), DIR_SEPARATOR, filename);
        sprintf (text, cn);
        if(cn) free(cn); 
      }
    }

    { /* remove the executable name */
      char *tmp = strrchr(text, DIR_SEPARATOR_C);
      if(tmp)
        *tmp = 0;
    }
    while (text[strlen(text)-1] == '.')
      text[strlen(text)-1] = 0;
    while (text[strlen(text)-1] == DIR_SEPARATOR_C)
      text[strlen(text)-1] = 0;

    exec_path = text;
  }
  DBG_PROG_ENDE
  return exec_path;
}

#include "fl_i18n/fl_i18n.H"

void
setI18N( const char *exename )
{
  const char *locale_paths[4] = {0,0,0,0};
  signed int is_path = -1;
  int num_paths = 0;
# if __APPLE__
  std::string bdr;
  // RESOURCESPATH is set in the bundle by "Contents/MacOS/ICC Examin.sh"
  if(getenv("RESOURCESPATH")) {
    bdr = getenv("RESOURCESPATH");
    bdr += "/locale";
    locale_paths[0] = bdr.c_str(); ++num_paths;
  }
  if(!locale_paths[0]) {
    bdr = icc_examin_ns::holeBundleResource("locale","");
    if(bdr.size())
    {
      locale_paths[0] = bdr.c_str();
      ++num_paths;
    }
  }
  locale_paths[1] = LOCALEDIR; ++num_paths;
  locale_paths[2] = SRC_LOCALEDIR; ++num_paths;
# else
  locale_paths[0] = LOCALEDIR; ++num_paths;

  DBG_NUM_V( exename )

  { const char *reloc_path = {"../share/locale"};
    int len = (strlen(exename) + strlen(reloc_path)) * 2 + 128;
    char *path = (char*) malloc( len ); // small one time leak
    char *text = NULL;

    text = getExecPath( exename );
    snprintf (path, len-1, "%s%s%s", text, DIR_SEPARATOR, reloc_path);
    locale_paths[1] = path; ++num_paths;
    locale_paths[2] = SRC_LOCALEDIR; ++num_paths;
    DBG_NUM_V( path );
    if (text) free (text);
  }
# endif
  const char* tdd = getenv("TEXTDOMAINDIR");
  if(tdd)
  {
    locale_paths[num_paths] = tdd;
    DBG_V( num_paths <<" "<< tdd );
    ++num_paths;
  }

  is_path = fl_search_locale_path (num_paths, locale_paths, "de", "icc_examin");
  if(is_path >= 0) {
    fl_initialise_locale ( "icc_examin", locale_paths[is_path], 1 );
    DBG_NUM_S( "locale gefunden in: " << locale_paths[is_path] )
  } else {
    if(is_path < 0)
    {
      is_path = fl_search_locale_path (num_paths, locale_paths, "de",
                strrchr(exename, DIR_SEPARATOR_C)+1 );

      if(is_path >= 0) {
        fl_initialise_locale ( strrchr(exename, DIR_SEPARATOR_C)+1,
                               locale_paths[is_path], 1 );
        DBG_NUM_S( "locale gefunden in: " << locale_paths[is_path] )
      }
    }
  }
}

namespace icc_parser {

const char *alnum = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz_|/-+=()[]{}<>&?!:;,.0123456789";
const char *numerisch = "-+,.0123456789";
const char *ziffer = "0123456789";
const char *alpha = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz_|/=()[]{}<>&?!:;";
const char *leer_zeichen = " \t\n\v\f\r";

int
suchenErsetzen          (std::string &text,
                         std::string &suchen,
                         std::string &ersetzen,
                         std::string::size_type pos)
{
  int n = 0;
  while ((pos = text.find (suchen, pos)) != std::string::npos) {
    text.replace (pos, suchen.size(), ersetzen);
    pos = pos + ersetzen.size();
    ++n;
  }

  return n;
}

int
suchenErsetzen          (std::string &text,
                         const char  *suchen,
                         const char  *ersetzen,
                         std::string::size_type  pos)
{
  static std::string such, ersetz;
  such = suchen;
  ersetz = ersetzen;
  int n = suchenErsetzen(text, such, ersetz, pos);
  return n;
}

//#define PARSER_DEBUG
#ifdef PARSER_DEBUG
#define DBG_PARSER DBG_PROG
#define DBG_PARSER_START DBG_PROG_START
#define DBG_PARSER_ENDE DBG_PROG_ENDE
#define DBG_PARSER_S( text ) DBG_NUM_S( text )
#define DBG_PARSER_V( text ) DBG_NUM_V( text )
#else
#define DBG_PARSER
#define DBG_PARSER_START
#define DBG_PARSER_ENDE
#define DBG_PARSER_S( text ) 
#define DBG_PARSER_V( text )
#endif

std::vector<std::string>
zeilenNachVector(std::string &text)
{ DBG_PROG_START
  // fueilen aus einen Text in einen Vector
  std::vector <std::string> texte;

      int len = strlen(text.c_str());
      std::string text_line;
      char c;
      const char *chars = text.c_str();
      for (int zeichen = 0; zeichen < len; zeichen++) {
        c = chars[zeichen];
        if (c == '\n' || (int)c == 0) { // LF oder 0
          text_line += '\0';
          texte.push_back(text_line.c_str());
          text_line.clear();
        } else
          text_line += c;
      }
      if (text_line.size() > 0) // falls was uebrig bleibt
        texte.push_back(text_line.c_str());

  DBG_PROG_ENDE
  return texte;
}

std::string::size_type
sucheWort         ( std::string            &text,
                    std::string             wort,
                    std::string::size_type  pos )
{ DBG_PARSER_START
  std::string::size_type pos_ = std::string::npos;
  bool fertig = false;

  while( !fertig )
  { // vielleicht etwas viel Aufwand ...
    if( (pos = text.find( wort, pos )) != std::string::npos )
    { // Bestaetige das Ende des Wortes
      //DBG_NUM_V( (pos + wort.size()) <<" "<< text.size() )
      if( (text[pos + wort.size()] == 0    ||  // NUL
           text[pos + wort.size()] == ' '  ||  // SP
           text[pos + wort.size()] == '\t' ||  // HT
           text[pos + wort.size()] == '\n' ||  // LF
           text[pos + wort.size()] == '\v' ||  // VT
           text[pos + wort.size()] == '\f' ||  // FF
           text[pos + wort.size()] == '\r' ||  // CR
           (pos + wort.size()) == text.size() ) )
      {
        pos_ = pos;
        fertig = true;
      } else
        ++pos;
    } else if( pos == std::string::npos )
      fertig = true;
  }

  DBG_PARSER_ENDE
  return pos_;
}

ZifferWort zifferWort( std::string & wert )
{ ZifferWort t; t.wort.first = true; t.wort.second = wert; return t; }
ZifferWort zifferWort( double wert )
{ ZifferWort t; t.zahl.first = true; t.zahl.second = wert; return t; }
ZifferWort zifferWort( int wert )
{ ZifferWort t; t.ganz_zahl.first = true; t.ganz_zahl.second = wert; return t; }


std::vector<ZifferWort>
unterscheideZiffernWorte ( std::string &zeile,
                           bool         anfuehrungsstriche_setzen,
                           const char  *trennzeichen )
{ DBG_PARSER_START
  std::string::size_type pos = 0, ende = 0, pos2, pos3;
  static char text[64];
  bool in_anfuehrung = false;
  std::string txt;
  std::vector<ZifferWort> ergebnis;
  std::vector<std::string> worte;
  char trenner [16];

  if( trennzeichen ) { DBG_PARSER
    sprintf (trenner, trennzeichen );
  } else {
    sprintf (trenner, leer_zeichen );
    suchenErsetzen( zeile, ",", ".", 0 );
  }

  // Worte Suchen und von Zahlen scheiden
  for( pos = 0; pos < zeile.size() ; ++pos )
  { DBG_PARSER_V( pos <<" "<< zeile.size() )
    in_anfuehrung = false;
    pos2 = pos;

    // erstes Zeichen suchen
    if( (pos = zeile.find_first_of( alnum, pos )) != std::string::npos )
    {
      bool anf_zaehlen = true;
      // erstes Anfuehrungszeichen suchen
      if( zeile.find_first_of( "\"", pos2 ) != std::string::npos &&
          zeile.find_first_of( "\"", pos2 ) < pos )
        pos2 = zeile.find_first_of( "\"", pos2 );
      else
        anf_zaehlen = false;
      DBG_PARSER_V( pos2 )

      // Anfuehrungszeichen zaehlen [ ""  " "  ABC ] - zeichenweise
      int letzes_anf_zeichen = -1;
      if( anf_zaehlen )
        for( pos3 = pos2; pos3 < pos; ++pos3)
          if( zeile[pos3] == '"' &&
              letzes_anf_zeichen >= 0 )
            letzes_anf_zeichen = -1;
          else
            letzes_anf_zeichen = pos3;

      // falls ein Anfuehrungszeichen vor dem Wort ungerade sind // ["" " ABC ]
      if( letzes_anf_zeichen >= 0 )
      {
        in_anfuehrung = true;
        pos2 = pos;
        pos = letzes_anf_zeichen+1;
      }
      DBG_PARSER_V( pos )
      // das Ende des Wortes finden
      if( in_anfuehrung )
        ende = zeile.find_first_of( "\"", pos + 1 );
      else
        if( (ende = zeile.find_first_of( trenner, pos2 + 1 ))
            == std::string::npos )
          ende = zeile.size();

      // bitte das erste Wort einbeziehen
      if( zeile.find_first_of( alnum, pos ) > ende )
        ende = zeile.find_first_of( trenner,
                                zeile.find_first_of( alnum, pos ) + 1 );
      // das Wort extrahieren
      txt = zeile.substr( pos, ende-pos );
      DBG_PARSER_V( pos <<" "<< ende )
#     ifdef PARSER_DEBUG
      cout << zeile << endl;
      for(unsigned int j = 0; j < zeile.size();    ++j)
        if( j != pos && j != ende )
          cout << " ";
        else
          cout << "^";
      cout << "\n";
#     endif
      // das Wort untersuchen
      if( txt.find_first_of( numerisch ) != std::string::npos &&
          txt.find( "." ) != std::string::npos &&
          txt.find_first_of( alpha ) == std::string::npos )
      { // ist Fliesskommazahl
        pos += txt.size();
        sprintf( text, "%f", atof( zeile.substr( pos, ende-pos ).c_str() ) );
        DBG_PARSER_S( "Fliesskommazahl: " << txt )
        ergebnis.push_back( zifferWort((double)atof(txt.c_str())) );
        DBG_PARSER_S( "pruefen: "<< ergebnis[ergebnis.size()-1].zahl.first <<" "<< ergebnis[ergebnis.size()-1].zahl.second )
      } else
      if( txt.find_first_of( ziffer ) != std::string::npos &&
          txt.find( "." ) == std::string::npos &&
          txt.find_first_of( alpha ) == std::string::npos )
      { // ist Ganzzahl
        pos += txt.size();
        sprintf( text, "%d", atoi( zeile.substr( pos, ende-pos ).c_str() ) );
        ergebnis.push_back( zifferWort(atoi(txt.c_str())) );
        DBG_PARSER_S( "Ganzzahl: " << txt )
      } else
      if( txt.find_first_of( alnum ) != std::string::npos )
      { // ist Text
        bool neusetzen = false;
        int anf = 0;
        ergebnis.push_back( zifferWort(txt) );
        DBG_PARSER_S( "Text: " << txt )
        DBG_PARSER_V( in_anfuehrung )
        // Anfuehrungszeichen Beginn
        if( !in_anfuehrung && anfuehrungsstriche_setzen )
        {
          zeile.insert( pos, "\"" );
          in_anfuehrung = true;
          neusetzen = true;
          ++pos;
          ++anf;
        }
        if( anf_zaehlen )
          in_anfuehrung = false;
        // Ende des Wortes
        if( (pos2 = zeile.find_first_of( trenner, pos ))
            == std::string::npos)
          pos2 = zeile.size();
        // Es folgt ein Anfuehrungszeichen
        if( pos2 > zeile.find_first_of( "\"", pos ) &&
            zeile.find_first_of( "\"", pos ) != std::string::npos &&
            !neusetzen )
          pos2 = zeile.find_first_of( "\"", pos );
        
        if( in_anfuehrung && anfuehrungsstriche_setzen )
        { DBG_PARSER_V( zeile )
          zeile.insert( pos2, "\"" );
          in_anfuehrung = false;
          ++anf;
        }
        ende += anf;
        pos = ende;
      }
      DBG_PARSER_V( pos << zeile.size() )
      // Schleife beenden
      if( zeile.find_first_of( alnum, pos ) == std::string::npos )
      {
        DBG_PARSER_V( zeile.find_first_of( alnum, pos ) )
        pos = zeile.size();
      }
      DBG_PARSER_V( pos << zeile )
    } else
      pos = zeile.size();
  }
  for( unsigned i = 0; i < worte.size(); i++)
    DBG_PARSER_V( worte[i] );

  DBG_PARSER_ENDE
  return ergebnis;
}

}



