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
 * Start Funktion.
 * 
 */


#include "config.h"
#include "icc_examin.h"
#include <vector>


#include <locale.h>
#include <libintl.h>


ICCexamin *icc_examin;


int
main (int argc, char** argv)
{
  if(getenv("ICCEXAMIN_DEBUG") && atoi(getenv("ICCEXAMIN_DEBUG"))>0)
    icc_debug = atoi(getenv("ICCEXAMIN_DEBUG"));
  else
    icc_debug = 0;

  // i18n
  std::string locale = setlocale (LC_MESSAGES, "");
  if(locale.size())
    DBG_PROG_S( locale );
  
  std::string::size_type pos = locale.find("de",0);
  if(pos == 0) { // de
    if((pos = locale.find_first_of(".")) != std::string::npos) {
      locale.erase( pos, locale.size()-pos );
    }
    locale += ".ISO-8859-1";
    setlocale (LC_MESSAGES, locale.c_str());
    system("export LANG=ISO-8859-1");
    DBG_PROG_V( locale )
  }

  textdomain ("icc_examin");
  char test[1024];
  sprintf(test, "%s%s", LOCALEDIR, "/de/LC_MESSAGES/icc_examin.mo");
  char* bdtd = 0;
  if( holeDateiModifikationsZeit(test) ) {
    bdtd = bindtextdomain ("icc_examin", LOCALEDIR);
    DBG_PROG_S( "Hat geklappt: " << bdtd );
  } else {
    DBG_PROG_S( "daneben: " << test );
    bdtd = bindtextdomain ("icc_examin", SRC_LOCALEDIR);
    DBG_PROG_S( "Versuche locale in " << bdtd );
  }
  // i18n Ende

  ICCexamin hauptprogramm;

  icc_examin = &hauptprogramm;

  hauptprogramm.start(argc, argv);

  return false;
}


