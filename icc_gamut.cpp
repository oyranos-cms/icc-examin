/*
 * ICC Examin ist eine ICC Profil Betrachter
 * 
 * Copyright (C) 2004-2011  Kai-Uwe Behrmann 
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
 * the gamut calculator.
 * 
 */

// Date:      20. 05. 2005

#include <alpha/oyranos_alpha.h>

#include "icc_gamut.h"
#include "icc_profile.h"
#include "icc_utils.h"

namespace icc_examin_ns {

/** @brief creates a colour gradient for RGB colours */
icUInt16Number* holeRGBRampen( size_t & zahl )
{
  DBG_PROG_START
  int k = 3;
  double schritte = 10.,
         max = 65535.;
  zahl = (int)schritte*k*2 + 1;

  icUInt16Number *block = (icUInt16Number*) calloc( zahl*k,
                                                    sizeof(icUInt16Number) );

  for(int i = 0; i < (int)zahl; ++i) {
    // red
    if(i >= schritte * 5 && i < schritte * 6)
      block[k*i+0] = (int)(max/schritte*(i-5*schritte));
    if(i >= schritte * 0 && i < schritte * 2)
      block[k*i+0] = (int)max;
    if(i >= schritte * 2 && i < schritte * 3)
      block[k*i+0] = (int)(max/schritte*(3*schritte-i));
    // green
    if(i >= schritte * 1 && i < schritte * 2)
      block[k*i+1] = (int)(max/schritte*(i-1*schritte));
    if(i >= schritte * 2 && i < schritte * 4)
      block[k*i+1] = (int)max;
    if(i >= schritte * 4 && i < schritte * 5)
      block[k*i+1] = (int)(max/schritte*(5*schritte-i));
    // blue
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
  return block;
}

icUInt16Number* holeCMYKRampen( size_t & zahl )
{
  DBG_PROG_START
  int k = 4;
  double schritte = 10.,
         max = 65535.;

  zahl = (int)schritte*(k-1)*2 + 1;

  icUInt16Number *block = (icUInt16Number*) calloc( zahl*k,
                                                    sizeof(icUInt16Number) );

  for(int i = 0; i < (int)zahl*k; ++i) {
    block[i] = 0;
  }

  for(int i = 0; i < (int)zahl; ++i) {
    // cyan
    if(i >= schritte * 5 && i < schritte * 6)
      block[k*i+0] = (int)(max/schritte*(i-5*schritte));
    if(i >= schritte * 0 && i < schritte * 2)
      block[k*i+0] = (int)max;
    if(i >= schritte * 2 && i < schritte * 3)
      block[k*i+0] = (int)(max/schritte*(3*schritte-i));
    // magenta
    if(i >= schritte * 1 && i < schritte * 2)
      block[k*i+1] = (int)(max/schritte*(i-1*schritte));
    if(i >= schritte * 2 && i < schritte * 4)
      block[k*i+1] = (int)max;
    if(i >= schritte * 4 && i < schritte * 5)
      block[k*i+1] = (int)(max/schritte*(5*schritte-i));
    // yellow
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
  return block;
}

/** @brief creates a linie around the saturated colours of Cmyk and Rgb profiles */
double*
iccGrenze(ICCprofile & profil, oyOptions_s * options, size_t & groesse)
{
  DBG_PROG_START
  double *lab_erg = 0;
  icColorSpaceSignature csp = profil.colorSpace();
  if(csp == icSigRgbData ||
     csp == icSigCmykData)
  {
    icUInt16Number *block = 0;
    icUInt16Number *lab_block = (icUInt16Number*) malloc(200*4*sizeof(double));
    size_t size=0;
    char  *p_block = 0;
    p_block = profil.saveProfileToMem(&size);

    // scan here the colour space border
    oyProfile_s * lab = oyProfile_FromStd(oyEDITING_LAB, 0);
    oyProfile_s * p = oyProfile_FromMem( size, p_block, 0,0);
    {
      size_t lenght = 0;
      if(csp == icSigRgbData)
        block = (icUInt16Number*) holeRGBRampen( lenght );
      else if(csp == icSigCmykData)
        block = (icUInt16Number*) holeCMYKRampen( lenght );

      oyColourConvert_( p, lab, block, lab_block,
                        oyUINT16, oyUINT16, options, lenght );
      groesse = lenght;
      lab_erg = new double [groesse*3];
      for(int i = 0; i < (int)groesse*3; ++i) {
        lab_erg[i] = lab_block[i]/65535.;
      }
      DBG_PROG_V( groesse )
    }
    oyProfile_Release( &lab );
    oyProfile_Release( &p );
    if(p_block) free (p_block);
    if(block) free (block);
    if(lab_block) free (lab_block);
  }
  DBG_PROG_ENDE
  return (double*)lab_erg;
}


}
