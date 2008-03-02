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
 * -----------------------------------------------------------------------------
 *
 * Werkzeuge.
 * 
 */

// Date:      August 2004


#include "icc_utils.h"
#include "icc_helfer.h"
#include <fstream>


int level_PROG_[DBG_MAX_THREADS] = {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1};
int icc_debug = 1;
std::ostringstream debug_s_;
#ifdef HAVE_PTHREAD_H
pthread_mutex_t debug_s_mutex_        = PTHREAD_MUTEX_INITIALIZER;
Fl_Thread       debug_s_mutex_thread_ = THREAD_HAUPT;
int             debug_s_mutex_threads_ = 0;
#endif

//#define WRITE_DBG

void
dbgWriteF (/*std::ostringstream & ss*/)
{
#ifdef WRITE_DBG
    std::string dateiname = "/tmp/icc_examin_dbg_";
    dateiname += getenv("USER");
    dateiname += ".txt";
    std::ofstream f ( dateiname.c_str(),
         std::ios::out | std::ios::app | std::ios::binary | std::ios::ate );

    DBG_MEM_V( dateiname )
    if (dateiname == "")
    {
      DBG_PROG_ENDE
      throw ausn_file_io (_("no filename given"));
    }
    DBG_MEM
    if (!f) {
      DBG_PROG_ENDE
      throw ausn_file_io (dateiname.c_str());
      dateiname = "";
    }

    size_t size = (unsigned int)f.tellp();
    DBG_MEM_V ( size << "|" << f.tellp() )
    //f.seekp(size);
    const char* data = ss.str().c_str();
    size = ss.str().size();
    if(size) {
      f.write (data, size);
      DBG_MEM_V ( size << "|" << f.tellp() <<" "<< (int*)data <<" "<< strlen(data) )
      f.close();
    } else {
      data = 0;
      WARN_S( _("Dateigroesse 0 fuer ") << dateiname )
    }
#else
  cout << debug_s_/*ss*/.str();
#endif
}


// Threads identifizieren
Fl_Thread icc_thread_liste[12] = {0,0,0,0,0,0,0,0,0,0,0,0};


/**
 * Steuerschnipsel
 * Beschreibung entnommen aus SuSE's /etc/rc.status
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

Fl_Thread
wandelThreadId(Fl_Thread id)
{
  Fl_Thread pos = id;
  {
    if      (icc_thread_liste[THREAD_HAUPT] == id)
      pos = THREAD_HAUPT;
    else if (icc_thread_liste[THREAD_GL1] == id)
      pos = THREAD_GL1;
    else if (icc_thread_liste[THREAD_GL2] == id)
      pos = THREAD_GL2;
    else if (icc_thread_liste[THREAD_LADEN] == id)
      pos = THREAD_LADEN;
    else if (icc_thread_liste[THREAD_WACHE] == id)
      pos = THREAD_WACHE;
    else
      pos = id;
  }
  return pos;
}

void//std::string
dbgThreadId(Fl_Thread id)
{
  //std::string s("??");
  Fl_Thread dbg_id = wandelThreadId ( id );
  //printf("%d\n", (int*)s.c_str());
  switch (dbg_id)
  {
    // in icc_thread_liste eingetragene Fl_Thread's lassen sich identifizieren
    case THREAD_HAUPT:
      dbgWrite( "\033[30m\033[1m[HAUPT]\033[m" ); break;
    case THREAD_GL1:
      dbgWrite( "\033[33m\033[1m[GL  1]\033[m" ); break;
    case THREAD_GL2:
      dbgWrite( "\033[35m\033[1m[GL  2]\033[m" ); break;
    case THREAD_LADEN:
      dbgWrite( "\033[32m\033[1m[LADEN]\033[m" ); break;
    case THREAD_WACHE:
      dbgWrite( "\033[34m\033[1m[WACHE]\033[m" ); break;
    default:
      dbgWrite( "\033[31m\033[1m[" ); dbgWrite(/*s +=*/ dbg_id ); dbgWrite( "]\033[m" ); break;
  }
  //dbgWrite( s;
  //return s;
}

const char*
threadGettext( const char* text)
{
  //printf("START %s:%d %s()\n", __FILE__,__LINE__,__func__);
  const char *translation = text;
# ifdef HAVE_PTHREAD_H
  static pthread_mutex_t translation_mutex_         = PTHREAD_MUTEX_INITIALIZER;
  static Fl_Thread       translation_mutex_thread_  = THREAD_HAUPT;
  static int             translation_mutex_threads_ = 0;
  // im selben Zweig gesperrten Rat ausschliesen
  if( translation_mutex_thread_ != wandelThreadId( pthread_self() ) ||
      translation_mutex_threads_ == 0 )
    // Warten bis der Rat von einem anderen Zweig freigegeben wird
    while (pthread_mutex_trylock( &translation_mutex_ )) {
      //printf("translation_mutex_ nicht verfügbar\n");
      icc_examin_ns::sleep(0.001);
    }
  translation_mutex_threads_++ ;
  if(translation_mutex_threads_ == 1)
     translation_mutex_thread_ = wandelThreadId( pthread_self() );
  //printf("translation_mutex_ ist lock\n");

  translation = gettext( text );

  --translation_mutex_threads_;
  if(!translation_mutex_threads_)
    pthread_mutex_unlock( &translation_mutex_ );
  //printf("translation_mutex_ ist unlock %d\n", translation_mutex_threads_);
# else
  translation = gettext( text );
# endif
  //printf("ENDE  %s:%d %s()\n", __FILE__,__LINE__,__func__);
  //DBG_PROG_ENDE
  return translation;
}

int
iccLevel_PROG(int plus_minus_null)
{
  int pth = wandelThreadId( pthread_self() );
  if(pth < DBG_MAX_THREADS) {
    level_PROG_ [pth] = level_PROG_[pth] + plus_minus_null;
    return level_PROG_ [pth];
  } else {
    return 0;
  }
}

