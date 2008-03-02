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


//#  include <stdio.h>
//#  include <stdlib.h>
//#  include <math.h>
//#  include <sys/types.h>
//#  include <time.h>

#  if !defined(WIN32) && !defined(__EMX__)
//#    include <sys/time.h>
#  endif // !WIN32 && !__EMX__

#  include "agviewer.h"
#include "icc_gl.h"

#include <cmath>
#include <sstream>

// ICC Kopfdateien und Definitionen
#include "icc_utils.h"
#include "icc_examin.h"
#include "icc_info.h"

/* Some <math.h> files do not define M_PI... */
#ifndef M_PI
#define M_PI 3.14159265
#endif


std::vector<Agviewer> agviewers;

/***************************************************************/
/************************ agvInit ******************************/
/***************************************************************/

void
Agviewer::agvInit(int window)
{ DBG_PROG_START
  RedisplayWindow = window;
  this->agvSetAllowIdle(window);
  duenn = false; // ICC Schnitt - Kai-Uwe
  DBG_PROG_ENDE
}

/***************************************************************/
/************************ VIEWPOINT STUFF **********************/
/***************************************************************/

  /*
   * viewing transformation modified from page 90 of red book
   */
void
Agviewer::PolarLookFrom(GLfloat dist, GLfloat elevation, GLfloat azimuth)
{ DBG_PROG_START
  glTranslatef(0, 0, -dist);
  glRotatef(elevation, 1, 0, 0);
  glRotatef(azimuth, 0, 1, 0);
  DBG_PROG_V( dist )
  DBG_PROG_ENDE
}

  /*
   * I took the idea of tracking eye position in absolute
   * coords and direction looking in Polar form from denis
   */
void
Agviewer::FlyLookFrom(GLfloat x, GLfloat y, GLfloat z, GLfloat az, GLfloat el)
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
Agviewer::agvViewTransform(void)
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
Agviewer::ConstrainEl(void)
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

void
Agviewer::setIdle(bool set)
{
  DBG_PROG_V( set )
  if(set) {
    Fl::add_idle(agvMove_statisch,this);
    AllowIdle = true;
  } else {
    Fl::remove_idle(agvMove_statisch,this);
    AllowIdle = false;
  }
}

 /*
  * Idle Function - moves eyeposition
  */
void
Agviewer::agvMove_statisch(void* agv)
{
  Agviewer *obj = (Agviewer*)agv;
  if (obj)
    obj->agvMove_();
  else
    WARN_S( _("kein Agviewer uebergeben; kann agvMove_ nicht ausfuehren") )

  if(!obj->parent->visible())
    obj->setIdle(0);
}
void
Agviewer::agvMove_(void)
{ DBG_PROG_START
  int sl = 0;
  static double rz = 0;
  DBG_PROG_V(redisplayWindow())
  if(icc_examin->frei())
  {
  switch (MoveMode)  {
    case FLYING:
      Ex += EyeMove*sin(TORAD(EyeAz))*cos(TORAD(EyeEl));
      Ey += EyeMove*sin(TORAD(EyeEl));
      Ez -= EyeMove*cos(TORAD(EyeAz))*cos(TORAD(EyeEl));
      if(fabs(EyeDist+0.01) < fabs(EyeDist)) {
        int button = FL_BUTTON1,
            event = FL_PUSH,
            x=0, y=0;
        agvHandleButton(button,event,x,y);;
      }
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
      parent->redraw();

      // 50 fps TODO: missing precission
      // clock_gettime is not available
/*__uint64_t readTSC (void)
{
  struct timespec tp;
  clock_gettime (CLOCK_SGI_CYCLE, &tp);
  return (__uint64_t)(tp.tv_sec * (__uint64_t)1000000000) + (__uint64_t)tp.tv_nsec;
}*/
      double tmp = (double)clock()/(double)CLOCKS_PER_SEC;
      double z = tmp - rz;
      rz = tmp;
      sl = 25000-(int)(z*1000000.0);
      if (sl>5000)
        ;//usleep(sl);
      DBG_PROG_S( clock() << "z: " << z*1000000. << " sl " << sl )
      DBG_PROG_S( "AllowIdle: " << AllowIdle )
    }
  } else
    WARN_S( _("GL Fenster nicht frei") )

  DBG_PROG_ENDE
}


  /*
   * Don't install agvMove as idle unless we will be updating the view
   * and we've been given a RedisplayWindow
   */
void
Agviewer::MoveOn(int v)
{ DBG_PROG_START
  if (v && ((MoveMode == FLYING && EyeMove != 0) ||
             (MoveMode == POLAR &&
             (AzSpin != 0 || ElSpin != 0 || AdjustingAzEl))))
  {
    agvMoving = 1;
    if (AllowIdle) {
        DBG_PROG_S( "idle: agvMove_statisch" )
        Fl::add_idle(agvMove_statisch,this);
        agvMove_();
    }
  } else {
    agvMoving = 0;
    if (AllowIdle) {
      Fl::remove_idle(agvMove_statisch,this);
      DBG_PROG_S( "idle: ---" )
    }
  }
  DBG_PROG_ENDE
}

  /*
   * set new redisplay window.  If <= 0 it means we are not to install
   * an idle function and will rely on whoever does install one to 
   * put statement like "if (agvMoving) agvMove();" at end of it
   */
void
Agviewer::agvSetAllowIdle(int allowidle)
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
Agviewer::agvSwitchMoveMode(int move)
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
      break;
    case ICCFLY_L:
      MoveMode = POLAR;
      EyeAz   = init_polar_az;
      EyeEl   = init_polar_el;
      AzSpin  = init_az_spin;
      ElSpin  = init_el_spin;
      move = FLYING;
      this->agvSwitchMoveMode( FLYING );
      break;
    case ICCFLY_a:
      MoveMode = POLAR;
      EyeDist = init_dist_a;
      EyeAz   = 0;
      EyeEl   = 0;
      AzSpin  = init_az_spin;
      ElSpin  = init_el_spin;
      move = FLYING;
      this->agvSwitchMoveMode( FLYING );
      break;
    case ICCFLY_b:
      MoveMode = POLAR;
      EyeDist = init_dist_b;
      EyeAz   = 270;
      EyeEl   = 0;
      AzSpin  = init_az_spin;
      ElSpin  = init_el_spin;
      move = FLYING;
      this->agvSwitchMoveMode( FLYING );
      break;
    case ICCPOLAR:
      move = POLAR;
      EyeDist = 4.2; // der Schnittabstand
      //EyeAz   = 0;
      EyeEl   = 0;
      AzSpin  = 1.0;
      ElSpin  = init_el_spin;
      break;
    case POLAR:
      EyeDist = init_dist;
      EyeAz   = init_polar_az;
      EyeEl   = init_polar_el;
      AzSpin  = init_az_spin;
      ElSpin  = init_el_spin;
      break;
    case AGV_STOP:
      MoveMode = POLAR;
      AzSpin  = 0;
      ElSpin  = 0;
      move = POLAR;
      break;
    }
  MoveMode = move;
  MoveOn(1);
  DBG_PROG_ENDE
}

/***************************************************************/
/*******************    MOUSE HANDLING   ***********************/
/***************************************************************/

void
Agviewer::agvHandleButton(int button, int event, int x, int y)
{
  DBG_PROG_START
  DBG_PROG_V( button <<" "<< event);

 if (event == FL_PUSH && downb == -1)
 {
    lastx = downx = x;
    lasty = downy = y;
    downb = button;    

    if (button & FL_BUTTON1)
    {
        if (MoveMode == FLYING)
          EyeEl = -EyeEl;
        lastEl = downEl = EyeEl;
        lastAz = downAz = EyeAz;
        AzSpin = ElSpin = dAz = dEl = 0;
        AdjustingAzEl = 1;
        MoveOn(0); //ICC stop
        if (MoveMode == FLYING) {
          icc_examin_ns::status_info(_("left-/middle-/right mouse button -> rotate/cut/menu"));
        }
        duenn = false;
        MoveMode = POLAR;
    } else
      if(button & FL_BUTTON2)
    {
        downDist = EyeDist;
        downEx = Ex;
        downEy = Ey;
        downEz = Ez;
        downEyeMove = EyeMove;
        EyeMove = 0;
        if (MoveMode == FLYING)
          icc_examin_ns::status_info(_("Pause"));
    }

  } else if (event == FL_RELEASE && /*button ==*/ downb) {
    DBG_PROG

    if (downb & FL_BUTTON1)
      {
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
        if (MoveMode == FLYING) {
          icc_examin_ns::status_info(_("left mouse button -> go back"));
          duenn = false;
        }
      } else
      if(downb & FL_BUTTON2)
      {
        EyeMove = downEyeMove;
        if (MoveMode == FLYING) {
          icc_examin_ns::status_info(_("left mouse button -> go back"));
          duenn = true;
        }
      } else
        WARN_S( _("nicht erkennbare Maustaste: ") << button )

    downb = -1;

  } else
    WARN_S( _("keine Anweisung erkennbar") )

  DBG_PROG_ENDE
}

 /*
  * change EyeEl and EyeAz and position when mouse is moved w/ button down
  */
void
Agviewer::agvHandleMotion(int x, int y)
{ DBG_PROG_START
  int deltax = x - downx, deltay = y - downy;

  if (downb & FL_BUTTON1)
  {
      DBG_PROG_S( "FL_BUTTON1" )
      EyeEl  = downEl + el_sens * deltay;
      ConstrainEl();
      EyeAz  = downAz + az_sens * deltax;
      dAz    = prev_daz*dAz + cur_daz*(lastAz - EyeAz);
      dEl    = prev_del*dEl + cur_del*(lastEl - EyeEl);
      lastAz = EyeAz;
      lastEl = EyeEl;
  } else
  if (downb & FL_BUTTON2)
  {
      DBG_PROG_S( "FL_BUTTON2" )
      EyeDist = downDist + dist_sens*deltay;
      Ex = downEx - e_sens*deltay*sin(TORAD(EyeAz))*cos(TORAD(EyeEl));
      Ey = downEy - e_sens*deltay*sin(TORAD(EyeEl));
      Ez = downEz + e_sens*deltay*cos(TORAD(EyeAz))*cos(TORAD(EyeEl));
  }
  parent->redraw();
  DBG_PROG_ENDE
}

/***************************************************************/
/********************* KEYBOARD HANDLING ***********************/
/***************************************************************/

  /*
   * set EyeMove (current speed) for FLYING mode
   */
void
Agviewer::SetMove(float newmove)
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
Agviewer::agvHandleKeys_(unsigned char key, int&, int&)
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
Agviewer::normalize(GLfloat v[3])
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
Agviewer::ncrossprod(float v1[3], float v2[3], float cp[3])
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
Agviewer::agvMakeAxesList(int displaylistnum)
{ DBG_PROG_START
  DBG_PROG_V( displaylistnum )
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


