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
 * Implementation des Modell/Betrachter Musters (observer pattern)
 * 
 */

//Datum: 26. Februar 2005


#include "icc_utils.h"

#include <list>
#include <string>

#include "icc_modell_beobachter.h"


using namespace icc_examin_ns;

// --- Beobachter ---

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

/** @brief Empf&auml;nger von Modell

    Diese Funktion übernimmt die Informationen aus dem Modell und veranlasst
    eine entsprechende Reaktion. \n
    in der abgeleiteten Klasse konkretisieren
 */
void
Beobachter::nachricht ( Modell* modell , int infos )
{
  DBG_PROG;
}


// --- Modell ---

Modell::Modell()
{
  DBG_PROG_START
  DBG_PROG_ENDE
}

Modell::~Modell()
{
  DBG_PROG_START
  // bei allen Beobachtern abmelden
  std::list<Beobachter*>::iterator it;
  for(it = beobachter_.begin(); it != beobachter_.end(); ++it)
  {
    (*it)->modellFort(this);
  }
  DBG_PROG_ENDE
}

void
Modell::beobachterFort(Beobachter* beo)
{
  DBG_PROG_START
  // einzelnen Beobachter aus der Liste entfernen
  std::list<Beobachter*>::iterator it;
  for(it = beobachter_.begin(); it != beobachter_.end(); ++it)
  {
    if(beo == *it)
    {
      beobachter_.erase ( it );
      break;
    }
  }

  if(!beobachter_.size())
    delete this;

  DBG_PROG_ENDE
}


/** @brief alle meine Beobachter informieren */
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


