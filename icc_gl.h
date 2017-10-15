/*
 * ICC Examin ist eine ICC Profil Betrachter
 * 
 * Copyright (C) 2004-2013  Kai-Uwe Behrmann 
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

#include "icc_utils.h"
//#include "agviewer.h"
#include "icc_fenster.h"
#include "icc_vrml_parser.h"
#include "icc_oyranos.h"
#include "icc_thread_daten.h"
#if defined(__APPLE__)
#  include <OpenGL/glu.h>
#else
#  include <GL/glu.h> // added for FLTK
#endif
#include "icc_model_observer.h"
#include "bsp/bsp.h"
//#include "Fl_Slot.H"

#include <FL/Fl_Gl_Window.H>
#include <FL/Fl_Group.H>

class Fl_Menu_Button;
class Agviewer;

/*template <typename T>
class ICCnetzList: public ICClist,
                   public icc_examin_ns::ThreadDaten {
};*/

class GL_View : public Fl_Gl_Window,
                   public icc_examin_ns::ThreadDaten,
                   public icc_examin_ns::Observer,
                   public icc_examin_ns::Model {
  // internal data
    //! position: colour1, colour2, colour3, colour channel No., value
  ICClist<ICClist<ICClist<ICClist<double> > > > table_;
  ICClist<std::string>to_channel_names_;
  ICClist<std::string>from_channel_names_;
  ICClist<int>        channels_;
  oyStructList_s * colours_;
  oyNamedColor_s  * epoint_;            //!< emphasize point
  oyProfile_s * edit_;                   //!< editing colour space
  oyRectangle_s * window_geometry;
public:
  oyNamedColor_s  * mouse_3D_hit;       //!< a point recently hit by the mouse

private:
  void windowShape();

  // adapt inner struktures at data change
  void renewMenue_();
  int  createGLLists_();
  void adornText_();
  void adorn_();
  int  refresh_();      //!< refresh without init()

  // menues
  Fl_Menu_Button  *menue_;
  Fl_Menu_Button  *menue_button_;
  Fl_Menu_Button  *menue_cut_;
  Fl_Menu_Button  *menue_shape_;
  Fl_Menu_Button  *menue_background_;
  static void c_(Fl_Widget* w, void* data);
  
  // IDs
  Agviewer *agv_;
  static int  ref_;
  int  id_;
  int  type_;
  void GLinit_();
  void menueInit_();

  typedef enum {NOTALLOWED, AXES, RASTER, POINTS , SPEKTRUM, HELPER, CONTOURS, DL_MAX } DisplayLists;
  int gl_listen_[DL_MAX];
public:
  typedef enum {
   MENU_AXES,
   MENU_QUIT,
   MENU_SPHERE,           //!< form of 3DLut-representation
   MENU_CUBE,
   MENU_STAR,
   MENU_GRAY,            //!< the colour displaying of the 3DLut
   MENU_COLOUR,
   MENU_HIGHCONTRAST,
   MENU_ONIONSKIN,         //!< space of the 3DLut / plug-in candidate
   MENU_dE1SPHERE,        //!< meashurement-/profile differences with colour location
   MENU_dE2SPHERE,
   MENU_dE4SPHERE,
   MENU_dE1STAR,
   MENU_DIFFERENCE_LINE, //!< meashurement-/profile differences with put lines
   MENU_SPECTRAL_LINE,    //!< spectral colours as lines
   MENU_HELPER,          //!< show texts and arrows
   MENU_WHITE,           //!< background colour
   MENU_LIGHT_GRAY,
   MENU_GRAY_GRAY,
   MENU_DARK_GRAY,
   MENU_BLACK,
   MENU_MAX
  } MenuChoices;
private:
  int backgroundColourToMenuEntry( float farbe );

  int DrawAxes;

public:
  GL_View(int X,int Y,int W,int H);
  GL_View(int X,int Y,int W,int H, const char *l);
  ~GL_View();
  GL_View (const GL_View & gl)
    : Fl_Gl_Window(0,0,gl.w(),gl.h()), ThreadDaten() { id_ = ref_; ++ref_; copy(gl); }
  GL_View& copy(const GL_View& gl);
  GL_View& operator = (const GL_View& gl) { return copy(gl); }
private:
  void init_();
  void zero_();
  bool initialised_;
public:
  void init(int window);
  void savePreferences();

  // which window is managed?
  int  id()          {return id_; } //!< equal to agviewer::RedisplayWindow
  void id(int i)     { id_ = i; }
  int  type()   {return type_; } //!< window ID / display mode
  void type(int t_)   { type_ = t_; }

  static Agviewer* getAgv(GL_View *me, GL_View *referenz);
  void resetContexts ();

  // fltk virtual
  void redraw();
private:
  void draw();
  int  handle(int event);
  int  waiting_;       //!< dont generate and display new movement
  ICCnetz net;             //!< internal net representation, thread entry
  icc_examin_ns::BSPNODE *bsp;   //**< BSP tree root
  void loadNets_  (const icc_examin_ns::ICCThreadList<ICCnetz> & triangle_nets);
  void setBspProperties_( icc_examin_ns::BSPNODE * bsp );
  void setBspFaceProperties_( icc_examin_ns::FACE * faceList );
  void updateNet_    ();
public:
  void show();
  void hide();
  // redraw request from agv_
  void message( icc_examin_ns::Model* model, int info );

  // import data
  //TODO: use oyNamedColor_s
  void              namedColours (oyStructList_s * colours);
  oyStructList_s *  namedColours ();
  void              namedColoursRelease ();
  void emphasizePoint (oyNamedColor_s  * colour);  //!< a named colour
  void              clearNet ();
  icc_examin_ns::ICCThreadList<ICCnetz> triangle_nets;
  void achsNamen    (ICClist<std::string> achs_namen);

  void loadTable(ICClist<ICClist<ICClist<ICClist<double> > > >vect,
                               ICClist<std::string> fromColours,
                               ICClist<std::string> toColours,
                                       ICClist<int>        channels );
  ICClist<int>     channels() { return channels_; }
  void             channels( ICClist<int>channels ) { channels_ = channels; }

  // transparent displaying
  int  channel;               //!< selected channel
       // displaying of grid points of the transformation table
  int  point_form;           //!< MENU_SPHERE MENU_CUBE MENU_STAR
  int  point_colour;          //!< MENU_GRAY MENU_COLOUR MENU_HIGHCONTRAST
  int  point_size;        //!< size in pixel
  double pointRadius();     //*< estimated size of a point

  float background_colour;   //!< background colour / colour sheme
  float text_colour[3];
  float arrow_colour[3];
  float shadow;
  float line_mult;         //!< multiplicator
  char  line_1, line_2, line_3;
  int   onion_skin;            //!< MENU_ONIONSKIN

  // drawing functions
  void setPerspective();  //!< actualise perspektive
  void refreshTable();//!< glCompile for table
  void refreshPoints(); //!< glCompile for points
  void refreshNets();  //!< sort and drawing
  double window_proportion; //!< proportion of window
  static const double std_front_cut;
  double front_cut;    //!< front cut plane
  double cut_distance;      //!< thickness of the GL slice
  double level;             //*< level for table slicing 0...1
  double level_step;        //*< modification of level variable
  double cie_a_display_stretch; //!< direction CIE*a   for stretching
  double cie_b_display_stretch; //!< ~         CIE*b ; where CIE*L max is 1.0
  bool show_points_as_pairs;
  bool show_points_as_measurements;
  int  spectral_line;        //!< show spectral saturated colour line
  int  show_helpers;        //!< show arrows and text
  char text[128];           //!< Status line text
  void iccPoint3d                    ( oyPROFILE_e         projection,
                                       double            * vertex,
                                       double              radius );
private:
  void showSprectralLine_();
  void showContours_();
  // Debug
  void drawCoordinates_();

public:
  // display functions
  void drawGL();          //!< gl drawing
  int  keyEvents(int e);
  void menuEvents(int value);
  // Bewegungsfunktionen
  void move(bool setze);
  bool canMove();
  void canMove(int d);
  void invalidate(void) { Fl_Gl_Window::invalidate(); valid_ = 0; line_mult = SCALE(1.0); }
private:
  static void moveStatic_(void* GL_View);
private:
  double time_diff_;        //!< seconds per frame
  double time_;
  int  valid_;              //!< remembers valid() from within draw()
  int  update_geometries_;  //!< remembers creating GL lists from within draw()
  char t[128];              //!< text for searching errors
  int  mouse_x_;
  int  mouse_y_;
  int  mouse_x_old, mouse_y_old;
  bool mouse_stays;
  void mousePoint_( GLdouble & oX, GLdouble & oY, GLdouble & oZ,
                  GLdouble & X, GLdouble & Y, GLdouble & Z, int from_mouse );
public:
  // speed
  int  smooth;                    //!< smooth drawing
  int  blend;                     //!<   -"-

  // data informations
  const char* channelName() const {
                      if (to_channel_names_.size() &&
                          (int)to_channel_names_.size() > channel) {
                        return to_channel_names_[channel].c_str();
                      } else {
                        return "";}  }
  const char* channelName(unsigned int i) const {
                      if (to_channel_names_.size()>i) 
                        return (const char*)to_channel_names_[i].c_str();
                      else  return _("not available"); }
  unsigned int channel_count() {return (unsigned int)to_channel_names_.size(); }
};


#endif //ICC_GL_H
