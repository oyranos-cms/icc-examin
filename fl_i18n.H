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
 * I18N Helfer
 * 
 */

// Date:      Januar 2006

#ifndef FL_I18N_H
#define FL_I18N_H

#ifndef USE_GETTEXT
#define USE_GETTEXT
#endif
#ifdef USE_GETTEXT
# include <libintl.h>
# define _(text) threadGettext(text)
const char* threadGettext( const char* text);
#else
# define _(text) text
#endif


// I18N
void initialiseI18N();

#include <FL/Fl_Menu_Item.H>
// I18N in fl_i18n.cxx
void        menue_translate( Fl_Menu_Item *menueleiste );
void        file_chooser_translate ();

#endif //FL_I18N_H
