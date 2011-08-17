/*
 * ICC Examin ist eine ICC Profil Betrachter
 * 
 * Copyright (C) 2004-2010  Kai-Uwe Behrmann 
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
 * preparation of ICC inner informations
 * 
 */

// Date:      04. 05. 2004

#if 0
# ifndef DEBUG
#  define DEBUG
# endif
# define DEBUG_ICCFUNKT
#endif

#include <icc34.h>
#include "icc_utils.h"
#include "icc_formeln.h"
#include "icc_helfer.h"
#include "icc_oyranos.h"
#include "icc_profile.h"
#include "icc_examin_version.h"

#include <cstring>

#define g_message printf

/** @brief MSB<->LSB */
icUInt16Number
icValue (icUInt16Number val)
{
  return oyValueUInt16 (val);
}

icUInt32Number
icValue (icUInt32Number val)
{
  return oyValueUInt32 (val);
}

my_uint64_t
icValue (icUInt64Number val)
{
  return oyValueUInt64 (val);
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
  cout << *erg << " size after conversion " << (int)korb[0] << " "
       << (int)korb[1] << " " << (int)korb[2] << " " <<(int)korb[3]
       << " "; DBG_PROG
# endif
# undef BYTES
# undef KORB
  return (signed int)*erg;
#else
  return val;
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
  cout << *erg << " size after conversion " << (int)korb[0] << " "
       << (int)korb[1] << " " << (int)korb[2] << " " <<(int)korb[3]
       << " "; DBG_PROG
# endif
# undef BYTES
# undef KORB
  return (signed int)*erg;
#else
  return val;
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
  return (typ) oyValueUInt32 (i); \
}

icValue_to_icUInt32Number(icPlatformSignature)
icValue_to_icUInt32Number(icProfileClassSignature)
icValue_to_icUInt32Number(icTagSignature)
icValue_to_icUInt32Number(icTagTypeSignature)
icValue_to_icUInt32Number(icTechnologySignature)
icValue_to_icUInt32Number(icStandardObserver)
icValue_to_icUInt32Number(icMeasurementGeometry)
icValue_to_icUInt32Number(icMeasurementFlare)
icValue_to_icUInt32Number(icIlluminant)

void
icValueXYZ (icXYZNumber* ic_xyz,double X, double Y, double Z)
{
  ic_xyz->X = icValueSF(X);
  ic_xyz->Y = icValueSF(Y);
  ic_xyz->Z = icValueSF(Z);
}

// Farbkonvertierungen

void
xyYto_XYZ (ICClist<double> & Y)
{
  if( (Y.size()%3) || Y.size() == 0 ) {
    WARN_S( "xyY is not a multiply of 3" )
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
XYZto_xyY (ICClist<double> & Y)
{
  if( (Y.size()%3) || Y.size() == 0 ) {
    WARN_S( "XYZ is not a multiply of 3" )
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
      s << _("Perceptual");
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
  return oyICCColourSpaceGetChannelCount( color );
}

icColorSpaceSignature getColorSpaceGeneric( int channels )
{
  icColorSpaceSignature cs = (icColorSpaceSignature)0;
  switch (channels) {
    case 2: return icSig2colorData; break;
    case 3: return icSig3colorData; break;
    case 4: return icSig4colorData; break;
    case 5: return icSig5colorData; break;
    case 6: return icSig6colorData; break;
    case 7: return icSig7colorData; break;
    case 8: return icSig8colorData; break;
    case 9: return icSig9colorData; break;
    case 10: return icSig10colorData; break;
    case 11: return icSig11colorData; break;
    case 12: return icSig12colorData; break;
    case 13: return icSig13colorData; break;
    case 14: return icSig14colorData; break;
    case 15: return icSig15colorData; break;
    default: break;
  }
  return cs;
}


std::string
getColorSpaceName (icColorSpaceSignature sig)
{
  std::string text;
  text = oyICCColourSpaceGetName( sig );
  return text;
}

ICClist<std::string>
getChannelNamesShort (icColorSpaceSignature color)
{
  ICClist<std::string> texte;
  std::stringstream s;
# define nFARBEN(n) for (int i = 1; i <= n; i++) \
                       { s << i; \
                         texte.push_back (s.str()); s.str(""); \
                       }

  switch ((uint32_t)color) {
    case icSigXYZData: texte.push_back ("X");
                       texte.push_back ("Y");
                       texte.push_back ("Z"); break;
    case icSigLabData: texte.push_back ("*L");
                       texte.push_back ("*a");
                       texte.push_back ("*b"); break;
    case icSigLuvData: texte.push_back ("*L");
                       texte.push_back ("*u");
                       texte.push_back ("*v"); break;
    case icSigYCbCrData: texte.push_back ("Y");
                       texte.push_back ("b");
                       texte.push_back ("r"); break;
    case icSigYxyData: texte.push_back ("Y");
                       texte.push_back ("x");
                       texte.push_back ("y"); break;
    case icSigRgbData: texte.push_back ("R");
                       texte.push_back ("G");
                       texte.push_back ("B"); break;
    case icSigGrayData: texte.push_back ("K"); break;
    case icSigHsvData: texte.push_back ("H");
                       texte.push_back ("S");
                       texte.push_back ("V"); break;
    case icSigHlsData: texte.push_back ("H");
                       texte.push_back ("L");
                       texte.push_back ("S"); break;
    case icSigCmykData: texte.push_back ("C");
                       texte.push_back ("M");
                       texte.push_back ("Y");
                       texte.push_back ("K"); break;
    case icSigCmyData: texte.push_back ("C");
                       texte.push_back ("M");
                       texte.push_back ("Y"); break;
//    case icSigMCH2Data:
    case icSig2colorData: nFARBEN(2) break;
//    case icSigMCH3Data:
    case icSig3colorData: nFARBEN(3) break;
//    case icSigMCH4Data:
    case icSig4colorData: nFARBEN(4) break;
    case icSigMCH5Data:
    case icSig5colorData: nFARBEN(5) break;
    case icSigMCH6Data:
    case icSig6colorData: nFARBEN(6) break;
    case icSigMCH7Data:
    case icSig7colorData: nFARBEN(7) break;
    case icSigMCH8Data:
    case icSig8colorData: nFARBEN(8) break;
    case icSigMCH9Data:
    case icSig9colorData: nFARBEN(9) break;
    case icSigMCHAData:
    case icSig10colorData: nFARBEN(10) break;
    case icSigMCHBData:
    case icSig11colorData: nFARBEN(11) break;
    case icSigMCHCData:
    case icSig12colorData: nFARBEN(12) break;
    case icSigMCHDData:
    case icSig13colorData: nFARBEN(13) break;
    case icSigMCHEData:
    case icSig14colorData: nFARBEN(14) break;
    case icSigMCHFData:
    case icSig15colorData: nFARBEN(15) break;
    default: texte.push_back ("-"); break;
  }
# undef nFARBEN
  return texte;
}

ICClist<std::string>
getChannelNames (icColorSpaceSignature sig)
{
  ICClist<std::string> texte;
  std::stringstream s;
# define nFARBEN(n) for (int i = 0; i < n; i++) \
                       { s << i << ". " << _("Colour"); \
                         texte.push_back (s.str()); s.str(""); \
                       }

  switch (sig) {
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
# undef nFARBEN
  return texte;
}


std::string
getDeviceClassName (icProfileClassSignature sig)
{
  std::string text = oyICCDeviceClassDescription( sig );
#if 0
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
#endif
  return text;
}

std::string
getPlatformName (icPlatformSignature platform)
{
  std::string text = oyICCPlatformDescription( platform );

#if 0
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
#endif
  return text;
}

std::string
getSigTagName               ( icTagSignature  sig )
{
  std::string text = oyICCTagName( sig );

#if 0
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
    case 0x62303135: text = "b015"; break; // binuscan targ data
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
#endif
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
  std::string text = oyICCTagDescription( sig );

#if 0
  std::string text = _("Description");

  switch (sig) {
    case icSigAToB0Tag: text = _("Lookup table, device to PCS, intent photometric"); break;
    case icSigAToB1Tag: text = _("Lookup table, device to PCS, intent relative colorimetric"); break;
    case icSigAToB2Tag: text = _("Lookup table, device to PCS, intent saturation"); break;
    case icSigBlueColorantTag: text = _("Blue Colorant"); break;
    case icSigBlueTRCTag: text = _("Blue tone reproduction curve"); break;
    case icSigBToA0Tag: text = _("Lookup table, PCS to device, intent photometric"); break;
    case icSigBToA1Tag: text = _("Lookup table, PCS to device, intent relative colorimetric"); break;
    case icSigBToA2Tag: text = _("Lookup table, PCS to device, intent saturation"); break;
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
#endif
  return text;
}

std::string
getSigTypeName               ( icTagTypeSignature  sig )
{
  std::string text = oyICCTagTypeName( sig );
#if 0
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
#endif
  return text;
}

std::string
getSigTechnology             ( icTechnologySignature sig )
{
  std::string text = oyICCTechnologyDescription( sig );
#if 0
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
#endif
  return text;
}

std::string
getChromaticityColorantType( int sig )
{
  std::string text = oyICCChromaticityColorantDescription( sig );
#if 0
  std::string text;
  switch (type) {
    case 0: text = ""; break;
    case 1: text = _("ITU-R BT.709"); break;
    case 2: text = _("SMPTE RP145-1994"); break;
    case 3: text = _("EBU Tech.3213-E"); break;
    case 4: text = _("P22"); break;

    default: DBG text = _("???"); break;
  }
#endif
  return text;
}

std::string
getIlluminant             ( icIlluminant sig )
{
  std::string text = oyICCIlluminantDescription( sig );
#if 0
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
#endif
  return text;
}

std::string
getStandardObserver             ( icStandardObserver sig )
{
  std::string text = oyICCStandardObserverDescription( sig );
#if 0
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
#endif
  return text;
}

std::string
getMeasurementGeometry             ( icMeasurementGeometry sig )
{
  std::string text = oyICCMeasurementGeometryDescription( sig );
#if 0
  std::string text;
  switch (sig) {
    case icGeometryUnknown: text = _("unknown"); break;
    case icGeometry045or450: text = _("0/45, 45/0"); break;
    case icGeometry0dord0: text = _("0/d or d/0"); break;
    case icMaxGeometry: text = _("---"); break;

    default: text = _("???"); break;
  }
#endif
  return text;
}

std::string
getMeasurementFlare             ( icMeasurementFlare sig )
{
  std::string text = oyICCMeasurementFlareDescription( sig );
#if 0
  std::string text;
  switch (sig) {
    case icFlare0: text = _("0"); break;
    case icFlare100: text = _("100"); break;
    case icMaxFlare: text = _("---"); break;

    default: text = _("???"); break;
  }
#endif
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


const char* cp_nchar (char* text, int n)
{ DBG_MEM_START
  static char string[1024];

/*  for (int i = 0; i < 1024 ; i++)
    string[i] = '\000';*/
  memset( string, 0, 1024 );

  if (n < 1024)
  {
#   if 0
    memcpy (string, text, n);
#   else
    memcpy(&string[0], text, n);
#   endif
    string[n] = '\000';
  }

# ifdef DEBUG
  DBG_MEM_V( n << " letters copy " <<  (intptr_t)text << " " << string)
# endif
  DBG_MEM_ENDE
  return string;
}

void
oyStrAdd( std::string & text, const char * add )
{
  char * ptr = (char*)malloc( strlen(text.c_str()) + strlen(add) + 64 );
  sprintf( ptr, "%s%s", text.c_str(), add );
  text = ptr;
  free( ptr );
}


namespace icc_examin_ns {

#          if defined(__GNUC__) || defined(LINUX) || defined(APPLE) || defined(SOLARIS)
# include <sys/time.h>
# define   ZEIT_TEILER 10000
#          else // WINDOWS TODO
# define   ZEIT_TEILER CLOCKS_PER_SEC;
#          endif

#ifndef WIN32
# include <unistd.h>
#endif

#include <time.h>
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
#          if defined(__GCC__) || defined(__APPLE__)
             timespec ts;
             double ganz;
             double rest = modf(sekunden, &ganz);
             ts.tv_sec = (time_t)ganz;
             ts.tv_nsec = (time_t)(rest * 1000000000);
             //DBG_PROG_V( sekunden<<" "<<ts.tv_sec<<" "<<ganz<<" "<<rest )
             nanosleep(&ts, 0);
#          else
#            if defined( WIN32 ) 
               Sleep((DWORD)(sekunden*(double)CLOCKS_PER_SEC));
#            else
               usleep((time_t)(sekunden*(double)CLOCKS_PER_SEC));
#            endif
#          endif
  }
  void wait(double sekunden, int aktualisieren)
  {
    icc_examin_ns::sleep(sekunden);
    if(aktualisieren && waitFunc)
      waitFunc();
  }
}

std::string  zeig_bits_bin           ( const void        * mem,
                                       int                 size,
                                       int                 type)
{
  std::string text;
  int byte_zahl;
  char txt[12];

  //@todo TODO: ->hexadezimal
  switch(type)
  {
  case oyFORMAT_BIN:
    for (int k = 0; k < size; k++)
    {
      for (int i = 8-1; i >= 0; i--)
      {
              unsigned char* u8 = (unsigned char*)mem;
              byte_zahl = (u8[k] >> i) & 1;
              sprintf (&txt[7-i], "%d", byte_zahl);
      }
      text.append( txt, strlen (txt));
      text.append( " ", 1); /* aller 8 bit ein leerzeichen */
    }
       break;
  case oyFORMAT_HEX:
       for (int k = 0; k < size; k++)
       {
         unsigned char* u8 = (unsigned char*)mem;
         sprintf (txt, "%x", u8[k]);
         if((2-strlen(txt)) > 0)
           text.append( "0", 2-strlen(txt));
         text.append( txt, strlen (txt));
         text.append( " ", 1);
       }
       break;
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
    case ENAMETOOLONG: WARN_S("ENAMETOOLONG = " << r); break;
    case ENOENT:       WARN_S("ENOENT = " << r); break;
    case ENOTDIR:      WARN_S("ENOTDIR = " << r); break;
#if !defined( WIN32 )
    case ELOOP:        WARN_S("ELOOP = " << r); break;
    case EOVERFLOW:    WARN_S("EOVERFLOW = " << r); break;
#endif
  }

  r = !r &&
       (   ((status.st_mode & S_IFMT) & S_IFREG)
#if !defined( WIN32 )
        || ((status.st_mode & S_IFMT) & S_IFLNK)
#endif
       );

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


/** @func    guessFileType
 *  @internal
 *  @brief   analyse a given filename for defacing compression
 *
 *  @version ICC Examin: 0.45
 *  @date    2008/02/27
 *  @since   2008/02/27 (ICC Examin: 0.45)
 */
ICCprofile::ICCDataType guessFileType( const char        * filename )
{
  ICCprofile::ICCDataType type = ICCprofile::ICCnullDATA;
  static char file_name_[1024];
  char suffix[8] = {0,0,0,0,0,0,0,0};;
  char * pos = 0;
  int i = 0;

  if(filename && strlen(filename) > 0 && strlen(filename) < 1023)
    sprintf( file_name_, "%s", filename );
  else
    return type;

  if( filename && strlen(filename) &&
      (memcmp(&filename[strlen(filename)-3],".gz", 3 )==0 ||
       memcmp(&filename[strlen(filename)-4],".bz2", 4 )==0 ) )
  {
    pos = strrchr( file_name_, '.' );

    if(pos)
      *pos = 0;
  }

  pos = strrchr( file_name_, '.' );
  if(pos && strlen(pos) < 8)
    sprintf(suffix, "%s", pos + 1);

  if(strlen(suffix))
    for(i = 0; i < (int)strlen(suffix); ++i)
      suffix[i] = (char) tolower( suffix[i] );

  if(strcmp(suffix, "icc")==0 ||
     strcmp(suffix, "icm")==0 )
    type = ICCprofile::ICCprofileDATA; 
  else
  if(strcmp(suffix, "wrl")==0 ||
     strcmp(suffix, "vrml")==0 )
    type = ICCprofile::ICCvrmlDATA; 
  else
  if(strcmp(suffix, "cgats")==0 ||
     strcmp(suffix, "cie")==0 ||
     strcmp(suffix, "it8")==0 ||
     strcmp(suffix, "q60")==0 ||
     strcmp(suffix, "rgb")==0 ||
     strcmp(suffix, "cmyk")==0 ||
     strcmp(suffix, "lab")==0 ||
     strcmp(suffix, "xyz")==0 ||
     strcmp(suffix, "ti1")==0 ||
     strcmp(suffix, "ti2")==0 ||
     strcmp(suffix, "ti3")==0 ||
     strcmp(suffix, "dly")==0 ||
     strcmp(suffix, "ncie")==0 ||
     strcmp(suffix, "orpt")==0 ||
     strcmp(suffix, "meas")==0 ||
     strcmp(suffix, "txt")==0 )
    type = ICCprofile::ICCmeasurementDATA; 

  return type;
}

/** @func    preLoadFile
 *  @internal
 *  @brief   virtualise a given filename for web content and compression
 *
 *  @version ICC Examin: 0.45
 *  @date    2008/02/26
 *  @since   2008/02/26 (ICC Examin: 0.45)
 */
const char * preLoadFile             ( const char        * filename )
{
  const char *list[12] = {".www",".gunzip",
                    "icc_examin_preload2.bunzip2",0,0,0,0,0,0,0,0,0};
  const char * commands[12] = { 0 , "gzip -dc", "bzip2 -dc", 0 };
  const char * name = filename;
  int error = 0;
  char * command = 0;
  const char * tmp = NULL;
  char * tmp_name = 0;
  char* ptr = (char*) malloc(1024);
  char * user = 0;
  static char file_name_[1024];
  int typ = -1; 

  if(getenv("USER"))
    user=strdup(getenv("USER"));
  else if(getenv("UID"))
    user=strdup(getenv("UID"));
  else
    user=strdup("user");
 
  if(filename && strlen(filename) > 0 && strlen(filename) < 1023)
    sprintf( file_name_, "%s", filename );
  else
    goto END;

  command = (char*) malloc(strlen(file_name_) * 2 + 1024);
  if(!command)
    goto END;

  if(getenv("TMP") && strlen(getenv("TMP")))
    tmp = getenv("TMP");
  else
  if(getenv("TEMP") && strlen(getenv("TEMP")))
    tmp = getenv("TEMP");
  else
  if(getenv("TMPDIR") && strlen(getenv("TMPDIR")))
    tmp = getenv("TMPDIR");

  if(!tmp)
  {
#if !defined(WIN32)
    FILE *out = icc_popen_m("mktemp", "r");
    if(out)
    {  
      char name[64] = {0};
      size_t r = fscanf( out, "%60s", name ); r=r;
      if( name[0] != 0 )
      {
        tmp_name = strdup( name );
        tmp = tmp_name;
        remove(tmp);
        mkdir(tmp, 0777);
      }
      icc_pclose_m(out);
    } 
#endif
  }

  // web content
  if( filename && strlen(filename) &&
      (memcmp(filename,"http://", 7 )==0 || memcmp(filename,"ftp://", 6 )==0 ||
       memcmp(filename,"https://", 8 )==0 ) )
  {
    typ = 0;

#if !defined(__APPLE__)
  sprintf( ptr, "which wget && wget -U \"ICC Examin %s %s %s\" -O || curl -o", ICC_EXAMIN_V, UNAME, DISTNAME);
#else
  sprintf( ptr, "curl -o" );
#endif

    sprintf(command,
            "which wget && wget -U \"ICC Examin %s %s %s\" -O %s/icc_examin_preload-%s%s \"%s\" "
            "|| curl -L \"%s\" -o %s/icc_examin_preload-%s%s",
            ICC_EXAMIN_V, UNAME, DISTNAME, tmp, user, list[typ], filename,
            filename, tmp, user, list[typ] );

    fprintf(stderr, "ICC Examin %s: %s\n", ICC_EXAMIN_V, command);
    error = system(command);

    if(!error)
    {
      sprintf( file_name_, "%s/icc_examin_preload-%s%s", tmp, user, list[typ] );
      name = file_name_;
    }
  }

  // compressed content
  if( filename && strlen(filename) &&
      (memcmp(&filename[strlen(filename)-3],".gz", 3 )==0 ||
       memcmp(&filename[strlen(filename)-4],".bz2", 4 )==0 ) )
  {
    if(memcmp(&filename[strlen(filename)-3],".gz", 3)==0)
      typ = 1;
    else if(memcmp(&filename[strlen(filename)-4],".bz2", 4 )==0)
      typ = 2;
    
    sprintf(command,"%s \"%s\" > %s/icc_examin_preload-%s%s", 
            commands[typ], file_name_,  tmp, user, list[typ]);

    fprintf(stderr, "ICC Examin %s: %s\n", ICC_EXAMIN_V, command);
    error = system(command);

    if(!error)
    {
      sprintf( file_name_, "%s/icc_examin_preload-%s%s", tmp, user, list[typ] );
      name = file_name_;
    }
  }

  free(command); command = 0;

  END:
  if(ptr) free(ptr);
  if(tmp_name) free(tmp_name);
  if(user) free(user);

  return name;
}

char*
ladeDatei ( std::string dateiname, size_t *size )
{ DBG_PROG_START

    char* data = 0;
    *size = 0;

    dateiname = preLoadFile(dateiname.c_str());

    std::ifstream f ( dateiname.c_str(), std::ios::binary | std::ios::ate );

    DBG_PROG_V( dateiname )
    if (dateiname == "" || !isFileFull(dateiname.c_str()) || !f)
    { DBG_MEM
      if(dateiname != _("Monitor Profile")) 
      {
        if(dateiname == "") WARN_S( "no file name" );
        if(!isFileFull(dateiname.c_str())) WARN_S( "no file: " << dateiname );
        if(!f) WARN_S( "erroneous file: " << dateiname );
#       if HAVE_EXCEPTION
        DBG_PROG_ENDE
        throw ausn_file_io (dateiname.c_str());
#       endif
        dateiname = "";
      }
#     if !HAVE_EXCEPTION
      goto myERROR;
#     endif
    }
    DBG_MEM

    *size = (unsigned int)f.tellg();
    DBG_MEM_V ( *size << "|" << f.tellg() )
    f.seekg(0);
    if(*size) {
      data = (char*)calloc (sizeof (char), *size+1);
	  f.read ((char*)data, (std::streamsize)*size);
      DBG_MEM_V ( *size << "|" << f.tellg() <<" "<< (int*)data <<" "<< strlen(data) )
      f.close();
    } else {
      data = 0;
      WARN_S( _("file size 0 for ") << dateiname )
    }

  myERROR:

  DBG_PROG_ENDE
  return data;
}

void
saveMemToFile (const char* filename, const char *block, size_t size)
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
  s.zeit (holeDateiModifikationsZeit( dateiname.c_str() ));
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
  s.zeit (holeDateiModifikationsZeit( dateiname.c_str() ));
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
#if !defined( WIN32 )
        || ((status.st_mode & S_IFMT) & S_IFLNK)
#endif
       );

  double m_zeit = 0.0;
  if (r)
  {
#   if defined(APPLE) || defined(BSD)
    m_zeit = status.st_mtime ;
    m_zeit += status.st_mtimespec.tv_nsec/1000000. ;
#   elif defined(WIN32) 
    m_zeit = (double)status.st_mtime ;
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

std::string
tempFileName ()
{
  std::string profil_temp_name, text;
  const char * tmp_path = getenv("TMPDIR"); // WIN32
  if(!tmp_path)
    tmp_path = getenv("TMP"); //*nix
  if(!tmp_path)
    tmp_path = getenv("TEMP"); //*nix

  if(tmp_path)
  {
    profil_temp_name += tmp_path;
    profil_temp_name += ICC_DIR_SEPARATOR;
    profil_temp_name += "oyranos_";
    text = profil_temp_name;
    char * ptr = (char*)malloc( strlen(text.c_str()) + 64 );
    sprintf( ptr, "%s%ld", text.c_str(), time(0) );
    text = ptr;
    free( ptr );
    profil_temp_name = text;
  } else {
    profil_temp_name += ICC_DIR_SEPARATOR;
    profil_temp_name += "tmp";
    profil_temp_name += ICC_DIR_SEPARATOR;
    profil_temp_name += "oyranos_";
    text = profil_temp_name;
    char * ptr = (char*)malloc( strlen(text.c_str()) + 64 );
    sprintf( ptr, "%s%ld", text.c_str(), time(0) );
    text = ptr;
    free( ptr );
    profil_temp_name = text;
  }

  return profil_temp_name;
}

char*
getExecPath(const char *filename)
{
  DBG_PROG_START
  char *exec_path = NULL;

  if (filename)
  {
    int len = (int)strlen(filename) * 2 + 1024;
    char *text = (char*) calloc( sizeof(char), len );
    text[0] = 0;
    /* whats the path for the executeable ? */
    snprintf (text, len-1, "%s", filename);

    if(text[0] == '~')
    {
      /* home directory */
      if(getenv("HOME"))
        sprintf( text, "%s%s", getenv("HOME"), &filename[0]+1 );
    }

    /* relative names - where the first sign is no directory separator */
#if defined( WIN32 )
    if (text[1] != ':')
#else // unix
    if (text[0] != ICC_DIR_SEPARATOR_C)
#endif
    {
      FILE *pp = NULL;
  
      if (text) free (text);
      text = (char*) malloc( 1024 );

      /* Suche das ausfuehrbare Programm
         TODO symbolische Verknuepfungen */
      snprintf( text, 1024, "which %s", filename);
      pp = icc_popen_m( text, "r" );
      if (pp) {
        if (fscanf (pp, "%s", text) != 1)
        {
          icc_pclose_m (pp);
          WARN_S( "no executeable path found" );
        }
      } else { 
        WARN_S( "could not ask for executeable path" );
      }
    
#if defined( WIN32 )
      if (text[1] != ':')
#else // unix
      if (text[0] != ICC_DIR_SEPARATOR_C)
#endif
      {
        char* cn = (char*) calloc(2048, sizeof(char));
        sprintf (cn, "%s%s%s", getenv("PWD"), ICC_DIR_SEPARATOR, filename);
        sprintf (text, "%s", cn);
        if(cn) free(cn); 
      }
    }

    { /* remove the executable name */
      char *tmp = strrchr(text, '/');
      char *tmp2 = strrchr(text, '\\');
      if(tmp > tmp2)
        *tmp = 0;
      if(tmp < tmp2)
        *tmp2 = 0;
    }
    while (text[strlen(text)-1] == '.')
      text[strlen(text)-1] = 0;
    while (text[strlen(text)-1] == ICC_DIR_SEPARATOR_C)
      text[strlen(text)-1] = 0;

    exec_path = text;
  }
  DBG_PROG_ENDE
  return exec_path;
}

extern "C" {
char ** oyStringSplit_(const char * name, const char delimiter, int * n, oyAlloc_f alloc);
void oyStringListRelease_( char***, int, oyDeAlloc_f );
} /* extern "C" */
/* resembles which */
char * findApplication(const char * app_name)
{
  const char * path = getenv("PATH");
  char * full_app_name = NULL;
  if(path && app_name)
  {
    int paths_n = 0, i;
    char ** paths = oyStringSplit_( path, ':', &paths_n, malloc );
    for(i = 0; i < paths_n; ++i)
    {
      std::string full_name = paths[i];
      full_name += OY_SLASH;
      full_name += app_name;
      int found = isFileFull( full_name.c_str() );
      if(found)
      {
        i = paths_n;
        full_app_name = strdup( full_name.c_str() );
      }
    }
    oyStringListRelease_( &paths, paths_n, free );
  }
  return full_app_name;
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
      locale_paths[num_paths] = bdr.c_str(); ++num_paths;
    }
  }
  locale_paths[num_paths] = ICCEXAMIN_LOCALEDIR; ++num_paths;
  locale_paths[num_paths] = SRC_LOCALEDIR; ++num_paths;
# else
  locale_paths[num_paths] = ICCEXAMIN_LOCALEDIR; ++num_paths;

  DBG_NUM_V( exename )

  {
    char *reloc_path = (char*) malloc(128);
    sprintf( reloc_path,"..%cshare%clocale", ICC_DIR_SEPARATOR_C, ICC_DIR_SEPARATOR_C);
    int len = (int)(strlen(exename) + strlen(reloc_path)) * 2 + 128;
    char *path = (char*) malloc( len ); // small one time leak
    char *text = NULL;

    text = getExecPath( exename );
    snprintf (path, len-1, "%s%s%s", text, ICC_DIR_SEPARATOR, reloc_path);
    locale_paths[num_paths] = path; ++num_paths;
    locale_paths[num_paths] = SRC_LOCALEDIR; ++num_paths;
    DBG_NUM_V( path );
    if (text) free (text);
    if (reloc_path) free (reloc_path);
  }
# endif
  const char* tdd = getenv("TEXTDOMAINDIR");
  if(tdd)
  {
    locale_paths[num_paths] = tdd;
    DBG_NUM_V( num_paths <<" "<< tdd );
    ++num_paths;
  }

  const char *dname = strrchr(exename, ICC_DIR_SEPARATOR_C);
  if(dname && strlen(dname))
    dname++;
  if(!dname)
    dname = exename;
  is_path = fl_search_locale_path (num_paths, locale_paths, "de", dname);
#if defined(_Xutf8_h) || HAVE_FLTK_UTF8
  FL_I18N_SETCODESET set_charset = FL_I18N_SETCODESET_UTF8;
#else
  FL_I18N_SETCODESET set_charset = FL_I18N_SETCODESET_SELECT;
#endif
  if(is_path >= 0) {
    int err = fl_initialise_locale ( dname, locale_paths[is_path],
                                     set_charset );
    if(err) {
      WARN_S("i18n initialisation failed");
    } else {
      DBG_NUM_S( "locale found in: " << locale_paths[is_path] );
    }
  } else {
    if(is_path < 0)
    {
      if(exename && strlen(exename))
      {
        dname = "icc_examin";
        is_path = fl_search_locale_path (num_paths, locale_paths, "de", dname );
      }
      if(is_path >= 0) {
        int err = fl_initialise_locale ( dname,
                                         locale_paths[is_path], set_charset );
        if(!err) {
          DBG_NUM_S( "locale found in: " << locale_paths[is_path] );
        } else {
          WARN_S( "No locale initialised from: " << locale_paths[is_path] );
        }
      }
    }
  }
  if(fl_i18n_codeset)
    oy_domain_codeset = fl_i18n_codeset;
  else
    oy_domain_codeset = "UTF-8";
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

ICClist<std::string>
zeilenNachVector(std::string &text)
{ DBG_PROG_START
  // fueilen aus einen Text in einen Vector
  ICClist <std::string> texte;

      int len = (int)strlen(text.c_str());
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


ICClist<ZifferWort>
unterscheideZiffernWorte ( std::string &zeile,
                           bool         anfuehrungsstriche_setzen,
                           const char  *trennzeichen )
{ DBG_PARSER_START
  std::string::size_type pos = 0, ende = 0, pos2, pos3;
  static char text[64];
  bool in_anfuehrung = false;
  std::string txt;
  ICClist<ZifferWort> ergebnis;
  ICClist<std::string> worte;
  char trenner [16];

  if( trennzeichen ) { DBG_PARSER
    sprintf (trenner, "%s",trennzeichen );
  } else {
    sprintf (trenner, "%s", leer_zeichen );
    suchenErsetzen( zeile, ",", ".", 0 );
  }

  // search words and detect numbers
  for( pos = 0; pos < zeile.size() ; ++pos )
  { DBG_PARSER_V( pos <<" "<< zeile.size() )
    in_anfuehrung = false;
    pos2 = pos;

    // search first sign
    if( (pos = zeile.find_first_of( alnum, pos )) != std::string::npos )
    {
      bool anf_zaehlen = true;
      // search first quotation sign
      if( zeile.find_first_of( "\"", pos2 ) != std::string::npos &&
          zeile.find_first_of( "\"", pos2 ) < pos )
        pos2 = zeile.find_first_of( "\"", pos2 );
      else
        anf_zaehlen = false;
      DBG_PARSER_V( pos2 )

      // count quotation sign [ ""  " "  ABC ] - pice wise
      int letzes_anf_zeichen = -1;
      if( anf_zaehlen )
        for( pos3 = pos2; pos3 < pos; ++pos3)
          if( zeile[pos3] == '"' &&
              letzes_anf_zeichen >= 0 )
            letzes_anf_zeichen = -1;
          else
            letzes_anf_zeichen = (int)pos3;

      // in case a quotation mark in front of a word is odd // ["" " ABC ]
      if( letzes_anf_zeichen >= 0 )
      {
        in_anfuehrung = true;
        pos2 = pos;
        pos = letzes_anf_zeichen+1;
      }
      DBG_PARSER_V( pos )
      // find the end of the word
      if( in_anfuehrung )
        ende = zeile.find_first_of( "\"", pos + 1 );
      else
        if( (ende = zeile.find_first_of( trenner, pos2 + 1 ))
            == std::string::npos )
          ende = zeile.size();

      // include the first word
      if( zeile.find_first_of( alnum, pos ) > ende )
        ende = zeile.find_first_of( trenner,
                                zeile.find_first_of( alnum, pos ) + 1 );
      // extract the word
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
      // inspect the word
      if( txt.find_first_of( numerisch ) != std::string::npos &&
          txt.find( "." ) != std::string::npos &&
          txt.find_first_of( alpha ) == std::string::npos )
      { // is floating point number
        pos += txt.size();
        sprintf( text, "%f", atof( zeile.substr( pos, ende-pos ).c_str() ) );
        DBG_PARSER_S( "floating point number: " << txt )
        ergebnis.push_back( zifferWort((double)atof(txt.c_str())) );
        DBG_PARSER_S( "check: "<< ergebnis[ergebnis.size()-1].zahl.first <<" "<< ergebnis[ergebnis.size()-1].zahl.second )
      } else
      if( txt.find_first_of( ziffer ) != std::string::npos &&
          txt.find( "." ) == std::string::npos &&
          txt.find_first_of( alpha ) == std::string::npos )
      { // is whole number
        pos += txt.size();
        sprintf( text, "%d", atoi( zeile.substr( pos, ende-pos ).c_str() ) );
        ergebnis.push_back( zifferWort(atoi(txt.c_str())) );
        DBG_PARSER_S( "whole number: " << txt )
      } else
      if( txt.find_first_of( alnum ) != std::string::npos )
      { // is text
        bool neusetzen = false;
        int anf = 0;
        ergebnis.push_back( zifferWort(txt) );
        DBG_PARSER_S( "text: " << txt )
        DBG_PARSER_V( in_anfuehrung )
        // quotation sign start
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
        // end of word
        if( (pos2 = zeile.find_first_of( trenner, pos ))
            == std::string::npos)
          pos2 = zeile.size();
        // a quotation sign follows
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
      // end the loop
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


/**
  *  general functions
  */

void
lcms_error (int ErrorCode, const char* ErrorText)
{ DBG_PROG_START
   g_message ("LCMS error:%d %s", ErrorCode, ErrorText);
  DBG_PROG_ENDE
}

void* myAllocFunc(size_t size)
{
  return new char [size];
} 
  
void myDeAllocFunc(char * buf)
{   
  if(buf)
    delete [] buf;
} 

