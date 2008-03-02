/*
 * ICC Examin ist eine ICC Profil Betrachter
 * 
 * Copyright (C) 2005  Kai-Uwe Behrmann 
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
 * Die Kette von Profilen Klasse.
 * 
 */

#include "icc_kette.h"
#include "icc_info.h"
//#include "callback_simple.h"

ICCkette profile;


bool
ICCkette::oeffnen (std::vector<std::string> dateinamen)
{ DBG_PROG_START
  bool erfolgreich = false;

  // Laden TODO: test auf Existenz der Datei (oyranos?)
  _profile.resize(dateinamen.size());
  for (unsigned int i = 0; i < dateinamen.size(); i++)
    _profile[i].load (dateinamen[i]);

  _profilnamen = dateinamen;
  _aktiv.resize(_profile.size());
  for(unsigned int i = 0; i < _aktiv.size(); i++)
    _aktiv[i] = true;

  if( _profile.size() )
    _aktuelles_profil = 0;

  if (dateinamen.size()) { DBG_PROG
    for (unsigned int i = 0; i < _profile.size(); i++) {
      fortschritt (0.8);

      std::vector<std::string> url;
      std::vector<std::string> param;
        std::string st = dateinamen[i];
        st.append (" ");
        st.append (_("geladen"));
        status_info(st.c_str());
    }
    fortschritt (1.0);
    fortschritt (1.1);
    DBG_PROG

    //Button a( make_callback((Callback1<Button*>*)0, icc_examin->icc_betrachter->tag_browser, &TagBrowser::reopen) );
    //icc_examin->icc_betrachter->tag_browser->reopen ();

    //icc_examin->icc_betrachter->measurement( _profile[_aktuelles_profil].hasMeasurement() );
    erfolgreich = true;
  } else { DBG_PROG
    status_info(_("Datei nicht geladen!"));
  }

  DBG_PROG_ENDE
  return erfolgreich;
}




