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
 * Implementation of the model/observer pattern
 * 
 */

//Datum: 26. Februar 2005


#include "icc_utils.h"

#include <list>
#include <string>

#include "icc_modell_beobachter.h"


using namespace icc_examin_ns;

// --- observer ---

Beobachter::Beobachter()
{
  DBG_PROG_START
  DBG_PROG_ENDE
}

Beobachter::~Beobachter()
{
  DBG_PROG_START
  std::list<Modell*>::iterator m_it;
  for(m_it = modell_.begin(); m_it != modell_.end(); ++m_it)
  {
    (*m_it)->beobachterFort(this);
  }
  DBG_PROG_ENDE
}

void
Beobachter::modellDazu ( Modell* modell )
{
  DBG_PROG_START
  modell_.push_back (modell);
  modell->beobachterDazu(this);
  DBG_PROG_ENDE
}

void
Beobachter::modellFort(Modell* modell)
{
  DBG_PROG_START
  std::list<Modell*>::iterator m_it;
  for(m_it = modell_.begin(); m_it != modell_.end(); ++m_it)
  {
    if(modell == *m_it)
    {
      modell_.erase ( m_it );
      break;
    }
  }
  DBG_PROG_ENDE
}

/** @brief observer of news from model

    This function recieves the informations from the model and can cause
    a according reaktion. \n
    in the derived class to implement
 */
void
Beobachter::nachricht ( Modell* modell , int infos )
{
  DBG_PROG;
}


// --- model ---

Modell::Modell()
{
  DBG_PROG_START
  DBG_PROG_ENDE
}

Modell::~Modell()
{
  DBG_PROG_START
  // unregister with all observers
  std::list<Beobachter*>::iterator it;
  for(it = beobachter_.begin(); it != beobachter_.end(); ++it)
  {
    (*it)->modellFort(this);
  }
  DBG_PROG_ENDE
}

void
Modell::beobachterDazu(Beobachter* beo)
{
  DBG_PROG_START
  beobachter_.push_back (beo);
  DBG_PROG_ENDE
}

void
Modell::beobachterFort(Beobachter* beo)
{
  DBG_PROG_START
  if(!beobachter_.size())
    return;

  // delete single observer from the list
  std::list<Beobachter*>::iterator it = beobachter_.begin();
  intptr_t isis = (intptr_t)*it;
  if(isis > 0)
  {
    for(; it != beobachter_.end(); ++it)
    {
      if(beo == *it)
      {
        beobachter_.erase ( it );
        break;
      }
    }
  }

  DBG_PROG_ENDE
}


/** @brief inform all my observers */
void
Modell::benachrichtigen(int infos)
{
  DBG_PROG_START
  std::list<Beobachter*>::iterator it;
  for(it = beobachter_.begin(); it != beobachter_.end(); ++it)
  {
    (*it)->nachricht(this, infos);
  }
  DBG_PROG_ENDE
}


