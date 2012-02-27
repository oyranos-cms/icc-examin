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
 * I/O of the central class
 * 
 */

// Date:      Mai 2004

#ifndef ICC_EXAMIN_IO_H
#define ICC_EXAMIN_IO_H

#include "icc_utils.h"
#include "icc_gl.h"
#include "icc_kette.h"
#include "icc_oyranos.h"
#include "icc_model_observer.h"
#include "icc_thread_daten.h"

#include <string>
#include <set>

class  ICCfltkBetrachter;
class  ICCwaehler;
class  ICCexamin;
extern ICCexamin *icc_examin;


/** @brief file I/O of the central program instance
 *
 *  This class manages the coordination with file events
 */
class ICCexaminIO /*: public icc_examin_ns::Beobachter,
                    public icc_examin_ns::ThreadDaten*/
{
  friend   class ICCexamin;
  public:
                 ICCexaminIO ();
                 ~ICCexaminIO () {; }

  private:
    bool         lade_;
    bool         neu_laden_;
    std::set<int> erneuern_;
  public:
    int          erneuern();
    void         erneuern(int pos);
  private:
    ICClist<Speicher> speicher_vect_;
    static
#if USE_THREADS
    void*
#else
    void
#endif
                 oeffnenStatisch_ ( void* ICCexamina );
    void         oeffnenThread_ ();            //!< only one time pro ICCexamin
    void         oeffnenThread_ (int erneuern__); 
    int          intent_alt_;   //!< old rendering intent / de: Uebertragungsart
    void         farbraum_ (int info);
  public:
    void         oeffnen ();                   //!< interactive
    void         oeffnen (ICClist<std::string> dateinamen);
    bool         lade () {
                           return lade_;
                         };
    void         lade (ICClist<Speicher> & neu);

  public:
    bool         berichtSpeichern (void);      //!< GCATS report -> html file

    bool         gamutSpeichern (icc_examin_ns::IccGamutFormat format); //!< gamut
};

#endif //ICC_EXAMIN_IO_H

