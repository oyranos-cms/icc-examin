/*
 * ICC Examin ist eine ICC Profil Betrachter
 * 
 * Copyright (C) 2004-2005  Kai-Uwe Behrmann 
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
 * Farbumfang betrachten im WWW-3D Format vrml.
 * 
 */

// Date:      Mai 2004

#include "icc_vrml.h"
#include "icc_utils.h"
#include "icc_info.h"
#include "icc_helfer.h"

#include <stdio.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string.h>


void dump_vrml_header (char *vrml);

int
erase_file (const char *file)
{
  FILE *fp;

  fp = fopen (file, "r");
  if (fp) {
    fclose (fp);
    remove (file);
    return 0;
  }

  DBG_PROG_S( _("Datei ") << file << _(" konnte nicht geloescht werden") )
  return 1;
}


std::string
icc_create_vrml( const char* p, int size, int intent )
{
  DBG_PROG_START
  std::string vrml;

  if (!p || !size)
    return vrml;

  std::stringstream profil_temp_name, s;
  if(getenv("TMPDIR"))
    profil_temp_name << getenv("TMPDIR") << "/oyranos_" << time(0) ;
  else
    profil_temp_name << "/tmp/oyranos_" << time(0) ;
  DBG_PROG_V( profil_temp_name.str() )
  std::string ptn = profil_temp_name.str(); ptn.append(".icc");

  // Speichern
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
  // vrml produzieren - argyll Variante
  int ret;
# if APPLE
  std::string argyll_bundle_pfad = icc_examin_ns::holeBundleResource("iccgamut",
                                                                     "");
  if(argyll_bundle_pfad.size()) {
    icc_parser::suchenErsetzen(argyll_bundle_pfad," ","\\ ",0);
    s << argyll_bundle_pfad;
  } else
# endif
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
  ptn = profil_temp_name.str(); ptn.append(".icc");
  erase_file (ptn.c_str());
  ptn = profil_temp_name.str(); ptn.append(".gam");
  erase_file (ptn.c_str());

  // Datei Oeffnen
  {
    ptn = profil_temp_name.str(); ptn.append(".wrl");

    DBG_PROG

    size_t size;
    char *data = 0;
    try {
      data = ladeDatei (ptn.c_str(), &size);
    }
#   if HAVE_EXCEPTION
      catch (Ausnahme & a) {  // faengt alles von Ausnahme Abstammende
        DBG_NUM_V (_("Ausnahme aufgetreten: ") << a.what());
        a.report();
      }
      catch (std::exception & e) { // faengt alles von exception Abstammende
        DBG_NUM_V (_("Std-Ausnahme aufgetreten: ") << e.what());
      }
      catch (...) {       // faengt alles Uebriggebliebene
        DBG_NUM_V (_("Huch, unbekannte Ausnahme"));
      }
#   endif

    if(data)
    {
      vrml = data;
      free(data);
    }
    erase_file (ptn.c_str());
  }

  DBG_PROG_ENDE
  return vrml;
}

int 
create_vrml              ( const char *profilA, char *profilB, char *vrml)
{
  char system_befehl[1024];

  if (!vrml || (!profilA && !profilB))
  return (0);

  erase_file (vrml);
  dump_vrml_header (vrml);

  // gamut A
  if (profilA) {
  if (!erase_file ("/tmp/tmpA.icc")) remove ("/tmp/tmpA.icc");
  sprintf (system_befehl, "ln -s \"%s\" /tmp/tmpA.icc", profilA);
  system (system_befehl);
  system ("iccgamut -n -w -d 6.0 /tmp/tmpA.icc");
  erase_file ("/tmp/tmpA.wrl");
  erase_file ("/tmp/tmpA.icc");
  system ("viewgam -n -c n /tmp/tmpA.gam /tmp/tmp.wrl");
  sprintf (system_befehl ,"cat /tmp/tmp.wrl >> \"%s\"", vrml);
  system (system_befehl);
  erase_file ("/tmp/tmp.wrl");
  }

  // gamut B transparent
  if (profilB) {
  if (!erase_file ("/tmp/tmpB.icc")) remove ("/tmp/tmpB.icc");
  sprintf (system_befehl, "ln -s \"%s\" /tmp/tmpB.icc", profilB);
  system (system_befehl);
  system ("iccgamut -n -w -d 6.0 /tmp/tmpB.icc");
  erase_file ("/tmp/tmpB.wrl");
  erase_file ("/tmp/tmpB.icc");
  system ("viewgam -n -t 0.5 -c w /tmp/tmpB.gam /tmp/tmp.wrl");
  sprintf (system_befehl ,"cat /tmp/tmp.wrl >> \"%s\"", vrml);
  system (system_befehl);
  erase_file ("/tmp/tmp.wrl");
  }

  // Unterschiede
  if (profilA && profilB) {
  system ("smthtest /tmp/tmpA.gam /tmp/tmpB.gam /tmp/tmp.wrl");
  sprintf (system_befehl ,"cat /tmp/tmp.wrl >> \"%s\"", vrml);
  system (system_befehl);
  erase_file ("/tmp/tmp.wrl");
  erase_file ("/tmp/tmpA.gam");
  erase_file ("/tmp/tmpB.gam");
  }

  return 0;  
}

void
dump_vrml_header (char *vrml)
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


