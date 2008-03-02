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

#include "icc_utils.h"
//#include "agviewer.h"
#include "icc_vrml_parser.h"
#include "icc_thread_daten.h"
#if APPLE
#  include <OpenGL/glu.h>
#else
#  include <GL/glu.h> // added for FLTK
#endif
#include "icc_modell_beobachter.h"
//#include "Fl_Slot.H"

#include <FL/Fl_Gl_Window.H>
#include <FL/Fl_Group.H>

class Fl_Menu_Button;
class Agviewer;

class GL_Ansicht : public Fl_Gl_Window, /*, public Fl_Slot*/
                   public icc_examin_ns::ThreadDaten,
                   public icc_examin_ns::Beobachter {
  // Datenhaltung
    // Position: Farbe1, Farbe2, Farbe3, Farbkanal Nr., Wert
  std::vector<std::vector<std::vector<std::vector<double> > > > tabelle_;
  std::vector<std::string>nach_farb_namen_;
  std::vector<std::string>von_farb_namen_;
  std::vector<std::string>farb_namen_;
  std::vector<double> punkte_;        //                (n*3)
  std::vector<float>  farben_;        // rgba 0.0 - 1.0 (n*4)

  void fensterForm();

  // inner Strukturen bei Datenwechsel anpassen
  void menueErneuern_();
  void erstelleGLListen_();
  void textGarnieren_();
  void garnieren_();
  void auffrischen_();      //!< Erneuerung ohne init()

  // Menues
  Fl_Menu_Button  *menue_;
  Fl_Menu_Button  *menue_button_;
  Fl_Menu_Button  *menue_schnitt_;
  Fl_Menu_Button  *menue_form_;
  Fl_Menu_Button  *menue_hintergrund_;
  static void c_(Fl_Widget* w, void* daten);
  
  // IDs
  Agviewer *agv_;
  static int  ref_;
  int  id_;
  int  typ_;
  void GLinit_();
  void menueInit_();

  typedef enum {NOTALLOWED, AXES, RASTER, PUNKTE , SPEKTRUM, HELFER, DL_MAX } DisplayLists;
  int glListen[DL_MAX];
  typedef enum {
   MENU_AXES,
   MENU_QUIT,
   MENU_KUGEL,           //!< Formen der 3DLut-darstellung
   MENU_WUERFEL,
   MENU_STERN,
   MENU_GRAU,            //!< Die Farbdarstellung der 3DLut
   MENU_FARBIG,
   MENU_KONTRASTREICH,
   MENU_SCHALEN,         //!< Aussparen der 3DLut
   MENU_dE1KUGEL,        //!< Mess-/Profilwertdifferenzen mit Farborten
   MENU_dE2KUGEL,
   MENU_dE4KUGEL,
   MENU_dE1STERN,
   MENU_DIFFERENZ_LINIE, //!< Mess-/Profilwertdifferenzen mit Geraden pur
   MENU_SPEKTRALBAND,    //!< Darstellung der Spektralfarben als Band
   MENU_HELFER,          //!< Texte und Pfeile darstellen
   MENU_WEISS,           //!< Hintergrundfarben
   MENU_HELLGRAU,
   MENU_GRAUGRAU,
   MENU_DUNKELGRAU,
   MENU_SCHWARZ,
   MENU_MAX
  } MenuChoices;

  int hintergrundfarbeZuMenueeintrag( float farbe );

  int DrawAxes;

public:
  GL_Ansicht(int X,int Y,int W,int H);
  ~GL_Ansicht();
  GL_Ansicht (const GL_Ansicht & gl)
    : Fl_Gl_Window(0,0,gl.w(),gl.h()) { id_ = ref_; ++ref_; copy(gl); }
  GL_Ansicht& copy(const GL_Ansicht& gl);
  GL_Ansicht& operator = (const GL_Ansicht& gl) { return copy(gl); }
  void init(int fenster);

  // welches Fenster wird verwaltet?
  int  id()          {return id_; } //!< gleich zu agviewer::RedisplayWindow
  int  typ()   {return typ_; } //!< Fenster ID / Darstellungsart
  void typ(int t)   { typ_ = t; }

  static Agviewer* getAgv(GL_Ansicht *me, GL_Ansicht *referenz);

  // fltk virtual
  void draw();
  int  handle(int event);
  void redraw();
  void show();
  void hide();
  // redraw Aufforderung von agv_
  void nachricht( icc_examin_ns::Modell* modell, int info );

  // Daten Laden
  void hineinPunkte (std::vector<double> &vect,
                     std::vector<std::string> &achsNamen);
  void hineinPunkte (std::vector<double> &vect, 
                     std::vector<float>  &farben_,
                     std::vector<std::string> &achsNamen);
  void hineinPunkte (std::vector<double> &punktKoordinaten, //!< XYZ
                     std::vector<float>  &punktFarben,      //!< RGBA
                     std::vector<std::string> &farb_namen_, //!< pro Punkt
                     std::vector<std::string> &achsNamen);  //!< 3*
  void punkte_clear () { punkte_.clear(); farben_.clear(); }
  void hineinNetze  (const std::vector<ICCnetz> & dreiecks_netze);
  std::vector<ICCnetz> dreiecks_netze;
  void achsNamen    (std::vector<std::string> achs_namen);

  void hineinTabelle(std::vector<std::vector<std::vector<std::vector<double> > > >vect,
                               std::vector<std::string> vonFarben,
                               std::vector<std::string> nachFarben);

  // transparente Darstellung
  int  kanal;               //!< gewaehlter Kanal
       // Darstellung der Gitterpunkte der Transformationstabelle
  int  punktform;           //!< MENU_KUGEL MENU_WUERFEL MENU_STERN
  int  punktfarbe;          //!< MENU_GRAU MENU_FARBIG MENU_KONTRASTREICH
  int  punktgroesse;        //!< Groesse in Pixel
  int  punkt_zahl_alt;

  float hintergrundfarbe;   //!< Hintergrundfarben Farbschema
  float textfarbe[3];
  float pfeilfarbe[3];
  float schatten;
  float strichmult;         //!< Strichmultiplikator
  char  strich1, strich2, strich3;
  int   schalen;            //!< MENU_SCHALEN

  // Darstellungsfunktionen
  void setzePerspektive();  //!< Perspektive aktualisieren
  void tabelleAuffrischen();//!< glCompile fuer Tabelle
  void punkteAuffrischen(); //!< glCompile fuer Punkte
  void netzeAuffrischen();  //!< Sortieren und Zeichnen
  double seitenverhaeltnis; //!< Proportion des Fensters
  static const double std_vorder_schnitt;
  double vorder_schnitt;    //!< Entfernung der ersten Schnittebene
  double schnitttiefe;      //!< Dicke der GL Schnitttiefe
  double a_darstellungs_breite; //!< Richtung CIE*a   fuer Zoom und Pfeillaengen
  double b_darstellungs_breite; //!< ~        CIE*b ; wobei CIE*L immer 1.0
  bool zeig_punkte_als_messwert_paare;
  bool zeig_punkte_als_messwerte;
  int  spektralband;        //!< stelle die spektral gesaettigten Farben dar
  int  zeige_helfer;        //!< zeige Pfeile und Text
private:
  void zeigeSpektralband_();
  void zeigeUmrisse_();
  // Debug
  void zeichneKoordinaten_();

public:
  // Darstellungsfunktionen
  void zeichnen();          //!< gl Zeichnen
//  int  dID (int display_liste);
  void tastatur(int e);
  void menueAufruf(int value);
  // Bewegungsfunktionen
  void bewegen(bool setze);
  bool darfBewegen();
  void darfBewegen(int d);
private:
  void stupps_(bool lauf);
  static void bewegenStatisch_(void* GL_Ansicht);
  //bool darf_bewegen_;
  //! Ist es zu drehen möglich? Fenster sichtbar?
  bool ist_bewegt_;
private:
  double zeit_diff_;        //!< Sekunde pro Bild
  double zeit_;
  int  valid_;              //!< notiert ob in draw() valid() false war
  char t[128];              //!< Text zur Fehlersuche
  int  maus_x_;
  int  maus_y_;
  int maus_x_alt, maus_y_alt;
  bool maus_steht;
  void mausPunkt_( GLdouble & oX, GLdouble & oY, GLdouble & oZ,
                  GLdouble & X, GLdouble & Y, GLdouble & Z );
public:
  // Geschwindigkeit
  int  smooth;                    //!< glatt zeichnen
  int  blend;                     //!<   -"-
  int  wiederholen;               //!< bezogen auf smooth/blend

  // Daten Informationen
  const char* kanalName() const {
                      if (nach_farb_namen_.size() &&
                          (int)nach_farb_namen_.size() > kanal) {
                        return nach_farb_namen_[kanal].c_str();
                      } else {
                        return "";}  }
  const char* kanalName(unsigned int i) const {
                      if (nach_farb_namen_.size()>i) 
                        return (const char*)nach_farb_namen_[i].c_str();
                      else  return _("Gibts nicht"); }
  unsigned int kanaele() {return nach_farb_namen_.size(); }
};


#endif //ICC_GL_H
