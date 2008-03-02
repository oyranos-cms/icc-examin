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

/** @brief eine Klasse mit thread Ueberwachungsmöglichkeit
 * 
 */

 //! @date      11. 09. 2006


#ifndef ICC_THREAD_DATEN_H
#define ICC_THREAD_DATEN_H

#include "icc_utils.h"

/** @brief * Die Klasse mit Anmeldung und Freigabe
 *
 *  erlaubt threadsicheren Datenzugang
 */

namespace icc_examin_ns {

class ThreadDaten
{
    bool frei_;               //!<@brief wird nicht von weiterem Prozess benutzt
    int  zahl_;               //!<@brief Anzahl der Wartenden
    Fl_Thread pth;
protected:
    ThreadDaten() { frei_ = true; zahl_ = 0; pth = 0; }
    ~ThreadDaten() {;}
public:
    bool frei()     { return frei_; }          //!<@brief ist nicht gesperrt
    void frei(int freigeben); //!@brief Sperren mit Warten/Freigeben
};

}

#endif //ICC_THREAD_DATEN_H
