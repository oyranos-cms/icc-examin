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


Fl_Thread level_PROG_[DBG_MAX_THREADS] = {THREAD_ICCEXAMIN_MAX,THREAD_ICCEXAMIN_MAX,THREAD_ICCEXAMIN_MAX,THREAD_ICCEXAMIN_MAX,THREAD_ICCEXAMIN_MAX,THREAD_ICCEXAMIN_MAX,THREAD_ICCEXAMIN_MAX,THREAD_ICCEXAMIN_MAX,THREAD_ICCEXAMIN_MAX,THREAD_ICCEXAMIN_MAX,THREAD_ICCEXAMIN_MAX,THREAD_ICCEXAMIN_MAX};
int icc_debug = 1;
std::ostringstream debug_s_;
#ifdef HAVE_PTHREAD_H
pthread_mutex_t debug_s_mutex_        = PTHREAD_MUTEX_INITIALIZER;
Fl_Thread       debug_s_mutex_thread_ = (Fl_Thread)THREAD_HAUPT;
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


void
registerThreadId(Fl_Thread id, int pos)
{
  if(0 <= pos && pos < 12 )
    icc_thread_liste[pos] = id;
}

Fl_Thread
wandelThreadId(Fl_Thread id)
{
  Fl_Thread pos = id;
  {
    if      (icc_thread_liste[THREAD_HAUPT] == id)
      pos = (Fl_Thread)THREAD_HAUPT;
    else if (icc_thread_liste[THREAD_GL1] == id)
      pos = (Fl_Thread)THREAD_GL1;
    else if (icc_thread_liste[THREAD_GL2] == id)
      pos = (Fl_Thread)THREAD_GL2;
    else if (icc_thread_liste[THREAD_LADEN] == id)
      pos = (Fl_Thread)THREAD_LADEN;
    else if (icc_thread_liste[THREAD_WACHE] == id)
      pos = (Fl_Thread)THREAD_WACHE;
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
  switch ((int)dbg_id)
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

int
iccLevel_PROG(int plus_minus_null)
{
  Fl_Thread pth = wandelThreadId( pthread_self() );
  if(0 <= (intptr_t)pth && (intptr_t)pth < DBG_MAX_THREADS) {
    level_PROG_ [(intptr_t)pth] = level_PROG_[(intptr_t)pth] + plus_minus_null;
    if(level_PROG_ [(intptr_t)pth] < 0)
      return 0;
    else
      return level_PROG_ [(intptr_t)pth];
  } else {
    return 0;
  }
}

