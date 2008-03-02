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

#ifndef ICC_MODELL_BETRACHTER_H
#define ICC_MODELL_BETRACHTER_H

#include "icc_utils.h"

#include <list>
#include <string>

/*
 *  Arbeitsweise
 *
 *  Die Klasse modell stellt einen beobachtbaren Zustand dar.
 *  Beobachter der Klasse beobachter können sich über den Zustand von modell
 *  informieren lassen.
 *  Eine einfache Ganzzahl informiert Ã¼ber den sich geÃ¤nderten Zustand.
 *  Dabei ist es wichtig sein Modell zu kennen und den Ã¼bergebenen Wert zu
 *  interpretieren.
 *  Die Festlegung auf einen Argumenttyp reduziert die Instanzierungen und
 *  erhöht den Interpretationsaufwand.
 *  Modelle und Beobachter wissen alle Übereinander. (Es wäre alternativ möglich
 *  einen Händling zu übergeben, an dem abgelesen werden könnte ob die
 *  Beobachterklasse noch existiert.)
 *
 *  Benutzung:
 *  Von den folgenden Klassen muss abgeleitet
 *  und die virtuellen Funktionen konkretisiert werden.
 */

namespace icc_examin_ns {

  class Modell;

  class Beobachter
  {
      std::list<Modell*> modell_;
    protected:
      virtual ~Beobachter();
    public:
      Beobachter();
      virtual void nachricht ( Modell* modell , int infos );
      void modellFort ( Modell* modell );
      void modellDazu ( Modell* modell );
  };

  class Modell
  {
    friend class Beobachter;
    protected:
      Modell();
      void beobachterDazu(Beobachter* beo)
           { beobachter_.push_back (beo); }
      void beobachterFort(Beobachter* beo);
      void benachrichtigen(int infos);

      virtual ~Modell();
    private:
      std::list<Beobachter*> beobachter_;
  };

}


#endif // ICC_MODELL_BETRACHTER_H

