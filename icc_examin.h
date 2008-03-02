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
 * Die zentrale Klasse
 * 
 */

// Date:      Mai 2004

#ifndef ICC_EXAMIN_H
#define ICC_EXAMIN_H

#include <string>
#include <vector>
#include "icc_utils.h"
#include "icc_gl.h"
class ICCfltkBetrachter;
class ICCexamin;
extern ICCexamin *icc_examin;

class ICCexamin
{
  public:
                 ICCexamin ();
                 ~ICCexamin () {; }

    void         start(int argc, char** argv);
    void         quit(void);

    void         oeffnen (std::vector<std::string> dateinamen);
    void         oeffnen ();	// interaktiv
    void         neuzeichnen (void* widget);   // Oberfläche neuzeichnen

    std::string  waehleTag (int item);
    void         waehleMft (int item);
    int          kurve_umkehren;
  private:
    int  _item,  _mft_item;
    int  _zeig_prueftabelle,
         _zeig_histogram;

  public:
    ICCfltkBetrachter* icc_betrachter;
    std::string statlabel;
  public:
    int  tag_nr () { return _item; }
    int  mft_nr () { return _mft_item; }
    void zeig_prueftabelle ();

    std::vector<std::vector<double> > kurven;
    std::vector<double> punkte;
    std::vector<std::string> texte;

  private:
    int  _feld;

  public:
    void histogram();
  private:
    int _gl_ansicht;
    std::vector<GL_Ansicht*> _gl_ansichten;
  public:
    void glAnsicht (GL_Ansicht* dazu) { DBG_PROG_START bool vorhanden = false;
                              for (unsigned i = 0; i < _gl_ansichten.size();i++)
                                if (dazu == _gl_ansichten[i]) { DBG_PROG
                                  vorhanden = true;
                                  _gl_ansicht = i;
                                }
                              if(!vorhanden) { DBG_PROG
                                _gl_ansicht = _gl_ansichten.size();
                                _gl_ansichten.resize( _gl_ansicht + 1 );
                                _gl_ansichten[_gl_ansicht] = dazu; DBG_PROG_ENDE
                              } DBG_PROG_ENDE
                            }
    GL_Ansicht* glAnsicht() { if(_gl_ansicht>=0)
                                return _gl_ansichten[_gl_ansicht];
                              else return 0; }

};

#endif //ICC_EXAMIN_H

