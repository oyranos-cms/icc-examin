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

class ICCkette;
extern ICCkette profile;

class ICCkette
{
  public:
                 ICCkette  () {_aktuelles_profil = -1; }
                 ~ICCkette () {; }
    void         clear()      {_profile.resize(0); _profilnamen.resize(0); }
  private:
    int                      _aktuelles_profil;
    // Liste der geladenen Profile
    std::vector<ICCprofile>  _profile;
    std::vector<std::string> _profilnamen;
    std::vector<int>         _aktiv;
  public:
    bool         oeffnen   (std::vector<std::string> dateinamen);
    //void         oeffnen   ();	// interaktiv
    bool         oeffnen   (std::string dateiname, int pos);
    void         aktiv     (int pos);
    void         passiv    (int pos);
    std::vector<int> aktiv () { return _aktiv; }
    void         aktuell   (int pos) {
                                if(pos < (int)_profile.size())
                                  _aktuelles_profil = (pos > -1) ? pos : -1; }
    int          aktuell   () { return _aktuelles_profil; }
    ICCprofile*  profil    () { return &(_profile[_aktuelles_profil]); }
    std::string  name      () {
               if(_profilnamen.size()) return _profilnamen[_aktuelles_profil];
               else return ""; }

  public:
/*    operator ICCprofile ()  {
               if(_profile.size()) return _profile[_aktuelles_profil];
               else return ICCprofile(); }
    operator std::string ()  {
               if(_profilnamen.size()) return _profilnamen[_aktuelles_profil];
               else return ""; }*/
    operator std::vector<std::string> ()  {
               return _profilnamen; }
    operator int ()  {
               return _aktuelles_profil; }

    int          size      () {return _profile.size(); }
};


#endif //ICC_KETTE_H

