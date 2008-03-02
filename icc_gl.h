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
 * the 3D viewer.
 * 
 */

// Date:      12. 09. 2004


#ifndef ICC_GL_H
#define ICC_GL_H
#include <vector>

#include "icc_utils.h"
//#include "agviewer.h"
#include "icc_vrml_parser.h"
#include "icc_oyranos.h"
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

class GL_Ansicht : public Fl_Gl_Window,
                   public icc_examin_ns::ThreadDaten,
                   public icc_examin_ns::Beobachter,
                   public icc_examin_ns::Modell {
  // internal data
    //! position: colour1, colour2, colour3, colour channel No., value
  std::vector<std::vector<std::vector<std::vector<double> > > > tabelle_;
  std::vector<std::string>nach_farb_namen_;
  std::vector<std::string>von_farb_namen_;
  std::vector<std::string>farb_namen_;
  std::vector<double> punkte_;        //!<                (n*3)
  std::vector<float>  farben_;        //!< rgba 0.0 - 1.0 (n*4)
  oyNamedColour_s * epoint_;            //!< emphasize point
public:
  oyNamedColour_s * mouse_3D_hit;       //!< a point recently hit by the mouse

private:
  void fensterForm();

  // adapt inner struktures at data change
  void menueErneuern_();
  int  erstelleGLListen_();
  void textGarnieren_();
  void garnieren_();
  int  auffrischen_();      //!< refresh without init()

  // menues
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

  typedef enum {NOTALLOWED, AXES, RASTER, PUNKTE , SPEKTRUM, HELFER, UMRISSE, DL_MAX } DisplayLists;
  int gl_listen[DL_MAX];
public:
  typedef enum {
   MENU_AXES,
   MENU_QUIT,
   MENU_KUGEL,           //!< form of 3DLut-representation
   MENU_WUERFEL,
   MENU_STERN,
   MENU_GRAU,            //!< the colour displaying of the 3DLut
   MENU_FARBIG,
   MENU_KONTRASTREICH,
   MENU_SCHALEN,         //!< sparce of the 3DLut / plug-in candidate
   MENU_dE1KUGEL,        //!< meashurement-/profile differences with colour location
   MENU_dE2KUGEL,
   MENU_dE4KUGEL,
   MENU_dE1STERN,
   MENU_DIFFERENZ_LINIE, //!< meashurement-/profile differences with put lines
   MENU_SPEKTRALBAND,    //!< spectral colours as lines
   MENU_HELFER,          //!< show texts and arrows
   MENU_WEISS,           //!< background colour
   MENU_HELLGRAU,
   MENU_GRAUGRAU,
   MENU_DUNKELGRAU,
   MENU_SCHWARZ,
   MENU_MAX
  } MenuChoices;
private:
  int hintergrundfarbeZuMenueeintrag( float farbe );

  int DrawAxes;

public:
  GL_Ansicht(int X,int Y,int W,int H);
  GL_Ansicht(int X,int Y,int W,int H, const char *l);
  ~GL_Ansicht();
  GL_Ansicht (const GL_Ansicht & gl)
    : Fl_Gl_Window(0,0,gl.w(),gl.h()) { id_ = ref_; ++ref_; copy(gl); }
  GL_Ansicht& copy(const GL_Ansicht& gl);
  GL_Ansicht& operator = (const GL_Ansicht& gl) { return copy(gl); }
private:
  void init_();
public:
  void init(int fenster);

  // which window is managed?
  int  id()          {return id_; } //!< equal to agviewer::RedisplayWindow
  int  typ()   {return typ_; } //!< window ID / display mode
  void typ(int t_)   { typ_ = t_; }

  static Agviewer* getAgv(GL_Ansicht *me, GL_Ansicht *referenz);

  // fltk virtual
  void redraw();
private:
  void draw();
  int  handle(int event);
  int  waiting_;       //!< dont generate and display new movement
public:
  void show();
  void hide();
  // redraw request from agv_
  void nachricht( icc_examin_ns::Modell* modell, int info );

  // import data
/*  void hineinPunkte (std::vector<double> &vect,
                     std::vector<std::string> &achsNamen);
  void hineinPunkte (std::vector<double> &vect, 
                     std::vector<float>  &farben_,
                     std::vector<std::string> &achsNamen);
  void hineinPunkte (std::vector<double> &punktKoordinaten, //!< Lab
                     std::vector<float>  &punktFarben,      //!< RGBA
                     std::vector<std::string> &farb_namen_, //!< per point
                     std::vector<std::string> &achsNamen);  //!< 3* */
TODO: Punkte auf oyNamedColour_s umstellen
  void emphasizePoint (oyNamedColour_s * colour);  //!< a named colour
  void punkte_clear () { punkte_.clear(); farben_.clear(); }
  void herausNormalPunkte (std::vector<double> & p, std::vector<float> & f);
  void hineinNetze  (const std::vector<ICCnetz> & dreiecks_netze);
  std::vector<ICCnetz> dreiecks_netze;
  void achsNamen    (std::vector<std::string> achs_namen);

  void hineinTabelle(std::vector<std::vector<std::vector<std::vector<double> > > >vect,
                               std::vector<std::string> vonFarben,
                               std::vector<std::string> nachFarben);

  // transparent displaying
  int  kanal;               //!< selected channel
       // displaying of grid points of the transformation table
  int  punktform;           //!< MENU_KUGEL MENU_WUERFEL MENU_STERN
  int  punktfarbe;          //!< MENU_GRAU MENU_FARBIG MENU_KONTRASTREICH
  int  punktgroesse;        //!< size in pixel
  ICCnetz netz;             //!< internal net representation, thread entry

  float hintergrundfarbe;   //!< background colour / colour sheme
  float textfarbe[3];
  float pfeilfarbe[3];
  float schatten;
  float strichmult;         //!< multiplicator
  char  strich1, strich2, strich3;
  int   schalen;            //!< MENU_SCHALEN

  // drawing functions
  void setzePerspektive();  //!< actualise perspektive
  void tabelleAuffrischen();//!< glCompile for table
  void punkteAuffrischen(); //!< glCompile for points
  void netzeAuffrischen();  //!< sort and drawing
  double seitenverhaeltnis; //!< proportion of window
  static const double std_vorder_schnitt;
  double vorder_schnitt;    //!< front cut plane
  double schnitttiefe;      //!< thickness of the GL slice
  double a_darstellungs_breite; //!< direction CIE*a   for stretching
  double b_darstellungs_breite; //!< ~         CIE*b ; where CIE*L max is 1.0
  bool zeig_punkte_als_paare;
  bool zeig_punkte_als_messwerte;
  int  spektralband;        //!< show spectral saturated colour line
  int  zeige_helfer;        //!< show arrows and text
  char text[128];           //!< Status line text
private:
  void zeigeSpektralband_();
  void zeigeUmrisse_();
  // Debug
  void zeichneKoordinaten_();

public:
  // display functions
  void zeichnen();          //!< gl drawing
  void tastatur(int e);
  void menueAufruf(int value);
  // Bewegungsfunktionen
  void bewegen(bool setze);
  bool darfBewegen();
  void darfBewegen(int d);
private:
  static void bewegenStatisch_(void* GL_Ansicht);
private:
  double zeit_diff_;        //!< seconds per frame
  double zeit_;
  int  valid_;              //!< remembers valid() from within draw()
  char t[128];              //!< text for searching errors
  int  maus_x_;
  int  maus_y_;
  int  maus_x_alt, maus_y_alt;
  bool maus_steht;
  void mausPunkt_( GLdouble & oX, GLdouble & oY, GLdouble & oZ,
                  GLdouble & X, GLdouble & Y, GLdouble & Z, int from_mouse );
public:
  // speed
  int  smooth;                    //!< smooth drawing
  int  blend;                     //!<   -"-

  // data informations
  const char* kanalName() const {
                      if (nach_farb_namen_.size() &&
                          (int)nach_farb_namen_.size() > kanal) {
                        return nach_farb_namen_[kanal].c_str();
                      } else {
                        return "";}  }
  const char* kanalName(unsigned int i) const {
                      if (nach_farb_namen_.size()>i) 
                        return (const char*)nach_farb_namen_[i].c_str();
                      else  return _("not available"); }
  unsigned int kanaele() {return (unsigned int)nach_farb_namen_.size(); }
};


#endif //ICC_GL_H
