/*
 * fracviewer.h [from agviewer.h  (version 1.0)]
 *
 * AGV: a glut viewer. Routines for viewing a 3d scene w/ glut
 *
 * The two view movement modes are POLAR and FLYING.  Both move the eye, NOT
 * THE OBJECT.  You can never be upside down or twisted (roll) in either mode.
 *
 * A nice addition would be an examiner type trackball mode where you are
 * moving the object and so could see it from any angle.  Also less restricted
 * flying and polar modes (fly upside down, do rolls, etc.).
 *
 * Controls for Polar are just left and middle buttons -- for flying it's
 * those plus 0-9 number keys and +/- for speed adjustment.
 *
 * See agv_example.c and agviewer.c for more info.  Probably want to make
 * a copy of these and then edit for each program.  This isn't meant to be
 * a library, just something to graft onto your own programs.
 *
 * I welcome any feedback or improved versions.
 *
 * Philip Winston - 4/11/95
 * pwinston@hmc.edu
 * http://www.cs.hmc.edu/people/pwinston
 */

/*
 * move (mostly) all into one class - Kai-Uwe Behrmann 2004 - 2005
 *
 */


#ifndef AGVIEWER_H
#define AGVIEWER_H

#include "icc_utils.h" // ICC Examin
#if APPLE
#  include <OpenGL/glu.h>
#else
#  include <GL/glu.h> // added for FLTK
#endif
#include "icc_modell_beobachter.h"

class GL_Ansicht;

class Agviewer : public icc_examin_ns::Modell
{
  public:
  GL_Ansicht* parent;

  Agviewer (GL_Ansicht* p)  {
    parent = p;
/***************************************************************/
/************************** SETTINGS ***************************/
/***************************************************************/

   /* Initial polar movement settings */
          init_polar_az = 90.0;
          init_polar_el = 90.0;
          init_dist     = init_dist_a = init_dist_b = 4.74;
          init_az_spin  = 0.0;
          init_el_spin  = 0.0;


  /* Start in this mode */
          MoveMode = POLAR;  /* FLYING or POLAR mode? */

  /* Initial flying movement settings */
          init_move     = 0.01;
          minmove       = 0.001;
          Ex = 0.0;             /* flying parameters */
          Ey = -2.0;
          Ez = -2.0;
          EyeMove = init_move;

          EyeDist = init_dist;      /* polar params */
          AzSpin  = init_az_spin;
          ElSpin  = init_el_spin;

          EyeAz = init_polar_az;    /* used by both */
          EyeEl = init_polar_el;

  /* Multiply EyeMove by (1+-movefraction) when +/- hit in FLYING mode */
          movefraction = 0.25;

  /* What to multiply number of pixels mouse moved by to get rotation amount */
          el_sens   = 0.5;
          az_sens   = 0.5;

  /* What to multiply number of pixels mouse moved by for movement amounts */
          dist_sens = 0.01;
          e_sens    = 0.01;

  /* Minimum spin to allow in polar (lower forced to zero) */
          min_azspin = 0.1;
          min_elspin = 0.1;

  /* Factors used in computing dAz and dEl (which determine AzSpin, ElSpin) */
          slow_daz = 0.90;
          slow_del = 0.90;
          prev_daz = 0.80;
          prev_del = 0.80;
          cur_daz  = 0.20;
          cur_del  = 0.20;

          downb = -1;
          AdjustingAzEl = 0;
          }
  
  ~Agviewer () {; }

 /*
  * Call agvInit() with glut's current window set to the window in 
  * which you want to run the viewer. Right after creating it is fine.  It
  * will remember that window for possible later use (see below) and
  * registers mouse, motion, and keyboard handlers for that window (see below).
  *
  * allowidle is 1 or 0 depnding on whether you will let AGV install
  * and uninstall an idle function.  0 means you will not let it (because
  * you will be having your own idle function). In this case it is your
  * responsibility to put a statement like:
  *
  *     if (agvMoving)
  *       agvMove();
  *
  * at the end of your idle function, to let AGV update the viewpoint if it
  * is moving. 
  *
  * If allowidle is 1 it means AGV will install its own idle which
  * will update the viewpoint as needed and send glutPostRedisplay() to the
  * window which was current when agvInit() was called.
  *
  * agvSetIdleAllow changes this value so you can let AGV install its idle
  * when your idle isn't installed. 
  *
  */
  void agvInit(int allowidle);
  void agvSetAllowIdle(int allowidle);

 /*
  * Set which movement mode you are in.
  */
  typedef enum {
  FLYING = 100,
  POLAR ,
  ICCPOLAR ,
  ICCFLY_L,
  ICCFLY_a,
  ICCFLY_b,
  AGV_STOP
  } MovementType;

  void agvSwitchMoveMode(int move);


  // thin slice visualisation
 bool duenn;

 /*
  * agvViewTransform basically does the appropriate gluLookAt() for the 
  * current position.  So call it in your display on the projection matrix
  */
  void agvViewTransform(void);

 /*
  * agvMoving will be set by AGV according to whether it needs you to call
  * agvMove() at the end of your idle function.  You only need these if 
  * you aren't allowing AGV to do its own idle.
  * (Don't change the value of agvMoving)
  */
  int agvMoving;    /* Currently moving?  */
  void agvMove_(void);
  //static void agvMove_statisch(void* agv);

 /*
  * These are the routines AGV registers to deal with mouse and keyboard input.
  * Keyboard input only matters in flying mode, and then only to set speed.
  * Mouse input only uses left two buttons in both modes.
  * These are all registered with agvInit(), but you could register
  * something else which called these, or reregister these as needed 
  */
  void agvHandleButton(int button, int event, int x, int y);
  void agvHandleMotion(int x, int y);
  void agvHandleKeys_(unsigned char key, int &x, int &y);

 /*
  * Just an extra routine which makes an x-y-z axes (about 10x10x10)
  * which is nice for aligning things and debugging.  Pass it an available
  * displaylist number.
  */
  void agvMakeAxesList(int displaylist);



  private:

  /* map 0-9 to an EyeMove value when number key is hit in FLYING mode */
#define SPEEDFUNCTION(x) ((x)*(x)*0.001)  

  double movefraction,

         el_sens,
         az_sens,

         dist_sens,
         e_sens,

         min_azspin,
         min_elspin,

         slow_daz,
         slow_del,
         prev_daz,
         prev_del,
         cur_daz,
         cur_del;

/***************************************************************/
/************************** GLOBALS ****************************/
/***************************************************************/

  int     MoveMode;  /* FLYING or POLAR mode? */

  GLfloat Ex,           /* flying parameters */
          Ey,
          Ez,
          EyeMove,     

          EyeDist,      /* polar params */
          AzSpin,
          ElSpin,

          EyeAz,    /* used by both */
          EyeEl;

  int downx, downy,   /* for tracking mouse position */
      lastx, lasty,
      downb;     /* and button status */
						
  GLfloat downDist, downEl, downAz, /* for saving state of things */
          downEx, downEy, downEz,   /* when button is pressed */
          downEyeMove;                

  GLfloat dAz, dEl, lastAz, lastEl;  /* to calculate spinning w/ polar motion */
  int     AdjustingAzEl;

   /* Initial polar movement settings */
  double init_polar_az,
         init_polar_el,
         init_dist,
         init_dist_a,
         init_dist_b,
         init_az_spin,
         init_el_spin,

         init_move,
         minmove;
public:
  float eyeX () const { return Ex; } // ICC Examin
  float eyeY () const { return Ey; } // ICC Examin
  float eyeZ () const { return Ez; } // ICC Examin
  float eyeDist () const { return EyeDist; } // ICC Examin
  void  eyeDist(double d) { EyeDist = d; }
  float eyeAzimuth () const { return EyeAz; } // ICC Examin
  float eyeElevation () const { return EyeEl; } // ICC Examin

  void distA(double a) { init_dist_a = a; }
  void distB(double b) { init_dist_b = b; }
  double distA()       { return init_dist_a; }
  double distB()       { return init_dist_b; }
  double dist()        { return init_dist; }

  int RedisplayWindow;                      // glutWindow
  public: 
  int redisplayWindow() { return RedisplayWindow; }
  void setIdle(bool set);
  private:
   /* If AllowIdle is 1 it means AGV will install its own idle which
    * will update the viewpoint as needed and send glutPostRedisplay() to the
    * window RedisplayWindow which was set in agvInit().  AllowIdle of 0
    * means AGV won't install an idle funciton, and something like
    * "if (agvMoving) agvMove()" should exist at the end of the running
    * idle function.
    */

# define TORAD(x) ((M_PI/180.0)*(x))
# define TODEG(x) ((180.0/M_PI)*(x))

/***************************************************************/
/************************ PROTOTYPES ***************************/
/***************************************************************/

  /*
   * these are functions meant for internal use only
   * the other prototypes are in agviewer.h
   */

  void PolarLookFrom(GLfloat dist, GLfloat elevation, GLfloat azimuth);
  void FlyLookFrom(GLfloat x, GLfloat y, GLfloat z,
                        GLfloat az, GLfloat el);
  int  ConstrainEl(void);
  void MoveOn(int v);
  void SetMove(float newmove);
  void normalize(GLfloat v[3]);
  void ncrossprod(float v1[3], float v2[3], float cp[3]);

};

#include <vector>

extern std::vector<Agviewer> agviewers;

namespace agv {

}

#endif // AGVIEWER_H

