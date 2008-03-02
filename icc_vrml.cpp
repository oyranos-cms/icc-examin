/*
 * ICC Examin ist eine ICC Profil Betrachter
 * 
 * Copyright (C) 2004-2007  Kai-Uwe Behrmann 
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
 * visualise colour gamut in WWW-3D format vrml.
 * 
 */

// Date:      Mai 2004

#include "icc_utils.h"
#include "icc_vrml.h"
#include "icc_info.h"
#include "icc_helfer.h"

#include <stdio.h>
#include <iostream>
#include <fstream>
#include <sstream>
//#include <string.h>

extern "C" {
int iccCreateArgyllVrml( const char* prof_name, int intent, double * volume );
}

int
eraseFile (const char *file)
{
  FILE *fp;

  fp = fopen (file, "r");
  if (fp) {
    fclose (fp);
    remove (file);
    return 0;
  }

  DBG_PROG_S( "file " << file << " could not be erased" )
  return 1;
}

std::string
iccCreateVrml( const char* p, int size, int intent )
{
  DBG_PROG_START
  std::string vrml;

  if (!p || !size)
    return vrml;

  std::string profil_temp_name = tempFileName();
  std::stringstream s;
  DBG_PROG_V( profil_temp_name )
  std::string ptn = profil_temp_name; oyStrAdd( ptn, ".icc");

  // save
  {
    std::ofstream f;
    f.clear();
    f.open ( ptn.c_str(),  std::ios::out );
    if(f.good())
    {
      size_t sz = size;
      f.write ( p, sz );
    }
    f.close();
  }

  int ret;
#ifdef USE_ARGYLL
  double volume = -1.0;
  ret = iccCreateArgyllVrml( ptn.c_str(), intent, &volume );
  DBG_V(volume)

#else
  // generate vrml - argyll external variant
# if APPLE
  std::string argyll_bundle_pfad = icc_examin_ns::holeBundleResource("iccgamut",
                                                                     "");
  if(argyll_bundle_pfad.size()) {
    icc_parser::suchenErsetzen(argyll_bundle_pfad," ","\\ ",0);
    s << argyll_bundle_pfad;
  } else
# endif /* __APPLE__ */
  {
    DBG_PROG_V( PATH_SHELLSTRING )
    s.str("");
    s << PATH_SHELLSTRING ;
    s << "iccgamut";
  }
  s << " -n ";
  if(intent == 0)
    s << "-i p ";
  else if(intent == 1)
    s << "-i r ";
  else if(intent == 2)
    s << "-i s ";
  else if(intent == 3)
    s << "-i a ";
  s << "-w " << ptn;
  DBG_PROG_V( s.str() )
  std::string icc_sys_c = s.str();
  ret = system (icc_sys_c.c_str()); DBG_PROG
#endif /* USE_ARGYLL */

  ptn = profil_temp_name; oyStrAdd( ptn, ".icc");
# ifndef DEBUG_
  eraseFile (ptn.c_str());
# endif
  ptn = profil_temp_name; oyStrAdd( ptn, ".gam");
  eraseFile (ptn.c_str());

  // open file
  {
    ptn = profil_temp_name; oyStrAdd( ptn, ".wrl" );

    DBG_PROG

    size_t size;
    char *data = 0;
#   if HAVE_EXCEPTION
    try {
#   endif
      data = ladeDatei (ptn.c_str(), &size);
#   if HAVE_EXCEPTION
    }
      catch (Ausnahme & a) {  // catches all from exception
        DBG_NUM_V (_("exception occured: ") << a.what());
        a.report();
      }
      catch (std::exception & e) { // catches all from exception
        DBG_NUM_V (_("Std-exception occured: ") << e.what());
      }
      catch (...) {       // catches all from exception
        DBG_NUM_V (_("Huch, unknown exception"));
      }
#   endif

    if(data)
    {
      vrml = data;
      free(data);
    }
    eraseFile (ptn.c_str());
  }

  DBG_PROG_ENDE
  return vrml;
}

#if 0
void dumpVrmlHeader (char *vrml);

int 
create_vrml              ( const char *profilA, char *profilB, char *vrml)
{
  char *system_befehl = NULL;
  int r;

  if (!vrml || (!profilA && !profilB))
  return (0);

  eraseFile (vrml);
  dumpVrmlHeader (vrml);
  std::string profil_temp_name = tempFileName();
  std::string icc = profil_temp_name; icc += ".icc";
  std::string wrl = profil_temp_name; wrl += ".wrl";
  std::string gam = profil_temp_name; gam += ".gam";

  // gamut A
  if (profilA) {
  if (!eraseFile (icc.c_str())) remove (icc.c_str());
  char * system_befehl = (char*) malloc( strlen(profilA) + icc.size()*3 + 64 );
  sprintf (system_befehl, "ln -s \"%s\" \"%s\"", profilA, icc.c_str());
  r = system (system_befehl);
  sprintf (system_befehl, "iccgamut -n -w -d 6.0 \"%s\"", icc.c_str());
  r = system (system_befehl);
  eraseFile (wrl.c_str());
  eraseFile (icc.c_str());
  sprintf (system_befehl, "viewgam -n -c n \"%s\" \"%s\"", gam.c_str(), wrl.c_str());
  r = system (system_befehl);
  sprintf (system_befehl ,"cat \"%s\" >> \"%s\"", wrl.c_str(),  vrml);
  r = system (system_befehl);
  eraseFile (wrl.c_str());
  }

  profil_temp_name = tempFileName();
  std::string icc2 = profil_temp_name; icc2 += ".icc";
  std::string wrl2 = profil_temp_name; wrl2 += ".wrl";
  std::string gam2 = profil_temp_name; gam2 += ".gam";
  // gamut B transparent
  if (profilB) {
  if (!eraseFile (icc2.c_str())) remove (icc2.c_str());
  sprintf (system_befehl, "ln -s \"%s\" \"%s\"", profilB, icc2.c_str());
  r = system (system_befehl);
  sprintf (system_befehl, "iccgamut -n -w -d 6.0 \"%s\"", icc2.c_str());
  r = system (system_befehl);
  eraseFile (wrl2.c_str());
  eraseFile (icc2.c_str());
  sprintf (system_befehl, "viewgam -n -t 0.5 -c w \"%s\" \"%s\"", gam2.c_str(), wrl.c_str() );
  r = system (system_befehl);
  sprintf (system_befehl ,"cat \"%s\" >> \"%s\"", wrl.c_str(), vrml);
  r = system (system_befehl);
  eraseFile (wrl.c_str());
  }

  // differences
  if (profilA && profilB) {
  sprintf (system_befehl, "smthtest \"%s\" \"%s\" \"%s\"", gam.c_str(), gam2.c_str(), wrl.c_str());
  r = system (system_befehl);
  sprintf (system_befehl ,"cat \"%s\" >> \"%s\"", wrl.c_str(), vrml);
  r = system (system_befehl);
  eraseFile (wrl.c_str());
  eraseFile (gam.c_str());
  eraseFile (gam2.c_str());
  }

  return 0;  
}

void
dumpVrmlHeader (char *vrml)
{
  FILE *fp;
  char vrml_text[] = "#VRML V2.0 utf8 \nTransform { children [ DirectionalLight { color 0.500000 0.500000 0.500000 } Viewpoint { position 0 0 255 } ] } Transform { children [ NavigationInfo { avatarSize [ 0.250000 1.600000 0.750000 ] type [ \"EXAMINE\" ] } Transform { children [ Shape { appearance 	  Appearance { material 	    Material { diffuseColor 0.495000 0 0 } } geometry 	  Box { size 100 2 2 } } ] scale 1 0.500000 0.500000 translation 50 0 -50 } Transform { children [ Shape { appearance 	  Appearance { material 	    Material { diffuseColor 0 0 0.504998 } } geometry 	  Box { size 2 100 2 } } ] scale 0.500000 1 0.500000 translation 0 -50 -50 } Transform { children [ Shape { appearance 	  Appearance { material 	    Material { diffuseColor 0 0.504998 0 } } geometry 	  Box { size 100 2 2 } } ] scale 1 0.500000 0.500000 translation -50 0 -50 } Transform { children [ Shape { appearance 	  Appearance { material 	    Material { diffuseColor 1 1 0 } } geometry 	  Box { size 2 100 2 } } ] scale 0.500000 1 0.500000 translation 0 50 -50 } Shape { appearance       Appearance { material 	Material { diffuseColor 0.699998 0.699998 0.699998 } } geometry       Box { size 1 1 100 } } ] } Group { children [ Transform { children [ Billboard { axisOfRotation 0 0 0 children [ Shape { appearance 	      Appearance { material 		Material { } } geometry 	      Text { string [ \"*L=100\" ] } } ] } ] scale 5 5 5 translation 0 0 52 } Transform { children [ Billboard { axisOfRotation 0 0 0 children [ Shape { appearance 	      Appearance { material 		Material { } } geometry 	      Text { string [ \"*L= 0\" ] } } ] bboxSize 1 1 1 } ] scale 5 5 5 translation 0 0 -57 } ] } Viewpoint { fieldOfView 0.790000 orientation 0.999990 1.788147e-3 -3.940986e-3 1.515311 position 0 -340 20 } Background { groundColor [ 0.500000 0.500000 0.500000 ] skyColor [ 0.500000 0.500000 0.500000 ] } ";

  int text_len = strlen (vrml_text);
  fp = fopen (vrml, "w+");
  if (fp) {
    for (int i = 0 ; i < text_len ; i++ )
      fputc ( vrml_text[i] , fp);

    fclose (fp);
  }
}

#endif

