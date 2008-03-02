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
 * -----------------------------------------------------------------------------
 *
 * Zeichenroutinen für Diagramme.
 * 
 */

//#define DEBUG_DRAW

#include "icc_helfer.h"
#include "icc_profile.h"
#include "icc_helfer_ui.h"
#include <stdio.h>
#include <iostream>
#include <vector>
#include <string.h>
#include <FL/Fl.H>
#include <FL/fl_draw.H>

#include <lcms.h>
#include "cccie64.h"
#include "ciexyz64_1.h"

#ifdef HAVE_FLTK
#include "icc_draw_fltk.h"
#endif

// Zeichenbereich
float w,h, xO, yO;
// Zeichenbereichvariablen
int tab_border_x=30;
int tab_border_y=30;
// Diagrammvariablen
float n = 1.0;

#if 0
//#ifdef __APPLE__
  #define BG FL_DARK3
  #define VG FL_DARK2
  #define DIAG FL_DARK2-4
//#else
#if 0
  #define BG FL_GRAY
  #define VG FL_LIGHT2
  #define DIAG FL_LIGHT1
#else
  #define BG FL_DARK2
  #define VG FL_BACKGROUND_COLOR
  #define DIAG VG
#endif
//#endif
#endif

#ifdef DEBUG_DRAW
  #define DBG_prog_start DBG_PROG_START
  #define DBG_prog_ende DBG_PROG_ENDE
#else
  #define DBG_prog_start
  #define DBG_prog_ende
#endif


int raster = 4;
int init_s = FALSE;
// lcms Typen
cmsHPROFILE hXYZ;
cmsHPROFILE hsRGB;
cmsHTRANSFORM xform;
double rechenzeit = 0.1;
static unsigned char* RGB_speicher = 0;
static cmsCIEXYZ* XYZ_speicher = 0;
static int n_speicher = 0;

void
init_shoe() {
  // Initialisierung für lcms
  hXYZ  = cmsCreateXYZProfile();
  hsRGB = cmsCreate_sRGBProfile();

  xform = cmsCreateTransform              (hXYZ, TYPE_XYZ_DBL,
                                           hsRGB, TYPE_RGB_8,
                                           INTENT_ABSOLUTE_COLORIMETRIC,
                                           cmsFLAGS_NOTPRECALC);
}

void
draw_cie_shoe (int X, int Y, int W, int H,
                    std::vector<std::string>               texte,
                    std::vector<double>                    punkte,
                    int                                    repeated)
{ DBG_prog_start
  if (!init_s)
    init_shoe();
  init_s = TRUE;

  double rz = (double)clock()/(double)CLOCKS_PER_SEC;
  int raster_alt = raster;
  if (repeated)
    raster = 1;

  // Zeichenflaeche
  fl_color(BG);
  fl_rectf(X,Y,W,H);

  xO = X + tab_border_x + 10;     // Ursprung
  yO = Y + H - tab_border_y - 10; // Ursprung
  w  = (W - 2*tab_border_x);      // Breite des Diagrammes
  h  = (H - 2*tab_border_y);      // Hoehe des Diagrammes

  // dargestellter Ausschnitt 
  n = .85;

  #define x(val) (int)(((double)xO + (double)(val)*w/n)+0.5)
  #define y(val) (int)(((double)yO - (double)(val)*h/n)+0.5)
  #define x2cie(val) (((val)-xO)/w)
  #define y2cie(val) ((yO-(val))/h)
  fl_push_clip( X,y(n), x(n),(int)(h+tab_border_y+0.5) );

  // Spektrumvariablen
  int nano_min = 63; // 420 nm
  int nano_max = 341; // 700 nm

  // Tangente
  fl_color(DIAG);
  fl_line(x(1), y(0), x(0), y(1));

  // Farbfläche
  if (!repeated) {
    register char RGB[3];
    register cmsCIEXYZ XYZ;

    for (float cie_y=y(0.01) ; cie_y > y(n) ; cie_y -= raster)
      for (float cie_x=x(0) ; cie_x < x(0.73) ; cie_x+=raster) {

        XYZ.X = x2cie(cie_x);
        XYZ.Y = y2cie(cie_y);
        XYZ.Z = 1 - (XYZ.X +  XYZ.Y);

        // Hintergrund zeichnen (lcms)
        cmsDoTransform(xform, &XYZ, RGB, 1);

        fl_color (fl_rgb_color (RGB[0],RGB[1],RGB[2]));
        if (raster > 1)
          fl_rectf ((int)cie_x , (int)cie_y, raster,raster);
        else
          fl_point ((int)cie_x , (int)cie_y);
      }
  } else {
    int wi = x(0.73)-x(0);
    int hi = y(0.01)-y(n);

    int    n_pixel = wi * hi, i = 0;
    if ((n_speicher > 3*n_pixel * 2)
     || (n_speicher < 3*n_pixel))
    { if (RGB_speicher)
      { delete [] RGB_speicher;
        delete [] XYZ_speicher;
      }
      int multi = 2;
      n_speicher = 3*n_pixel;
      RGB_speicher = (unsigned char*) new char [n_speicher * multi];
      XYZ_speicher = (cmsCIEXYZ*) new cmsCIEXYZ [n_pixel * multi];
    }

    for (float cie_y=y(n) ; cie_y < y(0.01) ; cie_y ++) {
      for (float cie_x=x(0) ; cie_x < x(0.73) ; cie_x ++) {

        XYZ_speicher[i].X = x2cie(cie_x);
        XYZ_speicher[i].Y = y2cie(cie_y);
        XYZ_speicher[i].Z = 1 - (XYZ_speicher[i].X +  XYZ_speicher[i].Y);
        i++;
      }
    }
    // Hintergrund zeichnen (lcms)
    cmsDoTransform(xform, XYZ_speicher, RGB_speicher, n_pixel);
    fl_draw_image(RGB_speicher, x(0), y(n), wi, hi, 3, 0);
  }

  // Dauer des Neuzeichnens bestimmen
  if (!repeated) {
    rz = (double)clock()/(double)CLOCKS_PER_SEC - rz;
    rechenzeit += rz;
    //cout << rechenzeit << " Sekunden  "; DBG
    if (rechenzeit > 0.05)
      raster ++;
    else if (rechenzeit < 0.03)
      raster --;
    if (raster < 1)
      raster = 1;
    rechenzeit = rz;
  } else {
    raster = raster_alt;
  }

  // Verdecke den Rest des cie_xy - scheußlich umständlich
  //fl_push_no_clip();
  fl_color(BG);
  #define x_xyY cieXYZ[i][0]/(cieXYZ[i][0]+cieXYZ[i][1]+cieXYZ[i][2])
  #define y_xyY cieXYZ[i][1]/(cieXYZ[i][0]+cieXYZ[i][1]+cieXYZ[i][2])


  fl_begin_polygon();
  fl_vertex (X, Y+H);
  fl_vertex (x(.18), Y+H);
  for (int i=nano_min ; i<=(int)(nano_max*.38+0.5); i++)
    fl_vertex( x(x_xyY), y(y_xyY) );
  fl_vertex (X, y(.25));
  fl_end_polygon();
  #ifdef DEBUG_DRAW
  fl_color(FL_WHITE);
  #endif
  fl_begin_polygon();
  fl_vertex (X, y(.25));
  for (int i=(int)(nano_max*.38+0.5) ; i<=(int)(nano_max*.45+0.5); i++)
    fl_vertex( x(x_xyY), y(y_xyY) );
  fl_vertex (x(.065), Y);
  fl_vertex (X, Y);
  fl_end_polygon();
  #ifdef DEBUG_DRAW
  fl_color(FL_YELLOW);
  #endif
  fl_begin_polygon();
  {
  int i = (int)(nano_max*.457+0.5)-2;
  fl_vertex( x(x_xyY), y(y_xyY) );
  i = (int)(nano_max*.457+0.5)-1;
  fl_vertex( x(x_xyY), y(y_xyY) );
  fl_vertex (x(.1), Y);
  fl_vertex (x(.065), Y);
  for (i=(int)(nano_max*.45+0.5); i<=(int)(nano_max*.457+0.5)-3; i++) {
    fl_vertex( x(x_xyY), y(y_xyY) );
    }
  fl_end_polygon();
  fl_begin_polygon();
  i = (int)(nano_max*.457+0.5)-1;
  fl_vertex( x(x_xyY), y(y_xyY) );
  i = (int)(nano_max*.457+0.5);
  fl_vertex( x(x_xyY), y(y_xyY) );
  fl_vertex (x(.1), Y);
  }
  fl_end_polygon();
  #ifdef DEBUG_DRAW
  fl_color(FL_BLUE);
  #endif
  fl_begin_polygon();
  fl_vertex (x(.1), Y);
  for (int i=(int)(nano_max*.457+0.5) ; i<=(int)(nano_max*.48+0.5); i++)
    fl_vertex( x(x_xyY), y(y_xyY) );
  fl_vertex (x(.2), Y);
  fl_end_polygon();
  #ifdef DEBUG_DRAW
  fl_color(FL_RED);
  #endif
  fl_begin_polygon();
  fl_vertex (X+W, Y);
  fl_vertex (x(0.2), Y);
  for (int i=(int)(nano_max*.48+0.5); i<=(int)(nano_max*.6+0.5); i++)
    fl_vertex( x(x_xyY), y(y_xyY) );
  fl_end_polygon();
  #ifdef DEBUG_DRAW
  fl_color(FL_GREEN);
  #endif
  fl_begin_polygon();
  fl_vertex (X+W, Y);
  for (int i=(int)(nano_max*.6+0.5); i<=(int)(nano_max+0.5) ; i++)
    fl_vertex( x(x_xyY), y(y_xyY) );
  fl_vertex( x(cieXYZ[nano_min][0]/(cieXYZ[nano_min][0]+cieXYZ[nano_min][1]+cieXYZ[nano_min][2])),
             y(cieXYZ[nano_min][1]/(cieXYZ[nano_min][0]+cieXYZ[nano_min][1]+cieXYZ[nano_min][2])) );
  fl_vertex (x(.18), Y+H);
  fl_vertex (X+W, Y+H);
  fl_end_polygon();

  //fl_pop_clip();


  // Diagramm
  fl_color(VG);
    // Text
  fl_font (FL_HELVETICA, 10);
    // Raster
  for (float f=0 ; f<n ; f+=0.1) {
    static char text[64];
    fl_line ( x(f),(int)(yO+5), x(f),y(n));
    fl_line ( (int)(xO-5),y(f), x(n),y(f));
    sprintf ( text, "%.1f", f);
    fl_draw ( text, (int)(xO-30), y(f)+4 );
    fl_draw ( text, x(f)-7, (int)(yO+20) );
  }
  

  { // Primärfarben / Weisspunkt
    register char RGB[3];
    register cmsCIEXYZ XYZ;
    std::vector<double> pos;
    for (unsigned int i = 0; i < texte.size(); i++) {
        double _XYZ[3] = {punkte[i*3+0], punkte[i*3+1], punkte[i*3+2]};
        double* xyY = XYZto_xyY ( _XYZ );
        pos.push_back ( x (xyY[0]) );
        pos.push_back ( y (xyY[1]) );
        #ifdef DEBUG_DRAW
        cout << texte[i] << " " << punkte.size(); DBG
        #endif
    }

    if (texte[0] != "wtpt") { // markiert den Weisspunkt nur
      double* xyY = XYZto_xyY ( profile[0].getWhitePkt() );
      int g = 2;

      fl_color (FL_WHITE);
      fl_line ( x(xyY[0])-g, y(xyY[1])-g, x(xyY[0])+g, y(xyY[1])+g );
      fl_line ( x(xyY[0])-g, y(xyY[1])+g, x(xyY[0])+g, y(xyY[1])-g );
    }

    fl_color(BG);
    if (punkte.size() == 9) {
        for (int k = 0; k <= 3; k+=2) {
            fl_line( (int)(pos[k+0]), (int)(pos[k+1]),
                     (int)(pos[k+2]), (int)(pos[k+3]));
            #ifdef DEBUG_DRAW
            cout << "Linie "; DBG
            #endif
        }
        fl_line( (int)(pos[0]), (int)(pos[1]),
                 (int)(pos[4]), (int)(pos[5]));
        #ifdef DEBUG_DRAW
        cout << "Linie "; DBG
        #endif
    }

    int j = 0;
    for (unsigned int i = 0; i < texte.size(); i++) {
        #ifdef DEBUG_DRAW
        cout << punkte[j] << " ";
        #endif
        XYZ.X = punkte[j++]; 
        #ifdef DEBUG_DRAW
        cout << punkte[j] << " ";
        #endif
        XYZ.Y = punkte[j++];
        #ifdef DEBUG_DRAW
        cout << punkte[j] << " " << texte[i] << " " << punkte.size(); DBG
        #endif
        XYZ.Z = punkte[j++]; //1 - ( punkte[i][0] +  punkte[i][1] );

        // Farbe für Darstellung konvertieren (lcms)
        cmsDoTransform (xform, &XYZ, RGB, 1);

        double _XYZ[3] = {XYZ.X, XYZ.Y, XYZ.Z};
        double* xyY = XYZto_xyY ( _XYZ );
        double pos_x = x(xyY[0]);
        double pos_y = y(xyY[1]);

        fl_color (BG);
        fl_circle ( pos_x , pos_y , 9.0);
        fl_color (fl_rgb_color (RGB[0],RGB[1],RGB[2]));
        fl_circle ( pos_x , pos_y , 7.0);
        // etwas Erklärung zu den Farbpunkten
        fl_font (FL_HELVETICA, 12);
        std::stringstream s;
        std::stringstream t;
        // lcms hilft bei Weisspunkbeschreibung aus
        if (texte[i] == "wtpt") {
          static char txt[1024] = {'\000'};
          _cmsIdentifyWhitePoint (&txt[0], &XYZ);
          t << " (" << &txt[12] << ")";
        }
          
        s << texte[i] << t.str() << " = " << _XYZ[0] <<", "<< _XYZ[1] <<", "<< _XYZ[2];
        int _w = 0, _h = 0;
        // Text einpassen
        fl_measure (s.str().c_str(), _w, _h, 1);
        fl_color(FL_WHITE);
        fl_draw ( s.str().c_str(),
                  (int)(pos_x +9 + _w > x(n) ? x(n) - _w : pos_x +9), 
                  (int)(pos_y -9 - _h < y(n) ? y(n) + _h : pos_y -9)  );
        i++;
      }
  }

  fl_pop_clip();
  DBG_prog_ende
}

void draw_kurve    (int X, int Y, int W, int H,
                    std::vector<std::string> texte,
                    std::vector<std::vector<double> > kurven)
{ DBG_prog_start
  // Zeichenflaeche
  fl_color(BG);
  fl_rectf(X,Y,W,H);

  // Diagrammvariablen
  n = 1.0;                        // maximale Hoehe 

  xO = X + tab_border_x + 10;     // Ursprung
  yO = Y + H - tab_border_y - 10; // Ursprung
  w  = (W - 2*tab_border_x);      // Breite des Diagrammes
  h  = (H - 2*tab_border_y);      // Hoehe des Diagrammes

  fl_push_clip( X,y(n), x(n),(int)(h+tab_border_y+0.5) );

  // Tangente
  fl_color(DIAG);
  fl_line(x(0), y(0), x(1), y(1));

  // Diagramm
  fl_color(VG);
    // Text
  fl_font (FL_HELVETICA, 10);
    // Raster
  for (float f=0 ; f<=n ; f+=0.1) {
    static char text[64];
    fl_line ( x(f),(int)(yO+5), x(f),y(n));
    fl_line ( (int)(xO-5),y(f), x(n),y(f));
    sprintf ( text, "%.1f", f);
    fl_draw ( text, (int)(xO-30), y(f)+4 );
    fl_draw ( text, x(f)-7, (int)(yO+20) );
  }

  // Kurve
  fl_font ( FL_HELVETICA, 12) ;
  std::stringstream s ;
  std::string name;
  bool ist_kurve = false;
  icColorSpaceSignature icc_colour_space_signature = icSigLabData;
  int* flFarben = getChannel_flColours (icc_colour_space_signature);
  for (unsigned int j = 0; j < kurven.size(); j++) {
    if (kurven.size() <= texte.size())
      name = texte[j];
    else
      name = _("unbekannte Farbe");
    if (kurven.size() < texte.size()
     && texte[texte.size()-1] == "curv")
      ist_kurve = true;

    fl_color( flFarben[j] );
    fl_color(FL_LIGHT2);
    if        (name == "rTRC") {
      fl_color(FL_RED);
      name = _("Rot");
    } else if (name == "gTRC") {
      fl_color(FL_GREEN);
      name = _("Grün");
    } else if (name == "bTRC") {
      fl_color(FL_BLUE);
      name = _("Blau");
    } else if (name == "kTRC"
            || name == "bdf") {
      fl_color(FL_LIGHT2);
      name = _("Grau");
    } else if (name == _("Rot")) {
      fl_color(FL_RED);
    } else if (name == _("Grün")) {
      fl_color(FL_GREEN);
    } else if (name == _("Blau")) {
      fl_color(FL_BLUE);
    } else if (name == _("Cyan")) {
      fl_color(FL_CYAN);
    } else if (name == _("Magenta")) {
      fl_color(FL_MAGENTA);
    } else if (name == _("Gelb")) {
      fl_color(FL_YELLOW);
    } else if (name == _("Schwarz")) {
      fl_color(FL_BLACK);
    } else if (name == _("Leuchtdichte")) {
      fl_color(FL_WHITE);
    } else if (name == _("Leuchtdichte Y")) {
      fl_color(FL_WHITE);
    } else if (name == _("CIE X")) {
      fl_color(FL_RED);
    } else if (name == _("CIE Y (Leuchtdichte)")) {
      fl_color(FL_GREEN);
    } else if (name == _("CIE Z")) {
      fl_color(FL_BLUE);
    } else if (name == _("Helligkeit")) {
      fl_color(FL_WHITE);
    } else if (name == _("Wert")) {
      fl_color(FL_WHITE);
    } else if (name == _("CIE *L")) {
      fl_color(FL_WHITE);
    } else if (name == _("CIE *a")) {
      fl_color(FL_RED);
    } else if (name == _("CIE *b")) {
      fl_color(FL_BLUE);
    } else if (name == _("Farbanteil b")) {
      fl_color(FL_BLUE);
    } else if (name == _("Farbanteil r")) {
      fl_color(FL_RED);
    } else {
      fl_color(9 + j);
    }
    #ifdef DEBUG//_DRAW
    cout << "Zeichne Kurve "<< name << " " << j << " " << kurven[j].size() << " Teile "; DBG
    #endif
    s.str("");
    if (kurven[j].size() == 0
     && ist_kurve) {
      fl_line (x( 0 ), y( 0 ), x( 1 ), y( 1 ) );
      // Infos einblenden 
      s << name << _(" mit Gamma: 1.0");
      fl_draw ( s.str().c_str(), x(0) + 2, y(n) + j*16 + 12);
    // parametrischer Eintrag
    } else if (kurven[j].size() == 1
            && ist_kurve) {
      int segmente = 256;
      double gamma = kurven[j][0]; DBG_V( gamma )
      for (int i = 1; i < segmente; i++)
        fl_line (x( pow( (double)(i-1.0)/segmente, gamma ) ),
                 y( (i-1) / ((segmente-1) *n) ),
                 x( pow( (double)i/segmente, gamma ) ),
                 y( (i) / ((segmente-1) *n) ) );
      // Infos einblenden 
      s << name << _(" mit einem Eintrag für Gamma: ") << gamma; DBG_V( gamma )
      fl_draw ( s.str().c_str(), x(0) + 2, y(n) + j*16 + 12);
    // parametrischer Eintrag mit Wert für Minimum und Maximum 
    } else if (kurven[j].size() == 3
            && texte[texte.size()-1] == "gamma_start_ende") {
      int segmente = 256;
      double gamma = kurven[j][0]; DBG_V( gamma )
      double start = kurven[j][1]; DBG_V( start )
      double ende  = kurven[j][2]; DBG_V( ende )
      double mult  = (ende - start); DBG_V( mult )
      for (int i = 1; i < segmente; i++) {
        fl_line (x( pow( (double)(i-1.0)/segmente, gamma ) * mult + start ),
                 y( (i-1) / ((segmente-1) *n) ),
                 x( pow( (double)i/segmente, gamma ) * mult + start),
                 y( (i) / ((segmente-1) *n) ) );
      }
      // Infos einblenden 
      s << name << _(" mit einem Eintrag für Gamma: ") << gamma;
      fl_draw ( s.str().c_str(), x(0) + 2, y(n) + j*16 + 12);
    // segmentierte Kurve
    } else {
      for (unsigned int i = 1; i < kurven[j].size(); i++) {
        fl_line (x( kurven[j][i-1] ),
                 y( (i-1) / ((kurven[j].size()-1) *n) ),
                 x( kurven[j][i] ),
                 y( (i) / ((kurven[j].size()-1) *n) ) );
      }
      // Infos einblenden 
      s << name << _(" mit ") << kurven[j].size() << _(" Punkten");
      fl_draw ( s.str().c_str(), x(0) + 2, y(n) + j*16 + 12);
    }
  }
  
  fl_pop_clip();
  DBG_prog_ende
}

#if 0
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

#endif
