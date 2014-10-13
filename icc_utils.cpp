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
 * tools.
 * 
 */

// Date:      August 2004


#include "config.h"
#include "icc_utils.h"
#include "icc_helfer.h"
#include "icc_fenster.h"
#include "Flmm_Message.H"
#include <fstream>


int level_PROG_[DBG_MAX_THREADS];
int icc_debug = 1;
std::ostringstream debug_s_;
#ifdef HAVE_PTHREAD_H
pthread_mutex_t debug_s_mutex_        = PTHREAD_MUTEX_INITIALIZER;
Fl_Thread       debug_s_mutex_thread_
#if !defined(WIN32)
                                      = (Fl_Thread)THREAD_HAUPT
#endif
				      ;
int             debug_s_mutex_threads_ = 0;
#endif

void     dbgWriteLock                ( void )
{
#ifdef HAVE_PTHREAD_H
#if 1
//  if( !iccThreadEqual(debug_s_mutex_thread_, iccThreadSelf()) ||
//      debug_s_mutex_threads_ == 0 )
    while (pthread_mutex_trylock( &debug_s_mutex_ )) {
    /*printf("%s:%d %s() debug_s_mutex_ nicht verfuegbar\n",__FILE__,__LINE__,__func__);*/
      icc_examin_ns::sleep(.001);
    }
  debug_s_mutex_threads_++ ;
  if(debug_s_mutex_threads_ == 1)
    debug_s_mutex_thread_ = iccThreadSelf();
#else
  icc_examin_ns::lock(__FILE__,__LINE__);
#endif
#endif
}

void     dbgWriteUnLock              ( void )
{
#ifdef HAVE_PTHREAD_H
#if 1
  --debug_s_mutex_threads_;
  //if(!debug_s_mutex_threads_)
    pthread_mutex_unlock( &debug_s_mutex_ );
#else
  icc_examin_ns::unlock( icc_examin_ns::log_window?icc_examin_ns::log_window:0,__FILE__,__LINE__);
#endif
#endif
}

void dbgWriteS( std::string s )
{
  dbgWriteLock();
  debug_s_ << s;
  dbgWriteUnLock();
}

//#define WRITE_DBG
void
dbgWriteF (int code)
{
#ifdef WRITE_DBG
    std::string dateiname = "/tmp/icc_examin_dbg_";
    dateiname += getenv("USER");
    dateiname += ".txt";
    std::ofstream f ( dateiname.c_str(),
         std::ios::out | std::ios::app | std::ios::binary | std::ios::ate );


    size_t size = (unsigned int)f.tellp();
    //f.seekp(size);
    const char* data = debug_s_.str().c_str();
    size = debug_s_.str().size();
    if(size) {
      f.write (data, size);
      f.close();
    } else {
      data = 0;
    }
#endif
  icc_examin_ns::log(debug_s_.str().c_str(), code);
  debug_s_.str("");
}


// threads identify
Fl_Thread icc_thread_liste[12] = {0,0,0,0,0,0,0,0,0,0,0,0};


/**
 * console formating snippets
 * description taken over from SuSE's /etc/rc.status
 *
#    \033          ascii ESCape
#    \033[<NUM>G   move to column <NUM> (linux console, xterm, not vt100)
#    \033[<NUM>C   move <NUM> columns forward but only upto last column
#    \033[<NUM>D   move <NUM> columns backward but only upto first column
#    \033[<NUM>A   move <NUM> rows up
#    \033[<NUM>B   move <NUM> rows down
#    \033[1m       switch on bold
#    \033[31m      switch on red
#    \033[32m      switch on green
#    \033[33m      switch on yellow
#    \033[m        switch off color/bold
#    \017          exit alternate mode (xterm, vt100, linux console)
#    \033[10m      exit alternate mode (linux console)
#    \015          carriage return (without newline)
 *
 */


void
registerThreadId( Fl_Thread id, int pos )
{
  if(!(0 <= pos && pos < 12 ))
    WARN_S( "outside supported thread range: 0-11 | " << pos );

  icc_thread_liste[pos] = id;
}

Fl_Thread &
getThreadId( int pos )
{
  if(!(0 <= pos && pos < 12 ))
    WARN_S( "outside supported thread range: 0-11 | " << pos );

  return icc_thread_liste[pos];
}

int
wandelThreadId( Fl_Thread id )
{
  int pos = -1;
  {
    if      (iccThreadEqual( icc_thread_liste[THREAD_HAUPT], id ))
      pos = THREAD_HAUPT;
    else if (iccThreadEqual( icc_thread_liste[THREAD_GL1], id ))
      pos = THREAD_GL1;
    else if (iccThreadEqual( icc_thread_liste[THREAD_GL2], id ))
      pos = THREAD_GL2;
    else if (iccThreadEqual( icc_thread_liste[THREAD_LADEN], id ))
      pos = THREAD_LADEN;
    else if (iccThreadEqual( icc_thread_liste[THREAD_WACHE], id ))
      pos = THREAD_WACHE;
  }
  return pos;
}

std::string
dbgThreadId(Fl_Thread thread)
{
  std::string s("??");
  int dbg_id = wandelThreadId ( thread );
  char t[64] = {0};
  switch (dbg_id)
  {
    // in icc_thread_liste registred Fl_Thread's can be identified
#if defined(LINUX) && defined(never_nie)
    case THREAD_HAUPT:
      s = ( "\033[30m\033[1m[HAUPT]\033[m" ); break;
    case THREAD_GL1:
      s = ( "\033[33m\033[1m[GL  1]\033[m" ); break;
    case THREAD_GL2:
      s = ( "\033[35m\033[1m[GL  2]\033[m" ); break;
    case THREAD_LADEN:
      s = ( "\033[32m\033[1m[LADEN]\033[m" ); break;
    case THREAD_WACHE:
      s = ( "\033[34m\033[1m[WACHE]\033[m" ); break;
    default:
      sprintf(t,"%d",dbg_id);
      s = ( "\033[31m\033[1m[" ); s.append( t ); s.append( "]\033[m" ); break;
#else
    case THREAD_HAUPT:
      s = ( "[HAUPT]" ); break;
    case THREAD_GL1:
      s = ( "[GL  1]" ); break;
    case THREAD_GL2:
      s = ( "[GL  2]" ); break;
    case THREAD_LADEN:
      s = ( "[LADEN]" ); break;
    case THREAD_WACHE:
      s = ( "[WACHE]" ); break;
    default:
      sprintf(t,"%d",dbg_id);
      s = ( "[" ); s.append( t ); s.append( "]" ); break;
#endif
  }
  return s;
}

void//std::string
dbgThreadId()
{
  std::string s = dbgThreadId( iccThreadSelf() );
  dbgWrite( s );
}

int
iccLevel_PROG(int plus_minus_null)
{
  int pth = wandelThreadId( iccThreadSelf() );
  if(0 <= pth && pth < DBG_MAX_THREADS) {
    level_PROG_ [pth] = level_PROG_[pth] + plus_minus_null;
    if(level_PROG_ [pth] < 0)
      return 0;
    else
      return level_PROG_ [pth];
  } else {
    return 0;
  }
}



char*
icc_strdup (const char* t)
{
  size_t len = 0;
  char *temp = NULL;

  if(t)
     len = strlen(t);
  if(len)
  {
    temp = (char*) malloc(len+1);
    memcpy( temp, t, len );
    temp[len] = 0;
  }
  return temp;
}
