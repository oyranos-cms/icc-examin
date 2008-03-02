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


#include "icc_examin.h"
#include "icc_formeln.h"
#include "icc_oyranos.h"
#include "icc_utils.h"
#include "icc_info.h"
#include "agviewer.h"
#include "icc_gl.h"
#include "icc_helfer.h"
#include "config.h"

//#ifdef LINUX
#include <sys/time.h>
//#endif

#include <FL/Fl_Menu_Button.H>
#include <FL/Fl.H>
#include <FL/gl.h>

#ifdef HAVE_FTGL
#include <FTGL/FTFont.h>
#include <FTGL/FTGLTextureFont.h>
#include <FTGL/FTGLPixmapFont.h>
#include <FTGL/FTGLPolygonFont.h>
#include <FTGL/FTGLExtrdFont.h>
#endif

#include <cmath>

#define Beleuchtung
//#define Lab_STERN 1

void zeichneKegel( GLdouble breite, GLdouble hoehe, GLint seiten );
void zeichneKegel( GLdouble breite, GLdouble hoehe, GLint seiten,
                   GLdouble x, GLdouble y, GLdouble z );

//#define DEBUG_ICCGL
#ifdef DEBUG_ICCGL
#define DBG_ICCGL_START DBG_PROG_START
#define DBG_ICCGL_ENDE DBG_PROG_ENDE
#define DBG_ICCGL_V( texte ) DBG_NUM_V( texte )
#define DBG_ICCGL_S( texte ) DBG_NUM_S( texte )
#else
#define DBG_ICCGL_START
#define DBG_ICCGL_ENDE
#define DBG_ICCGL_V( texte )
#define DBG_ICCGL_S( texte )
#endif

typedef enum {NOTALLOWED, AXES, RASTER, PUNKTE, SPEKTRUM, HELFER, UMRISSE, DL_MAX } DisplayLists;
int glListen[DL_MAX+1];


#define bNachX(b) (b*b_darstellungs_breite - b_darstellungs_breite/2.)
#define LNachY(L) (L - 0.5)
#define aNachZ(a) (a*a_darstellungs_breite - a_darstellungs_breite/2.)
#define LabNachXYZv(L,a,b) \
         (b*b_darstellungs_breite - b_darstellungs_breite/2.), \
         (L - 0.5), \
         (a*a_darstellungs_breite - a_darstellungs_breite/2.)

const double GL_Ansicht::std_vorder_schnitt = 4.2;
#ifdef HAVE_FTGL
FTFont *font, *ortho_font;
#endif

// Materialfarben setzen
#define FARBE(r,g,b) {farbe [0] = (r); farbe [1] = (g); farbe [2] = (b); \
                      glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, farbe); \
                      glColor4fv(farbe); }

// Text zeichnen
#ifdef HAVE_FTGL
#  define ZeichneText(Font, Zeiger) { \
   glLineWidth(strichmult); \
    if(blend) glDisable(GL_BLEND); \
      glTranslatef(.0,0,0.01); \
        glScalef(0.002,0.002,0.002); \
          if(Font) Font->Render(Zeiger); \
        glScalef(500,500,500); \
      glTranslatef(.0,0,-.01); \
    if(blend) glEnable(GL_BLEND); \
   glLineWidth(strichmult); }
#else
# define ZeichneText(Font, Zeiger)
#endif

#define ZeichneOText(Font, scal, buffer) { \
                                   glScalef(scal,scal*w()/(double)h(),scal); \
                                   ZeichneText(Font, buffer); \
                                   glScalef(1.0/scal,1.0/(scal*w()/(double)h()),1.0/scal); \
                                 }


GL_Ansicht::GL_Ansicht(int X,int Y,int W,int H)
  : Fl_Gl_Window(X,Y,W,H), agv_(this)
{ DBG_PROG_START
  kanal = 0;
  schnitttiefe = 0.01;
  vorder_schnitt = std_vorder_schnitt;
  a_darstellungs_breite = 1.0;
  b_darstellungs_breite = 1.0;
  schalen = 5;
  punktform = MENU_dE1STERN;
  punktgroesse = 8;
  hintergrundfarbe = MENU_HELLGRAU;
  spektralband = 0;
  zeige_helfer = true;
  zeig_punkte_als_messwert_paare = false;
  zeig_punkte_als_messwerte = false;
  id_ = -1;
  strichmult = 1.0;
  strich1 = 1;
  strich2 = 2;
  strich3 = 3;
  blend = false;
  smooth = false;
  wiederholen = 0;
  maus_x_ = 0;
  maus_y_ = 0;
  darf_bewegen_ = false;
  ist_bewegt_ = false;
  valid_ = false;
  zeit_diff_ = 0;

  for(int i = 0; i <= DL_MAX; ++i)
    glListen[i] = 0;

# ifdef HAVE_FTGL
  font = ortho_font = 0;
# endif

  DBG_PROG_ENDE
}

GL_Ansicht::~GL_Ansicht()
{ DBG_PROG_START
  if (glListen[RASTER]) {
    DBG_PROG_S( "delete glListe " << glListen[RASTER] )
    glDeleteLists (glListen[RASTER],1);
    glListen[RASTER] = 0;
  }
  if (glListen[HELFER]) {
    DBG_PROG_S( "delete glListe " << glListen[HELFER] )
    glDeleteLists (glListen[HELFER],1);
    glListen[HELFER] = 0;
  }
  if (glListen[PUNKTE]) {
    DBG_PROG_S( "delete glListe " << glListen[PUNKTE] )
    glDeleteLists (glListen[PUNKTE],1);
    glListen[PUNKTE] = 0;
  }
  if (glListen[SPEKTRUM]) {
    DBG_PROG_S( "delete glListe " << glListen[SPEKTRUM] )
    glDeleteLists (glListen[SPEKTRUM],1);
    glListen[SPEKTRUM] = 0;
  }
  if (glListen[UMRISSE]) {
    DBG_PROG_S( "delete glListe " << glListen[UMRISSE] )
    glDeleteLists (glListen[UMRISSE],1);
    glListen[UMRISSE] = 0;
  }
# ifdef HAVE_FTGL
  if(font) delete font;
  if(ortho_font) delete ortho_font;
# endif
  DBG_PROG_ENDE
}

void
GL_Ansicht::init(int init_id)
{ DBG_PROG_START

  id_ = init_id;

  DBG_PROG

  resizable(0);

# define TEST_GL(modus) { \
    mode(modus); \
    if(can_do()) { \
      mod |= modus; \
      mode(mod); \
      DBG_PROG_S( "OpenGL versteht: ja   " << #modus <<" "<< mode() ) \
    } else { \
      DBG_PROG_S( "OpenGL versteht: nein " << #modus <<" "<< mode() ) \
    } \
  }

  long mod = 0;
  TEST_GL(FL_RGB)
  TEST_GL(FL_DOUBLE)
  TEST_GL(FL_ALPHA)
  TEST_GL(FL_DEPTH)
  TEST_GL(FL_MULTISAMPLE)
  mode(mod);
  DBG_PROG_S( _("OpenGL mode: ") << mode() )

  DBG_PROG

  agv_.agvInit(id_);

  if (id_ > 1) {
    DBG_PROG_S("gl Fenster " << id_)
    a_darstellungs_breite = 2.55;
    b_darstellungs_breite = 2.55;
    agv_.distA (agv_.distA()
                           + a_darstellungs_breite/2.0 - 0.5);
    agv_.distB (agv_.distB()
                           + b_darstellungs_breite/2.0 - 0.5);
    agv_.eyeDist (agv_.dist()*2.0);
  }

  glListen[AXES] = glGenLists(1);
  agv_.agvMakeAxesList( glListen[AXES] ); DBG_PROG

  gl_font( FL_HELVETICA, 10 );

  // Initialisieren
  menueInit_(); DBG_PROG
  menueAufruf (MENU_HELLGRAU); // Farbschema
  menueAufruf (MENU_GRAU);     // CLUT Farbschema
  schatten = 0.1;
  if (id() == 1) menueAufruf (MENU_WUERFEL);

  DBG_PROG_ENDE
}

/** @brief den Ort lokalisieren */
void
GL_Ansicht::mausPunkt_( GLdouble &oX, GLdouble &oY, GLdouble &oZ,
                        GLdouble &X, GLdouble &Y, GLdouble &Z )
{
  DBG_PROG_START  
  // den Ort lokalisieren
  // wie weit ist das naechste Objekt in diese Richtung, sehr aufwendig
  GLfloat zBuffer;
  glReadPixels((GLint)maus_x_,(GLint)h()-maus_y_,1,1,GL_DEPTH_COMPONENT, GL_FLOAT, &zBuffer);
  GLdouble modell_matrix[16], projektions_matrix[16];
  GLint bildschirm[4];
  glGetDoublev(GL_MODELVIEW_MATRIX, modell_matrix);
  glGetDoublev(GL_PROJECTION_MATRIX, projektions_matrix);
  glGetIntegerv(GL_VIEWPORT, bildschirm);
  gluUnProject(maus_x_, h()-maus_y_, zBuffer,
               modell_matrix, projektions_matrix, bildschirm,
               &oX, &oY, &oZ);


  DBG_PROG_V( "X: "<<oX<<" Y: "<<oY<<" Z: "<<oZ )

  gluProject( oX, oY, oZ,
              modell_matrix, projektions_matrix, bildschirm,
              &X,&Y,&Z);
  DBG_PROG_ENDE
}

void
GL_Ansicht::bewegenStatisch_ (void* gl_a)
{
  DBG_PROG_START
  GL_Ansicht *gl_ansicht = (GL_Ansicht*)gl_a;
  if (!gl_ansicht) {
      WARN_S( "keine GL_Ansicht uebergeben" )
      return;
  }

  static int zahl = 0;
  if(zahl) {
    WARN_S( "zu viele Nutzer: " << zahl )
    return;
  }
  ++zahl;

  bool lauf = true;
  while(lauf)
  {
    if(!gl_ansicht->visible() || !icc_examin->frei() ||
       !gl_ansicht->ist_bewegt_ || !gl_ansicht->darf_bewegen_) {
      gl_ansicht->stupps_(false);
      lauf = false;
      DBG_PROG_S( _("redraw nicht ausgefuehrt") )
    } else {
      gl_ansicht->agv_.agvMove_();
      double zeichnen_schlaf = 0;  // keine endlos Warteschlangen

      if (gl_ansicht->zeit_diff_ < 1./25.) {
        zeichnen_schlaf = 1./25. - gl_ansicht->zeit_diff_;
      }

      gl_ansicht->redraw();
      // kurze Pause 
      Fl::wait(zeichnen_schlaf);
      DBG_PROG_S( "Pause " << zeichnen_schlaf <<" "<< gl_ansicht->ist_bewegt_ )
    }
  }
  
  --zahl;
  DBG_PROG_ENDE
}

void
GL_Ansicht::stupps_ (bool lauf)
{
  DBG_PROG_START
  ist_bewegt_ = lauf;
  if(lauf) {
    DBG_PROG_S( "anstuppsen" )
    if(darf_bewegen_)
      Fl::add_timeout(0.05, bewegenStatisch_,this);
  } else {
    DBG_PROG_S( "stoppen" )
  }
  DBG_PROG_ENDE
}

void
GL_Ansicht::bewegen (bool setze)
{
  DBG_PROG_START
  darf_bewegen_ = setze;
  DBG_PROG_V( setze )
  stupps_(setze);
  if(!setze) {
    agv_.agvSwitchMoveMode (Agviewer::AGV_STOP);
  }
  DBG_PROG_ENDE
}

void
GL_Ansicht::auffrischen_()
{
  DBG_PROG_START
  menueErneuern_();
  erstelleGLListen_();
  DBG_PROG_ENDE
}

void
GL_Ansicht::draw()
{
  DBG_PROG_START
  DBG_PROG_S( "draw() Eintritt ist_bewegt_|darf_bewegen_: "
               << ist_bewegt_ << "|" << darf_bewegen_ )
  Fl_Thread thread = wandelThreadId(pthread_self());
  if(thread != (Fl_Thread)THREAD_HAUPT) {
    WARN_S( ": falscher Thread" );
    DBG_PROG_ENDE
    return;
  }

  if(!visible() || !icc_examin->frei() || !shown()) {
    if(ist_bewegt_) {
      stupps_(false); DBG_NUM_S( "stoppen" )
    } else DBG;
    DBG_PROG_ENDE
    return;
  } else if(!ist_bewegt_ && darf_bewegen_) {
    stupps_(true); DBG_NUM_S( "anstuppsen" )
  } else if(ist_bewegt_ && !darf_bewegen_) {
    stupps_(false); DBG_NUM_S( "stoppen" )
  }

# if 0
  if(!valid()) {
    GLinit_();  DBG_PROG
    fensterForm();
    auffrischen_();
  }

  // aktualisiere Schatten
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
        aktualisiert = true;
      }
    }
# else
  if(!valid())
    valid_ = false;
# endif

  zeichnen();

  DBG_PROG_V( dreiecks_netze.size() )

  DBG_PROG_ENDE
}

#define DBG_BUTTON 0
#if DBG_BUTTON
# define DBG_BUTTON_S(text) DBG_S(text)
#else
# define DBG_BUTTON_S(text)
#endif

int
GL_Ansicht::handle( int event )
{
  DBG_ICCGL_START
  int mausknopf = Fl::event_state();
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
         agv_.agvHandleButton( mausknopf, event, Fl::event_x(),Fl::event_y());
         DBG_BUTTON_S( "FL_PUSH bei: " << Fl::event_x() << "," << Fl::event_y() )
         break;
    case FL_RELEASE:
         agv_.agvHandleButton(Fl::event_state(),event, Fl::event_x(),Fl::event_y());
         DBG_BUTTON_S( "FL_RELEASE bei: " << Fl::event_x() << "," << Fl::event_y() )
         break;
    case FL_DRAG:
         DBG_BUTTON_S( "FL_DRAG bei: " << Fl::event_x() << "," << Fl::event_y() )
         maus_x_ = Fl::event_x();
         maus_y_ = Fl::event_y();
         agv_.agvHandleMotion(maus_x_, maus_y_);
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
         if(visible() && !ist_bewegt_) {
           redraw();
         }
         break;
    case FL_MOUSEWHEEL:
         DBG_BUTTON_S( "FL_MOUSEWHEEL" << Fl::event_dx() << "," << Fl::event_dy() )
         break;
    case FL_LEAVE:
         DBG_BUTTON_S( "FL_LEAVE" )
    case FL_NO_EVENT:
    default:
         DBG_BUTTON_S( "default" )
         DBG_ICCGL_ENDE
         return Fl_Gl_Window::handle(event);
  }
  if(mausknopf)
  {
    switch(Fl::event_state()) {
      case FL_BUTTON1: DBG_BUTTON_S("FL_BUTTON1") break;
      case FL_BUTTON2: DBG_BUTTON_S("FL_BUTTON2") break;
      case FL_BUTTON3: DBG_BUTTON_S("FL_BUTTON3") break;
      default: DBG_BUTTON_S("unbekanner event_state()") break;
    }
  }

  DBG_ICCGL_ENDE
  return 1;
}

void
GL_Ansicht::GLinit_()
{ DBG_PROG_START
  GLfloat mat_ambuse[] = { 0.2, 0.2, 0.2, 1.0 };
  GLfloat mat_specular[] = { 1.0, 1.0, 1.0, 1.0 };

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
  GLfloat light0_position[] = { -2.4, -1.6, -1.2, 0.0 };
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
    WARN_S( _("Konnte Font nicht oeffnen:") << font_name )
    font_name = "/usr/X11R6/lib/X11/fonts/truetype/arial.ttf";
    if(!holeDateiModifikationsZeit(font_name)) {
      WARN_S( _("Konnte Font nicht oeffnen:") << font_name )
      font_name = "/Library/Fonts/Arial.ttf";
      if(!holeDateiModifikationsZeit(font_name)) {
        WARN_S( _("Konnte Font nicht oeffnen:") << font_name )
        char *n = (char*) calloc(sizeof(char), 1024);
        sprintf (n, "%s/.fonts/arial.ttf", getenv("HOME"));
        font_name = n;
        DBG_PROG_V( holeDateiModifikationsZeit(font_name) )
        if(!holeDateiModifikationsZeit(font_name)) {
          WARN_S( _("Konnte Font nicht oeffnen:") << font_name )
          sprintf (n, "%s/fonts/FreeSans.ttf", DATADIR);
          font_name = n;
          if(!holeDateiModifikationsZeit(font_name)) {
            WARN_S( _("Konnte Font nicht oeffnen:") << font_name )
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
    WARN_S( _("Konnte Font nicht oeffnen:") << font_name )
  } else {
    font->CharMap( ft_encoding_unicode );
    font->Depth(12);
    if(!font->FaceSize(72)) WARN_S(_("Fontgroesse nicht setzbar")); \
    ortho_font->CharMap( ft_encoding_unicode );
    if(!ortho_font->FaceSize(16)) WARN_S(_("Fontgroesse nicht setzbar"));
  }
# endif

  glFlush();
  DBG_PROG_ENDE
}

void
GL_Ansicht::tastatur(int e)
{ DBG_ICCGL_START
  if(visible())
  {
    //e = Fl::get_key(e);
    if(Fl::event_key() == FL_Up) {
      vorder_schnitt += 0.01;
    } else if(Fl::event_key() == FL_Down) {
      vorder_schnitt -= 0.01;
    } else if(Fl::event_key() == FL_Home) {
      vorder_schnitt = 4.2;
    } else if(Fl::event_key() == FL_End) {
      vorder_schnitt = agv_.eyeDist();
    }
    DBG_ICCGL_S("e = " << e << " " << Fl::event_key() )
    if(e == FL_SHORTCUT)
    switch (Fl::event_key()) {
      case 45: // '-' Dies ist nicht konform auf allen Tastaturen; benutzt FLTK native Tastaturkodes?
        if(punktform >= MENU_dE1KUGEL || punktform <= MENU_dE4KUGEL)
          if (punktform > MENU_dE1KUGEL) {
            --punktform;
          }
        else if (punktform == MENU_DIFFERENZ_LINIE)
          punktform = MENU_dE1KUGEL;
        else if(punktgroesse > 1) {
          --punktgroesse;
          auffrischen_();
          redraw();
        }
        DBG_PROG_V( Fl::event_key() <<" "<< punktgroesse )
        break;
      case 43: // '+'
        if(punktform >= MENU_dE1KUGEL || punktform <= MENU_dE4KUGEL) {
          if (punktform < MENU_dE4KUGEL) {
            ++punktform;
          }
        else if (punktform == MENU_DIFFERENZ_LINIE)
          punktform = MENU_dE4KUGEL;
        } else
        if(punktgroesse < 21) {
          ++punktgroesse;
          auffrischen_();
          redraw();
        }
        DBG_PROG_V( Fl::event_key()  <<" "<< punktgroesse )
        break;
      default:
        dbgFltkEvents(e);
        DBG_MEM_V( Fl::event_key() )
    }
  }
  DBG_ICCGL_ENDE
}

void
zeichneKegel( GLdouble breite, GLdouble hoehe, GLint seiten )
{ DBG_ICCGL_START
  GLdouble x, y,
           s = 2*M_PI/(GLdouble)seiten, // statische Variable
           hn = breite*tan(breite/2./hoehe); // Normalenhoehe
  // Boden
  glBegin(GL_TRIANGLE_FAN);
    glNormal3d( 0, 0, -1 );
    glVertex3d( 0, 0, 0 );
    for(int i = 0; i <= seiten; ++i)
      glVertex3d( cos(i*s)*breite, sin(i*s)*breite, 0 );
  glEnd();
  // Kegel
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
           s = 2*M_PI/(GLdouble)seiten, // statische Variable
           hn = breite*tan(breite/2./hoehe); // Normalenhoehe
  // Boden
  glBegin(GL_TRIANGLE_FAN);
    glNormal3d( 0, 0, -1 );
    glVertex3d( x, y, z );
    for(int i = 0; i <= seiten; ++i)
      glVertex3d( x+cos(i*s)*breite, y+sin(i*s)*breite, z );
  glEnd();
  // Kegel
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

  // Koordinatenachsen
    glBegin(GL_LINES);
        glVertex3f(.1, 0, 0); glVertex3f(0, 0, 0);
    glEnd();
    glBegin(GL_LINES);
        glVertex3f(0, 0, .1); glVertex3f(0, 0, 0);
    glEnd();
    glBegin(GL_LINES);
        glVertex3f(0, .1, 0); glVertex3f(0, 0, 0);
    glEnd();

    glRotatef (90,0.0,0,1.0);
      glMatrixMode(GL_MODELVIEW);
      glTranslatef((0.0-0.3),(0.0-0.1),(0.0-0.05));
        sprintf (&text[0],_("0,0,0"));
        ZeichneText(font, text)
      glTranslatef(0.3,0.1,0.05);
    glRotatef (-90,0.0,0,1.0);

    FARBE(1,1,1)
    glTranslatef(.1,0,0);
      FARBE(1,0,1)
      glRotatef (90,0.0,1.0,.0);
        zeichneKegel(0.01, 0.025, 8);
      glRotatef (-90,0.0,1.0,.0);
      FARBE(1,1,1)
      glTranslatef(.02,0,0);
        ZeichneText(font, "X")
      glTranslatef((-0.02),0,0);
    glTranslatef((-0.1),0,0);

    glTranslatef(.0,.1,0);
      glRotatef (270,1.0,.0,.0);
        FARBE(1,1,0)
        zeichneKegel(0.01, 0.025, 8);
      glRotatef (90,1.0,.0,.0);
      glRotatef (90,0.0,.0,1.0);
        FARBE(1,1,1)
        ZeichneText(font, "Y")
      glRotatef (270,0.0,.0,1.0);
    glTranslatef(.0,(-0.1),0);

    glTranslatef(0,0,.1);
      FARBE(0,1,1)
      zeichneKegel(0.01, 0.025, 8);
      glRotatef (90,0.0,.5,.0);
        glTranslatef(-.1,0,0);
          FARBE(1,1,1)
          ZeichneText(font, "Z")
        glTranslatef(.1,0,0);
      glRotatef (270,0.0,.5,.0);
    glTranslatef(0,0,-.1);
  DBG_ICCGL_ENDE
}

void
GL_Ansicht::erstelleGLListen_()
{ DBG_PROG_START

  garnieren_();

  tabelleAuffrischen();

  if(dreiecks_netze.size())
    zeigeUmrisse_();
 
  zeigeSpektralband_();

  DBG_PROG_V( punktform <<" "<< MENU_dE1STERN )
  punkteAuffrischen();

  //Hintergrund
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

  DBG_PROG_ENDE
}

void
GL_Ansicht::textGarnieren_()
{
  DBG_PROG_START
  char text[256];
  char* ptr = 0;
  GLfloat ueber = 0.035;
  GLfloat farbe[] =   { 1.0, 1.0, 1.0, 1.0 };

    glDisable(GL_TEXTURE_2D);
    glDisable(GL_LIGHTING);
    FARBE(textfarbe[0],textfarbe[1],textfarbe[2])

    // CIE*L - oben
    glPushMatrix();
      glLoadIdentity();
      if (von_farb_namen_.size())
      {
        ptr = (char*) von_farb_namen_[0].c_str();
        sprintf (&text[0], ptr);
        glRasterPos3f (0, .5+ueber, 0);
        ZeichneOText(ortho_font, 1, text)
      }

    // CIE*a - rechts
      if (von_farb_namen_.size())
      {
        ptr = (char*) von_farb_namen_[1].c_str();
        sprintf (&text[0], ptr);
        if (von_farb_namen_.size() &&
            von_farb_namen_[1] == _("CIE *a"))
          glRasterPos3f (.0, -.5, a_darstellungs_breite/2.+ueber);
        else
          glRasterPos3f (.0, .0, a_darstellungs_breite/2.+ueber);
        ZeichneOText(ortho_font, 1, text)
      }

    // CIE*b - links
      if (von_farb_namen_.size())
      {
        ptr = (char*) von_farb_namen_[2].c_str();
        sprintf (&text[0], ptr);
        if (von_farb_namen_.size() &&
            von_farb_namen_[2] == _("CIE *b"))
          glRasterPos3f (b_darstellungs_breite/2.+ueber, -.5, .0);
        else
          glRasterPos3f (b_darstellungs_breite/2.+ueber, .0, .0);
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
  // Pfeile und Text
  if (glListen[HELFER]) {
    glDeleteLists (glListen[HELFER], 1);
  }

  GL_Ansicht::glListen[HELFER] = glGenLists(1);

  glNewList( glListen[HELFER], GL_COMPILE); DBG_PROG_V( glListen[HELFER] )
    GLfloat farbe[] =   { pfeilfarbe[0],pfeilfarbe[1],pfeilfarbe[2], 1.0 };
    glLineWidth(strich3*strichmult);

    // Farbkanalname
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    if(!smooth) {
      glDisable(GL_BLEND);
      glDisable(GL_LINE_SMOOTH);
    } else {
      glEnable(GL_BLEND);
      glEnable(GL_LINE_SMOOTH);
    }
    glEnable(GL_LIGHTING);

    // CIE*L - oben
    glPushMatrix();
      FARBE(pfeilfarbe[0],pfeilfarbe[1],pfeilfarbe[2])
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

    // CIE*a - rechts
    glPushMatrix();
      if (von_farb_namen_.size() &&
          von_farb_namen_[1] == _("CIE *a"))
        glTranslatef(0,-.5,0);
      FARBE(pfeilfarbe[0],pfeilfarbe[1],pfeilfarbe[2])
      float schritt = .25, Schritt = 1., start, ende;
      start = - floor (a_darstellungs_breite/2./schritt) * schritt;
      ende = floor (a_darstellungs_breite/2./schritt) * schritt;
      // Gitter
      glDisable(GL_LIGHTING);
      for(float i = start; i <= ende; i+=schritt) {

        if(i/Schritt == floor(i/Schritt))
          glLineWidth(strich3*strichmult);
        else
          glLineWidth(strich1*strichmult);

        glBegin(GL_LINES);
          glVertex3f( i, 0,  a_darstellungs_breite/2.);
          glVertex3f( i, 0, -a_darstellungs_breite/2.);
        glEnd();
      }
      glEnable(GL_LIGHTING);

      glTranslatef(0.0,0.0,a_darstellungs_breite/2.);
      glRotatef (180,0.0,.5,.0);
      glTranslatef(.0,0.0,a_darstellungs_breite);
      if (von_farb_namen_.size() &&
          von_farb_namen_[1] == _("CIE *a"))
      {
        FARBE(.2,.9,0.7)
        PFEILSPITZE
      }
      glTranslatef(.0,0.0,-a_darstellungs_breite);
      glRotatef (180,0.0,.5,.0);
      if (von_farb_namen_.size() &&
          von_farb_namen_[1] == _("CIE *a"))
        FARBE(.9,0.2,0.5)
      PFEILSPITZE
    glPopMatrix(); DBG_PROG

    // CIE*b - links
    glPushMatrix();
      if (von_farb_namen_.size() &&
          von_farb_namen_[2] == _("CIE *b"))
        glTranslatef(0,-0.5,0);
      FARBE(pfeilfarbe[0],pfeilfarbe[1],pfeilfarbe[2])
      // Gitter
      glDisable(GL_LIGHTING);
      for(float i = start; i <= ende; i+=schritt) {

        if(i/Schritt == floor(i/Schritt))
          glLineWidth(strich3*strichmult);
        else
          glLineWidth(strich1*strichmult);

        glBegin(GL_LINES);
          glVertex3f( b_darstellungs_breite/2., 0, i);
          glVertex3f(-b_darstellungs_breite/2., 0, i);
        glEnd();
      }
      glEnable(GL_LIGHTING);
      glTranslatef(b_darstellungs_breite/2.,0,0);
      if (von_farb_namen_.size() &&
          von_farb_namen_[2] == _("CIE *b"))
        FARBE(.9,.9,0.2)
      glRotatef (90,0.0,.5,.0);
      PFEILSPITZE
      glRotatef (180,.0,.5,.0);
      glTranslatef(.0,.0,b_darstellungs_breite);
      if (von_farb_namen_.size() &&
          von_farb_namen_[2] == _("CIE *b"))
      {
        FARBE(.7,.8,1.0)
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
  // Kanalauswahl korrigieren
  if(tabelle_.size()) {
    if( (int)tabelle_[0][0][0].size() <= kanal) {
      kanal = tabelle_[0][0][0].size()-1;
      DBG_PROG_S( _("Kanalauswahl geaendert: ") << kanal )
    }
  }

  // Tabelle
  if (glListen[RASTER]) {
    glDeleteLists ( glListen[RASTER], 1);
    glListen[RASTER] = 0;
  }

  if (tabelle_.size())
  {
    glListen[RASTER] = glGenLists(1);
    glNewList( glListen[RASTER], GL_COMPILE); DBG_PROG_V( glListen[RASTER] )
      int n_L = tabelle_.size(), n_a=tabelle_[0].size(), n_b=tabelle_[0][0].size();
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
      float korr = 0.995/2.0;
      glTranslatef(-0.5/0.995+dim_x/2,-0.5/0.995+dim_y/2,-0.5/0.995+dim_z/2);
      for (int L = 0; L < (int)n_L; L++) {
        x = start_x + L * dim_y;
        for (int a = 0; a < (int)n_a; a++)
        {
          y = start_y + a * dim_z;
          for (int b = 0; b < (int)n_b; b++) {
            z = start_z + b * dim_x;
            wert = tabelle_[L][a][b][kanal]; //DBG_PROG_V( L << a << b << kanal )
#           ifdef Beleuchtung_
            FARBE(wert, wert, wert)
            //glColor3f(wert, wert, wert);
#           else
            switch (punktfarbe) {
              case MENU_GRAU:   glColor4f( wert, wert, wert, 1.0); break;
              case MENU_FARBIG: glColor4f((wert*2),
                                           wert*2-1.0,
                                           1.0-wert, 1.0);         break;
              case MENU_KONTRASTREICH: wert = wert * 6;
                                while(wert > 1.0) { wert = wert - 1.0; }
                                if (schalen && wert < 0.80) wert = 0.0;
                                if (0/*schalen*/) {
                                  glDisable(GL_LIGHTING);
                                  FARBE(wert, wert, wert)
                                  glEnable(GL_LIGHTING);
                                } else glColor4f( wert, wert, wert, 1.0);
                                                                   break;
            }
#           endif
            if (wert) {
                glBegin(GL_TRIANGLE_FAN);
                  glVertex3f(dim_x*b+ dim_x*korr,dim_y*L+ dim_y*korr,dim_z*a+ dim_z*korr);
                  glVertex3f(dim_x*b+ dim_x*korr,dim_y*L+-dim_y*korr,dim_z*a+ dim_z*korr);
                  glVertex3f(dim_x*b+-dim_x*korr,dim_y*L+-dim_y*korr,dim_z*a+ dim_z*korr);
                  glVertex3f(dim_x*b+-dim_x*korr,dim_y*L+ dim_y*korr,dim_z*a+ dim_z*korr);
                  glVertex3f(dim_x*b+-dim_x*korr,dim_y*L+ dim_y*korr,dim_z*a+-dim_z*korr);
                  glVertex3f(dim_x*b+ dim_x*korr,dim_y*L+ dim_y*korr,dim_z*a+-dim_z*korr);
                  glVertex3f(dim_x*b+ dim_x*korr,dim_y*L+-dim_y*korr,dim_z*a+-dim_z*korr);
                  glVertex3f(dim_x*b+ dim_x*korr,dim_y*L+-dim_y*korr,dim_z*a+ dim_z*korr);

                glEnd();
                glBegin(GL_TRIANGLE_FAN);
                  glVertex3f(dim_x*b+-dim_x*korr,dim_y*L+-dim_y*korr,dim_z*a+-dim_z*korr);
                  glVertex3f(dim_x*b+ dim_x*korr,dim_y*L+-dim_y*korr,dim_z*a+-dim_z*korr);
                  glVertex3f(dim_x*b+ dim_x*korr,dim_y*L+-dim_y*korr,dim_z*a+ dim_z*korr);
                  glVertex3f(dim_x*b+-dim_x*korr,dim_y*L+-dim_y*korr,dim_z*a+ dim_z*korr);
                  glVertex3f(dim_x*b+-dim_x*korr,dim_y*L+ dim_y*korr,dim_z*a+ dim_z*korr);
                  glVertex3f(dim_x*b+-dim_x*korr,dim_y*L+ dim_y*korr,dim_z*a+-dim_z*korr);
                  glVertex3f(dim_x*b+ dim_x*korr,dim_y*L+ dim_y*korr,dim_z*a+-dim_z*korr);
                  glVertex3f(dim_x*b+ dim_x*korr,dim_y*L+-dim_y*korr,dim_z*a+-dim_z*korr);
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
      DBG_ICCGL_V( dreiecks_netze[0].transparenz )
    }

  unsigned int j,k;

      // Netze sortieren
      glPushMatrix();
         // Die Matrix auf die Kamera ausrichten
       float EyeAz = agv_.eyeAzimuth(),
             EyeEl = agv_.eyeElevation(),
             EyeDist = agv_.eyeDist(),
             X = -EyeDist*sin(TORAD(EyeAz))*cos(TORAD(EyeEl)),  // CIE*b
             Y = EyeDist*sin(TORAD(EyeEl)),                     // CIE*L
             Z = EyeDist*cos(TORAD(EyeAz))*cos(TORAD(EyeEl));   // CIE*a

       float lX = -EyeDist*sin(TORAD(EyeAz-30))*cos(TORAD(EyeEl)),
             lY = Y,
             lZ = EyeDist*cos(TORAD(EyeAz-30))*cos(TORAD(EyeEl));
       glEnable(GL_LIGHTING);
       glEnable(GL_DEPTH_TEST);

       GLfloat lmodel_ambient[] = {0.125+hintergrundfarbe/8,
                                   0.125+hintergrundfarbe/8,
                                   0.125+hintergrundfarbe/8, 1.0};
       glLightModelfv(GL_LIGHT_MODEL_AMBIENT, lmodel_ambient);

       GLfloat light1_position[] = { lX, lY, lZ, 1.0 };
       GLfloat light_ambient[] = {0.1, 0.1, 0.1, 1.0};
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
       glTranslatef(X/1.2,Y/1.2,Z/1.2);
       DBG_ICCGL_S( "X:"<<X<<" Y:"<<Y<<" Z:"<<Z )
       zeichneKoordinaten_();
       glTranslatef(-X/1.2,-Y/1.2,-Z/1.2);
       }
      
       static ICCnetz netz;
       netz.punkte.clear();
       netz.indexe.clear();
       static int punkt_zahl_alt = 0;
       int punkt_zahl = 0;
       for (unsigned int i = 0; i < dreiecks_netze.size(); ++i)
         punkt_zahl += dreiecks_netze[i].punkte.size();
       if(punkt_zahl > punkt_zahl_alt)
         netz.punkte.reserve(punkt_zahl);
       punkt_zahl_alt = punkt_zahl;
       //netz.indexe.reserve(punkt_zahl);
       int punkte_n = 0;
       double abstand;
       for( j = 0; j < dreiecks_netze.size(); j++ )
       {
         dreiecks_netze[j].schattierung = .93 - .8/dreiecks_netze.size()*j;
         if(dreiecks_netze[j].aktiv && dreiecks_netze[j].transparenz)
         {
             // ich hoffe das dauert nicht zu lange
           netz.punkte. insert( netz.punkte.begin()+punkte_n ,
                                dreiecks_netze[j].punkte.begin(),
                                dreiecks_netze[j].punkte.end()    );
             // den Punkten im neuen Netz die Transparenz zuweisen
           for( k = punkte_n; k < netz.punkte.size(); ++k) {
             if(dreiecks_netze[j].grau) {
               netz.punkte[k].farbe[0] = dreiecks_netze[j].schattierung;
               netz.punkte[k].farbe[1] = dreiecks_netze[j].schattierung;
               netz.punkte[k].farbe[2] = dreiecks_netze[j].schattierung;
             }
             netz.punkte[k].farbe[3] = dreiecks_netze[j].transparenz;
             netz.punkte[k].koord[1] *= a_darstellungs_breite;
             netz.punkte[k].koord[2] *= b_darstellungs_breite;
           }

           std::multimap<double,DreiecksIndexe>::const_iterator it;
           for( it = dreiecks_netze[j].indexe.begin();
                it != dreiecks_netze[j].indexe.end(); ++it )
           {
               // die Indexe werden gesondert eingefuegt, um sie neu zu zaehlen
       /*A*/ std::pair<double,DreiecksIndexe> index_p( *it );

       /*B*/ for( k = 0; k < 3; ++k)
               index_p.second.i[k] += punkte_n;
       /*C*/
               // Mittelpunkt des Dreiecks
             double seitenhalbierende[3];
             seitenhalbierende[0] =
                    (  (netz.punkte[index_p.second.i[0]].koord[0]+Y)
                     + (netz.punkte[index_p.second.i[1]].koord[0]+Y))/2.0;
             seitenhalbierende[1] =
                    (  (netz.punkte[index_p.second.i[0]].koord[1]+Z)
                     + (netz.punkte[index_p.second.i[1]].koord[1]+Z))/2.0;
             seitenhalbierende[2] =
                    (  (netz.punkte[index_p.second.i[0]].koord[2]+X)
                     + (netz.punkte[index_p.second.i[1]].koord[2]+X))/2.0;
             double mittelpunkt[3];
             mittelpunkt[0] = (  2.0 * seitenhalbierende[0]
                                + netz.punkte[index_p.second.i[2]].koord[0]+Y)
                              / 3.0;
             mittelpunkt[1] = (  2.0 * seitenhalbierende[1]
                                + netz.punkte[index_p.second.i[2]].koord[1]+Z)
                              / 3.0;
             mittelpunkt[2] = (  2.0 * seitenhalbierende[2]
                                + netz.punkte[index_p.second.i[2]].koord[2]+X)
                              / 3.0;
             abstand = HYP3( mittelpunkt[0], mittelpunkt[1], mittelpunkt[2] );
             index_p.first = abstand;
               // der Behaelter std::map uebernimmt die Sortierung
       /*D*/ netz.indexe.insert(index_p);
           }
             // neue Basis fuer Indexnummern
           punkte_n += dreiecks_netze[j].punkte.size();
         }
       }
      glPopMatrix();


  if (glListen[RASTER]) {
    glDeleteLists (glListen[RASTER], 1);
    glListen[RASTER] = 0;
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
      glPolygonMode(GL_FRONT, GL_FILL);   // Vorderseite als Flaeche 
      glPolygonMode(GL_BACK, GL_LINE);   // Rueckseite als Linien
#     endif
      glFrontFace(GL_CCW);
      glPushMatrix();
        // zurecht setzen
      glTranslatef( -b_darstellungs_breite/2, -.5, -a_darstellungs_breite/2 );

      
      DBG_ICCGL_V( netz.indexe.size() <<" "<< netz.punkte.size() )

#if 1
        std::multimap<double,DreiecksIndexe>::const_iterator it;
        for( it = netz.indexe.begin(); it != netz.indexe.end(); ++it )
        {
          glBegin(GL_TRIANGLES);
          int index;
          double normale[3], len, v1[3], v2[3];
          for( int l = 2; l >= 0; --l)
          {
            index = it->second.i[l];
#           ifdef Beleuchtung
            {
              if(l == 2) {
              // Kreuzprodukt
              v1[0] = netz.punkte[it->second.i[1]].koord[0]-
                      netz.punkte[it->second.i[0]].koord[0];
              v1[1] = netz.punkte[it->second.i[1]].koord[1]-
                      netz.punkte[it->second.i[0]].koord[1];
              v1[2] = netz.punkte[it->second.i[1]].koord[2]-
                      netz.punkte[it->second.i[0]].koord[2];
              v2[0] = netz.punkte[it->second.i[2]].koord[0]-
                      netz.punkte[it->second.i[0]].koord[0];
              v2[1] = netz.punkte[it->second.i[2]].koord[1]-
                      netz.punkte[it->second.i[0]].koord[1];
              v2[2] = netz.punkte[it->second.i[2]].koord[2]-
                      netz.punkte[it->second.i[0]].koord[2];
              // Flächennormale bestimmen
              normale[0] =   v1[2]*v2[1] - v1[1]*v2[2];
              normale[1] =   v1[0]*v2[2] - v1[2]*v2[0];
              normale[2] =   v1[1]*v2[0] - v1[0]*v2[1];
              len = HYP3( normale[0],normale[1],normale[2] );
              // Einheitsvektor der Normale setzen
              glNormal3d( normale[2]/len,
                          normale[0]/len,
                          normale[1]/len );
              }
              GLfloat farbe[] =   { netz.punkte[index].farbe[0],
                                    netz.punkte[index].farbe[1],
                                    netz.punkte[index].farbe[2],
                                    netz.punkte[index].farbe[3] };
 
              FARBE (    netz.punkte[index].farbe[0],
                         netz.punkte[index].farbe[1],
                         netz.punkte[index].farbe[2] );

              // Alle Materialfarben erhalten hier die Transparanz
              glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, farbe);
              //farbe[0] *= 0.5; farbe[1] *= 0.5; farbe[2] *= 0.5;
              glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, farbe);
            }

            if(0) // wird eigentlich in FARBE schon gesetzt
#           endif
              glColor4d( netz.punkte[index].farbe[0],
                         netz.punkte[index].farbe[1],
                         netz.punkte[index].farbe[2],
                         netz.punkte[index].farbe[3] );

            // Punktkoordinaten setzen
            glVertex3d( netz.punkte[index].koord[2],
                        netz.punkte[index].koord[0],
                        netz.punkte[index].koord[1] );
          }
          glEnd();
          //DBG_V( index <<" "<< len <<" "<< normale[0] <<" "<< v1[0] <<" "<< v2[0] );
          if(icc_debug != 0)
          {
          glLineWidth(strich1*strichmult);
          glBegin(GL_LINES);
            glVertex3d( netz.punkte[index].koord[2],
                        netz.punkte[index].koord[0],
                        netz.punkte[index].koord[1] );
            glVertex3d( netz.punkte[index].koord[2]+normale[2]/len*.1,
                        netz.punkte[index].koord[0]+normale[0]/len*.1,
                        netz.punkte[index].koord[1]+normale[1]/len*.1 );
          glEnd();
          }
        }
#else
      for( j = 0; j < dreiecks_netze.size(); j++ )
      {
        std::multimap<double,DreiecksIndexe>::const_iterator it;
        for( it = dreiecks_netze[j].indexe.begin(); it != dreiecks_netze[j].indexe.end(); ++it )
        {
          glBegin(GL_TRIANGLES);
          int index;
          for( int l = 2; l >= 0; --l)
          {
            index = it->second.i[l];
            glColor4d( dreiecks_netze[j].punkte[index].farbe[0],
                       dreiecks_netze[j].punkte[index].farbe[1],
                       dreiecks_netze[j].punkte[index].farbe[2],
                       dreiecks_netze[j].transparenz);

            // Punktkoordinaten setzen
            glVertex3d( dreiecks_netze[j].punkte[index].koord[2]*b_darstellungs_breite,
                        dreiecks_netze[j].punkte[index].koord[0],
                        dreiecks_netze[j].punkte[index].koord[1]*a_darstellungs_breite );
          }
          glEnd();
        }
      }
#endif
      glPopMatrix();

      glDisable (GL_BLEND);
      glDepthMask(GL_TRUE);
#     ifndef Beleuchtung
      glEnable(GL_LIGHTING);
#     endif

  DBG_PROG_ENDE
}


void
GL_Ansicht::punkteAuffrischen()
{ DBG_PROG_START

  if (glListen[PUNKTE]) {
    glDeleteLists (glListen[PUNKTE], 1);
    glListen[PUNKTE] = 0;
  }

  //Koordinaten  in CIE*b CIE*L CIE*a Reihenfolge 
  if (punkte_.size()) {
    if( punkte_.size() )
      DBG_PROG_V( punkte_.size() )
    if( farben_.size() )
      DBG_PROG_V( farben_.size() )
    if( farb_namen_.size() )
      DBG_PROG_V( farb_namen_.size() )

    glListen[PUNKTE] = glGenLists(1);
    glNewList( glListen[PUNKTE], GL_COMPILE); DBG_PROG_V( glListen[PUNKTE] )
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
        // zurecht setzen
        glTranslatef( -b_darstellungs_breite/2,-.5,-a_darstellungs_breite/2 );

        if(zeig_punkte_als_messwert_paare)
          for (unsigned i = 0; i < punkte_.size(); i+=6)
          {
            glLineWidth(strich2*strichmult);
            glColor4f(.97, .97, .97, 1. );
            glBegin(GL_LINES);
              glVertex3f(punkte_[i+2], punkte_[i+0], punkte_[i+1]);
              glColor4f(1., .6, .6, 1.0 );
              glVertex3f(punkte_[i+5], punkte_[i+3], punkte_[i+4] );
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
             glColor4f(.97, .97, .97, 1. );
             glBegin(GL_POINTS);
               for (unsigned i = 0; i < punkte_.size(); i+=3)
               {
                 if (!dreiecks_netze[0].grau && farben_.size())
                   glColor4f(farben_[i/3*4+0], farben_[i/3*4+1],
                             farben_[i/3*4+2], farben_[i/3*4+3] );

                 glVertex3d( punkte_[i+2], punkte_[i+0], punkte_[i+1] );
               }
             glEnd();
             // Schatten
             glColor4f( schatten, schatten, schatten, 1. );
             glPointSize((punktgroesse/2-1)>0?(punktgroesse/2-1):1);
             glBegin(GL_POINTS);
               for (unsigned i = 0; i < punkte_.size(); i+=3)
                 glVertex3d( punkte_[i+2], 0, punkte_[i+1] );
             glEnd();
            break;
          case MENU_dE1KUGEL: rad = 0.005;dim = 5; kugeln_zeichnen = true;break;
          case MENU_dE2KUGEL: rad = 0.01; dim = 8; kugeln_zeichnen = true;break;
          case MENU_dE4KUGEL: rad = 0.02; dim =12; kugeln_zeichnen = true;break;
               break;
          case MENU_DIFFERENZ_LINIE: // die werden sowieso gezeichnet
               break;
        }
        if(kugeln_zeichnen) 
               for (unsigned i = 0; i < punkte_.size(); i+=3) {
                 glPushMatrix();
                   if (farben_.size())
                     glColor4f(farben_[i/3*4+0], farben_[i/3*4+1],
                               farben_[i/3*4+2], farben_[i/3*4+3] );
                   glTranslated( punkte_[i+2], punkte_[i+0], punkte_[i+1] );
                   gluSphere( quad, rad, dim, dim );
                 glPopMatrix();
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

  if (glListen[UMRISSE]) {
    DBG_PROG_S( "delete glListe " << glListen[UMRISSE] )
    glDeleteLists (glListen[UMRISSE],1);
    glListen[UMRISSE] = 0;
  }

  //if (spektralband == MENU_SPEKTRALBAND)
  for (unsigned int d=0; d < dreiecks_netze.size(); ++d)
  {
    double *RGB_Speicher = 0,
           *RGBSchatten_Speicher = 0,
           *Lab_Speicher = 0,
           *Lab_Speicher_schatten = 0;

    // Umrechnung
    int n = dreiecks_netze[d].umriss.size();
    DBG_PROG_V( n )
    if(!n) continue;

    Lab_Speicher = (double*) malloc (sizeof(double) * n*3);
    if(!Lab_Speicher)  WARN_S( _("Lab_speicher Speicher nicht verfuegbar") )

    for ( int j = 0; j < n; ++j)
      for(int k = 0; k < 3 ; ++k)
        Lab_Speicher[j*3+k] = dreiecks_netze[d].umriss[j].koord[k];

    RGB_Speicher = icc_oyranos.wandelLabNachBildschirmFarben( Lab_Speicher,
                                 (size_t)n, icc_examin->intent(), 0);
    DBG_PROG_V( n )
    // Schatten erzeugen
    Lab_Speicher_schatten = (double*) malloc (sizeof(double) * n*3);
    for (int i = 0; i < n*2; ++i)
      Lab_Speicher_schatten[i] = Lab_Speicher[i];
    for (int i = 0; i < n; ++i) {
      Lab_Speicher_schatten[i*3] = hintergrundfarbe*.40+.35;
      Lab_Speicher_schatten[i*3+1] = (Lab_Speicher[i*3+1]-.5)*.25+0.5;
      Lab_Speicher_schatten[i*3+2] = (Lab_Speicher[i*3+2]-.5)*.25+0.5;
    }

    RGBSchatten_Speicher = icc_oyranos.wandelLabNachBildschirmFarben(
                             Lab_Speicher_schatten, n, icc_examin->intent(), 0);
    if(!RGB_Speicher)  WARN_S( _("RGB_speicher Ergebnis nicht verfuegbar") )
    if(!RGBSchatten_Speicher)  WARN_S( _("RGB_speicher Ergebnis nicht verfuegbar") )

    for ( int j = 0; j < n; ++j)
      for(int k = 0; k < 3 ; ++k)
        dreiecks_netze[d].umriss[j].farbe[k] = RGBSchatten_Speicher[j*3+k];

    if (Lab_Speicher) free (Lab_Speicher);
    if (Lab_Speicher_schatten) free (Lab_Speicher_schatten);
    if (RGB_Speicher) delete [] RGB_Speicher;

  }
    GLfloat farbe[] =   { pfeilfarbe[0],pfeilfarbe[1],pfeilfarbe[2], 1.0 };

  glListen[UMRISSE] = glGenLists(1);
  glNewList( glListen[UMRISSE], GL_COMPILE );
  DBG_PROG_V( glListen[UMRISSE] ) 

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
         dreiecks_netze[i].transparenz)
      {
#       if 0
        glLineWidth(6.0*strichmult);
        glColor4f(1., 1.0, 1.0, 1.0);
        glBegin(GL_LINE_STRIP);
        for (int z=0 ; z < (int)dreiecks_netze[i].umriss.size(); z++) {
          glVertex3d( 
         (dreiecks_netze[i].umriss[z*3].koord[2]*b_darstellungs_breite - b_darstellungs_breite/2.),
         (dreiecks_netze[i].umriss[z*3].koord[0] - 0.5),
         (dreiecks_netze[i].umriss[z*3].koord[1]*a_darstellungs_breite - a_darstellungs_breite/2.)
          );
        }
        glEnd();
#       endif
        // Schatten
        int n = dreiecks_netze[i].umriss.size();
        if(!dreiecks_netze[i].schattierung)
          ;//netzeAuffrischen();
        DBG_PROG_V( n )
        glLineWidth(strich2*strichmult);
#       if 1
        GLfloat fv[3] = {dreiecks_netze[i].schattierung,
                         dreiecks_netze[i].schattierung,
                         dreiecks_netze[i].schattierung};
        glColor4fv(fv);
#       else
        FARBE (          dreiecks_netze[i].schattierung,
                         dreiecks_netze[i].schattierung,
                         dreiecks_netze[i].schattierung);
#       endif
        glBegin(GL_LINE_STRIP);
        for (int z=0 ; z < n; z++)
        {
          if(!dreiecks_netze[i].grau) {
            FARBE(dreiecks_netze[i].umriss[z].farbe[0],
                  dreiecks_netze[i].umriss[z].farbe[1],
                  dreiecks_netze[i].umriss[z].farbe[2])
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

  if (glListen[SPEKTRUM])
    glDeleteLists (glListen[SPEKTRUM], 1);
  glListen[SPEKTRUM] = 0;
  if (spektralband == MENU_SPEKTRALBAND)
  {
    double *RGB_Speicher = 0,
           *RGBSchatten_Speicher = 0,
           *XYZ_Speicher = 0,
           *Lab_Speicher = 0,
           *Lab_Speicher_schatten = 0;

    // Spektrumvariablen
    //int nano_min = 63; // 420 nm
    int nano_max = 471;//341; // 700 nm

    // Umrechnung
    XYZ_Speicher = new double [nano_max*3];
    if(!XYZ_Speicher)  WARN_S( _("XYZ_speicher Speicher nicht verfuegbar") )
    for (int i = 0; i < nano_max; ++i)
    { for(int j = 0; j < 3 ; ++j)
      { XYZ_Speicher[i*3+j] = (double)cieXYZ[i][j];
      }
    }
    Lab_Speicher = new double [nano_max*3];
    Lab_Speicher_schatten = new double [nano_max*3];

    if(!Lab_Speicher)  WARN_S( _("Lab_speicher Speicher nicht verfuegbar") )

    XYZtoLab (XYZ_Speicher, Lab_Speicher, nano_max);

    RGB_Speicher = icc_oyranos.wandelLabNachBildschirmFarben( Lab_Speicher,
                                 (size_t)nano_max, icc_examin->intent(), 0);
    DBG_PROG_V( nano_max )
    // Schatten erzeugen
    for (int i = 0; i < nano_max*2; ++i)
      Lab_Speicher_schatten[i] = Lab_Speicher[i];
    for (int i = 0; i < nano_max; ++i) {
      Lab_Speicher_schatten[i*3] = hintergrundfarbe*.40+.35;
      Lab_Speicher_schatten[i*3+1] = (Lab_Speicher[i*3+1]-.5)*.25+0.5;
      Lab_Speicher_schatten[i*3+2] = (Lab_Speicher[i*3+2]-.5)*.25+0.5;
    }

    RGBSchatten_Speicher = icc_oyranos.wandelLabNachBildschirmFarben(
                               Lab_Speicher_schatten, nano_max, icc_examin->intent(), 0);
    if(!RGB_Speicher)  WARN_S( _("RGB_speicher Ergebnis nicht verfuegbar") )
    if(!RGBSchatten_Speicher)  WARN_S( _("RGB_speicher Ergebnis nicht verfuegbar") )

    GLfloat farbe[] =   { pfeilfarbe[0],pfeilfarbe[1],pfeilfarbe[2], 1.0 };

    glListen[SPEKTRUM] = glGenLists(1);
    glNewList( glListen[SPEKTRUM], GL_COMPILE );
    DBG_PROG_V( glListen[SPEKTRUM] ) 

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
        for (int i=0 ; i <= (nano_max - 1); i++) {
          DBG_ICCGL_S( i<<" "<<Lab_Speicher[i*3]<<"|"<<Lab_Speicher[i*3+1]<<"|"<<Lab_Speicher[i*3+2] )
          DBG_ICCGL_S( i<<" "<<RGB_Speicher[i*3]<<"|"<<RGB_Speicher[i*3+1]<<"|"<<RGB_Speicher[i*3+2] )
          FARBE(RGB_Speicher[i*3],RGB_Speicher[i*3+1],RGB_Speicher[i*3+2]);
          glVertex3d( 
         (Lab_Speicher[i*3+2]*b_darstellungs_breite - b_darstellungs_breite/2.),
         (Lab_Speicher[i*3+0] - 0.5),
         (Lab_Speicher[i*3+1]*a_darstellungs_breite - a_darstellungs_breite/2.)
          );
        }
      glEnd();
      // Schatten
      glLineWidth(strich2*strichmult);
      glBegin(GL_LINE_STRIP);
      //#define S * .25 + textfarbe[0] - schatten
        for (int i=0 ; i <= (nano_max - 1); i++) {
          FARBE(RGBSchatten_Speicher[i*3],RGBSchatten_Speicher[i*3+1],RGBSchatten_Speicher[i*3+2])
          glVertex3d( 
         (Lab_Speicher[i*3+2]*b_darstellungs_breite - b_darstellungs_breite/2.),
         (- 0.5),
         (Lab_Speicher[i*3+1]*a_darstellungs_breite - a_darstellungs_breite/2.)
          );
        }
      glEnd();

    glEndList();

    if (XYZ_Speicher) delete [] XYZ_Speicher;
    if (RGB_Speicher) delete [] RGB_Speicher;
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

  // Loeschen
  menue_schnitt_->clear();
  menue_hintergrund_->clear();
  menue_form_->clear();
  menue_->clear();
  menue_button_->clear();
  DBG_PROG

  // ->Querschnitt
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

  // ->Darstellung->Hintergrundfarbe
  menue_hintergrund_->add(_("White"), 0,c_, (void*)MENU_WEISS, 0);
  menue_hintergrund_->add(_("Light gray"), 0,c_, (void*)MENU_HELLGRAU, 0);
  menue_hintergrund_->add(_("Gray"), 0,c_, (void*)MENU_GRAUGRAU, 0);
  menue_hintergrund_->add(_("Dark gray"), 0,c_, (void*)MENU_DUNKELGRAU, 0);
  menue_hintergrund_->add(_("Black"), 0,c_, (void*)MENU_SCHWARZ, 0);
  DBG_PROG

  menue_form_->add (_("Background colour"),0,c_,cpMenueButton(menue_hintergrund_),FL_SUBMENU_POINTER);

  // ->Darstellung
  if(id_ == 1)
  {
    menue_form_->add( _("gray"), 0,c_, (void*)MENU_GRAU, 0 );
    menue_form_->add( _("coloured"), 0,c_, (void*)MENU_FARBIG, 0 );
    menue_form_->add( _("high contrast"), 0,c_, (void*)MENU_KONTRASTREICH, 0 );
    menue_form_->add( _("shells"), 0,c_, (void*)MENU_SCHALEN, 0 );
  } else {
    // Kugeln mit ihrem Radius symbolisieren Messfarben
    if(zeig_punkte_als_messwerte)
    {
      menue_form_->add( _("Sphere 1dE"), 0,c_, (void*)MENU_dE1KUGEL, 0 );
      menue_form_->add( _("Sphere 2dE"), 0,c_, (void*)MENU_dE2KUGEL, 0 );
      menue_form_->add( _("Sphere 4dE"), 0,c_, (void*)MENU_dE4KUGEL, 0 );
    } else {
#   ifdef Lab_STERN
      menue_form_->add( _("Star"), 0,c_, (void*)MENU_dE1STERN, 0 );
#   else
      // Punkte werden fuer Bildfarben reserviert
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


  // -> (Hauptmenue)
  for (int i = 0; i < (int)nach_farb_namen_.size(); i++) {
    char* p = (char*) nach_farb_namen_[i].c_str();
    menue_->add( p, 0,c_, (void*)(MENU_MAX + i), 0 );
    DBG_PROG_V( MENU_MAX + i <<" "<< nach_farb_namen_[i] )
  }

  // TODO: -> oberes Menue
  menue_button_->copy(menue_->menu());
  menue_button_->callback(c_);

  //icc_examin_ns::status_info(_("left-/middle-/right mouse button -> rotate/cut/menu"));

  DBG_PROG_ENDE
}

void
GL_Ansicht::menueInit_()
{
  DBG_PROG_START
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
  DBG_PROG_ENDE
}


/*inline*/ void
GL_Ansicht::setzePerspektive()
{ //DBG_ICCGL_START
    if (agv_.duenn)
      gluPerspective(15, seitenverhaeltnis,
                     vorder_schnitt,
                     vorder_schnitt + schnitttiefe);
    else
      gluPerspective(15, seitenverhaeltnis,
                     vorder_schnitt, 50);
                  // ^-- vordere Schnittfläche
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
  int thread = wandelThreadId(pthread_self());
  if(thread != THREAD_HAUPT) {
    WARN_S( "falscher Thread: " << dbgThreadId(pthread_self()) );
    DBG_PROG_ENDE
  }
# endif
  if(!valid_) {
    GLinit_();  DBG_PROG
    fensterForm();
    auffrischen_();
    valid_ = true;
  }

  double rechen_zeit = icc_examin_ns::zeitSekunden();

  // aktualisiere Schatten
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
  static char text[128] = {""};
  std::string kanalname;

  if(visible() &&
     icc_examin->frei() )
  {
    static bool maus_steht = false;
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

    // Beginn der Zeichnung
    glPushMatrix();

      GL_Ansicht::setzePerspektive();

      /* so this replaces gluLookAt or equiv */
      agv_.agvViewTransform();

      glMatrixMode(GL_MODELVIEW);
      glLoadIdentity();

      glCallList( glListen[SPEKTRUM] ); DBG_ICCGL_V( glListen[SPEKTRUM] )
      glCallList( glListen[UMRISSE] ); DBG_ICCGL_V( glListen[UMRISSE] )
      if (zeige_helfer) {
        glCallList( glListen[HELFER] ); DBG_ICCGL_V( glListen[HELFER] )
      }
      glCallList( glListen[RASTER] ); DBG_ICCGL_V( glListen[RASTER] )
      if(punktform == MENU_dE1STERN)
        glCallList( glListen[PUNKTE] );
      glCallList( glListen[PUNKTE] ); DBG_ICCGL_V( glListen[PUNKTE] )


      // den Ort lokalisieren
      mausPunkt_ (oX, oY, oZ, X, Y, Z);

                    GLfloat grenze = 3.2;
                    if(von_farb_namen_.size() &&
                       -grenze < oY && oY < grenze &&
                       -grenze < oX && oX < grenze &&
                       -grenze < oZ && oZ < grenze)
                      sprintf( text,"%s:%.01f %s:%.01f %s:%.01f",
                               von_farb_namen_[0].c_str(), oY*100+50.,
                               von_farb_namen_[1].c_str(), oZ*100,
                               von_farb_namen_[2].c_str(), oX*100 );
                    else
                      text[0] = 0;

                    if(strlen(text) && id() != 1)
                    {
                        double lab[3] = {oY+0.5, oZ/2.55+0.5, oX/2.55+0.5},
                              *rgb = 0;
                        icc_examin->statusFarbe(lab[0],lab[1],lab[2]);
                        DBG_PROG_V( lab[0]<<" "<<lab[1]<<" "<<lab[2] )
                        rgb = icc_oyranos. wandelLabNachBildschirmFarben(lab, 1,
                                 icc_examin->intent(),
                                 icc_examin->gamutwarn()?cmsFLAGS_GAMUTCHECK:0);
                        if(!rgb)  WARN_S( _("RGB Ergebnis nicht verfuegbar") )
                        else {
#                         ifndef Beleuchtung_
                          glDisable(GL_LIGHTING);
#                         endif
                          glPushMatrix();
                            glLineWidth(strich3*strichmult);
                            FARBE(rgb[0], rgb[1], rgb[2])
                            glBegin(GL_LINES);
                              glVertex3f( oX, oY, oZ );
                              glVertex3f( oX, -0.5, oZ );
                            glEnd();
                          glPopMatrix();
                          DBG_PROG_V( rgb[0] <<" "<< rgb[1] <<" "<< rgb[2] )
#                         ifndef Beleuchtung_
                          glEnable(GL_LIGHTING);
#                         endif
                        }
                    }

      if(dreiecks_netze.size())
        netzeAuffrischen();

    glPopMatrix();
    // Ende der Zeichnung

    // Text
    FARBE(textfarbe[0],textfarbe[1],textfarbe[2])

    GLfloat lmodel_ambient[] = {hintergrundfarbe,
                                hintergrundfarbe,
                                hintergrundfarbe, 1.0};
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, lmodel_ambient);

    if(1)
    {
       glPushMatrix(); 
         // Text in der Szene
         glDisable(GL_TEXTURE_2D);
         glDisable(GL_LIGHTING);
         glLoadIdentity();
         glMatrixMode(GL_MODELVIEW);
         glOrtho( 0, w(), 0, h(), 0.1, 100.0);

         if (zeige_helfer)
           textGarnieren_();

         // Kann aus der Zeichenfunktion auswandern
         if(strlen(text))
         {
            DBG_ICCGL_V( X<<" "<<Y<<" "<<Z )
            DBG_ICCGL_V( oX<<" "<<oY<<" "<<oZ )

            // Text ueber der Szene
            glLoadIdentity();
            glMatrixMode(GL_PROJECTION);

            glLoadIdentity();
            glOrtho(0,w(),0,h(),-10.0,10.0);

            glRasterPos3f (X, Y, 9.999);
                      
            if(id() == 1) {
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
                   double wert = tabelle_[L][a][b][kanal];
                   DBG_PROG_V( L<<" "<<a<<" "<<b<<" "<<wert )
                   DBG_PROG_V( tabelle_.size()<<" "<<tabelle_[L].size()<<" "<<
                               tabelle_[L][a].size()<<" "<<kanal )
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
                   icc_examin_ns::status_info( text );
                   ZeichneOText (ortho_font, scal, text)
                 }}}
               }
            } else {
              icc_examin_ns::status_info( text );
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
    rechen_zeit = icc_examin_ns::zeitSekunden() - rechen_zeit;
    if(rechen_zeit > 1./15. )
    {
      if(smooth)
      {
        smooth = 0;
        strich_neu = true;
      }
    }
    if(rechen_zeit < 1./40.)
    {
      if(!smooth)
      {
        smooth = 1;
        strich_neu = true;
      }
    }
    if(strich_neu) {
      zeigeSpektralband_();
      garnieren_();
    }

    // Geschwindigkeit
    static double zeit_alt = 0;
    double zeit = icc_examin_ns::zeitSekunden();
    double dzeit = zeit - zeit_alt;
    if(dzeit < 0.001)
      dzeit = 0.001;
    zeit_diff_ = dzeit;;

    zeit_alt = zeit;
    sprintf(t, "zeit_diff_:%01f zeit:%f dzeit:%f",
            zeit_diff_, zeit, dzeit);
    DBG_PROG_V( t )


#   ifdef HAVE_FTGL
    if(ortho_font)
      glRasterPos2f(0, h() -10 );
#   endif
    if(icc_debug)
      ZeichneOText (ortho_font, scal, t)

    static int maus_x_alt, maus_y_alt;
    if(maus_x_alt != maus_x_ || maus_y_alt != maus_y_)
    maus_steht = true;
    maus_x_alt = maus_x_;
    maus_y_alt = maus_y_;

    // Farbkanalname
    // Text
    {
      glPushMatrix();
       glLoadIdentity();
       glMatrixMode(GL_PROJECTION);
       glLoadIdentity();
       glOrtho(0,w(),0,h(),-10.0,10.0);

       FARBE(textfarbe[0],textfarbe[1],textfarbe[2])

       glTranslatef(5,-12,0/*8.8 - schnitttiefe*3*/);

       if(id() == 1) {
         kanalname.append(_("Channel"));
         kanalname.append(": ");
         kanalname.append(kanalName());

#        ifdef HAVE_FTGL
         if(ortho_font)
           glRasterPos3f(0,ortho_font->LineHeight()/5+20,9.99);
#        endif
         ZeichneOText (ortho_font, scal, kanalname.c_str()) 
       } else {
         for (unsigned int i=0;
                i < dreiecks_netze.size();
                  ++i)
         {
           std::string text;
           text = dreiecks_netze[i].name.c_str();
           DBG_PROG_V( dreiecks_netze[i].name )
#          ifdef HAVE_FTGL
           if(ortho_font)
             glRasterPos2f(0, 20 + ortho_font->LineHeight()
                                   / 1.5 * (dreiecks_netze.size()-i-1) );
#          endif
           if(dreiecks_netze[i].aktiv)
             ZeichneOText (ortho_font, scal, text.c_str())
         }
       }

       glEnable(GL_TEXTURE_2D);
       glEnable(GL_LIGHTING);
      glPopMatrix();
      if(!ist_bewegt_ && maus_steht) {
        //redraw();
      }
    }

#   if 0
    glFlush();
#   else
    //glFinish();
#   endif
  } else
    DBG
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

void
GL_Ansicht::hineinPunkte       (std::vector<double>      &vect,
                                std::vector<std::string> &achs_namen)
{ DBG_PROG_START

  DBG_PROG_V( vect.size() )

  if(!punkte_.size() &&
     vect.size() > (1000*3) )
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
  
  DBG_PROG_V( zeig_punkte_als_messwert_paare<<"|"<<punktform<<"|"<<punkte_.size() )

  if (!zeig_punkte_als_messwert_paare &&
      punktform != MENU_dE1STERN &&
      punkte_.size())
    punktform = MENU_dE1STERN;

  if (zeig_punkte_als_messwert_paare &&
      punktform == MENU_dE1STERN &&
      punkte_.size())
    punktform = MENU_dE1KUGEL;

  valid_=false;
  DBG_PROG_ENDE
}

void
GL_Ansicht::hineinPunkte      (std::vector<double>      &vect,
                               std::vector<float>       &punkt_farben,
                               std::vector<std::string> &achsNamen)
{ DBG_PROG_START
  //Kurve aus tag_browser anzeigen
  farben_.clear();
  farben_ = punkt_farben;
  DBG_PROG_S( farben_.size()/4 << " Farben" )

  hineinPunkte(vect,achsNamen);

  DBG_PROG_ENDE
}

void
GL_Ansicht::hineinPunkte      (std::vector<double> &vect,
                               std::vector<float>  &punkt_farben,
                               std::vector<std::string> &farb_namen,
                               std::vector<std::string> &achs_namen)
{ DBG_PROG_START
  //Kurve aus tag_browser anzeigen
  DBG_NUM_V( farb_namen.size() <<"|"<< punkt_farben.size() )
  farb_namen_.clear();
  farb_namen_ = farb_namen;

  hineinPunkte(vect, punkt_farben, achs_namen);

  DBG_PROG_ENDE
}

void
GL_Ansicht::hineinNetze       (const std::vector<ICCnetz> & d_n)
{
  DBG_PROG_START
  if(d_n.size()) {
    dreiecks_netze.resize(0);
    DBG_V( dreiecks_netze.size() )
    dreiecks_netze = d_n;
  }

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
  //Kurve aus tag_browser anzeigen
  tabelle_ = vect;  DBG_PROG
  nach_farb_namen_ = nach; DBG_PROG

  achsNamen(achs_namen);
  punkte_.clear(); DBG_PROG

  valid_=false;
  redraw();

  icc_examin_ns::status_info(_("left-/middle-/right mouse button -> rotate/cut/menu"));

  DBG_PROG_ENDE
}


void
GL_Ansicht::menueAufruf ( int value )
{
  DBG_PROG_START

  if(visible()) {
    ;//DBG_PROG_S( "sichtbar "<< id )
  }

  {
    if (value >= MENU_MAX &&
        value < 100) {
      kanal = value - MENU_MAX; DBG_PROG_V( kanal )
      icc_examin_ns::status_info(_("left-/middle-/right mouse button -> rotate/cut/menu"));
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
      for (int i=0; i < 3 ; ++i) pfeilfarbe[i] = 1.*farb_faktor;
      for (int i=0; i < 3 ; ++i) textfarbe[i] = .75*farb_faktor;
      break;
    case MENU_HELLGRAU:
      hintergrundfarbe = 0.75;//MENU_HELLGRAU;
      for (int i=0; i < 3 ; ++i) pfeilfarbe[i] = 1.0*farb_faktor;
      for (int i=0; i < 3 ; ++i) textfarbe[i] = 0.5*farb_faktor;
      break;
    case MENU_GRAUGRAU:
      hintergrundfarbe = 0.5;//MENU_GRAUGRAU;
      for (int i=0; i < 3 ; ++i) pfeilfarbe[i] = .75*farb_faktor;
      for (int i=0; i < 3 ; ++i) textfarbe[i] = 0.25*farb_faktor;
      break;
    case MENU_DUNKELGRAU:
      hintergrundfarbe = 0.25;//MENU_DUNKELGRAU;
      for (int i=0; i < 3 ; ++i) pfeilfarbe[i] = 0.5*farb_faktor;
      for (int i=0; i < 3 ; ++i) textfarbe[i] = 0.75*farb_faktor;
      break;
    case MENU_SCHWARZ:
      hintergrundfarbe = 0.0;//MENU_SCHWARZ;
      for (int i=0; i < 3 ; ++i) pfeilfarbe[i] = .25*farb_faktor;
      for (int i=0; i < 3 ; ++i) textfarbe[i] = 0.5*farb_faktor;
      break;
    case Agviewer::FLYING:
      icc_examin_ns::status_info(_("left mouse button -> go back"));
      agv_.duenn = true;
      break;
    case Agviewer::ICCFLY_L:
      if(id() == 2) {
        agv_.eyeDist( 2 * agv_.dist() );
        vorder_schnitt = std_vorder_schnitt + agv_.dist();
      } else {
        agv_.eyeDist( agv_.dist() );
        vorder_schnitt = std_vorder_schnitt;
      }
      icc_examin_ns::status_info(_("left mouse button -> go back"));
      agv_.duenn = true;
      break;
    case Agviewer::ICCFLY_a:
      vorder_schnitt = std_vorder_schnitt;
      icc_examin_ns::status_info(_("left mouse button -> go back"));
      agv_.duenn = true;
      break;
    case Agviewer::ICCFLY_b:
      vorder_schnitt = std_vorder_schnitt;
      icc_examin_ns::status_info(_("left mouse button -> go back"));
      agv_.duenn = true;
      break;
    case Agviewer::ICCPOLAR:
      agv_.duenn = true;
    case Agviewer::POLAR:
      if(id() == 1)
        agv_.duenn = true;
      else
        agv_.duenn = false;
      break;
    case Agviewer::AGV_STOP:
      agv_.duenn = false;
      icc_examin_ns::status_info(_("left-/middle-/right mouse button -> rotate/cut/menu"));
      break;
    }
  }

  if(visible()) {
    erstelleGLListen_();
    redraw();
  }

  DBG_PROG_V( value )
  DBG_PROG_ENDE
}

void
GL_Ansicht::c_ ( Fl_Widget* w, void* daten )
{ DBG_PROG_START

  intptr_t value = (intptr_t) daten;
  DBG_PROG_V( value )

  GL_Ansicht *gl_obj = dynamic_cast<GL_Ansicht*>(w->parent());
  DBG_MEM_V( (intptr_t)gl_obj )
  DBG_MEM_V( (intptr_t)w->parent() )
  if(!w->parent())
    WARN_S(_("Konnte keine Eltern finden"))
  else
  if (gl_obj)
  {
    gl_obj->menueAufruf(value);

    if(value >= 100)
      gl_obj->agv_ .agvSwitchMoveMode (value);
  }
  else
    WARN_S(_("Konnte keine passende Programmstruktur finden"))

  DBG_PROG_ENDE
}

int
GL_Ansicht::dID (int display_list)
{ DBG_ICCGL_V( id_ )
  return id_ *DL_MAX + display_list;
}



#undef ZeichneText
#undef FARBE
#undef implementGlutFunktionen

