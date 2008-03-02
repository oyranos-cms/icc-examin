/*
 * Internationalisation functions
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
 * FLTK specific Internationalisation functions
 * 
 */

#include "fl_i18n.H"

#include <locale.h>
#include <libintl.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>


#ifdef DEBUG
#include "icc_utils.h"
#else
extern int icc_debug;
#endif
#ifndef DBG_PROG_START
#define DBG_PROG_START
#endif
#ifndef DBG_PROG
#define DBG_PROG
#endif
#ifndef DBG_PROG_V
#define DBG_PROG_V(text)
#endif
#ifndef DBG_PROG_S
#define DBG_PROG_S(text)
#endif
#ifndef DBG_PROG_ENDE
#define DBG_PROG_ENDE
#endif

#define TEXTLEN 48


#include <FL/Fl_Menu_Item.H>
void
fl_translate_menue( Fl_Menu_Item* menueleiste )
{
#ifdef USE_GETTEXT
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
#endif
}

#include <Fl/Fl_File_Chooser.H>
void
fl_translate_file_chooser( )
{
#ifdef USE_GETTEXT
  DBG_PROG_START
    Fl_File_Chooser::add_favorites_label = _("Add to Favorites");
    Fl_File_Chooser::all_files_label = _("All Files (*)");
    Fl_File_Chooser::custom_filter_label = _("Custom Filter");
    Fl_File_Chooser::existing_file_label = _("Please choose an existing file!");
    Fl_File_Chooser::favorites_label = _("Favorites");
    Fl_File_Chooser::filename_label = _("Filename");
    Fl_File_Chooser::manage_favorites_label = _("Manage Favorites");
#   ifdef WIN32
    Fl_File_Chooser::filesystems_label = _("My Computer");
#   else
    Fl_File_Chooser::filesystems_label = _("Filesystems");
#   endif
    Fl_File_Chooser::new_directory_label = _("New Directory?");
    Fl_File_Chooser::preview_label = _("Preview");
#   if (FL_MAJOR_VERSION == 1 && FL_MINOR_VERSION >= 1 && FL_PATCH_VERSION >= 7)
    Fl_File_Chooser::save_label = _("Save"); // since 1.1.7?
#   endif
    Fl_File_Chooser::show_label = _("Show:");
  DBG_PROG_ENDE
#endif
}


