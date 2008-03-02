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
 * Zeichenroutinen für Diagramme.
 * 
 */

// Date:      Mai 2004

#ifndef ICC_DRAW_H
#define ICC_DRAW_H

#include <vector>
#include <FL/Fl.H>
#include <lcms.h>

class TagDrawings : public Fl_Widget {
  int  X, Y, W, H;
public:
  int  wiederholen,
       id;
  TagDrawings       ( int X,int Y,int W,int H) ;
  void draw         ( );
  // der Vektor vect enthält fortlaufende XYZ Daten, txt den passenden Text
  void hineinPunkt  ( std::vector<double> &vect,
                      std::vector<std::string> &txt);
  // vect enthält Kurven mit 2 Koordinaten, txt den passenden Text dazu
  void hineinKurven ( std::vector<std::vector<double> > &vect,
                      std::vector<std::string> &txt);
  void ruhigNeuzeichnen (void);

private:
  void drawKurve_   ( int id, int X, int Y, int W, int H);

  void drawCieShoe_ ( int id, int X, int Y, int W, int H,
                      int  repeated);
  // Zeichenbereich
  float breite,hoehe, xO, yO;
  // Zeichenbereichvariablen
  int tab_border_x;
  int tab_border_y;
  // Diagrammvariablen
  float n;

  int raster;
  int init_s;
  // lcms Typen
  cmsHPROFILE hXYZ;
  cmsHPROFILE hsRGB;
  cmsHTRANSFORM xform;
  double rechenzeit;
  unsigned char* RGB_speicher;
  cmsCIEXYZ* XYZ_speicher;
  int n_speicher;

  void init_shoe();
};




#endif //ICC_DRAW_H

