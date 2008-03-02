/*
 * ICC Examin ist eine ICC Profil Betrachter
 * 
 * Copyright (C) 2004-2007  Kai-Uwe Behrmann 
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

/** @brief a class with thread observer capabilities
 * 
 */

 //! @date      11. 09. 2006


#ifndef ICC_THREAD_DATEN_H
#define ICC_THREAD_DATEN_H

#include "icc_utils.h"

/** @brief * the class with locking
 *
 *  allowes thread save daten handling
 */

namespace icc_examin_ns {

class ThreadDaten
{
    bool frei_;               //!<@brief is not used from further process
    int  zahl_;               //!<@brief count of waiters
    Fl_Thread pth;
protected:
    ThreadDaten() { frei_ = true; zahl_ = 0; pth = 0; report_owner = 0; }
    ~ThreadDaten() {;}
public:
    bool frei();              //!<@brief is not locked
    void frei(int freigeben); //!<@brief lock with wait/unlock
    bool report_owner; 
};

}

#endif //ICC_THREAD_DATEN_H
