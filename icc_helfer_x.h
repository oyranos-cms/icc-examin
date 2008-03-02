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
 * ICC Helfer - X abh�ngig
 * 
 */

// Date:      Januar 2005

#ifndef ICC_HELFER_X_H
#define ICC_HELFER_X_H

#include <string>
#include <vector>

// Helferfunktionen
// definiert in icc_helfer_x.cpp

std::vector<std::vector<double> > getXgamma ( std::string display_name,
                                              std::vector<std::string> &texte ); 



#endif //ICC_HELFER_X_H