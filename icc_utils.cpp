/*
 * ICC Examin ist eine ICC Profil Betrachter
 * 
 * Copyright (C) 2004  Kai-Uwe Behrmann 
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
 * Werkzeuge.
 * 
 */

// Date:      August 2004


#ifndef HAVE_OY
int level_PROG = -1;
#endif
int icc_debug = 1;

#include <icc_utils.h>
#include <fstream>
std::stringstream debug_s;

//#define WRITE_DBG

void
dbgWriteF (std::stringstream & ss)
{
#ifdef WRITE_DBG
    std::string dateiname = "/tmp/icc_examin_dbg_";
    dateiname += getenv("USER");
    dateiname += ".txt";
    std::ofstream f ( dateiname.c_str(),
         std::ios::out | std::ios::app | std::ios::binary | std::ios::ate );

    DBG_MEM_V( dateiname )
    if (dateiname == "")
    {
      DBG_PROG_ENDE
      throw ausn_file_io (_("no filename given"));
    }
    DBG_MEM
    if (!f) {
      DBG_PROG_ENDE
      throw ausn_file_io (dateiname.c_str());
      dateiname = "";
    }

    size_t size = (unsigned int)f.tellp();
    DBG_MEM_V ( size << "|" << f.tellp() )
    //f.seekp(size);
    const char* data = ss.str().c_str();
    size = ss.str().size();
    if(size) {
      f.write (data, size);
      DBG_MEM_V ( size << "|" << f.tellp() <<" "<< (int*)data <<" "<< strlen(data) )
      f.close();
    } else {
      data = 0;
      WARN_S( _("Dateigroesse 0 fuer ") << dateiname )
    }



#else
  cout << ss.str();
#endif
}


