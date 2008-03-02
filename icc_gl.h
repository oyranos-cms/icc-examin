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
 * Der 3D Betrachter.
 * 
 */

// Date:      12. 09. 2004


#ifndef ICC_GL_H
#define ICC_GL_H
#include <vector>

#include "agviewer.h"

class GL_Ansicht : public Fl_Group {
  std::vector<std::vector<std::vector<std::vector<double> > > > tabelle_;
  std::vector<std::string>nach_farb_namen_;
  std::vector<std::string>von_farb_namen_;
  std::vector<std::string>farb_namen_;
  std::vector<std::vector<double> > dreiecks_netze_;
  std::vector<std::vector<float> >  dreiecks_farben_; // rgba 0.0 - 1.0 (n*4)
  std::vector<std::string>          netz_namen_;
  std::vector<double> punkte_;        //                (n*3)
  std::vector<float>  farben_;        // rgba 0.0 - 1.0 (n*4)
  std::vector<std::vector<double> >kurven_;
  bool auffrischen_;
  Fl_Group *gl_fenster_;
  void menueErneuern_();
  void makeDisplayLists_();
  int  menue_;
  int  menue_kanal_eintraege_;
  int  menue_schnitt_;
  int  menue_form_;
  int  menue_hintergrund_;
  void myGLinit_();
  void menuInit_();
  bool gl_fenster_zeigen_;
  int  agv_,
       glut_id_;
  bool beruehrt_;
public:
  GL_Ansicht(int X,int Y,int W,int H);
  ~GL_Ansicht();
  void init();
  bool beruehrt () {return beruehrt_; }
  //void setzteGlutId(int id) {if (!beruehrt_) glut_id_ = id; }

  int  id()          {return glut_id_; }
  int  agv()         {return agv_; }
  void draw();
  void hineinPunkte (std::vector<double> vect,
                     std::vector<std::string> achsNamen);
  void hineinPunkte (std::vector<double> vect, 
                     std::vector<float> farben_,
                     std::vector<std::string> achsNamen);
  void hineinPunkte (std::vector<double> punktKoordinaten, // XYZ
                     std::vector<float>  punktFarben,      // RGBA
                     std::vector<std::string> farb_namen_, // pro Punkt
                     std::vector<std::string> achsNamen);  // 3*
  void hineinNetze  (std::vector<std::vector<double> >dreiecks_netze_, 
                     std::vector<std::vector<float> > dreiecks_farben_,
                     std::vector<std::string> netz_namen_,
                     std::vector<std::string> _achsNamen);

  void hineinKurven(std::vector<std::vector<double> >vect,
                     std::vector<std::string> txt);

  void hineinTabelle(std::vector<std::vector<std::vector<std::vector<double> > > >vect,
                               std::vector<std::string> vonFarben,
                               std::vector<std::string> nachFarben);

  int  kanal;               // gewählter Kanal
  // Darstellung der Gitterpunkte der Transformationstabelle
  int  punktform;           // MENU_KUGEL MENU_WUERFEL MENU_STERN
  int  punktfarbe;          // MENU_GRAU MENU_FARBIG MENU_KONTRASTREICH
  int  hintergrundfarbe;    // Hintergrundfarben Farbschema
  float textfarbe[3];
  float pfeilfarbe[3];
  int  schalen;             // MENU_SCHALEN
  void auffrischen();       // Erneuerung ohne init()
  void punkteAuffrischen(); // glCompile für Punkte
  double seitenverhaeltnis; // Proportion des Fensters
  double schnitttiefe;      // Dicke der GL Schnitttiefe
  double a_darstellungs_breite; // Richtung CIE*a   für Zoom und Pfeillängen
  double b_darstellungs_breite; // ~        CIE*b ; wobei CIE*L immer 1.0
  bool zeig_punkte_als_messwert_paare;
  int  spektralband;        // stelle die spektral gesättigten Farben dar
  int  zeige_helfer;        // zeige Pfeile und Text
private:
  void zeigeSpektralband_();

public:
  void zeigen();            // diese Klasse anzeigen (fltk + glut + gl)
  void verstecken();        //  ~           verstecken      ~
  bool sichtbar() {return gl_fenster_zeigen_; } // angezeigt / versteckt
  void stop() {if (beruehrt_) { agvSwitchMoveMode (Agviewer::AGV_STOP); } }//Bew

  char* kanalName() { DBG_PROG_START DBG_PROG_V( nach_farb_namen_.size() <<"|"<< kanal )
                      if (nach_farb_namen_.size() &&
                          (int)nach_farb_namen_.size() > kanal) { DBG_PROG_ENDE
                        return (char*)nach_farb_namen_[kanal].c_str();
                      } else { DBG_PROG_ENDE
                        return _("");}  }
  char* kanalName(unsigned int i) { if (nach_farb_namen_.size()>i) 
                                      return (char*)nach_farb_namen_[i].c_str();
                                    else  return _("Gibts nicht"); }
  unsigned int kanaele() {return nach_farb_namen_.size(); }
};

#define deklariereGlutFunktionen(n) \
void reshape##n( int w, int h ); \
void display##n(); \
void sichtbar##n(int v); \
void menuuse##n(int v); \
void handlemenu##n(int value);

deklariereGlutFunktionen(1)  // mft Tabellenfenster
deklariereGlutFunktionen(2)  // Farbraumhüllansicht

void reshape(int id, int w, int h);
void display(int id);
void sichtbar(int id, int v);
void menuuse(int id, int v);
void handlemenu(int id, int value);

int  dID(int id, int display_list);



#endif //ICC_GL_H
