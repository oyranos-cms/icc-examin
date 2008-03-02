/*
 * ICC Examin ist eine ICC Profil Betrachter
 * 
 * Copyright (C) 2004-2006  Kai-Uwe Behrmann 
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
#include "icc_utils.h"
#include "icc_helfer.h"
#include "icc_examin.h"
#include "icc_kette.h"
#include "fl_i18n/fl_i18n.H"

//#include <FL/Fl.H>
//#include <FL/Fl_Double_Window.H>

void MallocDebug_CheckFreeList();

int
main (int argc, char** argv)
{
# if HAVE_PTHREAD_H
  icc_thread_liste[THREAD_HAUPT] = pthread_self();
# endif

# if APPLE
  //MallocDebug_CheckFreeList();
# endif

  if(getenv("ICCEXAMIN_DEBUG") && atoi(getenv("ICCEXAMIN_DEBUG"))>0)
    icc_debug = atoi(getenv("ICCEXAMIN_DEBUG"));
  else
    icc_debug = 0;

  DBG_PROG_START

  const char *locale_paths[3] = {0,0,0};
  int is_path = -1;
  int num_paths = 2;
# if __APPLE__
  std::string bdr;
  // RESOURCESPATH is set in the bundle by "Contents/MacOS/ICC Examin.sh"
  if(getenv("RESOURCESPATH")) {
    bdr = getenv("RESOURCESPATH");
    bdr += "/locale";
    locale_paths[0] = bdr.c_str();
  }
  if(!locale_paths[0]) {
    bdr = icc_examin_ns::holeBundleResource("locale","");
    if(bdr.size())
      locale_paths[0] = bdr.c_str();
  } 
  locale_paths[1] = LOCALEDIR;
  locale_paths[2] = SRC_LOCALEDIR; ++num_paths;
# else
  locale_paths[0] = LOCALEDIR;
#ifdef WIN32
#define DIR_SEPARATOR_C '\\'
#define DIR_SEPARATOR "\\"
#else
#define DIR_SEPARATOR_C '/'
#define DIR_SEPARATOR "/"
#endif
  DBG_NUM_V( argc <<" "<< argv[0] )
  if (argc)
  { const char *reloc_path = {"../share/locale"};
    int len = (strlen(argv[0]) + strlen(reloc_path)) * 2;
    char *path = (char*) malloc( len ); // small one time leak
    char *text = (char*) malloc( len );
    text[0] = 0;
    // whats the path for the executeable ?
    snprintf (text, len-1, argv[0]);
    if (strrchr(text, DIR_SEPARATOR_C)) {
      char *tmp = strrchr(text, DIR_SEPARATOR_C);
      *tmp = 0;
    }
    snprintf (path, len-1, "%s%s%s",text,DIR_SEPARATOR,reloc_path);
    locale_paths[1] = path;
    locale_paths[2] = SRC_LOCALEDIR; ++num_paths;
    DBG_NUM_V( path );
    if (text) free (text);
  } else
    locale_paths[1] = SRC_LOCALEDIR;
# endif
  is_path = fl_search_locale_path (num_paths, locale_paths, "de", "icc_examin");

  if(is_path >= 0) {
    fl_initialise_locale ( "icc_examin", locale_paths[is_path] );
    DBG_NUM_S( "locale gefunden in: " << locale_paths[is_path] )
  }


  ICCexamin hauptprogramm;

  icc_examin = &hauptprogramm;

  hauptprogramm.start(argc, argv);

  DBG_PROG_ENDE
  return false;
}


