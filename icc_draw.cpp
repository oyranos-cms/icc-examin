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
 * drawing routines for diagrams
 * 
 */

//#define DEBUG_DRAW

#include "icc_utils.h"
#include "icc_helfer.h"
#include "icc_profile.h"
#include "icc_helfer_ui.h"
#include "icc_info.h"
#include "icc_kette.h"
#include "icc_oyranos.h"

#include <stdio.h>
#include <iostream>
#include <vector>
#include <string.h>
#include <FL/Fl.H>
#include <FL/Fl_Widget.H>
#include <FL/fl_draw.H>

#include <lcms.h>
#include "cccie64.h"
#include "ciexyz64_1.h"

#ifdef HAVE_FLTK
#include "icc_draw_fltk.h"
#endif

#include "icc_draw.h"

// internal functions


#ifdef DEBUG_DRAW
# define DBG_prog_start DBG_PROG_START
# define DBG_prog_ende DBG_PROG_ENDE
#else
# define DBG_prog_start
# define DBG_prog_ende
#endif


TagDrawings::TagDrawings (int X,int Y,int W,int H, const char* l)
  : Fl_Widget(X,Y,W,H,l)
{
  DBG_PROG_START
  // drawing area variables
  tab_rand_x=20;
  tab_rand_y=20;
  linker_text_rand  = 25;
  unterer_text_rand = 17;
  raster_abstand = 35;
  zeige_raster = true;
  // value range variables
  min_x = min_y = 0.0;
  max_x = max_y = 1.0;

  kurve_umkehren = false;
  zeichne_symbole = true;
  zeichne_linie = true;
  ursprung_zeichnen = true;

  raster = 4;
  init_s = FALSE;
  rechenzeit = 0.1;
  hXYZ = hsRGB = 0;
  xform = 0;
  RGB_speicher = 0;
  XYZ_speicher = 0;
  n_speicher = 0;
  DBG_PROG_ENDE
}

TagDrawings::~TagDrawings ()
{
  DBG_PROG_START
  if(hXYZ) cmsCloseProfile(hXYZ);
  if(hsRGB) cmsCloseProfile(hsRGB);
  if(xform) cmsDeleteTransform(xform);
  if(RGB_speicher) delete [] RGB_speicher;
  if(XYZ_speicher) delete [] XYZ_speicher;

  DBG_PROG_ENDE
}

void
TagDrawings::hineinPunkt ( std::vector<double>      &vect,
                           std::vector<std::string> &txt )
{
  DBG_PROG_START
  clear();
  punkte = vect;
  texte = txt;

  // displayed area 
  min_x = min_y = 0.0;
  max_x = max_y = .85;

  // show CIExyY from tag_browser

  wiederholen = false;
  DBG_PROG_ENDE
}

#include <limits>

void
TagDrawings::hineinDaten (
                     std::vector<std::vector<std::pair<double,double> > > &vect,
                     std::vector<std::string> &txt )
{
  DBG_PROG_START
  clear();
  kurven2 = vect;
  texte = txt;

  std::numeric_limits<double> l;
  max_x = max_y = l.min();
  min_x = min_y = l.max();

  for(unsigned int i = 0; i < kurven2.size(); ++i)
    for(unsigned int j = 0; j < kurven2[i].size(); ++j) {
      if(kurven2[i][j].first > max_x)  max_x = kurven2[i][j].first;
      if(kurven2[i][j].second > max_y) max_y = kurven2[i][j].second;
      if(kurven2[i][j].first <  min_x) min_x = kurven2[i][j].first;
      if(kurven2[i][j].second < min_y) min_y = kurven2[i][j].second;
    }

  wiederholen = false;

  DBG_PROG_ENDE
}

void
TagDrawings::hineinKurven ( std::vector<std::vector<double> > &vect,
                            std::vector<std::string> &txt)
{
  DBG_PROG_START
  clear();
  kurven = vect;
  texte = txt;
  kurve_umkehren = false;

  min_x = min_y = 0.0;
  max_x = max_y = 1.0;

  //show curve from tag_browser

  wiederholen = false;

  DBG_PROG_ENDE
}

void
TagDrawings::ruhigNeuzeichnen (void)
{
  DBG_PROG_START
  drawCieShoe_ (true);
  DBG_PROG_ENDE
}



void
TagDrawings::init_shoe_ ()
{
  // initialisation for lcms
  hXYZ  = cmsCreateXYZProfile();

  size_t groesse = 0;
  const char* block = 0;
  block = icc_oyranos.moni(0,0, groesse);
  if(groesse &&
     icc_debug != 14) {
    hsRGB = cmsOpenProfileFromMem(const_cast <char*> (block), (DWORD)groesse);
  } else {
    hsRGB = cmsCreate_sRGBProfile();
  }

  xform = cmsCreateTransform              (hXYZ, TYPE_XYZ_DBL,
                                           hsRGB, TYPE_RGB_8,
                                           INTENT_ABSOLUTE_COLORIMETRIC,
                                           cmsFLAGS_NOTPRECALC);
}

void
TagDrawings::draw ()
{
  DBG_PROG_START
  // draw curves or points
  if (icc_examin_ns::laeuft())
  {
    // diagramvariables in image points
    xO = (float)(x() +       tab_rand_x + linker_text_rand);     // origin
    yO = (float)(y() + h() - tab_rand_y - unterer_text_rand);    // origin
    breite  = (float)(w() - 2*tab_rand_x - linker_text_rand);  // width of diagram
    float hoehe_   = (float)(h() - 2*tab_rand_y - unterer_text_rand); // height of diagram
    hoehe   = MAX( 0, hoehe_ );

    DBG_PROG_S( kurven.size() <<" "<< punkte.size() )
    
    if (kurven.size() || kurven2.size())
    { DBG_PROG
      wiederholen = false;
      drawKurve_ ();
    } else if (punkte.size()) {
      if (wiederholen)
      { drawCieShoe_ (false);
        Fl::add_timeout( 1.2, /*(void(*)(void*))*/dHaendler ,(void*)this);
      } else {
        drawCieShoe_ (true);
      }
      wiederholen = true; 
    }
  } else
    DBG_PROG_S( __func__ << _(" used too early!") );
  DBG_PROG_ENDE
}

void
TagDrawings::drawCieShoe_ ( int repeated)
{ DBG_prog_start

  //TODO -> icc_oyranos
  if (!init_s)
    init_shoe_();
  init_s = TRUE;

  double rz = (double)clock()/(double)CLOCKS_PER_SEC;
  int raster_alt = raster;
  if (repeated)
    raster = 1;

  // drawing area
  fl_color(BG);
  fl_rectf(x(),y(),w(),h());


  fl_push_clip( x(),yNachBild(max_x), xNachBild(max_x),(int)(hoehe+tab_rand_y+0.5) );

  // spectal variables
  int nano_min = 63; // 420 nm
  int nano_max = 341; // 700 nm

  // tangent
  fl_color(DIAG);
  fl_line(xNachBild(1), yNachBild(0), xNachBild(0), yNachBild(1));

  // colour area
  if (!repeated)
  {
    register char RGB[3];
    register cmsCIEXYZ XYZ;

    for (int cie_y=yNachBild(0.01) ; cie_y > yNachBild(0.85); cie_y -= raster)
      for (int cie_x=xNachBild(0) ; cie_x < xNachBild(0.73) ; cie_x += raster)
      {
        XYZ.X = bildNachX(cie_x);
        XYZ.Y = bildNachY(cie_y);
        XYZ.Z = 1 - (XYZ.X +  XYZ.Y);

        // draw background (lcms)
        cmsDoTransform(xform, &XYZ, RGB, 1);

        fl_color (fl_rgb_color (RGB[0],RGB[1],RGB[2]));
        if (raster > 1)
          fl_rectf (cie_x , cie_y, raster,raster);
        else
          fl_point (cie_x , cie_y);
      }
  } else {
    int wi = xNachBild(0.73)-xNachBild(0);
    int hi = yNachBild(0.01)-yNachBild(0.85);

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

    for (int cie_y=yNachBild(0.85) ; cie_y < yNachBild(0.01) ; cie_y ++) {
      for (int cie_x=xNachBild(0) ; cie_x < xNachBild(0.73) ; cie_x ++) {

        XYZ_speicher[i].X = bildNachX(cie_x);
        XYZ_speicher[i].Y = bildNachY(cie_y);
        XYZ_speicher[i].Z = 1 - (XYZ_speicher[i].X +  XYZ_speicher[i].Y);
        i++;
      }
    }
    // draw background (lcms)
    cmsDoTransform(xform, XYZ_speicher, RGB_speicher, n_pixel);
    fl_draw_image(RGB_speicher, xNachBild(min_x), yNachBild(0.85), wi, hi, 3, 0);
  }

  // detect time of refresh
  if (!repeated) {
    rz = (double)clock()/(double)CLOCKS_PER_SEC - rz;
    rechenzeit += rz;
    //cout << rechenzeit << " Sekunden  "; DBG_PROG
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

  // hide of remainder of cie_xy - terrible code
  //fl_push_no_clip();
  fl_color(BG);
# define x_xyY cieXYZ[i][0]/(cieXYZ[i][0]+cieXYZ[i][1]+cieXYZ[i][2])
# define y_xyY cieXYZ[i][1]/(cieXYZ[i][0]+cieXYZ[i][1]+cieXYZ[i][2])


  fl_begin_polygon();
  fl_vertex (x(), y()+h());
  fl_vertex (xNachBild(.18), y()+h());
  for (int i=nano_min ; i<=(int)(nano_max*.38+0.5); i++)
    fl_vertex( xNachBild(x_xyY), yNachBild(y_xyY) );
  fl_vertex (x(), yNachBild(.25));
  fl_end_polygon();
# ifdef DEBUG_DRAW
  fl_color(FL_WHITE);
# endif
  fl_begin_polygon();
  fl_vertex (x(), yNachBild(.25));
  for (int i=(int)(nano_max*.38+0.5) ; i<=(int)(nano_max*.45+0.5); i++)
    fl_vertex( xNachBild(x_xyY), yNachBild(y_xyY) );
  fl_vertex (xNachBild(.065), y());
  fl_vertex (x(), y());
  fl_end_polygon();
# ifdef DEBUG_DRAW
  fl_color(FL_YELLOW);
# endif
  fl_begin_polygon();
  {
  int i = (int)(nano_max*.457+0.5)-2;
  fl_vertex( xNachBild(x_xyY), yNachBild(y_xyY) );
  i = (int)(nano_max*.457+0.5)-1;
  fl_vertex( xNachBild(x_xyY), yNachBild(y_xyY) );
  fl_vertex (xNachBild(.1), y());
  fl_vertex (xNachBild(.065), y());
  for (i=(int)(nano_max*.45+0.5); i<=(int)(nano_max*.457+0.5)-3; i++) {
    fl_vertex( xNachBild(x_xyY), yNachBild(y_xyY) );
    }
  fl_end_polygon();
  fl_begin_polygon();
  i = (int)(nano_max*.457+0.5)-1;
  fl_vertex( xNachBild(x_xyY), yNachBild(y_xyY) );
  i = (int)(nano_max*.457+0.5);
  fl_vertex( xNachBild(x_xyY), yNachBild(y_xyY) );
  fl_vertex (xNachBild(.1), y());
  }
  fl_end_polygon();
# ifdef DEBUG_DRAW
  fl_color(FL_BLUE);
# endif
  fl_begin_polygon();
  fl_vertex (xNachBild(.1), y());
  for (int i=(int)(nano_max*.457+0.5) ; i<=(int)(nano_max*.48+0.5); i++)
    fl_vertex( xNachBild(x_xyY), yNachBild(y_xyY) );
  fl_vertex (xNachBild(.2), y());
  fl_end_polygon();
# ifdef DEBUG_DRAW
  fl_color(FL_RED);
# endif
  fl_begin_polygon();
  fl_vertex (x()+w(), y());
  fl_vertex (xNachBild(0.2), y());
  for (int i=(int)(nano_max*.48+0.5); i<=(int)(nano_max*.6+0.5); i++)
    fl_vertex( xNachBild(x_xyY), yNachBild(y_xyY) );
  fl_end_polygon();
# ifdef DEBUG_DRAW
  fl_color(FL_GREEN);
# endif
  fl_begin_polygon();
  fl_vertex (x()+w(), y());
  for (int i=(int)(nano_max*.6+0.5); i<=(int)(nano_max+0.5) ; i++)
    fl_vertex( xNachBild(x_xyY), yNachBild(y_xyY) );
  fl_vertex( xNachBild(cieXYZ[nano_min][0]/(cieXYZ[nano_min][0]+cieXYZ[nano_min][1]+cieXYZ[nano_min][2])),
             yNachBild(cieXYZ[nano_min][1]/(cieXYZ[nano_min][0]+cieXYZ[nano_min][1]+cieXYZ[nano_min][2])) );
  fl_vertex (xNachBild(.18), y()+h());
  fl_vertex (x()+w(), y()+h());
  fl_end_polygon();

# undef x_xyY
# undef y_xyY

  fl_pop_clip();

  if(icc_debug != 14)
  {

    // diagram
    fl_color(VG);

    // raster
    zeichneRaster_ ();

    fl_push_clip( x(),yNachBild(max_x), xNachBild(max_x),(int)(hoehe+tab_rand_y+0.5) );

    // Primaries / white point
    register char RGB[3];
    register cmsCIEXYZ XYZ;
    std::vector<double> pos;
    for (unsigned int i = 0; i < punkte.size()/3; i++) {
        double _XYZ[3] = {punkte[i*3+0], punkte[i*3+1], punkte[i*3+2]};
        const double* xyY = XYZto_xyY ( _XYZ );
        pos.push_back ( xNachBild (xyY[0]) );
        pos.push_back ( yNachBild (xyY[1]) );
#       ifdef DEBUG_DRAW
        cout << texte[i] << " " << punkte.size(); DBG_PROG
#       endif
    }

    if (texte[0] != "wtpt") { // mark the white point only
      if(!profile.profil())
        return;
      std::vector<double> xyY = profile.profil()->getWhitePkt();
      XYZto_xyY ( xyY );
      int g = 2;

      fl_color (FL_WHITE);
      fl_line ( xNachBild(xyY[0])-g, yNachBild(xyY[1])-g,
                xNachBild(xyY[0])+g, yNachBild(xyY[1])+g );
      fl_line ( xNachBild(xyY[0])-g, yNachBild(xyY[1])+g,
                xNachBild(xyY[0])+g, yNachBild(xyY[1])-g );
    }

    fl_color(BG);
    if (punkte.size() == 9) {
        for (int k = 0; k <= 3; k+=2) {
            fl_line( (int)(pos[k+0]), (int)(pos[k+1]),
                     (int)(pos[k+2]), (int)(pos[k+3]));
#           ifdef DEBUG_DRAW
            cout << "Linie "; DBG_PROG
#           endif
        }
        fl_line( (int)(pos[0]), (int)(pos[1]),
                 (int)(pos[4]), (int)(pos[5]));
#       ifdef DEBUG_DRAW
        cout << "Linie "; DBG_PROG
#       endif
    }
    DBG_PROG_V( punkte.size() )
    unsigned int i;
    for (i = 0; i < punkte.size()/3; i++)
    {
        DBG_PROG_V( i )
#       ifdef DEBUG_DRAW
        cout << punkte[i*3+0] << " ";
#       endif
        XYZ.X = punkte[i*3+0]; 
#       ifdef DEBUG_DRAW
        cout << punkte[i*3+1] << " ";
#       endif
        XYZ.Y = punkte[i*3+1];
#       ifdef DEBUG_DRAW
        cout << punkte[i*3+2] << " " << texte[i] << " " << punkte.size(); DBG_PROG
#       endif
        XYZ.Z = punkte[i*3+2]; //1 - ( punkte[i][0] +  punkte[i][1] );

        // convert colour for displaying (lcms)
        cmsDoTransform (xform, &XYZ, RGB, 1);

        double _XYZ[3] = {XYZ.X, XYZ.Y, XYZ.Z};
        const double* xyY = XYZto_xyY ( _XYZ );
        double pos_x = xNachBild(xyY[0]);
        double pos_y = yNachBild(xyY[1]);

        fl_color (BG);
        fl_circle ( pos_x , pos_y , 9.0);
        fl_color (fl_rgb_color (RGB[0],RGB[1],RGB[2]));
        fl_circle ( pos_x , pos_y , 7.0);
        // some description for the colour points
        fl_font (FL_HELVETICA, 12);
        std::stringstream s;
        std::stringstream t;
        // lcms helps with wither point description
        if (texte.size()>i)
        { if (texte[i] == "wtpt") {
            static char txt[1024] = {'\000'};
            _cmsIdentifyWhitePoint (&txt[0], &XYZ);
            t << " (" << &txt[12] << ")";
          }
          
          s << texte[i] << t.str() << " = " <<
               _XYZ[0] <<", "<< _XYZ[1] <<", "<< _XYZ[2];
          int _w = 0, _h = 0;
          // fit in the text
          fl_measure (s.str().c_str(), _w, _h, 1);
          fl_color(FL_WHITE);//FOREGROUND_COLOR);
          fl_draw ( s.str().c_str(),
                    (int)(pos_x +9 + _w > xNachBild(max_x) ?
                          xNachBild(max_x) - _w : pos_x +9), 
                    (int)(pos_y -9 - _h < yNachBild(max_x) ?
                          yNachBild(max_x) + _h : pos_y -9)  );
        }
    }
    int j = 0;
    for( ; i < texte.size()-1; ++i) {
      fl_draw ( texte[i].c_str(), xNachBild(min_x) + 2, yNachBild(max_y) + j*16 +12);
      ++j;
    }
  }

  fl_pop_clip();
  DBG_prog_ende
}

void
TagDrawings::zeichneRaster_ ()
{
  
  raster_wert_x_ = (max_x-min_x) * raster_abstand / (double)breite;
  raster_wert_y_ = (max_y-min_y) * raster_abstand / (double)hoehe;

  // text
  fl_font (FL_HELVETICA, 10);

  // TODO round

  for (double f=min_x ; f <= max_x ; f += raster_wert_x_)
  {
    static char text[64];
    // raster lines
    if(zeige_raster)
      fl_line ( xNachBild(f),(int)(yO+5), xNachBild(f),yNachBild(max_y));
    else
      fl_line ( xNachBild(f),(int)(yO+5), xNachBild(f),yNachBild(min_y));

    // values
    if(raster_wert_x_ < 0.01)
      sprintf ( text, "%.3f", f);
    else if(raster_wert_x_ < 0.1)
      sprintf ( text, "%.2f", f);
    else if(raster_wert_x_ < 1)
      sprintf ( text, "%.1f", f);
    else
      sprintf ( text, "%.f", f);
    fl_draw ( text, xNachBild(f)-7, (int)(yO+20) );
  }
  for (double f=min_y ; f <= max_y ; f += raster_wert_y_)
  {
    static char text[64];
    // raster lines
    if(zeige_raster)
      fl_line ( (int)(xO-5),yNachBild(f), xNachBild(max_x),yNachBild(f));
    else
      fl_line ( (int)(xO-5),yNachBild(f), xNachBild(min_x),yNachBild(f));

    // values
    if(raster_wert_y_ < 0.01)
      sprintf ( text, "%.3f", f);
    else if(raster_wert_y_ < 0.1)
      sprintf ( text, "%.2f", f);
    else if(raster_wert_y_ < 1)
      sprintf ( text, "%.1f", f);
    else
      sprintf ( text, "%.f", f);
    fl_draw ( text, (int)(xO-30), yNachBild(f)+4 );
  }
  fl_color(FL_WHITE);
  if (ursprung_zeichnen)
  {
    if (min_x < 0)
      fl_line ( xNachBild(0),(int)(yO+5), xNachBild(0),yNachBild(max_y));
    if (min_y < 0)
      fl_line ( (int)(xO-5),yNachBild(0), xNachBild(max_x),yNachBild(0));
  }
  fl_color(VG);
}

void
TagDrawings::drawKurve_    ()
{ DBG_prog_start

  // drawing area
  fl_color(BG);
  fl_rectf(x(),y(),w(),h());

  // diagram
  fl_color(VG);
  zeichneRaster_ ();

  // drawing area
  fl_push_clip( x(),yNachBild(max_y), xNachBild(max_x),(int)(hoehe+tab_rand_y+0.5) );

  // tangent
  fl_color(DIAG);
  fl_line( xNachBild(min_x), yNachBild(min_y), xNachBild(max_x), yNachBild(max_y) );


  // curv
  fl_font ( FL_HELVETICA, 12) ;
  std::stringstream s ;
  std::string name;
  bool ist_kurve = false;
  icColorSpaceSignature icc_colour_space_signature = icSigLabData;
  int* flFarben = getChannel_flColours (icc_colour_space_signature);
  unsigned kurven_n;
  if(kurven.size()) kurven_n = (unsigned)kurven.size();
  else              kurven_n = (unsigned)kurven2.size();

  static const char symbol_[][4] = {"*","o","#","x","$","s"};
  const int sym_n=sizeof(symbol_)/4; DBG_PROG_V( sym_n )
  int symbol_n_ = 0;
  for (unsigned int j = 0; j < kurven_n; j++) {
    if (kurven_n <= texte.size())
      name = texte[j];
    else
      name = _("unknown Colour");
    if (kurven_n < texte.size()
     && texte[texte.size()-1] == "curv")
      ist_kurve = true;

    fl_color( flFarben[j] );
    fl_color(FL_LIGHT2);
    if        (name == "rTRC") {
      fl_color(FL_RED);
      name = _("Red");
    } else if (name == "gTRC") {
      fl_color(FL_GREEN);
      name = _("Green");
    } else if (name == "bTRC") {
      fl_color(FL_BLUE);
      name = _("Blue");
    } else if (name == "kTRC"
            || name == "bdf") {
      fl_color(FL_LIGHT2);
      name = _("Gray");
    } else if (name == _("Red")) {
      fl_color(FL_RED);
    } else if (name == _("Green")) {
      fl_color(FL_GREEN);
    } else if (name == _("Blue")) {
      fl_color(FL_BLUE);
    } else if (name == _("Cyan")) {
      fl_color(FL_CYAN);
    } else if (name == _("Magenta")) {
      fl_color(FL_MAGENTA);
    } else if (name == _("Yellow")) {
      fl_color(FL_YELLOW);
    } else if (name == _("Black")) {
      fl_color(FL_BLACK);
    } else if (name == _("Luminance")) {
      fl_color(FL_WHITE);
    } else if (name == _("Luminance Y")) {
      fl_color(FL_WHITE);
    } else if (name == _("CIE X")) {
      fl_color(FL_RED);
    } else if (name == _("CIE Y (Luminance)")) {
      fl_color(FL_GREEN);
    } else if (name == _("CIE Z")) {
      fl_color(FL_BLUE);
    } else if (name == _("Lightness")) {
      fl_color(FL_WHITE);
    } else if (name == _("Value")) {
      fl_color(FL_WHITE);
    } else if (name == _("CIE *L")) {
      fl_color(FL_WHITE);
    } else if (name == _("CIE *a")) {
      fl_color(FL_RED);
    } else if (name == _("CIE *b")) {
      fl_color(FL_BLUE);
    } else if (name == _("Colour b")) {
      fl_color(FL_BLUE);
    } else if (name == _("Colour r")) {
      fl_color(FL_RED);
    } else {
      fl_color(9 + j);
    }
    if(kurven.size())
      DBG_PROG_S( cout << "draw curv "<< name << " " << j << " " << kurven[j].size() << " parts " );
    s.str("");
    if (kurven2.size())
    {
      if (symbol_n_ >= sym_n) symbol_n_ = 0;
      int korr_x, korr_y;
      fl_measure(symbol_[symbol_n_],  korr_x,  korr_y, 1);
      korr_x = (int)(korr_x* -1./6. +.5);
      korr_y = (int)(korr_y*1./4. +.5);
      for (unsigned int i = 0; i < kurven2[j].size(); i++)
        if( zeichne_symbole ) {
          if( kurve_umkehren )
            fl_draw (symbol_[symbol_n_], xNachBild(kurven2[j][i].first)+korr_x,
                     yNachBild( kurven2[j][i].second)+korr_y );
          else
            fl_draw (symbol_[symbol_n_], xNachBild(kurven2[j][i].second)+korr_x,
                     yNachBild( kurven2[j][i].first)+korr_y );
        }
      
      for (unsigned int i = 1; i < kurven2[j].size(); i++) {
        if( zeichne_linie ) {
          if( kurve_umkehren )
            fl_line (xNachBild( kurven2[j][i-1].first),
                     yNachBild( kurven2[j][i-1].second),
                     xNachBild( kurven2[j][i].first),
                     yNachBild( kurven2[j][i].second) );
          else
            fl_line (xNachBild( kurven2[j][i-1].second),
                     yNachBild( kurven2[j][i-1].first),
                     xNachBild( kurven2[j][i].second),
                     yNachBild( kurven2[j][i].first) );
        }
      }
      s << name << _(" with ") << kurven2[j].size() << _(" points")<<": "<<symbol_[symbol_n_];
      fl_draw ( s.str().c_str(), xNachBild(min_x) + 2, yNachBild(max_y) + j*16 + 12);
      ++symbol_n_;
    } else if (kurven[j].size() == 0 &&
               ist_kurve) {
      fl_line (xNachBild( min_x ), yNachBild( min_y ), xNachBild( max_x ), yNachBild( max_y ) );
      // show infos 
      s << name << _(" with Gamma: 1.0");
      fl_draw ( s.str().c_str(), xNachBild(0) + 2, yNachBild(max_y) + j*16 +12);
    // parametric entry
    } else if (kurven[j].size() == 1
            && ist_kurve) {
      double segmente = 256;
      double gamma = kurven[j][0]; DBG_NUM_V( gamma )
      for (int i = 1; i < segmente; i++)
        fl_line (xNachBild( pow( (double)(i-1.0)/segmente, 1./gamma ) * max_x ),
                 yNachBild( (i-1) / ((segmente-1) / max_y) ),
                 xNachBild( pow( (double)i/segmente, 1./gamma ) * max_x ),
                 yNachBild( (i) / ((segmente-1) / max_y) ) );
      // show infos 
      s << name << _(" with one entry for gamma: ") << gamma; DBG_NUM_V( gamma )
      fl_draw ( s.str().c_str(), xNachBild(0) + 2, yNachBild(max_y) + j*16 +12);
    // parametric entry with Min und Max 
    } else if (kurven[j].size() == 3
            && texte[texte.size()-1] == "gamma_start_ende") {
      double segmente = 256;
      double gamma = kurven[j][0]; DBG_NUM_V( gamma )
      double start = kurven[j][1]; DBG_NUM_V( start )
      double ende  = kurven[j][2]; DBG_NUM_V( ende )
      double mult  = (ende - start); DBG_NUM_V( mult )
      for (int i = 1; i < segmente; i++) {
        fl_line (xNachBild( (pow( (double)(i-1.0)/segmente, 1./gamma)
                            * mult + start) * max_x ),
                 yNachBild( (i-1) / ((segmente-1) / max_y) ),
                 xNachBild( (pow( (double)i/segmente, 1./gamma ) * mult + start)
                            * max_x ),
                 yNachBild( (i) / ((segmente-1) / max_y) ) );
      }
      // show infos
      s << name << _(" with one entry for gamma: ") << gamma;
      fl_draw ( s.str().c_str(), xNachBild(0) + 2, yNachBild(max_y) + j*16 +12);
    // segmented curv
    } else { // value range 0.0 -> max_[x,y]
      for (unsigned int i = 1; i < kurven[j].size(); i++) {
        //if( kurve_umkehren )
          fl_line (xNachBild( (i-1) / ((kurven[j].size() -1)
                              / max_x) ),
                   yNachBild( kurven[j][i-1] * max_y ),
                   xNachBild( (i) / ((kurven[j].size() - 1)
                              / max_x) ),
                   yNachBild( kurven[j][i] * max_y) );
        /*else
          fl_line (xNachBild( kurven[j][i-1] * max_x),
                   yNachBild( (i-1) / ((kurven[j].size() -1)
                              / max_y) ),
                   xNachBild( kurven[j][i] * max_x),
                   yNachBild( (i) / ((kurven[j].size() - 1)
                              / max_y) ) );
          */
      }
      // show infos
      s << name << _(" with ") << kurven[j].size() << _(" points");
      fl_draw ( s.str().c_str(), xNachBild(min_x) + 2, yNachBild(max_y) + j*16 +12);
    }
  }
  // additional text
  if (texte.size() > kurven.size() &&
      texte.size() > kurven2.size() )
  { 
    fl_color(FL_BLACK);
    for(unsigned j = (unsigned)kurven.size();
          j < (unsigned)texte.size(); ++j)
      if(texte[j] != "gamma_start_ende" &&
         texte[j] != "curv")
        fl_draw ( texte[j].c_str(), xNachBild(min_x) + 2, yNachBild(max_y) + j*16 + 12);
  }

  fl_pop_clip();
  DBG_prog_ende
}

void
TagDrawings::dHaendler(void* o)
{
  DBG_PROG_START
  Fl::remove_timeout( (void(*)(void*))dHaendler, 0 );

  if (!Fl::has_timeout( (void(*)(void*))dHaendler, 0 )
   && ((TagDrawings*)o)->active()
   && ((TagDrawings*)o)->visible_r()
   && ((TagDrawings*)o)->wiederholen)
  {
    ((TagDrawings*)o)->ruhigNeuzeichnen();

#   ifdef DEBUG
    DBG_PROG_V( ((TagDrawings*)o)->wiederholen )
#   endif
  }
  DBG_PROG_ENDE
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
