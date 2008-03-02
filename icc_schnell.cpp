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
 * Schnell kompilieren.
 * 
 */

// Date:      Mai 2005

#include <lcms.h>

#ifdef HAVE_OY
#include "oyranos/oyranos.h"
#include "oyranos/oyranos_monitor.h"
using namespace oyranos;
#endif

#include "icc_formeln.h"
#include "icc_oyranos.h"
#include "icc_profile.h"
#include "icc_utils.h"

#include "icc_helfer.h"

#define PRECALC cmsFLAGS_NOTPRECALC 
#if 0
#define BW_COMP cmsFLAGS_WHITEBLACKCOMPENSATION
#else
#define BW_COMP 0
#endif

int gamutCheckSampler(register WORD In[],
                      register WORD Out[],
                      register LPVOID Cargo)
{
  cmsCIELab Lab1, Lab2;

  cmsLabEncoded2Float(&Lab1, In);
  cmsDoTransform( Cargo, &Lab1, &Lab2, 1 );
  cmsFloat2LabEncoded(Out, &Lab2);

  return TRUE;
}

Speicher
Oyranos::gamutCheckAbstract(Speicher s, int intent, int flags)
{
  DBG_PROG_START
  cmsHPROFILE profil = 0,
              hLab = 0;
  size_t groesse = s.size();
  const char* block = s;

  DBG_MEM_V( (int*) block <<" "<<groesse )

      hLab  = cmsCreateLabProfile(cmsD50_xyY());
      if(!hLab)  WARN_S( _("hLab Profil nicht geoeffnet") )

      block = (char*) s(groesse);
      profil = cmsOpenProfileFromMem(const_cast<char*>(block), groesse);
      cmsHTRANSFORM tr1 = cmsCreateProofingTransform  (hLab, TYPE_Lab_DBL,
                                               hLab, TYPE_Lab_DBL,
                                               profil,
                                               intent,
                                               INTENT_RELATIVE_COLORIMETRIC,
                                               flags|cmsFLAGS_HIGHRESPRECALC);
     
#if 0 // Gamut tag
      LPLUT lut = _cmsPrecalculateGamutCheck( tr1 ); DBG
      cmsHPROFILE gmt = _cmsCreateProfilePlaceholder();
      cmsSetDeviceClass( gmt, icSigOutputClass );
      cmsSetColorSpace( gmt, icSigLabData );
      cmsSetPCS( gmt, icSigCmykData );
      _cmsAddLUTTag( gmt, icSigGamutTag, lut ); DBG
      cmsAddTag( gmt, icSigProfileDescriptionTag,  (char*)"GamutCheck");
      _cmsSaveProfile ( gmt,"proof_gamut.icc"); DBG
#endif

      // Wir berechnen die Farbhüllwarnung für ein abstraktes Profil
      LPLUT gmt_lut = cmsAllocLUT(),
            lut = cmsReadICCLut( cmsTransform2DeviceLink(tr1,0), icSigAToB0Tag);
      cmsAlloc3DGrid( gmt_lut, lut->cLutPoints, 3, 3);
      DBG_V( lut->cLutPoints )
      cmsSample3DGrid( gmt_lut, gamutCheckSampler, tr1, 0 );

      cmsHPROFILE gmt = _cmsCreateProfilePlaceholder();
      cmsSetDeviceClass( gmt, icSigAbstractClass );
      cmsSetColorSpace( gmt, icSigLabData );
      cmsSetPCS( gmt, icSigLabData );
      cmsAddTag( gmt, icSigProfileDescriptionTag,  (char*)"GamutCheck");
      cmsAddTag( gmt, icSigAToB0Tag, gmt_lut );
      _cmsSaveProfileToMem ( gmt, 0, groesse );
  
  DBG_PROG_ENDE
}


double*
Oyranos::wandelLabNachBildschirmFarben(double *Lab_Speicher, // 0.0 - 1.0
                                       size_t  size, int intent, int flags)
{
  DBG_PROG_START

  DBG_PROG_V( size <<" "<< intent <<" "<< flags )

    // lcms Typen
    cmsHPROFILE hsRGB = 0,
                hLab = 0;
    static cmsHTRANSFORM hLabtoRGB = 0;
    double *RGB_Speicher = 0;
    
    static int flags_ = 0;

    if(flags_ != flags && hLabtoRGB) {
      cmsDeleteTransform(hLabtoRGB);
      hLabtoRGB = 0;
      flags_ = flags;
    }

    // Initialisierung für lcms
    if(!hLabtoRGB)
    {
      size_t groesse = 0;
      char* block = 0;
      block = (char*) moni(groesse);
      DBG_MEM_V( (int*) block <<" "<<groesse )

      if(groesse)
        hsRGB = cmsOpenProfileFromMem(const_cast<char*>(block), groesse);
      else
        hsRGB = cmsCreate_sRGBProfile();
      if(!hsRGB) WARN_S( _("hsRGB Profil nicht geoeffnet") )
      hLab  = cmsCreateLabProfile(cmsD50_xyY());
      if(!hLab)  WARN_S( _("hLab Profil nicht geoeffnet") )

      hLabtoRGB = cmsCreateProofingTransform  (hLab, TYPE_Lab_DBL,
                                               hsRGB, TYPE_RGB_DBL,
                                               hsRGB,
                                               intent,
                                               INTENT_RELATIVE_COLORIMETRIC,
                                               PRECALC|BW_COMP|flags);
#if 1
      int n_profile = 2;
      cmsHPROFILE viele[n_profile], cmyk_p, temp;
      block = (char*) cmyk(groesse);
      cmyk_p = cmsOpenProfileFromMem(const_cast<char*>(block), groesse);
      cmsHTRANSFORM tr1 = cmsCreateProofingTransform  (hLab, TYPE_Lab_DBL,
                                               hLab, TYPE_Lab_DBL,
                                               cmyk_p,
                                               intent,
                                               INTENT_RELATIVE_COLORIMETRIC,
                                               cmsFLAGS_SOFTPROOFING|cmsFLAGS_GAMUTCHECK);//|cmsFLAGS_HIGHRESPRECALC);
      // device -> XYZ
      //viele[0] = cmsOpenProfileFromMem(const_cast<char*>(block), groesse);
      // Lab -> RGB
      viele[0] = cmsTransform2DeviceLink(tr1, 0);
      temp = viele[0];
      _cmsSaveProfileToMem(viele[0], 0, &groesse); DBG_V( groesse )
      block = (char*) alloca( groesse );
      _cmsSaveProfileToMem( viele[0], (char*)block, &groesse );
     
      //cmsCloseProfile( viele[0] );
      // Austausch
      //icHeader *h = (icHeader*) block;
      //memcpy( &h->deviceClass, "abst", 4);
      saveMemToFile("proof1b.icc", block, groesse);
      //viele[0] = cmsOpenProfileFromMem((char*)block, groesse);
      viele[0] = cmsOpenProfileFromFile( "proof1b.icc", "r");

      _cmsSaveProfile ( viele[0],"proof1.icc");
      cmsCloseProfile( viele[0] );
      viele[0] = cmsOpenProfileFromFile( "proof1b.icc", "r");
      DBG_V( "proof1.icc geschrieben" )
      groesse = 0; // oyGetProfileSize("lab.icc");
      block = (char*) (oyGetProfileBlock("lab.icc", &groesse));
      DBG_V( (int*)block )
      // device -> XYZ
      cmsHTRANSFORM tr2 = cmsCreateTransform  (cmsTransform2DeviceLink(tr1, 0),
                                               TYPE_Lab_DBL,
                                               0, TYPE_Lab_DBL,
                                               INTENT_RELATIVE_COLORIMETRIC,
                                               0); DBG

#if 0 // Gamut tag
      LPLUT lut = _cmsPrecalculateGamutCheck( tr1 ); DBG
      cmsHPROFILE gmt = _cmsCreateProfilePlaceholder();
      cmsSetDeviceClass( gmt, icSigOutputClass );
      cmsSetColorSpace( gmt, icSigLabData );
      cmsSetPCS( gmt, icSigCmykData );
      _cmsAddLUTTag( gmt, icSigGamutTag, lut ); DBG
      cmsAddTag( gmt, icSigProfileDescriptionTag,  (char*)"GamutCheck");
      _cmsSaveProfile ( gmt,"proof_gamut.icc"); DBG
#endif

      // Wir berechnen die Farbhüllwarnung für ein abstraktes Profil
      viele[1] = hsRGB;
      LPLUT gmt_lut = cmsAllocLUT(), lut; DBG
      lut = cmsReadICCLut( cmyk_p, icSigAToB0Tag);
      cmsAlloc3DGrid( gmt_lut, lut->cLutPoints, 3, 3); DBG
      cmsSample3DGrid( gmt_lut, gamutCheckSampler, tr1, 0 ); DBG

      cmsHPROFILE gmt = _cmsCreateProfilePlaceholder(); DBG
      cmsSetDeviceClass( gmt, icSigAbstractClass ); DBG
      cmsSetColorSpace( gmt, icSigLabData ); DBG
      cmsSetPCS( gmt, icSigLabData ); DBG
      cmsAddTag( gmt, icSigProfileDescriptionTag,  (char*)"GamutCheck"); DBG
      cmsAddTag( gmt, icSigAToB0Tag, gmt_lut ); DBG
      _cmsSaveProfile ( gmt,"proof_gamut.icc"); DBG

      viele[0] = cmsOpenProfileFromMem(const_cast<char*>(block), groesse);
      viele[0] = temp;
      //viele[3] = cmsOpenProfileFromMem(const_cast<char*>(block), groesse);
DBG
      cmsHTRANSFORM tr3 = cmsCreateMultiprofileTransform  (viele, n_profile,
                                               TYPE_Lab_DBL,
                                               TYPE_Lab_DBL,
                                               INTENT_RELATIVE_COLORIMETRIC,
                                               PRECALC|BW_COMP|flags);
DBG
      cmsHPROFILE dp = cmsTransform2DeviceLink(tr3, 0);
DBG
      _cmsSaveProfile (dp,"proof2.icc");
      DBG_V( "proof2.icc geschrieben" )
      if (!hLabtoRGB) WARN_S( _("keine hXYZtoRGB Transformation gefunden") )
#endif
    }

    RGB_Speicher = new double[size*3];
    if(!RGB_Speicher)  WARN_S( _("RGB_speicher Speicher nicht verfuegbar") )

    double *cielab = (double*) alloca (sizeof(double)*3*size);
    LabToCIELab (Lab_Speicher, cielab, size);

    cmsDoTransform (hLabtoRGB, cielab, RGB_Speicher, size);

    //if(hLabtoRGB) cmsDeleteTransform(hLabtoRGB);
    if(hsRGB)     cmsCloseProfile(hsRGB);
    if(hLab)      cmsCloseProfile(hLab);

  DBG_PROG_ENDE
  return RGB_Speicher;
}




