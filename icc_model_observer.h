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

#ifndef ICC_MODELL_BETRACHTER_H
#define ICC_MODELL_BETRACHTER_H

#include "icc_utils.h"

#include <list>
#include <string>

/*
 *  mode of work
 *
 *  The class model provides the means to observe a condition.
 *  Observers of the class beobachter can subscribe to changes in model.
 *  A simple whole number informs about the changing state.
 *  It is important to know what kind the model is and to interprete the
 *  obtained number, possibly as a enumeration.
 *  The reduction to one argument type reduses the instanciations and
 *  increases the effort of interpretation.
 *  Models and observers should know each about the other. (Alternatively
 *  it would be possible to provide a reference to the model to know whether the
 *  observer still exists. But with destroying of the observer class a 
 *  information can easily sent to the model.)
 *
 *  Usage:
 *  It is needed to derive from the following classes and implement the
 *  virtual functions.
 */

namespace icc_examin_ns {

  class Model;

  class Observer
  {
      std::list<Model*> model_;
    protected:
      virtual ~Observer();
    public:
      Observer();
      virtual void message ( Model* model , int infos );
      void modelDel ( Model* model );
      void modelAdd ( Model* model );
  };

  class Model
  {
    friend class Observer;
    protected:
      Model();
      void observerAdd(Observer* beo);
      void observerDel(Observer* beo);
      void notify(int infos);

      virtual ~Model();
    private:
      std::list<Observer*> observer_;
  };

  class aModel : public Model
  {
    public:
      aModel() { ; }
      ~aModel() { ; }
      void observerAdd(Observer* beo) { Model::observerAdd(beo); }
      void observerDel(Observer* beo) { Model::observerDel(beo); }
      void notify(int infos)          { Model::notify(infos); }
  };

}


#endif // ICC_MODELL_BETRACHTER_H

