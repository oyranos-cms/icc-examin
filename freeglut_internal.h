/*
 * freeglut_internal.h
 *
 * The freeglut library private include file.
 *
 * Copyright (c) 1999-2000 Pawel W. Olszta. All Rights Reserved.
 * Written by Pawel W. Olszta, <olszta@sourceforge.net>
 * Creation date: Thu Dec 2 1999
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * PAWEL W. OLSZTA BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef  FREEGLUT_INTERNAL_H
#define  FREEGLUT_INTERNAL_H

#ifdef __cplusplus
extern "C" {
namespace icc_gl
{
#endif /* __cplusplus */


#if HAVE_CONFIG_H
#    include "config.h"
#endif

/* XXX Update these for each release! */
#define  VERSION_MAJOR 2
#define  VERSION_MINOR 2
#define  VERSION_PATCH 0

/* Freeglut is meant to be available under all Unix/X11 and Win32 platforms. */
#if defined(_WIN32_WCE)
#   define  TARGET_HOST_UNIX_X11    0
#   define  TARGET_HOST_WIN32       0
#   define  TARGET_HOST_WINCE       1
#elif defined(_MSC_VER) || defined(__CYGWIN__) || defined(__MINGW32__)
#   define  TARGET_HOST_UNIX_X11    0
#   define  TARGET_HOST_WIN32       1
#   define  TARGET_HOST_WINCE       0
#else
#   define  TARGET_HOST_UNIX_X11    1
#   define  TARGET_HOST_WIN32       0
#   define  TARGET_HOST_WINCE       0
#endif

#define  FREEGLUT_MAX_MENUS         3

/* Somehow all Win32 include headers depend on this one: */
#if TARGET_HOST_WIN32
#include <windows.h>
#include <windowsx.h>
#include <mmsystem.h>
#include <TCHAR.H>
#endif

#if defined(_MSC_VER)
#define strdup   _strdup
#endif

/* Those files should be available on every platform. */
#include <GL/gl.h>
#include <GL/glu.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#if HAVE_SYS_TYPES_H
#    include <sys/types.h>
#endif
#if HAVE_UNISTD_H
#    include <unistd.h>
#endif
#if TIME_WITH_SYS_TIME
#    include <sys/time.h>
#    include <time.h>
#else
#    if HAVE_SYS_TIME_H
#        include <sys/time.h>
#    else
#        include <time.h>
#    endif
#endif

/* The system-dependant include files should go here: */
#if TARGET_HOST_UNIX_X11
    #include <GL/glx.h>
    #include <X11/Xlib.h>
    #include <X11/Xatom.h>
    #include <X11/keysym.h>

    #ifdef HAVE_X11_EXTENSIONS_XF86VMODE_H
    #include <X11/extensions/xf86vmode.h>
    #endif
#endif

/* Microsoft VisualC++ 5.0's <math.h> does not define the PI */
#ifndef M_PI
#    define  M_PI  3.14159265358979323846
#endif

#ifndef TRUE
#    define  TRUE  1
#endif

#ifndef FALSE
#    define  FALSE  0
#endif

// ICC Examin
void glutSolidSphere(GLdouble radius, GLint slices, GLint stacks);
void glutSolidCone( GLdouble base, GLdouble height, GLint slices, GLint stacks );
void glutSolidCube( GLdouble dSize );
void glutStrokeCharacter( void* fontID, int character );
int  glutStrokeWidth( void* fontID, int character );
#   define  ICCGLUT_STROKE_ROMAN               ((void *)0x0000)
#   define  ICCGLUT_STROKE_MONO_ROMAN          ((void *)0x0001)


/* -- GLOBAL TYPE DEFINITIONS ---------------------------------------------- */

/* A list structure */
typedef struct tagSFG_List SFG_List;
struct tagSFG_List
{
    void *First;
    void *Last;
};

/* A list node structure */
typedef struct tagSFG_Node SFG_Node;
struct tagSFG_Node
{
    void *Next;
    void *Prev;
};

/* A helper structure holding two ints and a boolean */
typedef struct tagSFG_XYUse SFG_XYUse;
struct tagSFG_XYUse
{
    GLint           X, Y;               /* The two integers...               */
    GLboolean       Use;                /* ...and a single boolean.          */
};


/* -- PRIVATE FUNCTION DECLARATIONS ---------------------------------------- */

/*
 * A call to this function makes us sure that the Display and Structure
 * subsystems have been properly initialized and are ready to be used
 */
#define  FREEGLUT_EXIT_IF_NOT_INITIALISED( string )               \
  if ( ! fgState.Initialised )                                    \
  {                                                               \
    fgError ( " ERROR:  Function <%s> called"                     \
              " without first calling 'glutInit'.", (string) ) ;  \
  }

#define  FREEGLUT_INTERNAL_ERROR_EXIT_IF_NOT_INITIALISED( string )  \
  if ( ! fgState.Initialised )                                      \
  {                                                                 \
    fgError ( " ERROR:  Internal <%s> function called"              \
              " without first calling 'glutInit'.", (string) ) ;    \
  }

#define  FREEGLUT_INTERNAL_ERROR_EXIT( cond, string, function )  \
  if ( ! ( cond ) )                                              \
  {                                                              \
    fgError ( " ERROR:  Internal error <%s> in function %s",     \
              (string), (function) ) ;                           \
  }

/*
 * Following definitions are somewhat similiar to GLib's,
 * but do not generate any log messages:
 */
#define  freeglut_return_if_fail( expr ) \
    if( !(expr) )                        \
        return;
#define  freeglut_return_val_if_fail( expr, val ) \
    if( !(expr) )                                 \
        return val ;

/*
 * A call to those macros assures us that there is a current
 * window set, respectively:
 */
#define  FREEGLUT_EXIT_IF_NO_WINDOW( string )                   \
  if ( ! fgStructure.Window )                                   \
  {                                                             \
    fgError ( " ERROR:  Function <%s> called"                   \
              " with no current window defined.", (string) ) ;  \
  }

/*
 * The deinitialize function gets called on glutMainLoop() end. It should clean up
 * everything inside of the freeglut
 */
void fgDeinitialize( void );

/*
 * Those two functions are used to create/destroy the freeglut internal
 * structures. This actually happens when calling glutInit() and when
 * quitting the glutMainLoop() (which actually happens, when all windows
 * have been closed).
 */
void fgCreateStructure( void );
void fgDestroyStructure( void );


/* List functions */
void fgListInit(SFG_List *list);
void fgListAppend(SFG_List *list, SFG_Node *node);
void fgListRemove(SFG_List *list, SFG_Node *node);
int fgListLength(SFG_List *list);
void fgListInsert(SFG_List *list, SFG_Node *next, SFG_Node *node);

/*
 * This structure is used for the enumeration purposes.
 * You can easily extend its functionalities by declaring
 * a structure containing enumerator's contents and custom
 * data, then casting its pointer to (SFG_Enumerator *).
 */
typedef struct tagSFG_Enumerator SFG_Enumerator;
struct tagSFG_Enumerator
{
    GLboolean   found;                          /* Used to terminate search  */
    void*       data;                           /* Custom data pointer       */
};

/* The bitmap font structure */
typedef struct tagSFG_Font SFG_Font;
struct tagSFG_Font
{
    char*           Name;         /* The source font name             */
    int             Quantity;     /* Number of chars in font          */
    int             Height;       /* Height of the characters         */
    const GLubyte** Characters;   /* The characters mapping           */

    float           xorig, yorig; /* Relative origin of the character */
};

/* The stroke font structures */

typedef struct tagSFG_StrokeVertex SFG_StrokeVertex;
struct tagSFG_StrokeVertex
{
    GLfloat         X, Y;
};

typedef struct tagSFG_StrokeStrip SFG_StrokeStrip;
struct tagSFG_StrokeStrip
{
    int             Number;
    const SFG_StrokeVertex* Vertices;
};

typedef struct tagSFG_StrokeChar SFG_StrokeChar;
struct tagSFG_StrokeChar
{
    GLfloat         Right;
    int             Number;
    const SFG_StrokeStrip* Strips;
};

typedef struct tagSFG_StrokeFont SFG_StrokeFont;
struct tagSFG_StrokeFont
{
    char*           Name;                       /* The source font name      */
    int             Quantity;                   /* Number of chars in font   */
    GLfloat         Height;                     /* Height of the characters  */
    const SFG_StrokeChar** Characters;          /* The characters mapping    */
};



/* Error Message functions */
void fgError( const char *fmt, ... );
void fgWarning( const char *fmt, ... );

#ifdef __cplusplus
} // namespace icc_gl
} // extern "C"
#endif /* __cplusplus */


#endif /* FREEGLUT_INTERNAL_H */

/*** END OF FILE ***/
