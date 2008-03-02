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
#include <string>
#include <FL/Fl.H>
#include <lcms.h>

class TagDrawings : public Fl_Widget {
public:
  int  wiederholen,
       id;
  TagDrawings       ( int X,int Y,int W,int H);
  ~TagDrawings      ( );
  void draw         ( );  // fltk virtual
  void ruhigNeuzeichnen (void);
  // der Vektor vect enthält fortlaufende XYZ Daten, txt den passenden Text
  void hineinPunkt  ( std::vector<double> &vect,
                      std::vector<std::string> &txt);
  // vect enthält Kurven mit 1 Koordinaten, txt den passenden Text dazu
  void hineinKurven ( std::vector<std::vector<double> > &vect,
                      std::vector<std::string> &txt);
  // vect enthält Kurven mit 2 Koordinaten, txt den passenden Text dazu
  void hineinDaten  ( std::vector<std::vector<std::pair<double,double> > >&vect,
                      std::vector<std::string> &txt);
//  void hineinDaten  ( Handler* daten, std::vector<std::string> &txt);

private:
//  void *daten;
  std::vector<std::string> texte; // Texte zu Punkten oder Kurven
  void drawKurve_   ( );
  std::vector<std::vector<double> > kurven; // Daten
  std::vector<std::vector<std::pair<double,double> > > kurven2;
  static void dHaendler(void* o);

public:
  bool zeichne_linie;             // Typ kurven2
  bool zeichne_symbole;           // Typ kurven2
  bool ursprung_zeichnen;         // Koordinatenursprung zeichnen
  bool kurve_umkehren;            // tauscht x und y Achse für Typ kurven

private:
  void drawCieShoe_ ( int  repeated);
  std::vector<double> punkte;               // Daten
  void clear() {punkte.clear(); kurven.clear(); kurven2.clear(); }

  // Zeichenbereich
  float breite,hoehe,
        xO, yO;
public:
  // Zeichenbereichvariablen
  int tab_rand_x;                  // Tabellenrand rechts und links
  int tab_rand_y;
  int linker_text_rand;            // zusätzlicher Rand für Text links
  int unterer_text_rand;
  int raster_abstand;              // Abstand der Rasterlinien in Punkte
  bool zeige_raster;               // Schalter für Darstellung der Rasterlinien

  // Wertebereiche
  double min_x, min_y, max_x, max_y;

private:
  double raster_wert_x_;           // Rasterweite in Benutzereinheiten
  double raster_wert_y_;
  void   zeichneRaster_(void);     // Darstellung des Raster mit Werten

  // CIExy Raster
  int raster;                      // Variable für CIExy Diagram
  int init_s;
  void init_shoe_ ();

  // lcms Typen
  cmsHPROFILE hXYZ;
  cmsHPROFILE hsRGB;
  cmsHTRANSFORM xform;
  double rechenzeit;
  unsigned char* RGB_speicher;
  cmsCIEXYZ* XYZ_speicher;
  int n_speicher;

  // einige Koordinaten
  inline int xNachBild (double val)
                 { return (int)(((double)xO + (val-min_x)*breite
                                              /(max_x-min_x)) + 0.5); }
  inline int yNachBild (double val)
                 { return (int)(((double)yO - (val-min_y)*hoehe
                                              /(max_y-min_y)) + 0.5); }
  inline double bildNachX (int val) { return ((val-min_x-xO)*(max_x-min_x)/breite); }
  inline double bildNachY (int val) { return ((yO-val-min_y)*(max_y-min_y)/hoehe); }
};




#endif //ICC_DRAW_H

