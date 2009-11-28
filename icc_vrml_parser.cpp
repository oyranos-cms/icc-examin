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
#include "icc_examin_version.h"

#include <sstream>
#include <string>

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
  DBG_PROG_V( original_.size() )

  std::string::size_type netz_pos=0;


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
  {
    DBG_NUM_V( loc_alt )
  } else {
    DBG_NUM_S( "did not find LANG variable" )
  }
  setlocale(LC_NUMERIC,"C");

  
  // search for mesh in vrml file
  while( (netz_pos = original_.find( "IndexedFaceSet", ++netz_pos )) !=
        std::string::npos )
    netz_n++;

  int * netz_pos_n = new int[netz_n+1],
      i = 0;

  netz_pos = 0;
  while( (netz_pos = original_.find( "IndexedFaceSet", ++netz_pos )) !=
        std::string::npos )
    netz_pos_n[i++] = netz_pos;

  netz_pos_n[i] = 0;

#pragma omp parallel for private(netz_pos)
  for(i = 0; i < netz_n; ++i)
  {
    ICClist<std::string> zeilen;      // editing row
    ICClist<ZifferWort> werte;        // intermediate return value
    char trennzeichen[12];                // to be used separating signs
    trennzeichen[0] = ',';
    sprintf(&trennzeichen[1], "%s", leer_zeichen);
    const bool anfuehrungstriche = false; // set no quotation marks
    unsigned int dimensionen;             // number of belonging values
    int achse;                            // selected value from dimensionen
    std::string::size_type pos=0;
    std::string::size_type netz_ende;

    netz_pos = netz_pos_n[i];
    ++ netz_pos;
 
    // get the area of the mesh
    if( (netz_ende = original_.find( "IndexedFaceSet", netz_pos )) ==
        std::string::npos )
      netz_ende = original_.size();
    if( netz_pos == std::string::npos )
      continue;

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
          flaeche_klammer *= -1;
        if( flaeche_klammer * -1 - 1 > geschweifte_klammer )
          flaeche_klammer = false;
        if( flaeche_klammer > 0 ) {
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

            index_p.first = (double)z;

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
      if( flaeche_klammer < 0 &&
          zeile.find("transparency") != std::string::npos )
      {
        werte =
          unterscheideZiffernWorte( zeile, anfuehrungstriche, trennzeichen );

        // sort the read values
        for(unsigned int w = 0; w < werte.size(); ++w)
          if(werte[w].zahl.first)
            netze_[endnetz].undurchsicht = 1.0 - werte[w].zahl.second;
      } else // transparency
      if( flaeche_klammer < 0 &&
          zeile.find("emissiveColor") != std::string::npos )
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

#pragma omp critical
    if(index_min < 0)
      netze_[endnetz].clear();
    if(index_max > (int)netze_[endnetz].punkte.size())
      netze_[endnetz].clear();
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


/** @func    writeVRMLbody
 *  @brief   write the basics of a VRML
 *
 *  Wrap the content in useful stuff like version info and a viewpoint.
 *
 *  @version ICC Examin: 0.45
 *  @date    2008/02/20
 *  @since   2008/02/20 (ICC Examin: 0.45)
 */
std::string writeVRMLbody( std::string content )
{
  std::string vrml;

    vrml.append("\
#VRML V2.0 utf8\n\
\n\
WorldInfo {\n\
  title \"ICC Examin CIE*Lab view\"\n\
  info [ \"Generated by ICC Examin: ");
    vrml.append(ICC_EXAMIN_V);
    vrml.append("\",\n\
         \"http://www.behrmann.name\" ]\n\
}\n\
\n\
Transform {\n\
  children [\n\
    NavigationInfo {\n\
      type \"EXAMINE\"\n\
    }\n\
\n\
    Viewpoint {\n\
      position 0 0 1000 # top\n\
      fieldOfView 0.20\n\
      description \"Top\"\n\
    }\n\
\n");
    vrml.append( content );
    vrml.append("\
  ] # children\n\
}");

  return vrml;
}

std::string netzNachVRML( icc_examin_ns::ICCThreadList<ICCnetz> & netze )
{
  std::string vrml, temp;

  size_t n = netze.size();
  doLocked_m( std::string loc_alt = setlocale(LC_NUMERIC, NULL);,NULL)
  doLocked_m( setlocale(LC_NUMERIC,"C");,NULL);

#pragma omp parallel for
  for(int i = 0; i < (int)n; ++i)
  {
    ICCnetz & netz = netze[i];
    char num[24];

    if(!netz.aktiv || netz.undurchsicht <= 0.)
      continue;

    if(!netz.punkte.size())
      continue;

    const char * tn = "";

    if(netze[i].name.size())
      tn = netze[i].name.c_str();

    char * txt = (char*) malloc (256);
    sprintf( txt, "gamut pos: %d\",\n\
                 \"generated from file: %s", i, tn );

    temp.append("\
    Transform {\n\
      translation 0 0 0\n\
      children [\n\
        WorldInfo {\n\
          title \"");
    temp.append(tn);
    temp.append("\"\n\
          info [ \"");
    temp.append(txt);
    temp.append("\" ]\n\
        }\n\
        Shape {\n\
          geometry IndexedFaceSet {\n\
            ccw FALSE\n\
            convex TRUE\n\
\n");
    if(txt) free(txt);

    // write
    if(netz.punkte.size())
    {
      int p_n = (int)netz.punkte.size();

      temp.append("\
            coord Coordinate {\n\
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
        temp.append( ", " );
        sprintf(num, "# %d", j);
        temp.append( num );
        temp.append( "\n" );
      }
                                 
      temp.append("\
              ] # point\n\
            } #coord Coordinate\n\n");

      {
        temp.append("\
            coordIndex [\n\
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
        temp.append("\
            ] # coordIndex\n");
      }

      temp.append("\n\
          colorPerVertex TRUE\n\
            color Color {\n\
              color [\n");

      for(int j = 0; j < p_n; ++j)
      {
        temp.append( "\
               " );
        for(int k = 0; k < 3; ++k)
        {
          sprintf(num," %.03f", netz.punkte[j].farbe[k] );
          temp.append( num );
        }
        temp.append( ", " );
        sprintf(num, "# %d", j);
        temp.append( num );
        temp.append( "\n" );
      }
                                 
      temp.append("\
              ] # color\n\
            } # Color\n");
    }
    temp.append("\
          } # FaceSet\n\
          appearance Appearance { \n\
            material Material {\n");
    if(0 < netz.undurchsicht && netz.undurchsicht < 1.0)
    {
      sprintf(num," %.03f\n", 1.0 - netz.undurchsicht );
      temp.append("\
              transparency");
      temp.append( num );
    }
    temp.append("\
              ambientIntensity 0.3\n\
              shininess 0.5\n");

    if(netz.grau && netz.schattierung > 0)
    {
      sprintf(num," %.03f", netz.schattierung );
      temp.append("\
              emissiveColor");
      temp.append( num ); temp.append( num ); temp.append( num );
      temp.append("\n");
    }

    temp.append("            }\n\
          } \n\
        } # Shape\n\
      ] # children\n\
    } # Transform\n\
\n");
  }

  if(temp.size())
    vrml = temp;

  if(loc_alt.size())
    doLocked_m( setlocale(LC_NUMERIC,loc_alt.c_str()) , NULL);

  return vrml;
}


std::string    writeTextArrows       ( double              text_colour,
                                       double              arrow_colour )
{
  std::string temp;
  char t[16];

  // text
  if(text_colour > 0 || arrow_colour > 0)
  {
    sprintf(t, " %.02f %.02f %.02f", 
               text_colour, text_colour, text_colour );
    temp.append( "\n\
    # CIE *L text\n\
    Transform {\n\
      translation 0.0 0.0 55.0\n\
      children [\n\
        Billboard {\n\
          axisOfRotation 0 0 0\n\
          children [\n\
            Shape {\n\
              geometry Text {\n\
                string [\"CIE *L\"]\n\
                fontStyle FontStyle {\n\
                  size 5.0\n\
                } # fontStyle\n\
              } # Text\n\
              appearance Appearance {\n\
                material Material {\n\
                  diffuseColor ");
        temp.append( t );
        temp.append("\n\
                } # material\n\
              } # appearance\n\
            } # Shape \n\
          ] # children\n\
        } # Billbord\n\
      ] # children\n\
    } # Transform\n\
\n");
    temp.append( "\n\
    # CIE *a text\n\
    Transform {\n\
      translation 127.0 0.0 -50.0\n\
      children [\n\
        Billboard {\n\
          axisOfRotation 0 0 0\n\
          children [\n\
            Shape {\n\
              geometry Text {\n\
                string [\"CIE *a\"]\n\
                fontStyle FontStyle {\n\
                  size 5.0\n\
                } # fontStyle\n\
              } # Text\n\
              appearance Appearance {\n\
                material Material {\n\
                  diffuseColor ");
        temp.append( t );
        temp.append("\n\
                } # material\n\
              } # appearance\n\
            } # Shape \n\
          ] # children\n\
        } # Billbord\n\
      ] # children\n\
    } # Transform\n\
\n");
    temp.append( "\n\
    # CIE *b text\n\
    Transform {\n\
      translation 0.0 127.0 -50.0\n\
      children [\n\
        Billboard {\n\
          axisOfRotation 0 0 0\n\
          children [\n\
            Shape {\n\
              geometry Text {\n\
                string [\"CIE *b\"]\n\
                fontStyle FontStyle {\n\
                  size 5.0\n\
                } # fontStyle\n\
              } # Text\n\
              appearance Appearance {\n\
                material Material {\n\
                  diffuseColor ");
        temp.append( t );
        temp.append("\n\
                } # material\n\
              } # appearance\n\
            } # Shape \n\
          ] # children\n\
        } # Billbord\n\
      ] # children\n\
    } # Transform\n\
\n");

    sprintf( t, " %.02f %.02f %.02f",
                arrow_colour, arrow_colour, arrow_colour );
    // CIE *L arrow
    temp.append( "\n\
    # CIE *L arrow\n\
    Transform {\n\
      translation 0.0 0.0 52.5\n\
      rotation 1 0 0 1.5707\n\
      children [\n\
        Shape {\n\
          geometry Cone {\n\
            bottomRadius 2        # (0,)\n\
            height       5        # (0,)\n\
          } # Cone\n\
          appearance Appearance {\n\
            material Material {\n\
              diffuseColor ");
        temp.append( t );
        temp.append("\n\
            } # material\n\
          } # appearance\n\
        } # Shape \n\
      ] # children\n\
    } # Transform\n");
#ifdef USE_CYLINDER
    temp.append( "Transform {\n\
  translation 0.0 0.0 -2.5\n\
  rotation 1 0 0 1.5707\n\
  children [\n\
    Shape {\n\
      geometry Cylinder {\n\
        height  95  # (0,)\n\
        radius  .5  # (0,)\n\
      } # Cylinder\n\
      appearance Appearance {\n\
        material Material {\n\
          diffuseColor ");
    temp.append( t );
    temp.append("\n\
        } # material\n\
      } # appearance\n\
    } # Shape \n\
  ] # children\n\
} # Transform\n");
#endif
    // CIE *a arrow
    temp.append( "\n\
    # CIE *a arrow\n\
    Transform {\n\
      rotation 0 0 1 -1.5707\n\
      translation 124.5 0.0 -50.0\n\
      children [\n\
        Shape {\n\
          geometry Cone {\n\
            bottomRadius 2        # (0,)\n\
            height       5        # (0,)\n\
          } # Cone\n\
          appearance Appearance {\n\
            material Material {\n\
              diffuseColor 0.9 0.2 0.5\n\
            } # material\n\
          } # appearance\n\
        } # Shape \n\
      ] # children\n\
    } # Transform\n");
#ifdef USE_CYLINDER
    temp.append( "Transform {\n\
  rotation 0 0 1 1.5707\n\
  translation 0.0 0.0 -50.0\n\
  children [\n\
    Shape {\n\
      geometry Cylinder {\n\
        height  250  # (0,)\n\
        radius  0.5 # (0,)\n\
      } # Cylinder\n\
      appearance Appearance {\n\
        material Material {\n\
          diffuseColor ");
    temp.append( t );
    temp.append("\n\
        } # material\n\
      } # appearance\n\
    } # Shape \n\
  ] # children\n\
} # Transform\n");
#endif
    // CIE *b arrow
    temp.append( "\n\
    # CIE *b arrow\n\
    Transform {\n\
      translation 0.0 124.5 -50.0\n\
      children [\n\
        Shape {\n\
          geometry Cone {\n\
            bottomRadius 2        # (0,)\n\
            height       5        # (0,)\n\
          } # Cone\n\
          appearance Appearance {\n\
            material Material {\n\
              diffuseColor 0.9 0.9 0.2\n\
            } # material\n\
          } # appearance\n\
        } # Shape \n\
      ] # children\n\
    } # Transform\n");
#ifdef USE_CYLINDER
    temp.append( "Transform {\n\
  translation 0.0 -2.5 -50.0\n\
  children [\n\
    Shape {\n\
      geometry Cylinder {\n\
        height  250  # (0,)\n\
        radius  0.5 # (0,)\n\
      } # Cylinder\n\
      appearance Appearance {\n\
        material Material {\n\
          diffuseColor ");
    temp.append( t );
    temp.append("\n\
        } # material\n\
      } # appearance\n\
    } # Shape \n\
  ] # children\n\
} # Transform\n");
#else
    temp.append( "\n\
    Shape {\n\
      geometry IndexedLineSet {\n\
        coord Coordinate {\n\
          point [\n\
            0.0  0.0   -50.0,\n\
            0.0  0.0    50.0,\n\
             124.5 0.0 -50.0,\n\
            -124.5 0.0 -50.0,\n\
            0.0  124.5 -50.0,\n\
            0.0 -124.5 -50.0,\n\
          ] # point\n\
        } # coord\n\
        coordIndex [\n\
            0, 1, -1\n\
            2, 3, -1\n\
            4, 5, -1\n\
        ] # coordIndex\n\
      } # geometry\n\
      appearance Appearance {\n\
        material Material {\n\
          emissiveColor ");
        temp.append( t );
        temp.append("\n\
        } # material\n\
      } # appearance\n\
    } # Shape \n\
");
#endif
  }

  return temp;
}

std::string    vrmlScene          ( icc_examin_ns::ICCThreadList<ICCnetz> netze,
                                       oyStructList_s    * colour_lists,
                                       double              text_colour,
                                       double              arrow_colour,
                                       double              background_colour,
                                       double              radius )
{
  std::string text, temp;
  char t[16];
  const char * names[3] = {0,0,0};
  int show_points_as_measurements = 0, show_points_as_pairs = 0,
      spectralline = 0;
  float L = 340.0, a = 0.0, b = 0.0;
  char * txt = (char*) malloc (256);

  if(!(oyStructList_Count( colour_lists ) || netze.size()))
    return temp;

  if(colour_lists)
  {
    names[0] = oyObject_GetName(colour_lists->oy_, oyNAME_NAME);
    names[1] = oyObject_GetName(colour_lists->oy_, oyNAME_NICK);
    names[2] = oyObject_GetName(colour_lists->oy_, oyNAME_DESCRIPTION);
    if(names[2])
    {
      if(strstr(names[2],"show_points_as_pairs"))
        show_points_as_pairs = 1;
      if(strstr(names[2],"show_points_as_measurements"))
        show_points_as_measurements = 1;
      if(strstr(names[2],"spectralline"))
        spectralline = 1;
      if(strstr(names[2],"viewpoint:"))
        sscanf( strstr(names[2],"viewpoint:")+strlen("viewpoint:"),
                "%f %f %f", &L,&a,&b );
    }
  }

  doLocked_m( std::string loc_alt = setlocale(LC_NUMERIC, NULL);,NULL)
  doLocked_m( setlocale(LC_NUMERIC,"C");,NULL);


  // text
  temp.append( writeTextArrows( text_colour, arrow_colour ) );


  // set the background
  sprintf(t, " %.02f", background_colour );
  temp.append("\n\
    Background {\n\
      skyColor     [");
  temp.append( t );
  temp.append( t );
  temp.append( t );
  temp.append("]  # [0,1]\n\
    }\n\
\n");

  /* colour conversion context */
  double lab[3], rgba[4];
  oyOptions_s * opts = 0;
  oyOptions_SetFromText( &opts, "////rendering_intent", "0", OY_CREATE_NEW );

  // named colours
  if (oyStructList_Count( colour_lists ))
  {
    int c_n = oyStructList_Count( colour_lists ),
        j, i;
    const double * XYZ = 0;

    for(j = 0; j < c_n; ++j)
    {
      ICClist<std::string> point, shadow, colour;

      oyNamedColours_s * colours = 
           (oyNamedColours_s*) oyStructList_GetRefType( colour_lists, j,
                                                oyOBJECT_NAMED_COLOURS_S );

      int n = oyNamedColours_Count( colours );
      int aktiv = 1;
      int grau = 0;
      oyNamedColour_s * c = NULL;
      double schattierung = 1;
      int has_mesh = 0;
      std::string name;

      rgba[3] = 1;

      if(netze.size() > (unsigned)j)
      {
        aktiv = netze[j].aktiv;
        grau = netze[j].grau;
        rgba[3] = netze[j].undurchsicht;
        schattierung = netze[j].schattierung;
        if(netze[j].punkte.size())
          has_mesh = 1;
        name = netze[j].name;
      }

      if(!aktiv)
        continue;


      for (i = 0; i < n; ++i)
      {
          c = oyNamedColours_Get( colours, i );
          XYZ = oyNamedColour_GetXYZConst( c );

          oyXYZ2Lab( XYZ, lab );
          sprintf( txt, "                %.03f %.03f %.03f,\n",
                        lab[1], lab[2], lab[0]-50.);
          point.push_back( txt );
          sprintf( txt, "%.03f %.03f %.03f,\n",
                        lab[1], lab[2], -50.);
          shadow.push_back( txt );

          if(netze[j].grau)
            rgba[0]= rgba[1]= rgba[2] = schattierung;
          else
            oyNamedColour_GetColourStd( c, oyASSUMED_WEB, rgba, oyDOUBLE, 0,opts );

          sprintf( txt, "                %.03f %.03f %.03f,\n",
                        rgba[0], rgba[1], rgba[2]);
          colour.push_back( txt );

          oyNamedColour_Release( &c );
      }

      for(i = 0; i < n; ++i)
      {
        const char * tn = "", *tp = "";

        if(show_points_as_pairs && i >= n/2)
          break;

        if(name.size())
          tn = name.c_str();
        c = oyNamedColours_Get( colours, i );
        tp = oyNamedColour_GetName( c, oyNAME_NICK, 1 );
        if(!tp)
          tp = "";

        sprintf( txt, "\
    # point pos:%d:%d  file: %s  name: %s\n",
                      j, i+1, tn, tp );
        temp.append( txt );
        oyNamedColour_Release( &c );

        temp.append("\
    Transform { translation ");
        sprintf( txt, "%s", point[i].c_str() );
        temp.append( txt );
        temp.append("\
      children [\n");
        if(tp && strlen(tp))
        {
          temp.append("\
        WorldInfo {\n\
          title \"");
          temp.append(tp);
          temp.append("\"\n\
        }\n");
        }
        temp.append("\
        Shape {\n\
          geometry Sphere { radius ");
        sprintf( txt, "%.03f", radius );
        temp.append( txt );
        temp.append(" }\n\
          appearance Appearance {\n\
            material Material {\n\
              diffuseColor 0.0 0.0 0.0\n\
              emissiveColor ");
        sprintf( txt, "%s", colour[i].c_str() );
        temp.append( txt );
        if(!has_mesh && rgba[3] < 1.0)
        {
          sprintf( txt, "\
              transparency %.03f\n", 1.0 - rgba[3] );
          temp.append( txt );
        }
        temp.append("\
            } # material\n\
          } # appearance\n\
        } # Shape\n\
      ] # children\n\
    } # transform\n\
");
        temp.append("\
    Transform { translation ");
        sprintf( txt, "%s", shadow[i].c_str() );
        temp.append( txt );
        temp.append("\
      children Shape {\n\
        geometry Sphere { radius ");
        sprintf( txt, "%.03f", radius/3.0 );
        temp.append( txt );
        temp.append("\
        }\n\
        appearance Appearance {\n\
          material Material {\n\
            diffuseColor 0.05 0.05 0.05\n\
          } # material\n\
        } # appearance\n\
      } # children Shape\n\
    } # transform\n\
\n");
      }

      n = 0;
      if(point.size() == colour.size() && show_points_as_pairs)
        n = point.size();

      if(n)
      {
        const char * tn = "";
        if(name.size())
          tn = name.c_str();
        sprintf( txt, "lines pos: %d\",\n\
                 \"generated from file: %s", j, tn );

        temp.append("\
    Group {\n\
      children [\n\
        WorldInfo {\n\
          title \"");
        temp.append(tn);
        temp.append("\"\n\
          info [ \"");
        temp.append(txt);
        temp.append("\" ]\n\
        }");

        temp.append( "\n\
        Shape {\n\
          geometry IndexedLineSet {\n\
            coord Coordinate {\n\
              point [\n\
");
        for(i = 0; i < n; ++i)
          temp.append( point[i] );
        temp.append("\
              ] # point\n\
            } # coord\n\
\n\
            coordIndex [\n\
");
        for(i = 0; i < n/2; ++i)
        {
          sprintf(txt, "\
              %d, %d, -1,\n", i, n/2+i);
          temp.append( txt );
        }

        temp.append("\
            ] # coordIndex\n\
\n\
            colorPerVertex TRUE\n\
            color Color {\n\
              color [ \n");
        for(i = 0; i < n; ++i)
          temp.append( colour[i] );
        temp.append("\
              ] # color\n\
            } # color\n\
          } # geometry\n\
        } # Shape\n\
      ] # children\n\
    } # Group\n\
\n");

      }
    }

  }

  oyProfile_s * prof = oyProfile_FromStd( oyEDITING_XYZ, NULL );
  oyNamedColour_s * c = oyNamedColour_Create( NULL, NULL,0, prof, 0 );


  // net preparation and *ab gamut bound lines
  for(unsigned int i = 0; i < netze.size(); ++i)
  {
    int p_n = netze[i].punkte.size();
    if(netze[i].aktiv)
    {
      for(int j = 0; j < p_n; ++j)
      {

        // collect gamut colours
        if(netze[i].grau)
        {
          for(int k = 0; k < 3 ; ++k)
          netze[i].punkte[j].farbe[k] = netze[i].schattierung;

        } else {

          lab[0] = netze[i].punkte[j].koord[0]*100.;
          lab[1] = (netze[i].punkte[j].koord[1]-.5)*255.;
          lab[2] = (netze[i].punkte[j].koord[2]-.5)*255.;

          oyNamedColour_SetColourStd( c, oyEDITING_LAB, lab, oyDOUBLE, 0, opts);
          oyNamedColour_GetColourStd( c, oyASSUMED_WEB, rgba, oyDOUBLE, 0,opts);
          for(int k = 0; k < 3 ; ++k)
            netze[i].punkte[j].farbe[k] = rgba[k];
        }

      }

      // write *ab bounding lines
      if(netze[i].umriss.size())
      {
        const char * tn = "";
        temp.append( "\
    Group {\n\
      children [\n");

        if(netze[i].name.size())
          tn = netze[i].name.c_str();
        sprintf( txt, "gamut ab bounds pos: %d\",\n\
                 \"generated from file: %s", i, tn );

        temp.append("\
        WorldInfo {\n\
          title \"");
        temp.append(tn);
        temp.append("\"\n\
          info [ \"");
        temp.append(txt);
        temp.append("\" ]\n\
        }\n");

        temp.append("\
        Shape {\
          geometry IndexedLineSet {\n\
            coord Coordinate {\n\
              point [\n\
");
        int u_n = netze[i].umriss.size();
        for(int j = 0; j < u_n; ++j)
        {
          sprintf(txt, "\
                %.02f %.02f %.02f,\n",
          (netze[i].umriss[j].koord[1] - .5) * 255.,
          (netze[i].umriss[j].koord[2] - .5) * 255.,
                           -50.//dreiecks_netze[i].umriss[j].koord[0]*255.-- b_darstellungs_breite/2.,
                           );
          temp.append( txt );
        }
        temp.append("\
              ] # point\n\
            } # coord\n\
\n\
            coordIndex [\n");
        for(int j = 0; j < u_n-1; ++j)
        {
          sprintf(txt, "\
              %d, %d, -1,\n", j, j+1);
          temp.append( txt );
        }

        temp.append("\
            ] # coordIndex\n\
            colorPerVertex TRUE\n\
            color Color {\n\
              color [ \n");
        for(int j = 0; j < u_n; ++j)
        {
          if(netze[i].grau)
            sprintf(txt, "\
                %.03f %.03f %.03f,\n",
                         netze[i].schattierung,
                         netze[i].schattierung,
                         netze[i].schattierung);
          else
            sprintf(txt, "\
                %.03f %.03f %.03f,\n",
                         netze[i].umriss[j].farbe[0],
                         netze[i].umriss[j].farbe[1],
                         netze[i].umriss[j].farbe[2]);
          temp.append( txt );
        }
        temp.append("\
              ] # color\n\
            } # color\n\
          } # geometry\n\
        } # Shape\n\
      ] # children\n\
    } # Group\n\
\n");

      }
    }
  }
  if(txt) free(txt);
  oyNamedColour_Release( &c );
  oyOptions_Release( &opts );

  if(loc_alt.size())
    doLocked_m( setlocale(LC_NUMERIC,loc_alt.c_str()) , NULL);

  // gamut
  temp.append( netzNachVRML( netze ) );

  text = writeVRMLbody( temp );

  return text;
}


