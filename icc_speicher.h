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
 * the Speicher (memory) class
 * 
 */

// Date:      03. 01. 2005


#ifndef ICC_SPEICHER_H
#define ICC_SPEICHER_H

#include "icc_utils.h"
#include <string>
#include <time.h>

#define DBG_SPEICHER_START   if(icc_debug >= 3) DBG_PROG_START
#define DBG_SPEICHER_ENDE    if(icc_debug >= 3) DBG_PROG_ENDE

/*
 * The Speicher class is a referenced memory block
 *
 * Additionally the object should be observed to see changes and not use blindly.
 *
 * ref_n_   : reference
 * zeiger_  : new [] allocated memory
 * lade()   : erases the old block and copies or allocates new memory
 * Forderung: zeiger_, groesse_, ref_n_ must be visible from all copies of the
 *            class
 */

class Speicher
{
    // pointer machine
    char**      zeiger_;
    size_t*     groesse_;
    int*        ref_n_;
    int         id_;
    static int  globale_id_;
    std::string *name_;                   // z.B. profile name
    double      *letze_aen_zeit_;         // lest time changed
    double      *letze_ben_zeit_;         // lest time used

    void        init  ()    {
                              DBG_MEM_START
                              id_ = globale_id_;
                                DBG_PROG_V( id_ )
                              ++globale_id_;
                              ref_n_=           (int*)         new int[1];
                              zeiger_=          (char**)       new char*[1];
                              groesse_=         (size_t*)      new size_t[1];
                              name_ =           (std::string*) new std::string;
                              letze_aen_zeit_ = (double*)      new double[1];
                              letze_ben_zeit_ = (double*)      new double[1];
                              *ref_n_ = 1; *zeiger_ = 0; *groesse_ = 0;
                              *letze_aen_zeit_ = *letze_ben_zeit_ = 0;
                              DBG_MEM_ENDE
                            }
    Speicher&   copy  (const Speicher& s) {
                              DBG_MEM_START
                              id_ = s.id_;
                              ref_n_ = s.ref_n_;
                              zeiger_ = s.zeiger_;
                                DBG_MEM_V( (int*)*zeiger_ <<" "<< id_ )
                              groesse_ = s.groesse_; DBG_MEM
                              name_ = s.name_; DBG_MEM
                              letze_aen_zeit_ = s.letze_aen_zeit_; DBG_MEM
                              letze_ben_zeit_ = s.letze_ben_zeit_; DBG_MEM
                                DBG_MEM_S( ref_n_ <<" = "<< *ref_n_ )
                              ++(*ref_n_); DBG_MEM
                                DBG_MEM_S( ref_n_ <<" = "<< *ref_n_ )
                              DBG_MEM_ENDE
                              return *this; }
    void        lade_ (char* zeiger, size_t groesse)
                            {
                              zeiger_clear_();
                              DBG_MEM_START
                              *zeiger_ = zeiger;
                                DBG_MEM_V( (int*)*zeiger_ <<" "<< id_ )
                              *groesse_ = groesse;
                                DBG_MEM_S( ref_n_ <<" = "<< *ref_n_ )
                                DBG_MEM_V(  letze_aen_zeit_ <<" "<<
                                           *letze_aen_zeit_ )
                                DBG_MEM_V( *groesse_ )
                              *letze_aen_zeit_ = (double)time(0);
                              *letze_ben_zeit_ = (double)time(0);
                              DBG_MEM_ENDE
                            }
    void        zeiger_clear_ () {
                              DBG_MEM_START
                              DBG_MEM_V( (int*)*zeiger_ <<" "<< id_ )
                              if(*zeiger_) {
                                DBG_MEM_S("delete[]zeiger_ " << (int*)*zeiger_ <<" "<< id_ )
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
                              DBG_MEM_START DBG_MEM_S( "~Speicher ()" )
                                DBG_MEM_S( ref_n_ <<" = "<< *ref_n_ )
                              --(*ref_n_);
                                DBG_MEM_S( ref_n_ <<" = "<< *ref_n_ <<"|"<< id_ )
                                DBG_MEM_V( (int*)zeiger_<<" "<<*groesse_ <<" "<< id_ )
                                DBG_MEM_V( *name_ )
                              if(*ref_n_==0 && *zeiger_) {
                                DBG_MEM_S("delete[]zeiger_ " << (int*)zeiger_ <<" "<< id_ )
                                delete [] *zeiger_;
                              }
                              if(*ref_n_==0) // Uesch buen dor letzaeaeae(h)
                              {              // h nach belieben   ---^
                                  DBG_MEM_S( "delete Referenz "<< *ref_n_ <<"|"<< id_ )
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

    void        clear    () {
                              DBG_MEM_START
                              zeiger_clear_();
                              DBG_MEM_ENDE
                            }
    // convenience functions
    void        ladeNew     (char* zeiger, size_t & groesse) {
                              DBG_MEM_START
                              if(zeiger && groesse) {
                                // allocated with new : simply take over
                                lade_(zeiger,groesse);
                              }
                              DBG_MEM_ENDE
                            }
    void        ladeUndFreePtr(char** zeiger, size_t & groesse) {
                              DBG_MEM_START
                              if(*zeiger && groesse) {
                                // allocated with *alloc : copy first through
                                // "lade"
                                lade(*zeiger,groesse);
                                free(*zeiger);
                                *zeiger = 0;
                                groesse = 0;
                              }
                              DBG_MEM_ENDE
                            }
    // load with copy in a block allocated with new
    void        lade     (const char* zeiger, size_t groesse) {
                              DBG_MEM_START
                              char *block = (char*) new char [groesse+1];
                              memcpy(block, zeiger, groesse);
                                DBG_MEM_S( "neu: "<<(int*)block<<" "<<groesse <<" "<< id_ )
                              lade_(block,groesse);
                              DBG_MEM_ENDE
                            }
                Speicher (char* zeiger, size_t groesse) {
                              DBG_MEM_START
                              init();
                              lade (zeiger, groesse);
                              DBG_MEM_ENDE
                            }
                Speicher (const char* zeiger, size_t groesse) {
                              DBG_MEM_START
                              init();
                              lade (zeiger, groesse);
                              DBG_MEM_ENDE
                            }

    std::string name     () const {
                              DBG_MEM_START
                                DBG_MEM_V( *name_ )
                                DBG_MEM_V( (int*)*zeiger_<<" "<<*groesse_ <<" "<< id_ )
                              DBG_MEM_ENDE
                              return *name_;
                            }
    size_t      size     () const {
                              DBG_MEM_START
                              DBG_MEM_ENDE
                              return *groesse_;
                            }
    double      zeit     () const {
                              return *letze_aen_zeit_;
                            }
    void        zeit     (double neue_zeit) {
                              *letze_aen_zeit_ = neue_zeit;
                            }

    operator const char* () const {
                              DBG_MEM_START
                              *letze_ben_zeit_ = (double)time(0);
                                DBG_MEM_V( *name_ )
                                DBG_MEM_V( (int*)*zeiger_<<" "<<*groesse_ <<" "<< id_ )
                              DBG_MEM_ENDE
                              return *zeiger_;
                            }
    operator const size_t () const {
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
                                DBG_MEM_V( (int*)*zeiger_<<" "<<*groesse_ <<" "<< id_ )
                              DBG_MEM_ENDE
                              return name_->c_str();
                            }
    const std::string & operator = (const std::string & s) const {
                              DBG_MEM_START
                              *name_ = s.c_str();
                                DBG_MEM_V( *name_ )
                                DBG_MEM_V( (int*)*zeiger_<<" "<<*groesse_ <<" "<< id_ )
                              DBG_MEM_ENDE
                              return *name_;
                            }
};


#endif //ICC_SPEICHER_H
