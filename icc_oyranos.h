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
 * Der CMS Sortierer
 * 
 */

// Date:      25. 11. 2004


#ifndef ICC_OYRANOS_H
#define ICC_OYRANOS_H
#define _(text) text

#include <string>
#include <vector>

class Speicher
{
    char* zeiger_;
    size_t groesse_;

  public:
    Speicher       () { zeiger_ = 0; groesse_ = 0; }
    ~Speicher      () { if( zeiger_ ) free (zeiger_); }
    Speicher       (const Speicher& s) { zeiger_ = 0; groesse_ = 0; }
    Speicher& operator = (const Speicher& s) { zeiger_ = 0; groesse_ = 0;
                        return *this; }

    void     lade  (char* zeiger, int groesse)
                      { zeiger_ = zeiger;
                        groesse_ = groesse; }
    Speicher       (char* zeiger, int groesse) { lade (zeiger, groesse); }

    size_t size          () { return groesse_; }
    std::string name;
    operator char*       () { return zeiger_; }
    operator std::string () { return name; }
    operator size_t      () { return groesse_; }
};

class Oyranos
{
  public:
    Oyranos() ;
    ~Oyranos();
    void    init() {; }

    std::string lab  ()                { lab_test_(); return lab_.name; }
    char*       lab  (size_t &g)       { lab_test_(); g = lab_.size();
                                         return lab_; }
    std::string rgb  ()                { rgb_test_(); return rgb_.name; }
    char*       rgb  (size_t &g)       { rgb_test_(); g = rgb_.size();
                                         return rgb_; }
    std::string cmyk ()                { cmyk_test_(); return cmyk_.name; }
    char*       cmyk (size_t &g)       { cmyk_test_(); g = cmyk_.size();
                                         return cmyk_; }
  private:
    void lab_test_();
    void rgb_test_();
    void cmyk_test_();
    Speicher lab_;
    Speicher rgb_;
    Speicher cmyk_;
};

extern Oyranos oyranos;

void	oyranos_pfade_einlesen();
void	oyranos_pfade_auffrischen();
void	oyranos_pfade_loeschen();
void	oyranos_pfad_dazu();



#endif //ICC_OYRANOS_H
