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
 * Internationalisation funktions
 * 
 */


#include <string>
#include <locale.h>
#include <libintl.h>

#ifdef __APPLE__
#include <CoreFoundation/CoreFoundation.h>
#endif


#include "config.h"
#include "icc_helfer.h"
#include "icc_utils.h"



void
initialiseI18N()
{
  DBG_PROG_START

  std::string locale;
  char codeset[24] = "ISO-8859-1";

  #ifdef __APPLE__
  // 1. get the locale info
  CFLocaleRef userLocaleRef = CFLocaleCopyCurrent();
  CFStringRef cfstring = CFLocaleGetIdentifier( userLocaleRef );

  CFShow( cfstring );
  DBG_PROG_V( CFStringGetLength(cfstring) )

    // copy to a C buffer
  CFIndex gr = 36;
  char text[36];
  Boolean fehler = CFStringGetCString( cfstring, text, gr, kCFStringEncodingISOLatin1 );

  if(fehler) {

    DBG_PROG_S( _("osX locale obtained: ") << text )

    locale = text;

  } else {

    DBG_PROG_S( _("osX locale not obtained") )

  }

  // set the locale info
  locale = setlocale (LC_MESSAGES, locale.c_str());
  #else

  // 1. get default locale info ..
  locale = setlocale (LC_MESSAGES, "");

    // .. or take locale info from environment
  if(getenv("LANG"))
    locale = getenv("LANG");

    // check special case; locale de
    // TODO: add more LINGUAS here
  std::string::size_type pos = locale.find("de",0);
  if(pos == 0)
  {
    // 1 a. select an appropriate charset (needed for non UTF-8 fltk/gtk1)
    sprintf(codeset, "ISO-8859-1");

      // merge charset with locale string
    if((pos = locale.find_first_of(".")) != std::string::npos)
    {
      locale.erase( pos, locale.size()-pos );
    }
    locale += ".";
    locale += codeset;

    DBG_PROG_V( locale )

    // 1b. set correct environment variable LANG
    setenv("LANG", locale.c_str(), 1);

    // 1c. set the locale info after LANG
    locale = setlocale (LC_MESSAGES, "");
  }

  #endif

  if(locale.size())
    DBG_PROG_S( locale );


  // 2. for GNU gettext, the locale info is usually stored in the LANG variable
  const char *loc = getenv("LANG");

  if(loc) {

      // good
    DBG_PROG_V( loc )

  } else {

    DBG_PROG_S( _("try to set LANG") )

      // set LANG
    if (locale.size())
      setenv("LANG", locale.c_str(), 0);

      // good?
    if(getenv("LANG"))
      DBG_PROG_S( getenv("LANG") );
  }

  DBG_PROG_V( system("set | grep LANG") )
  if(locale.size())
    DBG_PROG_S( locale );

  // 3. our translations
  textdomain ("icc_examin");

  char test[1024];
  sprintf(test, "%s%s", LOCALEDIR, "/de/LC_MESSAGES/icc_examin.mo");
  char* bdtd = 0;

  // 4. where to find the MO file? select an appropriate directory
  if( holeDateiModifikationsZeit(test) ) {
      // installation directory ..
    bdtd = bindtextdomain ("icc_examin", LOCALEDIR);

    DBG_PROG_S( _("fine with: ") << bdtd );
  } else {
    DBG_PROG_S( _("failed with: ") << test );

      // .. our source directory
    bdtd = bindtextdomain ("icc_examin", SRC_LOCALEDIR);

    DBG_PROG_S( _("try locale in ") << bdtd );
  }
  #ifndef __APPLE__
  // 5. set our charset
  char* cs = bind_textdomain_codeset("icc_examin", codeset);

  if(cs)
    DBG_PROG_S( _("set codeset for \"icc_examin\" to ") << cs )
  #endif

  // gettext initialisation end

  DBG_PROG_ENDE
}

#ifdef HAVE_FLTK
#include <FL/Fl_Menu_Item.H>
void
menue_translate( Fl_Menu_Item* menueleiste )
{
  DBG_PROG_START
  int size = menueleiste->size();
  DBG_PROG_V( size )
  for(int i = 0; i < size ; ++i) {
    const char* text = menueleiste[i].label();
    menueleiste[i].label( _(text) );
    DBG_PROG_V( i )
    if(text)
      DBG_PROG_V( text <<" "<< _(text) );
  }
  DBG_PROG_ENDE
}
#endif

