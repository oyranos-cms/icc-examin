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
 * Dateiwahl
 * 
 */

#include "icc_kette.h"
#include "icc_examin.h"
#include "icc_fenster.h"
#include "icc_utils.h"
#include <string>
#include <vector>

#ifdef HAVE_FLTK
#ifdef HAVE_FLU
Flu_File_Chooser *dateiwahl;

void
dateiwahl_cb (const char *dateiname, int typ, void *arg)
{ DBG_PROG_START

  // kein Profile Dialog
  if (strstr( dateiwahl->pattern(), "*.ic*") == 0 &&
      strstr( dateiwahl->pattern(), "*.txt") == 0 &&
      strstr( dateiwahl->pattern(), "*.TXT") == 0 &&
      strstr( dateiwahl->pattern(), "*.it8") == 0 &&
      strstr( dateiwahl->pattern(), "*.IT8") == 0 &&
      strstr( dateiwahl->pattern(), "*.CMYK") == 0 &&
      strstr( dateiwahl->pattern(), "*.LAB") == 0 &&
      strstr( dateiwahl->pattern(), "*.IC*") == 0 ) { DBG_PROG
    DBG_PROG_ENDE
    return;
  }

    if (dateiname)
    {
      std::vector<std::string> profilnamen;
      profilnamen.resize(1);
      //profilnamen[0] = dateiname;

      DBG_NUM_V( profile )
      profilnamen[0] = dateiwahl->get_current_directory();
      profilnamen[0].append( dateiname );
      DBG_NUM_V( profilnamen[0] )
      icc_examin->oeffnen( profilnamen );
    }

  DBG_PROG_ENDE
}
#else
Fl_File_Chooser  *dateiwahl;

void
dateiwahl_cb (Fl_File_Chooser *f,void *data)
{ DBG_PROG_START

  const char *filename;

    Fl_File_Chooser* fl = (Fl_File_Chooser*)f;

    DBG_NUM_V( data )
    filename = fl->value();
  
    // kein Profile Dialog
    if (strstr(fl->filter(), "*.{I,i}{C,c}") == 0) {
      if (filename)
        DBG_PROG_V( filename )
      DBG_PROG_ENDE
      return;
    }

    if (filename && fl->count() && dateiwahl->preview()) {
      std::vector<std::string> profilnamen;
      profilnamen.resize(fl->count());
      for (int i = 0; i < fl->count(); i++) {
        profilnamen[i] = fl->value(i);
      }
      DBG_NUM_V( profilnamen.size() << filename )
      icc_examin->oeffnen(profilnamen);
    }

  DBG_PROG_ENDE
}
#endif

void
nachricht(std::string text)
{ DBG_PROG_START
  fl_message(text.c_str());
  DBG_PROG_ENDE
}

#endif


