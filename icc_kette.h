/*
 * ICC Examin ist eine ICC Profil Betrachter
 * 
 * Copyright (C) 2005  Kai-Uwe Behrmann 
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
 * Die Kette von Profilen Klasse
 * 
 */

// Date:      Januar 2005

#ifndef ICC_KETTE_H
#define ICC_KETTE_H

#include <string>
#include <vector>
#include "icc_profile.h"
#include "icc_utils.h"
#include "icc_modell_beobachter.h"
#include "icc_thread_daten.h"
#include "icc_speicher.h"

#define USE_THREADS 1

class ICCkette;
extern ICCkette profile;

class ICCkette : public icc_examin_ns::ThreadDaten,
                 public icc_examin_ns::Modell
{
  public:
                 ICCkette  ();
    void         init ();
                 ~ICCkette () {; }
    void         clear()      {profile_.clear(); profilnamen_.clear();
                               aktiv_.clear(); profil_mzeit_.clear(); }
  private:
    int                      aktuelles_profil_;
    /** Liste der geladenen Profile */
    std::vector<ICCprofile>  profile_;
    std::vector<std::string> profilnamen_;
    std::vector<int>         aktiv_;
    std::vector<double>      profil_mzeit_;

    /** Starte einen pthread Wächter und lasse Ihn alle unsere Beobachter
        informieren, welches Profile gerade geändert wurde.
     */
    static
#   if USE_THREADS
    void*
#   else
    void
#   endif
                 waechter (void*);
  public:
    bool         einfuegen (const Speicher & profil, int pos);

    void         setzAktiv (int pos) { DBG_PROG aktiv_[pos]=true;}// benachrichtigen(pos);}
    void         passiv    (int pos) { DBG_PROG aktiv_[pos]=false;}// benachrichtigen(pos);}
    std::vector<int> aktiv () { return aktiv_; }
    int          aktiv (int pos) { return aktiv_[pos]; }
    void         aktuell   (int pos) {
                                if(pos < (int)profile_.size())
                                  aktuelles_profil_ = (pos > -1) ? pos : -1; }
    int          aktuell   () { return aktuelles_profil_; }
    ICCprofile*  profil    () { if (profile_.size()) {
                                  return &(profile_[aktuelles_profil_]);
                                } else return 0; }
    std::string  name      (int pos) {
               if(profilnamen_.size() && pos < (int)profile_.size())
                     return profilnamen_[pos];
               else return ""; }
    std::string  name      () {
               if(profilnamen_.size()) return profilnamen_[aktuelles_profil_];
               else return ""; }
    ICCprofile*  operator [] (int n)
             { if (profile_.size() && n >= 0 && n < (int)profile_.size()) {
                 return &(profile_[n]);
               } else return 0;
             }

  public:
/*    operator ICCprofile ()  {
               if(profile_.size()) return profile_[aktuelles_profil_];
               else return ICCprofile(); }
    operator std::string ()  {
               if(profilnamen_.size()) return profilnamen_[aktuelles_profil_];
               else return ""; }*/
    operator std::vector<std::string> ()  {
               return profilnamen_; }
    operator int ()  {
               return aktuelles_profil_; }

    int          size      () {return profile_.size(); }
};


#endif //ICC_KETTE_H

