/*
 * ICC Examin ist eine ICC Profil Betrachter
 * 
 * Copyright (C) 2004-2009  Kai-Uwe Behrmann 
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
 * some mathematical formulas
 * 
 */

// Date:      26. 08. 2004

#if 0
# ifndef DEBUG
#   define DEBUG
# endif
# define DEBUG_ICCFORMELN
#endif

#include "icc_formeln.h"

#include <cmath>
#include <cstring>

#ifdef DEBUG_ICCFORMELN
# define DBG_v(x) DBG_NUM_V(x)
#else
# define DBG_v(x)
#endif

double
gradATAN(const double b, const double a)
{
  if ((b) == 0 && (a) == 0) {
    return 0.0;
  } else {
    double d = atan2 ((b),(a));

    if (d < 0.0) 
      d = d + 2.0*M_PI;
    if (d > 2.0*M_PI)
      d = d - 2.0*M_PI;

    d = d * 180.0/M_PI; 
    return d;
  }
}

double
dE2000 (const Lab_s & Lab1_n, const Lab_s & Lab2_n,
        const double kL, const double kC, const double kH)             // (1)
{
  // convert to CIE*Lab
  Lab_s Lab1, Lab2; 
  Lab1.L = Lab1_n.L * 100.;
  Lab2.L = Lab2_n.L * 100.;
  Lab1.a = Lab1_n.a * 256. - 128.;
  Lab2.a = Lab2_n.a * 256. - 128.;
  Lab1.b = Lab1_n.b * 256. - 128.;
  Lab2.b = Lab2_n.b * 256. - 128.;

  /*
   *  Aus: "The CIEDE2000 Color-Difference Formula:
   *        Implementation Notes, Supplementary Test
   *        Data, and Mathematical Observations "
   *       Gaurav Sharma, Wencheng Wu, Edul N. Dalal
   *
   *  in ciede2000noteCRNA.pdf
   */

# define nachRAD(grad) (M_PI*(grad)/180.0)
# define QUAD(num) (num)*(num)

  double Cs1ab = sqrt( QUAD(Lab1.a) + QUAD(Lab1.b)); DBG_v (Cs1ab)
  double Cs2ab = sqrt( QUAD(Lab2.a) + QUAD(Lab2.b)); DBG_v (Cs2ab)       // (2)
  double Cab   = (Cs1ab + Cs2ab) / 2;                DBG_v (Cab)         // (3)
  double G     = 0.5 * (1.0 - sqrt( pow(Cab,7) / (pow(Cab,7) + pow((double)25,7)))); DBG_v (G)
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
# if 1
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
# else
  if ( fabs(h1-h2) > 180.0 ) {
    mean_h = MAX(h1,h2) + (360.0 - fabs ( h1-h2 )) / 2.0; DBG_v (mean_h)
  } else {
    mean_h = MIN(h1,h2) + fabs ( h1-h2 ) / 2.0; DBG_v (mean_h)
  }
  while (mean_h > 360.0)
    mean_h = mean_h - 360.0; DBG_v (mean_h)                              // (14)
# endif
  double T     = 1.0 - 0.17 * cos (nachRAD(mean_h - 30.0))
                     + 0.24 * cos (nachRAD(2.0 * mean_h))
                     + 0.32 * cos (nachRAD(3.0 * mean_h + 6.0))
                     - 0.20 * cos (nachRAD(4.0 * mean_h - 63.0));DBG_v (T)//(15)
  double dTheta= 30.0 * exp (- QUAD((mean_h - 275.0)/25.0)); DBG_v (dTheta)
                                                                         // (16) 
  double RC    = 2.0 * sqrt( pow(aC,7)/(pow(aC,7) + pow((double)25, 7))); DBG_v (RC)
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

double
dE ( const Lab_s Lab1, const Lab_s Lab2 )
{
  double l1[3], l2[3];
  LabToCIELab( Lab1, l1 );
  LabToCIELab( Lab2, l2 );

  return dE( l1, l2 );
}

double
dE ( const double * l1, const Lab_s Lab2 )
{
  double l2[3];
  LabToCIELab( Lab2, l2 );
  return dE( l1, l2 );
}

double
dE ( const double * l1, const double * l2 )
{
  return HYP3( l1[0]-l2[0], l1[1]-l2[1], l1[2]-l2[2] );
}

void
LabtoXYZ (const Lab_s &lab, XYZ_s &XYZ)
{
  double l[3];
  //double e = 216./24389.;             // 0.0088565
  //double k = 24389./27.;              // 903.30
  double d = 6./29.;                  // 0.20690

  double Xn = 0.964294;
  double Yn = 1.000000;
  double Zn = 0.825104;

  LabToCIELab(lab, l);
  double fy = (l[0] + 16) / 116.;
  double fx = fy + l[1] / 500.;
  double fz = fy - l[2] / 200.;


  if(fy > d)
    XYZ.Y = Yn * pow( fy, 3 );
  else
    XYZ.Y = (fy - 16./116.) * 3 * pow( d, 2 ) * Yn;
  if(fx > d)
    XYZ.X = Xn * pow( fx, 3 );
  else
    XYZ.X = (fx - 16./116.) * 3 * pow( d, 2 ) * Xn;
  if(fz > d)
    XYZ.Z = Zn * pow( fz, 3 );
  else
    XYZ.Z = (fz - 16./116.) * 3 * pow( d, 2 ) * Zn;
}

void
XYZtoLab (const XYZ_s &xyz, Lab_s &lab)
{
    /* white point D50 [0.964294 , 1.000000 , 0.825104]
     * XYZ->Lab is defined as (found with the help of Marti Maria):
     *
     * L* = 116*f(Y/Yn) - 16                     0 <= L* <= 100
     * a* = 500*[f(X/Xn) - f(Y/Yn)]
     * b* = 200*[f(Y/Yn) - f(Z/Zn)]
     *
     * and
     *
     *        f(t) = t^(1/3)                     1 >= t >  0.008856
     *         7.787*t + (16/116)          0 <= t <= 0.008856
     */

      double gamma = 1.0/3.0; // standard is 1.0/3.0
      XYZ_s xyz_;
      double K = 24389./27.;
      double k = K/116.;      // 7.787
      double e = 216./24389.; // 0.008856

      // CIE XYZ -> CIE*Lab (D50)
      xyz_.X = xyz.X / 0.964294;
      xyz_.Y = xyz.Y / 1.000000;
      xyz_.Z = xyz.Z / 0.825104;

      if ( xyz_.X > e)
         xyz_.X = pow (xyz_.X, gamma);
      else
         xyz_.X = k * xyz_.X + (16.0/116.0);
      if ( xyz_.Y > e)
         xyz_.Y = pow ( xyz_.Y, gamma);
      else
         xyz_.Y = k * xyz_.Y + (16.0/116.0);
      if ( xyz_.Z > e)
         xyz_.Z = pow ( xyz_.Z, gamma);
      else
         xyz_.Z = k * xyz_.Z + (16.0/116.0);

      // auf 0.0-1.0 normalise ------v
      lab.L = (116.0*  xyz_.Y -  16.0)   / 100.0;
      lab.a = (500.0*( xyz_.X -  xyz_.Y) + 128.0) / 256.0 ;
      lab.b = (200.0*( xyz_.Y -  xyz_.Z) + 128.0) / 256.0 ;
}

void
XYZtoLab (const double* xyz, double* lab, int n)
{
    XYZ_s xyz_;
    Lab_s lab_;
    for(int i = 0; i < n; ++i) {

      xyz_.X = xyz[i*3+0];
      xyz_.Y = xyz[i*3+1];
      xyz_.Z = xyz[i*3+2];

      XYZtoLab (xyz_, lab_);

      lab[i*3+0] = lab_.L;
      lab[i*3+1] = lab_.a;
      lab[i*3+2] = lab_.b;
    }
}

void
CIELabToLab (const double* cielab, double* lab, int n)
{
    for(int i = 0; i < n; ++i)
    {
      lab[i*3+0] =  cielab[i*3+0]          / 100.0;
      lab[i*3+1] = (cielab[i*3+1] + 128.0) / 256.0;
      lab[i*3+2] = (cielab[i*3+2] + 128.0) / 256.0;
    }
}

void
CIELabToLab (const double* cielab, Lab_s & lab)
{
      lab.L =  cielab[0]          / 100.0;
      lab.a = (cielab[1] + 128.0) / 256.0;
      lab.b = (cielab[2] + 128.0) / 256.0;
}

void
LabToCIELab (const double* lab, double* cielab, int n)
{
    for(int i = 0; i < n; ++i)
    {
      cielab[i*3+0] =  lab[i*3+0] * 100.0;
      cielab[i*3+1] = (lab[i*3+1] * 256.0) - 128.0;
      cielab[i*3+2] = (lab[i*3+2] * 256.0) - 128.0;
      //DBG_NUM_V( lab[i*3+0] <<" "<< lab[i*3+1] <<" "<< lab[i*3+2] )
      //DBG_NUM_V( cielab[i*3+0] <<" "<< cielab[i*3+1] <<" "<< cielab[i*3+2] )
    }
}

void
LabToCIELab (const Lab_s & lab, double* cielab)
{
      cielab[0] =  lab.L * 100.0;
      cielab[1] = (lab.a * 256.0) - 128.0;
      cielab[2] = (lab.b * 256.0) - 128.0;
}

void
OyLabToLab (const double* oylab, Lab_s & lab)
{
      lab.L =  oylab[0]          / 1.0;
      lab.a = (oylab[1] + 1.28f) / 2.56f;
      lab.b = (oylab[2] + 1.28f) / 2.56f;
}

void
OyLabToLab (const double* oylab, double* lab, int n)
{
    for(int i = 0; i < n; ++i)
    {
      lab[i*3+0] =  oylab[i*3+0]          / 1.0;
      lab[i*3+1] = (oylab[i*3+1] + 1.28f) / 2.56f;
      lab[i*3+2] = (oylab[i*3+2] + 1.28f) / 2.56f;
    }
}

void
LabToOyLab (const double* lab, double* oylab, int n)
{
    for(int i = 0; i < n; ++i)
    {
      oylab[i*3+0] =  lab[i*3+0];
      oylab[i*3+1] = (lab[i*3+1] * 2.56f) - 1.28f;
      oylab[i*3+2] = (lab[i*3+2] * 2.56f) - 1.28f;
    }
}

void
LabToOyLab (const Lab_s & lab, double* cielab)
{
      cielab[0] =  lab.L * 1.0f;
      cielab[1] = (lab.a * 2.56f) - 1.28f;
      cielab[2] = (lab.b * 2.56f) - 1.28f;
}

void
FarbeZuDouble (double* d_xyz, XYZ_s xyz)
{ DBG_PROG
  d_xyz[0] = xyz.X;
  d_xyz[1] = xyz.Y;
  d_xyz[2] = xyz.Z;
}

void
FarbeZuDouble (XYZ_s *xyz, double* d_xyz)
{ DBG_PROG
  xyz->X = d_xyz[0];
  xyz->Y = d_xyz[1];
  xyz->Z = d_xyz[2];
}

void
FarbeZuDouble (double* d_lab, Lab_s lab)
{ DBG_PROG
  d_lab[0] = lab.L;
  d_lab[1] = lab.a;
  d_lab[2] = lab.b;
}

void
FarbeZuDouble (Lab_s *lab, double* d_lab)
{ DBG_PROG
  lab->L = d_lab[0];
  lab->a = d_lab[1];
  lab->b = d_lab[2];
}

void
FarbeZuDouble (double* d_rgb, RGB_s rgb)
{ DBG_PROG
  d_rgb[0] = rgb.R;
  d_rgb[1] = rgb.G;
  d_rgb[2] = rgb.B;
}

void
FarbeZuDouble (RGB_s *rgb, double* d_rgb)
{ DBG_PROG
  rgb->R = d_rgb[0];
  rgb->G = d_rgb[1];
  rgb->B = d_rgb[2];
}

void
FarbeZuDouble (double* d_cmyk, CMYK_s cmyk)
{ DBG_PROG
  d_cmyk[0] = cmyk.C;
  d_cmyk[1] = cmyk.M;
  d_cmyk[2] = cmyk.Y;
  d_cmyk[3] = cmyk.K;
}

void
FarbeZuDouble (CMYK_s *cmyk, double* d_cmyk)
{ DBG_PROG
  cmyk->C = d_cmyk[0];
  cmyk->M = d_cmyk[1];
  cmyk->Y = d_cmyk[2];
  cmyk->K = d_cmyk[3];
}


