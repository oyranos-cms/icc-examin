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

#include "icc_utils.h"
#include "config.h"
#include "icc_helfer.h"
#include "icc_examin.h"
#include "icc_kette.h"
#include "fl_i18n/fl_i18n.H"

#ifdef WIN32
#define DIR_SEPARATOR_C '\\'
#define DIR_SEPARATOR "\\"
#else
#define DIR_SEPARATOR_C '/'
#define DIR_SEPARATOR "/"
#endif
//#include <FL/Fl.H>
//#include <FL/Fl_Double_Window.H>

void  MallocDebug_CheckFreeList ();
char* getExecPath               (const char *filename);

int
main (int argc, char** argv)
{
# ifdef CWDEBUG
  Debug(myproject::debug::init());		// This is a custom function defined
  						// in example-project/debug.cc.
# endif
# if HAVE_PTHREAD_H
  icc_thread_liste[THREAD_HAUPT] = pthread_self();

# ifdef CWDEBUG
  Debug(myproject::debug::init_thread());	// This is a custom function defined
  						// in example-project/debug.cc.
# endif
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
  signed int is_path = -1;
  int num_paths = 0;
# if __APPLE__
  std::string bdr;
  // RESOURCESPATH is set in the bundle by "Contents/MacOS/ICC Examin.sh"
  if(getenv("RESOURCESPATH")) {
    bdr = getenv("RESOURCESPATH");
    bdr += "/locale";
    locale_paths[0] = bdr.c_str(); ++num_paths;
  }
  if(!locale_paths[0]) {
    bdr = icc_examin_ns::holeBundleResource("locale","");
    if(bdr.size())
    {
      locale_paths[0] = bdr.c_str();
      ++num_paths;
    }
  } 
  locale_paths[1] = LOCALEDIR; ++num_paths;
  locale_paths[2] = SRC_LOCALEDIR; ++num_paths;
# else
  locale_paths[0] = LOCALEDIR; ++num_paths;

  DBG_NUM_V( argc <<" "<< argv[0] )

  if (argc)
  { const char *reloc_path = {"../share/locale"};
    int len = (strlen(argv[0]) + strlen(reloc_path)) * 2 + 128;
    char *path = (char*) malloc( len ); // small one time leak
    char *text = NULL;

    text = getExecPath( argv[0] );
    snprintf (path, len-1, "%s%s%s", text, DIR_SEPARATOR, reloc_path);
    locale_paths[1] = path; ++num_paths;
    locale_paths[2] = SRC_LOCALEDIR; ++num_paths;
    DBG_NUM_V( path );
    if (text) free (text);
  } else {
    locale_paths[1] = SRC_LOCALEDIR; ++num_paths;
  }
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

char*
getExecPath(const char *filename)
{
  DBG_PROG_START
  char *exec_path = NULL;

  if (filename)
  {
    int len = strlen(filename) * 2 + 1024;
    char *text = (char*) calloc( sizeof(char), len );
    text[0] = 0;
    /* whats the path for the executeable ? */
    snprintf (text, len-1, filename);

    if(text[0] == '~')
    {
      /* home directory */
      if(getenv("HOME"))
        sprintf( text, "%s%s", getenv("HOME"), &filename[0]+1 );
    }

    /* relative names - where the first sign is no directory separator */
    if (text[0] != DIR_SEPARATOR_C)
    {
      FILE *pp = NULL;

      if (text) free (text);
      text = (char*) malloc( 1024 );

      /* Suche das ausfuehrbare Programm
         TODO symbolische Verknuepfungen */
      snprintf( text, 1024, "which %s", filename);
      pp = popen( text, "r" );
      if (pp) {
        if (fscanf (pp, "%s", text) != 1)
        {
          pclose (pp);
          printf( "no executeable path found\n" );
        }
      } else {
        printf( "could not ask for executeable path\n" );
      }

      if(text[0] != DIR_SEPARATOR_C)
      {
        char* cn = (char*) calloc(2048, sizeof(char));
        sprintf (cn, "%s%s%s", getenv("PWD"), DIR_SEPARATOR, filename);
        sprintf (text, cn);
        if(cn) free(cn);
      }
    }

    { /* remove the executable name */
      char *tmp = strrchr(text, DIR_SEPARATOR_C);
      if(tmp)
        *tmp = 0;
    }
    while (text[strlen(text)-1] == '.')
      text[strlen(text)-1] = 0;
    while (text[strlen(text)-1] == DIR_SEPARATOR_C)
      text[strlen(text)-1] = 0;

    exec_path = text;
  }

  DBG_PROG_ENDE
  return exec_path;
}

