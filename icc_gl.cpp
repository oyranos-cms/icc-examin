/*
 * ICC Examin ist eine ICC Profil Betrachter
 * 
 * Copyright (C) 2004-2007  Kai-Uwe Behrmann 
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

#include <oyranos/oyranos_monitor.h>

#include <FL/Fl_Menu_Button.H>
#include <FL/Fl.H>
#include <FL/gl.h>
#include <FL/Fl_Preferences.H>
#include <FL/filename.H>

#ifdef HAVE_FTGL
#include <FTGL/FTFont.h>
#include <FTGL/FTGLTextureFont.h>
#include <FTGL/FTGLPixmapFont.h>
#include <FTGL/FTGLPolygonFont.h>
#include <FTGL/FTGLExtrdFont.h>
#endif

#include <cmath>

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
#define DBG_ICCGL_START DBG_NUM_START
#define DBG_ICCGL_ENDE DBG_NUM_ENDE
#define DBG_ICCGL_V( texte ) DBG_NUM_V( texte )
#define DBG_ICCGL_S( texte ) DBG_NUM_S( texte )
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

const double GL_Ansicht::std_vorder_schnitt = 4.2;
#ifdef HAVE_FTGL
FTFont *font = NULL, *ortho_font = NULL;
#endif

// set material colours
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

#define ZeichneOText(Font, scal, buffer) { \
                                   glScaled(scal,scal*w()/(double)h(),scal); \
                                   ZeichneText(Font, buffer); \
                                   glScaled(1.0/scal,1.0/(scal*w()/(double)h()),1.0/scal); \
                                 }


int GL_Ansicht::ref_ = 0;

GL_Ansicht::GL_Ansicht(int X,int Y,int W,int H)
  : Fl_Gl_Window(X,Y,W,H)
{ DBG_PROG_START
  init_();
  DBG_PROG_ENDE
}

GL_Ansicht::GL_Ansicht(int X,int Y,int W,int H, const char *l)
  : Fl_Gl_Window(X,Y,W,H,l)
{ DBG_PROG_START
  init_();
  DBG_PROG_ENDE
}

void
GL_Ansicht::init_()
{ DBG_PROG_START

  id_ = ref_;
  ++ref_;

  agv_ = NULL;
  agv_ = this->getAgv(this, NULL);

  kanal = 0;
  schnitttiefe = 0.01;
  vorder_schnitt = std_vorder_schnitt;
  a_darstellungs_breite = 1.0;
  b_darstellungs_breite = 1.0;
  schalen = 5;
  punktform = MENU_dE1STERN;
  punktgroesse = 8;
  //punkt_zahl_alt = 0;
  hintergrundfarbe = 0.75;
  spektralband = 0;
  zeige_helfer = true;
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
  maus_x_ = 0;
  maus_y_ = 0;
  maus_x_alt = -1;
  maus_y_alt = -1;
  maus_steht = false;
  valid_ = false;
  zeit_ = 0;
  text[0] = 0;
  colours_ = 0;
  epoint_ = 0;
  oyProfile_s * prof = oyProfile_FromStd( oyEDITING_XYZ, NULL );
  mouse_3D_hit = oyNamedColour_Create( NULL, NULL,0, prof, 0 );
  oyProfile_Release( &prof );

  for(int i = 0; i <= DL_MAX; ++i)
    gl_listen[i] = 0;

  DBG_PROG_ENDE
}

GL_Ansicht::~GL_Ansicht()
{ DBG_PROG_START
  if (gl_listen[RASTER]) {
    DBG_PROG_S( "delete glListe " << gl_listen[RASTER] )
    glDeleteLists (gl_listen[RASTER],1);
    gl_listen[RASTER] = 0;
  }
  if (gl_listen[HELFER]) {
    DBG_PROG_S( "delete glListe " << gl_listen[HELFER] )
    glDeleteLists (gl_listen[HELFER],1);
    gl_listen[HELFER] = 0;
  }
  if (gl_listen[PUNKTE]) {
    DBG_PROG_S( "delete glListe " << gl_listen[PUNKTE] )
    glDeleteLists (gl_listen[PUNKTE],1);
    gl_listen[PUNKTE] = 0;
  }
  if (gl_listen[SPEKTRUM]) {
    DBG_PROG_S( "delete glListe " << gl_listen[SPEKTRUM] )
    glDeleteLists (gl_listen[SPEKTRUM],1);
    gl_listen[SPEKTRUM] = 0;
  }
  if (gl_listen[UMRISSE]) {
    DBG_PROG_S( "delete glListe " << gl_listen[UMRISSE] )
    glDeleteLists (gl_listen[UMRISSE],1);
    gl_listen[UMRISSE] = 0;
  }
# ifdef HAVE_FTGL
  //if(font) delete font;
  //if(ortho_font) delete ortho_font;
# endif

  namedColoursRelease();

  --ref_;

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

  DBG_PROG_ENDE
}

GL_Ansicht&
GL_Ansicht::copy (const GL_Ansicht & gl)
{ DBG_PROG_START

  kanal = gl.kanal;
  schnitttiefe = gl.schnitttiefe;
  vorder_schnitt = gl.vorder_schnitt;
  a_darstellungs_breite = gl.a_darstellungs_breite;
  b_darstellungs_breite = gl.b_darstellungs_breite;
  schalen = gl.schalen;
  punktform = gl.punktform;
  punktgroesse = gl.punktgroesse;
  //punkt_zahl_alt = gl.punkt_zahl_alt;
  hintergrundfarbe = gl.hintergrundfarbe;
  spektralband = gl.spektralband;
  zeige_helfer = gl.zeige_helfer;
  zeig_punkte_als_paare = gl.zeig_punkte_als_paare;
  zeig_punkte_als_messwerte = gl.zeig_punkte_als_messwerte;
  typ_ = gl.typ_;
  strichmult = gl.strichmult;
  strich1 = gl.strich1;
  strich2 = gl.strich2;
  strich3 = gl.strich3;
  blend = gl.blend;
  smooth = gl.smooth;
  waiting_ = gl.waiting_;
  maus_x_ = gl.maus_x_;
  maus_y_ = gl.maus_y_;
  maus_steht = gl.maus_steht;
  valid_ = gl.valid_;
  zeit_ = gl.zeit_;

  for(int i = 0; i <= DL_MAX; ++i)
    gl_listen[i] = 0;

# ifdef HAVE_FTGL
  font = ortho_font = 0;
# endif

  tabelle_ = gl. tabelle_;
  nach_farb_namen_ = gl. nach_farb_namen_;
  von_farb_namen_ = gl. von_farb_namen_;
  colours_ = oyNamedColours_Copy( gl.colours_, NULL );

  DBG_PROG_ENDE
  return *this;
}

void
GL_Ansicht::init(int ty)
{ DBG_PROG_START

  // 1 fuer mft_gl und 2 fuer DD_farbraum
  typ_ = ty;

  DBG_PROG

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

  DBG_PROG_V( spektralband )

  resizable(0);

# define TEST_GL(modus) { \
    mode(modus); \
    if(can_do()) { \
      mod |= modus; \
      mode(mod); \
      DBG_PROG_S( "OpenGL understand: ja   " << #modus <<" "<< mode() ) \
    } else { \
      DBG_PROG_S( "OpenGL understand: nein " << #modus <<" "<< mode() ) \
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

  if (typ_ > 1) {
    DBG_PROG_S("gl window " << id_)
    a_darstellungs_breite = 2.55;
    b_darstellungs_breite = 2.55;
    agv_->distA (agv_->distA()
                           + a_darstellungs_breite/2.0 - 0.5);
    agv_->distB (agv_->distB()
                           + b_darstellungs_breite/2.0 - 0.5);
    agv_->eyeDist (agv_->dist()*2.0);
  }

  gl_listen[AXES] = glGenLists(1);
  agv_->agvMakeAxesList( gl_listen[AXES] ); DBG_PROG

  gl_font( FL_HELVETICA, 10 );

  // initialise
  menueInit_(); DBG_PROG
  menueAufruf (hintergrundfarbeZuMenueeintrag(hintergrundfarbe)); // colour sheme
  menueAufruf (MENU_GRAU);     // CLUT colour sheme
  schatten = 0.1f;
  if (typ() == 1) menueAufruf (MENU_WUERFEL);

  maus_steht = false;

  DBG_PROG_ENDE
}

/** @brief localise the position */
void
GL_Ansicht::mausPunkt_( GLdouble &oX, GLdouble &oY, GLdouble &oZ,
                        GLdouble &X, GLdouble &Y, GLdouble &Z, int from_mouse )
{
  DBG_PROG_START  
  // localise position
  // how far is the next object in this direction, very laborious
  GLfloat zBuffer = 0;
  glReadPixels((GLint)maus_x_,(GLint)h()-maus_y_,1,1,GL_DEPTH_COMPONENT, GL_FLOAT, &zBuffer);
  GLdouble modell_matrix[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
           projektions_matrix[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
  GLint bildschirm[4] = {0,0,0,0};
  glGetDoublev(GL_MODELVIEW_MATRIX, modell_matrix);
  glGetDoublev(GL_PROJECTION_MATRIX, projektions_matrix);
  glGetIntegerv(GL_VIEWPORT, bildschirm);
  if(from_mouse)
    gluUnProject(maus_x_, h()-maus_y_, zBuffer,
                 modell_matrix, projektions_matrix, bildschirm,
                 &oX, &oY, &oZ);


  DBG_PROG_V( "X: "<<oX<<" Y: "<<oY<<" Z: "<<oZ<<" "<<id_ )

  gluProject( oX, oY, oZ,
              modell_matrix, projektions_matrix, bildschirm,
              &X,&Y,&Z);
  DBG_PROG_ENDE
}

static int zahl = 0;
void
GL_Ansicht::bewegenStatisch_ (void* gl_a)
{
  DBG_ICCGL_START

  GL_Ansicht *gl_ansicht = (GL_Ansicht*)gl_a;

  if (!gl_ansicht) {
      WARN_S( "no GL_Ansicht provided " << gl_ansicht->id_ )
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

bool GL_Ansicht::darfBewegen()        { return agv_->darf_bewegen_; }
void GL_Ansicht::darfBewegen(int d)
{
  agv_->darf_bewegen_ = d?true:false; 
  if (d)
    Fl::add_timeout(0.04, bewegenStatisch_,this);

  DBG_ICCGL_S("Stop Bewegen "<<zahl<<" "<<d)
}

void
GL_Ansicht::bewegen (bool setze)
{
  DBG_ICCGL_START
  darfBewegen( setze );
  DBG_ICCGL_V( setze )
  if(!setze) {
    agv_->agvSwitchMoveMode (Agviewer::AGV_STOP);
    agv_->benachrichtigen(ICCexamin::GL_STOP);
  }
  DBG_ICCGL_ENDE
}

int
GL_Ansicht::auffrischen_()
{
  DBG_PROG_START

  menueErneuern_();

  int err = erstelleGLListen_();
  if(err)
    valid_ = false;

  DBG_PROG_ENDE
  return err;
}

void
GL_Ansicht::hide()
{   
  DBG_PROG_START
  DBG_PROG_V( visible()<<" "<<shown() )
  icc_examin->alle_gl_fenster->beobachterFort(this);
  Fl_Gl_Window::hide();
  DBG_PROG_ENDE
}     

void
GL_Ansicht::show()
{   
  DBG_PROG_START
  DBG_PROG_V( visible()<<" "<<shown() )
  icc_examin->alle_gl_fenster->beobachterDazu(this);
  if( window()->visible() )
    Fl_Gl_Window::show();
  DBG_PROG_ENDE
}     


/** recive of a drawing news */
void
GL_Ansicht::nachricht(icc_examin_ns::Modell* modell, int info)
{
  DBG_PROG_START
  DBG_PROG_V( info<<" "<<window()->visible()<<" "<<visible()<<" "<<shown()<<" "<<id_ )

  if( visible() && shown() && !agv_->parent->visible() )
    agv_->reparent(this);

  if(info == ICCexamin::GL_AUFFRISCHEN)
  {
    invalidate();
    erstelleGLListen_();
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
GL_Ansicht::redraw()
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
    agv_->benachrichtigen(ICCexamin::GL_ZEICHNEN);
  DBG_PROG_ENDE
}

void
GL_Ansicht::draw()
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
GL_Ansicht::getAgv( GL_Ansicht *ansicht, GL_Ansicht *referenz )
{ DBG_PROG_START
  Agviewer *agv = NULL;

  if(!agv && referenz)
  {
    agv = referenz->agv_;
    agv -> reparent( ansicht );
  }

  if(!agv)
    agv = new Agviewer (ansicht);

  if(ansicht->agv_)
    ansicht->modellFort( ansicht->agv_ );

  ansicht->agv_ = agv;

  ansicht->modellDazu( ansicht->agv_ );

  DBG_ICCGL_ENDE
  return agv;
}

void
GL_Ansicht::GLinit_()
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

  const char* font_name = "/usr/X11R6/lib/X11/fonts/truetype/FreeSans.ttf";
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
    font_name = "/usr/X11R6/lib/X11/fonts/truetype/arial.ttf";
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
            sprintf (n, "%s/FreeSans.ttf", SRCDIR);
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
    font->Depth(12);
    if(!font->FaceSize(72)) WARN_S("Fontsize not setable"); \
    ortho_font->CharMap( ft_encoding_unicode );
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
GL_Ansicht::zeichneKoordinaten_()
{ DBG_ICCGL_START
  char text[256];
  GLfloat farbe[] =   { 1.0, 1.0, 1.0, 1.0 };

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
        sprintf (&text[0],_("0,0,0"));
        ZeichneText(font, text)
      glTranslated(0.3,0.1,0.05);
    glRotated (-90,0.0,0,1.0);

    FARBE(1,1,1,1)
    glTranslated(.1,0,0);
      FARBE(1,0,1,1)
      glRotated (90,0.0,1.0,.0);
        zeichneKegel(0.01, 0.025, 8);
      glRotated (-90,0.0,1.0,.0);
      FARBE(1,1,1,1)
      glTranslated(.02,0,0);
        ZeichneText(font, "X")
      glTranslated((-0.02),0,0);
    glTranslated((-0.1),0,0);

    glTranslated(.0,.1,0);
      glRotated (270,1.0,.0,.0);
        FARBE(1,1,0,1)
        zeichneKegel(0.01, 0.025, 8);
      glRotated (90,1.0,.0,.0);
      glRotated (90,0.0,.0,1.0);
        FARBE(1,1,1,1)
        ZeichneText(font, "Y")
      glRotated (270,0.0,.0,1.0);
    glTranslated(.0,(-0.1),0);

    glTranslated(0,0,.1);
      FARBE(0,1,1,1)
      zeichneKegel(0.01, 0.025, 8);
      glRotated (90,0.0,.5,.0);
        glTranslated(-.1,0,0);
          FARBE(1,1,1,1)
          ZeichneText(font, "Z")
        glTranslated(.1,0,0);
      glRotated (270,0.0,.5,.0);
    glTranslated(0,0,-.1);
  DBG_ICCGL_ENDE
}

int
GL_Ansicht::erstelleGLListen_()
{ DBG_PROG_START

  if(!frei())
    return 1;

  MARK( frei(false); )

  garnieren_();

  tabelleAuffrischen();

  hineinNetze (dreiecks_netze);

  if(dreiecks_netze.size())
    zeigeUmrisse_();
 
  zeigeSpektralband_();

  DBG_PROG_V( punktform <<" "<< MENU_dE1STERN )
  punkteAuffrischen();

  //background
       static double hintergrundfarbe_statisch = hintergrundfarbe;
       if(hintergrundfarbe != hintergrundfarbe_statisch)
         DBG_NUM_S("background colour changed"
                <<" "<<id_<<" colour: "<<hintergrundfarbe );

  glClearColor(hintergrundfarbe,hintergrundfarbe,hintergrundfarbe,1.);
  /*switch (hintergrundfarbe) {
    case MENU_WEISS:
      glClearColor(1.,1.,1.,1.0);
      break;
    case MENU_HELLGRAU:
      glClearColor(.75,.75,.75,1.0);
      break;
    case MENU_GRAUGRAU:
      glClearColor(.5,.5,.5,1.0);
      break;
    case MENU_DUNKELGRAU:
      glClearColor(.25,.25,.25,1.0);
      break;
    case MENU_SCHWARZ:
      glClearColor(.0,.0,.0,1.0);
      break;
  }*/

  MARK( frei(true); )

  DBG_PROG_ENDE
  return 0;
}

void
GL_Ansicht::textGarnieren_()
{
  DBG_PROG_START
  char text[256];
  char* ptr = 0;
  GLfloat ueber = 0.035f;
  GLfloat farbe[] =   { 1.0, 1.0, 1.0, 1.0 };

    glDisable(GL_TEXTURE_2D);
    glDisable(GL_LIGHTING);
    FARBE(textfarbe[0],textfarbe[1],textfarbe[2],1)

    // CIE*L - top
    glPushMatrix();
      glLoadIdentity();
      if (von_farb_namen_.size())
      {
        ptr = (char*) von_farb_namen_[0].c_str();
        sprintf (&text[0], ptr);
        glRasterPos3d (0, .5+ueber, 0);
        ZeichneOText(ortho_font, 1, text)
      }

    // CIE*a - right
      if (von_farb_namen_.size())
      {
        ptr = (char*) von_farb_namen_[1].c_str();
        sprintf (&text[0], ptr);
        if (von_farb_namen_.size() &&
            von_farb_namen_[1] == _("CIE *a"))
          glRasterPos3d (.0, -.5, a_darstellungs_breite/2.+ueber);
        else
          glRasterPos3d (.0, .0, a_darstellungs_breite/2.+ueber);
        ZeichneOText(ortho_font, 1, text)
      }

    // CIE*b - left
      if (von_farb_namen_.size())
      {
        ptr = (char*) von_farb_namen_[2].c_str();
        sprintf (&text[0], ptr);
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
GL_Ansicht::garnieren_()
{
  DBG_PROG_START

# define PFEILSPITZE zeichneKegel(0.02, 0.05, 16);

  DBG_PROG_V( id() )
  // arrow and text
  if (gl_listen[HELFER]) {
    glDeleteLists (gl_listen[HELFER], 1);
  }

  GL_Ansicht::gl_listen[HELFER] = glGenLists(1);

  glNewList( gl_listen[HELFER], GL_COMPILE); DBG_PROG_V( gl_listen[HELFER] )
    GLfloat farbe[] =   { pfeilfarbe[0],pfeilfarbe[1],pfeilfarbe[2], 1.0 };
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
      FARBE(pfeilfarbe[0],pfeilfarbe[1],pfeilfarbe[2],1)
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

#   define ZEIG_GITTER 1

    // CIE*a - right
    glPushMatrix();
      if (von_farb_namen_.size() &&
          von_farb_namen_[1] == _("CIE *a"))
        glTranslatef(0,-.5,0);
      FARBE(pfeilfarbe[0],pfeilfarbe[1],pfeilfarbe[2],1)
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
      if (von_farb_namen_.size() &&
          von_farb_namen_[1] == _("CIE *a"))
      {
        FARBE(.2,.9,0.7,1)
        PFEILSPITZE
      }
      glTranslated(.0,0.0,-a_darstellungs_breite);
      glRotated (180,0.0,.5,.0);
      if (von_farb_namen_.size() &&
          von_farb_namen_[1] == _("CIE *a"))
        FARBE(.9,0.2,0.5,1)
      PFEILSPITZE
    glPopMatrix(); DBG_PROG

    // CIE*b - left
    glPushMatrix();
      if (von_farb_namen_.size() &&
          von_farb_namen_[2] == _("CIE *b"))
        glTranslated(0,-0.5,0);
      FARBE(pfeilfarbe[0],pfeilfarbe[1],pfeilfarbe[2],1)
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
      if (von_farb_namen_.size() &&
          von_farb_namen_[2] == _("CIE *b"))
        FARBE(.9,.9,0.2,1)
      glRotated (90,0.0,.5,.0);
      PFEILSPITZE
      glRotated (180,.0,.5,.0);
      glTranslated(.0,.0,b_darstellungs_breite);
      if (von_farb_namen_.size() &&
          von_farb_namen_[2] == _("CIE *b"))
      {
        FARBE(.7,.8,1.0,1)
        PFEILSPITZE
      }
    glPopMatrix();
    glLineWidth(strich1*strichmult);

  glEndList();

  DBG_PROG_ENDE
}

void
GL_Ansicht::tabelleAuffrischen()
{ DBG_PROG_START
  GLfloat farbe[] =   { textfarbe[0],textfarbe[1],textfarbe[2], 1.0 };

  DBG_PROG_V( tabelle_.size() )
  // correct the channel selection
  if(tabelle_.size()) {
    if( (int)tabelle_[0][0][0].size() <= kanal) {
      kanal = (int)tabelle_[0][0][0].size()-1;
      DBG_PROG_S( "Kanalauswahl geaendert: " << kanal )
    }
  }

  // table
  if (gl_listen[RASTER]) {
    glDeleteLists ( gl_listen[RASTER], 1);
    gl_listen[RASTER] = 0;
  }

  if (tabelle_.size())
  {
    gl_listen[RASTER] = glGenLists(1);
    glNewList( gl_listen[RASTER], GL_COMPILE); DBG_PROG_V( gl_listen[RASTER] )
      int n_L = (int)tabelle_.size(), n_a=(int)tabelle_[0].size(), n_b=(int)tabelle_[0][0].size();
      double dim_x = 1.0/(n_b); DBG_PROG_V( dim_x )
      double dim_y = 1.0/(n_L); DBG_PROG_V( dim_y )
      double dim_z = 1.0/(n_a); DBG_PROG_V( dim_z )
      double start_x,start_y,start_z, x,y,z;
      double wert;

      schnitttiefe= HYP3(dim_x,dim_y,dim_z);
      DBG_NUM_V( schnitttiefe );
      start_x = start_y = start_z = x = y = z = 0.5; start_y = y = -0.5;
      glPushMatrix();
#     ifndef Beleuchtung_
      glDisable(GL_LIGHTING);
#     endif
      DBG_PROG_V( tabelle_.size() <<" "<< tabelle_[0].size() )
      float korr = 0.995f/2.0f;
      glTranslated(-0.5/0.995+dim_x/2,-0.5/0.995+dim_y/2,-0.5/0.995+dim_z/2);
      for (int L = 0; L < (int)n_L; L++) {
        x = start_x + L * dim_y;
        for (int a = 0; a < (int)n_a; a++)
        {
          y = start_y + a * dim_z;
          for (int b = 0; b < (int)n_b; b++) {
            z = start_z + b * dim_x;
            wert = tabelle_[L][a][b][kanal]; //DBG_PROG_V( L << a << b << kanal )
#           ifdef Beleuchtung_
            FARBE(wert, wert, wert,1)
            //glColor3f(wert, wert, wert);
#           else
            switch (punktfarbe) {
              case MENU_GRAU:   glColor4d( wert, wert, wert, 1.0); break;
              case MENU_FARBIG: glColor4d((wert*2),
                                           wert*2-1.0,
                                           1.0-wert, 1.0);         break;
              case MENU_KONTRASTREICH: wert = wert * 6;
                                while(wert > 1.0) { wert = wert - 1.0; }
                                if (schalen && wert < 0.80) wert = 0.0;
                                if (0/*schalen*/) {
                                  glDisable(GL_LIGHTING);
                                  FARBE(wert, wert, wert,1)
                                  glEnable(GL_LIGHTING);
                                } else glColor4d( wert, wert, wert, 1.0);
                                                                   break;
            }
#           endif
            if (wert) {
                glBegin(GL_TRIANGLE_FAN);
                  glVertex3d(dim_x*b+ dim_x*korr,dim_y*L+ dim_y*korr,dim_z*a+ dim_z*korr);
                  glVertex3d(dim_x*b+ dim_x*korr,dim_y*L+-dim_y*korr,dim_z*a+ dim_z*korr);
                  glVertex3d(dim_x*b+-dim_x*korr,dim_y*L+-dim_y*korr,dim_z*a+ dim_z*korr);
                  glVertex3d(dim_x*b+-dim_x*korr,dim_y*L+ dim_y*korr,dim_z*a+ dim_z*korr);
                  glVertex3d(dim_x*b+-dim_x*korr,dim_y*L+ dim_y*korr,dim_z*a+-dim_z*korr);
                  glVertex3d(dim_x*b+ dim_x*korr,dim_y*L+ dim_y*korr,dim_z*a+-dim_z*korr);
                  glVertex3d(dim_x*b+ dim_x*korr,dim_y*L+-dim_y*korr,dim_z*a+-dim_z*korr);
                  glVertex3d(dim_x*b+ dim_x*korr,dim_y*L+-dim_y*korr,dim_z*a+ dim_z*korr);

                glEnd();
                glBegin(GL_TRIANGLE_FAN);
                  glVertex3d(dim_x*b+-dim_x*korr,dim_y*L+-dim_y*korr,dim_z*a+-dim_z*korr);
                  glVertex3d(dim_x*b+ dim_x*korr,dim_y*L+-dim_y*korr,dim_z*a+-dim_z*korr);
                  glVertex3d(dim_x*b+ dim_x*korr,dim_y*L+-dim_y*korr,dim_z*a+ dim_z*korr);
                  glVertex3d(dim_x*b+-dim_x*korr,dim_y*L+-dim_y*korr,dim_z*a+ dim_z*korr);
                  glVertex3d(dim_x*b+-dim_x*korr,dim_y*L+ dim_y*korr,dim_z*a+ dim_z*korr);
                  glVertex3d(dim_x*b+-dim_x*korr,dim_y*L+ dim_y*korr,dim_z*a+-dim_z*korr);
                  glVertex3d(dim_x*b+ dim_x*korr,dim_y*L+ dim_y*korr,dim_z*a+-dim_z*korr);
                  glVertex3d(dim_x*b+ dim_x*korr,dim_y*L+-dim_y*korr,dim_z*a+-dim_z*korr);
                glEnd();
            }
          }
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
GL_Ansicht::netzeAuffrischen()
{
  DBG_PROG_START
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


  if (gl_listen[RASTER]) {
    glDeleteLists (gl_listen[RASTER], 1);
    gl_listen[RASTER] = 0;
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
      glTranslated( -b_darstellungs_breite/2, -.5, -a_darstellungs_breite/2 );

      
      DBG_ICCGL_V( netz.indexe.size() <<" "<< netz.punkte.size() )


      glLineWidth(strich1*strichmult);
      int index[7];
      double normale[3], len=1.0;

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
            glVertex3d( netz.punkte[index[0]].koord[2]+normale[2]/len*.1,
                        netz.punkte[index[0]].koord[0]+normale[0]/len*.1,
                        netz.punkte[index[0]].koord[1]+normale[1]/len*.1 );
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

      glPopMatrix();

      glDisable (GL_BLEND);
      glDisable( GL_COLOR_MATERIAL );
      glDepthMask(GL_TRUE);
#     ifndef Beleuchtung
      glEnable(GL_LIGHTING);
#     endif

  DBG_PROG_ENDE
}


void
GL_Ansicht::iccPoint3d               ( oyPROFILE_e         projection,
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

void
GL_Ansicht::punkteAuffrischen()
{ DBG_PROG_START

  if (gl_listen[PUNKTE]) {
    glDeleteLists (gl_listen[PUNKTE], 1);
    gl_listen[PUNKTE] = 0;
  }

  //coordinates  in CIE*b CIE*L CIE*a 
  if (oyNamedColours_Count( colours_ )) {
    if( oyNamedColours_Count( colours_ ) )
      DBG_PROG_V( oyNamedColours_Count( colours_ ) )

    gl_listen[PUNKTE] = glGenLists(1);
    glNewList( gl_listen[PUNKTE], GL_COMPILE); DBG_PROG_V( gl_listen[PUNKTE] )
#     ifndef Beleuchtung_
      glDisable(GL_LIGHTING);
#     endif

#     if 1
      glDisable (GL_BLEND);
      //glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
      glDisable/*Enable*/ (GL_ALPHA_TEST_FUNC);
      //glAlphaFunc (GL_ALPHA_TEST, GL_ONE_MINUS_DST_ALPHA);
#     else
#     endif

      //glColor3f(0.9, 0.9, 0.9);
      glPushMatrix();
        // positioning
        glTranslated( -b_darstellungs_breite/2,-.5,-a_darstellungs_breite/2 );

        int x = this->x() + this->w()/2;
        int y = this->y() + this->h()/2;
        char * disp_name = oyGetDisplayNameFromPosition( 0, x,y, malloc );
        oyProfile_s * disp_prof = 0;
        if(disp_name)
        {
          char * moni_profile_name = oyGetMonitorProfileName(disp_name, malloc);
          if(moni_profile_name)
          {
            disp_prof = oyProfile_FromFile( moni_profile_name, 0, 0 );

          } else {

            size_t size = 0;
            char * buf = oyGetMonitorProfile( disp_name, &size, malloc );
            if(size && buf)
            {
              disp_prof = oyProfile_FromMem( size, buf, 0, 0 );
              free(buf); size = 0;
            }
          }
          free(disp_name);
        }

        int n = oyNamedColours_Count( colours_ );
        if(zeig_punkte_als_paare)
          n /= 2;
        oyNamedColour_s * c = NULL;
        double lab[3];
        double rgba[4] = {0,0,0,1};
        oyPROFILE_e projection = oyEDITING_LAB;
        if(zeig_punkte_als_paare)
          for (int i = 0; i < n; ++i) /* 6 */
          {
            glLineWidth(strich2*strichmult);
            glBegin(GL_LINES);
              c = oyNamedColours_GetRef( colours_, i );
              if(!zeig_punkte_als_messwerte)
                glColor4d(.97, .97, .97, 1. );
              else {
                oyNamedColour_GetColour( c, disp_prof, rgba, oyDOUBLE, 0 );
                glColor4dv( rgba );
              }
              oyNamedColour_GetColourStd( c, projection, lab, oyDOUBLE, 0 );
              iccPoint3d ( projection, lab, 0 );
              oyNamedColour_Release( &c );

              c = oyNamedColours_GetRef( colours_, n + i );
              if(!zeig_punkte_als_messwerte)
                glColor4d(1., .6, .6, 1.0 );
              else {
                oyNamedColour_GetColour( c, disp_prof, rgba, oyDOUBLE, 0 );
                glColor4dv( rgba );
              }

              oyNamedColour_GetColourStd( c, projection, lab, oyDOUBLE, 0 );
              iccPoint3d ( projection, lab, 0 );
              oyNamedColour_Release( &c );
            glEnd();
          }

        GLUquadricObj *quad;
        quad = gluNewQuadric();
        //gluQuadricTexture( quad, GL_TRUE );
        double rad = .02;
        int dim = 12;
        int kugeln_zeichnen = false;
        switch (punktform)
        {
          case MENU_dE1STERN:
             glPointSize(punktgroesse);
             glColor4d(.97, .97, .97, 1. );
             glBegin(GL_POINTS);
               for (int i = 0; i < n; ++i)
               {
                   c = oyNamedColours_GetRef( colours_, i );
                   oyNamedColour_GetColour( c, disp_prof, rgba, oyDOUBLE, 0 );
                   glColor4dv( rgba );
                   oyNamedColour_GetColourStd( c, projection, lab,
                                               oyDOUBLE, 0 );
                   iccPoint3d ( projection, lab, 0 );
                   oyNamedColour_Release( &c );
               }
             glEnd();
             // shadow
             glColor4f( schatten, schatten, schatten, 1. );
             glPointSize((punktgroesse/2-1)>0?(punktgroesse/2-1):1);
             glBegin(GL_POINTS);
               for (int i = 0; i < n; ++i)
               {
                 c = oyNamedColours_GetRef( colours_, i );
                 oyNamedColour_GetColourStd( c, projection, lab,
                                             oyDOUBLE, 0 );
                 lab[0] = 0;
                 iccPoint3d ( projection, lab, 0 );
                 oyNamedColour_Release( &c );
               }
             glEnd();
            break;
          case MENU_dE1KUGEL: rad = 0.005;dim = 5; kugeln_zeichnen = true;break;
          case MENU_dE2KUGEL: rad = 0.01; dim = 8; kugeln_zeichnen = true;break;
          case MENU_dE4KUGEL: rad = 0.02; dim =12; kugeln_zeichnen = true;break;
               break;
          case MENU_DIFFERENZ_LINIE: // they are drawn anyway
               break;
        }

        if(kugeln_zeichnen)
        {
          if(zeig_punkte_als_paare)
            n *= 2; 
            for (int i = 0; i < n; ++i)
            {
                 glPushMatrix();
                   c = oyNamedColours_GetRef( colours_, i );
                   oyNamedColour_GetColour( c, disp_prof, rgba, oyDOUBLE, 0);
                   glColor4dv( rgba );
                   oyNamedColour_GetColourStd( c, projection, lab,
                                               oyDOUBLE, 0 );
                   iccPoint3d( projection, lab, rad );
                   oyNamedColour_Release( &c );
                 glPopMatrix();
            }
        }
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
GL_Ansicht::zeigeUmrisse_()
{
  DBG_PROG_START

  for( unsigned int j = 0; j < dreiecks_netze.size(); j++ )
  {
    double schattierung = .93 - .8/dreiecks_netze.size()*j;
    dreiecks_netze[j].schattierung = schattierung;
  }

  if (gl_listen[UMRISSE]) {
    DBG_PROG_S( "delete glListe " << gl_listen[UMRISSE] )
    glDeleteLists (gl_listen[UMRISSE],1);
    gl_listen[UMRISSE] = 0;
  }

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

    RGB_Speicher = icc_oyranos.wandelLabNachBildschirmFarben( x(),y(),
                                 Lab_Speicher,
                                 (size_t)n, icc_examin->intentGet(NULL), 0);
    DBG_PROG_V( n )
    // create shadow
    Lab_Speicher_schatten = (double*) malloc (sizeof(double) * n*3);

    for (int i = 0; i < n; ++i) {
      Lab_Speicher_schatten[i*3+0] = dreiecks_netze[d].schattierung; //hintergrundfarbe*.40+.35;
      Lab_Speicher_schatten[i*3+1] = (Lab_Speicher[i*3+1]-.5)*.25+0.5;
      Lab_Speicher_schatten[i*3+2] = (Lab_Speicher[i*3+2]-.5)*.25+0.5;
    }

    RGBSchatten_Speicher = icc_oyranos.wandelLabNachBildschirmFarben( x(),y(),
                             Lab_Speicher_schatten, n, icc_examin->intentGet(NULL), 0);
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
    GLfloat farbe[] =   { pfeilfarbe[0],pfeilfarbe[1],pfeilfarbe[2], 1.0 };

  gl_listen[UMRISSE] = glGenLists(1);
  glNewList( gl_listen[UMRISSE], GL_COMPILE );
  DBG_PROG_V( gl_listen[UMRISSE] ) 

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
         dreiecks_netze[i].aktiv &&
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
            FARBE(dreiecks_netze[i].umriss[z].farbe[0],
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
        if(!dreiecks_netze[i].schattierung)
          ;//netzeAuffrischen();
        DBG_PROG_V( n )
        glLineWidth(strich2*strichmult);

        FARBE (          dreiecks_netze[i].schattierung,
                         dreiecks_netze[i].schattierung,
                         dreiecks_netze[i].schattierung,1);

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
GL_Ansicht::zeigeSpektralband_()
{
  DBG_PROG_START

  if (gl_listen[SPEKTRUM])
    glDeleteLists (gl_listen[SPEKTRUM], 1);
  gl_listen[SPEKTRUM] = 0;
  if (spektralband == MENU_SPEKTRALBAND)
  {
    double *RGB_Speicher = 0,
           *RGBSchatten_Speicher = 0,
           *XYZ_Speicher = 0,
           *Lab_Speicher = 0,
           *Lab_Speicher_schatten = 0;
    int n_punkte = 471;//341; // 700 nm

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

    RGB_Speicher = icc_oyranos.wandelLabNachBildschirmFarben( x(),y(),
                                 Lab_Speicher,
                                 (size_t)n_punkte, icc_examin->intentGet(NULL), 0);

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

    RGBSchatten_Speicher = icc_oyranos.wandelLabNachBildschirmFarben( x(),y(),
                               Lab_Speicher_schatten, n_punkte, icc_examin->intentGet(NULL), 0);
    if(!RGB_Speicher)  WARN_S( "RGB_speicher result not available" )
    if(!RGBSchatten_Speicher)  WARN_S( "RGB_speicher result not available" )

    GLfloat farbe[] =   { pfeilfarbe[0],pfeilfarbe[1],pfeilfarbe[2], 1.0 };

    gl_listen[SPEKTRUM] = glGenLists(1);
    glNewList( gl_listen[SPEKTRUM], GL_COMPILE );
    DBG_PROG_V( gl_listen[SPEKTRUM] ) 

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
GL_Ansicht::menueErneuern_()
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

  if (von_farb_namen_.size() >= 3)
  {
    static char text_L[64];
    static char text_a[64];
    static char text_b[64];
    static char text_S[64];
    sprintf (text_L, "%s %s", von_farb_namen_[0].c_str(), _("Slice"));
    menue_schnitt_->replace( 0, text_L);
    sprintf (text_a, "%s %s", von_farb_namen_[1].c_str(), _("Slice"));
    menue_schnitt_->replace( 1, text_a);
    sprintf (text_b, "%s %s", von_farb_namen_[2].c_str(), _("Slice"));
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
GL_Ansicht::menueInit_()
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
GL_Ansicht::setzePerspektive()
{ //DBG_ICCGL_START
    if (agv_->duenn)
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
GL_Ansicht::fensterForm( )
{ DBG_PROG_START
  if(visible()) {
    glViewport(0,0,w(),h());
    DBG_PROG_V( x()<<" "<< y()<<" "<<w()<<" "<<h())
    seitenverhaeltnis = (GLdouble)w()/(GLdouble)h();
  }
  DBG_PROG_ENDE
}

void
GL_Ansicht::zeichnen()
{
  DBG_ICCGL_START
# if 0
  int thread = wandelThreadId(iccThreadSelf());
  if(thread != THREAD_HAUPT) {
    WARN_S( "falscher Thread: " << dbgThreadId(iccThreadSelf()) );
    DBG_PROG_ENDE
  }
# endif

  if(!valid_) {
    // complete initialisation
    //init(id_);

    GLinit_();  DBG_PROG
    fensterForm();
    int err = auffrischen_();
    if(!err)
      valid_ = true;
    else
      return;
  }

  if(!frei()) return;
  MARK( frei(false); )

  zeit_ = icc_examin_ns::zeitSekunden();

  // actualise shodow
  static char aktive[64];
  static char grau[64];
  char aktualisiert = false;
  for(int i = 0; i < (int)dreiecks_netze.size(); ++i)
    if( dreiecks_netze[i].aktiv != aktive[i] ||
        dreiecks_netze[i].grau != grau[i]  ) {
      aktive[i] = dreiecks_netze[i].aktiv;
      grau[i] = dreiecks_netze[i].grau;
      if(!aktualisiert) {
        zeigeUmrisse_();
        punkteAuffrischen();
        aktualisiert = true;
      }
    }

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

      GL_Ansicht::setzePerspektive();

      /* so this replaces gluLookAt or equiv */
      agv_->agvViewTransform();

      glMatrixMode(GL_MODELVIEW);
      glLoadIdentity();

      glCallList( gl_listen[SPEKTRUM] ); DBG_ICCGL_V( gl_listen[SPEKTRUM] )
      glCallList( gl_listen[UMRISSE] ); DBG_ICCGL_V( gl_listen[UMRISSE] )
      if (zeige_helfer) {
        glCallList( gl_listen[HELFER] ); DBG_ICCGL_V( gl_listen[HELFER] )
      }
      glCallList( gl_listen[RASTER] ); DBG_ICCGL_V( gl_listen[RASTER] )
      if(punktform == MENU_dE1STERN)
        glCallList( gl_listen[PUNKTE] );
      glCallList( gl_listen[PUNKTE] ); DBG_ICCGL_V( gl_listen[PUNKTE] )


      // localisate
      if( epoint_ && Fl::belowmouse() != this )
      {
        double l[3];
        Lab_s lab;
        const char * temp = oyNamedColour_GetName( epoint_, oyNAME_NICK, 0 );
        if(temp)
          sprintf( text, "%s", temp );
        oyNamedColour_GetColourStd( epoint_, oyEDITING_LAB, l, oyDOUBLE, 0 );
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
                                       (oyPointer)d, oyDOUBLE, 0 );
          benachrichtigen( ICCexamin::GL_MOUSE_HIT3D );
          if(epoint_)
          {
            Lab_s lab;
            oyNamedColour_GetColourStd( epoint_, oyEDITING_LAB, d, oyDOUBLE, 0);
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
            sprintf( text,"%s:%.02f %s:%.02f %s:%.02f",
                               von_farb_namen_[0].c_str(), d[0],
                               von_farb_namen_[1].c_str(), d[1],
                               von_farb_namen_[2].c_str(), d[2] );
          }
        } else {
          if(!epoint_)
            ; //epoint_.name = (char*) myAllocFunc(1);
        }
      }

      if( (strlen(text) || epoint_) &&
          typ() != 1 )
      {
        double lab[3] = {oY+0.5, oZ/2.55+0.5, oX/2.55+0.5},
              *rgb_ = 0, *rgb;

        icc_examin->statusFarbe(lab[0],lab[1],lab[2]);
          DBG_PROG_V( lab[0]<<" "<<lab[1]<<" "<<lab[2] )
        /*if(epoint_ && epoint_->sig == icSigRgbData)
          rgb = &epoint_->channels[0];
        else*/
          rgb_ = rgb = icc_oyranos.wandelLabNachBildschirmFarben( x(),y(),lab,
                                 1, icc_examin->intentGet(NULL),
                                 icc_examin->gamutwarn()?cmsFLAGS_GAMUTCHECK:0);
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
        if(rgb_) delete [] rgb_;
      }

      if(dreiecks_netze.size())
        netzeAuffrischen();

    glPopMatrix();
    // End of drawing

    // Text
    FARBE(textfarbe[0],textfarbe[1],textfarbe[2],1)

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
            FARBE(f,f,f,1)

            // text above scene
            glLoadIdentity();
            glMatrixMode(GL_PROJECTION);

            glLoadIdentity();
            glOrtho(0,w(),0,h(),-10.0,10.0);

            glRasterPos3d (X, Y, 9.999);
                      
            if(typ() == 1) {
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
                   sprintf( text,"%s[%d][%d][%d]: ", _("Pos"),
                            (int)((oY+0.5)*tabelle_.size()),
                            (int)((oZ+0.5)*tabelle_[L].size()),
                            (int)((oX+0.5)*tabelle_[L][a].size()));
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
            } else {
              if(!epoint_)
                glStatus( text, typ_ );
              ZeichneOText (ortho_font, scal, text)
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
      if(smooth)
      {
        smooth = 0;
        strich_neu = true;
      }
    }
    if(dzeit < 1./40.)
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

       FARBE(textfarbe[0],textfarbe[1],textfarbe[2],1)

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
           glRasterPos3d(0,ortho_font->LineHeight()/5+20,9.99);
#        endif
         ZeichneOText (ortho_font, scal, kanalname.c_str()) 
       } else {
         for (unsigned int i=0;
                i < dreiecks_netze.size();
                  ++i)
         {
           std::string text;
           text = dreiecks_netze[i].name;
           DBG_PROG_V( dreiecks_netze[i].name )
#          ifdef HAVE_FTGL
           if(ortho_font)
             glRasterPos2d(0, 20 + ortho_font->LineHeight()
                                   / 1.5 * (dreiecks_netze.size()-i-1) );
#          endif
           if(dreiecks_netze[i].aktiv)
             ZeichneOText (ortho_font, scal, text.c_str())
         }
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


  } else
    if(!icc_examin->frei())
      DBG_PROG_S("icc_examin not free")
  icc_examin->report_owner = 0;

  MARK( frei(true); )

  DBG_ICCGL_ENDE
}

void
GL_Ansicht::achsNamen    (std::vector<std::string> achs_namen)
{ DBG_PROG_START
  if (achs_namen.size() == 3)
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

oyNamedColours_s*
GL_Ansicht::namedColours       ()
{
  oyNamedColours_s * colours;

  DBG_PROG_START
  colours = oyNamedColours_Copy( colours_, NULL );

  DBG_PROG_ENDE
  return colours;
}

void
GL_Ansicht::namedColours       (oyNamedColours_s       * colours)
{
  DBG_PROG_START
  MARK( frei(false); )
             oyNamedColours_Release( &colours_ );
  colours_ = oyNamedColours_Copy( colours, NULL );

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
GL_Ansicht::namedColoursRelease()
{
  frei(false); 
  oyNamedColours_Release( &colours_ );
  oyNamedColour_Release( &mouse_3D_hit );
  frei(true);
}


#if 0
void
GL_Ansicht::hineinPunkte       (std::vector<double>      &vect,
                                std::vector<std::string> &achs_namen)
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
GL_Ansicht::hineinPunkte      (std::vector<double>      &vect,
                               std::vector<float>       &punkt_farben,
                               std::vector<std::string> &achsNamen)
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
GL_Ansicht::hineinPunkte      (std::vector<double> &vect,
                               std::vector<float>  &punkt_farben,
                               std::vector<std::string> &farb_namen,
                               std::vector<std::string> &achs_namen)
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
GL_Ansicht::emphasizePoint    (oyNamedColour_s * colour)
{ DBG_PROG_START
  // show curve from tag_browser
  MARK( frei(false); )
  if(colour)
  {
    oyNamedColour_Release( &epoint_ );
    epoint_ = oyNamedColour_Copy( colour, 0 );

    double l[3];
    Lab_s lab;
    oyNamedColour_GetColourStd( colour, oyEDITING_LAB, l, oyDOUBLE, 0 );
    CIELabToLab( l, lab );

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

void
GL_Ansicht::hineinNetze       (const std::vector<ICCnetz> & d_n)
{
  DBG_PROG_START

  MARK( frei(false); )
  if(d_n.size()) {
    DBG_NUM_V( dreiecks_netze.size() )
    //dreiecks_netze = d_n;
    netz.clear();
    for(unsigned i = 0; i < d_n.size(); ++i)
      netz.insert( d_n[i] );
    for(unsigned k = 0; k < netz.punkte.size(); ++k)
    {
      netz.punkte[k].koord[1] *= a_darstellungs_breite;
      netz.punkte[k].koord[2] *= b_darstellungs_breite;
    }

  } else
    dreiecks_netze.resize(0);
  MARK( frei(true); )

  DBG_NUM_V( dreiecks_netze.size() )
  for(unsigned i = 0; i < dreiecks_netze.size(); ++i)
    DBG_NUM_V( dreiecks_netze[i].name );

  valid_=false;
  redraw();
  DBG_PROG_ENDE
}


void
GL_Ansicht::hineinTabelle (std::vector<std::vector<std::vector<std::vector<double> > > > vect,
                           std::vector<std::string> achs_namen,
                           std::vector<std::string> nach)
{ DBG_PROG_START

  MARK( frei(false); )
  tabelle_ = vect;  DBG_PROG
  nach_farb_namen_ = nach; DBG_PROG

  achsNamen(achs_namen);

  MARK( frei(true); )

  namedColoursRelease();

  valid_=false;
  redraw();

  glStatus(_("left-/middle-/right mouse button -> rotate/cut/menu"), typ_);

  DBG_PROG_ENDE
}


void
GL_Ansicht::menueAufruf ( int value )
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
      break;
    case MENU_FARBIG:
      punktfarbe = MENU_FARBIG;
      break;
    case MENU_KONTRASTREICH:
      punktfarbe = MENU_KONTRASTREICH;
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
      for (int i=0; i < 3 ; ++i) pfeilfarbe[i] = (float)(1.*farb_faktor);
      for (int i=0; i < 3 ; ++i) textfarbe[i] = (float)(.75*farb_faktor);
      break;
    case MENU_HELLGRAU:
      hintergrundfarbe = 0.75;//MENU_HELLGRAU;
      for (int i=0; i < 3 ; ++i) pfeilfarbe[i] = (float)(1.0*farb_faktor);
      for (int i=0; i < 3 ; ++i) textfarbe[i] = (float)(0.5*farb_faktor);
      break;
    case MENU_GRAUGRAU:
      hintergrundfarbe = 0.5;//MENU_GRAUGRAU;
      for (int i=0; i < 3 ; ++i) pfeilfarbe[i] = (float)(.75*farb_faktor);
      for (int i=0; i < 3 ; ++i) textfarbe[i] = (float)(0.25*farb_faktor);
      break;
    case MENU_DUNKELGRAU:
      hintergrundfarbe = 0.25;//MENU_DUNKELGRAU;
      for (int i=0; i < 3 ; ++i) pfeilfarbe[i] = (float)(0.5*farb_faktor);
      for (int i=0; i < 3 ; ++i) textfarbe[i] = (float)(0.75*farb_faktor);
      break;
    case MENU_SCHWARZ:
      hintergrundfarbe = 0.0;//MENU_SCHWARZ;
      for (int i=0; i < 3 ; ++i) pfeilfarbe[i] = (float)(.25*farb_faktor);
      for (int i=0; i < 3 ; ++i) textfarbe[i] = (float)(0.5*farb_faktor);
      break;
    case Agviewer::FLYING:
      glStatus(_("left mouse button -> go back"), typ_);
      agv_->duenn = true;
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
      agv_->duenn = true;
      break;
    case Agviewer::ICCFLY_a:
      vorder_schnitt = std_vorder_schnitt;
      glStatus(_("left mouse button -> go back"), typ_);
      agv_->duenn = true;
      break;
    case Agviewer::ICCFLY_b:
      vorder_schnitt = std_vorder_schnitt;
      glStatus(_("left mouse button -> go back"), typ_);
      agv_->duenn = true;
      break;
    case Agviewer::ICCPOLAR:
      agv_->duenn = true;
    case Agviewer::POLAR:
      if(typ() == 1)
        agv_->duenn = true;
      else
        agv_->duenn = false;
      break;
    case Agviewer::AGV_STOP:
      agv_->duenn = false;
      glStatus(_("left-/middle-/right mouse button -> rotate/cut/menu"), typ_);
      break;
    }
  }

  if(visible()) {
    icc_examin->alle_gl_fenster->benachrichtigen(ICCexamin::GL_AUFFRISCHEN);
  }

  if(value >= 100)
    agv_->agvSwitchMoveMode (value);

  DBG_PROG_V( value<<" "<<id_ )
  DBG_PROG_ENDE
}


#define DBG_BUTTON 1
#if DBG_BUTTON
# define DBG_BUTTON_S(text) DBG_NUM_S(text)
#else
# define DBG_BUTTON_S(text)
#endif

int
GL_Ansicht::handle( int event )
{
  DBG_ICCGL_START
  int mausknopf = Fl::event_state();
  int schluss = 1;
  DBG_MEM_V( dbgFltkEvent(event) )

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
         agv_->agvHandleButton( mausknopf, event, Fl::event_x(),Fl::event_y());
         DBG_BUTTON_S( "FL_PUSH bei: " << Fl::event_x() << "," << Fl::event_y() )
         break;
    case FL_RELEASE:
         agv_->agvHandleButton(Fl::event_state(),event, Fl::event_x(),Fl::event_y());
         DBG_BUTTON_S( "FL_RELEASE bei: " << Fl::event_x() << "," << Fl::event_y() )
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
         break;
    case FL_LEAVE:
         DBG_BUTTON_S( dbgFltkEvent(event) )
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

void
GL_Ansicht::tastatur(int e)
{ DBG_MEM_START
  Fl_Widget * wb = Fl::belowmouse();
  if(visible())
  if( this == wb )
  {
    //e = Fl::get_key(e);
    DBG_ICCGL_S("e = " << e << " " << Fl::event_key() )
    if(e == FL_SHORTCUT)
    {
      if(Fl::event_key() == FL_Up) {
        vorder_schnitt += 0.01;
      } else if(Fl::event_key() == FL_Down) {
        vorder_schnitt -= 0.01;
      } else if(Fl::event_key() == FL_Home) {
        vorder_schnitt = 4.2;
      } else if(Fl::event_key() == FL_End) {
        vorder_schnitt = agv_->eyeDist();
      }
      switch (Fl::event_key()) {
      case 45: // '-' this is not conform on all consoles; use FLTK native key codes?
        if(punktform >= MENU_dE1KUGEL && punktform <= MENU_dE4KUGEL)
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
        break;
      case 43: // '+'
        if (punktform >= MENU_dE1KUGEL && punktform <= MENU_dE4KUGEL)
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
        break;
      default:
        dbgFltkEvents(e);
        DBG_MEM_V( Fl::event_key() )
      }
    }
  }
  DBG_MEM_ENDE
}


void
GL_Ansicht::c_ ( Fl_Widget* w, void* daten )
{ DBG_ICCGL_START

  intptr_t value = (intptr_t) daten;
  DBG_PROG_V( value )

  GL_Ansicht *gl_obj = dynamic_cast<GL_Ansicht*>(w->parent());
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
GL_Ansicht::hintergrundfarbeZuMenueeintrag( float farbe )
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
#undef implementGlutFunktionen

