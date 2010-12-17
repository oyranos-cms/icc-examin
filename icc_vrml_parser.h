/*
 * ICC Examin ist eine ICC Profil Betrachter
 * 
 * Copyright (C) 2005-2008  Kai-Uwe Behrmann 
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

#include <map>
#include "icc_helfer.h"
#include "icc_thread_daten.h"
#include "icc_oyranos_extern.h"

/** a point in space with calculated colour */
struct ICCnetzPunkt {
  double koord[3]; //!< allways Lab 0..1
  double farbe[4]; //!< eigther from VRML parsed or calculated otherwise
};

/** point indexes for a representable plain */
struct DreiecksIndexe {
  int i[4];        //!< referes to ICClist<ICCnetzPunkt>punkte
  double normale[3]; //!< surface normals
  double midpoint[3];//!< midpoint
};

/** displayable informations to a colour space gamut */
struct ICCnetz {
  ICClist<ICCnetzPunkt> punkte;                  //!< single netz points
  std::multimap<double,DreiecksIndexe> indexe;   //!< assigning the points
  ICClist<ICCnetzPunkt> umriss;                  //!< mesh hull, a line
  std::string name;                              //!< name
private:
  int aktiv;                                     //!< show?
public:
  void active(int a) { WARN_S(aktiv<<" "<<a); aktiv = a;}
  int active() const {return aktiv;}
  int grau;                                      //!< no colour?
  double undurchsicht;                           //!< alpha 0..1
  double schattierung;                           //!< L* colour for gray 0..1
  double volume;                                 //*< net volume if available
  int kubus;                                     //!< generated from a cubus

             ICCnetz() {
    init();
  }
  void       prepare() { //!< calculate normals for all triangles

  }
  void       init() {
    aktiv = 1;
    grau = -1;
    undurchsicht = -1.0;
    schattierung = -1.0;
    volume = -1.0;
    kubus = 0;
  }
  void       clear() {   //!< release all memory
    punkte.clear();
    indexe.clear();
    umriss.clear();
    name.clear();
    init();
  }
  void       insert( const ICCnetz & netz ) {
    if(!netz.aktiv)
      return;

    int punkte_n = punkte.size();

    punkte. insert( punkte.begin() + punkte.size() ,
                    netz.punkte.begin(), netz.punkte.end() );

    std::multimap<double,DreiecksIndexe>::const_iterator it;
    for( it = netz.indexe.begin(); it != netz.indexe.end(); ++it )
    {
       int i[3]; // indices

               // insert indices, to count newly
       /*A*/ std::pair<double,DreiecksIndexe> index_p( *it );
               // adapt indices
       /*B*/ for( int k = 0; k < 3; ++k) {
               index_p.second.i[k] += punkte_n;
               i[k] = index_p.second.i[k];
             }
       /*C*/
               // midpoint of the triangle
             double seitenhalbierende[3];
             seitenhalbierende[0] =
                    (  (punkte[i[0]].koord[0])
                     + (punkte[i[1]].koord[0]))/2.0;
             seitenhalbierende[1] =
                    (  (punkte[i[0]].koord[1])
                     + (punkte[i[1]].koord[1]))/2.0;
             seitenhalbierende[2] =
                    (  (punkte[i[0]].koord[2])
                     + (punkte[i[1]].koord[2]))/2.0;
             index_p.second.midpoint[0] = (  2.0 * seitenhalbierende[0]
                                + punkte[i[2]].koord[0])
                              / 3.0;
             index_p.second.midpoint[1] = (  2.0 * seitenhalbierende[1]
                                + punkte[i[2]].koord[1])
                              / 3.0;
             index_p.second.midpoint[2] = (  2.0 * seitenhalbierende[2]
                                + punkte[i[2]].koord[2])
                              / 3.0;
             index_p.first = 1.0; // distant

       /*D*/
          // calculate normals for all triangles
          double normale[3], len=1.0, v1[3], v2[3];
          {
              // cross product
              v1[0] = punkte[i[1]].koord[0]-
                      punkte[i[0]].koord[0];
              v1[1] = punkte[i[1]].koord[1]-
                      punkte[i[0]].koord[1];
              v1[2] = punkte[i[1]].koord[2]-
                      punkte[i[0]].koord[2];
              v2[0] = punkte[i[2]].koord[0]-
                      punkte[i[0]].koord[0];
              v2[1] = punkte[i[2]].koord[1]-
                      punkte[i[0]].koord[1];
              v2[2] = punkte[i[2]].koord[2]-
                      punkte[i[0]].koord[2];
              // determine area normals
              normale[0] =   v1[2]*v2[1] - v1[1]*v2[2];
              normale[1] =   v1[0]*v2[2] - v1[2]*v2[0];
              normale[2] =   v1[1]*v2[0] - v1[0]*v2[1];
              len = HYP3( normale[0],normale[1],normale[2] );
              // Einheitsvektor der Normale setzen
              index_p.second.normale[0] = normale[0]/len;
              index_p.second.normale[1] = normale[1]/len;
              index_p.second.normale[2] = normale[2]/len;

          }
               // the container std::map does sorting
       /*E*/ indexe.insert(index_p);

       for(unsigned k = 0; k < 3; ++k)
       {
         if(netz.grau)
         {
           punkte[i[k]].farbe[0] = netz.schattierung;
           punkte[i[k]].farbe[1] = netz.schattierung;
           punkte[i[k]].farbe[2] = netz.schattierung;
         }
         punkte[i[k]].farbe[3] = netz.undurchsicht;
       }
    }

       // new base for index numbers
    punkte_n += (int)netz.punkte.size();

  }
  /*ICCnetz&   copy    (const ICCnetz & net) {
    return *this;
  }
             ICCnetz (const ICCnetz & net) {
    copy( net );
  }*/
};


icc_examin_ns::ICCThreadList<ICCnetz> extrahiereNetzAusVRML (std::string &vrml);
std::string netzNachVRML( icc_examin_ns::ICCThreadList<ICCnetz> & netze );
std::string    writeTextArrows       ( double              text_colour,
                                       double              arrow_colour );
std::string    vrmlScene          ( icc_examin_ns::ICCThreadList<ICCnetz> netze,
                                       oyStructList_s    * colours,
                                       double              text_colour,
                                       double              arrow_colour,
                                       double              background_colour,
                                       double              diameter );
std::string    writeVRMLbody         ( std::string         content );



#endif // ICC_VRML_PARSER_H
