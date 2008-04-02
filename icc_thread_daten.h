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

#define USE_THREADS 1

#include "icc_utils.h"
#include "icc_helfer.h"
#include "icc_list.h"
#include "threads.h"

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
    iccThreadMutex_m mutex_;
protected:
    void   init() { frei_ = true; zahl_ = 0; pth = 0; report_owner = 0;
                    iccThreadMutexInit_m( &mutex_, 0 ); }
    ThreadDaten() { init(); }
    ~ThreadDaten() {;}
    ThreadDaten (const ThreadDaten & s) {
                              copy(s); }
public:
    bool frei();              //!<@brief is not locked
    void frei(int freigeben); //!<@brief lock with wait/unlock
    bool report_owner;

    ThreadDaten&   copy  (const ThreadDaten& s) {
                              init();
                              return *this; }
    ThreadDaten& operator = (const ThreadDaten& d) {
                              return copy(d); }
};

/** @brief a thread save list
 *
 *  this safty makes the structure as well expensive
 */
template <typename T>
class ICCThreadList : public ICClist<T>,
                      public icc_examin_ns::ThreadDaten
{
public:
  /** @brief index access operator 
   *
   *  no check in this basic class
   */
  T &      operator [] (size_t i) {
    //if(i < n_)
      return ICClist<T>::operator[](i);
      //return *((ICClist<T>*)this)[i];
    /*else
      DBG_PROG_S("out of range");
    return list_[reserve_ + 1000000000]; // create exception */
  }

  /** @brief constant index access operator */
  const T& operator [] (const size_t i) const {
    //if(i < n_)
      return ICClist<T>::operator[](i);
    /*else
      DBG_PROG_S("out of range");
    return list_[reserve_ + 1000000000]; // create exception */
  }
  
  ICCThreadList () {;}
  ~ICCThreadList () {;}
  ICCThreadList (const ICCThreadList & s) {
                              copy(s); }
  ICCThreadList& copy  (const ICCThreadList& s) {
                              ICClist<T>::copy(s);
                              icc_examin_ns::ThreadDaten::copy(s);
                              return *this; }
  ICCThreadList& operator = (const ICCThreadList& d) {
                              return copy(d); }

};

}

#endif //ICC_THREAD_DATEN_H
