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

// start date:      Februar 2005


#include "icc_helfer.h"
#include "icc_vrml_parser.h"

#include <sstream>
#include <string>
#include <vector>

#define lp {l+=0.1; icc_examin->fortschritt(l);}

//#define DEBUG_VRML_PARSER 1
#if DEBUG_VRML_PARSER
#define DBG_VRML_PARSER_V(text) DBG_NUM_V(text)
#define DBG_VRML_PARSER_S(text) DBG_NUM_S(text)
#else
#define DBG_VRML_PARSER_V(text)
#define DBG_VRML_PARSER_S(text)
#endif

using namespace icc_parser;

class ICCvrmlParser
{
    std::string original_;
    std::string arbeit_;
    icc_examin_ns::ICCThreadList<ICCnetz> netze_;
  public:
    void lade ( std::string & vrml );
    icc_examin_ns::ICCThreadList<ICCnetz> zeigeNetze ();
  private:
    void lesen_ ();

};

void
ICCvrmlParser::lade( std::string & vrml )
{
  DBG_PROG_START
  DBG_PROG
  original_ = vrml;
  DBG_PROG_ENDE
}

icc_examin_ns::ICCThreadList<ICCnetz>
ICCvrmlParser::zeigeNetze ()
{
  DBG_PROG_START
  DBG_PROG
  lesen_();
  DBG_PROG_ENDE
  return netze_;
}

void
ICCvrmlParser::lesen_ ()
{
  int debug_alt = icc_debug;
  //icc_debug = 1;
  DBG_PROG_START
  int netz_n = 0;                       // the n-th net/mesh
  std::vector<std::string> zeilen;      // editing row
  std::vector<ZifferWort> werte;        // intermediate return value
  char trennzeichen[12];                // to be used separating signs
  trennzeichen[0] = ',';
  sprintf(&trennzeichen[1], leer_zeichen);
  const bool anfuehrungstriche = false; // set no quotation marks
  unsigned int dimensionen;             // number of belonging values
  int achse;                            // selected value from dimensionen
  DBG_PROG_V( original_.size() )

  std::string::size_type pos=0, netz_pos=0;
  std::string::size_type netz_ende;


# if DEBUG_VRML_PARSER
  double max_farbe[3],  min_farbe[3], min_pos[3], max_pos[3];
  for(int i = 0; i < 3; ++i) {
    max_farbe[i] = max_pos[i] = -100000.0;
    min_farbe[i] = min_pos[i] =  100000.0;
  }
# endif

  // locale - differenciate commas
  const char* temp = setlocale(LC_NUMERIC, NULL);
  char* loc_alt = icc_strdup_m(temp); //getenv("LANG");
  if(loc_alt)
    DBG_NUM_V( loc_alt )
  else
    DBG_NUM_S( "did not find LANG variable" )
  setlocale(LC_NUMERIC,"C");

  // search for mesh in vrml file
  while( (netz_pos = original_.find( "IndexedFaceSet", netz_pos )) !=
        std::string::npos )
  {
    ++netz_pos;
    // get the area of the mesh
    if( (netz_ende = original_.find( "IndexedFaceSet", netz_pos )) ==
        std::string::npos )
      netz_ende = original_.size();
    if( netz_pos == std::string::npos )
      break;

    DBG_VRML_PARSER_S( "IndexedFaceSet found at position " << netz_pos <<"-"<< netz_ende );

    arbeit_ = original_.substr (netz_pos-1, netz_ende-netz_pos+1);
    zeilen = zeilenNachVector (arbeit_);
    DBG_PROG_V( zeilen.size() <<"|"<< arbeit_.size() )

    int index_max = - 1, index_min = 1000000000;

    // from now row wise
    int flaeche_klammer = false;
    //int in_coordinate = false;
    bool in_punkte = false;
    bool in_farben = false;
    bool in_indexe = false; DBG_MEM_V( netze_.size() )
    netze_.resize( netze_.size()+1 ); DBG_MEM_V( netze_.size() )
    int endnetz = netze_.size()-1;
    std::string zeile; DBG_MEM
    int geschweifte_klammer = 0;
    int punkt_n = 0,
        wert_n = 0;
    for(unsigned int z = 0; z < zeilen .size(); z++)
    {
      // take a comment free line/row
      zeile = zeilen[z].substr( 0, zeilen[z].find( "#" ) );

      pos = 0;
      while( (pos = zeile.find("{",pos)) != std::string::npos ) {
        DBG_VRML_PARSER_V( zeilen[z] )
        ++geschweifte_klammer;
        ++pos;
      }
      pos = 0;
      while( (pos = zeile.find("}",pos)) != std::string::npos ) {
        DBG_VRML_PARSER_V( zeilen[z] )
        --geschweifte_klammer;
        ++pos;
      }

#     define VRMLBereichsTest(schalter, schluesselauf, schluesselzu) \
      if( zeile.find(schluesselauf) != std::string::npos ) { \
        DBG_VRML_PARSER_S( z << schluesselauf ) \
        wert_n = punkt_n = 0; \
        schalter = true; DBG_VRML_PARSER_V( schalter ) \
      } \
      if( zeile.find(schluesselzu) != std::string::npos && \
          schalter ) { \
        DBG_VRML_PARSER_S( z << schluesselzu ) \
        schalter = false; \
      }

      //if(geschweifte_klammer >= 1) 
      {
        if( flaeche_klammer < geschweifte_klammer ) {
          if( zeile.find("IndexedFaceSet") != std::string::npos ) {
            DBG_VRML_PARSER_S( z << " IndexedFaceSet " << flaeche_klammer )
            flaeche_klammer = geschweifte_klammer;
          }
        }
        if( flaeche_klammer > geschweifte_klammer )
          flaeche_klammer = false;
        if( flaeche_klammer ) {
          VRMLBereichsTest ( in_punkte, "point", "]" )
          VRMLBereichsTest ( in_farben, "Color", "}" )
          VRMLBereichsTest ( in_indexe, "coordIndex", "]" )
        }
      }
      DBG_VRML_PARSER_S( z <<" "<< geschweifte_klammer <<" "<< flaeche_klammer <<" "<< in_punkte <<" "<< in_farben <<" "<< in_indexe )
      achse = 0;

      if( in_punkte )
      {
        werte =
          unterscheideZiffernWorte( zeile, anfuehrungstriche, trennzeichen );

        // sort the read values
        for(unsigned int w = 0; w < werte.size(); ++w)
        { DBG_VRML_PARSER_S( "w [" << w <<"]" << werte.size() <<" net: "<< endnetz <<" poinkts "<< netze_[endnetz].punkte.size() )
          if(werte[w].zahl.first)
          {
            dimensionen = 3;
            achse = wert_n%dimensionen;
            DBG_VRML_PARSER_S( "w%3 " << wert_n%dimensionen <<" p_s "<< netze_[endnetz].punkte.size() <<" w/3 "<< wert_n/dimensionen )
            if(achse == 0)
            {
              if(netze_[endnetz].punkte.size() <= wert_n/dimensionen)
                netze_[endnetz].punkte. push_back (ICCnetzPunkt());

              punkt_n = wert_n/dimensionen;
            }

            DBG_VRML_PARSER_S( "w " << w <<" "<< werte[w].zahl.second )
            if(achse == 2)
              netze_[endnetz].punkte[punkt_n].koord[0] =
                  werte[w].zahl.second / 100.0 + 0.5;
            else
              netze_[endnetz].punkte[punkt_n].koord[achse+1] =
                  werte[w].zahl.second / 255.0 + 0.5;

#           if DEBUG_VRML_PARSER
            if(min_pos[achse] > werte[w].zahl.second)
              min_pos[achse] = werte[w].zahl.second;
            if(max_pos[achse] < werte[w].zahl.second)
              max_pos[achse] = werte[w].zahl.second;
#           endif

            DBG_VRML_PARSER_S( endnetz<<" punkt_n "<<punkt_n<<" achse "<<achse<<" "<< netze_[endnetz].punkte[punkt_n].koord[achse] )

            ++wert_n;
          } //else WARN_S(_("keine Zahl?? " << werte[w].zahl.second))
        }
      } else
      if( in_farben )
      {
        werte =
          unterscheideZiffernWorte( zeile, anfuehrungstriche, trennzeichen );

        // sort the read values
        for(unsigned int w = 0; w < werte.size(); ++w)
        {
          if(werte[w].zahl.first)
          {
            dimensionen = 3;
            achse = wert_n % dimensionen;
            if(achse == 0)
            {
              if(netze_[endnetz].punkte.size() <= wert_n/dimensionen)
                netze_[endnetz].punkte. push_back (ICCnetzPunkt());

              punkt_n = wert_n/dimensionen;
            } else if (achse == 2)
              netze_[endnetz].punkte[punkt_n].farbe[achse+1] = 1;

            DBG_VRML_PARSER_S( "w " << w <<" "<< werte[w].zahl.second )

            netze_[endnetz].punkte[punkt_n].farbe[achse] =
                werte[w].zahl.second;

#           if DEBUG_VRML_PARSER
            if(min_farbe[achse] > werte[w].zahl.second)
              min_farbe[achse] = werte[w].zahl.second;
            if(max_farbe[achse] < werte[w].zahl.second)
              max_farbe[achse] = werte[w].zahl.second;
#           endif

            ++wert_n;
          } //else WARN_S(_("keine Zahl?? " << werte[w].zahl.second))
        }
      } else // in_farben
      if( in_indexe )
      {
        werte =
          unterscheideZiffernWorte( zeile, anfuehrungstriche, trennzeichen );

        // sort the read values
        std::pair<double,DreiecksIndexe> index_p;
        for(unsigned int w = 0; w < werte.size(); ++w)
        {
          if(werte[w].ganz_zahl.first)
          {
            dimensionen = 4;
            achse = wert_n % dimensionen;
            if(achse == 0)
              punkt_n = wert_n/dimensionen;

            DBG_VRML_PARSER_S( "w " << w <<" "<< werte[w].ganz_zahl.second )
            DBG_VRML_PARSER_V( netze_[endnetz].indexe.size() )

            index_p.second.i[achse] =
                werte[w].ganz_zahl.second;

            if(index_min > index_p.second.i[achse] && w < 3)
              index_min = index_p.second.i[achse];
            else if(index_max < index_p.second.i[achse])
              index_max = index_p.second.i[achse]; 

            if(achse == (int)dimensionen - 1)
              if(netze_[endnetz].indexe.size() <= wert_n/dimensionen)
                netze_[endnetz].indexe.  insert( index_p );

            ++wert_n;
          }
        }
      } else // in_indexe
      if( zeile.find("transparency") != std::string::npos )
      {
        werte =
          unterscheideZiffernWorte( zeile, anfuehrungstriche, trennzeichen );

        // sort the read values
        for(unsigned int w = 0; w < werte.size(); ++w)
          if(werte[w].zahl.first)
            netze_[endnetz].undurchsicht = 1.0 - werte[w].zahl.second;
      } else // transparency
      if( zeile.find("emissiveColor") != std::string::npos )
      {
        werte =
          unterscheideZiffernWorte( zeile, anfuehrungstriche, trennzeichen );

        // sort the read values
        for(unsigned int w = 0; w < werte.size(); ++w)
          if(werte[w].zahl.first)
          {
            netze_[endnetz].schattierung = werte[w].zahl.second;
            break;
          }
      } // emissiveColor
    }

    if(index_min < 0)
      netze_[endnetz].clear();
    if(index_max > (int)netze_[endnetz].punkte.size())
      netze_[endnetz].clear();

    netz_n++;
  }
# if DEBUG_VRML_PARSER
  for(int i = 0; i < 3; ++i) {
    DBG_VRML_PARSER_S( "Position ["<<i<<"]: " <<min_pos[i] <<" - "<<max_pos[i]);
  }
  for(int i = 0; i < 3; ++i) {
    DBG_VRML_PARSER_S( "Farbe ["<<i<<"]: "<<min_farbe[i]<<" - "<< max_farbe[i]);
  }
# endif

  if(loc_alt)
  {
    DBG_PROG_S( "setting back from "<< setlocale(LC_NUMERIC,NULL) <<" to "<< loc_alt)
    setlocale(LC_NUMERIC,loc_alt);
    free( loc_alt );
  }

  DBG_PROG_ENDE
  icc_debug = debug_alt;
}


icc_examin_ns::ICCThreadList<ICCnetz>
extrahiereNetzAusVRML (std::string & vrml)
{
  ICCvrmlParser vrml_parser;
  vrml_parser.lade(vrml);
  return vrml_parser.zeigeNetze();
}


std::string netzNachVRML( icc_examin_ns::ICCThreadList<ICCnetz> & netze )
{
  std::string vrml, temp;

  size_t n = netze.size();
  doLocked_m( std::string loc_alt = setlocale(LC_NUMERIC, NULL);,NULL)
  doLocked_m( setlocale(LC_NUMERIC,"C");,NULL);

  for(size_t i = 0; i < n; ++i)
  {
    ICCnetz & netz = netze[i];
    char num[24];

    temp.append("    Transform {\n\
      translation 0 0 0\n\
      children [\n\
        Shape {\n\
          geometry IndexedFaceSet {\n\
            ccw FALSE\n\
            convex TRUE\n\
\n\
");

    // write
    if(netz.punkte.size())
    {
      int p_n = (int)netz.punkte.size();

      temp.append("            coord Coordinate {\n\
              point [\n\
");

      for(int j = 0; j < p_n; ++j)
      {
        temp.append( "               " );
        sprintf(num," %.03f", (netz.punkte[j].koord[1] -.5 ) * 255.0);
        temp.append( num );
        sprintf(num," %.03f", (netz.punkte[j].koord[2] -.5 ) * 255.0);
        temp.append( num );
        sprintf(num," %.03f", (netz.punkte[j].koord[0] -.5 ) * 100.0 );
        temp.append( num );
        temp.append( ",\n" );
      }
                                 
      temp.append("              ] # point\n\
            } #coord Coordinate\n\
\n");

      {
        temp.append("            coordIndex [\n\
");
        std::multimap<double,DreiecksIndexe>::const_iterator it;
        for( it = netz.indexe.begin(); it != netz.indexe.end(); ++it )
        {
          std::pair<double,DreiecksIndexe> index_p( *it );

          temp.append( "             " );
          for(int k = 0; k < 3; ++k)
          {
            sprintf(num," %d,", index_p.second.i[k] );
            temp.append( num );
          }
          temp.append( " -1\n" );
        }
        temp.append("            ] # coordIndex\n\
");
      }

      temp.append("\n\
         colorPerVertex TRUE\n\
            color Color {\n\
              color [\n\
");

      for(int j = 0; j < p_n; ++j)
      {
        temp.append( "                " );
        for(int k = 0; k < 3; ++k)
        {
          sprintf(num," %.03f", netz.punkte[j].farbe[k] );
          temp.append( num );
        }
        temp.append( ",\n" );
      }
                                 
      temp.append("              ] # color\n\
            } # Color \n\
");
    }

    

    temp.append("          } # IndexedFaceSet\n\
          appearance Appearance { \n\
            material Material {\n");
    if(0 < netz.undurchsicht && netz.undurchsicht < 1.0)
    {
      sprintf(num," %.03f\n", 1.0 - netz.undurchsicht );
      temp.append("              transparency");
      temp.append( num );
    }
    temp.append("              ambientIntensity 0.3\n\
              shininess 0.5\n");

    if(netz.grau && netz.schattierung > 0)
    {
      sprintf(num," %.03f", netz.schattierung );
      temp.append("              emissiveColor");
      temp.append( num ); temp.append( num ); temp.append( num );
      temp.append("\n");
    }

    temp.append("            }\n\
          } \n\
        } # Shape\n\
      ] # children\n\
    } # Transform\n\
\n\
");
  }

  if(temp.size())
  {
    vrml.append("#VRML V2.0 utf8\n\
\n\
# Created by ICC Examin\n\
Transform {\n\
  children [\n\
    NavigationInfo {\n\
      type \"EXAMINE\"\n\
    }\n\
\n\
");
    vrml.append(temp);
    vrml.append("  ] # children\n\
}");
  }

  if(loc_alt.size())
    doLocked_m( setlocale(LC_NUMERIC,loc_alt.c_str()) , NULL);

  return vrml;
}

