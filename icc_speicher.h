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
#include "icc_utils.h"

#define DBG_SPEICHER_START   if(icc_debug >= 3) DBG_PROG_START
#define DBG_SPEICHER_ENDE    if(icc_debug >= 3) DBG_PROG_ENDE

/*
 * Die Speicher Klasse ist ein Beobachter eines Speicherblocks mit Referenz
 *
 * ref_n_   : Referenz
 * zeiger_  : new [] Speicher
 * lade()   : löscht den alten Speicher und kopiert oder fordert neuen an
 * Forderung: zeiger_, groesse_, ref_n_ müssen von allen Kopien von Speicher aus
 *            sichtbar sein
 */

class Speicher
{
    // Zeigermachine
    char**      zeiger_;
    size_t*     groesse_;
    int*        ref_n_;
    std::string *name_;                   // z.B. Profilname
    time_t      *letze_aen_zeit_;         // letztes mal geändert
    time_t      *letze_ben_zeit_;         // letztes mal benutzt

    void        init  ()    {
                              DBG_MEM_START
                              ref_n_=           (int*)         new int[1];
                              zeiger_=          (char**)       new char*[1];
                              groesse_=         (size_t*)      new size_t[1];
                              name_ =           (std::string*) new std::string;
                              letze_aen_zeit_ = (time_t*)      new time_t[1];
                              letze_ben_zeit_ = (time_t*)      new time_t[1];
                              *ref_n_ = 1; *zeiger_ = 0; *groesse_ = 0;
                              *letze_aen_zeit_ = *letze_ben_zeit_ = 0;
                              DBG_MEM_ENDE
                            }
    Speicher&   copy  (const Speicher& s) {
                              DBG_MEM_START
                              ref_n_ = s.ref_n_;
                              zeiger_ = s.zeiger_;
                                DBG_MEM_V( (int*)*zeiger_ )
                              groesse_ = s.groesse_; DBG_MEM
                              name_ = s.name_; DBG_MEM
                              letze_aen_zeit_ = s.letze_aen_zeit_; DBG_MEM
                              letze_ben_zeit_ = s.letze_ben_zeit_; DBG_MEM
                                DBG_MEM_S( ref_n_ <<" = "<< *ref_n_ )
                              ++(*ref_n_); DBG_MEM
                                DBG_MEM_S( ref_n_ <<" = "<< *ref_n_ )
                              DBG_MEM_ENDE
                              return *this; }
    void        lade_ (char* zeiger, int groesse)
                            {
                              DBG_MEM_START
                              *zeiger_ = zeiger;
                                DBG_MEM_V( (int*)*zeiger_ )
                              *groesse_ = groesse;
                                DBG_MEM_S( ref_n_ <<" = "<< *ref_n_ )
                                DBG_MEM_V( letze_aen_zeit_ )
                                DBG_MEM_V( *letze_aen_zeit_ )
                              *letze_aen_zeit_ = time(0);
                              *letze_ben_zeit_ = time(0);
                              DBG_MEM_ENDE
                            }
    void        zeiger_clear_ () {
                              DBG_MEM_START
                              DBG_MEM_V( (int*)*zeiger_ )
                              if(*zeiger_) {
                                DBG_MEM_S("delete[]zeiger_ " << (int*)*zeiger_ )
                                delete [] *zeiger_;
                              }
                              *zeiger_ = 0;
                              *groesse_ = 0;
                              DBG_MEM_ENDE
                            }
  public:
                Speicher () {
                              DBG_MEM_START
                              init();
                              DBG_MEM_ENDE
                            }
               ~Speicher () {
                              DBG_MEM_START
                                DBG_MEM_S( ref_n_ <<" = "<< *ref_n_ )
                              --(*ref_n_);
                                DBG_MEM_S( ref_n_ <<" = "<< *ref_n_ )
                                DBG_MEM_V( (int*)zeiger_<<" "<<*groesse_ )
                                DBG_MEM_V( *name_ )
                              if(*ref_n_==0 && *zeiger_) {
                                DBG_MEM_S("delete[]zeiger_ " << (int*)zeiger_ )
                                delete [] *zeiger_;
                              }
                              if(*ref_n_==0) // üsch bün dor letzäää(h)
                              {              // h nach belieben   ---^
                                  DBG_MEM_S( "delete Referenz "<< *ref_n_ )
                                if(ref_n_!=0)         delete [] ref_n_;
                                if(zeiger_!=0)        delete [] zeiger_;
                                if(groesse_!=0)       delete [] groesse_;
                                if(name_!=0)          delete name_;
                                if(letze_aen_zeit_!=0)delete [] letze_aen_zeit_;
                                if(letze_ben_zeit_!=0)delete [] letze_ben_zeit_;
                              }
                              DBG_MEM_ENDE
                            }
                Speicher (const Speicher & s) {
                              DBG_MEM_START
                              copy(s);
                              DBG_MEM_ENDE
                            }

    void        lade     (char* zeiger, int groesse) {
                              DBG_MEM_START
                              lade_(zeiger,groesse);
                              DBG_MEM_ENDE
                            }
                Speicher (char* zeiger, int groesse) {
                              DBG_MEM_START
                              init();
                              lade (zeiger, groesse);
                              DBG_MEM_ENDE
                            }
                Speicher (const char* zeiger, int groesse) {
                              DBG_MEM_START
                              init();
                              char *z = (char*) new char [groesse];
                              memcpy(z, zeiger, groesse);
                                DBG_MEM_S( "neu: "<<(int*)z<<" "<<groesse )
                              lade (z, groesse);
                              DBG_MEM_ENDE
                            }

    std::string name     () const {
                              DBG_MEM_START
                                DBG_MEM_V( *name_ )
                                DBG_MEM_V( (int*)*zeiger_<<" "<<*groesse_ )
                              DBG_MEM_ENDE
                              return *name_;
                            }
    size_t      size     () const {
                              DBG_MEM_START
                              DBG_MEM_ENDE
                              return *groesse_;
                            }

    operator const char* () const {
                              DBG_MEM_START
                              *letze_ben_zeit_ = time(0);
                                DBG_MEM_V( *name_ )
                                DBG_MEM_V( (int*)*zeiger_<<" "<<*groesse_ )
                              DBG_MEM_ENDE
                              return *zeiger_;
                            }
    operator const size_t() const {
                              DBG_MEM_START
                              DBG_MEM_ENDE
                              return *groesse_;
                            }
    operator const size_t & () const {
                              DBG_MEM_START
                              DBG_MEM_ENDE
                              return *groesse_;
                            }
    Speicher&           operator = (const Speicher& s) {
                              DBG_MEM_START
                              DBG_MEM_ENDE
                              return copy(s);
                            }
    const char*         operator = (const char* s) const {
                              DBG_MEM_START
                              *name_ = s;
                                DBG_MEM_V( *name_ )
                                DBG_MEM_V( (int*)*zeiger_<<" "<<*groesse_ )
                              DBG_MEM_ENDE
                              return name_->c_str();
                            }
    const std::string & operator = (const std::string & s) const {
                              DBG_MEM_START
                              *name_ = s;
                                DBG_MEM_V( *name_ )
                                DBG_MEM_V( (int*)*zeiger_<<" "<<*groesse_ )
                              DBG_MEM_ENDE
                              return *name_;
                            }
};


#endif //ICC_SPEICHER_H
