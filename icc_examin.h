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
 * Die zentrale Klasse
 * 
 */

// Date:      Mai 2004

#ifndef ICC_EXAMIN_H
#define ICC_EXAMIN_H

#include <string>
#include <vector>

//#include "icc_betrachter.h"
class ICCfltkBetrachter;

class ICCexamin
{
  public:
                 ICCexamin ();
                 ~ICCexamin ();

    void         start(int argc, char** argv);

    void         oeffnen (std::vector<std::string> dateinamen);
    void         oeffnen ();	// interaktiv
    std::string  selected_tag (int item);

	// Liste der geladenen Profile
	std::vector<std::string> profilnamen;

//  private:
    ICCfltkBetrachter* icc_betrachter;
};

extern ICCexamin *icc_examin;

#endif //ICC_EXAMIN_H

