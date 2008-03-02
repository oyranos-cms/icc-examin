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
 * Farbumfang betrachten im WWW-3D Format vrml: Parser
 * 
 */

// Date:      Februar 2004

#include <sstream>
#include <string>
#include <vector>

#include "icc_helfer.h"
#include "icc_vrml_parser.h"

#define lp {l+=0.1; icc_examin->fortschritt(l);}

using namespace icc_parser;

class ICCvrmlParser
{
    std::string original_;
    std::string arbeit_;
    std::vector<ICCnetz> netze_;
  public:
    void lade ( std::string & vrml )   { original_ = vrml; }
    std::vector<ICCnetz> zeigeNetze () { lesen_(); return netze_; }
  private:
    void lesen_()
{
  int netz_n = 0;
  std::vector<std::string> zeilen;
  std::string::size_type pos=0, netz_pos=0;
  std::string::size_type ende, netz_ende;

  // zeilenweises Bearbeiten
  for (unsigned i = 0; i < zeilen.size(); ++i)
  {
    ++netz_pos;
    // Bereich des Netzes eingrenzen
    netz_pos = original_.find( "IndexedFaceSet", netz_pos );
    if( (netz_ende = original_.find( "IndexedFaceSet", netz_pos )) ==
        std::string::npos )
      ende = original_.size();
    if( netz_pos == std::string::npos )
      break;

    arbeit_ = original_.substr (netz_pos, netz_ende-pos+1);
    zeilen = zeilenNachVector (arbeit_);

    // ab nun zeilenweise
    bool in_punkte = false;
    bool in_farben = false;
    bool in_indexe = false;
    netze_.resize( netze_.size()+1 );
    int netz_ende = netze_.size()-1;
    std::string zeile;
    for(unsigned int z = 0; z < zeilen .size(); z++)
    {
      // Hole eine kommentarfreie Zeile
      zeile = zeilen[i].substr( 0, zeilen[i].find( "#" ) );

      int geschweifte_klammer = 0; pos = 0;
      while( (pos=zeile.find("{",pos)) != std::string::npos )
        ++geschweifte_klammer;
      pos = 0;
      while( (pos=zeile.find("}",pos)) != std::string::npos )
        --geschweifte_klammer;

      #define VRMLBereichsTest(schalter, schluesselauf, schluesselzu) \
      if( zeile.find(schluesselauf) == std::string::npos ) \
        schalter = true; \
      if( zeile.find(schluesselzu) == std::string::npos && \
          schalter ) \
        schalter = false;
      if(geschweifte_klammer > 1) {
        VRMLBereichsTest ( in_punkte, "points", "]" )
        VRMLBereichsTest ( in_farben, "Color", "}" )
        VRMLBereichsTest ( in_indexe, "coordIndex", "]" )
      }

      if( in_punkte ) {
        char trennzeichen[12];
        trennzeichen[0] = ',';
        sprintf(&trennzeichen[1], leer_zeichen);
        bool anfuehrungstriche = false;
        std::vector<ZifferWort> werte =
          unterscheideZiffernWorte( zeile, anfuehrungstriche, trennzeichen );

        int punkte_n = 0;
        for(unsigned int w = 0; w < werte.size(); ++w)
        {
          const unsigned int dimension = 3;
          int achse = 0;
          if(!punkte_n%dimension &&
             netze_[netz_ende].punkte.size() < punkte_n/dimension) {
            netze_[netz_ende].punkte.push_back(ICCnetzPunkt());
            achse = 0;
          }

          if(werte[w].zahl.first) {
            netze_[netz_ende].punkte[punkte_n].koord[achse] =
                werte[w].zahl.second;
            ++achse;
          }
          if(!punkte_n%dimension) {
            netze_[netz_ende].punkte.push_back(ICCnetzPunkt());
            ++punkte_n;
          }
        } 
      }

    }

    netz_n++;
  }
}

};



std::vector<ICCnetz>
extrahiereNetzAusVRML (std::string & vrml)
{
  ICCvrmlParser vrml_parser;
  vrml_parser.lade(vrml);
  return vrml_parser.zeigeNetze();
}



