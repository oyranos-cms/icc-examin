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
 * Inhalt für das informative "Über" Fenster.
 * 
 */

// Date:      26. 08. 2004

#if 0
  #ifndef DEBUG
   #define DEBUG
  #endif
  #define DEBUG_ICCFORMELN
#endif

#include "icc_profile.h"
#include "icc_utils.h"

#ifdef DEBUG_ICCFORMELN
  #define DBG_v(x) DBG_NUM_V(x)
#else
  #define DBG_v(x)
#endif

#define MIN(a,b) (((a) <= (b)) ? (a) : (b))
#define MAX(a,b) (((a) > (b)) ? (a) : (b))

double
gradATAN(double b, double a)
{
  if ((b) == 0 && (a) == 0) {
    return 0.0;
  } else {
    double d;
    d = atan2 ((b),(a));
    if (d < 0.0) {
      d = d + 2.0*M_PI;
    }
    d = d * 180.0/M_PI; 
    return d;
  }
}

double
dE2000 (Lab Lab1, Lab Lab2, double kL, double kC, double kH)             // (1)
{
  /*
   *  Aus: "The CIEDE2000 Color-Difference Formula:
   *        Implementation Notes, Supplementary Test
   *        Data, and Mathematical Observations "
   *       Gaurav Sharma, Wencheng Wu, Edul N. Dalal
   *
   *  in ciede2000noteCRNA.pdf
   */

  #define nachRAD(grad) (M_PI*(grad)/180.0)
  #define QUAD(num) (num)*(num)

  double Cs1ab = sqrt( QUAD(Lab1.a) + QUAD(Lab1.b)); DBG_v (Cs1ab)
  double Cs2ab = sqrt( QUAD(Lab2.a) + QUAD(Lab2.b)); DBG_v (Cs2ab)       // (2)
  double Cab   = (Cs1ab + Cs2ab) / 2;                DBG_v (Cab)         // (3)
  double G     = 0.5 * (1.0 - sqrt( pow(Cab,7) / (pow(Cab,7) + pow(25,7)))); DBG_v (G)
                                                                         // (4)
  double a1    = (1.0+G) * Lab1.a; DBG_v (a1)
  double a2    = (1.0+G) * Lab2.a; DBG_v (a2)                            // (5)
  double C1    = sqrt( QUAD(a1) + QUAD(Lab1.b)); DBG_v (C1)
  double C2    = sqrt( QUAD(a2) + QUAD(Lab2.b)); DBG_v (C2)              // (6)
  double h1    = gradATAN(Lab1.b, a1);
  double h2    = gradATAN(Lab2.b, a2); DBG_v (h1) DBG_v (h2)             // (7)
  //------------------------------------------------------
  double dL    = Lab2.L - Lab1.L; DBG_v (dL)                             // (8)
  double dC    = C2 - C1;         DBG_v (dC)                             // (9)
  double dh    = 0.0; // (C1*C2 == 0.0)
  if (C1*C2 != 0.0) {
    if ((h2 - h1) < -180.0) {
         dh    = (h2 - h1) + 360.0; DBG_v (dh)
    } else if ((h2 - h1) <= 180.0) {
         dh    = h2 - h1; DBG_v (dh <<" "<< h2 <<" "<< h1)
    } else {// if ((h2 - h1) > 180.0) {
         dh    = (h2 - h1) - 360.0; DBG_v (dh)
    }
  }              DBG_v (dh)                                              // (10)
  double dH    = 2.0 * sqrt(C1 * C2) * sin (nachRAD(dh / 2.0)); DBG_v (dH)//(11)
  //------------------------------------------------------
  double aL    = (Lab1.L + Lab2.L) / 2.0;          DBG_v (aL)            // (12)
  double aC    = (C1 + C2) / 2.0;                   DBG_v (aC)           // (13)
  double mean_h= (h1 + h2); // (C1*C2 == 0.0)
  #if 1
  if (C1*C2 != 0.0) {
    if (fabs(h1 - h2) <= 180.0) {
         mean_h= (h1 + h2) / 2.0; DBG_v (mean_h)
    } else {// if ((h1 - h2) > 180.0) {
      if ((h1 + h2) < 360.0) {
         mean_h= (h1 + h2) / 2.0 + 180.0; DBG_v (mean_h)
      } else {
         mean_h= (h1 + h2) / 2.0 - 180.0; DBG_v (mean_h)
      }
    }
  } DBG_v (mean_h)                                                       // (14)
  #else
  if ( fabs(h1-h2) > 180.0 ) {
    mean_h = MAX(h1,h2) + (360.0 - fabs ( h1-h2 )) / 2.0; DBG_v (mean_h)
  } else {
    mean_h = MIN(h1,h2) + fabs ( h1-h2 ) / 2.0; DBG_v (mean_h)
  }
  while (mean_h > 360.0)
    mean_h = mean_h - 360.0; DBG_v (mean_h)                              // (14)
  #endif
  double T     = 1.0 - 0.17 * cos (nachRAD(mean_h - 30.0))
                     + 0.24 * cos (nachRAD(2.0 * mean_h))
                     + 0.32 * cos (nachRAD(3.0 * mean_h + 6.0))
                     - 0.20 * cos (nachRAD(4.0 * mean_h - 63.0));DBG_v (T)//(15)
  double dTheta= 30.0 * exp (- QUAD((mean_h - 275.0)/25.0)); DBG_v (dTheta)
                                                                         // (16) 
  double RC    = 2.0 * sqrt( pow(aC,7)/(pow(aC,7) + pow(25, 7))); DBG_v (RC)
                                                                         // (17)
  double SL    = 1.0 + (0.015 * QUAD(aL - 50.0))
                     / sqrt(20.0 + QUAD(aL - 50.0)); DBG_v (SL)          // (18)
  double SC    = 1.0 + 0.045 * aC;                 DBG_v (SC)            // (19)
  double SH    = 1.0 + 0.015 * aC * T;              DBG_v (SH)           // (20)
  double RT    = - sin (nachRAD(2.0 * dTheta)) * RC; DBG_v (RT)          // (21)

  double de2000= sqrt( 
                     QUAD(dL / (kL * SL))
                   + QUAD(dC / (kC * SC))
                   + QUAD(dH / (kH * SH))
                   + RT * dC / (kC * SC)
                        * dH / (kH * SH)
                 );                                                      // (22)
  DBG_v ( de2000 )
  return de2000;
}


