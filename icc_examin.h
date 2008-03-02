/*
 * ICC Examin ist eine ICC Profil Betrachter
 * 
 * Copyright (C) 2004-2007  Kai-Uwe Behrmann 
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
 * the central class
 * 
 */

// Date:      Mai 2004

#ifndef ICC_EXAMIN_H
#define ICC_EXAMIN_H

#include "icc_utils.h"
//#include "icc_gl.h"
#include "icc_kette.h"
#include "icc_oyranos.h"
#include "icc_modell_beobachter.h"
#include "icc_thread_daten.h"

#include <string>
#include <vector>
#include <set>
#define USE_THREADS 1

class  ICCfltkBetrachter;
class  ICCwaehler;
class  ICCexamin;
extern ICCexamin *icc_examin;
class  ICCexaminIO;
class  icc_examin_ns::EinModell;

namespace icc_examin_ns {
    enum IccGamutFormat {
      ICC_ABSTRACT,
      ICC_VRML
    };
}

/** @brief central program instance
 *
 *  The class coordinates the user interface (UI) and data events,
 *  the start of the (fltk-)main loop and the threads.
 */
class ICCexamin : public icc_examin_ns::Beobachter,
                  public icc_examin_ns::ThreadDaten
{
  friend class ICCexaminIO;
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
    void         clear(void);

  private:
    ICCexaminIO *io_;
  public:
    int          erneuern();
    void         erneuern(int pos);
    enum AUFFRISCHEN {
    PROGRAMM = 1,
    OYRANOS = 2
    };
    void         auffrischen(int schalter);
  public:
    void         oeffnen ();                   //!< interactive
    void         oeffnen (std::vector<std::string> dateinamen);
    bool         lade ();
    void         lade (std::vector<Speicher> & neu);
    void         oyranos_einstellungen();

  private:
    void         erneuerTagBrowserText_ (void);//!< profile text in browserText
  public:
    bool         berichtSpeichern (void);      //!< GCATS report -> html file

    bool         gamutSpeichern (icc_examin_ns::IccGamutFormat format); //!< colour space hull / gamut
    void         zeig3D ();                    //!< 3D colour space view
    void         zeigPrueftabelle ();          //!< show controlling report
    void         zeigCGATS();                  //!< show corrected CGATS
    void         zeigMftTabellen();            //!< open all channels
    void         icc_betrachterNeuzeichnen (void* widget); //!< redraw UI

    std::string  waehleTag (int item);
    void         waehleMft (int item);
    std::vector<int> kurve_umkehren;
    enum { GL_STOP, GL_ZEICHNEN, GL_AUFFRISCHEN, GL_MOUSE_HIT3D }; //!< GL waiting
    icc_examin_ns::EinModell  * alle_gl_fenster;   //!< all Gl windows
  private:
    int  _item,  _mft_item;    //!< @brief selected profil items
    int  _zeig_prueftabelle,
         farbraum_angezeigt_;
    int  status_,
         intent_,              //!< rendering intent / de: Uebertragungsart
         bpc_,                 //!< black point compensation / de: Schwarzpunktkompensation
         gamutwarn_;           //!< gamut warning / de: Farbraumwarnung
    bool intent_selection_,    //!< interactive selected rendering intent
         farbraum_modus_;      //!< profile.profil() contains ncl2 named colours
    ICCwaehler *icc_waehler_;
  public:
    void waehlbar ( int pos, int v );//!< ICCwaehler item selectable or not
    int  laeuft ()  { return status_; }        //!< can be used with values > 1
    int  intentGet(int *interaktiv){ if(interaktiv)
                                       *interaktiv = (int)intent_selection_;
                                     return intent_; }        //!< the global rendering intent; see as well @see: ICCprofile.intent()
    void intent (int i);                       //!< set a rendering intent
    int  bpc ()  { return bpc_; }              //!< global BPC
    void bpc (int i);                          //!< set BPC
    int  gamutwarn ()  { return gamutwarn_; }  //!< gamut warning?
    void gamutwarn (int warn);
    void gamutAnsichtZeigen ();                //!< show DD_farbraum

  public:
    ICCfltkBetrachter* icc_betrachter;
    std::string statlabel[4];
    void        statusFarbe(double & CIEL, double & CIEa, double & CIEb);
    std::string detaillabel;
    void        setzeFensterTitel();
  public:
    int  tag_nr () { return _item; }
    int  mft_nr () { return _mft_item; }

    std::vector<std::vector<std::vector<double> > > kurven;
    std::vector<std::vector<double> >               punkte;
    std::vector<std::vector<std::string> >          texte;

    // loading of colour spaces
    void farbraum();
    void farbraum(int erneuere_nummer_in_profile_kette);
    bool farbraumModus( ) { /*DBG_PROG_V( farbraum_modus_ );*/ return farbraum_modus_; }
    void farbraumModus( int profil );
    void messwertLese  ( int n,
                         oyNamedColours_s ** list
                         /*std::vector<double> & p,
                         std::vector<double> & f,
                         std::vector<std::string> & namen*/);
    void setzMesswerte ( );
    void netzLese      ( int n,
                         std::vector<ICCnetz> * netz);
    void farbenLese    ( int n,
                         oyNamedColours_s ** list );
    void farbenLese    ( int n,
                         std::vector<double> & p,
                         std::vector<double> & f,
                         std::vector<std::string> & names );


    void vcgtZeigen ();
    void vcgtStoppen ();
  private:
    int  vcgt_cb_laeuft_b_;  //!< tells whether the vcgt window callback runs
  public:
    void testZeigen ();
    void moniHolen ();
    void moniSetzen ();
    std::string moniName();
    void standardGamma ();
  public:
      //! virtual from icc_examin_ns::Beobachter::
    void nachricht( icc_examin_ns::Modell* modell , int infos );

    // GUI functions
    double fortschritt( void );
    void fortschritt(double f, double anteil);

    void statusAktualisieren();  //!< use the "status" macro

    void scheme(const char* plastic_or_gtk); //!< "plastic" "gtk+"
};

int tastatur(int e);

#define status(texte) {std::stringstream s; s << texte; icc_examin->statlabel[0] = s.str(); icc_examin->statusAktualisieren();}

#endif //ICC_EXAMIN_H

