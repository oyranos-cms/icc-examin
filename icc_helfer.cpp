/*
 * ICC Examin ist eine ICC Profil Betrachter
 * 
 * Copyright (C) 2004  Kai-Uwe Behrmann 
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
 * Aufbereitung von ICC internen Informationen zu Text.
 * 
 */

// Date:      04. 05. 2004

#if 0
  #ifndef DEBUG
   #define DEBUG
  #endif
  #define DEBUG_ICCFUNKT
#endif

//#include "icc_profile.h"
#include <icc34.h>
//#include <lcms.h>
#include "icc_utils.h"
#include "icc_formeln.h"
#include "icc_helfer.h"

#define _(text) text
#define g_message printf

icUInt16Number
icValue (icUInt16Number val)
{
#if BYTE_ORDER == LITTLE_ENDIAN
  #define BYTES 2
  #define KORB  4
  unsigned char        *temp  = (unsigned char*) &val;
  static unsigned char  korb[KORB];
  for (int i = 0; i < KORB ; i++ )
    korb[i] = (int) 0;  // leeren

  int klein = 0,
      gross = BYTES - 1;
  for (; klein < BYTES ; klein++ ) {
    korb[klein] = temp[gross--];
    #ifdef DEBUG_ICCFUNKT
    cout << klein << " "; DBG
    #endif
  }

  unsigned int *erg = (unsigned int*) &korb[0];

  #ifdef DEBUG_ICCFUNKT
  #if 0
  cout << *erg << " Größe nach Wandlung " << (int)korb[0] << " "
       << (int)korb[1] << " " << (int)korb[2] << " " <<(int)korb[3]
       << " "; DBG
  #else
  cout << *erg << " Größe nach Wandlung " << (int)temp[0] << " " << (int)temp[1]
       << " "; DBG
  #endif
  #endif
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

icInt32Number
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

icInt16Number
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

// Namen

std::string
renderingIntentName (int intent)
{
  std::stringstream s;

  switch (intent) 
    {
    case 0:
      s << _("Fotografisch");
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
    default: icUInt32Number i = icValue(color); text = cp_nchar ((char*)&i, 5); text += "?"; break;
  }
  return text;
}

std::vector<std::string>
getChannelNames (icColorSpaceSignature color)
{
  std::vector<std::string> texte;
  std::stringstream s;
  #define nFARBEN(n) for (int i = 0; i < n; i++) \
                       { s << i << ". " << _("Farbe"); \
                         texte.push_back (s.str()); \
                       }

  switch (color) {
    case icSigXYZData: texte.push_back (_("CIE X"));
                       texte.push_back (_("CIE Y (Leuchtdichte)"));
                       texte.push_back (_("CIE Z")); break;
    case icSigLabData: texte.push_back (_("CIE *L"));
                       texte.push_back (_("CIE *a"));
                       texte.push_back (_("CIE *b")); break;
    case icSigLuvData: texte.push_back (_("CIE *L"));
                       texte.push_back (_("CIE *u"));
                       texte.push_back (_("CIE *v")); break;
    case icSigYCbCrData: texte.push_back (_("Leuchtdichte Y"));
                       texte.push_back (_("Farbanteil b"));
                       texte.push_back (_("Farbanteil r")); break;
    case icSigYxyData: texte.push_back (_("CIE Y (Leuchtdichte)"));
                       texte.push_back (_("CIE x"));
                       texte.push_back (_("CIE y")); break;
    case icSigRgbData: texte.push_back (_("Rot"));
                       texte.push_back (_("Grün"));
                       texte.push_back (_("Blau")); break;
    case icSigGrayData: texte.push_back (_("Schwarz")); break;
    case icSigHsvData: texte.push_back (_("Farbton"));
                       texte.push_back (_("Sättigung"));
                       texte.push_back (_("Wert")); break;
    case icSigHlsData: texte.push_back (_("Farbton"));
                       texte.push_back (_("Helligkeit"));
                       texte.push_back (_("Sättigung")); break;
    case icSigCmykData: texte.push_back (_("Cyan"));
                       texte.push_back (_("Magenta"));
                       texte.push_back (_("Gelb"));
                       texte.push_back (_("Schwarz")); break;
    case icSigCmyData: texte.push_back (_("Cyan"));
                       texte.push_back (_("Magenta"));
                       texte.push_back (_("Gelb")); break;
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
    default: texte.push_back (_("keine Farbe")); break;
  }
  return texte;
}

std::string
getDeviceClassName (icProfileClassSignature deviceClass)
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
    default: icUInt32Number i = icValue(deviceClass); text = cp_nchar ((char*)&i, 5); text += "?"; break;
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
    default: icUInt32Number i = icValue(platform); text = cp_nchar ((char*)&i, 5); text += "?"; break;
  }
  return text;
}

std::string
getSigTagName               ( icTagSignature  sig )
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
    case icSigChromaticityType: text = _("chrm"); break;
    case 1668051567: text = _("clro"); break;
    case 1668051572: text = _("clrt"); break;
    case 0: text = _("----"); break;
    default: icUInt32Number i = icValue(sig); text = cp_nchar ((char*)&i, 5); text += "?"; break;
  }
  #ifdef DEBUG_ICCTAG_
  char c[5] = "clrt";
  long* l = (long*) &c[0];
  cout << *l << ": " << (long)"clrt" << " "; DBG
  #endif
  return text;
}

std::string
getSigTagDescription                            ( icTagSignature  sig )
{
  std::string text = "Beschreibung";

  switch (sig) {
    case icSigAToB0Tag: text = _("Farbtabelle, Gerät an Kontaktfarbraum, Anpassung 0"); break;
    case icSigAToB1Tag: text = _("Farbtabelle, Gerät an Kontaktfarbraum, Anpassung 1"); break;
    case icSigAToB2Tag: text = _("Farbtabelle, Gerät an Kontaktfarbraum, Anpassung 2"); break;
    case icSigBlueColorantTag: text = _("blaue Grundfarbe"); break;
    case icSigBlueTRCTag: text = _("blaue Farbwiedergabekurve"); break;
    case icSigBToA0Tag: text = _("Farbtabelle, Kontaktfarbraum an Gerät, Anpassung 0"); break;
    case icSigBToA1Tag: text = _("Farbtabelle, Kontaktfarbraum an Gerät, Anpassung 1"); break;
    case icSigBToA2Tag: text = _("Farbtabelle, Kontaktfarbraum an Gerät, Anpassung 2"); break;
    case icSigCalibrationDateTimeTag: text = _("Kalibrationsdatum"); break;
    case icSigCharTargetTag: text = _("Farbmessdaten"); break;
    case icSigCopyrightTag: text = _("Kopierrecht"); break;
    case icSigCrdInfoTag: text = _("crdi"); break;
    case icSigDeviceMfgDescTag: text = _("Herstellerbeschreibung"); break;
    case icSigDeviceModelDescTag: text = _("Gerätebeschreibung"); break;
    case icSigGamutTag: text = _("Farbumfang"); break;
    case icSigGrayTRCTag: text = _("schwarze Wiedergabekurve"); break;
    case icSigGreenColorantTag: text = _("grüne Grundfarbe"); break;
    case icSigGreenTRCTag: text = _("grüne Farbwiedergabekurve"); break;
    case icSigLuminanceTag: text = _("Lichtintensität"); break;
    case icSigMeasurementTag: text = _("Messart"); break;
    case icSigMediaBlackPointTag: text = _("Medienschwarzpunkt"); break;
    case icSigMediaWhitePointTag: text = _("Medienweißpunkt"); break;
    case icSigNamedColorTag: text = _("'ncol"); break;
    case icSigNamedColor2Tag: text = _("ncl2"); break;
    case icSigPreview0Tag: text = _("Voransicht, fotografisch"); break;
    case icSigPreview1Tag: text = _("Voransicht, relativ farbmetrisch"); break;
    case icSigPreview2Tag: text = _("Voransicht, farbgesättigt"); break;
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
    case icSigTechnologyTag: text = _("Technologie"); break;
    case icSigUcrBgTag: text = _("bfd"); break;
    case icSigViewingCondDescTag: text = _("Beschreibung der Betrachtungbedingungen"); break;
    case icSigViewingConditionsTag: text = _("Betrachtungsbedingungen"); break;
    case 1147500100: text = _("Farbmessflächen"); break;//DevD
    case 1128875332: text = _("Farbmessergebnisse"); break;//CIED
    case 1349350514: text = _("Profilierungsparameter"); break;//Pmtr
    case 1986226036: text = _("GrafikKartenGammaTabelle"); break;//vcgt
    case 1667785060: text = _("Farbanpassungsmatrix"); break; //chad
    case icSigChromaticityType: text = _("Primärfarben"); break; //chrm
    case 1668051567: text = _("Schmuckfarbordnung"); break;//clro
    case 1668051572: text = _("Schmuckfarbnamen"); break;//clrt
    case 0: text = _("----"); break;
    default: icUInt32Number i = icValue(sig); text = cp_nchar ((char*)&i, 5); text += "?"; break;
  }
  return text;
}

std::string
getSigTypeName               ( icTagTypeSignature  sig )
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
    case icSigChromaticityType: text = _("chrm"); break;
    case 1986226036: text = _("vcgt"); break;
    case icSigCopyrightTag: text = _("cprt?"); break; //??? (Imacon)
    default: icUInt32Number i = icValue(sig); text = cp_nchar ((char*)&i, 5); text += "?"; break;
  }
  return text;
}

std::string
getSigTechnology             ( icTechnologySignature sig )
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
    default: icUInt32Number i = icValue(sig); text = cp_nchar ((char*)&i, 5); text += "?"; break;
  }
  return text;
}

std::string
getIlluminant             ( icIlluminant sig )
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
    case icIlluminantEquiPowerE: text = _("Illuminant mit ausgeglichener Energie E"); break;
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
    case icStdObsUnknown: text = _("unbekannt"); break;
    case icStdObs1931TwoDegrees: text = _("2 Grad (1931)");
         break;
    case icStdObs1964TenDegrees: text = _("10 Grad (1964)");
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
    case icGeometryUnknown: text = _("unbekannt"); break;
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
{ DBG
  std::stringstream s;
    s << _("Datum") << ":       " <<
                       icValue(date.day)     << "/" <<
                       icValue(date.month)   << "/" <<
                       icValue(date.year)    << " " <<
                       icValue(date.hours)   << ":";
    if (icValue(date.minutes) < 10)
      s << "0";
                  s << icValue(date.minutes) << " " << _("Uhr") << " " <<
                       icValue(date.seconds) << " " << _("Sekunden");
  return s.str();
}

std::string
zeig_bits_bin(void* speicher, int groesse)
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
              //DBG_PROG_V( 7-i <<" "<< byte_zahl )
      }
      text.append( txt, strlen (txt));
      text.append( " ", 1); /*aller 8 bit ein leerzeichen*/
      //DBG_PROG_V( k << txt )
  }

  return text;
}


