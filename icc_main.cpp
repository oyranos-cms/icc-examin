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

#ifdef __APPLE__
//#include <CFLocale.h>
#include <CoreFoundation/CoreFoundation.h>
#endif

#include <locale.h>
#include <libintl.h>


ICCexamin *icc_examin;

void
initialiseI18N()
{
  DBG_PROG_START
  // i18n
  std::string locale;
  char codeset[24] = "ISO-8859-1";

  #ifdef __APPLE__
  CFLocaleRef userLocaleRef = CFLocaleCopyCurrent();
  CFStringRef cfstring = CFLocaleGetIdentifier( userLocaleRef );
  CFShow( cfstring );
  DBG_PROG_V( CFStringGetLength(cfstring) )
  CFIndex gr = 36;
  char text[36];
  Boolean fehler = CFStringGetCString( cfstring, text, gr, kCFStringEncodingISOLatin1 );
  if(fehler) {
    DBG_PROG_S( "osX locale erhalten: " << text )
    locale = text;
  } else {
    DBG_PROG_S( "osX locale nicht erhalten" )
  }
  locale = setlocale (LC_MESSAGES, locale.c_str());
  #else

  if(getenv("LANG"))
    locale = getenv("LANG");
  else
    locale = setlocale (LC_MESSAGES, "");

    // locale de
  std::string::size_type pos = locale.find("de",0);
  if(pos == 0)
  {
    if((pos = locale.find_first_of(".")) != std::string::npos)
    {
      locale.erase( pos, locale.size()-pos );
    }
    locale += ".";
    locale += codeset;
    DBG_PROG_V( locale )
      // charset eintragen
    setenv("LANG", locale.c_str(), 1);
    locale = setlocale (LC_MESSAGES, "");
  }

  #endif

  if(locale.size())
    DBG_PROG_S( locale );


  // GNU gettext benoetigt die LANG Umgebungsvariable
  const char *loc = getenv("LANG");
  //system("set | grep LANG");
  if(loc) {
    DBG_PROG_V( loc )
  } else {
    DBG_PROG_S( "Versuche LANG zu setzen" )
    if (locale.size())
      setenv("LANG", locale.c_str(), 0);
    if(getenv("LANG"))
      DBG_PROG_S( getenv("LANG") );
  }
  DBG_PROG_V( system("set | grep LANG") )


  if(locale.size())
    DBG_PROG_S( locale );

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
  #ifndef __APPLE__
  char* cs = bind_textdomain_codeset("icc_examin", codeset);
  if(cs)
    DBG_PROG_S( "set codeset for \"icc_examin\" to " << cs )
  #endif
  // i18n Ende
  DBG_PROG_ENDE
}

int
main (int argc, char** argv)
{
  if(getenv("ICCEXAMIN_DEBUG") && atoi(getenv("ICCEXAMIN_DEBUG"))>0)
    icc_debug = atoi(getenv("ICCEXAMIN_DEBUG"));
  else
    icc_debug = 0;

  DBG_PROG_START

  initialiseI18N();

  ICCexamin hauptprogramm;

  icc_examin = &hauptprogramm;

  hauptprogramm.start(argc, argv);

  DBG_PROG_ENDE
  return false;
}


