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
 * Der Gamut Rechner.
 * 
 */

// Date:      20. 05. 2005


#include "icc_gamut.h"
#include "icc_profile.h"
#include "icc_utils.h"

namespace icc_examin_ns {

/** @brief holt eine Farbverlauf für RGB Farben */
void
holeRGBRampen( icUInt16Number *block, size_t & zahl )
{
  DBG_PROG_START
  int k = 3;
  double schritte = 10.,
         max = 65535.;

  zahl = (int)schritte*k*2 + 1;

  for(int i = 0; i < (int)zahl*k; ++i)
    block[i] = 0;

  for(int i = 0; i < (int)zahl; ++i) {
    // Rot
    if(i >= schritte * 5 && i < schritte * 6)
      block[k*i+0] = (int)(max/schritte*(i-5*schritte));
    if(i >= schritte * 0 && i < schritte * 2)
      block[k*i+0] = (int)max;
    if(i >= schritte * 2 && i < schritte * 3)
      block[k*i+0] = (int)(max/schritte*(3*schritte-i));
    // Grün
    if(i >= schritte * 1 && i < schritte * 2)
      block[k*i+1] = (int)(max/schritte*(i-1*schritte));
    if(i >= schritte * 2 && i < schritte * 4)
      block[k*i+1] = (int)max;
    if(i >= schritte * 4 && i < schritte * 5)
      block[k*i+1] = (int)(max/schritte*(5*schritte-i));
    // Blau
    if(i >= schritte * 3 && i < schritte * 4)
      block[k*i+2] = (int)(max/schritte*(i-3*schritte));
    if(i >= schritte * 4 && i < schritte * 6)
      block[k*i+2] = (int)max;
    if(i >= schritte * 0 && i < schritte * 1)
      block[k*i+2] = (int)(max/schritte*(1*schritte-i));
  }

  block[zahl*3-3+0] = (int)(max/schritte*(schritte-1/schritte));
  block[zahl*3-3+1] = 0;
  block[zahl*3-3+2] = (int)max;
  DBG_PROG_ENDE
}

void
holeCMYKRampen( icUInt16Number *block, size_t & zahl )
{
  DBG_PROG_START
  int k = 4;
  double schritte = 10.,
         max = 65535.;

  zahl = (int)schritte*(k-1)*2 + 1;

  for(int i = 0; i < (int)zahl*k; ++i) {
    block[i] = 0;
  }

  for(int i = 0; i < (int)zahl; ++i) {
    // Zyan
    if(i >= schritte * 5 && i < schritte * 6)
      block[k*i+0] = (int)(max/schritte*(i-5*schritte));
    if(i >= schritte * 0 && i < schritte * 2)
      block[k*i+0] = (int)max;
    if(i >= schritte * 2 && i < schritte * 3)
      block[k*i+0] = (int)(max/schritte*(3*schritte-i));
    // Magenta
    if(i >= schritte * 1 && i < schritte * 2)
      block[k*i+1] = (int)(max/schritte*(i-1*schritte));
    if(i >= schritte * 2 && i < schritte * 4)
      block[k*i+1] = (int)max;
    if(i >= schritte * 4 && i < schritte * 5)
      block[k*i+1] = (int)(max/schritte*(5*schritte-i));
    // Gelb
    if(i >= schritte * 3 && i < schritte * 4)
      block[k*i+2] = (int)(max/schritte*(i-3*schritte));
    if(i >= schritte * 4 && i < schritte * 6)
      block[k*i+2] = (int)max;
    if(i >= schritte * 0 && i < schritte * 1)
      block[k*i+2] = (int)(max/schritte*(1*schritte-i));
  }

  block[zahl*k-k+0] = (int)(max/schritte*(schritte-1/schritte));
  block[zahl*k-k+1] = 0;
  block[zahl*k-k+2] = (int)max;
  DBG_PROG_ENDE
}

/** @brief creiert eine Linie um die satten Farben in Cmyk und Rgb Profilen */
double*
iccGrenze(ICCprofile & profil, int intent, size_t & groesse)
{
  DBG_PROG_START
  double *lab_erg = 0;
  icColorSpaceSignature csp = profil.colorSpace();
  if(csp == icSigRgbData ||
     csp == icSigCmykData) {
    icUInt16Number *block = (icUInt16Number*) malloc(200*4*sizeof(double));
    icUInt16Number *lab_block = (icUInt16Number*) malloc(200*4*sizeof(double));
    size_t size=0;
    char  *p_block = 0;
    p_block = profil.saveProfileToMem(&size);
#   if 0 
    static int num = 0;
    char text[24]; 
    sprintf (text, "Übung%d.icc", num++);
    saveMemToFile( text, p_block, size );
#   endif
    // hier den Farbumfang abtasten
    cmsHPROFILE lab = cmsCreateLabProfile(cmsD50_xyY());
    cmsHPROFILE p = cmsOpenProfileFromMem(p_block, size);
    cmsHTRANSFORM xform = 0;
    if(csp == icSigRgbData || csp == icSigCmyData) {
      xform = cmsCreateTransform(p, TYPE_RGB_16, lab, TYPE_Lab_16,
                                               intent, cmsFLAGS_NOTPRECALC);
    } else if(csp == icSigCmykData) {
      xform = cmsCreateTransform(p, TYPE_CMYK_16, lab,TYPE_Lab_16,
                                               intent, cmsFLAGS_NOTPRECALC);
    }
    if(xform) {
      size_t zahl = 0;
      if(csp == icSigRgbData)
        holeRGBRampen( (icUInt16Number*)block, zahl );
      else if(csp == icSigCmykData)
        holeCMYKRampen( (icUInt16Number*)block, zahl );
      cmsDoTransform(xform, block, lab_block, zahl);
      groesse = zahl;
      lab_erg = new double [groesse*3];
      for(int i = 0; i < (int)groesse*3; ++i) {
        lab_erg[i] = lab_block[i]/65535.;
      }
      DBG_PROG_V( groesse )
    }
    if(xform) cmsDeleteTransform(xform);
    if(lab) cmsCloseProfile(lab);
    if(p) cmsCloseProfile(p);
    if(p_block) free (p_block);
    if(block) free (block);
    if(lab_block) free (lab_block);
  }
  DBG_PROG_ENDE
  return (double*)lab_erg;
}


}
