/*
 * ICC Examin ist eine ICC Profil Betrachter
 * 
 * Copyright (C) 2004  Kai-Uwe Behrmann 
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
 * Der 3D Betrachter.
 * 
 */

// Date:      12. 09. 2004


#include "icc_examin.h"
#include "icc_draw.h"
#include "agviewer.h"
#include "icc_gl.h"

#define DEBUG_ICCGL
//#define Beleuchtung

typedef enum {NOTALLOWED, AXES, RASTER, RING , HELFER } DisplayLists;
bool gl_voll[5] = {false,false,false,false,false};

typedef enum { MENU_AXES, MENU_QUIT, MENU_RING, MENU_KUGEL, MENU_WUERFEL, MENU_STERN, MENU_MAX } MenuChoices;

int DrawAxes = 0;
int kanal = 0;
bool duenn = false;
double Schnitttiefe = 0.1;

#define ROTATEINC 2;

GLfloat Rotation = 0;  /* start ring flat and not spinning */
int     Rotating = 0;



GL_Ansicht::GL_Ansicht(int X,int Y,int W,int H) : Fl_Group(X,Y,W,H)
{ DBG_PROG_START
  first = true;
  MenueKanalEintraege = 0;
  Punktform = MENU_WUERFEL;
  DBG_PROG_ENDE
}

GL_Ansicht::~GL_Ansicht()
{ DBG_PROG_START
  glDeleteLists (RASTER, 1);
  glDeleteLists (HELFER, 1);
  DBG_PROG_ENDE
}

void GL_Ansicht::zeigen() {
  if (first)
    init();
  else
    GLFenster->size(w(),h());

  DBG_PROG_V( w() <<" "<< h() )
  agviewers[agv].agvSetAllowIdle (1);
  GLfenster_zeigen = true;
  DBG_PROG
}

void
GL_Ansicht::verstecken()
{ DBG_PROG_START
  if (!first) {
    agviewers[agv].agvSwitchMoveMode (agviewer::AGV_STOP);
    GLFenster->size(1,1);
  }
  DBG_PROG_V( w() <<" "<< h() )
  GLfenster_zeigen = false; DBG_PROG
  
  DBG_PROG_ENDE
}

void GL_Ansicht::init() {
  DBG_PROG_START

  agv = agviewers.size();
  agviewers.resize( agv + 1 ); DBG_PROG_V( agv )

  first = false;
  MenueKanalEintraege = 0;
  this->begin();
  GLFenster = new Fl_Group (x(),y(),w(),h());
  this->end();

  GLFenster->show();
  this->show();

  DBG_PROG
  GLFenster->begin(); DBG_PROG
  glutInitWindowSize(w(),h()); DBG_PROG_V( w() << h() )
  glutInitWindowPosition(x(),y()); DBG_PROG_V( x() << y() )
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH | GLUT_MULTISAMPLE); DBG_PROG
  glutCreateWindow(_("GL Ansicht")); DBG_PROG

  GLFenster->end(); DBG_PROG
  GLFenster->resizable(glut_window);

  agviewers[agv].agvInit(1); DBG_PROG

  glutReshapeFunc(reshape); DBG_PROG
  glutDisplayFunc(display); DBG_PROG
  //glutVisibilityFunc(sichtbar); DBG_PROG
  //glutMenuStateFunc(menuuse); DBG_PROG

  agviewers[agv].agvMakeAxesList(AXES); DBG_PROG

  myGLinit();  DBG_PROG
  MakeDisplayLists(); DBG_PROG
  MenuInit(); DBG_PROG

  //glutMainLoop(); // you could use Fl::run() instead

  DBG_PROG_ENDE
}

void GL_Ansicht::draw() {
  DBG_PROG_START
  // Kurven oder Punkte malen
  DBG_PROG_S( punkte.size() << "/" << kurven.size() <<" "<< texte.size() )

  if (GLfenster_zeigen) {
    GLFenster->size(w(),h());
  } else {
    GLFenster->size(1,1);
  }

  DBG_PROG_V( GLfenster_zeigen )

  if (punkte.size() >= 3) {
    //draw_cie_shoe(x(),y(),w(),h(),texte,punkte,false);

  } else {
    //draw_kurve   (x(),y(),w(),h(),texte,kurven);
  }
  DBG_PROG
  DBG_PROG_ENDE
}

void GL_Ansicht::myGLinit() {
  DBG_PROG_START
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
#define FARBE(r,g,b) farbe [0] = (r); farbe [1] = (g); farbe [2] = (b); \
                     glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, farbe); 

// Text zeichnen
#define ZeichneText(Font,Zeiger) { \
   glDisable(GL_TEXTURE_2D); \
   glDisable(GL_LIGHTING); \
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
   glEnable(GL_LIGHTING); }
#define ZeichneBuchstaben(Font,Buchstabe) { \
        glScalef(0.001,0.001,0.001); \
          glutStrokeCharacter(Font, Buchstabe); \
          glTranslatef(0.0 - glutStrokeWidth(Font, Buchstabe),0,0); \
        glScalef(1000,1000,1000); }

void
zeichneKoordinaten()
{
  DBG_PROG_START
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
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glEnable(GL_LINE_SMOOTH);

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

void GL_Ansicht::MakeDisplayLists() {
  DBG_PROG_START
  char text[256];

  #define PFEILSPITZE glutSolidCone(0.02, 0.05, 8, 4);

  if (gl_voll[HELFER])
    glDeleteLists (HELFER, 1);
  glNewList(HELFER, GL_COMPILE);
    gl_voll[HELFER] = true;
  GLfloat farbe[] =   { .50, .50, .50, 1.0 };

    // Farbkanalname
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_LINE_SMOOTH);
    glPushMatrix();
      glMatrixMode(GL_MODELVIEW);
      glLineWidth(3.0);
      glTranslatef(.5,0.62,.5);
      glRotatef (270,0.0,1.0,.0);
      char* ptr = (char*) texte[kanal].c_str();
      sprintf (&text[0], ptr);
      ZeichneText(GLUT_STROKE_ROMAN,&text[0])
    glPopMatrix(); DBG_PROG

    // CIE*L - oben
    glPushMatrix();
      FARBE(1,1,1)
      glBegin(GL_LINES);
        glVertex3f(0, .5, 0); glVertex3f(0, -.5, 0);
      glEnd();
      glTranslatef(0,-.5,0);
      glBegin(GL_LINES);
        glVertex3f(0, 0, .5); glVertex3f(0, 0, -.5);
      glEnd();
      glBegin(GL_LINES);
        glVertex3f(.5, 0, 0); glVertex3f(-.5, 0, 0);
      glEnd();
      glTranslatef(0,1.0,0);
      FARBE(1,1,1)
      glRotatef (270,1.0,0.0,.0);
      PFEILSPITZE
      glRotatef (270,0.0,0.0,1.0);
      FARBE(1,1,1)
      glTranslatef(.02,0,0);
      ptr = (char*) pcsNamen[0].c_str();
      sprintf (&text[0], ptr);
      ZeichneText(GLUT_STROKE_ROMAN,&text[0])
    glPopMatrix(); DBG_PROG

    // CIE*a - rechts
    glPushMatrix();
      glTranslatef(0.0,-.5,0.5);
      FARBE(.2,.9,0.7)
      glRotatef (180,0.0,.5,.0);
      glTranslatef(.0,0.0,1.0);
      PFEILSPITZE
      glTranslatef(.0,0.0,-1.0);
      glRotatef (180,0.0,.5,.0);
      FARBE(.9,0.2,0.5)
      PFEILSPITZE
      FARBE(1,1,1)
      ptr = (char*) pcsNamen[1].c_str();
      sprintf (&text[0], ptr);
      ZeichneText(GLUT_STROKE_ROMAN,&text[0])
    glPopMatrix(); DBG_PROG

    // CIE*b - links
    glPushMatrix();
      glTranslatef(.5,-.5,0);
      FARBE(.9,.9,0.2)
      glRotatef (90,0.0,.5,.0);
      PFEILSPITZE
      glRotatef (180,.0,.5,.0);
      glTranslatef(.0,.0,1.0);
      FARBE(.7,.8,1.0)
      PFEILSPITZE
      glTranslatef(.0,.0,-1.0);
      glRotatef (180,0.0,.5,.0);
      FARBE(1,1,1)
      ptr = (char*) pcsNamen[2].c_str();
      sprintf (&text[0], ptr);
      ZeichneText(GLUT_STROKE_ROMAN,&text[0])
    glPopMatrix();
    glLineWidth(1.0);

  glEndList();
  DBG_PROG_V( tabelle.size() )


  // Tabelle
  if (gl_voll[RASTER])
    glDeleteLists (RASTER, 1);

  if (tabelle.size()) {
      glNewList(RASTER, GL_COMPILE);
      gl_voll[RASTER] = true;
      int n_L = tabelle.size(), n_a=tabelle[0].size(), n_b=tabelle[0][0].size();
      double dim_x = 1.0/(n_b); DBG_PROG_V( dim_x )
      double dim_y = 1.0/(n_L); DBG_PROG_V( dim_y )
      double dim_z = 1.0/(n_a); DBG_PROG_V( dim_z )
      double start_x,start_y,start_z, x,y,z;
      double groesse = (dim_x + dim_y + dim_z)/ 3;
      double wert;
              switch (Punktform) {
                case MENU_STERN:   
                Schnitttiefe= HYP3(dim_x,dim_y,dim_z);
                break;
                case MENU_WUERFEL: Schnitttiefe = HYP3(groesse,groesse,groesse);
                break;
                case MENU_KUGEL:   Schnitttiefe = groesse;
                break;
              } DBG_NUM_V( Schnitttiefe );
      start_x = start_y = start_z = x = y = z = 0.5; start_y = y = -0.5;
      glPushMatrix();
      #ifndef Beleuchtung
      glDisable(GL_LIGHTING);
      #endif
      glTranslatef(start_x + dim_x/2.0, start_y + dim_y/2.0, start_z + dim_z/2.0);
      DBG_PROG_V( tabelle.size() <<" "<< tabelle[0].size() )
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
            wert = tabelle[L][a][b][kanal]; //DBG_PROG_V( L << a << b << kanal )
            if (wert) {
              #ifdef Beleuchtung
              FARBE(wert, wert, wert)
              //glColor3f(wert, wert, wert);
              #else
              glColor3f(wert, wert, wert);
              #endif
              switch (Punktform) {
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

  glNewList(RING, GL_COMPILE);
    //glutSolidDodecahedron();
  glEndList();

  //Hintergrund
  #if 1 // Hellgrau
  glClearColor(.75,.75,.75,1.0);
  #else // Blauschwarz
  glClearColor(.0,.0,.1,1.0);
  #endif

  glPopMatrix();
  DBG_PROG_ENDE
}

void
GL_Ansicht::MenueErneuern()
{ DBG_PROG_START
  DBG_PROG_V( MenueKanalEintraege << " "<< glutGet(GLUT_MENU_NUM_ITEMS) )

  int me = glutGet(GLUT_MENU_NUM_ITEMS);
  for (int i = 0; i < MenueKanalEintraege; i++) {
    glutRemoveMenuItem (me - i);
    DBG_PROG_V( me - i )
  }

  MenueKanalEintraege = 0;

  for (int i = 0; i < (int)texte.size(); i++) {
    char* p = (char*) texte[i].c_str();
    glutAddMenuEntry(p, MENU_MAX + i);
    MenueKanalEintraege++;
    DBG_PROG_V( MENU_MAX + i << texte[i] )
  }

  if (kanal > MenueKanalEintraege)
    kanal = MenueKanalEintraege;
  status(_("linke-/mittlere-/rechte Maustaste -> Drehen/Schneiden/Menü"))

  DBG_PROG_ENDE
}

void GL_Ansicht::MenuInit() {
  static char text_L[32];
  static char text_a[32];
  static char text_b[32];
  DBG_PROG_START
  int sub2 = glutCreateMenu(agvSwitchMoveMode);   /* pass these right to */
  sprintf (text_L, "%s %s", pcsNamen[0].c_str(), _("Schnitt"));
  glutAddMenuEntry(text_L,  agviewer::ICCFLY_L);
  sprintf (text_a, "%s %s", pcsNamen[1].c_str(), _("Schnitt"));
  glutAddMenuEntry(text_a,  agviewer::ICCFLY_a);
  sprintf (text_b, "%s %s", pcsNamen[2].c_str(), _("Schnitt"));
  glutAddMenuEntry(text_b,  agviewer::ICCFLY_b); DBG_NUM_V( text_L )
/*  glutAddMenuEntry(_("L Schnitt"),  ICCFLY_L);
  glutAddMenuEntry(_("a Schnitt"),  ICCFLY_a);
  glutAddMenuEntry(_("b Schnitt"),  ICCFLY_b);*/
  glutAddMenuEntry(_("Schnitt"),    agviewer::agviewer::FLYING); /* agvSwitchMoveMode() */
  glutAddMenuEntry(_("Drehen um L-Schnitt"),  agviewer::ICCPOLAR);
//  glutAddMenuEntry(_("Betrachten"),   POLAR);

  int sub3 = glutCreateMenu(handlemenu);
  glutAddMenuEntry(_("Kugel"),  MENU_KUGEL);
  glutAddMenuEntry(_("Würfel"), MENU_WUERFEL);
  glutAddMenuEntry(_("Stern"),  MENU_STERN);
  glutCreateMenu(handlemenu);
  glutAddSubMenu(_("Querschnitte"), sub2);
  //glutAddMenuEntry(_("Achsen ein/aus"), MENU_AXES);
  //glutAddMenuEntry(_("Rotation an/aus"), MENU_RING);
  //glutAddMenuEntry(_("Beenden"), MENU_QUIT);
  glutAddSubMenu(_("Formen"), sub3);


  //MenueErneuern();

  glutAttachMenu(GLUT_RIGHT_BUTTON);
  DBG_PROG_ENDE
}

double seitenverhaeltnis;

void reshape(int w, int h) {
  DBG_PROG_START
  glViewport(0,0,w,h); DBG_PROG_V( w <<" "<< h )
  seitenverhaeltnis = (GLdouble)w/(GLdouble)h;
  glFlush();
  DBG_PROG_ENDE
}

void display() {
  //DBG_PROG_START
  static char text[256];
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

   glLoadIdentity();
   glMatrixMode(GL_PROJECTION);
   //glLineWidth(3.0);

   // Text
   glPushMatrix();
   glLoadIdentity();
   glOrtho(0,mft_gl->w(),0,mft_gl->h(),-10.0,10.0);

   glDisable(GL_TEXTURE_2D);
   glDisable(GL_LIGHTING);
   glEnable(GL_BLEND);
   glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
   glDisable(GL_LINE_SMOOTH);

   glColor4f(.0,0.0,.0,1.0);

   // Start von unten links
   glTranslatef(5,-12,8);

   #define ZeichneOText(font, scal, buffer) glScalef(scal,scal,scal); \
                                 ZeichneText(font,buffer); \
                                 glScalef(1.0/scal,1.0/scal,1.0/scal);

   int scal = 100, zeilenversatz = (int)(scal/6.0);

   #if 0
   for (unsigned int i=0; i < mft_gl->kanaele(); i++) {
     sprintf(&text[0],"%s: %f", mft_gl->kanalName(i), 12.0);
     glTranslatef(0,zeilenversatz,0);
     ZeichneOText (GLUT_STROKE_ROMAN, scal, text)
   }
   #endif

   sprintf(&text[0],"%s: %s", _("sichtbarer Kanal"),mft_gl->kanalName());
   glTranslatef(0,zeilenversatz,0);
   ZeichneOText (GLUT_STROKE_ROMAN, scal, text) 

   glEnable(GL_TEXTURE_2D);
   glEnable(GL_LIGHTING);
  glPopMatrix();

  glPushMatrix();  /* clear of last viewing xform, leaving perspective */

  glLoadIdentity();

  #define ForderSchnitt 4.2

  if (duenn)
    gluPerspective(15, seitenverhaeltnis, ForderSchnitt,
                                          ForderSchnitt + Schnitttiefe);
  else
    gluPerspective(15, seitenverhaeltnis, ForderSchnitt, 15);
                                        // ^-- vordere Schnittfläche

    /* so this replaces gluLookAt or equiv */
  agvViewTransform();

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

    /* we call agvMakeAxesList() to make this display list */
  if (DrawAxes)
    glCallList(AXES);

  glCallList(HELFER);
  glCallList(RASTER);

  glTranslatef(-2, 1, -2);
  glRotatef(Rotation, 1, 0, 0);
  glCallList(RING);


  #if 0
  glutSwapBuffers();
  glFlush();
  #else
  glFinish();
  #endif
  //DBG_PROG_ENDE
}

void GL_Ansicht::hinein_punkt(std::vector<double> vect, std::vector<std::string> txt) {
  DBG_PROG_START
  //CIExyY aus tag_browser anzeigen
  punkte.clear();
  for (unsigned int i = 0; i < vect.size(); i++)
    punkte.push_back (vect[i]);
  texte.clear();
  for (unsigned int i = 0; i < txt.size(); i++)
    texte.push_back (txt[i]);
  kurven.clear();

  zeig_mich(this);
  DBG_PROG_ENDE
}

void GL_Ansicht::hinein_kurven(std::vector<std::vector<double> >vect, std::vector<std::string> txt) {
  DBG_PROG_START
  //Kurve aus tag_browser anzeigen
  kurven = vect;
  texte = txt;
  punkte.clear();

  zeig_mich(this); DBG_PROG_V( first )

  if (first)
    init();

  DBG_PROG_ENDE
}

void
GL_Ansicht::hinein_tabelle(std::vector<std::vector<std::vector<std::vector<double> > > > vect,
                           std::vector<std::string> txt,
                           std::vector<std::string> pcs)
{
  DBG_PROG_START
  //Kurve aus tag_browser anzeigen
  tabelle = vect;  DBG_PROG
  texte = txt; DBG_PROG
  pcsNamen = pcs;
  kurven.clear(); DBG_PROG
  punkte.clear(); DBG_PROG

  zeig_mich(this); DBG_PROG_V( first )


  if (first)
    init();
  else
    MakeDisplayLists();

  MenueErneuern();

  status(_("linke-/mittlere-/rechte Maustaste -> Drehen/Schneiden/Menü"))

  DBG_PROG_ENDE
}

void GL_Ansicht::ruhig_neuzeichnen(void) {
  DBG_PROG_START
  draw_cie_shoe(x(),y(),w(),h(),texte,punkte,true);
  DBG_PROG_ENDE
}


void sichtbar(int v)
{
  DBG_PROG_START
  if (v == GLUT_VISIBLE)
    agvSetAllowIdle(1);
  else {
    glutIdleFunc(NULL);
    agvSetAllowIdle(0);
  }
  DBG_PROG_ENDE
}

void menuuse(int v)
{
  DBG_PROG_START
  if (v == GLUT_MENU_NOT_IN_USE)
    agvSetAllowIdle(1);
  else {
    glutIdleFunc(NULL);
    agvSetAllowIdle(0);
  }
  DBG_PROG_ENDE
}

  /* rotate the axis and adjust position if nec. */
void rotatethering(void)
{ 
  //DBG_PROG_START
  Rotation += ROTATEINC;

  if (agvMoving())   /* we since we are the only idle function, we must */
    agvMove();     /* give AGV the chance to update the eye position */

  glutPostRedisplay();
  //DBG_PROG_ENDE
}


void handlemenu(int value)
{
  DBG_PROG_START
  switch (value) {
    case MENU_AXES:
      DrawAxes = !DrawAxes;
      break;
    case MENU_QUIT:
      DBG_PROG_V( glutGetWindow() )
      glutDestroyWindow(glutGetWindow());
      mft_gl->first = true;
      break;
    case MENU_RING:
      Rotating = !Rotating;
      if (Rotating) {
	glutIdleFunc(rotatethering);    /* install our idle function */
	agvSetAllowIdle(0);         /* and tell AGV to not */
      } else {
	glutIdleFunc(NULL);    /* uninstall our idle function      */
	agvSetAllowIdle(1);/* and tell AGV it can mess with it */
      }
      break;
    case MENU_KUGEL:
      mft_gl-> Punktform = MENU_KUGEL;
      mft_gl->MakeDisplayLists();
      break;
    case MENU_WUERFEL:
      mft_gl-> Punktform = MENU_WUERFEL;
      mft_gl->MakeDisplayLists();
      break;
    case MENU_STERN:
      mft_gl-> Punktform = MENU_STERN;
      mft_gl->MakeDisplayLists();
      break;
    }

  if (value >= MENU_MAX) {
    kanal = value - MENU_MAX; DBG_PROG_V( kanal )
    status(_("linke-/mittlere-/rechte Maustaste -> Drehen/Schneiden/Menü"))
    mft_gl->MakeDisplayLists();
  }

  DBG_PROG_V( value )
  glutPostRedisplay();
  DBG_PROG_ENDE
}


