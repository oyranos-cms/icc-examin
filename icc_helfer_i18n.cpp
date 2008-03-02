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
 * Internationalisation functions
 * 
 */


#include <string>
#include <locale.h>
#include <libintl.h>

#include "config.h"
#include "icc_helfer.h"
#include "icc_utils.h"

#ifdef APPLE
#include <CoreFoundation/CoreFoundation.h>
#endif




void
initialiseI18N()
{
  DBG_PROG_START

  std::string locale;

  #ifdef APPLE
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

  char codeset[24] = "ISO-8859-1";

  // 1. get default locale info ..
  locale = setlocale (LC_MESSAGES, "");

    // .. or take locale info from environment
  if(getenv("LANG"))
    locale = getenv("LANG");

    #define SetCodesetForLocale( lang, codeset_ ) \
    { \
      std::string::size_type pos = locale.find( lang ,0); \
      if(pos == 0) \
      { \
        /* 1 a. select an appropriate charset (needed for non UTF-8 fltk/gtk1)*/ \
        sprintf (codeset, codeset_); \
        \
          /* merge charset with locale string */ \
        if((pos = locale.find_first_of(".")) != std::string::npos) \
        { \
          locale.erase( pos, locale.size()-pos ); \
        } \
        locale += "."; \
        locale += codeset; \
        \
        DBG_PROG_V( locale ) \
        \
        /* 1b. set correct environment variable LANG */ \
        /*setenv("LANG", locale.c_str(), 1);*/ /* setenv is not standard C */ \
        \
        /* 1c. set the locale info after LANG */ \
        locale = setlocale (LC_MESSAGES, ""); \
      } \
    }

      // add more LINGUAS here
      // borrowed from http://czyborra.com/charsets/iso8859.html
    SetCodesetForLocale( "af", "ISO-8859-1" ) // Afrikaans
    SetCodesetForLocale( "ca", "ISO-8859-1" ) // Catalan
    SetCodesetForLocale( "da", "ISO-8859-1" ) // Danish
    SetCodesetForLocale( "de", "ISO-8859-1" ) // German
    SetCodesetForLocale( "en", "ISO-8859-1" ) // English
    SetCodesetForLocale( "es", "ISO-8859-1" ) // Spanish
    SetCodesetForLocale( "eu", "ISO-8859-1" ) // Basque
    SetCodesetForLocale( "fi", "ISO-8859-1" ) // Finnish
    SetCodesetForLocale( "fo", "ISO-8859-1" ) // Faroese
    SetCodesetForLocale( "fr", "ISO-8859-1" ) // French
    SetCodesetForLocale( "ga", "ISO-8859-1" ) // Irish
    SetCodesetForLocale( "gd", "ISO-8859-1" ) // Scottish
    SetCodesetForLocale( "is", "ISO-8859-1" ) // Icelandic
    SetCodesetForLocale( "it", "ISO-8859-1" ) // Italian
    SetCodesetForLocale( "nl", "ISO-8859-1" ) // Dutch
    SetCodesetForLocale( "no", "ISO-8859-1" ) // Norwegian
    SetCodesetForLocale( "pt", "ISO-8859-1" ) // Portuguese
    SetCodesetForLocale( "rm", "ISO-8859-1" ) // Rhaeto-Romanic
    SetCodesetForLocale( "sq", "ISO-8859-1" ) // Albanian
    SetCodesetForLocale( "sv", "ISO-8859-1" ) // Swedish
    SetCodesetForLocale( "sw", "ISO-8859-1" ) // Swahili

    SetCodesetForLocale( "cs", "ISO-8859-2" ) // Czech
    SetCodesetForLocale( "hr", "ISO-8859-2" ) // Croatian
    SetCodesetForLocale( "hu", "ISO-8859-2" ) // Hungarian
    SetCodesetForLocale( "pl", "ISO-8859-2" ) // Polish
    SetCodesetForLocale( "ro", "ISO-8859-2" ) // Romanian
    SetCodesetForLocale( "sk", "ISO-8859-2" ) // Slovak
    SetCodesetForLocale( "sl", "ISO-8859-2" ) // Slovenian

    SetCodesetForLocale( "eo", "ISO-8859-3" ) // Esperanto
    SetCodesetForLocale( "mt", "ISO-8859-3" ) // Maltese

    SetCodesetForLocale( "et", "ISO-8859-4" ) // Estonian
    SetCodesetForLocale( "lv", "ISO-8859-4" ) // Latvian
    SetCodesetForLocale( "lt", "ISO-8859-4" ) // Lithuanian
    SetCodesetForLocale( "kl", "ISO-8859-4" ) // Greenlandic

    SetCodesetForLocale( "be", "ISO-8859-5" ) // Byelorussian
    SetCodesetForLocale( "bg", "ISO-8859-5" ) // Bulgarian
    SetCodesetForLocale( "mk", "ISO-8859-5" ) // Macedonian
    SetCodesetForLocale( "ru", "ISO-8859-5" ) // Russian
    SetCodesetForLocale( "sr", "ISO-8859-5" ) // Serbian
    SetCodesetForLocale( "uk", "ISO-8859-5" ) // Ukrainian

    SetCodesetForLocale( "ar", "ISO-8859-6" ) // Arabic
    SetCodesetForLocale( "fa", "ISO-8859-6" ) // Persian
    SetCodesetForLocale( "ur", "ISO-8859-6" ) // Pakistani Urdu

    SetCodesetForLocale( "el", "ISO-8859-7" ) // Greek

    SetCodesetForLocale( "iw", "ISO-8859-8" ) // Hebrew
    SetCodesetForLocale( "ji", "ISO-8859-8" ) // Yiddish

    SetCodesetForLocale( "tr", "ISO-8859-9" ) // Turkish

    SetCodesetForLocale( "th", "ISO-8859-11" ) // Thai

    SetCodesetForLocale( "ja", "SJIS" ) // Japan ; eucJP, ujis, EUC, PCK, jis7, SJIS
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
    #ifdef APPLE
    if (locale.size())
      setenv("LANG", locale.c_str(), 0);
    #endif

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
  #ifndef APPLE
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


