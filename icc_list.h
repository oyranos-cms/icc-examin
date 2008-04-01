/*
 * ICC Examin ist eine ICC Profil Betrachter
 * 
 * Copyright (C) 2007-2008  Kai-Uwe Behrmann 
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
 * ICC helpers
 * 
 */

// Date:      September 2007

#ifndef ICC_LIST_H
#define ICC_LIST_H

#include <stddef.h> // size_t

// data structures
// partitial in icc_speicher.h
/** @brief list class template
 */
template <typename T>
class ICClist {
  size_t     n_;          //!< the exposed or valid number of elements
  size_t     reserve_;    //!< the allocated elements in list_
  T        * list_;       //!< the data array of type T elements
  /** @brief set to defaults, ignore allocations */
  void       init() {
    zero();
    reserve_step = 1000/sizeof(T);
  }
  void       zero() {
    list_ = NULL;
    n_ = 0;
    reserve_ = 0;
  }
public:
             ICClist () {
    init();
  }
             ICClist (int n) {
    init();
    reserve(n + reserve_step);
  }
            ~ICClist () {
    clear();
  }
             ICClist (const ICClist & s) {
    init();
    copy(s);
  }
  size_t reserve_step; //!< reservation of elements on some allocations

  /** @brief copy the objects */
  ICClist&   copy    (const ICClist & l) {
    if(l.n_ > n_)
    {
      if(list_)
        delete [] list_;
      list_ = new T[l.n_];
      reserve_ = l.n_;
    }
    if(l.n_)
      for(size_t i = 0; i < l.n_; ++i)
        list_[i] = l.list_[i];
    n_ = l.n_;
    return *this;
  }

  /** @brief free and reset */
  void       clear   () {
    if(list_)
      delete [] (list_);
    zero();
  }
  const size_t size() const {
    return n_;
  }
  operator const size_t () const {
    return n_;
  }
  operator const size_t & () const {
    return n_;
  }

  /** @brief same as copy */
  ICClist& operator = (const ICClist& l) {
    return copy(l);
  }

  /** @brief index access operator 
   *
   *  no check in this basic class
   */
  T &      operator [] (size_t i) {
    //if(i < n_)
      return list_[i];
    /*else
      DBG_PROG_S("out of range");
    return list_[reserve_ + 1000000000]; // create exception */
  }

  /** @brief constant index access operator */
  const T& operator [] (const size_t i) const {
    //if(i < n_)
      return list_[i];
    /*else
      DBG_PROG_S("out of range");
    return list_[reserve_ + 1000000000]; // create exception */
  }

  /** @brief index access operator */
  T & at (size_t i) {
    return list_[i];
  }

  /** @brief constant index access operator */
  const T& at (const size_t i) const {
    return list_[i];
  }

  /** @brief add one element

   *  allocate new mem, once the limit is reached
   */
  void     push_back( T x ) {
    if(n_ >= reserve_)
      reserve( n_ + 1 + reserve_step );
    list_[n_] = x;
    ++n_;
  }

  T *      begin() {
    return &list_[0];
  }
  const T* begin() const {
    return &list_[0];
  }
  T *      end() {
    if(n_)
      return &list_[n_ - 1];
    else
      return NULL;
  }
  const T* end() const {
    if(n_)
      return &list_[n_ - 1];
    else
      return NULL;
  }

  /** @brief reserve if needed */
  void       reserve (size_t x) {
    if ( n_ == 0 && !reserve_ ) {
      list_ = new T[x];
      reserve_ = x;
    /* only enlarge */
    } else if( reserve_ < x ) {
      T* tmp = new T[x];
      reserve_ = x;
      for(size_t i = 0; i < n_; ++i)
        tmp[i] = list_[i];
      delete [] list_;
      list_ = tmp;
    }
  }

  /** @brief insert allocating automatically */
  void     insert( T* start_, const T* begin_, const T* end_ ) {
    size_t news = (size_t)((intptr_t)(end_ - begin_)) + 1;
    size_t begin_i = 0;
    if(&list_[n_] < start_)
      return;
    if(begin_ > end_ || !begin_)
      return;

    if(start_)
      begin_i =(size_t)((intptr_t)(start_ - begin()));
    else
      begin_i = 0;

    reserve( begin_i + news + reserve_step );

    if(start_)
      start_ = &list_[begin_i];
    else
      start_ = &list_[0];

    for(size_t i = 0; i < news; ++i)
      start_[i] = begin_[i];
    if( n_ < begin_i + news )
      n_ = begin_i + news;
  }
  /** @brief insert with automatic allocation */
  void     insert( T* start_, const T & x ) {
    size_t news = 1;
    size_t begin_i = 0;
    if(&list_[n_] < start_)
      return;

    if(start_)
      begin_i =(size_t)((intptr_t)(start_ - begin()));
    else
      begin_i = 0;

    if(begin_i + news < reserve_)
    {
      reserve( begin_i + news + reserve_step );
      start_ = &list_[begin_i];
    }

    if(!start_)
      start_ = &list_[0];

    start_[0/*begin_i*/] = T(x);
    if( n_ < begin_i + news )
      n_ = begin_i + news;
  }

  /** @brief erase with no deallocation */
  void     erase( T* x ) {
    for(size_t i = 0; i < n_; ++i)
      if(&list_[i] == x)
      {
        memmove( &list_[i], &list_[i+1], sizeof(T) * (n_ - i - 1) );
        -- n_;
        break;
      }
  }

  /** @brief ignore unused elements, avoid reallocation as possible */
  void     resize( size_t n ) {
    if(n > reserve_)
      reserve (n + reserve_step);
    n_ = n;
  }
};

#endif //ICC_LIST_H
