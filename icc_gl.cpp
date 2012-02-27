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

#ifdef DEBUG_
#define MARK(x) DBG_S( #x ) x
#else
#define MARK(x) x
#endif

#define Beleuchtung
//#define Lab_STERN 1

void zeichneKegel( GLdouble breite, GLdouble hoehe, GLint seiten );
void zeichneKegel( GLdouble breite, GLdouble hoehe, GLint seiten,
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


#define bNachX(b) ((b)*b_darstellungs_breite - b_darstellungs_breite/2.)
#define LNachY(L) ((L) - 0.5)
#define aNachZ(a) ((a)*a_darstellungs_breite - a_darstellungs_breite/2.)
#define LabNachXYZv(L,a,b) \
         (b*b_darstellungs_breite - b_darstellungs_breite/2.), \
         (L - 0.5), \
         (a*a_darstellungs_breite - a_darstellungs_breite/2.)
#define YNachL(Y) ((Y) + .5)
#define Znacha(Z) ((Z) / a_darstellungs_breite + .5)
#define Xnachb(X) ((X) / b_darstellungs_breite + .5)

const double GL_View::std_vorder_schnitt = 4.2;
#ifdef HAVE_FTGL
FTFont *font = NULL, *ortho_font = NULL;
#endif

// set material colours
#define Lab2GL(L,a,b,A) { double lab[] = {L,a,b}; float farbe[4]={0,0,0,A}; \
                      oyOptions_s * opts = icc_examin->options(); \
        double * rgb = icc_oyranos.wandelLabNachBildschirmFarben( \
             window()->x() + window()->w()/2, window()->y() + window()->h()/2, \
       (icc_oyranos.colourServerActive() & XCM_COLOR_SERVER_PROFILES)?edit_:0, \
                                 lab, 1, opts); \
                      for(int i = 0; i < 3; ++i) \
                        farbe[i] = rgb[i]; \
                      glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, farbe); \
                      glColor4fv(farbe); \
                      delete [] rgb; oyOptions_Release( &opts ); }
#define FARBE(r,g,b,a) {farbe[0] = (float)(r); farbe[1] = (float)(g); farbe[2] = (float)(b); \
                      farbe[3] = (float)(a);  \
                      glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, farbe); \
                      glColor4fv(farbe); }

// draw text
#ifdef HAVE_FTGL
#  define ZeichneText(Font, Zeiger) { \
   glLineWidth(strichmult); \
    if(blend) glDisable(GL_BLEND); \
      glTranslated(.0,0,0.01); \
        glScaled(0.002,0.002,0.002); \
          if(Font) Font->Render(Zeiger); \
        glScaled(500,500,500); \
      glTranslated(.0,0,-.01); \
    if(blend) glEnable(GL_BLEND); \
   glLineWidth(strichmult); }
#else
# define ZeichneText(Font, Zeiger)
#endif

#ifdef HAVE_FTGL
#define ZeichneOText(Font, scal, buffer) { \
                                   glScaled(scal,scal*w()/(double)h(),scal); \
                                     drawText(Font, buffer); \
                                   glScaled(1.0/scal,1.0/(scal*w()/(double)h()),1.0/scal); \
                                 }
#else
#define ZeichneOText(Font, scal, buffer) {}
#endif

#ifdef HAVE_FTGL
void drawText( FTFont * f, const char * in_txt )
{
  float strichmult = 1.0;
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

    ZeichneText( f, wchar );
    if(wchar)free(wchar);
    if(txt)free(txt);

  } else
    ZeichneText( f, in_txt );

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

  kanal = 0;
  schnitttiefe = 0.01;
  vorder_schnitt = std_vorder_schnitt;
  a_darstellungs_breite = 1.0;
  b_darstellungs_breite = 1.0;
  schalen = 5;
  punktform = MENU_dE1STERN;
  punktfarbe = 0;
  punktgroesse = 8;
  //punkt_zahl_alt = 0;
  hintergrundfarbe = 0.75;
  spektralband = 0;
  zeige_helfer = true;
  memset( text, 0, 128 );
  zeig_punkte_als_paare = false;
  zeig_punkte_als_messwerte = false;
  typ_ = -1;
  strichmult = 1.0;
  strich1 = 1;
  strich2 = 2;
  strich3 = 3;
  blend = false;
  smooth = false;
  waiting_ = 1;
  memset( t, 0, 128 );
  maus_x_ = 0;
  maus_y_ = 0;
  maus_x_alt = -1;
  maus_y_alt = -1;
  maus_steht = false;
  level = 0.5001;
  level_step = 0.1;
  valid_ = false;
  update_geometries_ = false;
  zeit_ = 0;
  text[0] = 0;
  colours_ = 0;
  epoint_ = 0;
  oyProfile_s * prof = oyProfile_FromStd( oyEDITING_XYZ, NULL );
  mouse_3D_hit = oyNamedColour_Create( NULL, NULL,0, prof, 0 );
  oyProfile_Release( &prof );
  window_geometry = NULL;
  edit_ = NULL;

  for (int i=1; i < 3 ; ++i) textfarbe[i] = 0.5;
  for (int i=1; i < 3 ; ++i) pfeilfarbe[i] = 0.5;

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
  if (gl_listen_[HELFER]) {
    DBG_PROG_S( "delete glListe " << gl_listen_[HELFER] )
    glDeleteLists (gl_listen_[HELFER],1);
    gl_listen_[HELFER] = 0;
  }
  if (gl_listen_[PUNKTE]) {
    DBG_PROG_S( "delete glListe " << gl_listen_[PUNKTE] )
    glDeleteLists (gl_listen_[PUNKTE],1);
    gl_listen_[PUNKTE] = 0;
  }
  if (gl_listen_[SPEKTRUM]) {
    DBG_PROG_S( "delete glListe " << gl_listen_[SPEKTRUM] )
    glDeleteLists (gl_listen_[SPEKTRUM],1);
    gl_listen_[SPEKTRUM] = 0;
  }
  if (gl_listen_[UMRISSE]) {
    DBG_PROG_S( "delete glListe " << gl_listen_[UMRISSE] )
    glDeleteLists (gl_listen_[UMRISSE],1);
    gl_listen_[UMRISSE] = 0;
  }
# ifdef HAVE_FTGL
  //if(font) delete font;
  //if(ortho_font) delete ortho_font;
# endif

  namedColoursRelease();
  oyProfile_Release( &edit_ );

  --ref_;

  doLocked_m( std::string loc_alt = setlocale(LC_NUMERIC, NULL);,NULL) //getenv("LANG");
  if(loc_alt.size()) {
    DBG_NUM_V( loc_alt )
  } else {
    DBG_NUM_S( "LANG variable not found" )
  }
  doLocked_m( setlocale(LC_NUMERIC,"C");,NULL);

  Fl_Preferences gl( Fl_Preferences::USER, "oyranos.org", "iccexamin");
  switch (typ_)
  {
    case 1:
    {
      Fl_Preferences gl_cube(gl, "gl_cube");
      gl_cube.set("spektralband", spektralband );
      gl_cube.set("hintergrundfarbe", hintergrundfarbe );
      gl_cube.set("zeige_helfer", zeige_helfer );
      gl_cube.set("schalen", schalen );

      DBG_PROG_V( spektralband )
    } break;
    case 2:
    {
      Fl_Preferences gl_gamut(gl, "gl_gamut");
      gl_gamut.set("spektralband", spektralband );
      gl_gamut.set("hintergrundfarbe", hintergrundfarbe );
      gl_gamut.set("zeige_helfer", zeige_helfer );
      gl_gamut.set("schalen", schalen );

      DBG_PROG_V( spektralband )
    } break;
  }

  if(loc_alt.size())
    doLocked_m( setlocale(LC_NUMERIC,loc_alt.c_str()) , NULL);


  DBG_PROG_ENDE
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

  tabelle_ = gl.tabelle_;
  nach_farb_namen_ = gl.nach_farb_namen_;
  von_farb_namen_ = gl.von_farb_namen_;
  channels_ = gl.channels_;
  colours_ = oyStructList_Copy( gl.colours_, NULL );
  epoint_ = oyNamedColour_Copy( gl.epoint_, 0 );
  mouse_3D_hit = oyNamedColour_Copy( gl.mouse_3D_hit, 0 );
  edit_ = oyProfile_Copy( gl.edit_, NULL );

  typ_ = gl.typ_;

  waiting_ = gl.waiting_;
  bsp = 0;

  kanal = gl.kanal;
  punktform = gl.punktform;
  punktfarbe = gl.punktfarbe;
  punktgroesse = gl.punktgroesse;
  hintergrundfarbe = gl.hintergrundfarbe;
  textfarbe[0] = gl.textfarbe[0];
  textfarbe[1] = gl.textfarbe[1];
  textfarbe[2] = gl.textfarbe[2];
  pfeilfarbe[0] = gl.pfeilfarbe[0];
  pfeilfarbe[1] = gl.pfeilfarbe[1];
  pfeilfarbe[2] = gl.pfeilfarbe[2];
  schatten = gl.schatten;
  strichmult = gl.strichmult;
  strich1 = gl.strich1;
  strich2 = gl.strich2;
  strich3 = gl.strich3;
  schalen = gl.schalen;

  seitenverhaeltnis = gl.seitenverhaeltnis;
  vorder_schnitt = gl.vorder_schnitt;
  schnitttiefe = gl.schnitttiefe;
  level = gl.level;
  level_step = gl.level_step;
  a_darstellungs_breite = gl.a_darstellungs_breite;
  b_darstellungs_breite = gl.b_darstellungs_breite;
  zeig_punkte_als_paare = gl.zeig_punkte_als_paare;
  zeig_punkte_als_messwerte = gl.zeig_punkte_als_messwerte;
  spektralband = gl.spektralband;
  zeige_helfer = gl.zeige_helfer;

  zeit_ = gl.zeit_;
  valid_ = gl.valid_;
  maus_x_ = gl.maus_x_;
  maus_y_ = gl.maus_y_;
  maus_x_alt = gl.maus_x_alt;
  maus_y_alt = gl.maus_y_alt;
  maus_steht = gl.maus_steht;

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
  typ_ = ty;

  DBG_PROG

  doLocked_m( std::string loc_alt = setlocale(LC_NUMERIC, NULL);,NULL) //getenv("LANG");
  if(loc_alt.size()) {
    DBG_NUM_V( loc_alt )
  } else {
    DBG_NUM_S( "LANG variable not found" )
  }
  doLocked_m( setlocale(LC_NUMERIC,"C");,NULL);

  Fl_Preferences gl( Fl_Preferences::USER, "oyranos.org", "iccexamin");

  switch(typ_)
  {
    case 1:
    {
      Fl_Preferences gl_cube(gl, "gl_cube");
      gl_cube.get("spektralband", spektralband, 0 );
      gl_cube.get("hintergrundfarbe", hintergrundfarbe, 0.75 );
      gl_cube.get("zeige_helfer", zeige_helfer, true );
      gl_cube.get("schalen", schalen, 5 );
    } break;
    case 2:
    {
      Fl_Preferences gl_gamut(gl, "gl_gamut");
      gl_gamut.get("spektralband", spektralband, 0 );
      gl_gamut.get("hintergrundfarbe", hintergrundfarbe, 0.75 );
      gl_gamut.get("zeige_helfer", zeige_helfer, true );
      gl_gamut.get("schalen", schalen, 5 );
    } break;
  }

  if(loc_alt.size())
    doLocked_m( setlocale(LC_NUMERIC,loc_alt.c_str()) , NULL);

  DBG_PROG_V( spektralband )

  resizable(0);

# define TEST_GL(modus) { \
    this->mode(modus); \
    if(this->can_do()) { \
      mod |= modus; \
      this->mode(mod); DBG_PROG_S( "can_do(): " << modus ); \
      DBG_PROG_S( "OpenGL understand: ja   " << #modus <<" "<< this->mode() ) \
    } else {  printf("can_do() false: %d\n", modus); \
      DBG_PROG_S( "OpenGL understand: nein " << #modus <<" "<< this->mode() ) \
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

  if (typ_ > 1)
  {
    DBG_PROG_S("gl window " << id_)
    a_darstellungs_breite = 2.55;
    b_darstellungs_breite = 2.55;
    agv_->distA (agv_->distA()
                           + a_darstellungs_breite/2.0 - 0.5);
    agv_->distB (agv_->distB()
                           + b_darstellungs_breite/2.0 - 0.5);
    agv_->eyeDist (agv_->dist()*2.0);
  }

  // initialise
  menueInit_(); DBG_PROG
  menueAufruf (hintergrundfarbeZuMenueeintrag(hintergrundfarbe)); // colour sheme
  int schalen_old = schalen;
  menueAufruf (MENU_FARBIG);     // CLUT colour sheme
  schalen = schalen_old;
  schatten = 0.1f;
  if (typ() == 1) menueAufruf (MENU_WUERFEL);

  maus_steht = false;

  DBG_PROG_ENDE
}

/** @brief localise the position */
void
GL_View::mausPunkt_( GLdouble &oX, GLdouble &oY, GLdouble &oZ,
                        GLdouble &X, GLdouble &Y, GLdouble &Z, int from_mouse )
{
  DBG_PROG_START  
  // localise position
  // how far is the next object in this direction, very laborious
  GLfloat zBuffer = 0;
  glReadPixels((GLint)maus_x_,(GLint)h()-maus_y_,1,1,GL_DEPTH_COMPONENT, GL_FLOAT, &zBuffer);
  GLdouble model_matrix[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
           projektions_matrix[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
  GLint bildschirm[4] = {0,0,0,0};
  glGetDoublev(GL_MODELVIEW_MATRIX, model_matrix);
  glGetDoublev(GL_PROJECTION_MATRIX, projektions_matrix);
  glGetIntegerv(GL_VIEWPORT, bildschirm);
  if(from_mouse)
    gluUnProject(maus_x_, h()-maus_y_, zBuffer,
                 model_matrix, projektions_matrix, bildschirm,
                 &oX, &oY, &oZ);


  DBG_PROG_V( "X: "<<oX<<" Y: "<<oY<<" Z: "<<oZ<<" "<<id_ )

  gluProject( oX, oY, oZ,
              model_matrix, projektions_matrix, bildschirm,
              &X,&Y,&Z);
  DBG_PROG_ENDE
}

static int zahl = 0;
void
GL_View::bewegenStatisch_ (void* gl_a)
{
  DBG_ICCGL_START

  GL_View *gl_ansicht = (GL_View*)gl_a;

  if (!gl_ansicht) {
      WARN_S( "no GL_View provided " << gl_ansicht->id_ )
      return;
  }

  // actualise UI
  gl_ansicht->waiting_ = 0;
  icc_examin_ns::wait( 0.0, true );
  gl_ansicht->waiting_ = 1;

  if(gl_ansicht->darfBewegen())
  {
    double zeichnen_schlaf = 0;  // no endless wait queues
    if(!icc_examin->frei())
    {
      zahl = 0;
      DBG_ICCGL_S( "redraw not allowed " << gl_ansicht->id_ )
    } else {
      double zeit = icc_examin_ns::zeitSekunden();

      if (zeit - gl_ansicht->zeit_ < 1./25.) {
        zeichnen_schlaf = 1./25. - gl_ansicht->zeit_;
      } else {
        gl_ansicht->redraw();
        zahl++;
      }

    }
    // short wait
    Fl::repeat_timeout( MIN(0.01,zeichnen_schlaf), bewegenStatisch_, gl_a );

    DBG_ICCGL_S( "Pause " << gl_ansicht->zeit_diff_<<" "<<zeichnen_schlaf <<" "<< " "<< gl_ansicht->id_ <<" "<<zahl<<" "<< gl_ansicht->darfBewegen())
  } 
  DBG_ICCGL_ENDE
}

bool GL_View::darfBewegen()        { return agv_ && agv_->can_move_; }
void GL_View::darfBewegen(int d)
{
  agv_->can_move_ = d?true:false; 
  if (d)
    Fl::add_timeout(0.04, bewegenStatisch_,this);

  DBG_ICCGL_S("Stop Bewegen "<<zahl<<" "<<d)
}

void
GL_View::bewegen (bool setze)
{
  DBG_ICCGL_START
  darfBewegen( setze );
  DBG_ICCGL_V( setze )
  if(!setze) {
    agv_->agvSwitchMoveMode (Agviewer::AGV_STOP);
    agv_->notify(ICCexamin::GL_STOP);
  }
  DBG_ICCGL_ENDE
}

int
GL_View::auffrischen_()
{
  DBG_PROG_START

  menueErneuern_();

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


/** recive of a drawing news */
void
GL_View::nachricht(icc_examin_ns::Model* model, int info)
{
  DBG_PROG_START
  DBG_PROG_V( info<<" "<<window()->visible()<<" "<<visible()<<" "<<shown()<<" "<<id_ )

  if( visible() && shown() && !agv_->parent->visible() )
    agv_->reparent(this);

  if( info == ICCexamin::GL_AUFFRISCHEN )
  {
    invalidate();
    auffrischen_();
    redraw();
  }

  if( info && visible() )
    Fl_Gl_Window::redraw();
  if(info == ICCexamin::GL_STOP)
  {
    darfBewegen( false );
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
  DBG_PROG_S( "entrance darfBewegen(): "
               << darfBewegen()<<" "<<id_<<" colour: "<<hintergrundfarbe )
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

  zeichnen();

  DBG_PROG_V( dreiecks_netze.size() )

  DBG_PROG_ENDE
}

Agviewer*
GL_View::getAgv( GL_View *ansicht, GL_View *referenz )
{ DBG_PROG_START
  Agviewer *agv = NULL;

  if(!agv && referenz)
  {
    agv = referenz->agv_;
    agv->reparent( ansicht );
  }

  if(!agv)
    agv = new Agviewer (ansicht);

  if(ansicht->agv_)
    ansicht->modelDel( ansicht->agv_ );

  ansicht->agv_ = agv;

  ansicht->modelAdd( ansicht->agv_ );

  DBG_ICCGL_ENDE
  return agv;
}

void
GL_View::GLinit_()
{ DBG_PROG_START
  GLfloat mat_ambuse[] = { 0.2f, 0.2f, 0.2f, 1.0f };
  GLfloat mat_specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };

# ifdef Beleuchtung
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
# ifndef Beleuchtung
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

  const char* font_name = "/usr/share/fonts/dejavu/DejaVuSans.ttf";

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
        font_name = (const char *) fname;
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
    font_name = f_n.c_str(); DBG_PROG_S( f_n )
  }
# endif
  if(!holeDateiModifikationsZeit(font_name)) {
    DBG_PROG_S( _("Could not open font in:") << font_name )
    font_name = "/usr/share/fonts/truetype/DejaVuSans.ttf";
    if(!holeDateiModifikationsZeit(font_name)) {
      DBG_PROG_S( _("Could not open font in:") << font_name )
#ifdef  WIN32
      font_name = "C:\\Windows\\Fonts\\arial.ttf";
#else
      font_name = "/Library/Fonts/Arial.ttf";
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
zeichneKegel( GLdouble breite, GLdouble hoehe, GLint seiten )
{ DBG_ICCGL_START
  GLdouble x, y,
           s = 2*M_PI/(GLdouble)seiten, // static variable
           hn = breite*tan(breite/2./hoehe); // normal hight
  // bottom
  glBegin(GL_TRIANGLE_FAN);
    glNormal3d( 0, 0, -1 );
    glVertex3d( 0, 0, 0 );
    for(int i = 0; i <= seiten; ++i)
      glVertex3d( cos(i*s)*breite, sin(i*s)*breite, 0 );
  glEnd();
  // cone
  glBegin(GL_TRIANGLE_STRIP);
    for(int i = 0; i <= seiten; ++i)
    {
      x = cos(i*s)*breite;
      y = sin(i*s)*breite;
      glNormal3d( x, y, hn );
      glVertex3d( x, y, 0 );
      glVertex3d( 0, 0, hoehe );
    }
  glEnd();

  DBG_ICCGL_ENDE
}

void
zeichneKegel( GLdouble breite, GLdouble hoehe, GLint seiten ,
              GLdouble x, GLdouble y, GLdouble z )
{ DBG_ICCGL_START
  GLdouble xk, yk,
           s = 2*M_PI/(GLdouble)seiten, // static variable
           hn = breite*tan(breite/2./hoehe); // hight of normal
  // bottom
  glBegin(GL_TRIANGLE_FAN);
    glNormal3d( 0, 0, -1 );
    glVertex3d( x, y, z );
    for(int i = 0; i <= seiten; ++i)
      glVertex3d( x+cos(i*s)*breite, y+sin(i*s)*breite, z );
  glEnd();
  // cone
  glBegin(GL_TRIANGLE_STRIP);
    for(int i = 0; i <= seiten; ++i)
    {
      xk = cos(i*s)*breite;
      yk = sin(i*s)*breite;
      glNormal3d( xk, yk, hn );
      glVertex3d( x+xk, y+yk, z+hoehe );
      glVertex3d( x, y, z );
    }
  glEnd();

  DBG_ICCGL_ENDE
}


void
GL_View::zeichneKoordinaten_()
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
        ZeichneText(font, text)
      glTranslated(0.3,0.1,0.05);
    glRotated (-90,0.0,0,1.0);

    Lab2GL(1,.5,.5,1)
    glTranslated(.1,0,0);
      Lab2GL(.9,0,-1.0,1)
      glRotated (90,0.0,1.0,.0);
        zeichneKegel(0.01, 0.025, 8);
      glRotated (-90,0.0,1.0,.0);
      Lab2GL(1,.5,.5,1)
      glTranslated(.02,0,0);
        ZeichneText(font, "X")
      glTranslated((-0.02),0,0);
    glTranslated((-0.1),0,0);

    glTranslated(.0,.1,0);
      glRotated (270,1.0,.0,.0);
        Lab2GL(1,0.0,1.0,1)
        zeichneKegel(0.01, 0.025, 8);
      glRotated (90,1.0,.0,.0);
      glRotated (90,0.0,.0,1.0);
        Lab2GL(1,.5,.5,1)
        ZeichneText(font, "Y")
      glRotated (270,0.0,.0,1.0);
    glTranslated(.0,(-0.1),0);

    glTranslated(0,0,.1);
      Lab2GL(0.9,0,-1.0,1)
      zeichneKegel(0.01, 0.025, 8);
      glRotated (90,0.0,.5,.0);
        glTranslated(-.1,0,0);
          Lab2GL(1,.5,.5,1)
          ZeichneText(font, "Z")
        glTranslated(.1,0,0);
      glRotated (270,0.0,.5,.0);
    glTranslated(0,0,-.1);
  DBG_ICCGL_ENDE
}

int
GL_View::erstelleGLListen_()
{ DBG_PROG_START

  if(!frei())
    return 1;

  MARK( frei(false); )

  garnieren_();

  tabelleAuffrischen();

  // actualise shodow
  static char aktive[64];
  static char grau[64];
  char aktualisiert = false;

  dreiecks_netze.frei(false);
  for(int i = 0; i < (int)dreiecks_netze.size(); ++i)
  {
    double netze_n = dreiecks_netze.size();
    double schattierung = .93 - .8/netze_n*i;
    schattierung = 1.0 - i/(double)netze_n;
    if(dreiecks_netze[i].schattierung < 0)
      dreiecks_netze[i].schattierung = schattierung;

    if( dreiecks_netze[i].active() != aktive[i] ||
        dreiecks_netze[i].grau != grau[i]  )
    {
      aktive[i] = dreiecks_netze[i].active();
      grau[i] = dreiecks_netze[i].grau;
      if(!aktualisiert)
      {
        //zeigeUmrisse_();
        aktualisiert = true;
      }
    }
  }
  dreiecks_netze.frei(true);

  MARK( frei(true); )
  updateNet_();
  MARK( frei(false); )

  dreiecks_netze.frei(false);
  if(dreiecks_netze.size())
    zeigeUmrisse_();
  dreiecks_netze.frei(true);
 
  zeigeSpektralband_();

  DBG_PROG_V( punktform <<" "<< MENU_dE1STERN )
  dreiecks_netze.frei(false);
  punkteAuffrischen();
  dreiecks_netze.frei(true);

  //background
  { double lab[] = {hintergrundfarbe,.5,.5};
                      oyOptions_s * opts = icc_examin->options();
        double * rgb = icc_oyranos.wandelLabNachBildschirmFarben(
             window()->x() + window()->w()/2, window()->y() + window()->h()/2,
       (icc_oyranos.colourServerActive() & XCM_COLOR_SERVER_PROFILES)?edit_:0,
                                 lab, 1, opts);
    glClearColor(rgb[0],rgb[1],rgb[2],1.0);
    delete [] rgb; oyOptions_Release( &opts );
  }

  MARK( frei(true); )

  DBG_PROG_ENDE
  return 0;
}

void
GL_View::textGarnieren_()
{
  DBG_PROG_START
  char text[256];
  char* ptr = 0;
  GLfloat ueber = 0.035f;

    glDisable(GL_TEXTURE_2D);
    glDisable(GL_LIGHTING);
    Lab2GL(textfarbe[0],textfarbe[1],textfarbe[2],1)

    // CIE*L - top
    glPushMatrix();
      glLoadIdentity();
      if (von_farb_namen_.size())
      {
        ptr = (char*) von_farb_namen_[0].c_str();
        sprintf (&text[0], "%s", ptr);
        glRasterPos3d (0, .5+ueber, 0);
        ZeichneOText(ortho_font, 1, text)
      }

    // CIE*a - right
      if (von_farb_namen_.size() > 1)
      {
        ptr = (char*) von_farb_namen_[1].c_str();
        sprintf (&text[0], "%s", ptr);
        if (von_farb_namen_.size() &&
            von_farb_namen_[1] == _("CIE *a"))
          glRasterPos3d (.0, -.5, a_darstellungs_breite/2.+ueber);
        else
          glRasterPos3d (.0, .0, a_darstellungs_breite/2.+ueber);
        ZeichneOText(ortho_font, 1, text)
      }

    // CIE*b - left
      if (von_farb_namen_.size() > 2)
      {
        ptr = (char*) von_farb_namen_[2].c_str();
        sprintf (&text[0], "%s", ptr);
        if (von_farb_namen_.size() &&
            von_farb_namen_[2] == _("CIE *b"))
          glRasterPos3d (b_darstellungs_breite/2.+ueber, -.5, .0);
        else
          glRasterPos3d (b_darstellungs_breite/2.+ueber, .0, .0);
        ZeichneOText(ortho_font, 1, text)
      }
    glPopMatrix();

  DBG_PROG_ENDE
}

void
GL_View::garnieren_()
{
  DBG_PROG_START

# define PFEILSPITZE zeichneKegel(0.02, 0.05, 16);

  DBG_PROG_V( id() )
  // arrow and text
  if (gl_listen_[HELFER]) {
    glDeleteLists (gl_listen_[HELFER], 1);
  }

  GL_View::gl_listen_[HELFER] = glGenLists(1);

  glNewList( gl_listen_[HELFER], GL_COMPILE); DBG_PROG_V( gl_listen_[HELFER] )
    glLineWidth(strich3*strichmult);

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
      Lab2GL(pfeilfarbe[0],pfeilfarbe[1],pfeilfarbe[2],1)
      glTranslatef(0,.5,0);
      glRotatef (270,1.0,0.0,.0);
      PFEILSPITZE
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
      if (von_farb_namen_.size() > 1 &&
          von_farb_namen_[1] == _("CIE *a"))
        glTranslatef(0,-.5,0);
      Lab2GL(pfeilfarbe[0],pfeilfarbe[1],pfeilfarbe[2],1)
      float schritt = .25, Schritt = 1., start, ende;
      start = (float)(- floor (a_darstellungs_breite/2./schritt) * schritt);
      ende  = (float)(  floor (a_darstellungs_breite/2./schritt) * schritt);
      // grid
      glDisable(GL_LIGHTING);
      for(float i = start; i <= ende; i+=schritt) {

        if(i/Schritt == floor(i/Schritt))
          glLineWidth(strich3*strichmult);
        else
          glLineWidth(strich1*strichmult);

        glBegin(GL_LINES);
          glVertex3d( i, 0,  a_darstellungs_breite/2.);
          glVertex3d( i, 0, -a_darstellungs_breite/2.);
        glEnd();
      }
      glEnable(GL_LIGHTING);

      glTranslated(0.0,0.0,a_darstellungs_breite/2.);
      glRotated (180,0.0,.5,.0);
      glTranslated(.0,0.0,a_darstellungs_breite);
      if (von_farb_namen_.size() > 1 &&
          von_farb_namen_[1] == _("CIE *a"))
      {
        Lab2GL(1.,-1.0,0.5,1)
        PFEILSPITZE
      }
      glTranslated(.0,0.0,-a_darstellungs_breite);
      glRotated (180,0.0,.5,.0);
      if (von_farb_namen_.size() > 1 &&
          von_farb_namen_[1] == _("CIE *a"))
        Lab2GL(1.,1.0,0.5,1)
      PFEILSPITZE
    glPopMatrix(); DBG_PROG

    // CIE*b - left
    glPushMatrix();
      if (von_farb_namen_.size() > 2 &&
          von_farb_namen_[2] == _("CIE *b"))
        glTranslated(0,-0.5,0);
      Lab2GL(pfeilfarbe[0],pfeilfarbe[1],pfeilfarbe[2],1)
      // grid
      glDisable(GL_LIGHTING);
      for(float i = start; i <= ende; i+=schritt) {

        if(i/Schritt == floor(i/Schritt))
          glLineWidth(strich3*strichmult);
        else
          glLineWidth(strich1*strichmult);

        glBegin(GL_LINES);
          glVertex3d( b_darstellungs_breite/2., 0, i);
          glVertex3d(-b_darstellungs_breite/2., 0, i);
        glEnd();
      }
      glEnable(GL_LIGHTING);
      glTranslated(b_darstellungs_breite/2.,0,0);
      if (von_farb_namen_.size() > 2 &&
          von_farb_namen_[2] == _("CIE *b"))
        Lab2GL(1.,0.5,1.0,1)
      glRotated (90,0.0,.5,.0);
      PFEILSPITZE
      glRotated (180,.0,.5,.0);
      glTranslated(.0,.0,b_darstellungs_breite);
      if (von_farb_namen_.size() > 2 &&
          von_farb_namen_[2] == _("CIE *b"))
      {
        Lab2GL(.6,0.5,-1.0,1)
        PFEILSPITZE
      }
    glPopMatrix();
    glLineWidth(strich1*strichmult);

  glEndList();

  DBG_PROG_ENDE
}

void
GL_View::tabelleAuffrischen()
{ DBG_PROG_START

  DBG_PROG_V( tabelle_.size() )
  // correct the channel selection
  if(tabelle_.size()) {
    if( (int)tabelle_[0][0][0].size() <= kanal) {
      kanal = (int)tabelle_[0][0][0].size()-1;
      DBG_PROG_S( "Kanalauswahl geaendert: " << kanal )
    }
  }

  // table
  if (gl_listen_[RASTER]) {
    //DBG_S( id_ << ": erase RASTER " << gl_listen_[RASTER], ICC_MSG_WARN );
    glDeleteLists ( gl_listen_[RASTER], 1);
    gl_listen_[RASTER] = 0;
  }

  if (tabelle_.size())
  {
    gl_listen_[RASTER] = glGenLists(1);
    glNewList( gl_listen_[RASTER], GL_COMPILE); DBG_PROG_V( gl_listen_[RASTER] )

      int n_L = (int)tabelle_.size(),
          n_a=(int)tabelle_[0].size(),
          n_b=(int)tabelle_[0][0].size();

      /* The small data cubes fit all into the whole table cubus.
       * The edge of the starting data cube lies on the edge of the table cubus.
       */
      double dim_x = 1.0/n_b;
      double dim_y = 1.0/n_L;
      double dim_z = 1.0/n_a;
      double start_x,start_y,start_z, x,y,z;
      double wert,
             A=.0,B=.0,C=.0,D=.0,E_=.0,F=.0,H=.0;

      /* The cubus for lines is completely filled from 0...1 .
       * The center of the starting data entry lies on the edge of the whole
       * cube.
       */
      if(schalen != 0)
      {
        dim_x = 1.0/(n_b-1);
        dim_y = 1.0/(n_L-1);
        dim_z = 1.0/(n_a-1);
      }

#if 0
      if(schalen)
        vorder_schnitt = 0.01;
      else
        vorder_schnitt = std_vorder_schnitt;
#endif
      schnitttiefe = HYP3(dim_x,dim_y,dim_z);
      DBG_NUM_V( schnitttiefe );
      start_x = start_y = start_z = x = y = z = 0.5; start_y = y = -0.5;
      glPushMatrix();

#     ifndef Beleuchtung_
      glDisable(GL_LIGHTING);
#     endif
      DBG_PROG_V( tabelle_.size() <<" "<< tabelle_[0].size() )

      float korr = 0.995f/2.0f;
      /* The cubus start point is shifted to let the data appear centred. */
      glTranslated( -0.5/0.995+dim_x/2,
                    -0.5/0.995+dim_y/2 - (schalen != 0 ? dim_y : 0),
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

            wert = tabelle_[L][a][b][kanal]; //DBG_PROG_V( L << a << b << kanal)

#           ifdef Beleuchtung_
            Lab2GL(wert, .5, .5,1)
            //glColor3f(wert, wert, wert);
#           else
            switch (punktfarbe) {
              case MENU_GRAU:   glColor4d( wert, wert, wert, 1.0); break;
              case MENU_FARBIG: glColor4d((wert*2),
                                           wert*2-1.0,
                                           1.0-wert, 1.0);         break;
              case MENU_KONTRASTREICH: wert = wert * 6;
                                while(wert > 1.0) { wert = wert - 1.0; }
                                if (schalen != 0 && wert < 0.80) wert = 0.0;
                                if (0/*schalen*/) {
                                  glDisable(GL_LIGHTING);
                                  Lab2GL(wert, .5, .5,1)
                                  glEnable(GL_LIGHTING);
                                } else glColor4d( wert, wert, wert, 1.0);
                                                                   break;
            }
#           endif

            if(schalen != 0)
            for(int n = 0; n < abs(schalen); ++n)
            {
              double l = 1./abs(schalen) * n + level/abs(schalen);

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

              A = tabelle_[L][a][b][kanal];
              if(b != n_b - 1)
              B = tabelle_[L][a][b+1][kanal];
              if(b != n_b - 1 && a != n_a - 1)
              C = tabelle_[L][a+1][b+1][kanal];
              if(a != n_a - 1)
              D = tabelle_[L][a+1][b][kanal];

              if(schalen > 0 && L != 0)
              {
              E_= tabelle_[L-1][a][b][kanal];
              if(b != n_b - 1)
              F = tabelle_[L-1][a][b+1][kanal];
              /*i if(b != n_b - 1 && a != n_a - 1)
              G = tabelle_[L-1][a+1][b+1][kanal]; */
              if(a != n_a - 1)
              H = tabelle_[L-1][a+1][b][kanal];
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
              if(schalen > 0 && L != 0)
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
            if (wert && 
                (schalen == 0 || geschaelt < 0))
            {
                glBegin(GL_TRIANGLE_FAN);
                  glVertex3d(dim_x*b+ dim_x*korr,
                             dim_y*L+ dim_y*korr,
                             dim_z*a+ dim_z*korr);
                  glVertex3d(dim_x*b+ dim_x*korr,
                             dim_y*L+-dim_y*korr,
                             dim_z*a+ dim_z*korr);
                  glVertex3d(dim_x*b+-dim_x*korr,
                             dim_y*L+-dim_y*korr,
                             dim_z*a+ dim_z*korr);
                  glVertex3d(dim_x*b+-dim_x*korr,
                             dim_y*L+ dim_y*korr,
                             dim_z*a+ dim_z*korr);
                  glVertex3d(dim_x*b+-dim_x*korr,
                             dim_y*L+ dim_y*korr,
                             dim_z*a+-dim_z*korr);
                  glVertex3d(dim_x*b+ dim_x*korr,
                             dim_y*L+ dim_y*korr,
                             dim_z*a+-dim_z*korr);
                  glVertex3d(dim_x*b+ dim_x*korr,
                             dim_y*L+-dim_y*korr,
                             dim_z*a+-dim_z*korr);
                  glVertex3d(dim_x*b+ dim_x*korr,
                             dim_y*L+-dim_y*korr,
                             dim_z*a+ dim_z*korr);
                glEnd();
                glBegin(GL_TRIANGLE_FAN);
                  glVertex3d(dim_x*b+-dim_x*korr,
                             dim_y*L+-dim_y*korr,
                             dim_z*a+-dim_z*korr);
                  glVertex3d(dim_x*b+ dim_x*korr,
                             dim_y*L+-dim_y*korr,
                             dim_z*a+-dim_z*korr);
                  glVertex3d(dim_x*b+ dim_x*korr,
                             dim_y*L+-dim_y*korr,
                             dim_z*a+ dim_z*korr);
                  glVertex3d(dim_x*b+-dim_x*korr,
                             dim_y*L+-dim_y*korr,
                             dim_z*a+ dim_z*korr);
                  glVertex3d(dim_x*b+-dim_x*korr,
                             dim_y*L+ dim_y*korr,
                             dim_z*a+ dim_z*korr);
                  glVertex3d(dim_x*b+-dim_x*korr,
                             dim_y*L+ dim_y*korr,
                             dim_z*a+-dim_z*korr);
                  glVertex3d(dim_x*b+ dim_x*korr,
                             dim_y*L+ dim_y*korr,
                             dim_z*a+-dim_z*korr);
                  glVertex3d(dim_x*b+ dim_x*korr,
                             dim_y*L+-dim_y*korr,
                             dim_z*a+-dim_z*korr);
                glEnd();
            }
          }
        }

        if(schalen != 0 && L == n_L -1 && geschaelt < 2 && geschaelt != -1)
        {
          L = 0;
          geschaelt = -1;
          dim_x = 1.0/n_b;
          dim_y = 1.0/n_L;
          dim_z = 1.0/n_a;
        }

      } DBG_PROG
      glPopMatrix();
#     ifndef Beleuchtung_
      glEnable(GL_LIGHTING);
#     endif
    glEndList();
  }
 
  DBG_PROG_ENDE
}

void
GL_View::netzeAuffrischen()
{
  DBG_PROG_START
  MARK( frei(false); )
    if( dreiecks_netze.size() )
    {
      DBG_ICCGL_V( dreiecks_netze.size() )
      DBG_ICCGL_V( dreiecks_netze[0].name )
      DBG_ICCGL_V( dreiecks_netze[0].punkte.size() )
      DBG_ICCGL_V( dreiecks_netze[0].indexe.size() )
      DBG_ICCGL_V( dreiecks_netze[0].undurchsicht )
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

       static double hintergrundfarbe_statisch = hintergrundfarbe;
       if(hintergrundfarbe != hintergrundfarbe_statisch)
         DBG_NUM_S("background colour changed "<<id_);
       GLfloat lmodel_ambient[] = {0.125f+hintergrundfarbe/8,
                                   0.125f+hintergrundfarbe/8,
                                   0.125f+hintergrundfarbe/8, 1.0f};
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
         zeichneKegel(0.02, 0.05, 16, lX, 0, lZ);


       if(icc_debug == 14) {
       strichmult = 3;
       DBG_ICCGL_S( "dist:"<<EyeDist<<" elevation:"<<EyeEl<<" azimuth:"<<EyeAz )
       glMatrixMode(GL_MODELVIEW);
       glLoadIdentity();
       glTranslated(X/1.2,Y/1.2,Z/1.2);
       DBG_ICCGL_S( "X:"<<X<<" Y:"<<Y<<" Z:"<<Z )
       zeichneKoordinaten_();
       glTranslated(-X/1.2,-Y/1.2,-Z/1.2);
       }

       std::multimap<double,DreiecksIndexe> indexe;
       double abstand;
       std::multimap<double,DreiecksIndexe>::const_iterator it;
       if(!bsp)
       for( it = netz.indexe.begin(); it != netz.indexe.end(); ++it )
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
       /*D*/ indexe.insert(index_p);
       }
      glPopMatrix();


  if (gl_listen_[RASTER]) {
    //DBG_S( id_ << ": erase RASTER " << gl_listen_[RASTER], ICC_MSG_WARN );
    glDeleteLists (gl_listen_[RASTER], 1);
    gl_listen_[RASTER] = 0;
  }

#     ifdef Beleuchtung
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
      glTranslated( -b_darstellungs_breite/2., -.5, -a_darstellungs_breite/2. );

      
      DBG_ICCGL_V( netz.indexe.size() <<" "<< netz.punkte.size() )


      glLineWidth(strich1*strichmult);
      int index[7];
      double len=1.0;

      // all material colours obtian here their transparency
      glColorMaterial( GL_FRONT_AND_BACK, GL_SPECULAR );
      glColorMaterial( GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE );
      glEnable( GL_COLOR_MATERIAL );
      //GLfloat farbe[4] = {  0.5,0.5,0.5,1.0 };
#define FARBEN(r,g,b,a) {/*farbe [0] = (r); farbe [1] = (g); farbe [2] = (b); \
                      farbe[3] = (a);  \
              glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, farbe); \
              glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, farbe);*/\
                      glColor4d(r,g,b,a); }

      if(bsp)
      {
        double dist = HYP3(X,Y,Z);
        icc_examin_ns::POINT pov = {Y + 0.5,
                                    Z + b_darstellungs_breite/2,
                                    X + b_darstellungs_breite/2.};
        if(icc_debug != 0)
        {
          zeichneKegel(0.02, 0.05, 16, X/dist, Y/dist, Z/dist);
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
        for( it = indexe.begin(); it != indexe.end(); ++it )
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
              FARBEN (   netz.punkte[index[l]].farbe[0],
                         netz.punkte[index[l]].farbe[1],
                         netz.punkte[index[l]].farbe[2],
                         netz.punkte[index[l]].farbe[3]);
              glVertex3d( netz.punkte[index[l]].koord[2],
                          netz.punkte[index[l]].koord[0],
                          netz.punkte[index[l]].koord[1] );
            }
          glEnd();
          //DBG_V( index <<" "<< len <<" "<< normale[0] );
          if(icc_debug != 0)
          {
          glLineWidth(strich1*strichmult);
          glBegin(GL_LINES);
            glVertex3d( netz.punkte[index[0]].koord[2],
                        netz.punkte[index[0]].koord[0],
                        netz.punkte[index[0]].koord[1] );
            glVertex3d( netz.punkte[index[0]].koord[2]+it->second.normale[2]/len*.1,
                        netz.punkte[index[0]].koord[0]+it->second.normale[0]/len*.1,
                        netz.punkte[index[0]].koord[1]+it->second.normale[1]/len*.1 );
          glEnd();
          }
          if(netz.kubus)
          {
            FARBEN ( 5.0,5.0,5.0,netz.punkte[index[0]].farbe[3] );
            glBegin(GL_LINES);
              glVertex3d( netz.punkte[index[1]].koord[2],
                          netz.punkte[index[1]].koord[0],
                          netz.punkte[index[1]].koord[1] );
              glVertex3d( netz.punkte[index[0]].koord[2],
                          netz.punkte[index[0]].koord[0],
                          netz.punkte[index[0]].koord[1] );
            glEnd();
          }
        }
      }

      glPopMatrix();

      glDisable (GL_BLEND);
      glDisable( GL_COLOR_MATERIAL );
      glDepthMask(GL_TRUE);
#     ifndef Beleuchtung
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
         d3[2] *= b_darstellungs_breite;
         d3[1] *= a_darstellungs_breite;
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
  switch (punktform)
  {
    case MENU_dE1KUGEL: rad = 0.5; break;
    case MENU_dE2KUGEL: rad = 1.0; break;
    case MENU_dE4KUGEL: rad = 2.0; break;
    default:            rad = ((double)punktgroesse)/4.0; break;
  }
  return rad;
}

void
GL_View::punkteAuffrischen()
{ DBG_PROG_START

  if (gl_listen_[PUNKTE]) {
    glDeleteLists (gl_listen_[PUNKTE], 1);
    gl_listen_[PUNKTE] = 0;
  }

  //coordinates  in CIE*b CIE*L CIE*a 
  if (oyStructList_Count( colours_ )) {
    DBG_PROG_V( oyStructList_Count( colours_ ) )

    gl_listen_[PUNKTE] = glGenLists(1);
    glNewList( gl_listen_[PUNKTE], GL_COMPILE); DBG_PROG_V( gl_listen_[PUNKTE] )
#     ifndef Beleuchtung_
      glDisable(GL_LIGHTING);
#     endif

      //glColor3f(0.9, 0.9, 0.9);
      glPushMatrix();
        // positioning
        glTranslated( -b_darstellungs_breite/2,-.5,-a_darstellungs_breite/2 );

        int x = this->window()->x() + this->window()->w()/2;
        int y = this->window()->y() + this->window()->h()/2;

        oyProfile_s * prof_disp = NULL;
        if(icc_oyranos.colourServerActive() & XCM_COLOR_SERVER_PROFILES)
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

          if(in && 
             (!in->pixel_data || in->pixel_data->type_ != oyOBJECT_ARRAY2D_S))
          {
            WARN_S( "unknown image backend found " );
          } else if(in)
            in_array = (oyArray2d_s*)in->pixel_data;
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
          int aktiv = 1;
          int grau = 0;
          oyNamedColour_s * c = NULL;
          double schattierung = 1;
          int has_mesh = 0;

          rgba[3] = 1;

          if(dreiecks_netze.size() > (unsigned)j)
          {
            aktiv = dreiecks_netze[j].active();
            grau = dreiecks_netze[j].grau;
            rgba[3] = dreiecks_netze[j].undurchsicht;
            schattierung = dreiecks_netze[j].schattierung;
            if(dreiecks_netze[j].punkte.size())
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

          if(!aktiv)
            continue;

          if(zeig_punkte_als_paare)
            n /= 2;

          if(zeig_punkte_als_paare)
            for (int i = 0; i < n; ++i)
            {
              glLineWidth(strich2*strichmult);
              // draw lines
              glBegin(GL_LINES);
                c = oyNamedColours_Get( colours, i );
                oyArray2d_DataSet( in_array,
                                   (void*) oyNamedColour_GetXYZConst( c ) );

                if(!zeig_punkte_als_messwerte)
                  glColor4d(.97, .97, .97, rgba[3] );
                else {
                  if(grau)
                    rgba[0]= rgba[1]= rgba[2] = schattierung;
                  else
                    oyConversion_RunPixels( cc_disp, 0 );
                  glColor4dv( rgba );
                }

                if(in_array)
                oyXYZ2Lab( (const double*) in_array->array2d[0], lab );
                iccPoint3d ( projection, lab, 0 );
                oyNamedColour_Release( &c );

                c = oyNamedColours_Get( colours, n + i );
                oyArray2d_DataSet( in_array,
                                   (void*) oyNamedColour_GetXYZConst( c ) );
                if(!zeig_punkte_als_messwerte)
                  glColor4d(1., .6, .6, 1.0 );
                else {
                  if(grau)
                    rgba[0]= rgba[1]= rgba[2] = schattierung;
                  else
                    oyConversion_RunPixels( cc_disp, 0 );
                  glColor4dv( rgba );
                }

                if(in_array)
                oyXYZ2Lab( (const double*) in_array->array2d[0], lab );
                iccPoint3d ( projection, lab, 0 );
                oyNamedColour_Release( &c );
              glEnd();
            }

          if(zeig_punkte_als_paare && !zeig_punkte_als_messwerte)
            n *= 2; 

          double rad = .02;
          int kugeln_zeichnen = false;
          switch (punktform)
          {
            case MENU_dE1STERN:
               glPointSize(punktgroesse);
               glColor4d(.97, .97, .97, rgba[3] );
               glBegin(GL_POINTS);
                 for (int i = 0; i < n; ++i)
                 {
                   c = oyNamedColours_Get( colours, i );
                   oyArray2d_DataSet( in_array,
                                      (void*) oyNamedColour_GetXYZConst( c ) );
                   if(grau)
                     rgba[0]= rgba[1]= rgba[2] = schattierung;
                   else
                     oyConversion_RunPixels( cc_disp, 0 );
                   glColor4dv( rgba );

                   if(in_array)
                   oyXYZ2Lab( (const double*) in_array->array2d[0], lab );
                   iccPoint3d ( projection, lab, 0 );
                   oyNamedColour_Release( &c );
                 }
               glEnd();
              break;
            case MENU_dE1KUGEL: rad = 0.005;kugeln_zeichnen = true;break;
            case MENU_dE2KUGEL: rad = 0.01; kugeln_zeichnen = true;break;
            case MENU_dE4KUGEL: rad = 0.02; kugeln_zeichnen = true;break;
                 break;
            case MENU_DIFFERENZ_LINIE: // they are drawn anyway
                 break;
          }

          if(kugeln_zeichnen)
          {
            for (int i = 0; i < n; ++i)
            {
                 glPushMatrix();
                   c = oyNamedColours_Get( colours, i );
                   oyArray2d_DataSet( in_array,
                                      (void*) oyNamedColour_GetXYZConst( c ) );
                   if(grau)
                     rgba[0]= rgba[1]= rgba[2] = schattierung;
                   else
                     oyConversion_RunPixels( cc_disp, 0 );
                   glColor4dv( rgba );

                   if(in_array)
                   oyXYZ2Lab( (const double*) in_array->array2d[0], lab );
                   iccPoint3d( projection, lab, rad );
                   oyNamedColour_Release( &c );
                 glPopMatrix();
            }
          }
          // shadow
          glColor4f( schatten, schatten, schatten, 1. );
          glPointSize((punktgroesse/2-1)>0?(punktgroesse/2-1):1);
          glBegin(GL_POINTS);
            if(zeig_punkte_als_messwerte)
                 for (int i = 0; i < n; ++i)
                 {
                   c = oyNamedColours_Get( colours, i );
                   oyArray2d_DataSet( in_array,
                                      (void*) oyNamedColour_GetXYZConst( c ) );
                   if(in_array)
                   oyXYZ2Lab( (const double*) in_array->array2d[0], lab );
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

      glPopMatrix();
#     ifndef Beleuchtung_
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
GL_View::zeigeUmrisse_()
{
  DBG_PROG_START

  if (gl_listen_[UMRISSE]) {
    DBG_PROG_S( "delete glListe " << gl_listen_[UMRISSE] )
    glDeleteLists (gl_listen_[UMRISSE],1);
    gl_listen_[UMRISSE] = 0;
  }

  oyOptions_s * opts = icc_examin->options();

  //if (spektralband == MENU_SPEKTRALBAND)
  for (unsigned int d=0; d < dreiecks_netze.size(); ++d)
  {
    double *RGB_Speicher = 0,
           *RGBSchatten_Speicher = 0,
           *Lab_Speicher = 0,
           *Lab_Speicher_schatten = 0;

    // conversion
    int n = (int)dreiecks_netze[d].umriss.size();
    DBG_PROG_V( n )
    if(!n) continue;

    Lab_Speicher = (double*) malloc (sizeof(double) * n*3);
    if(!Lab_Speicher)  WARN_S( "Lab_speicher Speicher nicht verfuegbar" )

    for ( int j = 0; j < n; ++j)
      for(int k = 0; k < 3 ; ++k)
        Lab_Speicher[j*3+k] = dreiecks_netze[d].umriss[j].koord[k];

    RGB_Speicher = icc_oyranos.wandelLabNachBildschirmFarben(
               window()->x() + window()->w()/2, window()->y() + window()->h()/2,
         (icc_oyranos.colourServerActive() & XCM_COLOR_SERVER_PROFILES)?edit_:0,
               Lab_Speicher, (size_t)n, opts);
    DBG_PROG_V( n )
    // create shadow
    Lab_Speicher_schatten = (double*) malloc (sizeof(double) * n*3);

    for (int i = 0; i < n; ++i) {
      Lab_Speicher_schatten[i*3+0] = dreiecks_netze[d].schattierung; //hintergrundfarbe*.40+.35;
      Lab_Speicher_schatten[i*3+1] = (Lab_Speicher[i*3+1]-.5)*.25+0.5;
      Lab_Speicher_schatten[i*3+2] = (Lab_Speicher[i*3+2]-.5)*.25+0.5;
    }

    RGBSchatten_Speicher = icc_oyranos.wandelLabNachBildschirmFarben(
               window()->x() + window()->w()/2, window()->y() + window()->h()/2,
         (icc_oyranos.colourServerActive() & XCM_COLOR_SERVER_PROFILES)?edit_:0,
                      Lab_Speicher_schatten, n, opts);
    if(!RGB_Speicher)  WARN_S( "RGB_speicher result is not available" )
    if(!RGBSchatten_Speicher)  WARN_S( "RGB_speicher result is not available" )

    for ( int j = 0; j < n; ++j)
      for(int k = 0; k < 3 ; ++k)
        dreiecks_netze[d].umriss[j].farbe[k] = RGBSchatten_Speicher[j*3+k];

    if (Lab_Speicher) free (Lab_Speicher);
    if (Lab_Speicher_schatten) free (Lab_Speicher_schatten);
    if (RGB_Speicher) delete [] RGB_Speicher;
    if (RGBSchatten_Speicher) delete [] RGBSchatten_Speicher;
  }
  oyOptions_Release( &opts );

    GLfloat farbe[] =   { pfeilfarbe[0],pfeilfarbe[1],pfeilfarbe[2], 1.0 };

  gl_listen_[UMRISSE] = glGenLists(1);
  glNewList( gl_listen_[UMRISSE], GL_COMPILE );
  DBG_PROG_V( gl_listen_[UMRISSE] ) 

    glDisable (GL_LIGHTING);
    glDisable (GL_ALPHA_TEST_FUNC);
    if(!smooth) {
      glDisable(GL_BLEND);
      glDisable(GL_LINE_SMOOTH);
    } else {
      glEnable(GL_BLEND);
      glEnable(GL_LINE_SMOOTH);
    }

    for (unsigned int i=0; i < dreiecks_netze.size(); ++i)
    {
      if(dreiecks_netze[i].umriss.size() &&
         dreiecks_netze[i].active() &&
         dreiecks_netze[i].undurchsicht)
      {
        //Farbband
        if(icc_debug)
        {
        glLineWidth(strich3*strichmult);
        glColor4f(1., 1.0, 1.0, 1.0);
        glBegin(GL_LINE_STRIP);
        for (int z=0 ; z < (int)dreiecks_netze[i].umriss.size(); z++) {
          if(!dreiecks_netze[i].grau) {
            Lab2GL(dreiecks_netze[i].umriss[z].farbe[0],
                  dreiecks_netze[i].umriss[z].farbe[1],
                  dreiecks_netze[i].umriss[z].farbe[2],1)
          }

          glVertex3d( 
                  (dreiecks_netze[i].umriss[z].koord[2] * b_darstellungs_breite
                    - b_darstellungs_breite/2.),
                  (dreiecks_netze[i].umriss[z].koord[0] - 0.5),
                  (dreiecks_netze[i].umriss[z].koord[1] * a_darstellungs_breite
                    - a_darstellungs_breite/2.)
          );
        }
        glEnd();
        }
        // colour line shadow
        int n = (int)dreiecks_netze[i].umriss.size();
        DBG_PROG_V( n )
        glLineWidth(strich2*strichmult);

        Lab2GL (          dreiecks_netze[i].schattierung, .5, .5, 1);

        glBegin(GL_LINE_STRIP);
        for (int z=0 ; z < n; z++)
        {
          if(!dreiecks_netze[i].grau) {
            FARBE(dreiecks_netze[i].umriss[z].farbe[0],
                  dreiecks_netze[i].umriss[z].farbe[1],
                  dreiecks_netze[i].umriss[z].farbe[2],1)
          }
          glVertex3d(dreiecks_netze[i].umriss[z].koord[2]*b_darstellungs_breite
                      - b_darstellungs_breite/2.,
                     -0.5,
                     dreiecks_netze[i].umriss[z].koord[1]*a_darstellungs_breite
                      - a_darstellungs_breite/2.);
        }
        glEnd();
      }
    }

  glEndList();
  DBG_PROG_ENDE
}

void
GL_View::zeigeSpektralband_()
{
  DBG_PROG_START

  if (gl_listen_[SPEKTRUM])
    glDeleteLists (gl_listen_[SPEKTRUM], 1);
  gl_listen_[SPEKTRUM] = 0;
  if (spektralband == MENU_SPEKTRALBAND)
  {
    double *RGB_Speicher = 0,
           *RGBSchatten_Speicher = 0,
           *XYZ_Speicher = 0,
           *Lab_Speicher = 0,
           *Lab_Speicher_schatten = 0;
    int n_punkte = 471;//341; // 700 nm
    oyOptions_s * opts = icc_examin->options();

    if(typ_ > 1)
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
      Lab_Speicher = new double [n_punkte*3];
      Lab_Speicher_schatten = new double [n_punkte*3];
      if(!Lab_Speicher)  WARN_S( "Lab_speicher Speicher not available" )

      XYZtoLab (XYZ_Speicher, Lab_Speicher, n_punkte);

    } else
    {
      //colour circle
      n_punkte = 90;
      Lab_Speicher = new double [n_punkte*3];
      Lab_Speicher_schatten = new double [n_punkte*3];
      if(!Lab_Speicher)  WARN_S( "Lab_speicher Speicher not available" )

      for (int i = 0; i < n_punkte; ++i)
      {
          Lab_Speicher[i*3+0] = (double)0.666;
          Lab_Speicher[i*3+1] = (double)cos(i*2*M_PI/(n_punkte-1))*.714 + .5;
          Lab_Speicher[i*3+2] = (double)sin(i*2*M_PI/(n_punkte-1))*.714 + .5;
      }
    }

    RGB_Speicher = icc_oyranos.wandelLabNachBildschirmFarben(
               window()->x() + window()->w()/2, window()->y() + window()->h()/2,
         (icc_oyranos.colourServerActive() & XCM_COLOR_SERVER_PROFILES)?edit_:0,
               Lab_Speicher, (size_t)n_punkte, opts);

    if(typ_ == 1)
      for (int i = 0; i < n_punkte; ++i)
        Lab_Speicher[i*3+0] = (double).0;


    DBG_PROG_V( n_punkte )
    // generate shadows
    for (int i = 0; i < n_punkte*2; ++i)
      Lab_Speicher_schatten[i] = Lab_Speicher[i];
    for (int i = 0; i < n_punkte; ++i) {
      Lab_Speicher_schatten[i*3] = hintergrundfarbe*.40+.35;
      Lab_Speicher_schatten[i*3+1] = (Lab_Speicher[i*3+1]-.5)*.25+0.5;
      Lab_Speicher_schatten[i*3+2] = (Lab_Speicher[i*3+2]-.5)*.25+0.5;
    }

    RGBSchatten_Speicher = icc_oyranos.wandelLabNachBildschirmFarben(
               window()->x() + window()->w()/2, window()->y() + window()->h()/2,
         (icc_oyranos.colourServerActive() & XCM_COLOR_SERVER_PROFILES)?edit_:0,
               Lab_Speicher_schatten, n_punkte, opts);
    if(!RGB_Speicher || !RGBSchatten_Speicher) 
    {
      WARN_S( "RGB_speicher result not available" )
      DBG_PROG_ENDE
      return;
    }

    GLfloat farbe[] =   { pfeilfarbe[0],pfeilfarbe[1],pfeilfarbe[2], 1.0 };

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

      glLineWidth(strich3*strichmult);
      glColor4f(0.5, 1.0, 1.0, 1.0);
      glBegin(GL_LINE_STRIP);
        for (int i=0 ; i <= (n_punkte - 1); i++) {
          DBG_ICCGL2_S( i<<" "<<Lab_Speicher[i*3]<<"|"<<Lab_Speicher[i*3+1]<<"|"<<Lab_Speicher[i*3+2] )
          DBG_ICCGL2_S( i<<" "<<RGB_Speicher[i*3]<<"|"<<RGB_Speicher[i*3+1]<<"|"<<RGB_Speicher[i*3+2] )
          FARBE(RGB_Speicher[i*3],RGB_Speicher[i*3+1],RGB_Speicher[i*3+2],1);
          glVertex3d( 
         (Lab_Speicher[i*3+2]*b_darstellungs_breite - b_darstellungs_breite/2.),
         (Lab_Speicher[i*3+0] - 0.5),
         (Lab_Speicher[i*3+1]*a_darstellungs_breite - a_darstellungs_breite/2.)
          );
        }
      glEnd();
      // shadow
      if(typ_ > 1)
      {
      glLineWidth(strich2*strichmult);
      glBegin(GL_LINE_STRIP);
      //#define S * .25 + textfarbe[0] - schatten
        for (int i=0 ; i <= (n_punkte - 1); i++) {
          FARBE(RGBSchatten_Speicher[i*3],RGBSchatten_Speicher[i*3+1],RGBSchatten_Speicher[i*3+2],1)
          glVertex3d( 
         (Lab_Speicher[i*3+2]*b_darstellungs_breite - b_darstellungs_breite/2.),
         (- 0.5),
         (Lab_Speicher[i*3+1]*a_darstellungs_breite - a_darstellungs_breite/2.)
          );
        }
      glEnd();
      }

    glEndList();

    if (XYZ_Speicher) delete [] XYZ_Speicher;
    if (RGB_Speicher) delete [] RGB_Speicher;
    if (RGBSchatten_Speicher) delete [] RGBSchatten_Speicher;
    if (Lab_Speicher) delete [] Lab_Speicher;
    if (Lab_Speicher_schatten) delete [] Lab_Speicher_schatten;
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
GL_View::menueErneuern_()
{ DBG_PROG_START
  MARK( frei(false); )

  // erase
  menue_schnitt_->clear();
  menue_hintergrund_->clear();
  menue_form_->clear();
  menue_->clear();
  menue_button_->clear();
  DBG_PROG

  // ->Darstellung
  menue_schnitt_->add("text_L", 0,c_, (void*)Agviewer::ICCFLY_L, 0);
  menue_schnitt_->add("text_a", 0,c_, (void*)Agviewer::ICCFLY_a, 0);
  menue_schnitt_->add("text_b", 0,c_, (void*)Agviewer::ICCFLY_b, 0);
  menue_schnitt_->add(_("Slice"), 0,c_, (void*)Agviewer::FLYING, 0);
  menue_schnitt_->add(_("Rotate around slice"),0,c_,(void*)Agviewer::ICCPOLAR, 0);
  DBG_PROG

  if (von_farb_namen_.size())
  {
    static char text_L[64];
    static char text_a[64];
    static char text_b[64];
    static char text_S[64];
    sprintf (text_L, "%s %s", von_farb_namen_[0].c_str(), _("Slice"));
    menue_schnitt_->replace( 0, text_L);
    if(von_farb_namen_.size() > 1)
      sprintf (text_a, "%s %s", von_farb_namen_[1].c_str(), _("Slice"));
    else
      sprintf (text_a, "%s", _("Slice"));
    menue_schnitt_->replace( 1, text_a);
    if(von_farb_namen_.size() > 2)
      sprintf (text_b, "%s %s", von_farb_namen_[2].c_str(), _("Slice"));
    else
      sprintf (text_a, "%s", _("Slice"));
    menue_schnitt_->replace( 2, text_b);
    sprintf (text_S, "%s %s %s", _("Rotate around"),von_farb_namen_[0].c_str(), _("axis"));
    menue_schnitt_->replace( 4, text_S);
  }
  DBG_PROG

  menue_->add (_("Slice plane"),0,0, cpMenueButton(menue_schnitt_),FL_SUBMENU_POINTER);

  // ->Darstellung->background colour
  menue_hintergrund_->add(_("White"), 0,c_, (void*)MENU_WEISS, 0);
  menue_hintergrund_->add(_("Light gray"), 0,c_, (void*)MENU_HELLGRAU, 0);
  menue_hintergrund_->add(_("Gray"), 0,c_, (void*)MENU_GRAUGRAU, 0);
  menue_hintergrund_->add(_("Dark gray"), 0,c_, (void*)MENU_DUNKELGRAU, 0);
  menue_hintergrund_->add(_("Black"), 0,c_, (void*)MENU_SCHWARZ, 0);
  DBG_PROG

  menue_form_->add (_("Background colour"),0,c_,cpMenueButton(menue_hintergrund_),FL_SUBMENU_POINTER);

  // ->Darstellung
  if(typ_ == 1)
  {
    menue_form_->add( _("gray"), 0,c_, (void*)MENU_GRAU, 0 );
    menue_form_->add( _("coloured"), 0,c_, (void*)MENU_FARBIG, 0 );
    menue_form_->add( _("high contrast"), 0,c_, (void*)MENU_KONTRASTREICH, 0 );
    // Shells alike displaying, alternatves are "Membranes" or "Shells"
    menue_form_->add( _("Onion skins"), 0,c_, (void*)MENU_SCHALEN, 0 );
    menue_form_->add( _("Colour line"), 0,c_, (void*)MENU_SPEKTRALBAND, 0 );
  } else {
    // spheres with their radius symbolise measurement colours
    if(!zeig_punkte_als_messwerte)
    {
      menue_form_->add( _("Sphere 1dE"), 0,c_, (void*)MENU_dE1KUGEL, 0 );
      menue_form_->add( _("Sphere 2dE"), 0,c_, (void*)MENU_dE2KUGEL, 0 );
      menue_form_->add( _("Sphere 4dE"), 0,c_, (void*)MENU_dE4KUGEL, 0 );
    } else {
#   ifdef Lab_STERN
      menue_form_->add( _("Star"), 0,c_, (void*)MENU_dE1STERN, 0 );
#   else
      // points are reserved for image colours
      menue_form_->add( _("Point"), 0,c_, (void*)MENU_dE1STERN, 0 );
#   endif
    }
    menue_form_->add( _("without markers"), 0,c_, (void*)MENU_DIFFERENZ_LINIE, 0);
    menue_form_->add( _("Spektral line"), 0,c_, (void*)MENU_SPEKTRALBAND, 0 );
  }
  DBG_PROG
  menue_form_->add( _("Texts"), 0,c_, (void*)MENU_HELFER, 0 );
  DBG_PROG_V( menue_form_->size() )
  menue_form_->replace( menue_form_->size()-2, _("Texts/Arrows on/off"));

  menue_->add (_("Illustration"),0,c_,cpMenueButton(menue_form_),FL_SUBMENU_POINTER);

  DBG_PROG_V( menue_->size() )


  // -> (main menue)
  for (int i = 0; i < (int)nach_farb_namen_.size(); i++) {
    char* p = (char*) nach_farb_namen_[i].c_str();
    menue_->add( p, 0,c_, (void*)((intptr_t)(MENU_MAX + i)), 0 );
    DBG_PROG_V( MENU_MAX + i <<" "<< nach_farb_namen_[i] )
  }

  // TODO: -> upper menue
  menue_button_->copy(menue_->menu());
  menue_button_->callback(c_);

  //glStatus(_("left-/middle-/right mouse button -> rotate/cut/menu"), typ_);

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
  ms[1] = menue_schnitt_ = new Fl_Menu_Button(0,0,w(),h(),""); menue_schnitt_->hide();
  ms[2] = menue_hintergrund_ = new Fl_Menu_Button(0,0,w(),h(),""); menue_hintergrund_->hide();
  ms[3] = menue_form_ = new Fl_Menu_Button(0,0,w(),h(),""); menue_form_->hide();

  for (int i = 0; i < 4; ++i)
    ms[i]->add("dummy",0,0,0,0);

  //menueErneuern_();

  this->end();
  MARK( frei(true); )
  DBG_PROG_ENDE
}


/*inline*/ void
GL_View::setzePerspektive()
{ //DBG_ICCGL_START
    // camera viewing angle
    if (agv_->thin)
      gluPerspective(15, seitenverhaeltnis,
                     vorder_schnitt,
                     vorder_schnitt + schnitttiefe);
    else
      gluPerspective(15, seitenverhaeltnis,
                     vorder_schnitt, 50);
                  // ^-- near cut plane
  //DBG_ICCGL_ENDE
}

void
GL_View::fensterForm( )
{ DBG_PROG_START
  if(visible()) {
    glViewport(0,0,w(),h());
    DBG_PROG_V( x()<<" "<< y()<<" "<<w()<<" "<<h())
    seitenverhaeltnis = (GLdouble)w()/(GLdouble)h();
    if(icc_oyranos.colourServerActive() & XCM_COLOR_SERVER_PROFILES &&
       typ_ != 1)
      icc_oyranos.colourServerRegionSet( this, edit_, window_geometry, 0 );
    oyRectangle_SetGeo( window_geometry, x(), y(), w(), h() );
  }
  DBG_PROG_ENDE
}

void
GL_View::zeichnen()
{
  DBG_ICCGL_START
# if 0
  int thread = wandelThreadId(iccThreadSelf());
  if(thread != THREAD_HAUPT) {
    WARN_S( "falscher Thread: " << dbgThreadId(iccThreadSelf()) );
    DBG_PROG_ENDE
  }
# endif

  if(!valid_)
  {
    // complete initialisation
    gl_font( FL_HELVETICA, 10 );
    auffrischen_();
    GLinit_();  DBG_PROG
    fensterForm();
  }

  if(update_geometries_)
  {
    int err = erstelleGLListen_();
    if(err)
      return;

    update_geometries_ = false;
    valid_ = true;
  }

  if(!frei()) return;
  MARK( frei(false); )

  zeit_ = icc_examin_ns::zeitSekunden();


  int scal = 1;
  GLfloat farbe[] =   { textfarbe[0],textfarbe[1],textfarbe[2], 1.0 };
  GLdouble oX=.0,oY=.0,oZ=.0;
  GLdouble X=.0,Y=.0,Z=.0;
  std::string kanalname;

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

      GL_View::setzePerspektive();

      /* so this replaces gluLookAt or equiv */
      agv_->agvViewTransform();

      glMatrixMode(GL_MODELVIEW);
      glLoadIdentity();

      glCallList( gl_listen_[SPEKTRUM] ); DBG_ICCGL_V( gl_listen_[SPEKTRUM] )
      glCallList( gl_listen_[UMRISSE] ); DBG_ICCGL_V( gl_listen_[UMRISSE] )
      if (zeige_helfer) {
        glCallList( gl_listen_[HELFER] ); DBG_ICCGL_V( gl_listen_[HELFER] )
      }
      glCallList( gl_listen_[RASTER] ); DBG_ICCGL_V( gl_listen_[RASTER] )
      //DBG_S( id_ << ": " << gl_listen_[RASTER], ICC_MSG_WARN)
      if(punktform == MENU_dE1STERN)
        glCallList( gl_listen_[PUNKTE] );
      glCallList( gl_listen_[PUNKTE] ); DBG_ICCGL_V( gl_listen_[PUNKTE] )

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
        oY = LNachY( lab.L );
        oZ = aNachZ( lab.a );
        oX = bNachX( lab.b );
        mausPunkt_( oX, oY, oZ, X, Y, Z, 0 );
      } else
      {
        text[0] = 0;
        mausPunkt_( oX, oY, oZ, X, Y, Z, 1 );

        GLfloat grenze = 3.2;

        if(von_farb_namen_.size() &&
                       -grenze < oY && oY < grenze &&
                       -grenze < oX && oX < grenze &&
                       -grenze < oZ && oZ < grenze)
        {
          double d[3];
          d[0] = YNachL(oY);
          d[1] = Znacha(oZ);
          d[2] = Xnachb(oX);
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
            oY = LNachY( lab.L );
            oZ = aNachZ( lab.a );
            oX = bNachX( lab.b );
            mausPunkt_( oX, oY, oZ, X, Y, Z, 0 );
            const char * temp = oyNamedColour_GetName( epoint_, oyNAME_NICK, 0 );
            if(temp)
              sprintf( text, "%s", temp );
            else
              text[0] = 0;
          } 
          if(!strlen(text))
          {
            double d[3];
            d[0] = YNachL(oY);
            d[1] = Znacha(oZ);
            d[2] = Xnachb(oX);
            LabToCIELab( d, d, 1 );
            text[0] = 0;
            for(int i = 0; i < (int)von_farb_namen_.size(); ++i)
              if(i < 3)
                sprintf( &text[strlen(text)], "%s:%.02f ",
                               von_farb_namen_[i].c_str(), d[i] );
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
          typ() != 1 )
      {
        rgb_ = rgb = icc_oyranos.wandelLabNachBildschirmFarben( 
               window()->x() + window()->w()/2, window()->y() + window()->h()/2,
         (icc_oyranos.colourServerActive() & XCM_COLOR_SERVER_PROFILES)?edit_:0,
                                 l, 1, opts);

        icc_examin->statusFarbe(l[0],l[1],l[2]);
          DBG_PROG_V( l[0]<<" "<<l[1]<<" "<<l[2] )
        if(rgb)
        {
#         ifndef Beleuchtung_
          glDisable(GL_LIGHTING);
#         endif
          glPushMatrix();
            glLineWidth(strich3*strichmult);
            FARBE(rgb[0], rgb[1], rgb[2],1)
            glBegin(GL_LINES);
              glVertex3f( oX, oY, oZ );
              glVertex3f( oX, -0.5, oZ );
            glEnd();
          glPopMatrix();
            DBG_PROG_V( rgb[0] <<" "<< rgb[1] <<" "<< rgb[2] )
#         ifndef Beleuchtung_
          glEnable(GL_LIGHTING);
#         endif
        }
        glLineWidth(strich1*strichmult);
      }


      if(dreiecks_netze.size())
      {
        MARK( frei(true); )
        dreiecks_netze.frei(false);

        int some_active = 0;
        for(size_t i = 0; i < dreiecks_netze.size(); ++i)
          if(dreiecks_netze[i].active())
            some_active = 1;

        if(some_active)
          netzeAuffrischen();

        dreiecks_netze.frei(true);
        MARK( frei(false); )
      }

    glPopMatrix();
    // End of drawing

    // Text
    Lab2GL(textfarbe[0],textfarbe[1],textfarbe[2],1)

    GLfloat lmodel_ambient[] = {hintergrundfarbe,
                                hintergrundfarbe,
                                hintergrundfarbe, 1.0};
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


         if (zeige_helfer)
           textGarnieren_();

         // can go away from drawing function
         if(strlen(text))
         {
            DBG_ICCGL_V( X<<" "<<Y<<" "<<Z )
            DBG_ICCGL_V( oX<<" "<<oY<<" "<<oZ )

            float f = 0.0f;
            if(hintergrundfarbe < 0.6)
              f = 1.f;
            Lab2GL(f,.5,.5,1)

            // text above scene
            glLoadIdentity();
            glMatrixMode(GL_PROJECTION);

            glLoadIdentity();
            glOrtho(0,w(),0,h(),-10.0,10.0);

            glRasterPos3d (X, Y, 9.999);
                      
            if(typ() == 1)
            {
              DBG_PROG_V( oY<<" "<<oZ<<" "<<oX )
                        /*if(-0.505 <= oY && oY <= 0.505 &&
                           -0.505 <= oZ && oZ <= 0.505 &&
                           -0.505 <= oX && oX <= 0.505)*/
              if(tabelle_.size())
              {
                 int L = (int)((oY+0.5)*tabelle_.size());
                 if(0 <= L && L < (int)tabelle_.size()) {
                 int a = (int)((oZ+0.5)*tabelle_[L].size());
                 if(0 <= a && a < (int)tabelle_[L].size()) {
                 int b = (int)((oX+0.5)*tabelle_[L][a].size());
                 if(0 <= b && b < (int)tabelle_[L][a].size()) {
# ifdef DEBUG
                   double wert = tabelle_[L][a][b][kanal];
# endif
                   DBG_PROG_V( L<<" "<<a<<" "<<b<<" "<<wert )
                   DBG_PROG_V( tabelle_.size()<<" "<<tabelle_[L].size()<<" "<<
                               tabelle_[L][a].size()<<" "<<kanal )
                   // "Pos" is a abreviation to position and will be visible in the small statusline with position coordinates
                   sprintf( text,"%s[%d][%d][%d]", _("Pos"),
                            (int)((oY+0.5)*tabelle_.size()),
                            (int)((oZ+0.5)*tabelle_[L].size()),
                            (int)((oX+0.5)*tabelle_[L][a].size()));
                   for (int i = 3 ; i < (int)channels_.size(); ++i) {
                     sprintf(&text[strlen(text)], "%d|", channels_[i]);
                   }
                   sprintf(&text[strlen(text)], " ");
                   for (int i = 0 ; i < (int)tabelle_[L][a][b].size(); ++i) {
                     if(i == kanal) sprintf(&text[strlen(text)], "[");
                     sprintf(&text[strlen(text)], "%.5f", tabelle_[L][a][b][i]);
                     if(i == kanal) sprintf(&text[strlen(text)], "]");
                     if (i != (int)tabelle_[L][a][b].size()-1)
                       sprintf(&text[strlen(text)], " ");
                   }
                   glStatus( text, typ_ );
                   ZeichneOText (ortho_font, scal, text)
                 }}}
               }
            } else
            {
              if(!epoint_)
                glStatus( text, typ_ );
              ZeichneOText (ortho_font, scal, text)

              /* Circle around point */
              if( epoint_) 
              {
                GLdouble breite = 24;
                int seiten = (int)(8 + breite/2);
                GLdouble x, y,
                         s = 2*M_PI/(GLdouble)seiten; // static variable

                // lines
                Lab2GL(f,.5,.5,1)
                glBegin(GL_LINE_STRIP);
                  glVertex3d( X+2*breite, Y-1, 9.999 );
                  glVertex3d( X+0.5*breite, Y-1, 9.999 );
                glEnd();
                glBegin(GL_LINE_STRIP);
                  glVertex3d( X-1.2*breite, Y-1, 9.999 );
                  glVertex3d( X-0.2*breite, Y-1, 9.999 );
                glEnd();
                glBegin(GL_LINE_STRIP);
                  glVertex3d( X, Y+1.2*breite, 9.999 );
                  glVertex3d( X, Y+0.5*breite, 9.999 );
                glEnd();

                // circle
                breite += 1;
                glBegin(GL_LINE_STRIP);
                  for(int i = 0; i <= seiten; ++i)
                  {
                    x = cos(i*(double)s)*breite;
                    y = sin(i*(double)s)*breite;
                    glVertex3d( X+x, Y+y, 9 );
                  }
                glEnd();
              }
            }

            if(icc_debug == 14) {
              glScalef(100,100,100);
              zeichneKoordinaten_();
              glScalef(.01,.01,.01);
            }
            DBG_ICCGL_V( maus_x_-x() <<" "<< -maus_y_+h() )
         }

       glPopMatrix();
    }

    bool strich_neu = false;
    double dzeit = icc_examin_ns::zeitSekunden() - zeit_;
    if(dzeit > 1./15. )
    {
      if(smooth && !schalen)
      {
        smooth = 0;
        strich_neu = true;
      }
    }
    if(dzeit < 1./40. || schalen)
    {
      if(!smooth)
      {
        smooth = 1;
        strich_neu = true;
      }
    }
    if(strich_neu) {
      //zeigeSpektralband_();
      garnieren_();
    }

    // measure speed
    static double zeit_alt = 0;
    snprintf(t, 128, "zeit_: %.01f id: %d f/s: %.03f theoretical f/s: %.03f",
             zeit_, id_, 1./(zeit_ - zeit_alt), 1./dzeit);
    zeit_alt = zeit_;
    DBG_PROG_V( t )


#   ifdef HAVE_FTGL
    if(ortho_font)
      glRasterPos2d(0, h() -10 );
#   endif
    if(maus_x_alt != maus_x_ || maus_y_alt != maus_y_)
      maus_steht = true;
    maus_x_alt = maus_x_;
    maus_y_alt = maus_y_;

    // colour channel name
    // text
    {
      glPushMatrix();
       glLoadIdentity();
       glMatrixMode(GL_PROJECTION);
       glLoadIdentity();
       glOrtho(0,w(),0,h(),-10.0,10.0);

       Lab2GL(textfarbe[0],textfarbe[1],textfarbe[2],1)

       glTranslatef(5,-12,0/*8.8 - schnitttiefe*3*/);

       if(typ() == 1) {
         kanalname.append(_("Channel"));
         kanalname.append(": ");
         kanalname.append(kanalName());

/*         oyNamedColour_s   * c = oyNamedColours_GetRef( colours_, 0 );
         const char* chan_name = 0;
         if(c)
         {
           chan_name = oyProfile_GetChannelName( c->profile_,
                                                           kanal, oyNAME_NAME );
           kanalname.append( chan_name );
           oyNamedColour_Release( &c );
  
         } else if(von_farb_namen_.size() > kanal) {

             kanalname.append( von_farb_namen_[kanal] );
         }
  */

#        ifdef HAVE_FTGL
         if(ortho_font)
           glRasterPos3d(0,ortho_font->FaceSize()*1.6/5.+20.,9.99);
#        endif
         ZeichneOText (ortho_font, scal, kanalname.c_str()) 
       } else
       {
         MARK( frei(true); )
         dreiecks_netze.frei(false);
         for (unsigned int i=0;
                i < dreiecks_netze.size();
                  ++i)
         {
           std::string text;
           text = dreiecks_netze[i].name;
           DBG_PROG_V( dreiecks_netze[i].name )
#          ifdef HAVE_FTGL
           if(ortho_font)
             glRasterPos2d(0, 20 + ortho_font->FaceSize() / 0.94
                                   * (dreiecks_netze.size()-i-1) );
#          endif
           if(dreiecks_netze[i].active())
             ZeichneOText (ortho_font, scal, text.c_str())
         }
         dreiecks_netze.frei(true);
         MARK( frei(false); )
       }

       if(icc_debug)
         ZeichneOText (ortho_font, scal, t)

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
    von_farb_namen_ = achs_namen;
  else
  { von_farb_namen_.clear();
    von_farb_namen_.push_back ("?");
    von_farb_namen_.push_back ("?");
    von_farb_namen_.push_back ("?");
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
 *  - show_points_as_pairs is present with zeig_punkte_als_paare being true
 *  - show_points_as_measurements with zeig_punkte_als_messwerte being true
 *  - spectralline with spektralband being true
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
  if(zeig_punkte_als_paare)
    text.append(" show_points_as_pairs");
  if(zeig_punkte_als_messwerte)
    text.append(" show_points_as_measurements");
  if(spektralband)
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

  if (!zeig_punkte_als_messwerte &&
      punktform == MENU_dE1STERN &&
      colours_)
    punktform = MENU_dE1KUGEL;

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

#if 0
void
GL_View::hineinPunkte       (ICClist<double>      &vect,
                                ICClist<std::string> &achs_namen)
{ DBG_PROG_START

  DBG_PROG_V( vect.size() )

  MARK( frei (false); )
  if(!punkte_.size() &&
     (int)vect.size() > (1000*3 *
       ((!zeig_punkte_als_messwerte && zeig_punkte_als_paare) ? 2:1)) )
    punktgroesse = 2;

  tabelle_.clear();DBG_PROG
  punkte_.clear(); DBG_PROG

  achsNamen(achs_namen);

  punkte_.resize( vect.size() );
  for (unsigned int i = 0; i < punkte_.size()/3; ++i)
  {
    punkte_[i*3+0] = vect[i*3+0];
    punkte_[i*3+1] = vect[i*3+1]*a_darstellungs_breite;
    punkte_[i*3+2] = vect[i*3+2]*b_darstellungs_breite;
  }
  
  DBG_PROG_V( zeig_punkte_als_paare<<"|"<<punktform<<"|"<<punkte_.size() )

  if (!zeig_punkte_als_messwerte &&
      punktform != MENU_dE1STERN &&
      punkte_.size())
    punktform = MENU_dE1STERN;

  if (!zeig_punkte_als_messwerte &&
      punktform == MENU_dE1STERN &&
      punkte_.size())
    punktform = MENU_dE1KUGEL;

  MARK( frei (true); )

  valid_=false;
  redraw();
  DBG_PROG_ENDE
}

void
GL_View::hineinPunkte      (ICClist<double>      &vect,
                               ICClist<float>       &punkt_farben,
                               ICClist<std::string> &achsNamen)
{ DBG_PROG_START

  MARK( frei (false); )
  // show curve from tag_browser
  farben_.clear();
  farben_ = punkt_farben;
  MARK( frei (true); )
  DBG_PROG_S( farben_.size()/4 << " colours" )

  hineinPunkte(vect,achsNamen);

  DBG_PROG_ENDE
}

void
GL_View::hineinPunkte      (ICClist<double> &vect,
                               ICClist<float>  &punkt_farben,
                               ICClist<std::string> &farb_namen,
                               ICClist<std::string> &achs_namen)
{ DBG_PROG_START
  // show curve from tag_browser
  DBG_NUM_V( farb_namen.size() <<"|"<< punkt_farben.size() )
  MARK( frei(false); )
  farb_namen_.clear();
  farb_namen_ = farb_namen;
  MARK( frei(true); )

  hineinPunkte(vect, punkt_farben, achs_namen);

  DBG_PROG_ENDE
}
#endif

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
              typ_ );
  }
  MARK( frei(true); )

  //valid_=false;
  redraw();
  DBG_PROG_ENDE
}

void getFacesFromICCnetz ( const ICCnetz & netz, icc_examin_ns::FACE **fList,
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
  for( it = netz.indexe.begin(); it != netz.indexe.end(); ++it )
  {
    std::pair<double,DreiecksIndexe> index_p( *it );
    icc_examin_ns::VERTEX * v = 0;

    for(int j = 2; j >= 0; --j)
    {
      if(netz.grau)
      {
        color.rr = color.gg = color.bb = netz.schattierung;
      } else {
        color.rr = netz.punkte[ index_p.second.i[j] ].farbe[0];
        color.gg = netz.punkte[ index_p.second.i[j] ].farbe[1];
        color.bb = netz.punkte[ index_p.second.i[j] ].farbe[2];
      }
      if(netz.active())
        color.aa = netz.undurchsicht;
      else
        color.aa = 0.0;

      v = allocVertex( netz.punkte[ index_p.second.i[j] ].koord[0]*scale_x,
                       netz.punkte[ index_p.second.i[j] ].koord[1]*scale_y,
                       netz.punkte[ index_p.second.i[j] ].koord[2]*scale_z,
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
          FARBEN (    vtrav->color.rr, vtrav->color.gg, vtrav->color.bb,
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
  if(icc_oyranos.colourServerActive() & XCM_COLOR_SERVER_PROFILES)
    prof_disp = oyProfile_Copy( edit_, NULL );
  else
    prof_disp = icc_oyranos.icc_oyranos.oyMoni(
              window()->x() + window()->w()/2, window()->y() + window()->h()/2);
  oyNamedColour_s * c = oyNamedColour_Create( 0, 0, 0, prof, 0 );
#endif
  oyOptions_s * opts = icc_examin->options();

  /* updateNet_ takes care of dreiecks_netze.frei */
  for( ftrav = faceList; ftrav != 0; ftrav = ftrav->fnext )
  {
    int pos = ftrav->group;
    ICCnetz & netz = dreiecks_netze[pos];

    for( vtrav = ftrav->vhead; vtrav->vnext != 0; vtrav = vtrav->vnext )
    {
      if(netz.grau)
      {
        vtrav->color.rr = vtrav->color.gg = vtrav->color.bb = netz.schattierung;
      } else {

#if USE_OY_NC
        double rgba[4] = {0,0,0,1};
        lab[0] = vtrav->xx; 
        lab[1] = vtrav->yy/a_darstellungs_breite;
        lab[2] = vtrav->zz/b_darstellungs_breite;
        LabToCIELab( lab, lab, 1 );

        oyNamedColour_SetColourStd( c, oyEDITING_LAB, lab, oyDOUBLE, 0, opts );

        oyNamedColour_GetColour( c, prof_disp, rgba, oyDOUBLE, 0, opts );

        vtrav->color.rr = rgba[0];
        vtrav->color.gg = rgba[1];
        vtrav->color.bb = rgba[2];
#else
        lab[0] = vtrav->xx;
        lab[1] = vtrav->yy/a_darstellungs_breite;
        lab[2] = vtrav->zz/b_darstellungs_breite;

        double * rgb = NULL;
        rgb = icc_oyranos.wandelLabNachBildschirmFarben( 
               window()->x() + window()->w()/2, window()->y() + window()->h()/2,
         (icc_oyranos.colourServerActive() & XCM_COLOR_SERVER_PROFILES)?edit_:0,
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
      if(netz.active())
        vtrav->color.aa = netz.undurchsicht;
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

/* updateNet_ takes care of dreiecks_netze.frei */
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
    hineinNetze_( dreiecks_netze );

  MARK( frei(false); )
  dreiecks_netze.frei(false);
  if(bsp)
    setBspProperties_(bsp);
  dreiecks_netze.frei(true);
  MARK( frei(true); )

  DBG_PROG_ENDE
}

void
GL_View::hineinNetze_       (const icc_examin_ns::ICCThreadList<ICCnetz> & d_n)
{
  DBG_PROG_START

  MARK( frei(false); )
  dreiecks_netze.frei(false);

  netz.clear();

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
                           1, a_darstellungs_breite, b_darstellungs_breite );

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
        netz.insert( d_n[i] );

    for(unsigned k = 0; k < netz.punkte.size(); ++k)
    {
      netz.punkte[k].koord[1] *= a_darstellungs_breite;
      netz.punkte[k].koord[2] *= b_darstellungs_breite;
    }

  }

  dreiecks_netze.frei(true);
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
GL_View::hineinTabelle (ICClist<ICClist<ICClist<ICClist<double> > > > vect,
                           ICClist<std::string> achs_namen,
                           ICClist<std::string> nach,
                                       ICClist<int>        channels)
{ DBG_PROG_START

  MARK( frei(false); )
  tabelle_ = vect;  DBG_PROG
  nach_farb_namen_ = nach; DBG_PROG

  achsNamen(achs_namen);
  channels_ = channels;

  MARK( frei(true); )

  namedColoursRelease();

  valid_=false;
  redraw();

  glStatus(_("left-/middle-/right mouse button -> rotate/cut/menu"), typ_);

  DBG_PROG_ENDE
}


void
GL_View::menueAufruf ( int value )
{
  DBG_PROG_START

  if(visible()) {
    ;//DBG_PROG_S( "visible "<< id )
  }

  {
    if (value >= MENU_MAX &&
        value < 100) {
      kanal = value - MENU_MAX; DBG_PROG_V( kanal )
      glStatus(_("left-/middle-/right mouse button -> rotate/cut/menu"), typ_);
    }
#   if APPLE
    double farb_faktor = 1./*0.6666*/ *0.8;
#   else
    double farb_faktor = 1.*.8;
#   endif

    switch (value) {
    case MENU_AXES:
      DrawAxes = !DrawAxes;
      break;
    case MENU_QUIT:
      break;
    case MENU_WUERFEL:
      punktform = MENU_WUERFEL;
      break;
    case MENU_GRAU:
      punktfarbe = MENU_GRAU;
      schalen = 0;
      break;
    case MENU_FARBIG:
      punktfarbe = MENU_FARBIG;
      schalen = 0;
      break;
    case MENU_KONTRASTREICH:
      punktfarbe = MENU_KONTRASTREICH;
      schalen = 0;
      break;
    case MENU_SCHALEN:
      if (! schalen)
         schalen = 5;
      else
         schalen = 0;
      break;
    case MENU_dE1STERN:
       punktform = MENU_dE1STERN;
      break;
    case MENU_dE1KUGEL:
       punktform = MENU_dE1KUGEL;
      break;
    case MENU_dE2KUGEL:
       punktform = MENU_dE2KUGEL;
      break;
    case MENU_dE4KUGEL:
       punktform = MENU_dE4KUGEL;
      break;
    case MENU_DIFFERENZ_LINIE:
       if(punktform == MENU_dE1STERN) punktform = MENU_DIFFERENZ_LINIE; else punktform = MENU_dE1STERN;
      break;
    case MENU_SPEKTRALBAND:
      if (! spektralband)
         spektralband = MENU_SPEKTRALBAND;
      else
         spektralband = 0;
      break;
    case MENU_HELFER:
      if (! zeige_helfer)
         zeige_helfer = MENU_HELFER;
      else
         zeige_helfer = 0;
      break;
    case MENU_WEISS:
      hintergrundfarbe = 1.;//MENU_WEISS;
      pfeilfarbe[0] = (float)(1.*farb_faktor);
      textfarbe[0] = (float)(.75*farb_faktor);
      break;
    case MENU_HELLGRAU:
      hintergrundfarbe = 0.75;//MENU_HELLGRAU;
      pfeilfarbe[0] = (float)(1.0*farb_faktor);
      textfarbe[0] = (float)(0.5*farb_faktor);
      break;
    case MENU_GRAUGRAU:
      hintergrundfarbe = 0.5;//MENU_GRAUGRAU;
      pfeilfarbe[0] = (float)(.75*farb_faktor);
      textfarbe[0] = (float)(0.25*farb_faktor);
      break;
    case MENU_DUNKELGRAU:
      hintergrundfarbe = 0.25;//MENU_DUNKELGRAU;
      pfeilfarbe[0] = (float)(0.5*farb_faktor);
      textfarbe[0] = (float)(0.75*farb_faktor);
      break;
    case MENU_SCHWARZ:
      hintergrundfarbe = 0.0;//MENU_SCHWARZ;
      pfeilfarbe[0] = (float)(.25*farb_faktor);
      textfarbe[0] = (float)(0.5*farb_faktor);
      break;
    case Agviewer::FLYING:
      glStatus(_("left mouse button -> go back"), typ_);
      agv_->thin = true;
      break;
    case Agviewer::ICCFLY_L:
      if(typ() == 2) {
        agv_->eyeDist( 2 * agv_->dist() );
        vorder_schnitt = std_vorder_schnitt + agv_->dist();
      } else {
        agv_->eyeDist( agv_->dist() );
        vorder_schnitt = std_vorder_schnitt;
      }
      glStatus(_("left mouse button -> go back"), typ_);
      agv_->thin = true;
      break;
    case Agviewer::ICCFLY_a:
      vorder_schnitt = std_vorder_schnitt;
      glStatus(_("left mouse button -> go back"), typ_);
      agv_->thin = true;
      break;
    case Agviewer::ICCFLY_b:
      vorder_schnitt = std_vorder_schnitt;
      glStatus(_("left mouse button -> go back"), typ_);
      agv_->thin = true;
      break;
    case Agviewer::ICCPOLAR:
      agv_->thin = true;
    case Agviewer::POLAR:
      if(typ() == 1)
        agv_->thin = true;
      else
        agv_->thin = false;
      break;
    case Agviewer::AGV_STOP:
      agv_->thin = false;
      glStatus(_("left-/middle-/right mouse button -> rotate/cut/menu"), typ_);
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
  int mausknopf = Fl::event_state();
  int schluss = 1;
  DBG_MEM_V( dbgFltkEvent(event) )

  tastatur(event);

  switch(event)
  {
    case FL_PUSH:
         if(mausknopf & FL_BUTTON3 ||
           (mausknopf & FL_BUTTON1 && mausknopf & FL_CTRL) ) {
           menue_button_->popup();
           break;
         }
         if(mausknopf & FL_BUTTON1 && mausknopf & FL_SHIFT)
           mausknopf = FL_BUTTON2;

         if(mausknopf & FL_BUTTON2)
           fl_cursor( FL_CURSOR_NS, FL_BLACK, FL_WHITE );
         else
           fl_cursor( FL_CURSOR_HAND, FL_BLACK, FL_WHITE );

         agv_->agvHandleButton( mausknopf, event, Fl::event_x(),Fl::event_y());
         DBG_BUTTON_S( "FL_PUSH bei: " << Fl::event_x() << "," << Fl::event_y() )
         break;
    case FL_RELEASE:
         agv_->agvHandleButton(Fl::event_state(),event, Fl::event_x(),Fl::event_y());
         DBG_BUTTON_S( "FL_RELEASE bei: " << Fl::event_x() << "," << Fl::event_y() )
         fl_cursor( FL_CURSOR_DEFAULT, FL_BLACK, FL_WHITE );
         break;
    case FL_DRAG:
         DBG_BUTTON_S( "FL_DRAG bei: " << Fl::event_x() << "," << Fl::event_y() )
         maus_x_ = Fl::event_x();
         maus_y_ = Fl::event_y();
         agv_->agvHandleMotion(maus_x_, maus_y_);
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
         maus_x_ = Fl::event_x();
         maus_y_ = Fl::event_y();
         if(visible() && !darfBewegen()) {
           redraw();
         }
         text[0] = 0;
         break;
    case FL_MOUSEWHEEL:
         DBG_BUTTON_S( "FL_MOUSEWHEEL" << Fl::event_dx() << "," << Fl::event_dy() )
         if(mausknopf & FL_SHIFT)
         {
           fl_cursor( FL_CURSOR_MOVE, FL_BLACK, FL_WHITE );
         
           double clip_old = vorder_schnitt;
           vorder_schnitt -= Fl::event_dy()*0.01;
           if(vorder_schnitt - clip_old != 0 &&
             !darfBewegen())
           redraw();
           fl_cursor( FL_CURSOR_DEFAULT, FL_BLACK, FL_WHITE );
         } else
         agv_->agvHandleButton( mausknopf,event, Fl::event_dx(),Fl::event_dy());
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
  if(mausknopf)
  {
    switch(Fl::event_state()) {
      case FL_BUTTON1: DBG_BUTTON_S("FL_BUTTON1") break;
      case FL_BUTTON2: DBG_BUTTON_S("FL_BUTTON2") break;
      case FL_BUTTON3: DBG_BUTTON_S("FL_BUTTON3") break;
      default: DBG_BUTTON_S("unknown event_state()") break;
    }
  }

  DBG_ICCGL_ENDE
  return schluss;
}

int
GL_View::tastatur(int e)
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
      double clip_old = vorder_schnitt;
      if(Fl::event_key() == FL_Up) {
        vorder_schnitt += 0.01;
      } else if(Fl::event_key() == FL_Down) {
        vorder_schnitt -= 0.01;
      } else if(Fl::event_key() == FL_Home) {
        vorder_schnitt = 4.2;
      } else if(Fl::event_key() == FL_End) {
        vorder_schnitt = agv_->eyeDist();
      }
      if(vorder_schnitt - clip_old != 0 &&
         !darfBewegen())
        redraw();

      int k = ((char*)Fl::event_text())[0];

      switch (k) {
      case '-':
        if(tabelle_.size() && schalen != 0)
        {
          this->level -= this->level_step;
          if(this->level <= .0)
            this->level = .0001;
          auffrischen_();
          redraw();
        }
        else if(punktform >= MENU_dE1KUGEL && punktform <= MENU_dE4KUGEL)
        {
          if (punktform > MENU_dE1KUGEL) {
            --punktform;
          }
        }
        else if (punktform == MENU_DIFFERENZ_LINIE)
          punktform = MENU_dE1KUGEL;
        else if(punktgroesse > 1)
        {
          --punktgroesse;
          auffrischen_();
          redraw();
        }
        DBG_PROG_V( Fl::event_key() <<" "<< punktgroesse )
        found = 1;
        break;
      case '+': // 43
        if(tabelle_.size() && schalen != 0)
        {
          this->level += this->level_step;
          if(this->level >= 1.0)
            this->level = .9999;
          auffrischen_();
          redraw();
        }
        else if (punktform >= MENU_dE1KUGEL && punktform <= MENU_dE4KUGEL)
        {
          if (punktform < MENU_dE4KUGEL) {
            ++punktform;
          }
        }
        else if (punktform == MENU_DIFFERENZ_LINIE)
        {
          punktform = MENU_dE4KUGEL;
        }
        else if (punktgroesse < 61)
        {
          ++punktgroesse;
          auffrischen_();
          redraw();
        }
        DBG_PROG_V( Fl::event_key()  <<" "<< punktgroesse <<" "<< punktform <<" "<< MENU_DIFFERENZ_LINIE )
        found = 1;
        break;
      case '*':
        if(tabelle_.size() && schalen != 0)
        {
          this->level_step *= 2;
          if(this->level_step > 0.999)
            this->level_step = 0.999;
        }
        found = 1;
        break;
      case '/':
        if(tabelle_.size() && schalen != 0)
        {
          this->level_step /= 2;
          if(this->level_step < 0.0001)
            this->level_step = 0.0001;
        }
        found = 1;
        break;
      case '_':
        if(tabelle_.size())
        {
          schalen = -schalen;
          auffrischen_();
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
        if(tabelle_.size())
        {
          if(schalen<0)
            schalen = -(k - '0');
          else
            schalen = (k - '0');

          auffrischen_();
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
GL_View::c_ ( Fl_Widget* w, void* daten )
{ DBG_ICCGL_START

  intptr_t value = (intptr_t) daten;
  DBG_PROG_V( value )

  GL_View *gl_obj = dynamic_cast<GL_View*>(w->parent());
  DBG_MEM_V( (intptr_t)gl_obj )
  DBG_MEM_V( (intptr_t)w->parent() )
  if(!w->parent())
    WARN_S("Could not find parents.")
  else
  if (gl_obj)
  {
    gl_obj->menueAufruf((int)value);
  }
  else
    WARN_S("could not find a suitable program structure")

  DBG_ICCGL_ENDE
}



int
GL_View::hintergrundfarbeZuMenueeintrag( float farbe )
{
  int eintrag = MENU_HELLGRAU;

  if(hintergrundfarbe == 1.0)
    eintrag = MENU_WEISS;
  if(hintergrundfarbe == 0.75)
    eintrag = MENU_HELLGRAU;
  if(hintergrundfarbe == 0.5)
    eintrag = MENU_GRAUGRAU;
  if(hintergrundfarbe == 0.25)
    eintrag = MENU_DUNKELGRAU;
  if(hintergrundfarbe == 0.0)
    eintrag = MENU_SCHWARZ;

  return eintrag;
}

#undef ZeichneText
#undef FARBE

