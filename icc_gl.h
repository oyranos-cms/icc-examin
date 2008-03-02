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
#include "icc_vrml_parser.h"
#include "Fl_Slot.H"

#include <FL/Fl_Gl_Window.H>
#include <FL/Fl_Group.H>

class Fl_Menu_Button;

class GL_Ansicht : public Fl_Group , public Fl_Slot {
  // Datenhaltung
  std::vector<std::vector<std::vector<std::vector<double> > > > tabelle_;
  std::vector<std::string>nach_farb_namen_;
  std::vector<std::string>von_farb_namen_;
  std::vector<std::string>farb_namen_;
  std::vector<double> punkte_;        //                (n*3)
  std::vector<float>  farben_;        // rgba 0.0 - 1.0 (n*4)
  std::vector<std::vector<double> >kurven_;

  bool auffrischen_;
  // Referenz zu einem abgekoppelten fltk Objekt
  // Der Gedanke ist : Fltk löscht den OpenGL Kontext.
  //                   Die Klasse gl_fenster_ behält ihre volle Größe.
  //                   Die GL_Ansicht kann in gl_fenster_ eingepasst oder
  //                   auf 1x1 verkleinert werden.
  Fl_Gl_Window *gl_fenster_;
  // GL_Ansicht an gl_fenster_ anpassen oder Größe 1x1
  bool gl_fenster_zeigen_;
  void fensterForm();

  // inner Strukturen bei Datenwechsel anpassen
  void menueErneuern_();
  void erstelleGLListen_();
  void garnieren_();
  // Menüs
  Fl_Menu_Button  *menue_;
  Fl_Menu_Button  *menue_button_;
  Fl_Menu_Button  *menue_schnitt_;
  Fl_Menu_Button  *menue_form_;
  Fl_Menu_Button  *menue_hintergrund_;
  static void c_(Fl_Widget* w, void* daten);
  
  // IDs
  Agviewer agv_;
  int  glut_id_;
  // gibt den Initalstatus an
  bool beruehrt_;
  void GLinit_();
  void menueInit_();

public:
  GL_Ansicht(int X,int Y,int W,int H);
  ~GL_Ansicht();
  void init(int id);
  bool beruehrt () {return beruehrt_; }

  // welches Glutfenster wird verwaltet?
  int  id()          {return glut_id_; } // gleich zu agviewer::RedisplayWindow
  // welches agvfenster wird benutzt?
  //Agviewer* agv()         {return &agv_; }
  // fltk virtual
  void draw();

  // Daten Laden
  void hineinPunkte (std::vector<double> vect,
                     std::vector<std::string> achsNamen);
  void hineinPunkte (std::vector<double> vect, 
                     std::vector<float> farben_,
                     std::vector<std::string> achsNamen);
  void hineinPunkte (std::vector<double> punktKoordinaten, // XYZ
                     std::vector<float>  punktFarben,      // RGBA
                     std::vector<std::string> farb_namen_, // pro Punkt
                     std::vector<std::string> achsNamen);  // 3*
  void punkte_clear () { punkte_.clear(); farben_.clear(); }
  void hineinNetze  (const std::vector<ICCnetz> & dreiecks_netze);
  std::vector<ICCnetz> dreiecks_netze;
  void achsNamen    (std::vector<std::string> achs_namen);
  void hineinKurven(std::vector<std::vector<double> >vect,
                     std::vector<std::string> txt);

  void hineinTabelle(std::vector<std::vector<std::vector<std::vector<double> > > >vect,
                               std::vector<std::string> vonFarben,
                               std::vector<std::string> nachFarben);

  // transparente Darstellung
  int  kanal;               // gewählter Kanal
       // Darstellung der Gitterpunkte der Transformationstabelle
  int  punktform;           // MENU_KUGEL MENU_WUERFEL MENU_STERN
  int  punktfarbe;          // MENU_GRAU MENU_FARBIG MENU_KONTRASTREICH
  int  punktgroesse;        // Größe in Pixel

  int  hintergrundfarbe;    // Hintergrundfarben Farbschema
  float textfarbe[3];
  float pfeilfarbe[3];
  float strichmult;         // Strichmultiplikator
  int  schalen;             // MENU_SCHALEN

  // Darstellungsfunktionen
  void setzePerspektive();  // Perspektive aktualisieren
  void auffrischen();       // Erneuerung ohne init()
  void tabelleAuffrischen(); // glCompile für Tabelle
  void punkteAuffrischen(); // glCompile für Punkte
  void netzeAuffrischen();    // Sortieren und Zeichnen
  double seitenverhaeltnis; // Proportion des Fensters
  static const double std_vorder_schnitt;
  double vorder_schnitt;      // Entfernung der ersten Schnittebene
  double schnitttiefe;      // Dicke der GL Schnitttiefe
  double a_darstellungs_breite; // Richtung CIE*a   für Zoom und Pfeillängen
  double b_darstellungs_breite; // ~        CIE*b ; wobei CIE*L immer 1.0
  bool zeig_punkte_als_messwert_paare;
  bool zeig_punkte_als_messwerte;
  int  spektralband;        // stelle die spektral gesättigten Farben dar
  int  zeige_helfer;        // zeige Pfeile und Text
private:
  void zeigeSpektralband_();

public:
  // Darstellungsfunktionen
  void zeigen();            // diese Klasse anzeigen (fltk + glut + gl)
  void verstecken();        //  ~           verstecken      ~
  bool sichtbar() {return gl_fenster_zeigen_; } // angezeigt / versteckt
  void zeichnen();          // gl Zeichnen
  void auffrischen__();       // Erneuerung ohne init() und ohne Menüs
  int  dID (int display_liste);
  void tastatur(int e);
  void menueAufruf(int value);
  // Bewegungsfunktionen
  void stop(); 

  // Daten Informationen
  const char* kanalName() const {
                      if (nach_farb_namen_.size() &&
                          (int)nach_farb_namen_.size() > kanal) {
                        return nach_farb_namen_[kanal].c_str();
                      } else {
                        return _("");}  }
  const char* kanalName(unsigned int i) const {
                      if (nach_farb_namen_.size()>i) 
                        return (const char*)nach_farb_namen_[i].c_str();
                      else  return _("Gibts nicht"); }
  unsigned int kanaele() {return nach_farb_namen_.size(); }
};

#if 0
#define deklariereGlutFunktionen(n) \
void reshape##n( int w, int h ); \
void display##n(); \
void sichtbar##n(int v); \
void menuuse##n(int v); \
void menueAufruf##n(int value);

deklariereGlutFunktionen(1)  // mft Tabellenfenster
deklariereGlutFunktionen(2)  // Farbraumhüllansicht

void reshape(int id, int w, int h);
void display(int id);
void sichtbar(int id, int v);
void menuuse(int id, int v);
void menueAufruf(int id, int value);

int  dID(int id, int display_list);
#endif


#endif //ICC_GL_H
