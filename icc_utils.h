/*
 * ICC Examin ist eine ICC Profil Betrachter
 * 
 * Copyright (C) 2004-2008  Kai-Uwe Behrmann 
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
 * tools
 * 
 */

/* Date:      August 2004 */

#ifndef ICC_UTILS_H
#define ICC_UTILS_H

#ifndef ICC_EXTERN
#include "config.h"
#endif

#ifdef CWDEBUG
# include "sys.h"
# include "debug.h"
#endif

#ifdef __cplusplus
#include <cstdio>		/* printf() */
#ifdef HAVE_EXCEPTION
#  include <exception>		/* class expeption */
#endif

#include "threads.h"
#include <new>			/* bad_alloc() */
#if defined(WIN32)
# define iccThreadSelf  GetCurrentThreadId
# define iccThreadEqual(a,b) ((a) == (b))
# define iccThreadMutex_m    int
# define iccThreadMutexInit_m(m) 
# define icc_popen_m    _popen
# define icc_pclose_m   _pclose
  char * icc_strdup(const char*);
# define icc_strdup_m(text_) icc_strdup(text_)
#else
# define iccThreadSelf  pthread_self 
# define iccThreadEqual(a,b) pthread_equal((a),(b)) 
# define iccThreadMutex_m    pthread_mutex_t
# define iccThreadMutexInit_m(m,a) pthread_mutex_init(m,a)
# define iccThreadMutexLock_m(m) pthread_mutex_lock(m)
# define iccThreadMutexUnLock_m(m) pthread_mutex_unlock(m)
# define iccThreadMutexDestroy_m(m) pthread_mutex_destroy(m)
# define icc_popen_m    popen
# define icc_pclose_m   pclose
# define icc_strdup_m(text_) strdup(text_)
#endif 
#include <iostream>
#include <sstream>
#include <cmath>
# include "fl_i18n/fl_i18n.H"
#else
# define threadGettext(text) text
#endif /* __cplusplus */

#include <errno.h>
#ifndef EAGAIN
#define EAGAIN 11
#endif

#ifdef _
#undef _
#endif
#ifdef USE_GETTEXT
# include <libintl.h>
# define _(text) threadGettext(text)
#else
# define _(text) text
#endif


/* ByteOrder on Solaris */
#ifndef BIG_ENDIAN
# define BIG_ENDIAN 4321
#endif
#ifndef LITTLE_ENDIAN
# define LITTLE_ENDIAN 1234
#endif

#ifdef WIN32
/*#include <WinSock2.h>*/
#if BIGENDIAN 
# ifdef BYTE_ORDER
#   undef BYTE_ORDER
# endif
# define BYTE_ORDER BIG_ENDIAN
#endif
#if LITTLEENDIAN 
# ifdef BYTE_ORDER
#   undef BYTE_ORDER
# endif
# define BYTE_ORDER LITTLE_ENDIAN
#endif
#ifndef BYTE_ORDER
# define BYTE_ORDER LITTLE_ENDIAN
#endif
#endif

#ifdef _BIG_ENDIAN
# ifdef BYTE_ORDER
#   undef BYTE_ORDER
# endif
# define BYTE_ORDER BIG_ENDIAN
#endif
#ifdef _LITTLE_ENDIAN
# ifdef BYTE_ORDER
#   undef BYTE_ORDER
# endif
# define BYTE_ORDER LITTLE_ENDIAN
#endif


#ifdef __cplusplus
#define DBG_MAX_THREADS 12
enum { THREAD_HAUPT, THREAD_GL1, THREAD_GL2, THREAD_WACHE, THREAD_LADEN,
  THREAD_ICCEXAMIN_MAX };
void        dbgThreadId ();
std::string dbgThreadId(Fl_Thread thread);
int         wandelThreadId (Fl_Thread id);
Fl_Thread & getThreadId ( int pos );
void        registerThreadId ( Fl_Thread id, int pos );

/* status messages for error tracking */
void dbgWriteF (int code);
extern std::ostringstream debug_s_;

# ifdef HAVE_PTHREAD_H
extern pthread_mutex_t debug_s_mutex_;
extern Fl_Thread       debug_s_mutex_thread_;
extern int             debug_s_mutex_threads_;
# endif
void     dbgWriteLock                ( void );
void     dbgWriteUnLock              ( void );

#define ICC_MSG_ERROR 300
#define ICC_MSG_WARN  301
#define ICC_MSG_DBG   302

#define dbgWrite(ss) { \
  dbgWriteLock(); \
  debug_s_ << ss; \
  dbgWriteUnLock(); \
}
/* look in icc_utils.cpp for the WRITE_DBG definition */

#define cout std::cout
#define endl std::endl

/* extern Fl_Thread level_PROG_ [DBG_MAX_THREADS]; */
int    iccLevel_PROG(int plus_minus_null);
#define icc_level_PROG       iccLevel_PROG( 0)
#define icc_level_PROG_plus  iccLevel_PROG( 1)
#define icc_level_PROG_minus iccLevel_PROG(-1)
extern int icc_debug;

#include <time.h>
/*  icc_debug is controled be the environment variale ICCEXAMIN_DEBUG in main()
 *  levels:
 *   0: DBG              // use only intermediate and then remove, standard
 *   1: DBG_NUM          // tell about important states
 *   2: DBG_PROG         // tell about program state
 *   3: DBG_MEM          // observe memory
 *   4: DBG_THREAD; file E/A
 *   9: DBG_PROG_START & DBG_PROG_ENDE  only function tracing
 *
 *   [1,2,...,9]    this and all lower levels
 *   10+[1,2,...,9] only this level , z.B. ICCEXAMIN_DEBUG=13 selects only 
 *                                                memory informations
 *
 */


#define DBG_UHR_ (double)clock()/(double)CLOCKS_PER_SEC

#ifndef WIN32
#define DBG_T_     dbgWrite ( __FILE__<<":"<<__LINE__ <<" "<< __func__ << "() " ); dbgThreadId(); dbgWrite ( " "<< DBG_UHR_ <<" " );
#else
#define DBG_T_     dbgWrite ( __FILE__<<":"<<__LINE__ <<" " ); dbgThreadId(); dbgWrite ( " "<< DBG_UHR_ <<" " );
#endif
#define LEVEL      { for (int i = 0; i < icc_level_PROG; i++) dbgWrite (" "); }
#define DBG_(code) { LEVEL dbgWrite ("        "); DBG_T_ dbgWrite (endl); dbgWriteF(code); }
#define DBG_S_(txt,code){ LEVEL dbgWrite ("        "); DBG_T_ dbgWrite (txt << endl); dbgWriteF(code); }
#define DBG_V_(txt,code){ LEVEL dbgWrite ("        "); DBG_T_ dbgWrite (#txt << " " << txt << endl); dbgWriteF(code); }
#define DBG        DBG_
#define LEVEL_PLUS double m; \
    for (int i = 0; i < icc_level_PROG; i++) { \
      if( (int)(modf( i / 10.0 , &m) * 10.) < 5 ) { \
        dbgWrite ("+"); \
      } else { \
        dbgWrite (/*"\033[1m+\033[m"*/"-"); \
      } \
    }
#define DBG_START  {icc_level_PROG_plus; LEVEL_PLUS dbgWrite (" Start: "); DBG_T_ dbgWrite (endl); dbgWriteF(ICC_MSG_DBG); }
#define DBG_ENDE   { LEVEL_PLUS dbgWrite (" Ende:  "); DBG_T_ icc_level_PROG_minus; dbgWrite (endl); dbgWriteF(ICC_MSG_DBG); }
#define DBG_S(txt,code) DBG_S_(txt,code)
#define DBG_V(txt,code) DBG_V_(txt,code)

#ifdef DEBUG
#define DBG_BED(n) if (icc_debug >= n && icc_debug < 10 || icc_debug == 1##n)
#define DBG_BED2(n1,n2) if ((icc_debug >= n1 && icc_debug < 10) || icc_debug == 1##n1 || (icc_debug >= n2 && icc_debug < 10) || icc_debug == 1##n2 )
#define DBG_NUM        DBG_BED(1) DBG(ICC_MSG_DBG)
#define DBG_NUM_START  DBG_BED2(1,9) DBG_START
#define DBG_NUM_ENDE   DBG_BED2(1,9) DBG_ENDE
#define DBG_NUM_S(txt) DBG_BED(1) DBG_S(txt,ICC_MSG_DBG)
#define DBG_NUM_V(txt) DBG_BED(1) DBG_V(txt,ICC_MSG_DBG)
#else
#define DBG_NUM ;
#define DBG_NUM_START
#define DBG_NUM_ENDE
#define DBG_NUM_S(txt) ;
#define DBG_NUM_V(txt) ;
#endif
#ifdef DEBUG
#define DBG_PROG        DBG_BED(2) DBG(ICC_MSG_DBG)
#define DBG_PROG_START  DBG_BED2(2,9) DBG_START
#define DBG_PROG_ENDE   DBG_BED2(2,9) DBG_ENDE
#define DBG_PROG_S(txt) DBG_BED(2) DBG_S(txt,ICC_MSG_DBG)
#define DBG_PROG_V(txt) DBG_BED(2) DBG_V(txt,ICC_MSG_DBG)
#else
#define DBG_PROG ;
#define DBG_PROG_START ;
#define DBG_PROG_ENDE ;
#define DBG_PROG_S(txt) ;
#define DBG_PROG_V(txt) ;
#endif
#ifdef DEBUG
#define DBG_MEM        DBG_BED(3) DBG(ICC_MSG_DBG)
#define DBG_MEM_START  DBG_BED(3) DBG_START
#define DBG_MEM_ENDE   DBG_BED(3) DBG_ENDE
#define DBG_MEM_S(txt) DBG_BED(3) DBG_S(txt,ICC_MSG_DBG)
#define DBG_MEM_V(txt) DBG_BED(3) DBG_V(txt,ICC_MSG_DBG)
#else
#define DBG_MEM ;
#define DBG_MEM_START ;
#define DBG_MEM_ENDE ;
#define DBG_MEM_S(txt) ;
#define DBG_MEM_V(txt) ;
#endif
#ifdef DEBUG
#define DBG_THREAD        DBG_BED(4) DBG(ICC_MSG_DBG)
#define DBG_THREAD_START  DBG_BED2(4,8) DBG_START
#define DBG_THREAD_ENDE   DBG_BED2(4,8) DBG_ENDE
#define DBG_THREAD_S(txt) DBG_BED(4) DBG_S(txt,ICC_MSG_DBG)
#define DBG_THREAD_V(txt) DBG_BED(4) DBG_V(txt,ICC_MSG_DBG)
#else
#define DBG_THREAD ;
#define DBG_THREAD_START ;
#define DBG_THREAD_ENDE ;
#define DBG_THREAD_S(txt) ;
#define DBG_THREAD_V(txt) ;
#endif
#ifdef DEBUG
#define DBG_5        DBG_BED(5) DBG(ICC_MSG_DBG)
#define DBG_5_START  DBG_BED2(5,8) DBG_START
#define DBG_5_ENDE   DBG_BED2(5,8) DBG_ENDE
#define DBG_5_S(txt) DBG_BED(5) DBG_S(txt,ICC_MSG_DBG)
#define DBG_5_V(txt) DBG_BED(5) DBG_V(txt,ICC_MSG_DBG)
#else
#define DBG_5 ;
#define DBG_5_START ;
#define DBG_5_ENDE ;
#define DBG_5_S(txt) ;
#define DBG_5_V(txt) ;
#endif
#define WARN { dbgWrite (_("!!! Warning !!!")); DBG_(ICC_MSG_WARN) }
#define WARN_S(txt) { dbgWrite (_("!!! Warning !!!")); DBG_S_(txt,ICC_MSG_WARN) }
#define WARN_V(txt) { dbgWrite (_("!!! Warning !!!")); DBG_V_(txt,ICC_MSG_WARN) }

namespace icc_examin_ns {
  /* Zeit / Uhr */
  void sleep(double Sekunden); /* definiert in icc_helfer.cpp */
}


/* mathematische Helfer */

#ifndef MIN
#define MIN(a,b)    (((a) <= (b)) ? (a) : (b))
#endif
#define MAX(a,b)    (((a) > (b)) ? (a) : (b))
#define HYP(a,b)    sqrt( (a)*(a) + (b)*(b) )
#define HYP3(a,b,c) sqrt( (a)*(a) + (b)*(b) + (c)*(c) )
#define RUND(a)     ((a) + 0.5)

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif

/* ============================================================ */
/* Provisorische Ausnahme-Klasse; von std::exception abstammend: */
/* ============================================================ */
#ifdef HAVE_EXCEPTION
class Ausnahme : public std::exception {
public:
    virtual void report () const throw() {}  /* oder = 0; */
};


class ausn_file_io : public Ausnahme {
public:
    const char *fname;

    ausn_file_io (const char *fn) throw()	{ fname = fn; }
    virtual ~ausn_file_io () throw()		{ DBG_PROG_S ("Destruktor von " << __func__ << endl); }
    virtual const char* what() const throw()	{ return "Datei I/O"; }
    virtual void report () const throw() {
      DBG_PROG_S ("Ausnahme-Report:\n");
      DBG_PROG_S ("\tDatei \""<< fname <<"\" war nicht zu oeffnen\n");/*testweise */
    };
};
#endif

#else /* __cplusplus */
#define WARN { printf ("%s:%d %s\n", __FILE__,__LINE__,_("!!! Warning !!!")); }
#define WARN_S(txt) { printf ("%s:%d %s %s\n", __FILE__,__LINE__,_("!!! Warning !!!"), txt); }
#define WARN_V(txt) { printf ("%s:%d %s %s %s\n", __FILE__,__LINE__,_("!!! Warning !!!"), #txt, txt); }
#endif /* __cplusplus */

#endif /* ICC_UTILS_H */
