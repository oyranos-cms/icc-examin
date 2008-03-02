/*
 * fractviewer.cxx [from agviewer.c  (version 1.0)]
 *
 * AGV: a glut viewer. Routines for viewing a 3d scene w/ glut
 *
 * See agv_example.c and agviewer.h comments within for more info.
 *
 * I welcome any feedback or improved versions!
 *
 * Philip Winston - 4/11/95
 * pwinston@hmc.edu
 * http://www.cs.hmc.edu/people/pwinston
 */

#  include <FL/glut.H>
#  ifdef __APPLE__
#    include <OpenGL/glu.h>
#  else
#    include <GL/glu.h> // added for FLTK
#  endif

#  include <stdio.h>
#  include <stdlib.h>
#  include <math.h>
#  include <sys/types.h>
#  include <time.h>
#  if !defined(WIN32) && !defined(__EMX__)
#    include <sys/time.h>
#  endif // !WIN32 && !__EMX__

#  include "agviewer.h"

// ICC Kopfdateien und Definitionen
#include "icc_utils.h"
#include "icc_betrachter.h"
#include "icc_gl.h"
#define _(text) text

/* Some <math.h> files do not define M_PI... */
#ifndef M_PI
#define M_PI 3.14159265
#endif


std::vector<agviewer> agviewers;

/***************************************************************/
/************************ agvInit ******************************/
/***************************************************************/

void
agviewer::agvInit(int window)
{ DBG_PROG_START
  glutMouseFunc(agvHandleButton);
  glutMotionFunc(agvHandleMotion);
  glutKeyboardFunc(agvHandleKeys);
  RedisplayWindow = glutGetWindow(); DBG_PROG_V( RedisplayWindow )
  agvSetAllowIdle(window);
  DBG_PROG_ENDE
}

/***************************************************************/
/************************ VIEWPOINT STUFF **********************/
/***************************************************************/

  /*
   * viewing transformation modified from page 90 of red book
   */
void
agviewer::PolarLookFrom(GLfloat dist, GLfloat elevation, GLfloat azimuth)
{ DBG_PROG_START
  glTranslatef(0, 0, -dist);
  glRotatef(elevation, 1, 0, 0);
  glRotatef(azimuth, 0, 1, 0);

  DBG_PROG_ENDE
}

  /*
   * I took the idea of tracking eye position in absolute
   * coords and direction looking in Polar form from denis
   */
void
agviewer::FlyLookFrom(GLfloat x, GLfloat y, GLfloat z, GLfloat az, GLfloat el)
{ DBG_PROG_START
  float lookat[3], perp[3], up[3];

  lookat[0] = sin(TORAD(az))*cos(TORAD(el));
  lookat[1] = sin(TORAD(el));
  lookat[2] = -cos(TORAD(az))*cos(TORAD(el));
  normalize(lookat);
  perp[0] = lookat[2];
  perp[1] = 0;
  perp[2] = -lookat[0];
  normalize(perp);
  ncrossprod(lookat, perp, up);
  gluLookAt(x, y, z,
            x+lookat[0], y+lookat[1], z+lookat[2],
            up[0], up[1], up[2]);
  DBG_PROG_ENDE
}

  /*
   * Call viewing transformation based on movement mode
   */
void
agviewer::agvViewTransform(void)
{ DBG_PROG_START
  switch (MoveMode) {
    case FLYING:
      FlyLookFrom(Ex, Ey, Ez, EyeAz, EyeEl);
      break;
    case POLAR:
      PolarLookFrom(EyeDist, EyeEl, EyeAz);
      break;
    }
  DBG_PROG_ENDE
}

  /*
   * keep them vertical; I think this makes a lot of things easier, 
   * but maybe it wouldn't be too hard to adapt things to let you go
   * upside down
   */
int
agviewer::ConstrainEl(void)
{ DBG_PROG_START
  if (EyeEl <= -90) {
    EyeEl = -89.99;
    DBG_PROG_ENDE
    return 1;
  } else if (EyeEl >= 90) {
    EyeEl = 89.99;
    DBG_PROG_ENDE
    return 1;
  }
  DBG_PROG_ENDE
  return 0;
}

 /*
  * Idle Function - moves eyeposition
  */
void
agviewer::_agvMove(void)
{ DBG_PROG_START
  switch (MoveMode)  {
    case FLYING:
      Ex += EyeMove*sin(TORAD(EyeAz))*cos(TORAD(EyeEl));
      Ey += EyeMove*sin(TORAD(EyeEl));
      Ez -= EyeMove*cos(TORAD(EyeAz))*cos(TORAD(EyeEl));
      break;

    case POLAR:
      EyeEl += ElSpin;
      EyeAz += AzSpin;
      if (ConstrainEl()) {  /* weird spin thing to make things look     */
        ElSpin = -ElSpin;      /* look better when you are kept from going */
                               /* upside down while spinning - Isn't great */
        if (fabs(ElSpin) > fabs(AzSpin))
          AzSpin = fabs(ElSpin) * ((AzSpin > 0) ? 1 : -1);
      }
      break;
    }

  if (AdjustingAzEl) {
    dAz *= slow_daz;
    dEl *= slow_del;
  }

  if (AllowIdle) {
    glutSetWindow(RedisplayWindow); DBG_PROG_V( RedisplayWindow )
    glutPostRedisplay();
  }
  DBG_PROG_ENDE
}


  /*
   * Don't install agvMove as idle unless we will be updating the view
   * and we've been given a RedisplayWindow
   */
void
agviewer::MoveOn(int v)
{ DBG_PROG_START
  if (v && ((MoveMode == FLYING && EyeMove != 0) ||
             (MoveMode == POLAR &&
             (AzSpin != 0 || ElSpin != 0 || AdjustingAzEl)))) {
    agvMoving = 1;
    if (AllowIdle)
      glutIdleFunc(agvMove);
  } else {
    agvMoving = 0;
    if (AllowIdle)
      glutIdleFunc(NULL);
  }
  DBG_PROG_ENDE
}

  /*
   * set new redisplay window.  If <= 0 it means we are not to install
   * an idle function and will rely on whoever does install one to 
   * put statement like "if (agvMoving) agvMove();" at end of it
   */
void
agviewer::agvSetAllowIdle(int allowidle)
{ DBG_PROG_START
  if ((AllowIdle = allowidle))
    MoveOn(1);
  DBG_PROG_ENDE
}

  /*
   * when moving to flying we stay in the same spot, moving to polar we
   * reset since we have to be looking at the origin (though a pivot from
   * current position to look at origin might be cooler)
   */
void
agviewer::agvSwitchMoveMode(int move)
{ DBG_PROG_START
  switch (move) {
    case FLYING:
      if (MoveMode == FLYING) return;
      Ex    = -EyeDist*sin(TORAD(EyeAz))*cos(TORAD(EyeEl));
      Ey    =  EyeDist*sin(TORAD(EyeEl));
      Ez    =  EyeDist*(cos(TORAD(EyeAz))*cos(TORAD(EyeEl)));
      EyeAz =  EyeAz;
      EyeEl = -EyeEl;
      EyeMove = init_move;
      status(_("Schnitt; linker Mausklick setzt zurück"));
      duenn = true;
      break;
    case ICCFLY_L:
      MoveMode = POLAR;
      EyeDist = init_dist;
      EyeAz   = init_polar_az;
      EyeEl   = init_polar_el;
      AzSpin  = init_az_spin;
      ElSpin  = init_el_spin;
      move = FLYING;
      agvSwitchMoveMode( FLYING );
      status(_("waagerechter Schnitt; linker Mausklick setzt zurück"));
      duenn = true;
      break;
    case ICCFLY_a:
      MoveMode = POLAR;
      EyeDist = init_dist;
      EyeAz   = 0;
      EyeEl   = 0;
      AzSpin  = init_az_spin;
      ElSpin  = init_el_spin;
      move = FLYING;
      agvSwitchMoveMode( FLYING );
      status(_("senkrechter Schnitt von rechts; linker Mausklick setzt zurück"));
      duenn = true;
      break;
    case ICCFLY_b:
      MoveMode = POLAR;
      EyeDist = init_dist;
      EyeAz   = 270;
      EyeEl   = 0;
      AzSpin  = init_az_spin;
      ElSpin  = init_el_spin;
      move = FLYING;
      agvSwitchMoveMode( FLYING );
      status(_("senkrechter Schnitt von vorn; linker Mausklick setzt zurück"));
      duenn = true;
      break;
    case ICCPOLAR:
      move = POLAR;
      EyeDist = 4.2; // der Schnittabstand
      //EyeAz   = 0;
      EyeEl   = 0;
      AzSpin  = 1.0;
      ElSpin  = init_el_spin;
      duenn = true;
      break;
    case POLAR:
      EyeDist = init_dist;
      EyeAz   = init_polar_az;
      EyeEl   = init_polar_el;
      AzSpin  = init_az_spin;
      ElSpin  = init_el_spin;
      duenn = false;
      break;
    case AGV_STOP:
      MoveMode = POLAR;
      AzSpin  = 0;
      ElSpin  = 0;
      move = POLAR;
      duenn = false;
      break;
    }
  MoveMode = move;
  MoveOn(1);
  glutPostRedisplay();
  DBG_PROG_ENDE
}

/***************************************************************/
/*******************    MOUSE HANDLING   ***********************/
/***************************************************************/

void
agviewer::_agvHandleButton(int &button, int &state, int &x, int &y)
{ DBG_PROG_START
  DBG_PROG_V( button <<" "<< state);

 if (state == GLUT_DOWN && downb == -1) {  
    lastx = downx = x;
    lasty = downy = y;
    downb = button;    

    switch (button) {
      case GLUT_LEFT_BUTTON:
        if (MoveMode == FLYING)
          EyeEl = -EyeEl;
        lastEl = downEl = EyeEl;
        lastAz = downAz = EyeAz;
        AzSpin = ElSpin = dAz = dEl = 0;
        AdjustingAzEl = 1;
	MoveOn(0); //ICC stop
        if (MoveMode == FLYING)
          status(_("linke-/mittlere-/rechte Maustaste -> Drehen/Schneiden/Menü"));
        duenn = false;
        MoveMode = POLAR;
        break;

      case GLUT_MIDDLE_BUTTON:
        downDist = EyeDist;
	downEx = Ex;
	downEy = Ey;
	downEz = Ez;
	downEyeMove = EyeMove;
	EyeMove = 0;
        if (MoveMode == FLYING)
          status(_("Pause"));
        duenn = false;
    }

  } else if (state == GLUT_UP && button == downb) {

    downb = -1;

    switch (button) {
      case GLUT_LEFT_BUTTON:
        if (MoveMode != FLYING) {
	  AzSpin =  -dAz;
	  if (AzSpin < min_azspin && AzSpin > -min_azspin)
	    AzSpin = 0;	
	  ElSpin = -dEl;
	  if (ElSpin < min_elspin && ElSpin > -min_elspin)
	    ElSpin = 0; 
	}
        AdjustingAzEl = 0;
        MoveOn(1);
        if (MoveMode == FLYING)
          status(_("linke-/mittlere-/rechte Maustaste -> Drehen/Schneiden/Menü"));
        duenn = false;
	break;

      case GLUT_MIDDLE_BUTTON:
	EyeMove = downEyeMove;
        if (MoveMode == FLYING)
          status(_("linke Maustaste -> zurück"));
        duenn = true;
      }
  }
  DBG_PROG_ENDE
}

 /*
  * change EyeEl and EyeAz and position when mouse is moved w/ button down
  */
void
agviewer::_agvHandleMotion(int &x, int &y)
{ DBG_PROG_START
  int deltax = x - downx, deltay = y - downy;

  switch (downb) {
    case GLUT_LEFT_BUTTON:
      EyeEl  = downEl + el_sens * deltay;
      ConstrainEl();
      EyeAz  = downAz + az_sens * deltax;
      dAz    = prev_daz*dAz + cur_daz*(lastAz - EyeAz);
      dEl    = prev_del*dEl + cur_del*(lastEl - EyeEl);
      lastAz = EyeAz;
      lastEl = EyeEl;
      break;
    case GLUT_MIDDLE_BUTTON:
        EyeDist = downDist + dist_sens*deltay;
        Ex = downEx - e_sens*deltay*sin(TORAD(EyeAz))*cos(TORAD(EyeEl));
        Ey = downEy - e_sens*deltay*sin(TORAD(EyeEl));
        Ez = downEz + e_sens*deltay*cos(TORAD(EyeAz))*cos(TORAD(EyeEl));
      break;
  }
  glutPostRedisplay();
  DBG_PROG_ENDE
}

/***************************************************************/
/********************* KEYBOARD HANDLING ***********************/
/***************************************************************/

  /*
   * set EyeMove (current speed) for FLYING mode
   */
void
agviewer::SetMove(float newmove)
{ DBG_PROG_START
  if (newmove > minmove) {
    EyeMove = newmove;
    MoveOn(1);
  } else {
    EyeMove = 0;
    MoveOn(0);
  }
  DBG_PROG_ENDE
}

  /*
   * 0->9 set speed, +/- adjust current speed  -- in FLYING mode
   */
void
agviewer::_agvHandleKeys(unsigned char key, int&, int&)
{ DBG_PROG_START
  if (MoveMode != FLYING)
  { DBG_PROG_ENDE
    return;
  }

  if (key >= '0' && key <= '9')
    SetMove(SPEEDFUNCTION((key-'0')));
  else
    switch(key) {
      case '+':  
        if (EyeMove == 0)
          SetMove(minmove);
         else
	  SetMove(EyeMove *= (1 + movefraction));
        break;
      case '-':
	SetMove(EyeMove *= (1 - movefraction));
        break;
    }
  DBG_PROG_ENDE
}

/***************************************************************/
/*********************** VECTOR STUFF **************************/
/***************************************************************/

  /* normalizes v */
void
agviewer::normalize(GLfloat v[3])
{ DBG_PROG_START
  GLfloat d = sqrt(v[0]*v[0] + v[1]*v[1] + v[2]*v[2]);

  if (d == 0)
    fprintf(stderr, "Zero length vector in normalize\n");
  else
    v[0] /= d; v[1] /= d; v[2] /= d;
  DBG_PROG_ENDE
}

  /* calculates a normalized crossproduct to v1, v2 */
void
agviewer::ncrossprod(float v1[3], float v2[3], float cp[3])
{ DBG_PROG_START
  cp[0] = v1[1]*v2[2] - v1[2]*v2[1];
  cp[1] = v1[2]*v2[0] - v1[0]*v2[2];
  cp[2] = v1[0]*v2[1] - v1[1]*v2[0];
  normalize(cp);
  DBG_PROG_ENDE
}

/***************************************************************/
/**************************** AXES *****************************/
/***************************************************************/


  /* draw axes -- was helpful to debug/design things */
void
agviewer::agvMakeAxesList(int displaylistnum)
{ DBG_PROG_START
  int i,j;
  GLfloat axes_ambuse[] =   { 0.5, 0.0, 0.0, 1.0 };
  glNewList(displaylistnum, GL_COMPILE);
  glPushAttrib(GL_LIGHTING_BIT);
  glMatrixMode(GL_MODELVIEW);
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, axes_ambuse);
    glBegin(GL_LINES);
      glVertex3f(15, 0, 0); glVertex3f(-15, 0, 0);
      glVertex3f(0, 15, 0); glVertex3f(0, -15, 0);
      glVertex3f(0, 0, 15); glVertex3f(0, 0, -15);
    glEnd();
    for (i = 0; i < 3; i++) {
      glPushMatrix();
        glTranslatef(-10*(i==0), -10*(i==1), -10*(i==2));
        for (j = 0; j < 21; j++) {
//          glutSolidCube(0.1);
          glTranslatef(i==0, i==1, i==2);
	}
      glPopMatrix();
    }
  glPopAttrib();
  glEndList();  
  DBG_PROG_ENDE
}


// variable Argumentlänge in einem Macro ?
void agvMove(void)
{ DBG_PROG_START
  std::vector<agviewer>::iterator it;
  for (it = agviewers.begin() ; it != agviewers.end(); it++)
    if (it->redisplayWindow() == glutGetWindow())
      it->_agvMove();
  DBG_PROG_ENDE
}

void agvHandleButton(int button, int state, int x, int y)
{ DBG_PROG_START
  std::vector<agviewer>::iterator it;
  for (it = agviewers.begin() ; it != agviewers.end(); it++)
    if (it->redisplayWindow() == glutGetWindow())
      it->_agvHandleButton( button, state, x, y);
  DBG_PROG_ENDE
}

void agvHandleMotion(int x, int y)
{ DBG_PROG_START
  std::vector<agviewer>::iterator it; DBG_PROG_V( agviewers.size() )
  for (it = agviewers.begin() ; it != agviewers.end(); it++) {
    DBG_PROG_V( it->redisplayWindow() )
    if (it->redisplayWindow() == glutGetWindow())
      it->_agvHandleMotion( x, y);
  }
  DBG_PROG_ENDE
}

void agvHandleKeys(unsigned char key, int x, int y)
{ DBG_PROG_START
  std::vector<agviewer>::iterator it;
  for (it = agviewers.begin() ; it != agviewers.end(); it++)
    if (it->redisplayWindow() == glutGetWindow())
      it->_agvHandleKeys( key, x, y);
  DBG_PROG_ENDE
}

void agvViewTransform(void)
{ DBG_PROG_START
  std::vector<agviewer>::iterator it;
  for (it = agviewers.begin() ; it != agviewers.end(); it++)
    if (it->redisplayWindow() == glutGetWindow())
      it->agvViewTransform();
  DBG_PROG_ENDE
}

void agvSwitchMoveMode(int move)
{ DBG_PROG_START
  std::vector<agviewer>::iterator it;
  for (it = agviewers.begin() ; it != agviewers.end(); it++)
    if (it->redisplayWindow() == glutGetWindow())
      it->agvSwitchMoveMode( move);
  DBG_PROG_ENDE
}

void agvSetAllowIdle(int allowidle)
{ DBG_PROG_START
  std::vector<agviewer>::iterator it;
  for (it = agviewers.begin() ; it != agviewers.end(); it++)
    if (it->redisplayWindow() == glutGetWindow())
      it->agvSetAllowIdle( allowidle);
  DBG_PROG_ENDE
}

int agvMoving(void)
{ DBG_PROG_START
  std::vector<agviewer>::iterator it;
  for (it = agviewers.begin() ; it != agviewers.end(); it++)
    if (it->redisplayWindow() == glutGetWindow()) {
      DBG_PROG_ENDE
      return it->agvMoving;
    }
  
  DBG_PROG_ENDE
  return -1;
}


