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
 */

/*  a class with thread observer capabilities
 * 
 */

 // date      11. 09. 2006


#include "icc_thread_daten.h"
#include "threads.h"

void
icc_examin_ns::ThreadDaten::frei(int freigeben)
{ DBG_PROG_START

  if(freigeben)
  {
    frei_ = true;
    --zahl_;
    DBG_THREAD_S( "unlock " << zahl_ )

  } else {

    Fl_Thread pth_alt = pth;
    pth = pthread_self();

    int x = 0;

    while(!frei_)
    {
      if(pth == pth_alt)
      {
        WARN_S( (intptr_t)pth << " request from same thread" )
        break;
      }
      icc_examin_ns::sleep(0.01);
      if(++x > 200)
      {
        WARN_S("More than 2 seconds frozen.")
        x = 0;
      }
    }

    frei_ = false;
    ++zahl_;
    DBG_THREAD_S( "lock   " << zahl_ )
  }

  DBG_PROG_ENDE
}

