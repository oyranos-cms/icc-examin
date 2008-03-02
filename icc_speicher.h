/*
 * ICC Examin ist eine ICC Profil Betrachter
 * 
 * Copyright (C) 2004-2005  Kai-Uwe Behrmann 
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
 * Die Speicher Klasse
 * 
 */

// Date:      03. 01. 2005


#ifndef ICC_SPEICHER_H
#define ICC_SPEICHER_H
#define _(text) text

#include <string>



class Speicher
{
    char*       zeiger_;
    size_t      groesse_;
    int*        ref_n_;
    void        init  ()    { ref_n_=(int*)new int[1];
                              *ref_n_ = 0; zeiger_ = 0; groesse_ = 0; }
    Speicher&   copy  (const Speicher& s) { init();
                              zeiger_ = s.zeiger_;
                              groesse_ = s.groesse_;
                              ++*ref_n_;
                              return *this; }
    void        lade_ (char* zeiger, int groesse)
                            { zeiger_ = zeiger;
                              groesse_ = groesse;
                              letze_aen_zeit = time(0);
                              letze_ben_zeit = time(0); }
    void        clear ()    { if(*ref_n_) --(*ref_n_);
                              if(*ref_n_==0 && zeiger_) free(zeiger_); }
  public:
    Speicher             () { init(); }
    ~Speicher            () { clear(); if(*ref_n_==0) delete ref_n_; }
    Speicher             (const Speicher& s) { copy(s); }
    Speicher& operator = (const Speicher& s) { return copy(s); }

    void        lade     (char* zeiger, int groesse)
                            { clear(); lade_(zeiger,groesse); }
                Speicher (char* zeiger, int groesse)
                            { init(); lade (zeiger, groesse); }
                Speicher (const char* zeiger, int groesse)
                            { init();
                              char* zeiger_ = (char*) new char [groesse];
                              lade (zeiger_, groesse); }

    size_t      size     () { return groesse_; }
    std::string name;                    // z.B. Profilname
    time_t      letze_aen_zeit;          // letztes mal geändert
    time_t      letze_ben_zeit;          // letztes mal benutzt

    operator const char* () { letze_ben_zeit = time(0); return zeiger_; }
    operator std::string () { return name; }
    operator size_t      () { return groesse_; }
};


#endif //ICC_SPEICHER_H
