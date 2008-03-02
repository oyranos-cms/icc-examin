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

#include "icc_utils.h"
#include "icc_gl.h"
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

namespace icc_examin_ns {
    enum IccGamutFormat {
      ICC_ABSTRACT,
      ICC_VRML,
    };
}

/** @brief zentrale Programminstanz
 *
 *  Die Klasse uebernimmt die Abstimmung von Oberflaechen und Datenereignissen,
 *  den Start der (fltk-)Warteschleife und der Threads.
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

  private:
    ICCexaminIO *io_;
  public:
    int          erneuern();
    void         erneuern(int pos);
  public:
    void         oeffnen ();                   //!< interaktiv
    void         oeffnen (std::vector<std::string> dateinamen);
    bool         lade ();
    void         lade (std::vector<Speicher> & neu);

  private:
    void         erneuerTagBrowserText_ (void);//!< Profil Text in browserText
  public:
    bool         berichtSpeichern (void);      //!< GCATS Auswertung -> html Datei

    bool         gamutSpeichern (icc_examin_ns::IccGamutFormat format); //!< Farbraumhuelle
    void         zeigPrueftabelle ();
    void         zeigCGATS();                  //!< korrigiertes CGATS zeigen
    void         icc_betrachterNeuzeichnen (void* widget); //!< Oberflaechenpflege

    std::string  waehleTag (int item);
    void         waehleMft (int item);
    std::vector<int> kurve_umkehren;
  private:
    int  _item,  _mft_item;    //!< @brief ausgewaehlte Profilbestandteile
    int  _zeig_prueftabelle,
         farbraum_angezeigt_;
    int  status_,
         intent_,              //!< Uebertragungsart
         bpc_,                 //!< Schwarzpunktkompensation
         gamutwarn_;           //!< Farbraumwarnung
    bool intent_selection_,    //!< interaktiv gewaehlte Uebertragungsart
         farbraum_modus_;      //!< profile.profil() enthaelt ncl2 Schmuckfarben
    ICCwaehler *icc_waehler_;
  public:
    int  laeuft ()  { return status_; }        //!< kann bei >1 genutzt werden
    int  intentGet(int *interaktiv){ if(interaktiv)
                                       *interaktiv = (int)intent_selection_;
                                     return intent_; }        //!< die globale Uebertragungsart; siehe auch @see: ICCprofile.intent()
    void intent (int i);                       //!< setze eine Uebertragungsart
    int  bpc ()  { return bpc_; }              //!< globale BPC
    void bpc (int i);                          //!< setze BPC
    int  gamutwarn ()  { return gamutwarn_; }  //!< Farbraumwarnung?
    void gamutwarn (int warn) { gamutwarn_ = warn; }
    void gamutAnsichtZeigen ();                //!< DD_farbraum zeigen

  public:
    ICCfltkBetrachter* icc_betrachter;
    std::string statlabel;
    void        statusFarbe(double & CIEL, double & CIEa, double & CIEb);
    std::string detaillabel;
  public:
    int  tag_nr () { return _item; }
    int  mft_nr () { return _mft_item; }

    std::vector<std::vector<std::vector<double> > > kurven;
    std::vector<std::vector<double> >               punkte;
    std::vector<std::vector<std::string> >          texte;

    // Farbraeume laden
    void farbraum();
    void farbraum(int erneuere_nummer_in_profile_kette);
    bool farbraumModus( ) { /*DBG_PROG_V( farbraum_modus_ );*/ return farbraum_modus_; }
    void farbraumModus( int profil );
    void messwertLese  ( int n,
                         std::vector<double> & p,
                         std::vector<float>  & f,
                         std::vector<std::string> & namen);
    void setzMesswerte ( );
    void netzLese      ( int n,
                         std::vector<ICCnetz> * netz);
    void farbenLese    ( int n,
                         std::vector<double> & p,
                         std::vector<float>  & f);


    void vcgtZeigen ();
    void vcgtStoppen ();
  private:
    int  vcgt_cb_laeuft_b_;  //!< erzaehlt ob der vcgt Fenster cp laeuft
  public:
    void testZeigen ();
    void moniHolen ();
    void moniSetzen ();
    void standardGamma ();
  public:
      //! virtual aus icc_examin_ns::Beobachter::
    void nachricht( icc_examin_ns::Modell* modell , int infos );

    // Oberflaechenfunktionen (GUI)
    void fortschritt(double f);  //!< Fortschritt: f<0-Start f=Wert f>1-Ende
    void fortschrittThreaded(double f);  //!< inclusive aller Thread Funktionen

    void statusAktualisieren();  //!< benutze das "status" Makro

};

int tastatur(int e);

#define status(texte) {std::stringstream s; s << texte; icc_examin->statlabel = s.str(); icc_examin->statusAktualisieren();}

#endif //ICC_EXAMIN_H

