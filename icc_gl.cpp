// Author:    Kai-Uwe Behrmann
// Copyright: Kai-Uwe Behrmann <ku.b@gmx.de>
// Date:      12. 09. 2004


#include "icc_examin.h"
#include "icc_draw.h"
#include "agviewer.h"
#include "icc_gl.h"

#define DEBUG_ICCGL

typedef enum {NOTALLOWED, AXES, RASTER, RING } DisplayLists;
typedef enum { MENU_AXES, MENU_QUIT, MENU_RING } MenuChoices;

int DrawAxes = 0;

#define ROTATEINC 2;

GLfloat Rotation = 0;  /* start ring flat and not spinning */
int     Rotating = 0;



GL_Ansicht::GL_Ansicht(int X,int Y,int W,int H) : Fl_Group(X,Y,W,H)
{ DBG_PROG_V( first )
  first = true;
  DBG_PROG_ENDE
}

void GL_Ansicht::zeigen() {
  if (!first)
    GLFenster->size(w(),h());
  DBG_PROG_V( w() <<" "<< h() )
  agvSetAllowIdle (1);
  GLfenster_zeigen = true;
  DBG_PROG
}

void
GL_Ansicht::verstecken()
{
  if (!first)
    GLFenster->size(1,1);
  DBG_PROG_V( w() <<" "<< h() )
  agvSetAllowIdle (0);
  GLfenster_zeigen = false;
  DBG_PROG
}

void GL_Ansicht::init() {
  DBG_PROG_START
  first = false;
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

  agvInit(1); DBG_PROG

  glutReshapeFunc(reshape); DBG_PROG
  glutDisplayFunc(display); DBG_PROG
  //glutVisibilityFunc(sichtbar); DBG_PROG
  //glutMenuStateFunc(menuuse); DBG_PROG

  agvMakeAxesList(AXES); DBG_PROG

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
  GLfloat mat_ambuse[] = { 0.95, 0.0, 0.0, 1.0 };
  GLfloat mat_specular[] = { 0.4, 0.4, 0.4, 1.0 };

  GLfloat light0_position[] = { 0.6, 0.4, 0.3, 0.0 };

  glLightfv(GL_LIGHT0, GL_POSITION, light0_position);
  glEnable(GL_LIGHTING);
  glEnable(GL_LIGHT0);

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

#define FARBE(r,g,b) farbe [0] = (r); farbe [1] = (g); farbe [2] = (b); \
                     glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, farbe); 

void GL_Ansicht::MakeDisplayLists() {
  char text[256];
  DBG_PROG_START
  glNewList(RASTER, GL_COMPILE);
  GLfloat farbe[] =   { 1.0, 1.0, 1.0, 1.0 };
  // Koordinaten
  #ifdef DEBUG_ICCGL
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glEnable(GL_LINE_SMOOTH);
  glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, farbe);
    glPushMatrix();
      glMatrixMode(GL_MODELVIEW);
      glLineWidth(3.0);
      glTranslatef(-0.3,-0.1,-0.05);
      sprintf (&text[0],_("0,0,0"));
      glScalef(0.001,0.001,0.001);
      for (char* p = &text[0]; *p; p++) {
        glutStrokeCharacter(GLUT_STROKE_ROMAN, *p);
      }
    glPopMatrix();

    glPushMatrix();
      FARBE(1,1,1)
      glBegin(GL_LINES);
        glVertex3f(.1, 0, 0); glVertex3f(0, 0, 0);
      glEnd();
      glBegin(GL_LINES);
        glVertex3f(0, 0, .1); glVertex3f(0, 0, 0);
      glEnd();
      glBegin(GL_LINES);
        glVertex3f(0, .1, 0); glVertex3f(0, 0, 0);
      glEnd();
      glTranslatef(.1,0,0);
      FARBE(1,0,1)
      glRotatef (90,0.0,1.0,.0);
      glutSolidCone(0.01, 0.025, 8, 2);
      glRotatef (-90,0.0,1.0,.0);
      FARBE(1,1,1)
      glTranslatef(.02,0,0);
      glScalef(0.001,0.001,0.001);
      glutStrokeCharacter(GLUT_STROKE_ROMAN, 'X');
    glPopMatrix();

    glPushMatrix();
      glTranslatef(0,.1,0);
      glRotatef (270,1.0,.0,.0);
      FARBE(1,1,0)
      glutSolidCone(0.01, 0.025, 8, 2);
      glRotatef (90,1.0,.0,.0);
      glRotatef (90,0.0,.0,1.0);
      glScalef(0.001,0.001,0.001);
      FARBE(1,1,1)
      glutStrokeCharacter(GLUT_STROKE_ROMAN, 'Y');
    glPopMatrix();

    glPushMatrix();
      glTranslatef(0,0,.1);
      FARBE(0,1,1)
      glutSolidCone(0.01, 0.025, 8, 2);
      FARBE(1,1,1)
      glRotatef (90,0.0,.5,.0);
      glTranslatef(-.1,0,0);
      glScalef(0.001,0.001,0.001);
      glutStrokeCharacter(GLUT_STROKE_ROMAN, 'Z');
    glPopMatrix();
    glLineWidth(1.0);
    #endif

    // Tabelle
    double dim_x = 1.0/(tabelle.size()); DBG_PROG_V( dim_x )
    double dim_y = 1.0/(tabelle[0].size()); DBG_PROG_V( dim_y )
    double dim_z = 1.0/(tabelle[0][0].size()); DBG_PROG_V( dim_z )
    double start_x,start_y,start_z, x,y,z;
    double groesse = (dim_x + dim_y + dim_z)/ 24.0;
    start_x = start_y = start_z = x = y = z = 0.5; start_x = x = -0.5;
    glPushMatrix();

    glTranslatef(start_x,start_y,start_z);
    DBG_PROG_V( tabelle.size() <<" "<< tabelle[0].size() )
    glTranslatef(-dim_x/2.0,-dim_y/2.0,-dim_z/2.0);
    for (int i = 0; i < (int)tabelle.size(); i++) { //DBG_PROG_V( i )
      x = start_x + i * dim_x;
      glTranslatef(dim_x,0.0,0.0);
      glTranslatef(0,-1.0,0.0);
      for (int j = 0; j < (int)tabelle[i].size(); j++) { //DBG_PROG_V( j )
        y = start_y + j * dim_y;
        glTranslatef(0.0, dim_y,0.0);
        glTranslatef(0,0.0,-1.0);
        for (int k = 0; k < (int)tabelle[i][j].size(); k++) { //DBG_PROG_V( k )
          z = start_z + k * dim_z;
          glTranslatef(0.0,0.0,dim_z); //DBG_PROG_S( "xyz: "<< x <<" "<< y <<" "<< z )
          glutSolidCube(groesse);
        }
      }
    }
    glPopMatrix();
  glEndList();

  glNewList(RING, GL_COMPILE);
    glutSolidDodecahedron();
  glEndList();
  DBG_PROG_ENDE
}

void GL_Ansicht::MenuInit() {
  DBG_PROG_START
  int sub2 = glutCreateMenu(agvSwitchMoveMode);   /* pass these right to */
  glutAddMenuEntry(_("Fliegen"),  FLYING); /* agvSwitchMoveMode() */
  glutAddMenuEntry(_("Betrachten"),   POLAR);

  glutCreateMenu(handlemenu);
  glutAddSubMenu(_("Bewegung"), sub2);
  glutAddMenuEntry(_("Achsen ein/aus"), MENU_AXES);
  glutAddMenuEntry(_("Rotation an/aus"), MENU_RING);
  glutAddMenuEntry(_("Beenden"), MENU_QUIT);
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
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  glPushMatrix();  /* clear of last viewing xform, leaving perspective */

  glLoadIdentity();

  gluPerspective(45, seitenverhaeltnis, 0.01, 100);

    /* so this replaces gluLookAt or equiv */
  agvViewTransform();

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

    /* we call agvMakeAxesList() to make this display list */
  if (DrawAxes)
    glCallList(AXES);

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
GL_Ansicht::hinein_tabelle(std::vector<std::vector<std::vector<std::vector<double> > > > vect, std::vector<std::string> txt)
{
  DBG_PROG_START
  //Kurve aus tag_browser anzeigen
  tabelle = vect;  DBG_PROG
  texte = txt; DBG_PROG
  kurven.clear(); DBG_PROG
  punkte.clear(); DBG_PROG

  zeig_mich(this); DBG_PROG_V( first )

  MakeDisplayLists();

  if (first)
    init();

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

  if (agvMoving)   /* we since we are the only idle function, we must */
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
	agvSetAllowIdle(0);             /* and tell AGV to not */
      } else {
	glutIdleFunc(NULL);    /* uninstall our idle function      */
	agvSetAllowIdle(1);    /* and tell AGV it can mess with it */
      }
      break;
    }
  glutPostRedisplay();
  DBG_PROG_ENDE
}


