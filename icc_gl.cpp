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

// Das glutWindow System ist komplitziert für Klassen und mehrere Fenster
// o Agviewer in agvviewers werden 0-1 gezählt
// o gl_ansichten in icc_examin werden auch von 0-1 gezählt haben aber eine
//   interne Nummer glut_id_

#include "icc_examin.h"
#include "icc_draw.h"
#include "icc_utils.h"
#include "icc_betrachter.h"
#include "agviewer.h"
#include "icc_gl.h"

#include <cmath>

//#define Beleuchtung

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
 MENU_RING,
 MENU_KUGEL,           // Formen der 3DLut
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
 MENU_GRAUGRAU,        // Hintergrundfarben
 MENU_WEISS,
 MENU_SCHWARZ,
 MENU_HELLGRAU,
 MENU_DUNKELGRAU,
 MENU_MAX
} MenuChoices;

std::vector<GL_Ansicht*> gl_ansichten;

int DrawAxes = 0;

#define ROTATEINC 2;

GLfloat Rotation = 0;  /* start ring flat and not spinning */
int     Rotating = 0;

#define bNachX(b) (b*b_darstellungs_breite - b_darstellungs_breite/2.)
#define LNachY(L) (L - 0.5)
#define aNachZ(a) (a*a_darstellungs_breite - a_darstellungs_breite/2.)
#define LabNachXYZv(L,a,b) \
         (b*b_darstellungs_breite - b_darstellungs_breite/2.), \
         (L - 0.5), \
         (a*a_darstellungs_breite - a_darstellungs_breite/2.)


GL_Ansicht::GL_Ansicht(int X,int Y,int W,int H) : Fl_Group(X,Y,W,H)
{ DBG_PROG_START
  kanal = 0;
  schnitttiefe = 0.1;
  beruehrt_ = false;
  auffrischen_ = true;
  menue_kanal_eintraege_ = 0;
  a_darstellungs_breite = 1.0;
  b_darstellungs_breite = 1.0;
  schalen = 5;
  spektralband = 0;
  zeige_helfer = true;
  gl_fenster_zeigen_ = false;
  zeig_punkte_als_messwert_paare = false;
  glut_id_ = -1;
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
GL_Ansicht::init()
{ DBG_PROG_START

  if (beruehrt_) {
    DBG_PROG_ENDE
    return;
  }

  if(this == icc_examin->icc_betrachter->mft_gl)
    DBG_PROG_S( "init mft_gl" )
  if(this == icc_examin->icc_betrachter->DD_histogram)
    DBG_PROG_S( "init DD_histogram" )

  agv_ = agviewers.size(); DBG_PROG
  agviewers.resize (agv_ +1);

  beruehrt_ = true;
  menue_kanal_eintraege_ = 0;
  this->begin();
  gl_fenster_ = new Fl_Group (x(),y(),w(),h());
  this->end();
  DBG_PROG
  gl_fenster_->show();

  if (!this->visible())
    WARN_S("Diese GL_Ansicht ist nicht sichtbar")
  this->show();

  DBG_PROG
  gl_fenster_->begin(); DBG_PROG
  glutInitWindowSize(w(),h()); DBG_PROG_V( w() << h() )
  glutInitWindowPosition(x(),y()); DBG_PROG_V( x() << y() )
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH | GLUT_MULTISAMPLE);
  DBG_PROG
  glut_id_ = glutCreateWindow(_("GL Ansicht")); DBG_PROG_V( glut_id_ )

  gl_fenster_->end(); DBG_PROG
  gl_fenster_->resizable(glut_window);

  agviewers[agv_].agvInit(glut_id_);
  DBG_PROG_V( agv_ <<" "<< agviewers[agv_].redisplayWindow() )

  #define setzeGlutFunktionen(n) \
  glutReshapeFunc(reshape##n); DBG_PROG \
  glutDisplayFunc(display##n); DBG_PROG \
  /*glutVisibilityFunc(sichtbar##n); DBG_PROG \
  glutMenuStateFunc(menuuse##n); DBG_PROG */

  if (glut_id_ == 1) { DBG_PROG_S("mft_gl " << glut_id_)
    setzeGlutFunktionen(1)
  } else { DBG_PROG_S("gl Fenster " << glut_id_)
    setzeGlutFunktionen(2)
    a_darstellungs_breite = 2.55;
    b_darstellungs_breite = 2.55;
    agviewers[agv_].distA (agviewers[agv_].distA()
                           + a_darstellungs_breite/2.0 - 0.5);
    agviewers[agv_].distB (agviewers[agv_].distB()
                           + b_darstellungs_breite/2.0 - 0.5);
  }

  agviewers[agv_].agvMakeAxesList(AXES); DBG_PROG

  myGLinit_();  DBG_PROG
  menuInit_(); DBG_PROG
  makeDisplayLists_(); DBG_PROG_V( id() )

  icc_examin->glAnsicht (this);

  // Initialisieren
  handlemenu (id(), MENU_HELLGRAU); // Farbschema
  handlemenu (id(), MENU_GRAU);     // CLUT Farbschema
  if (id() == 1) handlemenu (id(), MENU_WUERFEL);
  else           handlemenu (id(), MENU_DIFFERENZ_LINIE);

  //glutMainLoop(); // you could use Fl::run() instead

  DBG_PROG_ENDE
}

void
GL_Ansicht::zeigen()
{ DBG_PROG_START
  DBG_PROG_V( id() )

  if(!beruehrt_) icc_examin->initReihenfolgeGL_Ansicht(this);

  //glutSetWindow( agviewers[agv_].redisplayWindow() );
 
  if (!gl_fenster_zeigen_) { DBG_PROG
    gl_fenster_->resize(x(),y(),w(),h());
  }

  DBG_PROG_V( id() <<" "<< x()<<" "<< y()<<" "<< w()<<" "<< h() )

  agviewers[agv_].agvSetAllowIdle (1);
  gl_fenster_zeigen_ = true; DBG_PROG_V( glut_id_ )
  reshape(glut_id_, w(),h());
  auffrischen();
  //glutPostRedisplay();
  DBG_PROG_ENDE
}

void
GL_Ansicht::verstecken()
{ DBG_PROG_START
  DBG_PROG_V( id() )

  //DBG_PROG_V( w() <<" "<< h() )
  gl_fenster_zeigen_ = false; DBG_PROG

  if (!beruehrt_) {
    //icc_examin->initReihenfolgeGL_Ansicht(this);
  } else {
    //agviewers[agv_].agvSwitchMoveMode (Agviewer::AGV_STOP);
    gl_fenster_->size(1,1);
    stop();
    DBG_PROG_S("-------------- GL Fenster auf 1x1 verkleinert ----------------")
  }

  
  DBG_PROG_ENDE
}

void
GL_Ansicht::auffrischen()
{ DBG_PROG_START
  menueErneuern_();
  makeDisplayLists_();
  auffrischen_ = false;
  DBG_PROG_ENDE
}

void
GL_Ansicht::draw()
{ DBG_PROG_START
  if (gl_fenster_zeigen_)
  { DBG_PROG
    gl_fenster_->size(w(),h());
    if (auffrischen_) {
      auffrischen();
    }
  } else { DBG_PROG
    gl_fenster_->size(1,1);
    DBG_PROG_S("-------------- GL Fenster auf 1x1 verkleinert ----------------")
  }
  DBG_PROG_V( gl_fenster_zeigen_ <<"|"<< agv_ <<"|"<< (int)gl_fenster_ <<"|"<< agviewers[agv_].redisplayWindow() )

  DBG_PROG_ENDE
}

void
GL_Ansicht::myGLinit_()
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

  glShadeModel(GL_SMOOTH);

  glFlush();
  DBG_PROG_ENDE
}

// Materialfarben setzen
#define FARBE(r,g,b) {farbe [0] = (r); farbe [1] = (g); farbe [2] = (b); \
                      glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, farbe); \
                      glColor4f(farbe[0],farbe[1],farbe[2],1.0); }

// Text zeichnen
#define ZeichneText(Font,Zeiger) { \
   glDisable(GL_TEXTURE_2D); \
   glDisable(GL_LIGHTING); \
   glLineWidth(2); \
      glTranslatef(.0,0,0.01); \
        glScalef(0.001,0.001,0.001); \
          for (char* p = Zeiger; *p; p++) { \
             glutStrokeCharacter(Font, *p); \
          } \
          for (char* p = Zeiger; *p; p++) { \
            glTranslatef(0.0 - glutStrokeWidth(Font, *p),0,0); \
          } \
        glScalef(1000,1000,1000); \
      glTranslatef(.0,0,-.01); \
   glEnable(GL_TEXTURE_2D); \
   glEnable(GL_LIGHTING); \
   glLineWidth(1.0); }

#define ZeichneBuchstaben(Font,Buchstabe) { \
        glScalef(0.001,0.001,0.001); \
          glutStrokeCharacter(Font, Buchstabe); \
          glTranslatef(0.0 - glutStrokeWidth(Font, Buchstabe),0,0); \
        glScalef(1000,1000,1000); }

void
zeichneKoordinaten()
{ DBG_PROG_START
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
          ZeichneBuchstaben(GLUT_STROKE_ROMAN, 'Z')
        glTranslatef(.1,0,0);
      glRotatef (270,0.0,.5,.0);
    glTranslatef(0,0,-.1);
  DBG_PROG_ENDE
}

void
GL_Ansicht::makeDisplayLists_()
{ DBG_PROG_START
  char text[256];
  char* ptr = 0;

  #define PFEILSPITZE glutSolidCone(0.02, 0.05, 16, 4);


  // Pfeile und Text
  if (gl_voll[HELFER])
    glDeleteLists (id()*DL_MAX + HELFER, 1);
  glNewList(id()*DL_MAX + HELFER, GL_COMPILE); DBG_PROG_V( id()*DL_MAX + HELFER )
    gl_voll[HELFER] = true;
    GLfloat farbe[] =   { textfarbe[0],textfarbe[1],textfarbe[2], 1.0 };

    // Farbkanalname
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_LINE_SMOOTH);
    glEnable(GL_LIGHTING);

    glPushMatrix();
      glMatrixMode(GL_MODELVIEW);
      glLineWidth(1.0);
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
    glLineWidth(1.0);

  glEndList();
  DBG_PROG_V( tabelle_.size() )


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
  } else if (dreiecks_netze_.size()) {

    if( dreiecks_netze_.size() )
      DBG_PROG_V( dreiecks_netze_[0].size() )
    if( dreiecks_farben_.size() )
      DBG_PROG_V( dreiecks_farben_[0].size() )
    if( netz_namen_.size() )
      DBG_PROG_V( netz_namen_[0].size() )

      glNewList(id()*DL_MAX + RASTER, GL_COMPILE); DBG_PROG_V( id()*DL_MAX + RASTER )
      gl_voll[RASTER] = true;
      #ifndef Beleuchtung
      glDisable(GL_LIGHTING);
      #endif

      #if 1
      glEnable (GL_BLEND);
      glEnable (GL_DEPTH_TEST);
      glBlendFunc (GL_SRC_COLOR, GL_DST_ALPHA);
      //glEnable (GL_ALPHA_TEST_FUNC);
      //glAlphaFunc (GL_ALPHA_TEST, GL_ONE_MINUS_DST_ALPHA);
      #endif

      glColor3f(0.9, 0.9, 0.9);
      glPushMatrix();
      glTranslatef( -.5, -.5, -.5 );
      for(unsigned int j = 0; j < dreiecks_netze_.size(); j++ )
      { glBegin(GL_TRIANGLE_STRIP);
        DBG_PROG_V( j )
        for(unsigned int i = 0; i < dreiecks_netze_[j].size()-5; i=i+6 )
        { //DBG_PROG_V( i )
          if ( dreiecks_farben_.size() > j &&
               dreiecks_farben_[j].size() > dreiecks_netze_[j].size() )
          { //DBG_PROG_V( i/6*4+3 )
            glColor4f( dreiecks_farben_[j][i/6*4+0],dreiecks_farben_[j][i/6*4+1],
                       dreiecks_farben_[j][i/6*4+2],dreiecks_farben_[j][i/6*4+3]);
          }
          glVertex3d( dreiecks_netze_[j][i+1],
                      dreiecks_netze_[j][i+0],
                      dreiecks_netze_[j][i+2] );
          if ( dreiecks_farben_.size() > j &&
               dreiecks_farben_[j].size() > dreiecks_netze_[j].size() )
          { //DBG_PROG_V( i/6*4+3 )
            glColor4f( dreiecks_farben_[j][i/6*4+4],dreiecks_farben_[j][i/6*4+5],
                       dreiecks_farben_[j][i/6*4+6],dreiecks_farben_[j][i/6*4+7]);
          }
          glVertex3d( dreiecks_netze_[j][i+4],
                      dreiecks_netze_[j][i+3],
                      dreiecks_netze_[j][i+5] );
        }
        glEnd();
      }
      glPopMatrix();
      #ifndef Beleuchtung
      glEnable(GL_LIGHTING);
      #endif
      glEndList();
  }
 
  if (gl_voll[SPEKTRUM])
    glDeleteLists (id()*DL_MAX + SPEKTRUM, 1);
  if (spektralband == MENU_SPEKTRALBAND)
    zeigeSpektralband_();

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
  // Blauschwarz
  //glClearColor(.0,.0,.1,1.0);

  glPopMatrix();
  glutPostRedisplay();
  DBG_PROG_ENDE
}

void
GL_Ansicht::punkteAuffrischen()
{ DBG_PROG_START

  if (gl_voll[PUNKTE])
    glDeleteLists (id()*DL_MAX + PUNKTE, 1);

  //Koordinaten  in CIE*b CIE*L CIE*a Reihenfolge 
  if (punkte_.size()) {

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
      glEnable (GL_DEPTH_TEST);
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
        glTranslatef( -b_darstellungs_breite/2, -.5, -a_darstellungs_breite/2 );
        // Punktkoordinaten setzen
        glTranslatef( punkte_[i+2]*b_darstellungs_breite,
                      punkte_[i+0], punkte_[i+1]*a_darstellungs_breite );
        if(zeig_punkte_als_messwert_paare &&
           i%6 == 0)
        {
          glLineWidth(2.0);
          glColor4f(1., 1., 1., 1.0 );
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
            //glEnable (GL_DEPTH_TEST);
            //glBlendFunc (GL_SRC_COLOR, GL_DST_ALPHA);
          }
          glColor4f(farben_[i/3*4+0], farben_[i/3*4+1], farben_[i/3*4+2],
                    farben_[i/3*4+3] );
        }

        double groesse = 0.01;
        switch (punktform)
        {
          case MENU_dE1STERN:
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
            break;
          case MENU_dE1KUGEL: glutSolidSphere (0.005, 12, 12); break;
          case MENU_dE2KUGEL: glutSolidSphere (0.01, 12, 12); break;
          case MENU_dE4KUGEL: glutSolidSphere (0.02, 12, 12); break;
          case MENU_DIFFERENZ_LINIE: break;
        }
        glPopMatrix();
      }
      #if 0 // Test
      glColor4f(0.8,0.2,0.2,0.5);
      glutSolidSphere (0.7, 36, 36);
      glColor4f(0.2,0.2,0.8,0.5);
      glutSolidSphere (0.5, 36, 36);
      #endif
      glEnable (GL_BLEND);
      glEnable (GL_DEPTH_TEST);
      glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
      #ifndef Beleuchtung
      glEnable(GL_LIGHTING);
      #endif
    glEndList();
  }

  DBG_PROG_ENDE
}

extern float cieXYZ [471][3]; // in 
#include <lcms.h>

#define TYPE_COLOUR_DBL (COLORSPACE_SH(PT_ANY)|CHANNELS_SH(3)|BYTES_SH(0))
#define PRECALC cmsFLAGS_NOTPRECALC 
#if 0
#define BW_COMP cmsFLAGS_WHITEBLACKCOMPENSATION
#else
#define BW_COMP 0
#endif

void
GL_Ansicht::zeigeSpektralband_()
{ DBG_PROG_START

  // lcms Typen
  cmsHPROFILE hsRGB;
  cmsHPROFILE hLab;
  cmsHTRANSFORM hLabtoRGB;
  double *RGB_Speicher = 0;
  double *XYZ_Speicher = 0;
  double *Lab_Speicher = 0;

  // Initialisierung für lcms
  hsRGB = cmsCreate_sRGBProfile(); // TODO Monitorprofil
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

  #if 0
  cmsHPROFILE hXYZ;
  cmsHTRANSFORM hXYZtoLab;
  hXYZ  = cmsCreateXYZProfile();
  if(!hXYZ)  WARN_S( _("hXYZ Profil nicht geöffnet") )
  hXYZtoLab = cmsCreateTransform          (hXYZ, TYPE_XYZ_DBL,
                                           hLab, TYPE_COLOUR_DBL,
                                           INTENT_ABSOLUTE_COLORIMETRIC,
                                           PRECALC|BW_COMP);
  if (!hXYZtoLab) WARN_S( _("keine hXYZtoLab Transformation gefunden") )
  for(int i = 0; i < nano_max; ++i)
    cmsDoTransform (hXYZtoLab, &XYZ_Speicher[i*3], &Lab_Speicher[i*3], 1);
  cmsDoTransform (hXYZtoLab, XYZ_Speicher, Lab_Speicher, nano_max);
  #else

  XYZtoLab (XYZ_Speicher, Lab_Speicher, nano_max);
  #endif

  DBG_PROG_V( nano_max )
  double *cielab = new double[nano_max*3];
  LabToCIELab (Lab_Speicher, cielab, nano_max);
  cmsDoTransform (hLabtoRGB, cielab, RGB_Speicher, nano_max);
  if (cielab) delete [] cielab;


  GLfloat farbe[] =   { pfeilfarbe[0],pfeilfarbe[1],pfeilfarbe[2], 1.0 };

  DBG_PROG_V( id()*DL_MAX + SPEKTRUM ) 
  glNewList(id()*DL_MAX + SPEKTRUM, GL_COMPILE);
    gl_voll[SPEKTRUM] = true;

    #if 0
    glEnable (GL_BLEND);
    glEnable (GL_DEPTH_TEST);
    glBlendFunc (GL_SRC_COLOR, GL_DST_ALPHA);
    #else
    glDisable(GL_LIGHTING);
    glDisable (GL_BLEND);
    glEnable  (GL_DEPTH_TEST);
    glDisable (GL_ALPHA_TEST_FUNC);
    glEnable  (GL_LINE_SMOOTH);
    #endif

    glLineWidth(3.0);
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

  DBG_PROG_ENDE
}


void
GL_Ansicht::menueErneuern_()
{ DBG_PROG_START
  DBG_PROG_V( menue_kanal_eintraege_ << " "<< glutGet(GLUT_MENU_NUM_ITEMS) )

  glutSetMenu (menue_schnitt_); DBG_PROG_V( menue_schnitt_ )
  if (von_farb_namen_.size() >= 3)
  {
    static char text_L[64];
    static char text_a[64];
    static char text_b[64];
    static char text_S[64];
    sprintf (text_L, "%s %s", von_farb_namen_[0].c_str(), _("Schnitt"));
    glutChangeToMenuEntry(1, text_L, Agviewer::ICCFLY_L);
    sprintf (text_a, "%s %s", von_farb_namen_[1].c_str(), _("Schnitt"));
    glutChangeToMenuEntry(2, text_a, Agviewer::ICCFLY_a);
    sprintf (text_b, "%s %s", von_farb_namen_[2].c_str(), _("Schnitt"));
    glutChangeToMenuEntry(3, text_b, Agviewer::ICCFLY_b);
    sprintf (text_S, "%s %s %s", _("Drehen um"),von_farb_namen_[0].c_str(), _("Achse"));
    glutChangeToMenuEntry(5,text_S,  Agviewer::ICCPOLAR);
  }
  glutSetMenu (menue_); DBG_PROG_V( menue_ )

  int me = glutGet(GLUT_MENU_NUM_ITEMS);
  for (int i = 0; i < menue_kanal_eintraege_; i++) {
    glutRemoveMenuItem (me - i);
    DBG_PROG_V( me - i )
  }

  menue_kanal_eintraege_ = 0;

  for (int i = 0; i < (int)nach_farb_namen_.size(); i++) {
    char* p = (char*) nach_farb_namen_[i].c_str();
    glutAddMenuEntry(p, MENU_MAX + i);
    menue_kanal_eintraege_++;
    DBG_PROG_V( MENU_MAX + i << nach_farb_namen_[i] )
  }

  if (kanal >= menue_kanal_eintraege_)
    kanal = menue_kanal_eintraege_ - 1;

  DBG_PROG_V( menue_kanal_eintraege_ << kanal )
  status(_("linke-/mittlere-/rechte Maustaste -> Drehen/Schneiden/Menü"))

  DBG_PROG_ENDE
}

void
GL_Ansicht::menuInit_()
{ DBG_PROG_START
  if (glut_id_ == 1) { menue_ = glutCreateMenu(handlemenu1); }
  else {               menue_ = glutCreateMenu(handlemenu2); }
  DBG_PROG_V( menue_ )
  menue_schnitt_ = glutCreateMenu(agvSwitchMoveMode); DBG_PROG_V(menue_schnitt_)
  if (glut_id_ == 1) { menue_hintergrund_ = glutCreateMenu(handlemenu1); }
  else {               menue_hintergrund_ = glutCreateMenu(handlemenu2); }
  if (glut_id_ == 1) { menue_form_ = glutCreateMenu(handlemenu1); }
  else {               menue_form_ = glutCreateMenu(handlemenu2); }

  // Querschnitte
  glutSetMenu(menue_schnitt_);
  glutAddMenuEntry("text_L",  Agviewer::ICCFLY_L);
  glutAddMenuEntry("text_a",  Agviewer::ICCFLY_a);
  glutAddMenuEntry("text_b",  Agviewer::ICCFLY_b);
  glutAddMenuEntry(_("Schnitt"), Agviewer::FLYING); /* agvSwitchMoveMode() */
  glutAddMenuEntry(_("Drehen um Schnitt"),  Agviewer::ICCPOLAR);

  // Hintergrundfarben
  glutSetMenu(menue_hintergrund_);
  glutAddMenuEntry(_("Weiss"), MENU_WEISS);
  glutAddMenuEntry(_("Hellgrau"), MENU_HELLGRAU);
  glutAddMenuEntry(_("Grau"), MENU_GRAUGRAU);
  glutAddMenuEntry(_("Dunkelgrau"), MENU_DUNKELGRAU);
  glutAddMenuEntry(_("Schwarz"), MENU_SCHWARZ);

  glutSetMenu(menue_form_);
  glutAddSubMenu(_("Hintergrundfarbe"), menue_hintergrund_);

  glutSetMenu(menue_);
  glutAddSubMenu(_("Querschnitte"), menue_schnitt_);
  glutAddSubMenu(_("Darstellung"), menue_form_);

  if(glut_id_ == 1) {
    // Darstellung
    glutSetMenu(menue_form_);
    glutAddMenuEntry(_("grau"),  MENU_GRAU);
    glutAddMenuEntry(_("farbig"),  MENU_FARBIG);
    glutAddMenuEntry(_("kontrastreich"),  MENU_KONTRASTREICH);
    glutAddMenuEntry(_("schalen"),  MENU_SCHALEN);
    glutSetMenu(menue_);
  } else {
    glutSetMenu(menue_form_);
    glutAddMenuEntry(_("Kugel 1dE"), MENU_dE1KUGEL);
    glutAddMenuEntry(_("Kugel 2dE"), MENU_dE2KUGEL);
    glutAddMenuEntry(_("Kugel 4dE"), MENU_dE4KUGEL);
    glutAddMenuEntry(_("Stern"),     MENU_dE1STERN);
    glutAddMenuEntry(_("ohne Farborte"), MENU_DIFFERENZ_LINIE);
    glutAddMenuEntry(_("Spektrallinie"), MENU_SPEKTRALBAND);
  }

  glutSetMenu(menue_form_);
  glutAddMenuEntry(_("Texte/Pfeile an/aus"), MENU_HELFER);

  //menueErneuern_();

  glutSetMenu(menue_);
  glutAttachMenu(GLUT_RIGHT_BUTTON);
  DBG_PROG_ENDE
}

// alle Funktionen müssen noch für jedes Fenster angemeldet werden

#define implementGlutFunktionen(n) \
void reshape##n( int w, int h ) {reshape(n, w, h); } \
void display##n() {display(n); } \
void sichtbar##n(int v) {sichtbar(n, v); } \
void menuuse##n(int v) {menuuse(n, v); } \
void handlemenu##n(int value) {handlemenu(n, value); }

implementGlutFunktionen(1)
implementGlutFunktionen(2)

void
reshape(int id, int w, int h)
{ DBG_PROG_START
  if(icc_examin->glAnsicht(id)->sichtbar()) {
    glutSetWindow(id);
    glViewport(0,0,w,h); DBG_PROG_V( id <<"|"<< icc_examin->glAnsicht(id)->x()<<" "<<icc_examin->glAnsicht(id)->y()<<" "<<w<<" "<<h )
    glutPositionWindow(icc_examin->glAnsicht(id)->x(),icc_examin->glAnsicht(id)->y());
    icc_examin->glAnsicht(id)->seitenverhaeltnis = (GLdouble)w/(GLdouble)h;
    glFlush();
  }
  DBG_PROG_ENDE
}

void
display(int id)
{ DBG_ICCGL_START
  if(icc_examin->glAnsicht(id)->sichtbar()) {
    glutSetWindow(id);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glLoadIdentity();
    glMatrixMode(GL_PROJECTION);
    //glLineWidth(3.0);

    // Text
    if (id == 1) {
      glPushMatrix();
       glLoadIdentity();
       glOrtho(0,icc_examin->glAnsicht(id)->w(),0,icc_examin->glAnsicht(id)->h(),-10.0,10.0);

       glDisable(GL_TEXTURE_2D);
       glDisable(GL_LIGHTING);
       //glEnable(GL_BLEND);
       //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
       glDisable(GL_LINE_SMOOTH);

       glColor4f(icc_examin->glAnsicht(id)->textfarbe[0],
                 icc_examin->glAnsicht(id)->textfarbe[1],
                 icc_examin->glAnsicht(id)->textfarbe[2], 1.0);

       glTranslatef(5,-12,8.8-icc_examin->glAnsicht(id)->schnitttiefe*3);

       // Start von unten links
       #define ZeichneOText(font, scal, buffer) glScalef(scal,scal,scal); \
                                   ZeichneText(font,buffer); \
                                   glScalef(1.0/scal,1.0/scal,1.0/scal);


       int scal = 120, zeilenversatz = (int)(scal/6.0);
       #if 0
       static char text[256];
       for (unsigned int i=0; i < icc_examin->icc_betrachter->mft_gl->kanaele(); i++) {
         sprintf(&text[0],"%s: %f", icc_examin->icc_betrachter->mft_gl->kanalName(i), 12.0);
         glTranslatef(0,zeilenversatz,0);
         ZeichneOText (GLUT_STROKE_ROMAN, scal, text)
       }
       //sprintf(&text[0],"%s: %s", _("sichtbarer Kanal"),icc_examin->glAnsicht(id)->kanalName());
       #endif
       std::string text;
       text.append(_("Kanal:"));
       text.append(" ");
       text.append(icc_examin->glAnsicht(id)->kanalName());
       glTranslatef(0,zeilenversatz,0);

       ZeichneOText (GLUT_STROKE_ROMAN, scal, (char*)text.c_str()) 

       glEnable(GL_TEXTURE_2D);
       glEnable(GL_LIGHTING);
      glPopMatrix();
    }


    glPushMatrix();  /* clear of last viewing xform, leaving perspective */

    glLoadIdentity();

    #define ForderSchnitt 4.2

    if (agviewers[icc_examin->glAnsicht(id)->agv()].duenn)
      gluPerspective(15, icc_examin->glAnsicht(id)->seitenverhaeltnis,
                     ForderSchnitt,
                     ForderSchnitt + icc_examin->glAnsicht(id)->schnitttiefe);
    else
      gluPerspective(15, icc_examin->glAnsicht(id)->seitenverhaeltnis,
                     ForderSchnitt, 50);
                  // ^-- vordere Schnittfläche

    /* so this replaces gluLookAt or equiv */
    agvViewTransform();

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    if (DrawAxes)
      glCallList(AXES);

    glCallList(dID(id,SPEKTRUM)); DBG_ICCGL_V( dID(id,SPEKTRUM) )
    if (icc_examin->glAnsicht(id)->zeige_helfer)
      glCallList(dID(id,HELFER)); DBG_ICCGL_V( dID(id,HELFER) )
    glCallList(dID(id,RASTER)); DBG_ICCGL_V( dID(id,RASTER) )
    glCallList(dID(id,PUNKTE)); DBG_ICCGL_V( dID(id,PUNKTE) )


    #if 0
    glutSwapBuffers();
    glFlush();
    #else
    glFinish();
    #endif
  }
  DBG_ICCGL_ENDE
}

void
GL_Ansicht::hineinPunkte       (std::vector<double>      vect,
                                std::vector<std::string> achsNamen)
{ DBG_PROG_START

  DBG_PROG_V( vect.size() )

  tabelle_.clear();DBG_PROG  
  punkte_.clear(); DBG_PROG
  kurven_.clear(); DBG_PROG
  von_farb_namen_.clear();
  if (achsNamen.size() == 3)
    von_farb_namen_ = achsNamen;
  else
  {
    von_farb_namen_.push_back ("?");
    von_farb_namen_.push_back ("?");
    von_farb_namen_.push_back ("?");
  }

  punkte_ = vect;
  DBG_PROG_V( punkte_.size() )

  if (!zeig_punkte_als_messwert_paare &&
      punktform == MENU_DIFFERENZ_LINIE &&
      punkte_.size())
    punktform = MENU_dE1STERN;

  //icc_examin->neuzeichnen(this);
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
GL_Ansicht::hineinNetze       (std::vector<std::vector<double> >dreiecksNetze, 
                               std::vector<std::vector<float> > dreiecksFarben,
                               std::vector<std::string> netzNamen,
                               std::vector<std::string> achsNamen)
{ DBG_PROG_START
  // Farbraumhülle laden
  DBG_PROG_V( tabelle_.size() )
  DBG_PROG_V( dreiecks_netze_.size() )
  DBG_PROG_V( kurven_.size() )

  if (tabelle_.size()) tabelle_.clear();DBG_PROG  
  if (dreiecks_netze_.size()) dreiecks_netze_.clear(); DBG_PROG
  if (kurven_.size()) kurven_.clear(); DBG_PROG
  if (achsNamen.size() == 3)
    von_farb_namen_ = achsNamen;
  else
  { von_farb_namen_.clear();
    von_farb_namen_.push_back ("?");
    von_farb_namen_.push_back ("?");
    von_farb_namen_.push_back ("?");
  }

  dreiecks_netze_  = dreiecksNetze;
  dreiecks_farben_ = dreiecksFarben;
  netz_namen_      = netzNamen;
  if( dreiecks_netze_.size() )
    DBG_PROG_V( dreiecks_netze_[0].size() )
  if( dreiecks_farben_.size() )
    DBG_PROG_V( dreiecks_farben_[0].size() )
  if( netz_namen_.size() )
    DBG_PROG_V( netz_namen_[0].size() )

  //icc_examin->neuzeichnen(this);
  DBG_PROG_ENDE
}


void GL_Ansicht::hineinKurven(std::vector<std::vector<double> >vect, std::vector<std::string> txt)
{ DBG_PROG_START
  //Kurve aus tag_browser anzeigen
  kurven_ = vect;
  nach_farb_namen_ = txt;
  punkte_.clear();

  //icc_examin->neuzeichnen(this); DBG_PROG_V( beruehrt_ )

  DBG_PROG_ENDE
}

void
GL_Ansicht::hineinTabelle (std::vector<std::vector<std::vector<std::vector<double> > > > vect,
                           std::vector<std::string> von,
                           std::vector<std::string> nach)
{ DBG_PROG_START
  //Kurve aus tag_browser anzeigen
  tabelle_ = vect;  DBG_PROG
  nach_farb_namen_ = nach; DBG_PROG
  if (von.size() == 3)
    von_farb_namen_ = von;
  else
  { von_farb_namen_.clear();
    von_farb_namen_.push_back ("?");
    von_farb_namen_.push_back ("?");
    von_farb_namen_.push_back ("?");
  }
  kurven_.clear(); DBG_PROG
  punkte_.clear(); DBG_PROG

  //icc_examin->neuzeichnen(this); DBG_PROG_V( beruehrt_ )


  if (!beruehrt_)
    icc_examin->initReihenfolgeGL_Ansicht(this);
  else
    auffrischen_ = true;

  status(_("linke-/mittlere-/rechte Maustaste -> Drehen/Schneiden/Menü"))

  DBG_PROG_ENDE
}

void
sichtbar (int id, int v)
{ DBG_PROG_START
  if(icc_examin->glAnsicht(id)->sichtbar()) {
    glutSetWindow(id);
    if (v == GLUT_VISIBLE)
      agvSetAllowIdle(1);
    else {
      glutIdleFunc(NULL);
      agvSetAllowIdle(0);
    }
  }
  DBG_PROG_ENDE
}

void
menuuse (int id, int v)
{ DBG_PROG_START
  if(icc_examin->glAnsicht(id)->sichtbar()) {
    glutSetWindow(id);
    if (v == GLUT_MENU_NOT_IN_USE)
      agvSetAllowIdle(1);
    else {
      glutIdleFunc(NULL);
      agvSetAllowIdle(0);
    }
  }
  DBG_PROG_ENDE
}


void
handlemenu (int id, int value)
{ DBG_PROG_START

  if(icc_examin->glAnsicht(id)->sichtbar()) {
    DBG_PROG_S( "sichtbar "<< id )
    glutSetWindow(id);
  }

  {
    switch (value) {
    case MENU_AXES:
      DrawAxes = !DrawAxes;
      break;
    case MENU_QUIT:
      DBG_PROG_V( glutGetWindow() )
      //glutDestroyWindow(glutGetWindow());
      //icc_examin->glAnsicht(id)->beruehrt_ = false;
      break;
    case MENU_RING:
      Rotating = !Rotating;
      if (Rotating) {
	//glutIdleFunc(rotatethering);    /* install our idle function */
	agvSetAllowIdle(0);         /* and tell AGV to not */
      } else {
	glutIdleFunc(NULL);    /* uninstall our idle function      */
	agvSetAllowIdle(1);/* and tell AGV it can mess with it */
      }
      break;
    case MENU_KUGEL:
      icc_examin->glAnsicht(id)->punktform = MENU_KUGEL;
      break;
    case MENU_WUERFEL:
      icc_examin->glAnsicht(id)-> punktform = MENU_WUERFEL;
      break;
    case MENU_STERN:
      icc_examin->glAnsicht(id)-> punktform = MENU_STERN;
      break;
    case MENU_GRAU:
      icc_examin->glAnsicht(id)-> punktfarbe = MENU_GRAU;
      break;
    case MENU_FARBIG:
      icc_examin->glAnsicht(id)-> punktfarbe = MENU_FARBIG;
      break;
    case MENU_KONTRASTREICH:
      icc_examin->glAnsicht(id)-> punktfarbe = MENU_KONTRASTREICH;
      break;
    case MENU_SCHALEN:
      if (!icc_examin->glAnsicht(id)-> schalen)
        icc_examin->glAnsicht(id)-> schalen = 5;
      else
        icc_examin->glAnsicht(id)-> schalen = 0;
      break;
    case MENU_dE1STERN:
      icc_examin->glAnsicht(id)-> punktform = MENU_dE1STERN;
      break;
    case MENU_dE1KUGEL:
      icc_examin->glAnsicht(id)-> punktform = MENU_dE1KUGEL;
      break;
    case MENU_dE2KUGEL:
      icc_examin->glAnsicht(id)-> punktform = MENU_dE2KUGEL;
      break;
    case MENU_dE4KUGEL:
      icc_examin->glAnsicht(id)-> punktform = MENU_dE4KUGEL;
      break;
    case MENU_DIFFERENZ_LINIE:
      icc_examin->glAnsicht(id)-> punktform = MENU_DIFFERENZ_LINIE;
      break;
    case MENU_SPEKTRALBAND:
      if (!icc_examin->glAnsicht(id)-> spektralband)
        icc_examin->glAnsicht(id)-> spektralband = MENU_SPEKTRALBAND;
      else
        icc_examin->glAnsicht(id)-> spektralband = 0;
      break;
    case MENU_HELFER:
      if (!icc_examin->glAnsicht(id)-> zeige_helfer)
        icc_examin->glAnsicht(id)-> zeige_helfer = MENU_HELFER;
      else
        icc_examin->glAnsicht(id)-> zeige_helfer = 0;
      break;
    case MENU_WEISS:
      icc_examin->glAnsicht(id)-> hintergrundfarbe = MENU_WEISS;
      for (int i=0; i < 3 ; ++i) icc_examin->glAnsicht(id)->pfeilfarbe[i] = 1.;
      for (int i=0; i < 3 ; ++i) icc_examin->glAnsicht(id)->textfarbe[i] = .75;
      break;
    case MENU_HELLGRAU:
      icc_examin->glAnsicht(id)-> hintergrundfarbe = MENU_HELLGRAU;
      for (int i=0; i < 3 ; ++i) icc_examin->glAnsicht(id)->pfeilfarbe[i] = 1.0;
      for (int i=0; i < 3 ; ++i) icc_examin->glAnsicht(id)->textfarbe[i] = 0.5;
      break;
    case MENU_GRAUGRAU:
      icc_examin->glAnsicht(id)-> hintergrundfarbe = MENU_GRAUGRAU;
      for (int i=0; i < 3 ; ++i) icc_examin->glAnsicht(id)->pfeilfarbe[i] = .75;
      for (int i=0; i < 3 ; ++i) icc_examin->glAnsicht(id)->textfarbe[i] = 0.25;
      break;
    case MENU_DUNKELGRAU:
      icc_examin->glAnsicht(id)-> hintergrundfarbe = MENU_DUNKELGRAU;
      for (int i=0; i < 3 ; ++i) icc_examin->glAnsicht(id)->pfeilfarbe[i] = 0.5;
      for (int i=0; i < 3 ; ++i) icc_examin->glAnsicht(id)->textfarbe[i] = 0.75;
      break;
    case MENU_SCHWARZ:
      icc_examin->glAnsicht(id)-> hintergrundfarbe = MENU_SCHWARZ;
      for (int i=0; i < 3 ; ++i) icc_examin->glAnsicht(id)->pfeilfarbe[i] = .25;
      for (int i=0; i < 3 ; ++i) icc_examin->glAnsicht(id)->textfarbe[i] = 0.5;
      break;
    }

    if (value >= MENU_MAX) {
      icc_examin->glAnsicht(id)->kanal = value - MENU_MAX; DBG_PROG_V( icc_examin->glAnsicht(id)->kanal )
      status(_("linke-/mittlere-/rechte Maustaste -> Drehen/Schneiden/Menü"))
    }
  }

  if(icc_examin->glAnsicht(id)->sichtbar()) {
    icc_examin->glAnsicht(id)->auffrischen();
  }

  DBG_PROG_V( value )
  DBG_PROG_ENDE
}

  /* rotate the axis and adjust position if nec. */
void
rotatethering (void)
{ 
  //DBG_PROG_START
  Rotation += ROTATEINC;

//  if (agvMoving())   /* we since we are the only idle function, we must */
  //  agvMove();     /* give AGV the chance to update the eye position */

  glutPostRedisplay();
  //DBG_PROG_ENDE
}

int
dID (int id, int display_list)
{ DBG_ICCGL_V( id <<"|"<<  glutGetWindow() )
  return id*DL_MAX + display_list;
}


