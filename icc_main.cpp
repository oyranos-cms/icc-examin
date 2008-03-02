/*
 * ICC Examin ist eine ICC Profil Betrachter
 * 
 * Copyright (C) 2004-2006  Kai-Uwe Behrmann 
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
 * start function
 * 
 */

#include "icc_utils.h"
#include "config.h"
#include "icc_helfer.h"
#include "icc_examin.h"
#include "icc_kette.h"


void  MallocDebug_CheckFreeList ();

int
main (int argc, char** argv)
{
# ifdef CWDEBUG
  Debug(myproject::debug::init());		// This is a custom function defined
  						// in example-project/debug.cc.
# endif
# if HAVE_PTHREAD_H
  registerThreadId( pthread_self(), THREAD_HAUPT );
# ifdef CWDEBUG
  Debug(myproject::debug::init_thread());	// This is a custom function defined
  						// in example-project/debug.cc.
# endif
# endif



# if APPLE
  //MallocDebug_CheckFreeList();
# endif

  if(getenv("ICCEXAMIN_DEBUG") && atoi(getenv("ICCEXAMIN_DEBUG"))>0)
    icc_debug = atoi(getenv("ICCEXAMIN_DEBUG"));
  else
    icc_debug = 0;

  DBG_PROG_START

  setI18N( argv[0] );

  ICCexamin hauptprogramm;

  icc_examin = &hauptprogramm;

  hauptprogramm.start(argc, argv);

  DBG_PROG_ENDE
  return false;
}


