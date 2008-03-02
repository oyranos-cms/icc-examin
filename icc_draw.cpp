// Author:    Kai-Uwe Behrmann
// Copyright: Kai-Uwe Behrmann
// Date:      Mai 2004

#include "icc_examin.h"
//#include "icc_vrml.h"
#include <stdio.h>
#include <iostream>
#include <string.h>
#include <FL/Fl.H>
#include <FL/fl_draw.H>

#include <lcms.h>
#include "cccie64.h"
#include "ciexyz64_1.h"

void
draw_cie_shoe (int X, int Y, int W, int H)
{
  float w,h, xO, yO;

  // Zeichenflaeche
  fl_color(FL_GRAY);
  fl_rectf(X,Y,W,H);

  // Zeichenbereichvariablen
  int tab_border_x=30;
  int tab_border_y=30;
  // Diagrammvariablen
  float n = 0.85; 

  xO = X + tab_border_x + 10;     // Ursprung
  yO = Y + H - tab_border_y - 10; // Ursprung
  w  = (W - 2*tab_border_x);    // Breite des Diagrammes
  h  = (H - 2*tab_border_y);    // Hoehe des Diagrammes

  #define x(val) xO + (val)*w/n 
  #define y(val) yO - (val)*h/n
  #define x2cie(val) ((val)-xO)/w
  #define y2cie(val) (yO-(val))/h
  fl_push_clip( X,y(n), x(n),h+tab_border_y);

  // Spektrumvariablen
  int nano_min = 63; // 420 nm
  int nano_max = 341; // 700 nm

  // Tangente
  fl_color(FL_LIGHT1);
  fl_line(x(1), y(0), x(0), y(1));


  int r, g, b;
  cmsHPROFILE hXYZ, hsRGB;

  hXYZ = cmsCreateXYZProfile();
  hsRGB = cmsCreate_sRGBProfile();

  cmsHTRANSFORM xform = cmsCreateTransform(hXYZ, TYPE_XYZ_DBL,
                                           hsRGB, TYPE_RGB_8,
                                           INTENT_ABSOLUTE_COLORIMETRIC,
                                           cmsFLAGS_NOTPRECALC);
  register char RGB[3];
  register cmsCIEXYZ XYZ;
  cmsCIEXYZ xyz, MediaWhite;
  cmsCIExyY xyY, WhitePt;

  cmsTakeMediaWhitePoint(&MediaWhite, hsRGB);
  cmsXYZ2xyY(&WhitePt, &MediaWhite);

  
  // Farbfläche
  int raster = 4;
  for (float cie_y=y(0.01) ; cie_y > y(n) ; cie_y -= raster)
    for (float cie_x=x(0) ; cie_x < x(0.73) ; cie_x+=raster) {

      XYZ.X = x2cie(cie_x);
      XYZ.Y = y2cie(cie_y);
      XYZ.Z = 1 - (XYZ.X +  XYZ.Y);

      cmsDoTransform(xform, &XYZ, RGB, 1);

      fl_color (fl_rgb_color (RGB[0],RGB[1],RGB[2]));
      if (raster > 1)
          fl_rectf ((int)cie_x , (int)cie_y, raster,raster);
      else
          fl_point ((int)cie_x , (int)cie_y);
    }
  cmsDeleteTransform(xform);
  cmsCloseProfile(hXYZ);
  cmsCloseProfile(hsRGB);

  // Verdecke den Rest des cie_xy
  fl_push_no_clip();
  fl_color(FL_GRAY);
  #define x_xyY cieXYZ[i][0]/(cieXYZ[i][0]+cieXYZ[i][1]+cieXYZ[i][2])
  #define y_xyY cieXYZ[i][1]/(cieXYZ[i][0]+cieXYZ[i][1]+cieXYZ[i][2])


  fl_begin_polygon();
  fl_vertex (X, Y+H);
  fl_vertex (x(.18), Y+H);
  for (int i=nano_min ; i<=nano_max*.38; i++)
    fl_vertex( x(x_xyY), y(y_xyY) );
  fl_vertex (X, y(.25));
  fl_end_polygon();
//fl_color(FL_WHITE);
  fl_begin_polygon();
  fl_vertex (X, y(.25));
  for (int i=nano_max*.38 ; i<=nano_max*.457; i++)
    fl_vertex( x(x_xyY), y(y_xyY) );
  fl_vertex (x(.05), Y);
  fl_vertex (X, Y);
  fl_end_polygon();
//fl_color(FL_BLUE);
  fl_begin_polygon();
  fl_vertex (x(.05), Y);
  for (int i=nano_max*.457 ; i<=nano_max*.48; i++)
    fl_vertex( x(x_xyY), y(y_xyY) );
  fl_vertex (x(.2), Y);
  fl_end_polygon();
//fl_color(FL_RED);
  fl_begin_polygon();
  fl_vertex (X+W, Y);
  fl_vertex (x(0.2), Y);
  for (int i=nano_max*.48; i<=nano_max*.6; i++)
    fl_vertex( x(x_xyY), y(y_xyY) );
  fl_end_polygon();
//fl_color(FL_GREEN);
  fl_begin_polygon();
  fl_vertex (X+W, Y);
  for (int i=nano_max*.6; i<=nano_max ; i++)
    fl_vertex( x(x_xyY), y(y_xyY) );
  fl_vertex( x(cieXYZ[nano_min][0]/(cieXYZ[nano_min][0]+cieXYZ[nano_min][1]+cieXYZ[nano_min][2])),
             y(cieXYZ[nano_min][1]/(cieXYZ[nano_min][0]+cieXYZ[nano_min][1]+cieXYZ[nano_min][2])) );
  fl_vertex (x(.18), Y+H);
  fl_vertex (X+W, Y+H);
  fl_end_polygon();

  fl_pop_clip();


  // Diagramm
  fl_color(FL_LIGHT2);
    // Text
  fl_font (FL_HELVETICA, 10);
    // Raster
  for (float f=0 ; f<n ; f+=0.1) {
    static char text[64];
    fl_line ( x(f),yO+5, x(f),y(n));
    fl_line ( xO-5,y(f), x(n),y(f));
    sprintf (text, "%.1f", f);
    fl_draw ( text, xO-30,(int)y(f)+4 );
    fl_draw ( text, (int)x(f)-7,yO+20 );
  }
  

  // Weisspunkt
  fl_color (fl_rgb_color (255,255,255));
  fl_circle (x(WhitePt.x), y(WhitePt.y), 5.0);

  //Meldung
  //printf ("%d %d %d %d\n",X,Y,W,H);

  fl_pop_clip();
  DBG
}


/**********************************************************************/
// http://www.physics.sfasu.edu/astro/color/blackbodyc.txt

double DMAX1(double x, double y, double z);

/*
 ****************************************************************************
 *
 *      XYZ VALUES FROM TEMPERATURE OF OBJECT
 *
 *       A black body approximation is used where the temperature,
 *       T, is given in Kelvin.  The XYZ values are determined by
 *      "integrating" the product of the wavelength distribution of
 *       energy and the XYZ functions for a uniform source.
 */
void BlackBody(double temperature, double *X, double *Y,double *Z) {

/*
 *      CIE Color Matching Functions (x_bar,y_bar,z_bar)
 *      for wavelenghts in 5 nm increments from 380 nm to 780 nm.
 */
       double fColorMatch[][3]={
         {0.0014, 0.0000, 0.0065},
         {0.0022, 0.0001, 0.0105},
         {0.0042, 0.0001, 0.0201},
         {0.0076, 0.0002, 0.0362},
         {0.0143, 0.0004, 0.0679},
         {0.0232, 0.0006, 0.1102},
         {0.0435, 0.0012, 0.2074},
         {0.0776, 0.0022, 0.3713},
         {0.1344, 0.0040, 0.6456},
         {0.2148, 0.0073, 1.0391},
         {0.2839, 0.0116, 1.3856},
         {0.3285, 0.0168, 1.6230},
         {0.3483, 0.0230, 1.7471},
         {0.3481, 0.0298, 1.7826},
         {0.3362, 0.0380, 1.7721},
         {0.3187, 0.0480, 1.7441},
         {0.2908, 0.0600, 1.6692},
         {0.2511, 0.0739, 1.5281},
         {0.1954, 0.0910, 1.2876},
         {0.1421, 0.1126, 1.0419},
         {0.0956, 0.1390, 0.8130},
         {0.0580, 0.1693, 0.6162},
         {0.0320, 0.2080, 0.4652},
         {0.0147, 0.2586, 0.3533},
         {0.0049, 0.3230, 0.2720},
         {0.0024, 0.4073, 0.2123},
         {0.0093, 0.5030, 0.1582},
         {0.0291, 0.6082, 0.1117},
         {0.0633, 0.7100, 0.0782},
         {0.1096, 0.7932, 0.0573},
         {0.1655, 0.8620, 0.0422},
         {0.2257, 0.9149, 0.0298},
         {0.2904, 0.9540, 0.0203},
         {0.3597, 0.9803, 0.0134},
         {0.4334, 0.9950, 0.0087},
         {0.5121, 1.0000, 0.0057},
         {0.5945, 0.9950, 0.0039},
         {0.6784, 0.9786, 0.0027},
         {0.7621, 0.9520, 0.0021},
         {0.8425, 0.9154, 0.0018},
         {0.9163, 0.8700, 0.0017},
         {0.9786, 0.8163, 0.0014},
         {1.0263, 0.7570, 0.0011},
         {1.0567, 0.6949, 0.0010},
         {1.0622, 0.6310, 0.0008},
         {1.0456, 0.5668, 0.0006},
         {1.0026, 0.5030, 0.0003},
         {0.9384, 0.4412, 0.0002},
         {0.8544, 0.3810, 0.0002},
         {0.7514, 0.3210, 0.0001},
         {0.6424, 0.2650, 0.0000},
         {0.5419, 0.2170, 0.0000},
         {0.4479, 0.1750, 0.0000},
         {0.3608, 0.1382, 0.0000},
         {0.2835, 0.1070, 0.0000},
         {0.2187, 0.0816, 0.0000},
         {0.1649, 0.0610, 0.0000},
         {0.1212, 0.0446, 0.0000},
         {0.0874, 0.0320, 0.0000},
         {0.0636, 0.0232, 0.0000},
         {0.0468, 0.0170, 0.0000},
         {0.0329, 0.0119, 0.0000},
         {0.0227, 0.0082, 0.0000},
         {0.0158, 0.0057, 0.0000},
         {0.0114, 0.0041, 0.0000},
         {0.0081, 0.0029, 0.0000},
         {0.0058, 0.0021, 0.0000},
         {0.0041, 0.0015, 0.0000},
         {0.0029, 0.0010, 0.0000},
         {0.0020, 0.0007, 0.0000},
         {0.0014, 0.0005, 0.0000},
         {0.0010, 0.0004, 0.0000},
         {0.0007, 0.0002, 0.0000},
         {0.0005, 0.0002, 0.0000},
         {0.0003, 0.0001, 0.0000},
         {0.0002, 0.0001, 0.0000},
         {0.0002, 0.0001, 0.0000},
         {0.0001, 0.0000, 0.0000},
         {0.0001, 0.0000, 0.0000},
         {0.0001, 0.0000, 0.0000},
         {0.0000, 0.0000, 0.0000}};

    double XX=0.0, YY=0.0, ZZ=0.0; /* initialize accumulators */
    double con, dis, wavelength, weight;
    short band, nbands=81;

    /* loop over wavelength bands
     * integration by trapezoid method
     */
    for(band=0; band<nbands; band++) {
        weight=1.0;
        if((band==0)||(band==nbands-1)) weight=0.5; /* properly weight end
points */
        wavelength=380.0+(double)band*5.0;/* wavelength in nm */
        /* generate a black body spectrum */
        con=1240.0/8.617e-5;

dis=3.74183e-16*(1.0/pow(wavelength,5))/(exp(con/(wavelength*temperature))-1.);
/* simple integration over bands */
        XX=XX+weight*dis*fColorMatch[band][0];
        YY=YY+weight*dis*fColorMatch[band][1];
        ZZ=ZZ+weight*dis*fColorMatch[band][2];
    } /* end of 'band' loop */

    /* re-normalize the color scale */
    *X=XX/DMAX1(XX,YY,ZZ);
    *Y=YY/DMAX1(XX,YY,ZZ);
    *Z=ZZ/DMAX1(XX,YY,ZZ);

}

/* ************************************************************************/
double DMAX1(double x, double y, double z) {
    double max;
    max=x;
    if(y>max) max=y;
    if(z>max) max=z;
    return max;
}


