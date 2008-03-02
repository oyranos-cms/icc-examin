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
#include "icc_kette.h"
#include "icc_oyranos.h"

class  ICCfltkBetrachter;
class  ICCexamin;
extern ICCexamin *icc_examin;

class ICCexamin
{
  enum {
    TAG_VIEWER,
    MFT_VIEWER,
    VCGT_VIEWER,
    MAX_VIEWER
  };
  public:
                 ICCexamin ();
                 ~ICCexamin () {; }

    void         start(int argc, char** argv);
    void         quit(void);

    void         oeffnen (std::vector<std::string> dateinamen);
    void         oeffnen ();                   // interaktiv
    bool         berichtSpeichern (void);      // GCATS Auswertung -> html Datei
    void         zeigPrueftabelle ();
    void         zeigCGATS();                  // korrigiertes CGATS zeigen
    void         neuzeichnen (void* widget);   // Oberfläche neuzeichnen

    std::string  waehleTag (int item);
    void         waehleMft (int item);
    std::vector<int> kurve_umkehren;
  private:
    int  _item,  _mft_item;
    int  _zeig_prueftabelle,
         _zeig_histogram;
    int  status_;
  public:
    int  laeuft () { return status_; }          // kann bei >1 genutzt werden

  public:
    ICCfltkBetrachter* icc_betrachter;
    std::string statlabel;
  public:
    int  tag_nr () { return _item; }
    int  mft_nr () { return _mft_item; }

    std::vector<std::vector<std::vector<double> > > kurven;
    std::vector<std::vector<double> >               punkte;
    std::vector<std::vector<std::string> >          texte;

    void histogram();
    void vcgtZeigen ();
    void moniSetzen ();
    void standardGamma ();
  private:
    int _gl_ansicht;                   // glutWindow
    std::vector<GL_Ansicht*> _gl_ansichten;
  public:
    void initReihenfolgeGL_Ansicht(GL_Ansicht*);
    void glAnsicht (GL_Ansicht* dazu);
    GL_Ansicht* glAnsicht(int id);

    // Oberflächenfunktionen (GUI)
    void fortschritt(double f);  // Fortschritt: f<0-Start f=Wert f>1-Ende

    void statusAktualisieren();  // benutze das "status" Makro
};

#define _(text) text
#define status(texte) {std::stringstream s; s << texte; icc_examin->statlabel = s.str(); icc_examin->statusAktualisieren();}

#endif //ICC_EXAMIN_H

