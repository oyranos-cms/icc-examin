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
 * Dateiwahl
 * 
 */

// Date:      November 2006

#ifndef ICC_FATAIWAHL_H
#define ICC_DATEIWAHL_H

#include "icc_utils.h"

#if HAVE_FLTK
#if HAVE_FLU
#include <FLU/Flu_File_Chooser.h> 
#endif
#include <FL/Fl_Hold_Browser.H> 
#include <FL/Fl_File_Icon.H> 
#include <FL/Fl_Shared_Image.H> 
#include <FL/Fl_PNM_Image.H>
#include <FL/Fl_Double_Window.H> 
#include <FL/fl_ask.H>


#include "my_file_chooser.h"
#include "icc_fenster.h"
namespace icc_examin_ns {


#if HAVE_FLU
  extern Flu_File_Chooser *dateiwahl;
  void dateiwahl_cb(const char *dateiname, int typ, void *arg);
#else
  extern MyFl_File_Chooser  *dateiwahl;
  void dateiwahl_cb(MyFl_File_Chooser *f,void *data);
#endif

}

#endif

#endif //ICC_DATEIWAHL_H

