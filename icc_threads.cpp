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
 */

/** @brief  thread locking for Oyranos
 * 
 */

 //! @date      2008/01/22

#define USE_THREADS 1
#include "icc_threads.h"
#include "icc_utils.h"
#include "threads.h"

oyranos::oyPointer  
icc_examin_ns::iccStruct_LockCreate ( oyranos::oyStruct_s * obj )
{
  DBG_PROG_START
  int error = !obj;
  oyranos::oyPointer thread_lock = 0;

  if(error)
  {
    DBG_THREAD_S( "no object provided" )
  }

  iccThreadMutex_m * m = (iccThreadMutex_m *) malloc(sizeof(iccThreadMutex_m));
  error = iccThreadMutexInit_m( m, 0 );
  if(error)
  {
    WARN_S( "Error: " << error )
  } else
    thread_lock = m;

  DBG_PROG_ENDE
  return thread_lock;
}

void       
icc_examin_ns::iccLockRelease ( oyranos::oyPointer         thread_lock,
                                       const char        * marker,
                                       int                 line )
{
  int error = !thread_lock;
  DBG_PROG_START
  DBG_THREAD_S( marker<<":"<<line )

  if(error)
  {
    if(marker)
      DBG_THREAD_S( "no lock provided at " << marker <<":"<< line )
    else
      DBG_THREAD_S( "no lock provided" )
  } else {
    iccThreadMutex_m * m = (iccThreadMutex_m *) thread_lock;
    //iccUnLock( m, marker, line );
    error = iccThreadMutexDestroy_m( m );

    if(error)
    {
      int r = error;
      const char * err_string = "";
      switch (r)
      {
        case EACCES:       err_string = "EACCES = "; break;
        case EAGAIN:       err_string = "EAGAIN = "; break;
        case EBUSY:        err_string = "EBUSY = "; break;
        case EINVAL:       err_string = "EINVAL = "; break;
        case ENOMEM:       err_string = "ENOMEM = "; break;
        case EIO:          err_string = "EIO = "; break;
        case ELOOP:        err_string = "ELOOP = "; break;
        case ENAMETOOLONG: err_string = "ENAMETOOLONG = "; break;
        case ENOENT:       err_string = "ENOENT = "; break;
        case ENOTDIR:      err_string = "ENOTDIR = "; break;
        case EOVERFLOW:    err_string = "EOVERFLOW = "; break;
        case EPERM:        err_string = "EPERM = "; break;
        default:           err_string = "Error = "; break;
      }
      if(marker)
        WARN_S( err_string << error << " at " << marker <<":"<< line )
      else
        WARN_S( err_string << error )
    }
  }


  DBG_PROG_ENDE
}
void       
icc_examin_ns::iccLock      ( oyranos::oyPointer           thread_lock,
                                       const char        * marker,
                                       int                 line )
{
  int error = !thread_lock;
  DBG_PROG_START
  DBG_THREAD_S( marker<<":"<<line )

  static const char * last_marker = marker;
  static int last_line = line;

  if(error)
  {
    if(marker)
      WARN_S( "no lock provided at " << marker  <<":"<< line <<"  last: "
                                     << last_marker <<":"<< last_line)
    else
      WARN_S( "no lock provided" << last_marker <<":"<< last_line )
  } else {
    iccThreadMutex_m * m = (iccThreadMutex_m *) thread_lock;
#if defined(DEBUG) && defined(__unix__)
    while (pthread_mutex_trylock( m ))
    {
      DBG_S("mutex not available at "<<marker <<":"<< line<<"  last: "
                                     << last_marker <<":"<< last_line);
      icc_examin_ns::sleep(1.0);
    }
#else
    error = iccThreadMutexLock_m( m );
#endif

    if(error)
    {
      if(marker)
        WARN_S( "Error: " << error << " at " << marker <<":"<< line )
      else
        WARN_S( "Error: " << error )
    }
  }

  DBG_PROG_ENDE
}
void       
icc_examin_ns::iccUnLock    ( oyranos::oyPointer           thread_lock,
                                       const char        * marker,
                                       int                 line )
{
  int error = !thread_lock;
  DBG_PROG_START
  DBG_THREAD_S( marker<<":"<<line )

  if(error)
  {
    if(marker)
      WARN_S( "no lock provided at " << marker <<":"<< line )
    else
      WARN_S( "no lock provided" )
  } else {
    iccThreadMutex_m * m = (iccThreadMutex_m *) thread_lock;
    error = iccThreadMutexUnLock_m( m );

    if(error)
    {
      if(marker)
        WARN_S( "Error: " << error << " at " << marker <<":"<< line )
      else
        WARN_S( "Error: " << error )
    }
  }

  DBG_PROG_ENDE
}


