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
 * visualise colour gamut in WWW-3D format vrml: parser
 * 
 */

// Beginn Datum:      Februar 2005

#ifndef ICC_VRML_PARSER_H
#define ICC_VRML_PARSER_H

#include <vector>
#include <map>

/** a point in space with calculated colour */
struct ICCnetzPunkt {
  double koord[3]; //!< allways Lab 0..1
  double farbe[4]; //!< eigther from VRML parsed or calculated otherwise
};

/** point indexes for a representable plain */
struct DreiecksIndexe {
  int i[4];        //!< referes to std::vector<ICCnetzPunkt>punkte
};

/** displayable informations to a colour space gamut */
struct ICCnetz {
  std::vector<ICCnetzPunkt> punkte;              //!< single netz points
  std::multimap<double,DreiecksIndexe> indexe;   //!< assigning the points
  std::vector<ICCnetzPunkt> umriss;              //!< mesh hull, a line
  std::string name;                              //!< name
  int aktiv;                                     //!< show?
  int grau;                                      //!< no colour?
  double undurchsicht;                           //!< alpha 0..1
  double schattierung;                           //!< L* colour for gray 0..1
  int kubus;                                     //!< generated from a cubus
  ICCnetz() {
    aktiv = 1;
    grau = 0;
    undurchsicht = 0.5;
    schattierung = 0.2;
    kubus = 0;
  }
};

std::vector<ICCnetz> extrahiereNetzAusVRML (std::string & vrml);



#endif // ICC_VRML_PARSER_H
