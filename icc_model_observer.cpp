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
#include <unistd.h> // intptr_t

#include "icc_model_observer.h"


using namespace icc_examin_ns;

// --- observer ---

Observer::Observer()
{
  DBG_PROG_START
  DBG_PROG_ENDE
}

Observer::~Observer()
{
  DBG_PROG_START
  std::list<Model*>::iterator m_it;
  for(m_it = model_.begin(); m_it != model_.end(); ++m_it)
  {
    (*m_it)->observerDel(this);
  }
  DBG_PROG_ENDE
}

void
Observer::modelAdd ( Model* model )
{
  DBG_PROG_START
  model_.push_back (model);
  model->observerAdd(this);
  DBG_PROG_ENDE
}

void
Observer::modelDel(Model* model)
{
  DBG_PROG_START
  std::list<Model*>::iterator m_it;
  for(m_it = model_.begin(); m_it != model_.end(); ++m_it)
  {
    if(model == *m_it)
    {
      model_.erase ( m_it );
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
Observer::message ( Model* model , int infos )
{
  DBG_PROG;
}


// --- model ---

Model::Model()
{
  DBG_PROG_START
  DBG_PROG_ENDE
}

Model::~Model()
{
  DBG_PROG_START
  // unregister with all observers
  std::list<Observer*>::iterator it;
  for(it = observer_.begin(); it != observer_.end(); ++it)
  {
    (*it)->modelDel(this);
  }
  DBG_PROG_ENDE
}

void
Model::observerAdd(Observer* beo)
{
  DBG_PROG_START
  observer_.push_back (beo);
  DBG_PROG_ENDE
}

void
Model::observerDel(Observer* beo)
{
  DBG_PROG_START
  if(!observer_.size())
    return;

  // delete single observer from the list
  std::list<Observer*>::iterator it = observer_.begin();
  intptr_t isis = (intptr_t)*it;
  if(isis > 0)
  {
    for(; it != observer_.end(); ++it)
    {
      if(beo == *it)
      {
        observer_.erase ( it );
        break;
      }
    }
  }

  DBG_PROG_ENDE
}


/** @brief inform all my observers */
void
Model::notify(int infos)
{
  DBG_PROG_START
  std::list<Observer*>::iterator it;
  for(it = observer_.begin(); it != observer_.end(); ++it)
  {
    (*it)->message(this, infos);
  }
  DBG_PROG_ENDE
}


