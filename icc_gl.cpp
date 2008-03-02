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


//TODO: alpha benötigt eine Liste der Objekte in Reihenfolge von Objekt<->Kamera
//      die Objekte werden mit alpha in dieser Reihenfolge gezeichnet

#include "icc_examin.h"
#include "icc_formeln.h"
#include "icc_oyranos.h"
#include "icc_utils.h"
#include "icc_info.h"
#include "agviewer.h"
#include "icc_gl.h"
#include "icc_helfer.h"
#include <FL/Fl_Menu_Button.H>

#include <cmath>

//#define Beleuchtung
//#define Lab_STERN 1

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

typedef enum {NOTALLOWED, AXES, RASTER, PUNKTE , SPEKTRUM, HELFER, DL_MAX } DisplayLists;
bool gl_voll[DL_MAX] = {false,false,false,false,false,false};

typedef enum {
 MENU_AXES,
 MENU_QUIT,
 MENU_KUGEL,           // Formen der 3DLut-darstellung
 MENU_WUERFEL,
 MENU_STERN,
 MENU_GRAU,            // Die Farbdarstellung der 3DLut
 MENU_FARBIG,
 MENU_KONTRASTREICH,
 MENU_SCHALEN,         // Aussparen der 3DLut
 MENU_dE1KUGEL,        // Mess-/Profilwertdifferenzen mit Farborten
 MENU_dE2KUGEL,
 MENU_dE4KUGEL,
 MENU_dE1STERN,
 MENU_DIFFERENZ_LINIE, // Mess-/Profilwertdifferenzen mit Geraden pur
 MENU_SPEKTRALBAND,    // Darstellung der Spektralfarben als Band
 MENU_HELFER,          // Texte und Pfeile darstellen
 MENU_WEISS,           // Hintergrundfarben
 MENU_HELLGRAU,
 MENU_GRAUGRAU,
 MENU_DUNKELGRAU,
 MENU_SCHWARZ,
 MENU_MAX
} MenuChoices;

int DrawAxes = 0;

#define bNachX(b) (b*b_darstellungs_breite - b_darstellungs_breite/2.)
#define LNachY(L) (L - 0.5)
#define aNachZ(a) (a*a_darstellungs_breite - a_darstellungs_breite/2.)
#define LabNachXYZv(L,a,b) \
         (b*b_darstellungs_breite - b_darstellungs_breite/2.), \
         (L - 0.5), \
         (a*a_darstellungs_breite - a_darstellungs_breite/2.)

const double GL_Ansicht::std_vorder_schnitt = 4.2;

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
  DBG_PROG_ENDE
}

GL_Ansicht::~GL_Ansicht()
{ DBG_PROG_START
  if (gl_voll[RASTER]) {
    glDeleteLists (id()*DL_MAX + RASTER, 1);
    DBG_PROG_S( "delete glListe " << id()*DL_MAX + RASTER ) }
  if (gl_voll[HELFER]) {
    glDeleteLists (id()*DL_MAX + HELFER, 1);
    DBG_PROG_S( "delete glListe " << id()*DL_MAX + HELFER ) }
  if (gl_voll[PUNKTE]) {
    glDeleteLists (id()*DL_MAX + PUNKTE, 1);
    DBG_PROG_S( "delete glListe " << id()*DL_MAX + PUNKTE ) }
  if (gl_voll[SPEKTRUM]) {
    glDeleteLists (id()*DL_MAX + SPEKTRUM, 1);
    DBG_PROG_S( "delete glListe " << id()*DL_MAX + SPEKTRUM ) }
  DBG_PROG_ENDE
}

void
GL_Ansicht::init(int init_id)
{ DBG_PROG_START

  id_ = init_id;

  DBG_PROG

  resizable(0);

  mode(FL_RGB |FL_DOUBLE |FL_ALPHA |FL_DEPTH |FL_MULTISAMPLE);
  if(!can_do())
    WARN_S( _("OpenGL eventuell nicht korrekt gesetzt von ICC Examin "<< mode()) )

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

  agv_.agvMakeAxesList(AXES); DBG_PROG


  // Initialisieren
  menueInit_(); DBG_PROG
  menueAufruf (MENU_HELLGRAU); // Farbschema
  menueAufruf (MENU_GRAU);     // CLUT Farbschema
  if (id() == 1) menueAufruf (MENU_WUERFEL);

  DBG_PROG_ENDE
}

void
GL_Ansicht::stop()
{
  DBG_PROG_START
  agv_.agvSwitchMoveMode (Agviewer::AGV_STOP);
  DBG_PROG_ENDE
}

void
GL_Ansicht::auffrischen()
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

  if(!visible()) {
    DBG_PROG_ENDE
    return;
  }

  if(!valid()) {
    GLinit_();  DBG_PROG
    agv_.agvSetAllowIdle (1);
    fensterForm();
    auffrischen();
  }

  zeichnen();

  DBG_PROG_V( dreiecks_netze.size() )

  DBG_PROG_ENDE
}

int
GL_Ansicht::handle( int event )
{
  DBG_ICCGL_START
  int mausknopf = Fl::event_state();
  switch(event)
  {
    case FL_PUSH:
         if(mausknopf == FL_BUTTON3) {
           menue_button_->popup();
           break;
         }
         agv_.agvHandleButton( mausknopf, event, Fl::event_x(),Fl::event_y());
         DBG_ICCGL_S( "FL_PUSH bei: " << Fl::event_x() << "," << Fl::event_y() )
         break;
    case FL_RELEASE:
         agv_.agvHandleButton(Fl::event_state(),event, Fl::event_x(),Fl::event_y());
         DBG_ICCGL_S( "FL_RELEASE bei: " << Fl::event_x() << "," << Fl::event_y() )
         break;
    case FL_DRAG:
         DBG_ICCGL_S( "FL_DRAG bei: " << Fl::event_x() << "," << Fl::event_y() )
         agv_.agvHandleMotion(Fl::event_x(), Fl::event_y());
         break;
    case FL_KEYDOWN:
         DBG_ICCGL_S( "FL_KEYDOWN bei: " << Fl::event_x() << "," << Fl::event_y() )
         break;
    case FL_KEYUP:
         DBG_ICCGL_S( "FL_KEYUP bei: " << Fl::event_x() << "," << Fl::event_y() )
         break;
    case FL_MOVE:
         DBG_ICCGL_S( "FL_MOVE bei: " << Fl::event_x() << "," << Fl::event_y() )
         break;
    case FL_MOUSEWHEEL:
         DBG_ICCGL_S( "FL_MOUSEWHEEL" )
         break;
    default:
         DBG_ICCGL_S( "default" )
         DBG_ICCGL_ENDE
         return Fl_Gl_Window::handle(event);
  }
  if(mausknopf)
  {
    switch(Fl::event_state()) {
      case FL_BUTTON1: DBG_ICCGL_S("FL_BUTTON1") break;
      case FL_BUTTON2: DBG_ICCGL_S("FL_BUTTON2") break;
      case FL_BUTTON3: DBG_ICCGL_S("FL_BUTTON3") break;
      default: DBG_ICCGL_S("unbekanner event_state()") break;
    }
  }

  DBG_ICCGL_ENDE
  return 1;
}

void
GL_Ansicht::GLinit_()
{ DBG_PROG_START
  GLfloat mat_ambuse[] = { 0.2, 0.2, 0.2, 1.0 };
  GLfloat mat_specular[] = { 0.6, 0.6, 0.6, 1.0 };

  GLfloat light0_position[] = { 2.4, 1.6, 1.2, 0.0 };

  glLightfv(GL_LIGHT0, GL_POSITION, light0_position);
  //GLfloat light1_position[] = { -2.4, -1.6, -1.2, 0.0 };
  //glLightfv(GL_LIGHT1, GL_POSITION, light1_position);
  glEnable(GL_LIGHTING);
  glEnable(GL_LIGHT0);
  //glEnable(GL_LIGHT1);
  #ifndef Beleuchtung
  glDisable(GL_LIGHTING);
  #endif

  glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, mat_ambuse);
  glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
  glMaterialf(GL_FRONT, GL_SHININESS, 25.0);
  
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  glEnable(GL_NORMALIZE);

  glDepthFunc(GL_LESS);
  glEnable(GL_DEPTH_TEST);

  //glShadeModel(GL_SMOOTH);
  glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);

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
    DBG_ICCGL_S("e = " << Fl::event_key() )
  }
  DBG_ICCGL_ENDE
}

// Materialfarben setzen
#define FARBE(r,g,b) {farbe [0] = (r); farbe [1] = (g); farbe [2] = (b); \
                      glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, farbe); \
                      glColor4f(farbe[0],farbe[1],farbe[2],1.0); }

// Text zeichnen
#define ZeichneText(Font,Zeiger) { \
   glDisable(GL_TEXTURE_2D); \
   glDisable(GL_LIGHTING); \
   glLineWidth(strichmult); \
      glTranslatef(.0,0,0.01); \
        glScalef(0.001,0.001,0.001); \
          for (const char* p = Zeiger; *p; p++) { \
             glutStrokeCharacter(Font, *p); \
          } \
          for (const char* p = Zeiger; *p; p++) { \
            glTranslatef(0.0 - glutStrokeWidth(Font, *p),0,0); \
          } \
        glScalef(1000,1000,1000); \
      glTranslatef(.0,0,-.01); \
   glEnable(GL_TEXTURE_2D); \
   glEnable(GL_LIGHTING); \
   glLineWidth(strichmult); }

#define ZeichneOText(font, scal, buffer) \
                                   glScalef(scal,scal,scal); \
                                   ZeichneText(font,buffer); \
                                   glScalef(1.0/scal,1.0/scal,1.0/scal);

#define ZeichneBuchstaben(Font,Buchstabe) { \
        glScalef(0.001,0.001,0.001); \
          glutStrokeCharacter(Font, Buchstabe); \
          glTranslatef(0.0 - glutStrokeWidth(Font, Buchstabe),0,0); \
        glScalef(1000,1000,1000); }

void
zeichneKoordinaten(float strichmult)
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
        ZeichneText(GLUT_STROKE_ROMAN,&text[0])
      glTranslatef(0.3,0.1,0.05);
    glRotatef (-90,0.0,0,1.0);

    FARBE(1,1,1)
    glTranslatef(.1,0,0);
      FARBE(1,0,1)
      glRotatef (90,0.0,1.0,.0);
        glutSolidCone(0.01, 0.025, 8, 2);
      glRotatef (-90,0.0,1.0,.0);
      FARBE(1,1,1)
      glTranslatef(.02,0,0);
        ZeichneBuchstaben(GLUT_STROKE_ROMAN, 'X')
      glTranslatef((-0.02),0,0);
    glTranslatef((-0.1),0,0);

    glTranslatef(.0,.1,0);
      glRotatef (270,1.0,.0,.0);
        FARBE(1,1,0)
        glutSolidCone(0.01, 0.025, 8, 2);
      glRotatef (90,1.0,.0,.0);
      glRotatef (90,0.0,.0,1.0);
        FARBE(1,1,1)
        ZeichneBuchstaben(GLUT_STROKE_ROMAN, 'Y')
      glRotatef (270,0.0,.0,1.0);
    glTranslatef(.0,(-0.1),0);

    glTranslatef(0,0,.1);
      FARBE(0,1,1)
      glutSolidCone(0.01, 0.025, 8, 2);
      glRotatef (90,0.0,.5,.0);
        glTranslatef(-.1,0,0);
          FARBE(1,1,1)
          ZeichneBuchstaben(GLUT_STROKE_ROMAN, 'Z')
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
 
  zeigeSpektralband_();

  DBG_PROG_V( punktform <<" "<< MENU_dE1STERN )
  punkteAuffrischen();

  //Hintergrund
  switch (hintergrundfarbe) {
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
  }

  DBG_PROG_ENDE
}

void
GL_Ansicht::garnieren_()
{ DBG_PROG_START
  char text[256];
  char* ptr = 0;

  #define PFEILSPITZE glutSolidCone(0.02, 0.05, 16, 4);

  DBG_PROG_V( id() )
  // Pfeile und Text
  if (gl_voll[HELFER])
    glDeleteLists (id()*DL_MAX + HELFER, 1);
  glNewList(id()*DL_MAX + HELFER, GL_COMPILE); DBG_PROG_V( id()*DL_MAX + HELFER)
    gl_voll[HELFER] = true;
    GLfloat farbe[] =   { textfarbe[0],textfarbe[1],textfarbe[2], 1.0 };

    // Farbkanalname
    //glEnable(GL_BLEND);
    //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_LINE_SMOOTH);
    glEnable(GL_LIGHTING);

    glPushMatrix();
      glMatrixMode(GL_MODELVIEW);
      glLineWidth(1.0*strichmult);
      glTranslatef(.5,0.62,.5);
      glRotatef (270,0.0,1.0,.0);
      FARBE(textfarbe[0],textfarbe[1],textfarbe[2])
      if (nach_farb_namen_.size())
      {
        ptr = (char*) nach_farb_namen_[kanal].c_str();
        sprintf (&text[0], ptr);
        //ZeichneText(GLUT_STROKE_ROMAN,&text[0])
      }
    glPopMatrix(); DBG_PROG

    // CIE*L - oben
    glPushMatrix();
      FARBE(pfeilfarbe[0],pfeilfarbe[1],pfeilfarbe[2])
      glTranslatef(0,.5,0);
      glRotatef (270,1.0,0.0,.0);
      PFEILSPITZE
      glRotatef (270,0.0,0.0,1.0);
      glTranslatef(.02,0,0);
      FARBE(textfarbe[0],textfarbe[1],textfarbe[2])
      if (von_farb_namen_.size())
      {
        ptr = (char*) von_farb_namen_[0].c_str();
        sprintf (&text[0], ptr);
        ZeichneText(GLUT_STROKE_ROMAN,&text[0])
      }
    glPopMatrix(); DBG_PROG
    glPushMatrix();
      glBegin(GL_LINES);
        glVertex3f(0, .5, 0); glVertex3f(0, -.5, 0);
      glEnd();
    glPopMatrix();

    // CIE*a - rechts
    glPushMatrix();
      if (von_farb_namen_.size() &&
          von_farb_namen_[1] == _("CIE *a"))
        glTranslatef(0,-.5,0);
      glBegin(GL_LINES);
        glVertex3f(0, 0,  a_darstellungs_breite/2.);
        glVertex3f(0, 0, -a_darstellungs_breite/2.);
      glEnd();
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
      FARBE(textfarbe[0],textfarbe[1],textfarbe[2])
      if (von_farb_namen_.size())
      {
        ptr = (char*) von_farb_namen_[1].c_str();
        sprintf (&text[0], ptr);
        ZeichneText(GLUT_STROKE_ROMAN,&text[0])
      }
      if (von_farb_namen_.size() &&
          von_farb_namen_[1] == _("CIE *a"))
        glTranslatef(0,0.5,0);
    glPopMatrix(); DBG_PROG

    // CIE*b - links
    glPushMatrix();
      if (von_farb_namen_.size() &&
          von_farb_namen_[2] == _("CIE *b"))
        glTranslatef(0,-0.5,0);
      glBegin(GL_LINES);
        glVertex3f( b_darstellungs_breite/2., 0, 0);
        glVertex3f(-b_darstellungs_breite/2., 0, 0);
      glEnd();
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
        FARBE(textfarbe[0],textfarbe[1],textfarbe[2])
      }
      glTranslatef(.0,.0,-b_darstellungs_breite);
      glRotatef (180,0.0,.5,.0);
      if (von_farb_namen_.size())
      {
        ptr = (char*) von_farb_namen_[2].c_str();
        sprintf (&text[0], ptr);
        ZeichneText(GLUT_STROKE_ROMAN,&text[0])
      }
      if (von_farb_namen_.size() &&
          von_farb_namen_[2] == _("CIE *b"))
        glTranslatef(0,0.5,0);
    glPopMatrix();
    glLineWidth(1.0*strichmult);

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
      DBG_PROG_S( _("Kanalauswahl geändert: ") << kanal )
    }
  }

  // Tabelle
  if (gl_voll[RASTER])
    glDeleteLists (id()*DL_MAX + RASTER, 1);

  if (tabelle_.size())
  {
    glNewList(id()*DL_MAX + RASTER, GL_COMPILE); DBG_PROG_V( id()*DL_MAX + RASTER )
      gl_voll[RASTER] = true;
      int n_L = tabelle_.size(), n_a=tabelle_[0].size(), n_b=tabelle_[0][0].size();
      double dim_x = 1.0/(n_b); DBG_PROG_V( dim_x )
      double dim_y = 1.0/(n_L); DBG_PROG_V( dim_y )
      double dim_z = 1.0/(n_a); DBG_PROG_V( dim_z )
      double start_x,start_y,start_z, x,y,z;
      double groesse = (dim_x + dim_y + dim_z)/ 3;
      double wert;
              switch (punktform) {
                case MENU_STERN:   
                schnitttiefe= HYP3(dim_x,dim_y,dim_z);
                break;
                case MENU_WUERFEL: schnitttiefe = HYP3(groesse,groesse,groesse);
                break;
                case MENU_KUGEL:   schnitttiefe = groesse;
                break;
              } DBG_NUM_V( schnitttiefe );
      start_x = start_y = start_z = x = y = z = 0.5; start_y = y = -0.5;
      glPushMatrix();
      #ifndef Beleuchtung
      glDisable(GL_LIGHTING);
      #endif
      glTranslatef(start_x + dim_x/2.0, start_y + dim_y/2.0, start_z + dim_z/2.0);
      DBG_PROG_V( tabelle_.size() <<" "<< tabelle_[0].size() )
      glTranslatef(-dim_x,-dim_y,-dim_z);
      for (int L = 0; L < (int)n_L; L++) { //DBG_PROG_V( L )
        double y_versatz = (double)L/(n_L-1)*dim_y-n_L/((n_L-0.0)*2.0)*dim_y;
        x = start_x + L * dim_y;
        glTranslatef(0.0,dim_y,0.0);
        glTranslatef(0.0,0.0,-1.0);
        for (int a = 0; a < (int)n_a; a++) { //DBG_PROG_V( a )
          double z_versatz = (double)a/(n_a-1)*dim_z-n_a/((n_a-0.0)*2.0)*dim_z;
          y = start_y + a * dim_z;
          glTranslatef(0.0,0.0,dim_z);
          glTranslatef(-1,0.0,0.0);
          for (int b = 0; b < (int)n_b; b++) { //DBG_PROG_V( k )
            z = start_z + b * dim_x; //DBG_PROG_V( dim_x )
            double x_versatz= (double)b/(n_b-1)*dim_x-n_b/((n_b-0.0)*2.0)*dim_z;
            glTranslatef(dim_x,0.0,0.0); //DBG_PROG_V( dim_x )
            wert = tabelle_[L][a][b][kanal]; //DBG_PROG_V( L << a << b << kanal )
            #ifdef Beleuchtung
            FARBE(wert, wert, wert)
            //glColor3f(wert, wert, wert);
            #else
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
            if (wert) {
              #endif
              switch (punktform) {
                case MENU_STERN: {
                glBegin(GL_QUADS);
                  glVertex3f( dim_x/2,  dim_y/2, z_versatz);
                  glVertex3f( dim_x/2, -dim_y/2, z_versatz);
                  glVertex3f(-dim_x/2, -dim_y/2, z_versatz);
                  glVertex3f(-dim_x/2,  dim_y/2, z_versatz);
                glEnd();
                glBegin(GL_QUADS);
                  glVertex3f(x_versatz,  dim_y/2,-dim_z/2);
                  glVertex3f(x_versatz, -dim_y/2,-dim_z/2);
                  glVertex3f(x_versatz, -dim_y/2, dim_z/2);
                  glVertex3f(x_versatz,  dim_y/2, dim_z/2);
                glEnd();
                glBegin(GL_QUADS);
                  glVertex3f( dim_x/2, y_versatz,-dim_z/2);
                  glVertex3f(-dim_x/2, y_versatz,-dim_z/2);
                  glVertex3f(-dim_x/2, y_versatz, dim_z/2);
                  glVertex3f( dim_x/2, y_versatz, dim_z/2);
                glEnd();
                }
                break;
                case MENU_WUERFEL: glutSolidCube(groesse*0.995); break;
                case MENU_KUGEL:   glutSolidSphere (groesse*.75, 12, 12); break;
              }
            }
          }
        }
      } DBG_PROG
      glPopMatrix();
      #ifndef Beleuchtung
      glEnable(GL_LIGHTING);
      #endif
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
             X = EyeDist*sin(TORAD(EyeAz))*cos(TORAD(EyeEl)),   // CIE*b
             Y = EyeDist*sin(TORAD(EyeEl)),                     // CIE*L
             Z = EyeDist*cos(TORAD(EyeAz))*cos(TORAD(EyeEl));   // CIE*a

       if(icc_debug == 14) {
       strichmult = 3;
       DBG_ICCGL_S( "dist:"<<EyeDist<<" elevation:"<<EyeEl<<" azimuth:"<<EyeAz )
       glMatrixMode(GL_MODELVIEW);
       glLoadIdentity();
       glTranslatef(-X/1.2,Y/1.2,Z/1.2);
       DBG_ICCGL_S( "X:"<<X<<" Y:"<<Y<<" Z:"<<Z )
       zeichneKoordinaten(strichmult);
       }
      
       ICCnetz netz;
       int punkte_n = 0;
       double abstand;
       for( j = 0; j < dreiecks_netze.size(); j++ )
       {
         double schattierung = .93 - .8/dreiecks_netze.size()*j;
         if(dreiecks_netze[j].aktiv && dreiecks_netze[j].transparenz)
         {
             // ich hoffe das dauert nicht zu lange
           netz.punkte. insert( netz.punkte.begin()+punkte_n ,
                                dreiecks_netze[j].punkte.begin(),
                                dreiecks_netze[j].punkte.end()    );
             // den Punkten im neuen Netz die Transparenz zuweisen
           for( k = punkte_n; k < netz.punkte.size(); ++k) {
             if(dreiecks_netze[j].grau) {
               netz.punkte[k].farbe[0] = schattierung;
               netz.punkte[k].farbe[1] = schattierung;
               netz.punkte[k].farbe[2] = schattierung;
             }
             netz.punkte[k].farbe[3] = dreiecks_netze[j].transparenz;
             netz.punkte[k].koord[1] *= a_darstellungs_breite;
             netz.punkte[k].koord[2] *= b_darstellungs_breite;
           }

           std::multimap<double,DreiecksIndexe>::const_iterator it;
           for( it = dreiecks_netze[j].indexe.begin();
                it != dreiecks_netze[j].indexe.end(); ++it )
           {
               // die Indexe werden gesondert eingefügt, um sie neu zu zählen
       /*A*/ std::pair<double,DreiecksIndexe>
                            index_p( *it );
       /*B*/ for( k = 0; k < 3; ++k)
               index_p.second.i[k] += punkte_n;
       /*C*/
             #if 0
               // das wird der Mittelpunkt des umschließenden Quaders
             abstand =
                ( HYP3(netz.punkte[index_p.second.i[0]].koord[0]+Y,
                       netz.punkte[index_p.second.i[0]].koord[1]+Z,
                       netz.punkte[index_p.second.i[0]].koord[2]-X)
                + HYP3(netz.punkte[index_p.second.i[1]].koord[0]+Y,
                       netz.punkte[index_p.second.i[1]].koord[1]+Z,
                       netz.punkte[index_p.second.i[1]].koord[2]-X)
                + HYP3(netz.punkte[index_p.second.i[2]].koord[0]+Y,
                       netz.punkte[index_p.second.i[2]].koord[1]+Z,
                       netz.punkte[index_p.second.i[2]].koord[2]-X)
                ) / 3.0;
             #else
               // Mittelpunkt des Dreiecks
             double seitenhalbierende[3];
             seitenhalbierende[0] =
                    (  (netz.punkte[index_p.second.i[0]].koord[0]+Y)
                     + (netz.punkte[index_p.second.i[1]].koord[0]+Y))/2.0;
             seitenhalbierende[1] =
                    (  (netz.punkte[index_p.second.i[0]].koord[1]+Z)
                     + (netz.punkte[index_p.second.i[1]].koord[1]+Z))/2.0;
             seitenhalbierende[2] =
                    (  (netz.punkte[index_p.second.i[0]].koord[2]-X)
                     + (netz.punkte[index_p.second.i[1]].koord[2]-X))/2.0;
             double mittelpunkt[3];
             mittelpunkt[0] = (  2.0 * seitenhalbierende[0]
                                + netz.punkte[index_p.second.i[2]].koord[0]+Y)
                              / 3.0;
             mittelpunkt[1] = (  2.0 * seitenhalbierende[1]
                                + netz.punkte[index_p.second.i[2]].koord[1]+Z)
                              / 3.0;
             mittelpunkt[2] = (  2.0 * seitenhalbierende[2]
                                + netz.punkte[index_p.second.i[2]].koord[2]-X)
                              / 3.0;
             abstand = HYP3( mittelpunkt[0], mittelpunkt[1], mittelpunkt[2] );
             #endif  
             index_p.first = abstand;
               // der Behälter std::map übernimmt die Sortierung
       /*D*/ netz.indexe.insert(index_p);
           }
             // neue Basis für Indexnummern
           punkte_n += dreiecks_netze[j].punkte.size();
         }
       }
      glPopMatrix();


  if (gl_voll[RASTER])
    glDeleteLists (id()*DL_MAX + RASTER, 1);

      //glNewList(id()*DL_MAX + RASTER, GL_COMPILE); DBG_PROG_V( id()*DL_MAX + RASTER )
      //gl_voll[RASTER] = true;

      #ifndef Beleuchtung
      glDisable(GL_LIGHTING);
      #endif

      #if 0
      glEnable (GL_BLEND);
      glBlendFunc (GL_SRC_COLOR, GL_DST_ALPHA);
      glEnable (GL_ALPHA_TEST_FUNC);
      //glAlphaFunc (GL_ALPHA_TEST, GL_ONE_MINUS_DST_ALPHA);
      #else
      glEnable (GL_BLEND);
      //glDepthMask(GL_FALSE);              // Konturen und Schnittkanten müssen
                                          // richtig übergeben werden 
      glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
      glEnable (GL_ALPHA_TEST_FUNC);
      glAlphaFunc (GL_ALPHA_TEST, GL_ONE_MINUS_DST_ALPHA);
      #endif
      #if 0
      glPolygonMode(GL_FRONT, GL_FILL);   // Vorderseite als Fläche 
      glPolygonMode(GL_BACK, GL_LINE);   // Rückseite als Linien
      #endif
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
          for( int l = 2; l >= 0; --l)
          {
            index = it->second.i[l];
            glColor4d( netz.punkte[index].farbe[0],
                       netz.punkte[index].farbe[1],
                       netz.punkte[index].farbe[2],
                       netz.punkte[index].farbe[3]);

            // Punktkoordinaten setzen
            glVertex3d( netz.punkte[index].koord[2],
                        netz.punkte[index].koord[0],
                        netz.punkte[index].koord[1] );
          }
          glEnd();
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
      #ifndef Beleuchtung
      glEnable(GL_LIGHTING);
      #endif
      //glEndList();

  DBG_PROG_ENDE
}


void
GL_Ansicht::punkteAuffrischen()
{ DBG_PROG_START

  if (gl_voll[PUNKTE])
    glDeleteLists (id()*DL_MAX + PUNKTE, 1);

  //Koordinaten  in CIE*b CIE*L CIE*a Reihenfolge 
  if (punkte_.size()) {
    glPointSize(punktgroesse);
    if( punkte_.size() )
      DBG_PROG_V( punkte_.size() )
    if( farben_.size() )
      DBG_PROG_V( farben_.size() )
    if( farb_namen_.size() )
      DBG_PROG_V( farb_namen_.size() )

    glNewList(id()*DL_MAX + PUNKTE, GL_COMPILE); DBG_PROG_V( id()*DL_MAX + PUNKTE ) 
      gl_voll[PUNKTE] = true;
      #ifndef Beleuchtung
      glDisable(GL_LIGHTING);
      #endif

      #if 1
      glDisable (GL_BLEND);
      //glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
      glDisable/*Enable*/ (GL_ALPHA_TEST_FUNC);
      //glAlphaFunc (GL_ALPHA_TEST, GL_ONE_MINUS_DST_ALPHA);
      #else
      #endif

      //glColor3f(0.9, 0.9, 0.9);
      for (unsigned j = 0; j < punkte_.size(); j+=3)
      {
        unsigned i = j;
        glPushMatrix();
          // zurecht setzen
          glTranslatef( -b_darstellungs_breite/2,-.5,-a_darstellungs_breite/2 );
          // Punktkoordinaten setzen
          glTranslatef( punkte_[i+2]*b_darstellungs_breite,
                        punkte_[i+0], punkte_[i+1]*a_darstellungs_breite );
          if(zeig_punkte_als_messwert_paare &&
             i%6 == 0)
          {
            glLineWidth(2.0*strichmult);
            glColor4f(.97, .97, .97, 1. );
            glBegin(GL_LINES);
              glVertex3f(0, 0, 0);
              glColor4f(1., .6, .6, 1.0 );
              glVertex3f((punkte_[i+5]*b_darstellungs_breite)
                          -(punkte_[i+2]*b_darstellungs_breite),
                         punkte_[i+3]-punkte_[i+0],
                         (punkte_[i+4]*a_darstellungs_breite)
                          -(punkte_[i+1]*a_darstellungs_breite) );
            glEnd();
            #if 0
            glColor4f(0.1, 0.1, 0.1, 1.0 );
            glBegin(GL_POINTS);
              glVertex3f((punkte_[i+5]*b_darstellungs_breite)
                          -(punkte_[i+2]*b_darstellungs_breite),
                         punkte_[i+3]-punkte_[i+0],
                         (punkte_[i+4]*a_darstellungs_breite)
                          -(punkte_[i+1]*a_darstellungs_breite) );
            glEnd();
            #endif
          }

          if (farben_.size())
          {
            if ( farben_[i/3*4+3] < 1.0 )
            {
              //glEnable (GL_BLEND);
              //glBlendFunc (GL_SRC_COLOR, GL_DST_ALPHA);
            }
            glColor4f(farben_[i/3*4+0], farben_[i/3*4+1], farben_[i/3*4+2],
                      farben_[i/3*4+3] );
          }

          switch (punktform)
          {
          case MENU_dE1STERN:
               #ifdef Lab_STERN
               {
               double groesse = 0.01;
               glBegin(GL_QUADS);
                 glVertex3d(  groesse/2, 0, -groesse/2 );
                 glVertex3d( -groesse/2, 0, -groesse/2 );
                 glVertex3d( -groesse/2, 0,  groesse/2 );
                 glVertex3d(  groesse/2, 0,  groesse/2 );
               glEnd();
               glBegin(GL_QUADS);
                 glVertex3d(  groesse/2, -groesse/2, 0 );
                 glVertex3d( -groesse/2, -groesse/2, 0 );
                 glVertex3d( -groesse/2,  groesse/2, 0 );
                 glVertex3d(  groesse/2,  groesse/2, 0 );
               glEnd();
               glBegin(GL_QUADS);
                 glVertex3d( 0,  groesse/2, -groesse/2 );
                 glVertex3d( 0, -groesse/2, -groesse/2 );
                 glVertex3d( 0, -groesse/2,  groesse/2 );
                 glVertex3d( 0,  groesse/2,  groesse/2 );
               glEnd();
               }
               #else
               glBegin(GL_POINTS);
                 glVertex2d( 0,0 );
               glEnd();
               #endif
            break;
          case MENU_dE1KUGEL: glutSolidSphere (0.005, 5, 5); break;
          case MENU_dE2KUGEL: glutSolidSphere (0.01, 8, 8); break;
          case MENU_dE4KUGEL: glutSolidSphere (0.02, 12, 12);
               break;
          case MENU_DIFFERENZ_LINIE:
               break;
          }
        glPopMatrix();
      }
      #ifndef Beleuchtung
      glEnable(GL_LIGHTING);
      #endif
    glEndList();
  }

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  DBG_PROG_ENDE
}

extern float cieXYZ [471][3]; // in 
#include <lcms.h>

#define PRECALC cmsFLAGS_NOTPRECALC 
#if 0
#define BW_COMP cmsFLAGS_WHITEBLACKCOMPENSATION
#else
#define BW_COMP 0
#endif

void
GL_Ansicht::zeigeSpektralband_()
{ DBG_PROG_START

  if (gl_voll[SPEKTRUM])
    glDeleteLists (id()*DL_MAX + SPEKTRUM, 1);
  if (spektralband == MENU_SPEKTRALBAND)
  {
    // lcms Typen
    cmsHPROFILE hsRGB;
    cmsHPROFILE hLab;
    cmsHTRANSFORM hLabtoRGB;
    double *RGB_Speicher = 0;
    double *XYZ_Speicher = 0;
    double *Lab_Speicher = 0;

    // Initialisierung für lcms
    size_t groesse = 0;
    const char* block = 0;
    block = icc_oyranos.moni(groesse);
    DBG_MEM_V( (int*) block <<" "<<groesse )

    if(groesse)
      hsRGB = cmsOpenProfileFromMem(const_cast<char*>(block), groesse);
    else
      hsRGB = cmsCreate_sRGBProfile();
    if(!hsRGB) WARN_S( _("hsRGB Profil nicht geöffnet") )
    hLab  = cmsCreateLabProfile(cmsD50_xyY());
    if(!hLab)  WARN_S( _("hLab Profil nicht geöffnet") )

    hLabtoRGB = cmsCreateTransform          (hLab, TYPE_Lab_DBL,
                                             hsRGB, TYPE_RGB_DBL,
                                             INTENT_ABSOLUTE_COLORIMETRIC,
                                             PRECALC|BW_COMP);
    if (!hLabtoRGB) WARN_S( _("keine hXYZtoRGB Transformation gefunden") )

    // Spektrumvariablen
    //int nano_min = 63; // 420 nm
    int nano_max = 471;//341; // 700 nm

    // Umrechnung
    XYZ_Speicher = new double [nano_max*3];
    for (int i = 0; i < nano_max; ++i)
    { for(int j = 0; j < 3 ; ++j)
      { XYZ_Speicher[i*3+j] = (double)cieXYZ[i][j];
      }
    }
    Lab_Speicher = new double [nano_max*3];
    RGB_Speicher = new double [nano_max*3];
    if(!XYZ_Speicher)  WARN_S( _("XYZ_speicher Speicher nicht verfügbar") )
    if(!Lab_Speicher)  WARN_S( _("Lab_speicher Speicher nicht verfügbar") )
    if(!RGB_Speicher)  WARN_S( _("RGB_speicher Speicher nicht verfügbar") )


    XYZtoLab (XYZ_Speicher, Lab_Speicher, nano_max);

    DBG_PROG_V( nano_max )
    double *cielab = new double[nano_max*3];
    LabToCIELab (Lab_Speicher, cielab, nano_max);
    cmsDoTransform (hLabtoRGB, cielab, RGB_Speicher, nano_max);
    if (cielab) delete [] cielab;


    GLfloat farbe[] =   { pfeilfarbe[0],pfeilfarbe[1],pfeilfarbe[2], 1.0 };

    DBG_PROG_V( id()*DL_MAX + SPEKTRUM ) 
    glNewList(id()*DL_MAX + SPEKTRUM, GL_COMPILE);
      gl_voll[SPEKTRUM] = true;

      glDisable(GL_LIGHTING);
      glDisable (GL_BLEND);
      glDisable (GL_ALPHA_TEST_FUNC);
      glEnable  (GL_LINE_SMOOTH);

      glLineWidth(3.0*strichmult);
      glColor4f(0.5, 1.0, 1.0, 1.0);
      glBegin(GL_LINE_STRIP);
        for (int i=0 ; i <= (nano_max - 1); i++) {
          DBG_ICCGL_S( i<<" "<<Lab_Speicher[i*3]<<"|"<<Lab_Speicher[i*3+1]<<"|"<<Lab_Speicher[i*3+2] )
          DBG_ICCGL_S( i<<" "<<RGB_Speicher[i*3]<<"|"<<RGB_Speicher[i*3+1]<<"|"<<RGB_Speicher[i*3+2] )
          FARBE(RGB_Speicher[i*3],RGB_Speicher[i*3+1],RGB_Speicher[i*3+2]);
          glVertex3d( LabNachXYZv
               (Lab_Speicher[i*3+0], Lab_Speicher[i*3+1], Lab_Speicher[i*3+2]));
        }
      glEnd();

    glEndList();

    if (XYZ_Speicher) delete [] XYZ_Speicher;
    if (RGB_Speicher) delete [] RGB_Speicher;
    if (Lab_Speicher) delete [] Lab_Speicher;

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

  // Löschen
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
  menue_schnitt_->add(_("Schnitt"), 0,c_, (void*)Agviewer::FLYING, 0);
  menue_schnitt_->add(_("Drehen um Schnitt"),0,c_,(void*)Agviewer::ICCPOLAR, 0);
  DBG_PROG

  if (von_farb_namen_.size() >= 3)
  {
    static char text_L[64];
    static char text_a[64];
    static char text_b[64];
    static char text_S[64];
    sprintf (text_L, "%s %s", von_farb_namen_[0].c_str(), _("Schnitt"));
    menue_schnitt_->replace( 0, text_L);
    sprintf (text_a, "%s %s", von_farb_namen_[1].c_str(), _("Schnitt"));
    menue_schnitt_->replace( 1, text_a);
    sprintf (text_b, "%s %s", von_farb_namen_[2].c_str(), _("Schnitt"));
    menue_schnitt_->replace( 2, text_b);
    sprintf (text_S, "%s %s %s", _("Drehen um"),von_farb_namen_[0].c_str(), _("Achse"));
    menue_schnitt_->replace( 4, text_S);
  }
  DBG_PROG

  menue_->add (_("Querschnitt"),0,0, cpMenueButton(menue_schnitt_),FL_SUBMENU_POINTER);

  // ->Darstellung->Hintergrundfarbe
  menue_hintergrund_->add(_("Weiss"), 0,c_, (void*)MENU_WEISS, 0);
  menue_hintergrund_->add(_("Hellgrau"), 0,c_, (void*)MENU_HELLGRAU, 0);
  menue_hintergrund_->add(_("Grau"), 0,c_, (void*)MENU_GRAUGRAU, 0);
  menue_hintergrund_->add(_("Dunkelgrau"), 0,c_, (void*)MENU_DUNKELGRAU, 0);
  menue_hintergrund_->add(_("Schwarz"), 0,c_, (void*)MENU_SCHWARZ, 0);
  DBG_PROG

  menue_form_->add (_("Hintergrundfarbe"),0,c_,cpMenueButton(menue_hintergrund_),FL_SUBMENU_POINTER);

  // ->Darstellung
  if(id_ == 1)
  {
    menue_form_->add( _("grau"), 0,c_, (void*)MENU_GRAU, 0 );
    menue_form_->add( _("farbig"), 0,c_, (void*)MENU_FARBIG, 0 );
    menue_form_->add( _("kontrastreich"), 0,c_, (void*)MENU_KONTRASTREICH, 0 );
    menue_form_->add( _("schalen"), 0,c_, (void*)MENU_SCHALEN, 0 );
  } else {
    // Kugeln mit ihrem Radius symbolisieren Messfarben
    if(zeig_punkte_als_messwerte)
    {
      menue_form_->add( _("Kugel 1dE"), 0,c_, (void*)MENU_dE1KUGEL, 0 );
      menue_form_->add( _("Kugel 2dE"), 0,c_, (void*)MENU_dE2KUGEL, 0 );
      menue_form_->add( _("Kugel 4dE"), 0,c_, (void*)MENU_dE4KUGEL, 0 );
    } else {
    #ifdef Lab_STERN
      menue_form_->add( _("Stern"), 0,c_, (void*)MENU_dE1STERN, 0 );
    #else
      // Punkte werden für Bildfarben reserviert
      menue_form_->add( _("Punkt"), 0,c_, (void*)MENU_dE1STERN, 0 );
    #endif
    }
    menue_form_->add( _("ohne Farborte"), 0,c_, (void*)MENU_DIFFERENZ_LINIE, 0);
    menue_form_->add( _("Spektrallinie"), 0,c_, (void*)MENU_SPEKTRALBAND, 0 );
  }
  DBG_PROG
  menue_form_->add( _("Texte"), 0,c_, (void*)MENU_HELFER, 0 );
  DBG_PROG_V( menue_form_->size() )
  menue_form_->replace( menue_form_->size()-2, _("Texte/Pfeile an/aus"));

  menue_->add (_("Darstellung"),0,c_,cpMenueButton(menue_form_),FL_SUBMENU_POINTER);

  DBG_PROG_V( menue_->size() )


  // -> (Hauptmenü)
  for (int i = 0; i < (int)nach_farb_namen_.size(); i++) {
    char* p = (char*) nach_farb_namen_[i].c_str();
    menue_->add( p, 0,c_, (void*)(MENU_MAX + i), 0 );
    DBG_PROG_V( MENU_MAX + i <<" "<< nach_farb_namen_[i] )
  }

  menue_button_->copy(menue_->menu());
  menue_button_->callback(c_);

  icc_examin_ns::status_info(_("linke-/mittlere-/rechte Maustaste -> Drehen/Schneiden/Menü"));

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


inline void
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
  if(visible() &&
     icc_examin->frei() )
  {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


    glLoadIdentity();
    glMatrixMode(GL_PROJECTION);
    //glLineWidth(3.0*strichmult);

    // Text
    {
      glPushMatrix();
       glLoadIdentity();
       glOrtho(0,w(),0,h(),-10.0,10.0);

       glDisable(GL_TEXTURE_2D);
       glDisable(GL_LIGHTING);
       //glEnable(GL_BLEND);
       //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
       glDisable(GL_LINE_SMOOTH);

       glColor4f(textfarbe[0],
                 textfarbe[1],
                 textfarbe[2], 1.0);

       glTranslatef(5,-12,8.8 - schnitttiefe*3);

       int scal = 120, zeilenversatz = (int)(scal/6.0);
       float strichmult = 1.0;
       std::string text;
       if(id() == 1) {
         text.append(_("Kanal:"));
         text.append(" ");
         text.append(kanalName());
         glTranslatef(0,zeilenversatz,0);

         ZeichneOText (GLUT_STROKE_ROMAN, scal, (char*)text.c_str()) 
       } else {
         for (unsigned int i=0;
                i < dreiecks_netze.size();
                  ++i)
         {
           text = dreiecks_netze[i].name.c_str();
           DBG_PROG_V( dreiecks_netze[i].name )
           glTranslatef(0,zeilenversatz,0);
           ZeichneOText (GLUT_STROKE_ROMAN, scal, text.c_str())
         }
       }

       glEnable(GL_TEXTURE_2D);
       glEnable(GL_LIGHTING);
      glPopMatrix();
    }


    glPushMatrix();  /* clear of last viewing xform, leaving perspective */

    glLoadIdentity();

    setzePerspektive();

    /* so this replaces gluLookAt or equiv */
    agv_.agvViewTransform();

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    if (DrawAxes)
      glCallList(AXES);

    glCallList(dID(SPEKTRUM)); DBG_ICCGL_V( dID(SPEKTRUM) )
    if (zeige_helfer)
      glCallList(dID(HELFER)); DBG_ICCGL_V( dID(HELFER) )
    glCallList(dID(RASTER)); DBG_ICCGL_V( dID(RASTER) )

    if(punktform == MENU_dE1STERN)
      punkteAuffrischen();
    glCallList(dID(PUNKTE)); DBG_ICCGL_V( dID(UNKTE) )

    if(dreiecks_netze.size())
      netzeAuffrischen();


    #if 0
    glFlush();
    #else
    glFinish();
    #endif
  }
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
  valid(false);
  DBG_PROG_ENDE
}

void
GL_Ansicht::hineinPunkte       (std::vector<double>      vect,
                                std::vector<std::string> achs_namen)
{ DBG_PROG_START

  DBG_PROG_V( vect.size() )

  tabelle_.clear();DBG_PROG  
  punkte_.clear(); DBG_PROG
  kurven_.clear(); DBG_PROG

  achsNamen(achs_namen);

  punkte_ = vect;
  
  DBG_PROG_V( zeig_punkte_als_messwert_paare<<"|"<<punktform<<"|"<<punkte_.size() )

  if (!zeig_punkte_als_messwert_paare &&
      punktform != MENU_dE1STERN &&
      punkte_.size())
    punktform = MENU_dE1STERN;

  if (zeig_punkte_als_messwert_paare &&
      punktform == MENU_dE1STERN &&
      punkte_.size())
    punktform = MENU_dE1KUGEL;

  valid(false);
  DBG_PROG_ENDE
}

void
GL_Ansicht::hineinPunkte      (std::vector<double>      vect,
                               std::vector<float>       punkt_farben,
                               std::vector<std::string> achsNamen)
{ DBG_PROG_START
  //Kurve aus tag_browser anzeigen
  farben_.clear();
  farben_ = punkt_farben;
  DBG_PROG_S( farben_.size()/4 << " Farben" )

  hineinPunkte(vect,achsNamen);

  DBG_PROG_ENDE
}

void
GL_Ansicht::hineinPunkte      (std::vector<double> vect,
                               std::vector<float> punkt_farben,
                               std::vector<std::string> farb_namen,
                               std::vector<std::string> achs_namen)
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
  if(d_n.size())
    dreiecks_netze = d_n;

  DBG_NUM_V( dreiecks_netze.size() )
  for(unsigned i = 0; i < dreiecks_netze.size(); ++i)
    DBG_NUM_V( dreiecks_netze[i].name );

  valid(false);
  redraw();
  DBG_PROG_ENDE
}

void
GL_Ansicht::hineinKurven(std::vector<std::vector<double> > vect,
                         std::vector<std::string>          txt)
{ DBG_PROG_START
  //Kurve aus tag_browser anzeigen
  kurven_ = vect;
  nach_farb_namen_ = txt;
  punkte_.clear();

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
  kurven_.clear(); DBG_PROG
  punkte_.clear(); DBG_PROG

  valid(false);
  redraw();

  icc_examin_ns::status_info(_("linke-/mittlere-/rechte Maustaste -> Drehen/Schneiden/Menü"));

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
    switch (value) {
    case MENU_AXES:
      DrawAxes = !DrawAxes;
      break;
    case MENU_QUIT:
      break;
    case MENU_KUGEL:
      punktform = MENU_KUGEL;
      break;
    case MENU_WUERFEL:
      punktform = MENU_WUERFEL;
      break;
    case MENU_STERN:
      punktform = MENU_STERN;
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
       punktform = MENU_DIFFERENZ_LINIE;
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
      hintergrundfarbe = MENU_WEISS;
      for (int i=0; i < 3 ; ++i) pfeilfarbe[i] = 1.;
      for (int i=0; i < 3 ; ++i) textfarbe[i] = .75;
      break;
    case MENU_HELLGRAU:
      hintergrundfarbe = MENU_HELLGRAU;
      for (int i=0; i < 3 ; ++i) pfeilfarbe[i] = 1.0;
      for (int i=0; i < 3 ; ++i) textfarbe[i] = 0.5;
      break;
    case MENU_GRAUGRAU:
      hintergrundfarbe = MENU_GRAUGRAU;
      for (int i=0; i < 3 ; ++i) pfeilfarbe[i] = .75;
      for (int i=0; i < 3 ; ++i) textfarbe[i] = 0.25;
      break;
    case MENU_DUNKELGRAU:
      hintergrundfarbe = MENU_DUNKELGRAU;
      for (int i=0; i < 3 ; ++i) pfeilfarbe[i] = 0.5;
      for (int i=0; i < 3 ; ++i) textfarbe[i] = 0.75;
      break;
    case MENU_SCHWARZ:
      hintergrundfarbe = MENU_SCHWARZ;
      for (int i=0; i < 3 ; ++i) pfeilfarbe[i] = .25;
      for (int i=0; i < 3 ; ++i) textfarbe[i] = 0.5;
      break;
    case Agviewer::FLYING:
      icc_examin_ns::status_info(_("Schnitt; linker Mausklick setzt zurück"));
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
      icc_examin_ns::status_info(_("waagerechter Schnitt; linker Mausklick setzt zurück"));
      agv_.duenn = true;
      break;
    case Agviewer::ICCFLY_a:
      vorder_schnitt = std_vorder_schnitt;
     icc_examin_ns::status_info(_("senkrechter Schnitt von rechts; linker Mausklick setzt zurück"));
      agv_.duenn = true;
      break;
    case Agviewer::ICCFLY_b:
      vorder_schnitt = std_vorder_schnitt;
      icc_examin_ns::status_info(_("senkrechter Schnitt von vorn; linker Mausklick setzt zurück"));
      agv_.duenn = true;
      break;
    case Agviewer::ICCPOLAR:
      agv_.duenn = true;
    case Agviewer::POLAR:
    case Agviewer::AGV_STOP:
      agv_.duenn = false;
      break;
    }

    if (value >= MENU_MAX &&
        value < 100) {
      kanal = value - MENU_MAX; DBG_PROG_V( kanal )
      icc_examin_ns::status_info(_("linke-/mittlere-/rechte Maustaste -> Drehen/Schneiden/Menü"));
    }
  }

  valid(false);
  if(visible()) {
    auffrischen();
    redraw();
  }

  DBG_PROG_V( value )
  DBG_PROG_ENDE
}

void
GL_Ansicht::c_ ( Fl_Widget* w, void* daten )
{ DBG_PROG_START

  int value = (int) daten;
  DBG_PROG_V( value )

  GL_Ansicht *gl_obj = dynamic_cast<GL_Ansicht*>(w->parent());
  DBG_MEM_V( (int)gl_obj )
  DBG_MEM_V( (int)w->parent() )
  if(!w->parent())
    WARN_S(_("Konnte keine Eltern finden"))
  else
  if (gl_obj)
  {
    gl_obj->menueAufruf(value);

    if(value >= 100)
      gl_obj->agv_ .agvSwitchMoveMode (value);
  } else
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
#undef ZeichneBuchstaben
#undef implementGlutFunktionen

