/*
 * ICC Examin ist eine ICC Profil Betrachter
 * 
 * Copyright (C) 2004-2012  Kai-Uwe Behrmann 
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


#include "icc_examin.h"
#include "icc_formeln.h"
#include "icc_oyranos.h"
#include "icc_utils.h"
#include "icc_info.h"
#include "agviewer.h"
#include "icc_gl.h"
#include "icc_helfer.h"
#include "config.h"

#ifndef WIN32
#include <sys/time.h>
#endif

#include <oyranos.h>

#include <FL/Fl_Menu_Button.H>
#include <FL/Fl.H>
#include <FL/gl.h>
#include <FL/Fl_Preferences.H>
#include <FL/filename.H>
#include <FL/fl_draw.H>  /* fl_cursor() */

#ifdef HAVE_FTGL
// newer FTGL seems to deprecate FTFont.h
# if defined(FTGL_INLCUDE_VER)
#include <FTGL/ftgl.h>
# else
#include <FTGL/FTFont.h>
#include <FTGL/FTGLTextureFont.h>
#include <FTGL/FTGLPixmapFont.h>
#include <FTGL/FTGLPolygonFont.h>
#include <FTGL/FTGLExtrdFont.h>
# endif
# ifdef HAVE_FONTCONFIG
# include <fontconfig/fontconfig.h>
# endif
#endif

#include <cmath>
#include <iconv.h>    /* iconv */
#include <wchar.h>    /* wchar_t */

#include <X11/Xcm/Xcm.h>

#include <oyArray2d_s.h>
#include <oyImage_s.h>
#include <oyranos_colour.h>

#ifdef DEBUG_
#define MARK(x) DBG_S( #x ) x
#else
#define MARK(x) x
#endif

#define LIGHTING
//#define Lab_STERN 1

void drawCone( GLdouble width, GLdouble height, GLint steps );
void drawCone( GLdouble width, GLdouble height, GLint steps,
                   GLdouble x, GLdouble y, GLdouble z );

#ifdef DEBUG
#define DEBUG_ICCGL
#endif
#ifdef DEBUG_ICCGL
#define DBG_ICCGL_START DBG_PROG_START
#define DBG_ICCGL_ENDE DBG_PROG_ENDE
#define DBG_ICCGL_V( texte ) DBG_PROG_V( texte )
#define DBG_ICCGL_S( texte ) DBG_PROG_S( texte )
#define DBG_ICCGL2_S( texte )
#else
#define DBG_ICCGL_START
#define DBG_ICCGL_ENDE
#define DBG_ICCGL_V( texte )
#define DBG_ICCGL_S( texte )
#define DBG_ICCGL2_S( texte )
#endif

#define glStatus( txt, tuep ) icc_examin_ns::status_info( txt, tuep - 1 );


#define bToX(b) ((b)*cie_b_display_stretch - cie_b_display_stretch/2.)
#define LToY(L) ((L) - 0.5)
#define aToZ(a) ((a)*cie_a_display_stretch - cie_a_display_stretch/2.)
#define LabToXYZv(L,a,b) \
         (b*cie_b_display_stretch - cie_b_display_stretch/2.), \
         (L - 0.5), \
         (a*cie_a_display_stretch - cie_a_display_stretch/2.)
#define YToL(Y) ((Y) + .5)
#define ZToa(Z) ((Z) / cie_a_display_stretch + .5)
#define XTob(X) ((X) / cie_b_display_stretch + .5)

const double GL_View::std_front_cut = 4.2;
#ifdef HAVE_FTGL
FTFont *font = NULL, *ortho_font = NULL;
#endif

// set material colours
#define Lab2GL(L,a,b,A) { double lab[] = {L,a,b}; float colour[4]={0,0,0,A}; \
                      oyOptions_s * opts = icc_examin->options(); \
        double * rgb = icc_oyranos.wandelLabNachBildschirmFarben( \
             window()->x() + window()->w()/2, window()->y() + window()->h()/2, \
       (icc_oyranos.colourServerActive() & XCM_COLOR_SERVER_MANAGEMENT)?edit_:0, \
                                 lab, 1, opts); \
                      for(int i = 0; i < 3; ++i) \
                        colour[i] = rgb[i]; \
                      glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, colour); \
                      glColor4fv(colour); \
                      delete [] rgb; oyOptions_Release( &opts ); }
#define COLOUR(r,g,b,a) {colour[0] = (float)(r); colour[1] = (float)(g); colour[2] = (float)(b); \
                      colour[3] = (float)(a);  \
                      glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, colour); \
                      glColor4fv(colour); }

// draw text
#ifdef HAVE_FTGL
#  define drawTEXT(Font, ptr) { \
   glLineWidth(line_mult); \
    if(blend) glDisable(GL_BLEND); \
      glTranslated(.0,0,0.01); \
        glScaled(0.002,0.002,0.002); \
          if(Font) Font->Render(ptr); \
        glScaled(500,500,500); \
      glTranslated(.0,0,-.01); \
    if(blend) glEnable(GL_BLEND); \
   glLineWidth(line_mult); }
#else
# define drawTEXT(Font, ptr)
#endif

#ifdef HAVE_FTGL
#define drawOTEXT(Font, scal, buffer) { \
                                   glScaled(scal,scal*w()/(double)h(),scal); \
                                     drawText(Font, buffer); \
                                   glScaled(1.0/scal,1.0/(scal*w()/(double)h()),1.0/scal); \
                                 }
#else
#define drawOTEXT(Font, scal, buffer) {}
#endif

#ifdef HAVE_FTGL
void drawText( FTFont * f, const char * in_txt )
{
  float line_mult = 1.0;
  int blend = 1;

  size_t in_left,
         out_left;
  wchar_t * wchar, * wtmp;
  char * txt, * ttmp;
  int len = 0;

  if(strcmp(oy_domain_codeset,"UTF-32") != 0)
  {
    len = strlen( in_txt );

    in_left = len;
    out_left = ( len + 1 ) * sizeof(wchar_t);
    wtmp = wchar = (wchar_t*)calloc( len + 1, sizeof(wchar_t) );
    ttmp = txt = strdup(in_txt);

    iconv_t cd = iconv_open( "WCHAR_T", oy_domain_codeset );
    iconv( cd, &ttmp, &in_left, (char**)&wtmp, &out_left);
    iconv_close( cd );

    drawTEXT( f, wchar );
    if(wchar)free(wchar);
    if(txt)free(txt);

  } else
    drawTEXT( f, in_txt );

}
#endif

int GL_View::ref_ = 0;

GL_View::GL_View(int X,int Y,int W,int H)
  : Fl_Gl_Window(X,Y,W,H)
{ DBG_PROG_START
  id_ = ref_;
  ++ref_;
  init_();
  DBG_PROG_ENDE
}

GL_View::GL_View(int X,int Y,int W,int H, const char *l)
  : Fl_Gl_Window(X,Y,W,H,l)
{ DBG_PROG_START
  id_ = ref_;
  ++ref_;
  init_();
  DBG_PROG_ENDE
}

void
GL_View::zero_()
{ DBG_PROG_START

  initialised_ = false;

  agv_ = NULL;

  channel = 0;
  cut_distance = 0.01;
  front_cut = std_front_cut;
  cie_a_display_stretch = 1.0;
  cie_b_display_stretch = 1.0;
  onion_skin = 5;
  point_form = MENU_dE1STAR;
  point_colour = 0;
  point_size = 8;
  //punkt_zahl_old = 0;
  background_colour = 0.75;
  spectral_line = 0;
  show_helpers = true;
  memset( text, 0, 128 );
  show_points_as_pairs = false;
  show_points_as_measurements = false;
  type_ = -1;
  line_mult = 1.0;
  line_1 = 1;
  line_2 = 2;
  line_3 = 3;
  blend = false;
  smooth = false;
  waiting_ = 1;
  memset( t, 0, 128 );
  mouse_x_ = 0;
  mouse_y_ = 0;
  mouse_x_old = -1;
  mouse_y_old = -1;
  mouse_stays = false;
  level = 0.5001;
  level_step = 0.1;
  valid_ = false;
  update_geometries_ = false;
  time_ = 0;
  text[0] = 0;
  colours_ = 0;
  epoint_ = 0;
  oyProfile_s * prof = oyProfile_FromStd( oyEDITING_XYZ, NULL );
  mouse_3D_hit = oyNamedColour_Create( NULL, NULL,0, prof, 0 );
  oyProfile_Release( &prof );
  window_geometry = NULL;
  edit_ = NULL;

  for (int i=1; i < 3 ; ++i) text_colour[i] = 0.5;
  for (int i=1; i < 3 ; ++i) arrow_colour[i] = 0.5;

  for(int i = 0; i < DL_MAX; ++i)
    gl_listen_[i] = 0;
  DBG_S( id_ << ": zero RASTER " << gl_listen_[RASTER], ICC_MSG_WARN );

  bsp = 0;

  DBG_PROG_ENDE
}

void
GL_View::init_()
{ DBG_PROG_START

  zero_();
  agv_ = this->getAgv(this, NULL);

  edit_ = oyProfile_Copy( icc_oyranos.getEditingProfile(), NULL );

  window_geometry = oyRectangle_NewWith( x(), y(), w(), h(), NULL );

  DBG_PROG_ENDE
}

GL_View::~GL_View()
{ DBG_PROG_START
  if (gl_listen_[RASTER]) {
    DBG_PROG_S( "delete glListe " << gl_listen_[RASTER] )
    //DBG_S( id_ << ": erase RASTER " << gl_listen_[RASTER], ICC_MSG_WARN );
    glDeleteLists (gl_listen_[RASTER],1);
    gl_listen_[RASTER] = 0;
  }
  if (gl_listen_[HELPER]) {
    DBG_PROG_S( "delete glListe " << gl_listen_[HELPER] )
    glDeleteLists (gl_listen_[HELPER],1);
    gl_listen_[HELPER] = 0;
  }
  if (gl_listen_[POINTS]) {
    DBG_PROG_S( "delete glListe " << gl_listen_[POINTS] )
    glDeleteLists (gl_listen_[POINTS],1);
    gl_listen_[POINTS] = 0;
  }
  if (gl_listen_[SPEKTRUM]) {
    DBG_PROG_S( "delete glListe " << gl_listen_[SPEKTRUM] )
    glDeleteLists (gl_listen_[SPEKTRUM],1);
    gl_listen_[SPEKTRUM] = 0;
  }
  if (gl_listen_[CONTOURS]) {
    DBG_PROG_S( "delete glListe " << gl_listen_[CONTOURS] )
    glDeleteLists (gl_listen_[CONTOURS],1);
    gl_listen_[CONTOURS] = 0;
  }
# ifdef HAVE_FTGL
  //if(font) delete font;
  //if(ortho_font) delete ortho_font;
# endif

  namedColoursRelease();
  oyProfile_Release( &edit_ );

  --ref_;

  doLocked_m( std::string loc_old = setlocale(LC_NUMERIC, NULL);,NULL) //getenv("LANG");
  if(loc_old.size()) {
    DBG_NUM_V( loc_old )
  } else {
    DBG_NUM_S( "LANG variable not found" )
  }
  doLocked_m( setlocale(LC_NUMERIC,"C");,NULL);

  this->savePreferences();

  if(loc_old.size())
    doLocked_m( setlocale(LC_NUMERIC,loc_old.c_str()) , NULL);


  DBG_PROG_ENDE
}

void GL_View::savePreferences()
{
  Fl_Preferences gl( Fl_Preferences::USER, "oyranos.org", "iccexamin");
  switch (type_)
  {
    case 1:
    {
      Fl_Preferences gl_cube(gl, "gl_cube");
      gl_cube.set("spectral_line", spectral_line );
      gl_cube.set("background_colour", background_colour );
      gl_cube.set("show_helpers", show_helpers );
      gl_cube.set("onion_skin", onion_skin );

      DBG_PROG_V( spectral_line )
    } break;
    case 2:
    {
      Fl_Preferences gl_gamut(gl, "gl_gamut");
      gl_gamut.set("spectral_line", spectral_line );
      gl_gamut.set("background_colour", background_colour );
      gl_gamut.set("show_helpers", show_helpers );
      gl_gamut.set("onion_skin", onion_skin );

      DBG_PROG_V( spectral_line )
    } break;
  }
}

void GL_View::resetContexts ()
{
  /* reset the to be copied contexts */
  for(int i = 0; i < DL_MAX; ++i)
  {
    if( gl_listen_[i] )
      glDeleteLists( gl_listen_[i], 1 );
    gl_listen_[i] = 0;
  }
}


GL_View&
GL_View::copy (const GL_View & gl)
{ DBG_PROG_START

  /* preserve argv_ */
  Agviewer * tmp = this->agv_;
  zero_();
  this->agv_ = tmp;

  table_ = gl.table_;
  to_channel_names_ = gl.to_channel_names_;
  from_channel_names_ = gl.from_channel_names_;
  channels_ = gl.channels_;
  colours_ = oyStructList_Copy( gl.colours_, NULL );
  epoint_ = oyNamedColour_Copy( gl.epoint_, 0 );
  mouse_3D_hit = oyNamedColour_Copy( gl.mouse_3D_hit, 0 );
  edit_ = oyProfile_Copy( gl.edit_, NULL );

  type_ = gl.type_;

  waiting_ = gl.waiting_;
  bsp = 0;

  channel = gl.channel;
  point_form = gl.point_form;
  point_colour = gl.point_colour;
  point_size = gl.point_size;
  background_colour = gl.background_colour;
  text_colour[0] = gl.text_colour[0];
  text_colour[1] = gl.text_colour[1];
  text_colour[2] = gl.text_colour[2];
  arrow_colour[0] = gl.arrow_colour[0];
  arrow_colour[1] = gl.arrow_colour[1];
  arrow_colour[2] = gl.arrow_colour[2];
  shadow = gl.shadow;
  line_mult = gl.line_mult;
  line_1 = gl.line_1;
  line_2 = gl.line_2;
  line_3 = gl.line_3;
  onion_skin = gl.onion_skin;

  window_proportion = gl.window_proportion;
  front_cut = gl.front_cut;
  cut_distance = gl.cut_distance;
  level = gl.level;
  level_step = gl.level_step;
  cie_a_display_stretch = gl.cie_a_display_stretch;
  cie_b_display_stretch = gl.cie_b_display_stretch;
  show_points_as_pairs = gl.show_points_as_pairs;
  show_points_as_measurements = gl.show_points_as_measurements;
  spectral_line = gl.spectral_line;
  show_helpers = gl.show_helpers;

  time_ = gl.time_;
  valid_ = gl.valid_;
  mouse_x_ = gl.mouse_x_;
  mouse_y_ = gl.mouse_y_;
  mouse_x_old = gl.mouse_x_old;
  mouse_y_old = gl.mouse_y_old;
  mouse_stays = gl.mouse_stays;

  smooth = gl.smooth;
  blend = gl.blend;

# ifdef HAVE_FTGL
  font = ortho_font = 0;
# endif

  // initialise
  menueInit_();

  DBG_PROG_ENDE
  return *this;
}

/**
 *  Use this function, including mode(), outside of Fl_Gl_Window::draw().
 *
 *  @see http://www.fltk.org/str.php?L1945
 */
void
GL_View::init(int ty)
{ DBG_PROG_START

  if(initialised_)
    return;

  initialised_ = true;

  // 1 fuer mft_gl und 2 fuer DD_farbraum
  type_ = ty;

  DBG_PROG

  doLocked_m( std::string loc_old = setlocale(LC_NUMERIC, NULL);,NULL) //getenv("LANG");
  if(loc_old.size()) {
    DBG_NUM_V( loc_old )
  } else {
    DBG_NUM_S( "LANG variable not found" )
  }
  doLocked_m( setlocale(LC_NUMERIC,"C");,NULL);

  Fl_Preferences gl( Fl_Preferences::USER, "oyranos.org", "iccexamin");

  switch(type_)
  {
    case 1:
    {
      Fl_Preferences gl_cube(gl, "gl_cube");
      gl_cube.get("spectral_line", spectral_line, 0 );
      gl_cube.get("background_colour", background_colour, 0.75 );
      gl_cube.get("show_helpers", show_helpers, true );
      gl_cube.get("onion_skin", onion_skin, 5 );
    } break;
    case 2:
    {
      Fl_Preferences gl_gamut(gl, "gl_gamut");
      gl_gamut.get("spectral_line", spectral_line, 0 );
      gl_gamut.get("background_colour", background_colour, 0.75 );
      gl_gamut.get("show_helpers", show_helpers, true );
      gl_gamut.get("onion_skin", onion_skin, 5 );
    } break;
  }

  if(loc_old.size())
    doLocked_m( setlocale(LC_NUMERIC,loc_old.c_str()) , NULL);

  DBG_PROG_V( spectral_line )

  resizable(0);

# define TEST_GL(modus) { \
    this->mode(modus); \
    if(this->can_do()) { \
      mod |= modus; \
      this->mode(mod); DBG_PROG_S( "can_do(): " << modus ); \
      DBG_PROG_S( "OpenGL understand: yes  " << #modus <<" "<< this->mode() ) \
    } else {  printf("can_do() false: %d\n", modus); \
      DBG_PROG_S( "OpenGL understand: no   " << #modus <<" "<< this->mode() ) \
    } \
  }

  long mod = 0;
  TEST_GL(FL_RGB)
  TEST_GL(FL_DOUBLE)
  TEST_GL(FL_ALPHA)
  TEST_GL(FL_DEPTH)
  TEST_GL(FL_MULTISAMPLE)
  mode(mod);
  DBG_PROG_S( "OpenGL mode: " << mode() )

  DBG_PROG

  agv_->agvInit(id_);

  if (type_ > 1)
  {
    DBG_PROG_S("gl window " << id_)
    cie_a_display_stretch = 2.55;
    cie_b_display_stretch = 2.55;
    agv_->distA (agv_->distA()
                           + cie_a_display_stretch/2.0 - 0.5);
    agv_->distB (agv_->distB()
                           + cie_b_display_stretch/2.0 - 0.5);
    agv_->eyeDist (agv_->dist()*2.0);
  }

  // initialise
  menueInit_(); DBG_PROG
  menuEvents (backgroundColourToMenuEntry(background_colour)); // colour sheme
  int schalen_old = onion_skin;
  menuEvents (MENU_COLOUR);     // CLUT colour sheme
  onion_skin = schalen_old;
  shadow = 0.1f;
  if (type() == 1) menuEvents (MENU_CUBE);

  mouse_stays = false;

  DBG_PROG_ENDE
}

/** @brief localise the position */
void
GL_View::mousePoint_( GLdouble &oX, GLdouble &oY, GLdouble &oZ,
                        GLdouble &X, GLdouble &Y, GLdouble &Z, int from_mouse )
{
  DBG_PROG_START  
  // localise position
  // how far is the next object in this direction, very laborious
  GLfloat zBuffer = 0;
  glReadPixels((GLint)mouse_x_,(GLint)h()-mouse_y_,1,1,GL_DEPTH_COMPONENT, GL_FLOAT, &zBuffer);
  GLdouble model_matrix[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
           projection_matrix[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
  GLint display_coords[4] = {0,0,0,0};
  glGetDoublev(GL_MODELVIEW_MATRIX, model_matrix);
  glGetDoublev(GL_PROJECTION_MATRIX, projection_matrix);
  glGetIntegerv(GL_VIEWPORT, display_coords);
  if(from_mouse)
    gluUnProject(mouse_x_, h()-mouse_y_, zBuffer,
                 model_matrix, projection_matrix, display_coords,
                 &oX, &oY, &oZ);


  DBG_PROG_V( "X: "<<oX<<" Y: "<<oY<<" Z: "<<oZ<<" "<<id_ )

  gluProject( oX, oY, oZ,
              model_matrix, projection_matrix, display_coords,
              &X,&Y,&Z);
  DBG_PROG_ENDE
}

static int zahl = 0;
void
GL_View::moveStatic_ (void* gl_a)
{
  DBG_ICCGL_START

  GL_View *gl_view = (GL_View*)gl_a;

  if (!gl_view) {
      WARN_S( "no GL_View provided " << gl_view->id_ )
      return;
  }

  // actualise UI
  gl_view->waiting_ = 0;
  icc_examin_ns::wait( 0.0, true );
  gl_view->waiting_ = 1;

  if(gl_view->canMove())
  {
    double drawing_sleep = 0;  // no endless wait queues
    if(!icc_examin->frei())
    {
      zahl = 0;
      DBG_ICCGL_S( "redraw not allowed " << gl_view->id_ )
    } else {
      double time = icc_examin_ns::zeitSekunden();

      if (time - gl_view->time_ < 1./25.) {
        drawing_sleep = 1./25. - gl_view->time_;
      } else {
        gl_view->redraw();
        zahl++;
      }

    }
    // short wait
    Fl::repeat_timeout( MIN(0.01,drawing_sleep), moveStatic_, gl_a );

    DBG_ICCGL_S( "Pause " << gl_view->time_diff_<<" "<<drawing_sleep <<" "<< " "<< gl_view->id_ <<" "<<zahl<<" "<< gl_view->canMove())
  } 
  DBG_ICCGL_ENDE
}

bool GL_View::canMove()        { return agv_ && agv_->can_move_; }
void GL_View::canMove(int d)
{
  agv_->can_move_ = d?true:false; 
  if (d)
    Fl::add_timeout(0.04, moveStatic_,this);

  DBG_ICCGL_S("Stop Spin "<<zahl<<" "<<d)
}

void
GL_View::move (bool setze)
{
  DBG_ICCGL_START
  canMove( setze );
  DBG_ICCGL_V( setze )
  if(!setze) {
    agv_->agvSwitchMoveMode (Agviewer::AGV_STOP);
    agv_->notify(ICCexamin::GL_STOP);
  }
  DBG_ICCGL_ENDE
}

int
GL_View::refresh_()
{
  DBG_PROG_START

  renewMenue_();

  update_geometries_ = true;

  DBG_PROG_ENDE
  return 0;
}

void
GL_View::hide()
{   
  DBG_PROG_START
  DBG_PROG_V( visible()<<" "<<shown() )
  icc_examin->alle_gl_fenster->observerDel(this);
  icc_oyranos.colourServerRegionSet( this, NULL, window_geometry, 1 );
  Fl_Gl_Window::hide();
  DBG_PROG_ENDE
}     

void
GL_View::show()
{   
  DBG_PROG_START
  DBG_PROG_V( visible()<<" "<<shown() )
  icc_examin->alle_gl_fenster->observerAdd(this);
  if( window()->visible() )
    Fl_Gl_Window::show();
  DBG_PROG_ENDE
}     


/** receive of a drawing news */
void
GL_View::message(icc_examin_ns::Model* model, int info)
{
  DBG_PROG_START
  DBG_PROG_V( info<<" "<<window()->visible()<<" "<<visible()<<" "<<shown()<<" "<<id_ )

  if( visible() && shown() && !agv_->parent->visible() )
    agv_->reparent(this);

  if( info == ICCexamin::GL_AUFFRISCHEN )
  {
    invalidate();
    refresh_();
    redraw();
  }

  if( info && visible() )
    Fl_Gl_Window::redraw();
  if(info == ICCexamin::GL_STOP)
  {
    canMove( false );
  }

  DBG_PROG_ENDE
}

/** send of a drawing news to all observers of Agviewer
 *  this makes moving the viewes syncronised
 */
void
GL_View::redraw()
{
  DBG_PROG_START

  if(!waiting_) {
    DBG_PROG_S( "block redraw" )
    DBG_PROG_ENDE
    return;
  }

  int thread = wandelThreadId(iccThreadSelf());
  if(thread != THREAD_HAUPT) {
    Fl::awake(this);
    DBG_PROG_ENDE
    return;
  }

  if(agv_)
    agv_->notify(ICCexamin::GL_ZEICHNEN);
  DBG_PROG_ENDE
}

void
GL_View::draw()
{
  DBG_PROG_START
  --zahl;
  DBG_PROG_S( "entrance canMove(): "
               << canMove()<<" "<<id_<<" colour: "<<background_colour )
  int thread = wandelThreadId(iccThreadSelf());
  if(thread != THREAD_HAUPT) {
    WARN_S( ": wrong thread" );
    DBG_PROG_ENDE
    return;
  }

  if(!visible() || !shown() || !icc_examin->frei())
  {
    DBG_PROG_ENDE
    return;
  }

  agv_->agvMove_();

  if(!valid())
    valid_ = false;

  drawGL();

  DBG_PROG_V( triangle_nets.size() )

  DBG_PROG_ENDE
}

Agviewer*
GL_View::getAgv( GL_View *view, GL_View *reference )
{ DBG_PROG_START
  Agviewer *agv = NULL;

  if(!agv && reference)
  {
    agv = reference->agv_;
    agv->reparent( view );
  }

  if(!agv)
    agv = new Agviewer (view);

  if(view->agv_)
    view->modelDel( view->agv_ );

  view->agv_ = agv;

  view->modelAdd( view->agv_ );

  DBG_ICCGL_ENDE
  return agv;
}

void
GL_View::GLinit_()
{ DBG_PROG_START
  GLfloat mat_ambuse[] = { 0.2f, 0.2f, 0.2f, 1.0f };
  GLfloat mat_specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };

# ifdef LIGHTING
  glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, mat_ambuse);
  glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, mat_specular);
  glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 50.0);
# if 0
  GLfloat light0_position[] = { -2.4, -1.6, -1.2, 0.0 };
  glLightfv(GL_LIGHT0, GL_POSITION, light0_position);
  //GLfloat light1_position[] = { -2.4, -1.6, -1.2, 0.0 };
  //glLightfv(GL_LIGHT1, GL_POSITION, light1_position);
  glEnable(GL_LIGHTING);
  glEnable(GL_LIGHT0);
# else
  GLfloat light0_position[] = { -2.4f, -1.6f, -1.2f, 0.0f };
  glLightfv(GL_LIGHT0, GL_POSITION, light0_position);
  glDisable(GL_LIGHT0);

  glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_TRUE);
  glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);

  glEnable(GL_LIGHTING);
# endif
# endif
  //glEnable(GL_LIGHT1);
# ifndef LIGHTING
  glDisable(GL_LIGHTING);
# endif

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  glEnable(GL_NORMALIZE);

  glDepthFunc(GL_LESS);
  glEnable(GL_DEPTH_TEST);

  //glShadeModel(GL_SMOOTH);
  glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);

# ifdef HAVE_FTGL
  if(font) delete font;
  if(ortho_font) delete ortho_font;

  char * font_name = strdup("/usr/share/fonts/dejavu/DejaVuSans.ttf");

# ifdef HAVE_FONTCONFIG
  FcResult r = FcResultMatch;
  FcPattern * pat = FcPatternBuild (0, FC_SCALABLE, FcTypeBool, true,
                                       NULL );
  pat = FcNameParse( (FcChar8 *)"DejaVu Sans" );
  FcObjectSet * os = FcObjectSetBuild(FC_FAMILY, FC_STYLE, (void *)0);
  FcFontSet * font_list = FcFontList(0, pat, os);
  FcObjectSetDestroy( os );
  FcPatternDestroy( pat );
  int fc_n = font_list->nfont;
  for(int i = 0; i< fc_n; ++i)
  {
    {
      FcChar8 * fname = NULL;
      
      FcPattern* match = FcFontMatch(0, font_list->fonts[i], &r);
      r = FcPatternGetString( match, FC_FILE, 0, &fname );
      if( r == FcResultMatch &&
          fname && strstr((const char*)fname, "DejaVuSans.ttf") != NULL )
        font_name = strdup( fname );
      FcPatternDestroy( match ); match = 0;
    }
  }
  FcFontSetDestroy( font_list );
# endif

# if APPLE
  std::string f_n;
  if(getenv("RESOURCESPATH"))
  {
    f_n = getenv("RESOURCESPATH");
    f_n += "/share/FreeSans.ttf";
  }
  if(!f_n.size()) {
    f_n = icc_examin_ns::holeBundleResource("FreeSans", "ttf");
  }
  if(f_n.size()) {
    font_name = strdup(f_n.c_str()); DBG_PROG_S( f_n )
  }
# endif
  if(!holeDateiModifikationsZeit(font_name)) {
    DBG_PROG_S( _("Could not open font in:") << font_name )
    font_name = strdup("/usr/share/fonts/truetype/DejaVuSans.ttf");
    if(!holeDateiModifikationsZeit(font_name)) {
      DBG_PROG_S( _("Could not open font in:") << font_name )
#ifdef  WIN32
      font_name = strdup("C:\\Windows\\Fonts\\arial.ttf");
#else
      font_name = strdup("/Library/Fonts/Arial.ttf");
#endif
      if(!holeDateiModifikationsZeit(font_name)) {
        DBG_PROG_S( _("Could not open font in:") << font_name )
        char *n = (char*) calloc(sizeof(char), 1024);
#ifdef  WIN32
        sprintf (n, "%s%s.fonts%sarial.ttf", getenv("HOME"),
                 ICC_DIR_SEPARATOR, ICC_DIR_SEPARATOR);
#else
        sprintf (n, "%s/.fonts/arial.ttf", getenv("HOME"));
#endif
        font_name = n;
        DBG_PROG_V( holeDateiModifikationsZeit(font_name) )
        if(!holeDateiModifikationsZeit(font_name)) {
          DBG_PROG_S( _("Could not open font in:") << font_name )
          //sprintf (n, "%s/fonts/FreeSans.ttf", ICCEXAMIN_DATADIR);
          font_name = n;
          if(!holeDateiModifikationsZeit(font_name)) {
            DBG_PROG_S( _("Could not open font in:") << font_name )
            sprintf (n, "%s/DejaVuSans.ttf", SRCDIR);
            font_name = n;
          }
        }
      }
    }
  }
  font = new FTGLExtrdFont( font_name );
  ortho_font = new FTGLPixmapFont( font_name );
  DBG_MEM_V( (int*)font <<" "<< (int*)ortho_font )
  if(font->Error()) {
    delete font;
    delete ortho_font;
    font = ortho_font = 0;
    WARN_S( _("Could not open font in:") << font_name )
  } else {
    font->CharMap( ft_encoding_unicode );
    ortho_font->CharMap( ft_encoding_unicode );
    font->Depth(12);
    if(!font->FaceSize(72)) WARN_S("Fontsize not setable"); \
    if(!ortho_font->FaceSize(16)) WARN_S("Fontsize not setable");
  }
# endif

  glFlush();
  DBG_PROG_ENDE
}

void
drawCone( GLdouble width, GLdouble height, GLint steps )
{ DBG_ICCGL_START
  GLdouble x, y,
           s = 2*M_PI/(GLdouble)steps, // static variable
           hn = width*tan(width/2./height); // normal hight
  // bottom
  glBegin(GL_TRIANGLE_FAN);
    glNormal3d( 0, 0, -1 );
    glVertex3d( 0, 0, 0 );
    for(int i = 0; i <= steps; ++i)
      glVertex3d( cos(i*s)*width, sin(i*s)*width, 0 );
  glEnd();
  // cone
  glBegin(GL_TRIANGLE_STRIP);
    for(int i = 0; i <= steps; ++i)
    {
      x = cos(i*s)*width;
      y = sin(i*s)*width;
      glNormal3d( x, y, hn );
      glVertex3d( x, y, 0 );
      glVertex3d( 0, 0, height );
    }
  glEnd();

  DBG_ICCGL_ENDE
}

void
drawCone( GLdouble width, GLdouble height, GLint steps ,
              GLdouble x, GLdouble y, GLdouble z )
{ DBG_ICCGL_START
  GLdouble xk, yk,
           s = 2*M_PI/(GLdouble)steps, // static variable
           hn = width*tan(width/2./height); // hight of normal
  // bottom
  glBegin(GL_TRIANGLE_FAN);
    glNormal3d( 0, 0, -1 );
    glVertex3d( x, y, z );
    for(int i = 0; i <= steps; ++i)
      glVertex3d( x+cos(i*s)*width, y+sin(i*s)*width, z );
  glEnd();
  // cone
  glBegin(GL_TRIANGLE_STRIP);
    for(int i = 0; i <= steps; ++i)
    {
      xk = cos(i*s)*width;
      yk = sin(i*s)*width;
      glNormal3d( xk, yk, hn );
      glVertex3d( x+xk, y+yk, z+height );
      glVertex3d( x, y, z );
    }
  glEnd();

  DBG_ICCGL_ENDE
}


void
GL_View::drawCoordinates_()
{ DBG_ICCGL_START
  char text[256];

  // coordinate axis
    glBegin(GL_LINES);
        glVertex3d(.1, 0, 0); glVertex3f(0, 0, 0);
    glEnd();
    glBegin(GL_LINES);
        glVertex3d(0, 0, .1); glVertex3f(0, 0, 0);
    glEnd();
    glBegin(GL_LINES);
        glVertex3d(0, .1, 0); glVertex3f(0, 0, 0);
    glEnd();

    glRotated (90,0.0,0,1.0);
      glMatrixMode(GL_MODELVIEW);
      glTranslated((0.0-0.3),(0.0-0.1),(0.0-0.05));
        strcpy (&text[0],_("0,0,0"));
        drawTEXT(font, text)
      glTranslated(0.3,0.1,0.05);
    glRotated (-90,0.0,0,1.0);

    Lab2GL(1,.5,.5,1)
    glTranslated(.1,0,0);
      Lab2GL(.9,0,-1.0,1)
      glRotated (90,0.0,1.0,.0);
        drawCone(0.01, 0.025, 8);
      glRotated (-90,0.0,1.0,.0);
      Lab2GL(1,.5,.5,1)
      glTranslated(.02,0,0);
        drawTEXT(font, "X")
      glTranslated((-0.02),0,0);
    glTranslated((-0.1),0,0);

    glTranslated(.0,.1,0);
      glRotated (270,1.0,.0,.0);
        Lab2GL(1,0.0,1.0,1)
        drawCone(0.01, 0.025, 8);
      glRotated (90,1.0,.0,.0);
      glRotated (90,0.0,.0,1.0);
        Lab2GL(1,.5,.5,1)
        drawTEXT(font, "Y")
      glRotated (270,0.0,.0,1.0);
    glTranslated(.0,(-0.1),0);

    glTranslated(0,0,.1);
      Lab2GL(0.9,0,-1.0,1)
      drawCone(0.01, 0.025, 8);
      glRotated (90,0.0,.5,.0);
        glTranslated(-.1,0,0);
          Lab2GL(1,.5,.5,1)
          drawTEXT(font, "Z")
        glTranslated(.1,0,0);
      glRotated (270,0.0,.5,.0);
    glTranslated(0,0,-.1);
  DBG_ICCGL_ENDE
}

int
GL_View::createGLLists_()
{ DBG_PROG_START

  if(!frei())
    return 1;

  MARK( frei(false); )

  adorn_();

  refreshTable();

  // actualise shodow
  static char actives[64];
  static char gray[64];
  char aktualised = false;

  triangle_nets.frei(false);
  for(int i = 0; i < (int)triangle_nets.size(); ++i)
  {
    double netze_n = triangle_nets.size();
    double shade = .93 - .8/netze_n*i;
    shade = 1.0 - i/(double)netze_n;
    if(triangle_nets[i].schattierung < 0)
      triangle_nets[i].schattierung = shade;

    if( triangle_nets[i].active() != actives[i] ||
        triangle_nets[i].grau != gray[i]  )
    {
      actives[i] = triangle_nets[i].active();
      gray[i] = triangle_nets[i].grau;
      if(!aktualised)
      {
        //showContours_();
        aktualised = true;
      }
    }
  }
  triangle_nets.frei(true);

  MARK( frei(true); )
  updateNet_();
  MARK( frei(false); )

  triangle_nets.frei(false);
  if(triangle_nets.size())
    showContours_();
  triangle_nets.frei(true);
 
  showSprectralLine_();

  DBG_PROG_V( point_form <<" "<< MENU_dE1STAR )
  triangle_nets.frei(false);
  refreshPoints();
  triangle_nets.frei(true);

  //background
  { double lab[] = {background_colour,.5,.5};
                      oyOptions_s * opts = icc_examin->options();
        double * rgb = icc_oyranos.wandelLabNachBildschirmFarben(
             window()->x() + window()->w()/2, window()->y() + window()->h()/2,
       (icc_oyranos.colourServerActive() & XCM_COLOR_SERVER_MANAGEMENT)?edit_:0,
                                 lab, 1, opts);
    glClearColor(rgb[0],rgb[1],rgb[2],1.0);
    delete [] rgb; oyOptions_Release( &opts );
  }

  MARK( frei(true); )

  DBG_PROG_ENDE
  return 0;
}

void
GL_View::adornText_()
{
  DBG_PROG_START
  char text[256];
  char* ptr = 0;
  GLfloat ueber = 0.035f;

    glDisable(GL_TEXTURE_2D);
    glDisable(GL_LIGHTING);
    Lab2GL(text_colour[0],text_colour[1],text_colour[2],1)

    // CIE*L - top
    glPushMatrix();
      glLoadIdentity();
      if (from_channel_names_.size())
      {
        ptr = (char*) from_channel_names_[0].c_str();
        sprintf (&text[0], "%s", ptr);
        glRasterPos3d (0, .5+ueber, 0);
        drawOTEXT(ortho_font, 1, text)
      }

    // CIE*a - right
      if (from_channel_names_.size() > 1)
      {
        ptr = (char*) from_channel_names_[1].c_str();
        sprintf (&text[0], "%s", ptr);
        if (from_channel_names_.size() &&
            from_channel_names_[1] == _("CIE *a"))
          glRasterPos3d (.0, -.5, cie_a_display_stretch/2.+ueber);
        else
          glRasterPos3d (.0, .0, cie_a_display_stretch/2.+ueber);
        drawOTEXT(ortho_font, 1, text)
      }

    // CIE*b - left
      if (from_channel_names_.size() > 2)
      {
        ptr = (char*) from_channel_names_[2].c_str();
        sprintf (&text[0], "%s", ptr);
        if (from_channel_names_.size() &&
            from_channel_names_[2] == _("CIE *b"))
          glRasterPos3d (cie_b_display_stretch/2.+ueber, -.5, .0);
        else
          glRasterPos3d (cie_b_display_stretch/2.+ueber, .0, .0);
        drawOTEXT(ortho_font, 1, text)
      }
    glPopMatrix();

  DBG_PROG_ENDE
}

void
GL_View::adorn_()
{
  DBG_PROG_START

# define ARROW_TIP drawCone(0.02, 0.05, 16);

  DBG_PROG_V( id() )
  // arrow and text
  if (gl_listen_[HELPER]) {
    glDeleteLists (gl_listen_[HELPER], 1);
  }

  GL_View::gl_listen_[HELPER] = glGenLists(1);

  glNewList( gl_listen_[HELPER], GL_COMPILE); DBG_PROG_V( gl_listen_[HELPER] )
    glLineWidth(line_3*line_mult);

    // colour space channel name
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    if(!smooth) {
      glDisable(GL_BLEND);
      glDisable(GL_LINE_SMOOTH);
    } else {
      glEnable(GL_BLEND);
      glEnable(GL_LINE_SMOOTH);
    }
    glEnable(GL_LIGHTING);

    // CIE*L - top
    glPushMatrix();
      Lab2GL(arrow_colour[0],arrow_colour[1],arrow_colour[2],1)
      glTranslatef(0,.5,0);
      glRotatef (270,1.0,0.0,.0);
      ARROW_TIP
    glPopMatrix(); DBG_PROG
    glPushMatrix();
    glDisable(GL_LIGHTING);
      glBegin(GL_LINES);
        glVertex3f(0, .5, 0); glVertex3f(0, -.5, 0);
      glEnd();
    glEnable(GL_LIGHTING);
    glPopMatrix();

    // CIE*a - right
    glPushMatrix();
      if (from_channel_names_.size() > 1 &&
          from_channel_names_[1] == _("CIE *a"))
        glTranslatef(0,-.5,0);
      Lab2GL(arrow_colour[0],arrow_colour[1],arrow_colour[2],1)
      float schritt = .25, Schritt = 1., start, ende;
      start = (float)(- floor (cie_a_display_stretch/2./schritt) * schritt);
      ende  = (float)(  floor (cie_a_display_stretch/2./schritt) * schritt);
      // grid
      glDisable(GL_LIGHTING);
      for(float i = start; i <= ende; i+=schritt) {

        if(i/Schritt == floor(i/Schritt))
          glLineWidth(line_3*line_mult);
        else
          glLineWidth(line_1*line_mult);

        glBegin(GL_LINES);
          glVertex3d( i, 0,  cie_a_display_stretch/2.);
          glVertex3d( i, 0, -cie_a_display_stretch/2.);
        glEnd();
      }
      glEnable(GL_LIGHTING);

      glTranslated(0.0,0.0,cie_a_display_stretch/2.);
      glRotated (180,0.0,.5,.0);
      glTranslated(.0,0.0,cie_a_display_stretch);
      if (from_channel_names_.size() > 1 &&
          from_channel_names_[1] == _("CIE *a"))
      {
        Lab2GL(1.,-1.0,0.5,1)
        ARROW_TIP
      }
      glTranslated(.0,0.0,-cie_a_display_stretch);
      glRotated (180,0.0,.5,.0);
      if (from_channel_names_.size() > 1 &&
          from_channel_names_[1] == _("CIE *a"))
        Lab2GL(1.,1.0,0.5,1)
      ARROW_TIP
    glPopMatrix(); DBG_PROG

    // CIE*b - left
    glPushMatrix();
      if (from_channel_names_.size() > 2 &&
          from_channel_names_[2] == _("CIE *b"))
        glTranslated(0,-0.5,0);
      Lab2GL(arrow_colour[0],arrow_colour[1],arrow_colour[2],1)
      // grid
      glDisable(GL_LIGHTING);
      for(float i = start; i <= ende; i+=schritt) {

        if(i/Schritt == floor(i/Schritt))
          glLineWidth(line_3*line_mult);
        else
          glLineWidth(line_1*line_mult);

        glBegin(GL_LINES);
          glVertex3d( cie_b_display_stretch/2., 0, i);
          glVertex3d(-cie_b_display_stretch/2., 0, i);
        glEnd();
      }
      glEnable(GL_LIGHTING);
      glTranslated(cie_b_display_stretch/2.,0,0);
      if (from_channel_names_.size() > 2 &&
          from_channel_names_[2] == _("CIE *b"))
        Lab2GL(1.,0.5,1.0,1)
      glRotated (90,0.0,.5,.0);
      ARROW_TIP
      glRotated (180,.0,.5,.0);
      glTranslated(.0,.0,cie_b_display_stretch);
      if (from_channel_names_.size() > 2 &&
          from_channel_names_[2] == _("CIE *b"))
      {
        Lab2GL(.6,0.5,-1.0,1)
        ARROW_TIP
      }
    glPopMatrix();
    glLineWidth(line_1*line_mult);

  glEndList();

  DBG_PROG_ENDE
}

void
GL_View::refreshTable()
{ DBG_PROG_START

  DBG_PROG_V( table_.size() )
  // correct the channel selection
  if(table_.size()) {
    if( (int)table_[0][0][0].size() <= channel) {
      channel = (int)table_[0][0][0].size()-1;
      DBG_PROG_S( "Kanalauswahl geaendert: " << channel )
    }
  }

  // table
  if (gl_listen_[RASTER]) {
    //DBG_S( id_ << ": erase RASTER " << gl_listen_[RASTER], ICC_MSG_WARN );
    glDeleteLists ( gl_listen_[RASTER], 1);
    gl_listen_[RASTER] = 0;
  }

  if (table_.size())
  {
    gl_listen_[RASTER] = glGenLists(1);
    glNewList( gl_listen_[RASTER], GL_COMPILE); DBG_PROG_V( gl_listen_[RASTER] )

      int n_L = (int)table_.size(),
          n_a=(int)table_[0].size(),
          n_b=(int)table_[0][0].size();

      /* The small data cubes fit all into the whole table cubus.
       * The edge of the starting data cube lies on the edge of the table cubus.
       */
      double dim_x = 1.0/n_b;
      double dim_y = 1.0/n_L;
      double dim_z = 1.0/n_a;
      double start_x,start_y,start_z, x,y,z;
      double value,
             A=.0,B=.0,C=.0,D=.0,E_=.0,F=.0,H=.0;

      /* The cubus for lines is completely filled from 0...1 .
       * The center of the starting data entry lies on the edge of the whole
       * cube.
       */
      if(onion_skin != 0)
      {
        dim_x = 1.0/(n_b-1);
        dim_y = 1.0/(n_L-1);
        dim_z = 1.0/(n_a-1);
      }

#if 0
      if(onion_skin)
        front_cut = 0.01;
      else
        front_cut = std_front_cut;
#endif
      cut_distance = HYP3(dim_x,dim_y,dim_z);
      DBG_NUM_V( cut_distance );
      start_x = start_y = start_z = x = y = z = 0.5; start_y = y = -0.5;
      glPushMatrix();

#     ifndef LIGHTING_
      glDisable(GL_LIGHTING);
#     endif
      DBG_PROG_V( table_.size() <<" "<< table_[0].size() )

      float corr = 0.995f/2.0f;
      /* The cubus start point is shifted to let the data appear centred. */
      glTranslated( -0.5/0.995+dim_x/2,
                    -0.5/0.995+dim_y/2 - (onion_skin != 0 ? dim_y : 0),
                    -0.5/0.995+dim_z/2);

      int geschaelt = 0;
      for (int L = 0; L < (int)n_L; L++)
      {
        x = start_x + L * dim_y;

        for (int a = 0; a < (int)n_a; a++)
        {
          y = start_y + a * dim_z;

          for (int b = 0; b < (int)n_b; b++)
          {
            z = start_z + b * dim_x;

            value = table_[L][a][b][channel]; //DBG_PROG_V( L << a << b << channel)

#           ifdef LIGHTING_
            Lab2GL(value, .5, .5,1)
            //glColor3f(value, value, value);
#           else
            switch (point_colour) {
              case MENU_GRAY:   glColor4d( value, value, value, 1.0); break;
              case MENU_COLOUR: glColor4d((value*2),
                                           value*2-1.0,
                                           1.0-value, 1.0);         break;
              case MENU_HIGHCONTRAST: value = value * 6;
                                while(value > 1.0) { value = value - 1.0; }
                                if (onion_skin != 0 && value < 0.80) value = 0.0;
                                if (0/*onion_skin*/) {
                                  glDisable(GL_LIGHTING);
                                  Lab2GL(value, .5, .5,1)
                                  glEnable(GL_LIGHTING);
                                } else glColor4d( value, value, value, 1.0);
                                                                   break;
            }
#           endif

            if(onion_skin != 0)
            for(int n = 0; n < abs(onion_skin); ++n)
            {
              double l = 1./abs(onion_skin) * n + level/abs(onion_skin);

              /* 
                  The cubus can be imagined like:
                                                 
                        B x             __       
                         / \            AB == CIE*b
                        / | \           __       
                       /     \          AD == CIE*a
                      /   |   x C       __       
                   A x   F   /|         EA == CIE*L
                     |\   x / |                  
                     | \ / /  |                  
                     |  \ /  \|                  
                     | / x D  x G                
                   E x   |   /                   
                      \  |  /                    
                       \ | /                     
                        \|/                      
                         x H                     

               *  get table values:
               */

              A = table_[L][a][b][channel];
              if(b != n_b - 1)
              B = table_[L][a][b+1][channel];
              if(b != n_b - 1 && a != n_a - 1)
              C = table_[L][a+1][b+1][channel];
              if(a != n_a - 1)
              D = table_[L][a+1][b][channel];

              if(onion_skin > 0 && L != 0)
              {
              E_= table_[L-1][a][b][channel];
              if(b != n_b - 1)
              F = table_[L-1][a][b+1][channel];
              /*i if(b != n_b - 1 && a != n_a - 1)
              G = table_[L-1][a+1][b+1][channel]; */
              if(a != n_a - 1)
              H = table_[L-1][a+1][b][channel];
              }

              int edges;
              /* isolines */
              {
                int e[8] = {0,0,0,0,0,0,0,0};
                edges = 0;
                /* We assume, that a rectangle can be cut in basically three 
                 * ways: no interpolation is done - nothing to do,
                 *       two interpolations - rectangle is sliced by a face
                 *       four interpolations - two faces 
                 *
                 *  Here the edges are count. e is filled just for fun.
                 */
                if(A <l&&l< B && b != n_b - 1)
                {
                  ++edges;
                  e[0] = 1;
                }
                if(B <l&&l< A && b != n_b - 1)
                {
                  ++edges;
                  e[0] = 1;
                }
                if(B <l&&l< C && b != n_b - 1 && a != n_a - 1)
                {
                  ++edges;
                  e[1] = 1;
                }
                if(C <l&&l< B && b != n_b - 1 && a != n_a - 1)
                {
                  ++edges;
                  e[1] = 1;
                }
                if(D <l&&l< C && b != n_b - 1 && a != n_a - 1)
                {
                  ++edges;
                  e[2] = 1;
                }
                if(C <l&&l< D && b != n_b - 1 && a != n_a - 1)
                {
                  ++edges;
                  e[2] = 1;
                }
                if(A <l&&l< D && a != n_a - 1)
                {
                  ++edges;
                  e[3] = 1;
                }
                if(D <l&&l< A && a != n_a - 1)
                {
                  ++edges;
                  e[3] = 1;
                }
                geschaelt += edges;

                /*  For no interpolation (edges) we skip the following, 
                 *  for two interpolations (edges) one pass is needed and
                 *  for four we need to do a second pass.
                 */
                for(int m = edges; m > 0; m -= 2)
                {
                  /* reset the edges drawing instructor */
                  memset(e, 0, sizeof(int)*8);
                  /*  four interpolations, first pass:
                   *  We want just one line o be drawn. Eigther with one
                   *  interpolated point between AB && AD || BA && BC.
                   *  The two conditions are exclusive, as AB || BA is possible.
                   */
                  if(m == 4 && edges == 4)
                  {
                    if(A <l&&l< B && b != n_b - 1 &&
                       A <l&&l< D && a != n_a - 1)
                    {
                      e[0] = 1;
                      e[3] = -1;
                    }

                    if(B <l&&l< A && b != n_b - 1 &&
                       B <l&&l< C && b != n_b - 1 && a != n_a - 1)
                    {
                      e[0] = -1;
                      e[1] = 1;
                    }
                    if(oy_debug)
                    {
                    /* draw the non interpolated table rectangle */
                    glColor4d( 1, 0, 0, 1.0);
                    glBegin(GL_LINE_STRIP);
                      glVertex3d(dim_x*b+-dim_x*.5,
                                 dim_y*L+ dim_y*.5,
                                 dim_z*a+-dim_z*.5);
                      glVertex3d(dim_x*b+ dim_x*.5,
                                 dim_y*L+ dim_y*.5,
                                 dim_z*a+-dim_z*.5);
                      glVertex3d(dim_x*b+ dim_x*.5,
                                 dim_y*L+ dim_y*.5,
                                 dim_z*a+ dim_z*.5);
                    glEnd();
                    }
                  }
                  /* four interpolations, second pass:
                   * The other line we expect is ordered to be drawn here. */
                  if(m == 2 && edges == 4)
                  {
                    if(D <l&&l< C && b != n_b - 1 && a != n_a - 1 &&
                       D <l&&l< A && a != n_a - 1)
                    {
                      e[2] = -1;
                      e[3] = 1;
                    }

                    if(C <l&&l< D && b != n_b - 1 && a != n_a - 1 &&
                       C <l&&l< B && b != n_b - 1 && a != n_a - 1)
                    {
                      e[2] = 1;
                      e[1] = -1;
                    }
                  }

                  /* Only one line is expected. No particular order is needed. 
                   * Just the direction counts.
                   */
                  if(edges == 2)
                  {
                    if(A <l&&l< B && b != n_b - 1)
                      e[0] = 1;

                    if(B <l&&l< A && b != n_b - 1)
                      e[0] = -1;

                    if(B <l&&l< C && b != n_b - 1 && a != n_a - 1)
                      e[1] = 1;

                    if(C <l&&l< B && b != n_b - 1 && a != n_a - 1)
                      e[1] = -1;

                    if(C <l&&l< D && b != n_b - 1 && a != n_a - 1)
                      e[2] = 1;

                    if(D <l&&l< C && b != n_b - 1 && a != n_a - 1)
                      e[2] = -1;

                    if(D <l&&l< A && a != n_a - 1)
                      e[3] = 1;

                    if(A <l&&l< D && a != n_a - 1)
                      e[3] = -1;
                  }

if(0 && edges==4)
printf("%s:%d %d e:%d,%d,%d,%d\n",__FILE__,__LINE__,m, e[0],e[1],e[2],e[3]);

                  /* Draw one line, otherwise we get crossing lines.
                   * Variable e should contain only two fills.
                   */
                  glBegin(GL_LINES);
                    if(e[0] == 1 && b != n_b - 1)
                      glVertex3d(dim_x*b+ dim_x*(-.5+(l-A)/(B-A)),
                                 dim_y*L+ dim_y*.5,
                                 dim_z*a+-dim_z*.5);
                    if(e[0] == -1 && b != n_b - 1)
                      glVertex3d(dim_x*b+ dim_x*(.5+(B-l)/(A-B)),
                                 dim_y*L+ dim_y*.5,
                                 dim_z*a+-dim_z*.5);
                    if(e[1] == 1 && b != n_b - 1 && a != n_a - 1)
                      glVertex3d(dim_x*b+ dim_x*.5,
                                 dim_y*L+ dim_y*.5,
                                 dim_z*a+ dim_z*(-.5+(l-B)/(C-B)));
                    if(e[1] == -1 && b != n_b - 1 && a != n_a - 1)
                      glVertex3d(dim_x*b+ dim_x*.5,
                                 dim_y*L+ dim_y*.5,
                                 dim_z*a+ dim_z*(.5+(C-l)/(B-C)));
                    if(e[2] == 1 && b != n_b - 1 && a != n_a - 1)
                      glVertex3d(dim_x*b+ dim_x*(.5+(C-l)/(D-C)),
                                 dim_y*L+ dim_y*.5,
                                  dim_z*a+ dim_z*.5);
                    if(e[2] == -1 && b != n_b - 1 && a != n_a - 1)
                      glVertex3d(dim_x*b+ dim_x*(-.5+(l-D)/(C-D)),
                                 dim_y*L+ dim_y*.5,
                                 dim_z*a+ dim_z*.5);
                    if(e[3] == 1 && a != n_a - 1)
                      glVertex3d(dim_x*b+-dim_x*.5,
                                 dim_y*L+ dim_y*.5,
                                 dim_z*a+ dim_z*(.5+(D-l)/(A-D)));
                    if(e[3] == -1 && a != n_a - 1)
                      glVertex3d(dim_x*b+-dim_x*.5,
                                 dim_y*L+ dim_y*.5,
                                 dim_z*a+ dim_z*(-.5+(l-A)/(D-A)));
                  glEnd();
                }
              }


              /* The analysation as in isolines above should be below as well.*/
              if(onion_skin > 0 && L != 0)
              {
              if(b != n_b - 1)
              {
              glBegin(GL_LINE_STRIP);
                if(A <l&&l< B)
                  glVertex3d(dim_x*b+ dim_x*(-.5+(l-A)/(B-A)),
                             dim_y*L+ dim_y*.5,
                             dim_z*a+-dim_z*.5);
                if(E_ <l&&l< F)
                  glVertex3d(dim_x*b+ dim_x*(-.5+(l-E_)/(F-E_)),
                             dim_y*L+-dim_y*.5,
                             dim_z*a+-dim_z*.5);
                if(B <l&&l< A)
                  glVertex3d(dim_x*b+ dim_x*(.5+(B-l)/(A-B)),
                             dim_y*L+ dim_y*.5,
                             dim_z*a+-dim_z*.5);
                if(F <l&&l< E_)
                  glVertex3d(dim_x*b+ dim_x*(.5+(F-l)/(E_-F)),
                             dim_y*L+-dim_y*.5,
                             dim_z*a+-dim_z*.5);
                if(A <l&&l< E_)
                  glVertex3d(dim_x*b+-dim_x*.5,
                             dim_y*L+ dim_y*(.5+(A-l)/(E_-A)),
                             dim_z*a+-dim_z*.5);
                if(E_ <l&&l< A)
                  glVertex3d(dim_x*b+-dim_x*.5,
                             dim_y*L+ dim_y*(-.5+(l-E_)/(A-E_)),
                             dim_z*a+-dim_z*.5);
                if(B <l&&l< F)
                  glVertex3d(dim_x*b+ dim_x*.5,
                             dim_y*L+ dim_y*(.5+(B-l)/(F-B)),
                             dim_z*a+-dim_z*.5);
                if(F <l&&l< B)
                  glVertex3d(dim_x*b+ dim_x*.5,
                             dim_y*L+ dim_y*(-.5+(l-F)/(B-F)),
                             dim_z*a+-dim_z*.5);
              glEnd();
              }

              if(a != n_a - 1)
              {
              glBegin(GL_LINE_STRIP);
                if(A <l&&l< D)
                  glVertex3d(dim_x*b+-dim_x*.5,
                             dim_y*L+ dim_y*.5,
                             dim_z*a+ dim_z*(-.5+(l-A)/(D-A)));
                if(E_ <l&&l< H)
                  glVertex3d(dim_x*b+-dim_x*.5,
                             dim_y*L+-dim_y*.5,
                             dim_z*a+ dim_z*(-.5+(l-E_)/(H-E_)));
                if(D <l&&l< A)
                  glVertex3d(dim_x*b+-dim_x*.5,
                             dim_y*L+ dim_y*.5,
                             dim_z*a+ dim_z*(.5+(D-l)/(A-D)));
                if(H <l&&l< E_)
                  glVertex3d(dim_x*b+-dim_x*.5,
                             dim_y*L+-dim_y*.5,
                             dim_z*a+ dim_z*(.5+(H-l)/(E_-H)));
                if(A <l&&l< E_)
                  glVertex3d(dim_x*b+-dim_x*.5,
                             dim_y*L+ dim_y*(.5+(A-l)/(E_-A)),
                             dim_z*a+-dim_z*.5);
                if(E_ <l&&l< A)
                  glVertex3d(dim_x*b+-dim_x*.5,
                             dim_y*L+ dim_y*(-.5+(l-E_)/(A-E_)),
                             dim_z*a+-dim_z*.5);
                if(D <l&&l< H)
                  glVertex3d(dim_x*b+-dim_x*.5,
                             dim_y*L+ dim_y*(.5+(D-l)/(H-D)),
                             dim_z*a+ dim_z*.5);
                if(H <l&&l< D)
                  glVertex3d(dim_x*b+-dim_x*.5,
                             dim_y*L+ dim_y*(-.5+(l-H)/(D-H)),
                             dim_z*a+ dim_z*.5);
              glEnd();
              }
              }
            }

            /* the cubus representation */
            if (value && 
                (onion_skin == 0 || geschaelt < 0))
            {
                glBegin(GL_TRIANGLE_FAN);
                  glVertex3d(dim_x*b+ dim_x*corr,
                             dim_y*L+ dim_y*corr,
                             dim_z*a+ dim_z*corr);
                  glVertex3d(dim_x*b+ dim_x*corr,
                             dim_y*L+-dim_y*corr,
                             dim_z*a+ dim_z*corr);
                  glVertex3d(dim_x*b+-dim_x*corr,
                             dim_y*L+-dim_y*corr,
                             dim_z*a+ dim_z*corr);
                  glVertex3d(dim_x*b+-dim_x*corr,
                             dim_y*L+ dim_y*corr,
                             dim_z*a+ dim_z*corr);
                  glVertex3d(dim_x*b+-dim_x*corr,
                             dim_y*L+ dim_y*corr,
                             dim_z*a+-dim_z*corr);
                  glVertex3d(dim_x*b+ dim_x*corr,
                             dim_y*L+ dim_y*corr,
                             dim_z*a+-dim_z*corr);
                  glVertex3d(dim_x*b+ dim_x*corr,
                             dim_y*L+-dim_y*corr,
                             dim_z*a+-dim_z*corr);
                  glVertex3d(dim_x*b+ dim_x*corr,
                             dim_y*L+-dim_y*corr,
                             dim_z*a+ dim_z*corr);
                glEnd();
                glBegin(GL_TRIANGLE_FAN);
                  glVertex3d(dim_x*b+-dim_x*corr,
                             dim_y*L+-dim_y*corr,
                             dim_z*a+-dim_z*corr);
                  glVertex3d(dim_x*b+ dim_x*corr,
                             dim_y*L+-dim_y*corr,
                             dim_z*a+-dim_z*corr);
                  glVertex3d(dim_x*b+ dim_x*corr,
                             dim_y*L+-dim_y*corr,
                             dim_z*a+ dim_z*corr);
                  glVertex3d(dim_x*b+-dim_x*corr,
                             dim_y*L+-dim_y*corr,
                             dim_z*a+ dim_z*corr);
                  glVertex3d(dim_x*b+-dim_x*corr,
                             dim_y*L+ dim_y*corr,
                             dim_z*a+ dim_z*corr);
                  glVertex3d(dim_x*b+-dim_x*corr,
                             dim_y*L+ dim_y*corr,
                             dim_z*a+-dim_z*corr);
                  glVertex3d(dim_x*b+ dim_x*corr,
                             dim_y*L+ dim_y*corr,
                             dim_z*a+-dim_z*corr);
                  glVertex3d(dim_x*b+ dim_x*corr,
                             dim_y*L+-dim_y*corr,
                             dim_z*a+-dim_z*corr);
                glEnd();
            }
          }
        }

        if(onion_skin != 0 && L == n_L -1 && geschaelt < 2 && geschaelt != -1)
        {
          L = 0;
          geschaelt = -1;
          dim_x = 1.0/n_b;
          dim_y = 1.0/n_L;
          dim_z = 1.0/n_a;
        }

      } DBG_PROG
      glPopMatrix();
#     ifndef LIGHTING_
      glEnable(GL_LIGHTING);
#     endif
    glEndList();
  }
 
  DBG_PROG_ENDE
}

void
GL_View::refreshNets()
{
  DBG_PROG_START
  MARK( frei(false); )
    if( triangle_nets.size() )
    {
      DBG_ICCGL_V( triangle_nets.size() )
      DBG_ICCGL_V( triangle_nets[0].name )
      DBG_ICCGL_V( triangle_nets[0].punkte.size() )
      DBG_ICCGL_V( triangle_nets[0].indexe.size() )
      DBG_ICCGL_V( triangle_nets[0].undurchsicht )
    }

      // sort meshes
      glPushMatrix();
         // orientate the matrix to the camera
       float EyeAz = agv_->eyeAzimuth(),
             EyeEl = agv_->eyeElevation(),
             EyeDist = agv_->eyeDist(),
             X = (float)(-EyeDist*sin(TORAD(EyeAz))*cos(TORAD(EyeEl))),  // CIE*b
             Y = (float)(EyeDist*sin(TORAD(EyeEl))),                     // CIE*L
             Z = (float)(EyeDist*cos(TORAD(EyeAz))*cos(TORAD(EyeEl)));   // CIE*a

       float lX = (float)(-EyeDist*sin(TORAD(EyeAz-30))*cos(TORAD(EyeEl))),
             lY = (float)(Y),
             lZ = (float)(EyeDist*cos(TORAD(EyeAz-30))*cos(TORAD(EyeEl)));
       glEnable(GL_LIGHTING);
       glEnable(GL_DEPTH_TEST);

       static double hintergrundfarbe_statisch = background_colour;
       if(background_colour != hintergrundfarbe_statisch)
         DBG_NUM_S("background colour changed "<<id_);
       GLfloat lmodel_ambient[] = {0.125f+background_colour/8,
                                   0.125f+background_colour/8,
                                   0.125f+background_colour/8, 1.0f};
       glLightModelfv(GL_LIGHT_MODEL_AMBIENT, lmodel_ambient);

       GLfloat light1_position[] = { lX, lY, lZ, 1.0 };
       GLfloat light_ambient[] = {0.1f, 0.1f, 0.1f, 1.0f};
       GLfloat light_diffuse[] = {1.0, 1.0, 1.0, 1.0};
       GLfloat light_specular[] = {1.0, 1.0, 1.0, 1.0};
       glLightfv(GL_LIGHT1, GL_POSITION, light1_position);
       glLightfv(GL_LIGHT1, GL_AMBIENT, light_ambient);
       glLightfv(GL_LIGHT1, GL_DIFFUSE, light_diffuse);
       glLightfv(GL_LIGHT1, GL_SPECULAR, light_specular);
       //glLightf(GL_LIGHT1, GL_LINEAR_ATTENUATION, 0.0);
       //glLightf(GL_LIGHT1, GL_QUADRATIC_ATTENUATION, 0.2);

       glEnable (GL_LIGHT1);
#      if 0
       GLfloat light2_ambient[] = {0.2, 0.2, 0.2, 1.0};
       GLfloat light2_diffuse[] = {1.0, 1.0, 1.0, 1.0};
       GLfloat light2_specular[] = {1.0, 1.0, 1.0, 1.0};
       GLfloat light2_position[] = {X, Y, Z, 1.0};
       GLfloat spot_direction[] = {X, Y, Z};

       glLightfv(GL_LIGHT2, GL_AMBIENT, light2_ambient);
       glLightfv(GL_LIGHT2, GL_DIFFUSE, light2_diffuse);
       glLightfv(GL_LIGHT2, GL_SPECULAR, light2_specular);
       glLightfv(GL_LIGHT2, GL_POSITION, light2_position);
       //glLightf(GL_LIGHT2, GL_CONSTANT_ATTENUATION, 1.5);
       //glLightf(GL_LIGHT2, GL_LINEAR_ATTENUATION, 0.5);
       //glLightf(GL_LIGHT2, GL_QUADRATIC_ATTENUATION, 0.2);

       glLightf(GL_LIGHT2, GL_SPOT_CUTOFF, 45.0);
       glLightfv(GL_LIGHT2, GL_SPOT_DIRECTION, spot_direction);
       glLightf(GL_LIGHT2, GL_SPOT_EXPONENT, 2.0);

       glEnable (GL_LIGHT2);
#      endif
       glDisable(GL_LIGHT0);
       glEnable(GL_LIGHTING);

       if(icc_debug != 0)
         drawCone(0.02, 0.05, 16, lX, 0, lZ);


       if(icc_debug == 14) {
       line_mult = 3;
       DBG_ICCGL_S( "dist:"<<EyeDist<<" elevation:"<<EyeEl<<" azimuth:"<<EyeAz )
       glMatrixMode(GL_MODELVIEW);
       glLoadIdentity();
       glTranslated(X/1.2,Y/1.2,Z/1.2);
       DBG_ICCGL_S( "X:"<<X<<" Y:"<<Y<<" Z:"<<Z )
       drawCoordinates_();
       glTranslated(-X/1.2,-Y/1.2,-Z/1.2);
       }

       std::multimap<double,DreiecksIndexe> indices;
       double abstand;
       std::multimap<double,DreiecksIndexe>::const_iterator it;
       if(!bsp)
       for( it = net.indexe.begin(); it != net.indexe.end(); ++it )
       {
               // insert indicies, to count newly
       /*A*/ std::pair<double,DreiecksIndexe> index_p( *it );

       /*C*/
               // midpoint of the triangle
             abstand = HYP3( index_p.second.midpoint[0]+Y,
                             index_p.second.midpoint[1]+Z,
                             index_p.second.midpoint[2]+X );
             index_p.first = abstand;
               // the container std::map does sorting
       /*D*/ indices.insert(index_p);
       }
      glPopMatrix();


  if (gl_listen_[RASTER]) {
    //DBG_S( id_ << ": erase RASTER " << gl_listen_[RASTER], ICC_MSG_WARN );
    glDeleteLists (gl_listen_[RASTER], 1);
    gl_listen_[RASTER] = 0;
  }

#     ifdef LIGHTING
      glEnable(GL_LIGHTING);
#     else
      glDisable(GL_LIGHTING);
#     endif

#     if 0
      glEnable (GL_BLEND);
      glBlendFunc (GL_SRC_COLOR, GL_DST_ALPHA);
      glEnable (GL_ALPHA_TEST_FUNC);
      //glAlphaFunc (GL_ALPHA_TEST, GL_ONE_MINUS_DST_ALPHA);
#     else
      glEnable (GL_BLEND);
      //glDepthMask(GL_FALSE);              // Konturen und Schnittkanten muessn
                                          // richtig uebergeben werden 
      glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
      glEnable (GL_ALPHA_TEST_FUNC);
      glAlphaFunc (GL_ALPHA_TEST, GL_ONE_MINUS_DST_ALPHA);
#     endif
#     if 0
      glPolygonMode(GL_FRONT, GL_FILL);   // front side filled  
      glPolygonMode(GL_BACK, GL_LINE);   // back side as lines
#     endif
      glFrontFace(GL_CCW);
      glPushMatrix();
        // positioning
      glTranslated( -cie_b_display_stretch/2., -.5, -cie_a_display_stretch/2. );

      
      DBG_ICCGL_V( net.indexe.size() <<" "<< net.punkte.size() )


      glLineWidth(line_1*line_mult);
      int index[7];
      double len=1.0;

      // all material colours obtian here their transparency
      glColorMaterial( GL_FRONT_AND_BACK, GL_SPECULAR );
      glColorMaterial( GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE );
      glEnable( GL_COLOR_MATERIAL );
      //GLfloat colour[4] = {  0.5,0.5,0.5,1.0 };
#define COLOURS(r,g,b,a) {/*colour [0] = (r); colour [1] = (g); colour [2] = (b); \
                      colour[3] = (a);  \
              glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, colour); \
              glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, colour);*/\
                      glColor4d(r,g,b,a); }

      if(bsp)
      {
        double dist = HYP3(X,Y,Z);
        icc_examin_ns::POINT pov = {Y + 0.5,
                                    Z + cie_b_display_stretch/2,
                                    X + cie_b_display_stretch/2.};
        if(icc_debug != 0)
        {
          drawCone(0.02, 0.05, 16, X/dist, Y/dist, Z/dist);
          glBegin(GL_LINES);
            glVertex3d( X/dist,
                        Y/dist,
                        Z/dist);
            glVertex3d( 0,
                        0,
                        0 );
          glEnd();
        }
        icc_examin_ns::BSPtraverseTreeAndRender( bsp, &pov, 1);
      }
        else
      {
        for( it = indices.begin(); it != indices.end(); ++it )
        {
          index[0] = it->second.i[0];
          index[1] = it->second.i[1];
          index[2] = it->second.i[2];
          glBegin(GL_TRIANGLES);

            glNormal3d( it->second.normale[2],
                        it->second.normale[0],
                        it->second.normale[1] );
            for( int l = 2; l >= 0; --l)
            {
              COLOURS (   net.punkte[index[l]].farbe[0],
                         net.punkte[index[l]].farbe[1],
                         net.punkte[index[l]].farbe[2],
                         net.punkte[index[l]].farbe[3]);
              glVertex3d( net.punkte[index[l]].koord[2],
                          net.punkte[index[l]].koord[0],
                          net.punkte[index[l]].koord[1] );
            }
          glEnd();
          //DBG_V( index <<" "<< len <<" "<< normale[0] );
          if(icc_debug != 0)
          {
          glLineWidth(line_1*line_mult);
          glBegin(GL_LINES);
            glVertex3d( net.punkte[index[0]].koord[2],
                        net.punkte[index[0]].koord[0],
                        net.punkte[index[0]].koord[1] );
            glVertex3d( net.punkte[index[0]].koord[2]+it->second.normale[2]/len*.1,
                        net.punkte[index[0]].koord[0]+it->second.normale[0]/len*.1,
                        net.punkte[index[0]].koord[1]+it->second.normale[1]/len*.1 );
          glEnd();
          }
          if(net.kubus)
          {
            COLOURS ( 5.0,5.0,5.0,net.punkte[index[0]].farbe[3] );
            glBegin(GL_LINES);
              glVertex3d( net.punkte[index[1]].koord[2],
                          net.punkte[index[1]].koord[0],
                          net.punkte[index[1]].koord[1] );
              glVertex3d( net.punkte[index[0]].koord[2],
                          net.punkte[index[0]].koord[0],
                          net.punkte[index[0]].koord[1] );
            glEnd();
          }
        }
      }

      glPopMatrix();

      glDisable (GL_BLEND);
      glDisable( GL_COLOR_MATERIAL );
      glDepthMask(GL_TRUE);
#     ifndef LIGHTING
      glEnable(GL_LIGHTING);
#     endif

  MARK( frei(true); )
  DBG_PROG_ENDE
}


void
GL_View::iccPoint3d               ( oyPROFILE_e         projection,
                                       double            * vertex,
                                       double              radius )
{
  double d3[3];

  switch (projection)
  {
    case oyEDITING_LAB:
         CIELabToLab( vertex, d3, 1 );
         d3[2] *= cie_b_display_stretch;
         d3[1] *= cie_a_display_stretch;
         break;
    default:
         memcpy( d3, vertex, sizeof(double) * 3 );
  }

  if(radius)
  {
    int dim = 12;
    GLUquadricObj *quad;
    quad = gluNewQuadric();

    if(radius <= 0.005)
      dim = 5;
    else if (radius <= 0.01)
      dim = 8;
     glTranslated( d3[2], d3[0], d3[1] );
     gluSphere( quad, radius, dim, dim );
  }
  else
    glVertex3d( d3[2], d3[0], d3[1] );
  
}

double GL_View::pointRadius()
{
  double rad = 0.5;
  switch (point_form)
  {
    case MENU_dE1SPHERE: rad = 0.5; break;
    case MENU_dE2SPHERE: rad = 1.0; break;
    case MENU_dE4SPHERE: rad = 2.0; break;
    default:            rad = ((double)point_size)/4.0; break;
  }
  return rad;
}

void
GL_View::refreshPoints()
{ DBG_PROG_START

  if (gl_listen_[POINTS]) {
    glDeleteLists (gl_listen_[POINTS], 1);
    gl_listen_[POINTS] = 0;
  }

  //coordinates  in CIE*b CIE*L CIE*a 
  if (oyStructList_Count( colours_ )) {
    DBG_PROG_V( oyStructList_Count( colours_ ) )

    gl_listen_[POINTS] = glGenLists(1);
    glNewList( gl_listen_[POINTS], GL_COMPILE); DBG_PROG_V( gl_listen_[POINTS] )
#     ifndef LIGHTING_
      glDisable(GL_LIGHTING);
#     endif

      //glColor3f(0.9, 0.9, 0.9);
      glPushMatrix();
        // positioning
        glTranslated( -cie_b_display_stretch/2,-.5,-cie_a_display_stretch/2 );

        int x = this->window()->x() + this->window()->w()/2;
        int y = this->window()->y() + this->window()->h()/2;

        oyProfile_s * prof_disp = NULL;
        if(icc_oyranos.colourServerActive() & XCM_COLOR_SERVER_MANAGEMENT)
          prof_disp = oyProfile_Copy( edit_, NULL );
        else
          prof_disp = icc_oyranos.oyMoni(x,y);

        int nc = oyStructList_Count(colours_);

        double lab[3], rgba[4], XYZ[3];
        oyProfile_s * prof_in  = 0,
                    * prof_out = 0;
        oyImage_s * in = 0,
                  * out = 0,
                  * out_disp = 0;
        oyArray2d_s * in_array = 0;
        oyConversion_s * cc_disp = 0, * cc_lab = 0;
        oyPROFILE_e projection = oyEDITING_LAB;

        if(nc)
        {
          prof_in  = oyProfile_FromStd( oyEDITING_XYZ, 0 );
          prof_out = oyProfile_FromStd( projection, 0 );
          /* we assume Oyranos uses a oyArray2d_s backend for oyImage_s */
          in =  oyImage_Create( 1,1,
                         XYZ,
                         oyChannels_m(oyProfile_GetChannelsCount(prof_in)) |
                          oyDataType_m(oyDOUBLE),
                         prof_in,
                         0 );
          out = oyImage_Create( 1,1,
                         lab,
                         oyChannels_m(oyProfile_GetChannelsCount(prof_out)) |
                          oyDataType_m(oyDOUBLE),
                         prof_out,
                         0 );
          out_disp = oyImage_Create( 1,1,
                         rgba,
                         oyChannels_m(oyProfile_GetChannelsCount(prof_disp)) |
                          oyDataType_m(oyDOUBLE),
                         prof_disp,
                         0 );

          oyOptions_s * opts = icc_examin->options();
          cc_lab = oyConversion_CreateBasicPixels( in,out, opts, 0 );
          cc_disp = oyConversion_CreateBasicPixels( in,out_disp, opts, 0 );
          oyOptions_Release( &opts );

          in_array = (oyArray2d_s*) oyImage_GetPixelData( in );
          if(!in_array || in_array->type_ != oyOBJECT_ARRAY2D_S)
          {
            WARN_S( "unknown image backend found " );
          }
        }

        if(cc_disp)
        for(int j = 0; j < nc; ++j)
        {
          oyNamedColours_s * colours = 
            (oyNamedColours_s*) oyStructList_GetRefType( colours_, j,
                                                oyOBJECT_NAMED_COLOURS_S );

          if(!colours)
          {
            WARN_S( "missing list(" << nc << ") member[" << j << "]" )
            continue;
          }

          int n = oyNamedColours_Count( colours );
          int active = 1;
          int gray = 0;
          oyNamedColour_s * c = NULL;
          double shade = 1;
          int has_mesh = 0;

          rgba[3] = 1;

          if(triangle_nets.size() > (unsigned)j)
          {
            active = triangle_nets[j].active();
            gray = triangle_nets[j].grau;
            rgba[3] = triangle_nets[j].undurchsicht;
            shade = triangle_nets[j].schattierung;
            if(triangle_nets[j].punkte.size())
              has_mesh = 1;
          }

          if(!has_mesh)
          {
            glEnable (GL_BLEND);
            glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glEnable (GL_ALPHA_TEST_FUNC);
            glAlphaFunc (GL_ALPHA_TEST, GL_ONE_MINUS_DST_ALPHA);
          } else {
            glDisable (GL_BLEND);
            glDisable (GL_ALPHA_TEST_FUNC);
          }

          if(!active)
            continue;

          if(show_points_as_pairs)
            n /= 2;

          if(show_points_as_pairs)
            for (int i = 0; i < n; ++i)
            {
              glLineWidth(line_2*line_mult);
              // draw lines
              glBegin(GL_LINES);
                c = oyNamedColours_Get( colours, i );
                oyArray2d_SetData( in_array,
                                   (void*) oyNamedColour_GetXYZConst( c ) );

                if(!show_points_as_measurements)
                  glColor4d(.97, .97, .97, rgba[3] );
                else {
                  if(gray)
                    rgba[0]= rgba[1]= rgba[2] = shade;
                  else
                    oyConversion_RunPixels( cc_disp, 0 );
                  glColor4dv( rgba );
                }

                if(in_array)
                oyXYZ2Lab( ((double**)oyArray2d_GetData( in_array ))[0], lab );
                iccPoint3d ( projection, lab, 0 );
                oyNamedColour_Release( &c );

                c = oyNamedColours_Get( colours, n + i );
                oyArray2d_SetData( in_array,
                                   (void*) oyNamedColour_GetXYZConst( c ) );
                if(!show_points_as_measurements)
                  glColor4d(1., .6, .6, 1.0 );
                else {
                  if(gray)
                    rgba[0]= rgba[1]= rgba[2] = shade;
                  else
                    oyConversion_RunPixels( cc_disp, 0 );
                  glColor4dv( rgba );
                }

                if(in_array)
                oyXYZ2Lab( ((double**)oyArray2d_GetData( in_array ))[0], lab );
                iccPoint3d ( projection, lab, 0 );
                oyNamedColour_Release( &c );
              glEnd();
            }

          if(show_points_as_pairs && !show_points_as_measurements)
            n *= 2; 

          double rad = .02;
          int draw_spheres = false;
          switch (point_form)
          {
            case MENU_dE1STAR:
               glPointSize(point_size);
               glColor4d(.97, .97, .97, rgba[3] );
               glBegin(GL_POINTS);
                 for (int i = 0; i < n; ++i)
                 {
                   c = oyNamedColours_Get( colours, i );
                   oyArray2d_SetData( in_array,
                                      (void*) oyNamedColour_GetXYZConst( c ) );
                   if(gray)
                     rgba[0]= rgba[1]= rgba[2] = shade;
                   else
                     oyConversion_RunPixels( cc_disp, 0 );
                   glColor4dv( rgba );

                   if(in_array)
                   oyXYZ2Lab( ((double**)oyArray2d_GetData( in_array ))[0], lab );
                   iccPoint3d ( projection, lab, 0 );
                   oyNamedColour_Release( &c );
                 }
               glEnd();
              break;
            case MENU_dE1SPHERE: rad = 0.005;draw_spheres = true;break;
            case MENU_dE2SPHERE: rad = 0.01; draw_spheres = true;break;
            case MENU_dE4SPHERE: rad = 0.02; draw_spheres = true;break;
                 break;
            case MENU_DIFFERENCE_LINE: // they are drawn anyway
                 break;
          }

          if(draw_spheres)
          {
            for (int i = 0; i < n; ++i)
            {
                 glPushMatrix();
                   c = oyNamedColours_Get( colours, i );
                   oyArray2d_SetData( in_array,
                                      (void*) oyNamedColour_GetXYZConst( c ) );
                   if(gray)
                     rgba[0]= rgba[1]= rgba[2] = shade;
                   else
                     oyConversion_RunPixels( cc_disp, 0 );
                   glColor4dv( rgba );

                   if(in_array)
                   oyXYZ2Lab( ((double**)oyArray2d_GetData( in_array ))[0], lab );
                   iccPoint3d( projection, lab, rad );
                   oyNamedColour_Release( &c );
                 glPopMatrix();
            }
          }
          // shadow
          glColor4f( shadow, shadow, shadow, 1. );
          glPointSize((point_size/2-1)>0?(point_size/2-1):1);
          glBegin(GL_POINTS);
            if(show_points_as_measurements)
                 for (int i = 0; i < n; ++i)
                 {
                   c = oyNamedColours_Get( colours, i );
                   oyArray2d_SetData( in_array,
                                      (void*) oyNamedColour_GetXYZConst( c ) );
                   if(in_array)
                   oyXYZ2Lab( ((double**)oyArray2d_GetData( in_array ))[0], lab );
                   lab[0] = 0;
                   iccPoint3d ( projection, lab, 0 );
                   oyNamedColour_Release( &c );
                 }
          glEnd();
        }

        oyConversion_Release( &cc_lab );
        oyConversion_Release( &cc_disp );
        oyImage_Release( &in );
        oyImage_Release( &out );
        oyImage_Release( &out_disp );
        oyProfile_Release( &prof_in );
        oyProfile_Release( &prof_out );
        oyProfile_Release( &prof_disp );
        oyArray2d_Release( &in_array );

      glPopMatrix();
#     ifndef LIGHTING_
      glEnable(GL_LIGHTING);
#     endif
    glEndList();
  }

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  DBG_PROG_ENDE
}

extern float cieXYZ [471][3]; // in 

void
GL_View::showContours_()
{
  DBG_PROG_START

  if (gl_listen_[CONTOURS]) {
    DBG_PROG_S( "delete glListe " << gl_listen_[CONTOURS] )
    glDeleteLists (gl_listen_[CONTOURS],1);
    gl_listen_[CONTOURS] = 0;
  }

  oyOptions_s * opts = icc_examin->options();

  //if (spectral_line == MENU_SPECTRAL_LINE)
  for (unsigned int d=0; d < triangle_nets.size(); ++d)
  {
    double *RGB_buffer = 0,
           *RGB_shadow_buffer = 0,
           *Lab_buffer = 0,
           *Lab_shadow_buffer = 0;

    // conversion
    int n = (int)triangle_nets[d].umriss.size();
    DBG_PROG_V( n )
    if(!n) continue;

    Lab_buffer = (double*) malloc (sizeof(double) * n*3);
    if(!Lab_buffer)  WARN_S( "Lab_speicher Speicher nicht verfuegbar" )

    for ( int j = 0; j < n; ++j)
      for(int k = 0; k < 3 ; ++k)
        Lab_buffer[j*3+k] = triangle_nets[d].umriss[j].koord[k];

    RGB_buffer = icc_oyranos.wandelLabNachBildschirmFarben(
               window()->x() + window()->w()/2, window()->y() + window()->h()/2,
         (icc_oyranos.colourServerActive() & XCM_COLOR_SERVER_MANAGEMENT)?edit_:0,
               Lab_buffer, (size_t)n, opts);
    DBG_PROG_V( n )
    // create shadow
    Lab_shadow_buffer = (double*) malloc (sizeof(double) * n*3);

    for (int i = 0; i < n; ++i) {
      Lab_shadow_buffer[i*3+0] = triangle_nets[d].schattierung; //background_colour*.40+.35;
      Lab_shadow_buffer[i*3+1] = (Lab_buffer[i*3+1]-.5)*.25+0.5;
      Lab_shadow_buffer[i*3+2] = (Lab_buffer[i*3+2]-.5)*.25+0.5;
    }

    RGB_shadow_buffer = icc_oyranos.wandelLabNachBildschirmFarben(
               window()->x() + window()->w()/2, window()->y() + window()->h()/2,
         (icc_oyranos.colourServerActive() & XCM_COLOR_SERVER_MANAGEMENT)?edit_:0,
                      Lab_shadow_buffer, n, opts);
    if(!RGB_buffer)  WARN_S( "RGB_speicher result is not available" )
    if(!RGB_shadow_buffer)  WARN_S( "RGB_speicher result is not available" )

    for ( int j = 0; j < n; ++j)
      for(int k = 0; k < 3 ; ++k)
        triangle_nets[d].umriss[j].farbe[k] = RGB_shadow_buffer[j*3+k];

    if (Lab_buffer) free (Lab_buffer);
    if (Lab_shadow_buffer) free (Lab_shadow_buffer);
    if (RGB_buffer) delete [] RGB_buffer;
    if (RGB_shadow_buffer) delete [] RGB_shadow_buffer;
  }
  oyOptions_Release( &opts );

    GLfloat colour[] =   { arrow_colour[0],arrow_colour[1],arrow_colour[2], 1.0 };

  gl_listen_[CONTOURS] = glGenLists(1);
  glNewList( gl_listen_[CONTOURS], GL_COMPILE );
  DBG_PROG_V( gl_listen_[CONTOURS] ) 

    glDisable (GL_LIGHTING);
    glDisable (GL_ALPHA_TEST_FUNC);
    if(!smooth) {
      glDisable(GL_BLEND);
      glDisable(GL_LINE_SMOOTH);
    } else {
      glEnable(GL_BLEND);
      glEnable(GL_LINE_SMOOTH);
    }

    for (unsigned int i=0; i < triangle_nets.size(); ++i)
    {
      if(triangle_nets[i].umriss.size() &&
         triangle_nets[i].active() &&
         triangle_nets[i].undurchsicht)
      {
        // colour line
        if(icc_debug)
        {
        glLineWidth(line_3*line_mult);
        glColor4f(1., 1.0, 1.0, 1.0);
        glBegin(GL_LINE_STRIP);
        for (int z=0 ; z < (int)triangle_nets[i].umriss.size(); z++) {
          if(!triangle_nets[i].grau) {
            Lab2GL(triangle_nets[i].umriss[z].farbe[0],
                  triangle_nets[i].umriss[z].farbe[1],
                  triangle_nets[i].umriss[z].farbe[2],1)
          }

          glVertex3d( 
                  (triangle_nets[i].umriss[z].koord[2] * cie_b_display_stretch
                    - cie_b_display_stretch/2.),
                  (triangle_nets[i].umriss[z].koord[0] - 0.5),
                  (triangle_nets[i].umriss[z].koord[1] * cie_a_display_stretch
                    - cie_a_display_stretch/2.)
          );
        }
        glEnd();
        }
        // colour line shadow
        int n = (int)triangle_nets[i].umriss.size();
        DBG_PROG_V( n )
        glLineWidth(line_2*line_mult);

        Lab2GL (          triangle_nets[i].schattierung, .5, .5, 1);

        glBegin(GL_LINE_STRIP);
        for (int z=0 ; z < n; z++)
        {
          if(!triangle_nets[i].grau) {
            COLOUR(triangle_nets[i].umriss[z].farbe[0],
                  triangle_nets[i].umriss[z].farbe[1],
                  triangle_nets[i].umriss[z].farbe[2],1)
          }
          glVertex3d(triangle_nets[i].umriss[z].koord[2]*cie_b_display_stretch
                      - cie_b_display_stretch/2.,
                     -0.5,
                     triangle_nets[i].umriss[z].koord[1]*cie_a_display_stretch
                      - cie_a_display_stretch/2.);
        }
        glEnd();
      }
    }

  glEndList();
  DBG_PROG_ENDE
}

void
GL_View::showSprectralLine_()
{
  DBG_PROG_START

  if (gl_listen_[SPEKTRUM])
    glDeleteLists (gl_listen_[SPEKTRUM], 1);
  gl_listen_[SPEKTRUM] = 0;
  if (spectral_line == MENU_SPECTRAL_LINE)
  {
    double *RGB_buffer = 0,
           *RGB_shadow_buffer = 0,
           *XYZ_Speicher = 0,
           *Lab_buffer = 0,
           *Lab_shadow_buffer = 0;
    int n_punkte = 471;//341; // 700 nm
    oyOptions_s * opts = icc_examin->options();

    if(type_ > 1)
    {
      // spektrum variables
      //int nano_min = 63; // 420 nm

      XYZ_Speicher = new double [n_punkte*3];
      if(!XYZ_Speicher)  WARN_S( "XYZ_speicher Speicher not available" )
      for (int i = 0; i < n_punkte; ++i)
      { for(int j = 0; j < 3 ; ++j)
        { XYZ_Speicher[i*3+j] = (double)cieXYZ[i][j];
        }
      }
      Lab_buffer = new double [n_punkte*3];
      Lab_shadow_buffer = new double [n_punkte*3];
      if(!Lab_buffer)  WARN_S( "Lab_speicher Speicher not available" )

      XYZtoLab (XYZ_Speicher, Lab_buffer, n_punkte);

    } else
    {
      //colour circle
      n_punkte = 90;
      Lab_buffer = new double [n_punkte*3];
      Lab_shadow_buffer = new double [n_punkte*3];
      if(!Lab_buffer)  WARN_S( "Lab_speicher Speicher not available" )

      for (int i = 0; i < n_punkte; ++i)
      {
          Lab_buffer[i*3+0] = (double)0.666;
          Lab_buffer[i*3+1] = (double)cos(i*2*M_PI/(n_punkte-1))*.714 + .5;
          Lab_buffer[i*3+2] = (double)sin(i*2*M_PI/(n_punkte-1))*.714 + .5;
      }
    }

    RGB_buffer = icc_oyranos.wandelLabNachBildschirmFarben(
               window()->x() + window()->w()/2, window()->y() + window()->h()/2,
         (icc_oyranos.colourServerActive() & XCM_COLOR_SERVER_MANAGEMENT)?edit_:0,
               Lab_buffer, (size_t)n_punkte, opts);

    if(type_ == 1)
      for (int i = 0; i < n_punkte; ++i)
        Lab_buffer[i*3+0] = (double).0;


    DBG_PROG_V( n_punkte )
    // generate shadows
    for (int i = 0; i < n_punkte*2; ++i)
      Lab_shadow_buffer[i] = Lab_buffer[i];
    for (int i = 0; i < n_punkte; ++i) {
      Lab_shadow_buffer[i*3] = background_colour*.40+.35;
      Lab_shadow_buffer[i*3+1] = (Lab_buffer[i*3+1]-.5)*.25+0.5;
      Lab_shadow_buffer[i*3+2] = (Lab_buffer[i*3+2]-.5)*.25+0.5;
    }

    RGB_shadow_buffer = icc_oyranos.wandelLabNachBildschirmFarben(
               window()->x() + window()->w()/2, window()->y() + window()->h()/2,
         (icc_oyranos.colourServerActive() & XCM_COLOR_SERVER_MANAGEMENT)?edit_:0,
               Lab_shadow_buffer, n_punkte, opts);
    if(!RGB_buffer || !RGB_shadow_buffer) 
    {
      WARN_S( "RGB_speicher result not available" )
      DBG_PROG_ENDE
      return;
    }

    GLfloat colour[] =   { arrow_colour[0],arrow_colour[1],arrow_colour[2], 1.0 };

    gl_listen_[SPEKTRUM] = glGenLists(1);
    glNewList( gl_listen_[SPEKTRUM], GL_COMPILE );
    DBG_PROG_V( gl_listen_[SPEKTRUM] ) 

      glDisable (GL_LIGHTING);
      glDisable (GL_ALPHA_TEST_FUNC);
      if(!smooth) {
        glDisable(GL_BLEND);
        glDisable(GL_LINE_SMOOTH);
      } else {
        glEnable(GL_BLEND);
        glEnable(GL_LINE_SMOOTH);
      }

      glLineWidth(line_3*line_mult);
      glColor4f(0.5, 1.0, 1.0, 1.0);
      glBegin(GL_LINE_STRIP);
        for (int i=0 ; i <= (n_punkte - 1); i++) {
          DBG_ICCGL2_S( i<<" "<<Lab_buffer[i*3]<<"|"<<Lab_buffer[i*3+1]<<"|"<<Lab_buffer[i*3+2] )
          DBG_ICCGL2_S( i<<" "<<RGB_buffer[i*3]<<"|"<<RGB_buffer[i*3+1]<<"|"<<RGB_buffer[i*3+2] )
          COLOUR(RGB_buffer[i*3],RGB_buffer[i*3+1],RGB_buffer[i*3+2],1);
          glVertex3d( 
         (Lab_buffer[i*3+2]*cie_b_display_stretch - cie_b_display_stretch/2.),
         (Lab_buffer[i*3+0] - 0.5),
         (Lab_buffer[i*3+1]*cie_a_display_stretch - cie_a_display_stretch/2.)
          );
        }
      glEnd();
      // shadow
      if(type_ > 1)
      {
      glLineWidth(line_2*line_mult);
      glBegin(GL_LINE_STRIP);
      //#define S * .25 + text_colour[0] - shadow
        for (int i=0 ; i <= (n_punkte - 1); i++) {
          COLOUR(RGB_shadow_buffer[i*3],RGB_shadow_buffer[i*3+1],RGB_shadow_buffer[i*3+2],1)
          glVertex3d( 
         (Lab_buffer[i*3+2]*cie_b_display_stretch - cie_b_display_stretch/2.),
         (- 0.5),
         (Lab_buffer[i*3+1]*cie_a_display_stretch - cie_a_display_stretch/2.)
          );
        }
      glEnd();
      }

    glEndList();

    if (XYZ_Speicher) delete [] XYZ_Speicher;
    if (RGB_buffer) delete [] RGB_buffer;
    if (RGB_shadow_buffer) delete [] RGB_shadow_buffer;
    if (Lab_buffer) delete [] Lab_buffer;
    if (Lab_shadow_buffer) delete [] Lab_shadow_buffer;
    oyOptions_Release( &opts );
  }
  DBG_PROG_ENDE
}


Fl_Menu_Item*
cpMenueButton (Fl_Menu_Button* m)
{
  Fl_Menu_Item* mi = new Fl_Menu_Item [m->size()];
  memcpy (mi, m->menu(), sizeof(Fl_Menu_Item) * m->size());
  return mi;
}

void
GL_View::renewMenue_()
{ DBG_PROG_START
  MARK( frei(false); )

  // erase
  menue_cut_->clear();
  menue_background_->clear();
  menue_shape_->clear();
  menue_->clear();
  menue_button_->clear();
  DBG_PROG

  // ->Darstellung
  menue_cut_->add("text_L", 0,c_, (void*)Agviewer::ICCFLY_L, 0);
  menue_cut_->add("text_a", 0,c_, (void*)Agviewer::ICCFLY_a, 0);
  menue_cut_->add("text_b", 0,c_, (void*)Agviewer::ICCFLY_b, 0);
  menue_cut_->add(_("Slice"), 0,c_, (void*)Agviewer::FLYING, 0);
  menue_cut_->add(_("Rotate around slice"),0,c_,(void*)Agviewer::ICCPOLAR, 0);
  DBG_PROG

  if (from_channel_names_.size())
  {
    static char text_L[64];
    static char text_a[64];
    static char text_b[64];
    static char text_S[64];
    sprintf (text_L, "%s %s", from_channel_names_[0].c_str(), _("Slice"));
    menue_cut_->replace( 0, text_L);
    if(from_channel_names_.size() > 1)
      sprintf (text_a, "%s %s", from_channel_names_[1].c_str(), _("Slice"));
    else
      sprintf (text_a, "%s", _("Slice"));
    menue_cut_->replace( 1, text_a);
    if(from_channel_names_.size() > 2)
      sprintf (text_b, "%s %s", from_channel_names_[2].c_str(), _("Slice"));
    else
      sprintf (text_a, "%s", _("Slice"));
    menue_cut_->replace( 2, text_b);
    sprintf (text_S, "%s %s %s", _("Rotate around"),from_channel_names_[0].c_str(), _("axis"));
    menue_cut_->replace( 4, text_S);
  }
  DBG_PROG

  menue_->add (_("Slice plane"),0,0, cpMenueButton(menue_cut_),FL_SUBMENU_POINTER);

  // ->Darstellung->background colour
  menue_background_->add(_("White"), 0,c_, (void*)MENU_WHITE, 0);
  menue_background_->add(_("Light gray"), 0,c_, (void*)MENU_LIGHT_GRAY, 0);
  menue_background_->add(_("Gray"), 0,c_, (void*)MENU_GRAY_GRAY, 0);
  menue_background_->add(_("Dark gray"), 0,c_, (void*)MENU_DARK_GRAY, 0);
  menue_background_->add(_("Black"), 0,c_, (void*)MENU_BLACK, 0);
  DBG_PROG

  menue_shape_->add (_("Background colour"),0,c_,cpMenueButton(menue_background_),FL_SUBMENU_POINTER);

  // ->Darstellung
  if(type_ == 1)
  {
    menue_shape_->add( _("gray"), 0,c_, (void*)MENU_GRAY, 0 );
    menue_shape_->add( _("coloured"), 0,c_, (void*)MENU_COLOUR, 0 );
    menue_shape_->add( _("high contrast"), 0,c_, (void*)MENU_HIGHCONTRAST, 0 );
    // Shells alike displaying, alternatves are "Membranes" or "Shells"
    menue_shape_->add( _("Onion skins"), 0,c_, (void*)MENU_ONIONSKIN, 0 );
    menue_shape_->add( _("Colour line"), 0,c_, (void*)MENU_SPECTRAL_LINE, 0 );
  } else {
    // spheres with their radius symbolise measurement colours
    if(!show_points_as_measurements)
    {
      menue_shape_->add( _("Sphere 1dE"), 0,c_, (void*)MENU_dE1SPHERE, 0 );
      menue_shape_->add( _("Sphere 2dE"), 0,c_, (void*)MENU_dE2SPHERE, 0 );
      menue_shape_->add( _("Sphere 4dE"), 0,c_, (void*)MENU_dE4SPHERE, 0 );
    } else {
#   ifdef Lab_STERN
      menue_shape_->add( _("Star"), 0,c_, (void*)MENU_dE1STAR, 0 );
#   else
      // points are reserved for image colours
      menue_shape_->add( _("Point"), 0,c_, (void*)MENU_dE1STAR, 0 );
#   endif
    }
    menue_shape_->add( _("without markers"), 0,c_, (void*)MENU_DIFFERENCE_LINE, 0);
    menue_shape_->add( _("Spektral line"), 0,c_, (void*)MENU_SPECTRAL_LINE, 0 );
  }
  DBG_PROG
  menue_shape_->add( _("Texts"), 0,c_, (void*)MENU_HELPER, 0 );
  DBG_PROG_V( menue_shape_->size() )
  menue_shape_->replace( menue_shape_->size()-2, _("Texts/Arrows on/off"));

  menue_->add (_("Illustration"),0,c_,cpMenueButton(menue_shape_),FL_SUBMENU_POINTER);

  DBG_PROG_V( menue_->size() )


  // -> (main menue)
  for (int i = 0; i < (int)to_channel_names_.size(); i++) {
    char* p = (char*) to_channel_names_[i].c_str();
    menue_->add( p, 0,c_, (void*)((intptr_t)(MENU_MAX + i)), 0 );
    DBG_PROG_V( MENU_MAX + i <<" "<< to_channel_names_[i] )
  }

  // TODO: -> upper menue
  menue_button_->copy(menue_->menu());
  menue_button_->callback(c_);

  //glStatus(_("left-/middle-/right mouse button -> rotate/cut/menu"), type_);

  MARK( frei(true); )
  DBG_PROG_ENDE
}

void
GL_View::menueInit_()
{
  DBG_PROG_START
  MARK( frei(false); )
  this->begin();
  menue_button_ = new Fl_Menu_Button(x(),y(),w(),h(),0);
  menue_button_->type(Fl_Menu_Button::POPUP3);
  menue_button_->box(FL_NO_BOX);

  Fl_Menu_Button *ms[4];

  ms[0] = menue_ = new Fl_Menu_Button(0,0,w(),h(),""); menue_->hide();
  ms[1] = menue_cut_ = new Fl_Menu_Button(0,0,w(),h(),""); menue_cut_->hide();
  ms[2] = menue_background_ = new Fl_Menu_Button(0,0,w(),h(),""); menue_background_->hide();
  ms[3] = menue_shape_ = new Fl_Menu_Button(0,0,w(),h(),""); menue_shape_->hide();

  for (int i = 0; i < 4; ++i)
    ms[i]->add("dummy",0,0,0,0);

  //renewMenue_();

  this->end();
  MARK( frei(true); )
  DBG_PROG_ENDE
}


/*inline*/ void
GL_View::setPerspective()
{ //DBG_ICCGL_START
    // camera viewing angle
    if (agv_->thin)
      gluPerspective(15, window_proportion,
                     front_cut,
                     front_cut + cut_distance);
    else
      gluPerspective(15, window_proportion,
                     front_cut, 50);
                  // ^-- near cut plane
  //DBG_ICCGL_ENDE
}

void
GL_View::windowShape( )
{ DBG_PROG_START
  if(visible()) {
    glViewport(0,0,w(),h());
    DBG_PROG_V( x()<<" "<< y()<<" "<<w()<<" "<<h())
    window_proportion = (GLdouble)w()/(GLdouble)h();
    if(icc_oyranos.colourServerActive() & XCM_COLOR_SERVER_MANAGEMENT &&
       type_ != 1)
      icc_oyranos.colourServerRegionSet( this, edit_, window_geometry, 0 );
    oyRectangle_SetGeo( window_geometry, x(), y(), w(), h() );
  }
  DBG_PROG_ENDE
}

void
GL_View::drawGL()
{
  DBG_ICCGL_START

  if(!valid_)
  {
    // complete initialisation
    gl_font( FL_HELVETICA, 10 );
    refresh_();
    GLinit_();  DBG_PROG
    windowShape();
  }

  if(update_geometries_)
  {
    int err = createGLLists_();
    if(err)
      return;

    update_geometries_ = false;
    valid_ = true;
  }

  if(!frei()) return;
  MARK( frei(false); )

  time_ = icc_examin_ns::zeitSekunden();


  int scal = 1;
  GLfloat colour[] =   { text_colour[0],text_colour[1],text_colour[2], 1.0 };
  GLdouble oX=.0,oY=.0,oZ=.0;
  GLdouble X=.0,Y=.0,Z=.0;
  std::string channel_name;

  icc_examin->report_owner = 1;
  if(visible() &&
     icc_examin->frei() )
  {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


    glLoadIdentity();
    glMatrixMode(GL_PROJECTION);


    glEnable(GL_TEXTURE_2D);
    glDisable(GL_LIGHTING);
    if(!smooth) {
      glDisable(GL_BLEND);
      glDisable(GL_LINE_SMOOTH);
    } else {
      glEnable(GL_BLEND);
      glEnable(GL_LINE_SMOOTH);
    }

    // start drawing
    glPushMatrix();

      GL_View::setPerspective();

      /* so this replaces gluLookAt or equiv */
      agv_->agvViewTransform();

      glMatrixMode(GL_MODELVIEW);
      glLoadIdentity();

      glCallList( gl_listen_[SPEKTRUM] ); DBG_ICCGL_V( gl_listen_[SPEKTRUM] )
      glCallList( gl_listen_[CONTOURS] ); DBG_ICCGL_V( gl_listen_[CONTOURS] )
      if (show_helpers) {
        glCallList( gl_listen_[HELPER] ); DBG_ICCGL_V( gl_listen_[HELPER] )
      }
      glCallList( gl_listen_[RASTER] ); DBG_ICCGL_V( gl_listen_[RASTER] )
      //DBG_S( id_ << ": " << gl_listen_[RASTER], ICC_MSG_WARN)
      if(point_form == MENU_dE1STAR)
        glCallList( gl_listen_[POINTS] );
      glCallList( gl_listen_[POINTS] ); DBG_ICCGL_V( gl_listen_[POINTS] )

      oyOptions_s * opts = icc_examin->options();
      Lab_s lab;
      double l[3];

      // localisate
      if( epoint_ && Fl::belowmouse() != this )
      {
        const char * temp = oyNamedColour_GetName( epoint_, oyNAME_NICK, 0 );

        if(temp)
          sprintf( text, "%s", temp );
        oyNamedColour_GetColourStd( epoint_, oyEDITING_LAB, l, oyDOUBLE,0,opts);
        CIELabToLab( l, lab );
        oY = LToY( lab.L );
        oZ = aToZ( lab.a );
        oX = bToX( lab.b );
        mousePoint_( oX, oY, oZ, X, Y, Z, 0 );
      } else
      {
        text[0] = 0;
        mousePoint_( oX, oY, oZ, X, Y, Z, 1 );

        GLfloat grenze = 3.2;

        if(from_channel_names_.size() &&
                       -grenze < oY && oY < grenze &&
                       -grenze < oX && oX < grenze &&
                       -grenze < oZ && oZ < grenze)
        {
          double d[3];
          d[0] = YToL(oY);
          d[1] = ZToa(oZ);
          d[2] = XTob(oX);
          LabToCIELab( d, d, 1 );
          oyLab2XYZ(d, d);

          if(!mouse_3D_hit)
          {
            oyProfile_s * prof = oyProfile_FromStd( oyEDITING_XYZ, NULL );
            mouse_3D_hit = oyNamedColour_Create( NULL, NULL,0, prof, 0 );
            oyProfile_Release( &prof );
          }

          oyNamedColour_SetColourStd ( mouse_3D_hit, oyEDITING_XYZ,
                                       (oyPointer)d, oyDOUBLE, 0, opts );
          MARK( frei(true); )
          notify( ICCexamin::GL_MOUSE_HIT3D );
          MARK( frei(false); )
          if(epoint_)
          {
            oyNamedColour_GetColourStd( epoint_, oyEDITING_LAB, d, oyDOUBLE,
                                        0, opts );
            CIELabToLab( d, lab );
            oY = LToY( lab.L );
            oZ = aToZ( lab.a );
            oX = bToX( lab.b );
            mousePoint_( oX, oY, oZ, X, Y, Z, 0 );
            const char * temp = oyNamedColour_GetName( epoint_, oyNAME_NICK, 0 );
            if(temp)
              sprintf( text, "%s", temp );
            else
              text[0] = 0;
          } 
          if(!strlen(text))
          {
            double d[3];
            d[0] = YToL(oY);
            d[1] = ZToa(oZ);
            d[2] = XTob(oX);
            LabToCIELab( d, d, 1 );
            text[0] = 0;
            for(int i = 0; i < (int)from_channel_names_.size(); ++i)
              if(i < 3)
                sprintf( &text[strlen(text)], "%s:%.02f ",
                               from_channel_names_[i].c_str(), d[i] );
          }
        } else {
          if(!epoint_)
            ; //epoint_.name = (char*) myAllocFunc(1);
        }
      }

    l[0] = oY+0.5;
    l[1] = oZ/2.55+0.5;
    l[2] = oX/2.55+0.5;
    double *rgb_ = 0, *rgb = 0;

      if( (strlen(text) || epoint_) &&
          type() != 1 )
      {
        rgb_ = rgb = icc_oyranos.wandelLabNachBildschirmFarben( 
               window()->x() + window()->w()/2, window()->y() + window()->h()/2,
         (icc_oyranos.colourServerActive() & XCM_COLOR_SERVER_MANAGEMENT)?edit_:0,
                                 l, 1, opts);

        icc_examin->statusFarbe(l[0],l[1],l[2]);
          DBG_PROG_V( l[0]<<" "<<l[1]<<" "<<l[2] )
        if(rgb)
        {
#         ifndef LIGHTING_
          glDisable(GL_LIGHTING);
#         endif
          glPushMatrix();
            glLineWidth(line_3*line_mult);
            COLOUR(rgb[0], rgb[1], rgb[2],1)
            glBegin(GL_LINES);
              glVertex3f( oX, oY, oZ );
              glVertex3f( oX, -0.5, oZ );
            glEnd();
          glPopMatrix();
            DBG_PROG_V( rgb[0] <<" "<< rgb[1] <<" "<< rgb[2] )
#         ifndef LIGHTING_
          glEnable(GL_LIGHTING);
#         endif
        }
        glLineWidth(line_1*line_mult);
      }


      if(triangle_nets.size())
      {
        MARK( frei(true); )
        triangle_nets.frei(false);

        int some_active = 0;
        for(size_t i = 0; i < triangle_nets.size(); ++i)
          if(triangle_nets[i].active())
            some_active = 1;

        if(some_active)
          refreshNets();

        triangle_nets.frei(true);
        MARK( frei(false); )
      }

    glPopMatrix();
    // End of drawing

    // Text
    Lab2GL(text_colour[0],text_colour[1],text_colour[2],1)

    GLfloat lmodel_ambient[] = {background_colour,
                                background_colour,
                                background_colour, 1.0};
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, lmodel_ambient);

    if(1)
    {
       glPushMatrix(); 
         // text in scene
         glDisable(GL_TEXTURE_2D);
         glDisable(GL_LIGHTING);
         glLoadIdentity();
         glMatrixMode(GL_MODELVIEW);
         glOrtho( 0, w(), 0, h(), 0.1, 100.0);


         if (show_helpers)
           adornText_();

         // can go away from drawing function
         if(strlen(text))
         {
            DBG_ICCGL_V( X<<" "<<Y<<" "<<Z )
            DBG_ICCGL_V( oX<<" "<<oY<<" "<<oZ )

            float f = 0.0f;
            if(background_colour < 0.6)
              f = 1.f;
            Lab2GL(f,.5,.5,1)

            // text above scene
            glLoadIdentity();
            glMatrixMode(GL_PROJECTION);

            glLoadIdentity();
            glOrtho(0,w(),0,h(),-10.0,10.0);

            glRasterPos3d (X, Y, 9.999);
                      
            if(type() == 1)
            {
              DBG_PROG_V( oY<<" "<<oZ<<" "<<oX )
                        /*if(-0.505 <= oY && oY <= 0.505 &&
                           -0.505 <= oZ && oZ <= 0.505 &&
                           -0.505 <= oX && oX <= 0.505)*/
              if(table_.size())
              {
                 int L = (int)((oY+0.5)*table_.size());
                 if(0 <= L && L < (int)table_.size()) {
                 int a = (int)((oZ+0.5)*table_[L].size());
                 if(0 <= a && a < (int)table_[L].size()) {
                 int b = (int)((oX+0.5)*table_[L][a].size());
                 if(0 <= b && b < (int)table_[L][a].size()) {
# ifdef DEBUG
                   double value = table_[L][a][b][channel];
# endif
                   DBG_PROG_V( L<<" "<<a<<" "<<b<<" "<<value )
                   DBG_PROG_V( table_.size()<<" "<<table_[L].size()<<" "<<
                               table_[L][a].size()<<" "<<channel )
                   // "Pos" is a abreviation to position and will be visible in the small statusline with position coordinates
                   sprintf( text,"%s[%d][%d][%d]", _("Pos"),
                            (int)((oY+0.5)*table_.size()),
                            (int)((oZ+0.5)*table_[L].size()),
                            (int)((oX+0.5)*table_[L][a].size()));
                   for (int i = 3 ; i < (int)channels_.size(); ++i) {
                     sprintf(&text[strlen(text)], "%d|", channels_[i]);
                   }
                   sprintf(&text[strlen(text)], " ");
                   for (int i = 0 ; i < (int)table_[L][a][b].size(); ++i) {
                     if(i == channel) sprintf(&text[strlen(text)], "[");
                     sprintf(&text[strlen(text)], "%.5f", table_[L][a][b][i]);
                     if(i == channel) sprintf(&text[strlen(text)], "]");
                     if (i != (int)table_[L][a][b].size()-1)
                       sprintf(&text[strlen(text)], " ");
                   }
                   glStatus( text, type_ );
                   drawOTEXT (ortho_font, scal, text)
                 }}}
               }
            } else
            {
              if(!epoint_)
                glStatus( text, type_ );
              drawOTEXT (ortho_font, scal, text)

              /* Circle around point */
              if( epoint_) 
              {
                GLdouble width = 24;
                int steps = (int)(8 + width/2);
                GLdouble x, y,
                         s = 2*M_PI/(GLdouble)steps; // static variable

                // lines
                Lab2GL(f,.5,.5,1)
                glBegin(GL_LINE_STRIP);
                  glVertex3d( X+2*width, Y-1, 9.999 );
                  glVertex3d( X+0.5*width, Y-1, 9.999 );
                glEnd();
                glBegin(GL_LINE_STRIP);
                  glVertex3d( X-1.2*width, Y-1, 9.999 );
                  glVertex3d( X-0.2*width, Y-1, 9.999 );
                glEnd();
                glBegin(GL_LINE_STRIP);
                  glVertex3d( X, Y+1.2*width, 9.999 );
                  glVertex3d( X, Y+0.5*width, 9.999 );
                glEnd();

                // circle
                width += 1;
                glBegin(GL_LINE_STRIP);
                  for(int i = 0; i <= steps; ++i)
                  {
                    x = cos(i*(double)s)*width;
                    y = sin(i*(double)s)*width;
                    glVertex3d( X+x, Y+y, 9 );
                  }
                glEnd();
              }
            }

            if(icc_debug == 14) {
              glScalef(100,100,100);
              drawCoordinates_();
              glScalef(.01,.01,.01);
            }
            DBG_ICCGL_V( mouse_x_-x() <<" "<< -mouse_y_+h() )
         }

       glPopMatrix();
    }

    bool line_new = false;
    double frame_duration = icc_examin_ns::zeitSekunden() - time_;
    if(frame_duration > 1./15. )
    {
      if(smooth && !onion_skin)
      {
        smooth = 0;
        line_new = true;
      }
    }
    if(frame_duration < 1./40. || onion_skin)
    {
      if(!smooth)
      {
        smooth = 1;
        line_new = true;
      }
    }
    if(line_new) {
      //showSprectralLine_();
      adorn_();
    }

    // measure speed
    static double duration_old = 0;
    snprintf(t, 128, "time_: %.01f id: %d f/s: %.03f theoretical f/s: %.03f",
             time_, id_, 1./(time_ - duration_old), 1./frame_duration);
    duration_old = time_;
    DBG_PROG_V( t )


#   ifdef HAVE_FTGL
    if(ortho_font)
      glRasterPos2d(0, h() -10 );
#   endif
    if(mouse_x_old != mouse_x_ || mouse_y_old != mouse_y_)
      mouse_stays = true;
    mouse_x_old = mouse_x_;
    mouse_y_old = mouse_y_;

    // colour channel name
    // text
    {
      glPushMatrix();
       glLoadIdentity();
       glMatrixMode(GL_PROJECTION);
       glLoadIdentity();
       glOrtho(0,w(),0,h(),-10.0,10.0);

       Lab2GL(text_colour[0],text_colour[1],text_colour[2],1)

       glTranslatef(5,-12,0/*8.8 - cut_distance*3*/);

       if(type() == 1) {
         channel_name.append(_("Channel"));
         channel_name.append(": ");
         channel_name.append(channelName());

/*         oyNamedColour_s   * c = oyNamedColours_GetRef( colours_, 0 );
         const char* chan_name = 0;
         if(c)
         {
           chan_name = oyProfile_GetChannelName( c->profile_,
                                                           channel, oyNAME_NAME );
           channel_name.append( chan_name );
           oyNamedColour_Release( &c );
  
         } else if(from_channel_names_.size() > channel) {

             channel_name.append( from_channel_names_[channel] );
         }
  */

#        ifdef HAVE_FTGL
         if(ortho_font)
           glRasterPos3d(0,ortho_font->FaceSize()*1.6/5.+20.,9.99);
#        endif
         drawOTEXT (ortho_font, scal, channel_name.c_str()) 
       } else
       {
         MARK( frei(true); )
         triangle_nets.frei(false);
         for (unsigned int i=0;
                i < triangle_nets.size();
                  ++i)
         {
           std::string text;
           text = triangle_nets[i].name;
           DBG_PROG_V( triangle_nets[i].name )
#          ifdef HAVE_FTGL
           if(ortho_font)
             glRasterPos2d(0, 20 + ortho_font->FaceSize() / 0.94
                                   * (triangle_nets.size()-i-1) );
#          endif
           if(triangle_nets[i].active())
             drawOTEXT (ortho_font, scal, text.c_str())
         }
         triangle_nets.frei(true);
         MARK( frei(false); )
       }

       if(icc_debug)
         drawOTEXT (ortho_font, scal, t)

       glEnable(GL_TEXTURE_2D);
       glEnable(GL_LIGHTING);
      glPopMatrix();
    }

    if( epoint_ && Fl::belowmouse() == this )
      // allow other colours to appear
      oyNamedColour_Release( &epoint_ );

    if(rgb_) delete [] rgb_;
    oyOptions_Release( &opts );

  } else
    if(!icc_examin->frei())
      DBG_PROG_S("icc_examin not free")
  icc_examin->report_owner = 0;

  MARK( frei(true); )

  DBG_ICCGL_ENDE
}

void
GL_View::achsNamen    (ICClist<std::string> achs_namen)
{ DBG_PROG_START
  if (achs_namen.size())
    from_channel_names_ = achs_namen;
  else
  { from_channel_names_.clear();
    from_channel_names_.push_back ("?");
    from_channel_names_.push_back ("?");
    from_channel_names_.push_back ("?");
  }
  valid_=false;
  DBG_PROG_ENDE
}

/** @func  namedColours
 *  @brief export our colour spots
 *
 *  The returned oyStructList_s contains the unaltered oyNamedColours_s objects
 *  we obtained in GL_View::namedColours(x).
 *  We set the name in the oyStructList_s->oy_ member. You can read it with 
 *  oyranos::oyObject_GetNames. The nick (__FILE__) and name ("colour lists")
 *  are static. The description contains scene informations as follows
 *  - show_points_as_pairs is present with show_points_as_pairs being true
 *  - show_points_as_measurements with show_points_as_measurements being true
 *  - spectralline with spectral_line being true
 *  - viewpoint: is followed by the Lab position. The viewing angle is 15°.
 *
 *  @version ICC Examin: 0.45
 *  @date    2008/02/20
 *  @since   2005/00/00 (ICC Examin: 0.0.x)
 */
oyStructList_s*
GL_View::namedColours       ()
{
  oyStructList_s * colours;

  DBG_PROG_START

  std::string text = "ICC Examin scene:";
  char t[24];
  int error = 0;
  if(show_points_as_pairs)
    text.append(" show_points_as_pairs");
  if(show_points_as_measurements)
    text.append(" show_points_as_measurements");
  if(spectral_line)
    text.append(" spectralline");
    
       float EyeAz = agv_->eyeAzimuth(),
             EyeEl = agv_->eyeElevation(),
             EyeDist = agv_->eyeDist(),
             X = (float)(-EyeDist*sin(TORAD(EyeAz))*cos(TORAD(EyeEl))),  // CIE*b
             Y = (float)(EyeDist*sin(TORAD(EyeEl))),                     // CIE*L
             Z = (float)(EyeDist*cos(TORAD(EyeAz))*cos(TORAD(EyeEl)));   // CIE*a
  text.append(" viewpoint:");
  sprintf(t, "%.02f %.02f %.02f", Y*100.0, Z*100.0, X*100.0);
  text.append(t);
  
  if(colours_)
    error = oyObject_SetNames ( colours_->oy_, __FILE__, "colour lists",
                                  text.c_str() );
  if(error)
    WARN_S( "oyObject_SetNames: " << error );
  colours = oyStructList_Copy( colours_, NULL );

  DBG_PROG_ENDE
  return colours;
}

void
GL_View::namedColours       (oyStructList_s * colours)
{
  DBG_PROG_START
  MARK( frei(false); )
  oyStructList_Release( &colours_ );

  
  colours_ = oyStructList_Copy( colours, NULL );

  if (!show_points_as_measurements &&
      point_form == MENU_dE1STAR &&
      colours_)
    point_form = MENU_dE1SPHERE;

  MARK( frei(true); )

  valid_=false;
  redraw();

  DBG_PROG_ENDE
}

void
GL_View::namedColoursRelease()
{
  MARK( frei(false); )
  oyStructList_Release( &colours_ );
  oyNamedColour_Release( &mouse_3D_hit );
  MARK( frei(true); )
}

void
GL_View::clearNet ()
{
  MARK( frei(false); )
  icc_examin_ns::BSPfreeTree(&bsp);
  MARK( frei(true); )

  valid_ = false;
  damage(FL_DAMAGE_ALL);
  redraw();
}


void
GL_View::emphasizePoint    (oyNamedColour_s * colour)
{ DBG_PROG_START
  // show curve from tag_browser
  MARK( frei(false); )
  if(colour)
  {
    oyNamedColour_Release( &epoint_ );
    epoint_ = oyNamedColour_Copy( colour, 0 );
 
    double l[3];
    Lab_s lab;
    oyOptions_s * opts = icc_examin->options();
    oyNamedColour_GetColourStd( colour, oyEDITING_LAB, l, oyDOUBLE, 0, opts );
    CIELabToLab( l, lab );
    oyOptions_Release( &opts );

    icc_examin->statusFarbe( lab.L, lab.a, lab.b );
    uint32_t create = 1;
    glStatus( oyNamedColour_GetName( colour, oyNAME_DESCRIPTION, create ),
              type_ );
  }
  MARK( frei(true); )

  //valid_=false;
  redraw();
  DBG_PROG_ENDE
}

void getFacesFromICCnetz ( const ICCnetz & net, icc_examin_ns::FACE **fList,
                           float scale_x, float scale_y, float scale_z )
{
  icc_examin_ns::FACE *fListTail = 0;
  icc_examin_ns::VERTEX *vList = 0,
                        *vListTail = 0;
  icc_examin_ns::COLOR color;
  icc_examin_ns::PLANE plane;
  int err = 0;

  if(*fList)
  {
    fListTail = *fList;
    while(fListTail->fnext)
      fListTail = fListTail->fnext;
  }

  std::multimap<double,DreiecksIndexe>::const_iterator it;
  for( it = net.indexe.begin(); it != net.indexe.end(); ++it )
  {
    std::pair<double,DreiecksIndexe> index_p( *it );
    icc_examin_ns::VERTEX * v = 0;

    for(int j = 2; j >= 0; --j)
    {
      if(net.grau)
      {
        color.rr = color.gg = color.bb = net.schattierung;
      } else {
        color.rr = net.punkte[ index_p.second.i[j] ].farbe[0];
        color.gg = net.punkte[ index_p.second.i[j] ].farbe[1];
        color.bb = net.punkte[ index_p.second.i[j] ].farbe[2];
      }
      if(net.active())
        color.aa = net.undurchsicht;
      else
        color.aa = 0.0;

      v = allocVertex( net.punkte[ index_p.second.i[j] ].koord[0]*scale_x,
                       net.punkte[ index_p.second.i[j] ].koord[1]*scale_y,
                       net.punkte[ index_p.second.i[j] ].koord[2]*scale_z,
                       &color);

      appendVertex( &vList, &vListTail, v );
    }

    if (vList != 0)
    {
      err = 0;
      /* previous face? */
      icc_examin_ns::appendVertex( &vList,&vListTail, /* append duplicate 1st vertex  */
           icc_examin_ns::allocVertex( vList->xx,vList->yy,vList->zz,&color ) );

      err = icc_examin_ns::computePlane(vList->xx,vList->yy,vList->zz,
                         vList->vnext->xx,vList->vnext->yy,vList->vnext->zz,
                         vList->vnext->vnext->xx,
                         vList->vnext->vnext->yy,
                         vList->vnext->vnext->zz, &plane);

      if(!err)
        icc_examin_ns::appendFace( fList, &fListTail,
                                       icc_examin_ns::allocFace(vList,&plane) );
    }

         /* save vars for this face and start new vertex list */
         /* printf("f %f/%f/%f",color.rr,color.gg,color.bb); */
    vList = vListTail = 0;
  }

  if (vList != 0) { /* process last face (or only) */
      err = 0;
      appendVertex(&vList,&vListTail, /* append duplicate 1st vertex */
                   allocVertex(vList->xx,vList->yy,vList->zz,&color));
      err = computePlane(vList->xx,vList->yy,vList->zz,vList->vnext->xx,
                   vList->vnext->yy,vList->vnext->zz,vList->vnext->vnext->xx,
                   vList->vnext->vnext->yy,vList->vnext->vnext->zz,&plane);
      if(!err)
        appendFace(fList,&fListTail,allocFace(vList,&plane));
  }
}

void drawGLFaceList(FILE *fp,const icc_examin_ns::FACE *faceList)
{
  const icc_examin_ns::FACE *ftrav = 0;
  for (ftrav= faceList; ftrav != 0; ftrav= ftrav->fnext)
  {
    icc_examin_ns::VERTEX *vtrav = 0;
      
    if(icc_debug)
    {
      glPointSize(5);
      glColor4d(.0, .0, .0, 1. );
      glBegin(GL_POINTS);
        for (vtrav= ftrav->vhead; vtrav->vnext != 0;
             vtrav= vtrav->vnext)
          glVertex3f( vtrav->zz, vtrav->xx, vtrav->yy );
      glEnd();

      glBegin(GL_LINES);
        for (vtrav= ftrav->vhead; vtrav->vnext != 0;
             vtrav= vtrav->vnext)
        {
          glVertex3f( vtrav->zz, vtrav->xx, vtrav->yy );
          glVertex3f( vtrav->zz + ftrav->plane.cc*.1,
                      vtrav->xx + ftrav->plane.aa*.1,
                      vtrav->yy + ftrav->plane.bb*.1 );
        }
      glEnd();
    }

    if(ftrav->vhead->color.aa > 0.0)
    {
      glBegin(GL_TRIANGLE_FAN /*S*/);
        glNormal3f( ftrav->plane.cc, ftrav->plane.aa, ftrav->plane.bb );

        for (vtrav= ftrav->vhead; vtrav->vnext != 0;
             vtrav= vtrav->vnext)
        {
          COLOURS (    vtrav->color.rr, vtrav->color.gg, vtrav->color.bb,
                      vtrav->color.aa );
          glVertex3f( vtrav->zz, vtrav->xx, vtrav->yy );
        }
      glEnd();
    }
  }
}

void
GL_View::setBspFaceProperties_( icc_examin_ns::FACE *faceList )
{
  DBG_5_START

  icc_examin_ns::FACE * ftrav = 0;
  icc_examin_ns::VERTEX * vtrav = 0;

  if(faceList == 0) return;

//#define USE_OY_NC
  double lab[3];
#ifdef USE_OY_NC
  oyProfile_s * prof = oyProfile_FromStd( oyEDITING_LAB, 0 );
  oyProfile_s * prof_disp = NULL;
  if(icc_oyranos.colourServerActive() & XCM_COLOR_SERVER_MANAGEMENT)
    prof_disp = oyProfile_Copy( edit_, NULL );
  else
    prof_disp = icc_oyranos.icc_oyranos.oyMoni(
              window()->x() + window()->w()/2, window()->y() + window()->h()/2);
  oyNamedColour_s * c = oyNamedColour_Create( 0, 0, 0, prof, 0 );
#endif
  oyOptions_s * opts = icc_examin->options();

  /* updateNet_ takes care of triangle_nets.frei */
  for( ftrav = faceList; ftrav != 0; ftrav = ftrav->fnext )
  {
    int pos = ftrav->group;
    ICCnetz & net = triangle_nets[pos];

    for( vtrav = ftrav->vhead; vtrav->vnext != 0; vtrav = vtrav->vnext )
    {
      if(net.grau)
      {
        vtrav->color.rr = vtrav->color.gg = vtrav->color.bb = net.schattierung;
      } else {

#if USE_OY_NC
        double rgba[4] = {0,0,0,1};
        lab[0] = vtrav->xx; 
        lab[1] = vtrav->yy/cie_a_display_stretch;
        lab[2] = vtrav->zz/cie_b_display_stretch;
        LabToCIELab( lab, lab, 1 );

        oyNamedColour_SetColourStd( c, oyEDITING_LAB, lab, oyDOUBLE, 0, opts );

        oyNamedColour_GetColour( c, prof_disp, rgba, oyDOUBLE, 0, opts );

        vtrav->color.rr = rgba[0];
        vtrav->color.gg = rgba[1];
        vtrav->color.bb = rgba[2];
#else
        lab[0] = vtrav->xx;
        lab[1] = vtrav->yy/cie_a_display_stretch;
        lab[2] = vtrav->zz/cie_b_display_stretch;

        double * rgb = NULL;
        rgb = icc_oyranos.wandelLabNachBildschirmFarben( 
               window()->x() + window()->w()/2, window()->y() + window()->h()/2,
         (icc_oyranos.colourServerActive() & XCM_COLOR_SERVER_MANAGEMENT)?edit_:0,
                                 lab, 1, opts);

        if(rgb)
        {
          vtrav->color.rr = rgb[0];
          vtrav->color.gg = rgb[1];
          vtrav->color.bb = rgb[2];
          delete [] rgb;
        }
#endif
      }
      if(net.active())
        vtrav->color.aa = net.undurchsicht;
      else
        vtrav->color.aa = 0.0;
    }
  }

#ifdef USE_OY_NC
  oyNamedColour_Release( &c );
  oyProfile_Release( &prof_disp );
#endif
  oyOptions_Release( &opts );

  DBG_5_ENDE
}

/* updateNet_ takes care of triangle_nets.frei */
void
GL_View::setBspProperties_( icc_examin_ns::BSPNODE *bsp )
{
  //DBG_PROG_START

  icc_examin_ns::BSPNODE * bspNode = bsp;

  if (bspNode == 0) return;

  if (bspNode->kind == icc_examin_ns::PARTITION_NODE)
  {
    setBspProperties_( bspNode->node->negativeSide );
    setBspFaceProperties_( bspNode->node->sameDir );
    setBspFaceProperties_( bspNode->node->oppDir );
    setBspProperties_( bspNode->node->positiveSide );
  }

  //DBG_PROG_ENDE
}


void
GL_View::updateNet_()
{
  DBG_PROG_START

  if(!bsp)
    loadNets_( triangle_nets );

  MARK( frei(false); )
  triangle_nets.frei(false);
  if(bsp)
    setBspProperties_(bsp);
  triangle_nets.frei(true);
  MARK( frei(true); )

  DBG_PROG_ENDE
}

void
GL_View::loadNets_       (const icc_examin_ns::ICCThreadList<ICCnetz> & d_n)
{
  DBG_PROG_START

  MARK( frei(false); )
  triangle_nets.frei(false);

  net.clear();

    // used by BSPtraverseTreeAndRender
  icc_examin_ns::drawFaceList = drawGLFaceList;

  if(d_n.size())
  {
    icc_examin_ns::BSPfreeTree(&bsp);

    icc_examin_ns::FACE *faceList = 0;
    int old_faces_n = 0;
    for(unsigned i = 0; i < d_n.size(); ++i)
    {
      getFacesFromICCnetz( d_n[i], &faceList,
                           1, cie_a_display_stretch, cie_b_display_stretch );

      icc_examin_ns::FACE *ftrav = 0;
      int n = 0;
      for (ftrav= faceList; ftrav != 0; ftrav= ftrav->fnext)
      {
        ++n;
        if(n > old_faces_n)
          ftrav->group = i;         // tell about the net's group id
      }
      old_faces_n = n;
    }

    if(faceList)
    {
      bsp = icc_examin_ns::BSPconstructTree(&faceList);
      icc_examin_ns::freeFaceList( &faceList );
    } else if (0)
      for(unsigned i = 0; i < d_n.size(); ++i)
        net.insert( d_n[i] );

    for(unsigned k = 0; k < net.punkte.size(); ++k)
    {
      net.punkte[k].koord[1] *= cie_a_display_stretch;
      net.punkte[k].koord[2] *= cie_b_display_stretch;
    }

  }

  triangle_nets.frei(true);
  MARK( frei(true); )

  valid_=false;
  redraw();
  DBG_PROG_ENDE
}


/**
 *
 *  @param[in]     vect                The 3 dimensional tables
 *  @param[in]     achs_namen          the coordinate source channel names
 *  @param[in]     nach                the target channel names
  * @param[in]     channels            selected channels: 1 - selected,
 *                                     0 - not selected, -1 - non existent
 */
void
GL_View::loadTable (ICClist<ICClist<ICClist<ICClist<double> > > > vect,
                           ICClist<std::string> achs_namen,
                           ICClist<std::string> nach,
                                       ICClist<int>        channels)
{ DBG_PROG_START

  MARK( frei(false); )
  table_ = vect;  DBG_PROG
  to_channel_names_ = nach; DBG_PROG

  achsNamen(achs_namen);
  channels_ = channels;

  MARK( frei(true); )

  namedColoursRelease();

  valid_=false;
  redraw();

  glStatus(_("left-/middle-/right mouse button -> rotate/cut/menu"), type_);

  DBG_PROG_ENDE
}


void
GL_View::menuEvents ( int value )
{
  DBG_PROG_START

  if(visible()) {
    ;//DBG_PROG_S( "visible "<< id )
  }

  {
    if (value >= MENU_MAX &&
        value < 100) {
      channel = value - MENU_MAX; DBG_PROG_V( channel )
      glStatus(_("left-/middle-/right mouse button -> rotate/cut/menu"), type_);
    }
#   if APPLE
    double colour_corr = 1./*0.6666*/ *0.8;
#   else
    double colour_corr = 1.*.8;
#   endif

    switch (value) {
    case MENU_AXES:
      DrawAxes = !DrawAxes;
      break;
    case MENU_QUIT:
      break;
    case MENU_CUBE:
      point_form = MENU_CUBE;
      break;
    case MENU_GRAY:
      point_colour = MENU_GRAY;
      onion_skin = 0;
      break;
    case MENU_COLOUR:
      point_colour = MENU_COLOUR;
      onion_skin = 0;
      break;
    case MENU_HIGHCONTRAST:
      point_colour = MENU_HIGHCONTRAST;
      onion_skin = 0;
      break;
    case MENU_ONIONSKIN:
      if (! onion_skin)
         onion_skin = 5;
      else
         onion_skin = 0;
      break;
    case MENU_dE1STAR:
       point_form = MENU_dE1STAR;
      break;
    case MENU_dE1SPHERE:
       point_form = MENU_dE1SPHERE;
      break;
    case MENU_dE2SPHERE:
       point_form = MENU_dE2SPHERE;
      break;
    case MENU_dE4SPHERE:
       point_form = MENU_dE4SPHERE;
      break;
    case MENU_DIFFERENCE_LINE:
       if(point_form == MENU_dE1STAR) point_form = MENU_DIFFERENCE_LINE; else point_form = MENU_dE1STAR;
      break;
    case MENU_SPECTRAL_LINE:
      if (! spectral_line)
         spectral_line = MENU_SPECTRAL_LINE;
      else
         spectral_line = 0;
      break;
    case MENU_HELPER:
      if (! show_helpers)
         show_helpers = MENU_HELPER;
      else
         show_helpers = 0;
      break;
    case MENU_WHITE:
      background_colour = 1.;//MENU_WHITE;
      arrow_colour[0] = (float)(1.*colour_corr);
      text_colour[0] = (float)(.75*colour_corr);
      break;
    case MENU_LIGHT_GRAY:
      background_colour = 0.75;//MENU_LIGHT_GRAY;
      arrow_colour[0] = (float)(1.0*colour_corr);
      text_colour[0] = (float)(0.5*colour_corr);
      break;
    case MENU_GRAY_GRAY:
      background_colour = 0.5;//MENU_GRAY_GRAY;
      arrow_colour[0] = (float)(.75*colour_corr);
      text_colour[0] = (float)(0.25*colour_corr);
      break;
    case MENU_DARK_GRAY:
      background_colour = 0.25;//MENU_DARK_GRAY;
      arrow_colour[0] = (float)(0.5*colour_corr);
      text_colour[0] = (float)(0.75*colour_corr);
      break;
    case MENU_BLACK:
      background_colour = 0.0;//MENU_BLACK;
      arrow_colour[0] = (float)(.25*colour_corr);
      text_colour[0] = (float)(0.5*colour_corr);
      break;
    case Agviewer::FLYING:
      glStatus(_("left mouse button -> go back"), type_);
      agv_->thin = true;
      break;
    case Agviewer::ICCFLY_L:
      if(type() == 2) {
        agv_->eyeDist( 2 * agv_->dist() );
        front_cut = std_front_cut + agv_->dist();
      } else {
        agv_->eyeDist( agv_->dist() );
        front_cut = std_front_cut;
      }
      glStatus(_("left mouse button -> go back"), type_);
      agv_->thin = true;
      break;
    case Agviewer::ICCFLY_a:
      front_cut = std_front_cut;
      glStatus(_("left mouse button -> go back"), type_);
      agv_->thin = true;
      break;
    case Agviewer::ICCFLY_b:
      front_cut = std_front_cut;
      glStatus(_("left mouse button -> go back"), type_);
      agv_->thin = true;
      break;
    case Agviewer::ICCPOLAR:
      agv_->thin = true;
    case Agviewer::POLAR:
      if(type() == 1)
        agv_->thin = true;
      else
        agv_->thin = false;
      break;
    case Agviewer::AGV_STOP:
      agv_->thin = false;
      glStatus(_("left-/middle-/right mouse button -> rotate/cut/menu"), type_);
      break;
    }
  }

  if(visible()) {
    icc_examin->alle_gl_fenster->notify(ICCexamin::GL_AUFFRISCHEN);
  }

  if(value >= 100)
    agv_->agvSwitchMoveMode (value);

  DBG_PROG_V( value<<" "<<id_ )
  DBG_PROG_ENDE
}


#define DBG_BUTTON 1
#if DBG_BUTTON
# define DBG_BUTTON_S(text) DBG_MEM_S(text)
#else
# define DBG_BUTTON_S(text)
#endif

int
GL_View::handle( int event )
{
  DBG_ICCGL_START
  int mouse_button = Fl::event_state();
  int end = 1;
  DBG_MEM_V( dbgFltkEvent(event) )

  keyEvents(event);

  switch(event)
  {
    case FL_PUSH:
         if(mouse_button & FL_BUTTON3 ||
           (mouse_button & FL_BUTTON1 && mouse_button & FL_CTRL) ) {
           menue_button_->popup();
           break;
         }
         if(mouse_button & FL_BUTTON1 && mouse_button & FL_SHIFT)
           mouse_button = FL_BUTTON2;

         if(mouse_button & FL_BUTTON2)
           fl_cursor( FL_CURSOR_NS, FL_BLACK, FL_WHITE );
         else
           fl_cursor( FL_CURSOR_HAND, FL_BLACK, FL_WHITE );

         agv_->agvHandleButton( mouse_button, event, Fl::event_x(),Fl::event_y());
         DBG_BUTTON_S( "FL_PUSH bei: " << Fl::event_x() << "," << Fl::event_y() )
         break;
    case FL_RELEASE:
         agv_->agvHandleButton(Fl::event_state(),event, Fl::event_x(),Fl::event_y());
         DBG_BUTTON_S( "FL_RELEASE bei: " << Fl::event_x() << "," << Fl::event_y() )
         fl_cursor( FL_CURSOR_DEFAULT, FL_BLACK, FL_WHITE );
         break;
    case FL_DRAG:
         DBG_BUTTON_S( "FL_DRAG bei: " << Fl::event_x() << "," << Fl::event_y() )
         mouse_x_ = Fl::event_x();
         mouse_y_ = Fl::event_y();
         agv_->agvHandleMotion(mouse_x_, mouse_y_);
         redraw();
         break;
    case FL_KEYDOWN:
         DBG_BUTTON_S( "FL_KEYDOWN bei: " << Fl::event_x() << "," << Fl::event_y() )
         break;
    case FL_KEYUP:
         DBG_BUTTON_S( "FL_KEYUP bei: " << Fl::event_x() << "," << Fl::event_y() )
         break;
    case FL_ENTER:
    case FL_MOVE:
         DBG_BUTTON_S( "FL_MOVE bei: " << Fl::event_x() << "," << Fl::event_y() )
         mouse_x_ = Fl::event_x();
         mouse_y_ = Fl::event_y();
         if(visible() && !canMove()) {
           redraw();
         }
         text[0] = 0;
         break;
    case FL_MOUSEWHEEL:
         DBG_BUTTON_S( "FL_MOUSEWHEEL" << Fl::event_dx() << "," << Fl::event_dy() )
         if(mouse_button & FL_SHIFT)
         {
           fl_cursor( FL_CURSOR_MOVE, FL_BLACK, FL_WHITE );
         
           double clip_old = front_cut;
           front_cut -= Fl::event_dy()*0.01;
           if(front_cut - clip_old != 0 &&
             !canMove())
           redraw();
           fl_cursor( FL_CURSOR_DEFAULT, FL_BLACK, FL_WHITE );
         } else
         agv_->agvHandleButton( mouse_button,event, Fl::event_dx(),Fl::event_dy());
         redraw();
         break;
    case FL_LEAVE:
         DBG_BUTTON_S( dbgFltkEvent(event) )
         break;
    case FL_HIDE:
         hide();
         break;
    case FL_NO_EVENT:
    default:
         DBG_BUTTON_S( dbgFltkEvent(event) )
         DBG_ICCGL_ENDE
         return Fl_Gl_Window::handle(event);
  }
  if(mouse_button)
  {
    switch(Fl::event_state()) {
      case FL_BUTTON1: DBG_BUTTON_S("FL_BUTTON1") break;
      case FL_BUTTON2: DBG_BUTTON_S("FL_BUTTON2") break;
      case FL_BUTTON3: DBG_BUTTON_S("FL_BUTTON3") break;
      default: DBG_BUTTON_S("unknown event_state()") break;
    }
  }

  DBG_ICCGL_ENDE
  return end;
}

int
GL_View::keyEvents(int e)
{ DBG_MEM_START
  int found = 0;
  if(!icc_examin->laeuft())
    return found;

  Fl_Widget * wb = Fl::belowmouse();
  int is_visible = visible();
  if(is_visible)
  if( this == wb )
  {
    DBG_ICCGL_S("e = " << e << " " << Fl::event_key() )
    if(e == FL_SHORTCUT ||
       e == FL_KEYBOARD)
    {
      double clip_old = front_cut;
      if(Fl::event_key() == FL_Up) {
        front_cut += 0.01;
      } else if(Fl::event_key() == FL_Down) {
        front_cut -= 0.01;
      } else if(Fl::event_key() == FL_Home) {
        front_cut = 4.2;
      } else if(Fl::event_key() == FL_End) {
        front_cut = agv_->eyeDist();
      }
      if(front_cut - clip_old != 0 &&
         !canMove())
        redraw();

      int k = ((char*)Fl::event_text())[0];

      switch (k) {
      case '-':
        if(table_.size() && onion_skin != 0)
        {
          this->level -= this->level_step;
          if(this->level <= .0)
            this->level = .0001;
          refresh_();
          redraw();
        }
        else if(point_form >= MENU_dE1SPHERE && point_form <= MENU_dE4SPHERE)
        {
          if (point_form > MENU_dE1SPHERE) {
            --point_form;
          }
        }
        else if (point_form == MENU_DIFFERENCE_LINE)
          point_form = MENU_dE1SPHERE;
        else if(point_size > 1)
        {
          --point_size;
          refresh_();
          redraw();
        }
        DBG_PROG_V( Fl::event_key() <<" "<< point_size )
        found = 1;
        break;
      case '+': // 43
        if(table_.size() && onion_skin != 0)
        {
          this->level += this->level_step;
          if(this->level >= 1.0)
            this->level = .9999;
          refresh_();
          redraw();
        }
        else if (point_form >= MENU_dE1SPHERE && point_form <= MENU_dE4SPHERE)
        {
          if (point_form < MENU_dE4SPHERE) {
            ++point_form;
          }
        }
        else if (point_form == MENU_DIFFERENCE_LINE)
        {
          point_form = MENU_dE4SPHERE;
        }
        else if (point_size < 61)
        {
          ++point_size;
          refresh_();
          redraw();
        }
        DBG_PROG_V( Fl::event_key()  <<" "<< point_size <<" "<< point_form <<" "<< MENU_DIFFERENCE_LINE )
        found = 1;
        break;
      case '*':
        if(table_.size() && onion_skin != 0)
        {
          this->level_step *= 2;
          if(this->level_step > 0.999)
            this->level_step = 0.999;
        }
        found = 1;
        break;
      case '/':
        if(table_.size() && onion_skin != 0)
        {
          this->level_step /= 2;
          if(this->level_step < 0.0001)
            this->level_step = 0.0001;
        }
        found = 1;
        break;
      case '_':
        if(table_.size())
        {
          onion_skin = -onion_skin;
          refresh_();
          redraw();
        }
        found = 1;
        break;
      case '1':
      case '2':
      case '3':
      case '4':
      case '5':
      case '6':
      case '7':
      case '8':
      case '9':
      case '0':
        if(table_.size())
        {
          if(onion_skin<0)
            onion_skin = -(k - '0');
          else
            onion_skin = (k - '0');

          refresh_();
          redraw();
        }
        found = 1;
        break;
      default:
        dbgFltkEvents(e);
        DBG_MEM_V( Fl::event_key() )
      }
    }
  }

  DBG_MEM_ENDE
  return found;
}


void
GL_View::c_ ( Fl_Widget* w, void* data )
{ DBG_ICCGL_START

  intptr_t value = (intptr_t) data;
  DBG_PROG_V( value )

  GL_View *gl_obj = dynamic_cast<GL_View*>(w->parent());
  DBG_MEM_V( (intptr_t)gl_obj )
  DBG_MEM_V( (intptr_t)w->parent() )
  if(!w->parent())
    WARN_S("Could not find parents.")
  else
  if (gl_obj)
  {
    gl_obj->menuEvents((int)value);
  }
  else
    WARN_S("could not find a suitable program structure")

  DBG_ICCGL_ENDE
}



int
GL_View::backgroundColourToMenuEntry( float colour )
{
  int eintrag = MENU_LIGHT_GRAY;

  if(background_colour == 1.0)
    eintrag = MENU_WHITE;
  if(background_colour == 0.75)
    eintrag = MENU_LIGHT_GRAY;
  if(background_colour == 0.5)
    eintrag = MENU_GRAY_GRAY;
  if(background_colour == 0.25)
    eintrag = MENU_DARK_GRAY;
  if(background_colour == 0.0)
    eintrag = MENU_BLACK;

  return eintrag;
}

#undef drawTEXT
#undef COLOUR

