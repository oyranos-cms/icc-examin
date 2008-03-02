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
 * Die zentrale Klasse.
 * 
 */


#include "icc_examin.h"
#include "icc_betrachter.h"

using namespace icc_examin_ns;


//#define DEBUG_EXAMIN
#ifdef DEBUG_EXAMIN
#define DBG_EXAMIN_START DBG_PROG_START
#define DBG_EXAMIN_ENDE DBG_PROG_ENDE
#define DBG_EXAMIN_V( texte ) DBG_NUM_V( texte )
#define DBG_EXAMIN_S( texte ) DBG_NUM_S( texte )
#else
#define DBG_EXAMIN_START
#define DBG_EXAMIN_ENDE
#define DBG_EXAMIN_V( texte )
#define DBG_EXAMIN_S( texte )
#endif



void
ICCexamin::messwertLese (int n,
                         std::vector<double> & p,
                         std::vector<float>  & f,
                         std::vector<std::string> & namen)
{
  DBG_PROG_START
  if(profile.size() > n &&
     profile.aktuell() == n &&
     profile[n]->hasMeasurement() &&
     profile[n]->getMeasurement().hasXYZ() )
    { DBG_NUM_S( "nutze Messdaten" )
      ICCmeasurement messung = profile[n]->getMeasurement();

      if(messung.valid() && profile[n]->size())
        icc_betrachter->DD_histogram->zeig_punkte_als_messwert_paare = true;
      else
        icc_betrachter->DD_histogram->zeig_punkte_als_messwert_paare = false;
      DBG_NUM_V( icc_betrachter->DD_histogram->zeig_punkte_als_messwert_paare )
      icc_betrachter->DD_histogram->zeig_punkte_als_messwerte = true;
      DBG_NUM_V( icc_betrachter->DD_histogram->zeig_punkte_als_messwerte )

      unsigned int j;
      int n = messung.getPatchCount(); DBG_PROG_V( messung.getPatchCount() )
      for (j = 0; j < (unsigned) n; ++j)
      { // zuerst die Messwerte ...
        std::vector<double> daten = messung.getMessLab(j);
        for (unsigned i = 0; i < daten.size(); ++i)
          p.push_back(daten[i]);
        // ... dann die über das Profil errechneten Lab Werte
        if (icc_betrachter->DD_histogram->zeig_punkte_als_messwert_paare) {
          daten = messung.getCmmLab(j);
          for (unsigned i = 0; i < daten.size(); ++i)
            p.push_back(daten[i]);
        } 

        daten = messung.getMessRGB(j);
        for (unsigned i = 0; i < daten.size(); ++i) {
          f.push_back((float)daten[i]);
        }
        f.push_back(1.0);
        if (icc_betrachter->DD_histogram->zeig_punkte_als_messwert_paare)
        { daten = messung.getCmmRGB(j);
          for (unsigned i = 0; i < daten.size(); ++i)
            f.push_back(daten[i]);
          f.push_back(1.0);
        } 
      }
      namen = messung.getFeldNamen();
    }

  DBG_PROG_ENDE
}

void
ICCexamin::netzLese (int n,
                     std::vector<ICCnetz> *netz)
{
  DBG_PROG_START
  if( profile.size() == 0 && histogramModus()) {
    netz->resize(1);
    return;
  }

  std::vector<ICCnetz> netz_temp;
  Speicher s;

  if(profile.size() > n)
    if(profile[n]->valid()) {
      size_t groesse = 0;
      char* daten = profile[n]->saveProfileToMem(&groesse); 
      s.lade(daten, groesse);
      DBG_NUM_V( groesse );
    }

  if(s.size())
  {
    int intent = 1;
    if(histogramModus())
      intent = profile.profil()->intent();

    netz_temp = icc_oyranos. netzVonProfil( s, intent );
    netz_temp[0].transparenz = (*netz)[n].transparenz;
    netz_temp[0].grau = (*netz)[n].grau;
    if(netz_temp.size())
    {
      if(n >= (int)netz->size())
        netz->resize( n+1 );
      DBG_PROG_V( netz->size() <<" "<< netz_temp.size() )
      (*netz)[n] = netz_temp[0];
      (*netz)[n].name = profile[n]->filename();
      DBG_NUM_V( (*netz)[n].transparenz )
    }
  } else
    WARN_S(_("kein Profil im Speicher"))

  DBG_PROG_ENDE
}

void
ICCexamin::farbenLese (int n,
                       std::vector<double> & p,
                       std::vector<float>  & f)
{
  DBG_PROG_START
  // benannte Farben darstellen
  if( profile.size() > n )
  {
    DBG_PROG
    p = profile[n]->getTagNumbers (profile[n]->getTagByName("ncl2"),
                                         ICCtag::MATRIX);
    DBG_NUM_V( p[0] )
    f.resize( (int)p[0] * 4);
    DBG_NUM_V( f.size() )
    for(unsigned i = 0; i < f.size(); ++i)
      f[i] = 1.0;
    p.erase( p.begin() );
    icc_betrachter->DD_histogram->zeig_punkte_als_messwert_paare = false;
    icc_betrachter->DD_histogram->zeig_punkte_als_messwerte = false;
  }

  DBG_PROG_ENDE
}

void
ICCexamin::histogram (int n)
{
  DBG_PROG_START
  frei_ = false;

  std::vector<std::string> texte, namen;

  texte.push_back(_("CIE *L"));
  texte.push_back(_("CIE *a"));
  texte.push_back(_("CIE *b"));

  std::vector<double> p;
  std::vector<float>  f;
  DBG_PROG_V( n <<" "<< profile.size() )
  DBG_PROG_V( profile[n]->filename() )

  if(profile.size() > n &&
     profile.aktuell() == n &&
     profile[n]->hasMeasurement() &&
     profile[n]->getMeasurement().hasXYZ() )
    {
      DBG_PROG
      messwertLese(n, p,f,namen);
    }

  bool ncl2_profil = profile[n]->hasTagName("ncl2");

  // benannte Farben darstellen
  if( profile.size() > n && ncl2_profil )
  {
    DBG_PROG
    farbenLese(n, p,f);
  }

  bool neues_netz = false;
  if( n >= (int)icc_betrachter->DD_histogram-> dreiecks_netze.size() )
    neues_netz = true;

  if(p.size())
    icc_betrachter->DD_histogram->hineinPunkte( p, f, namen, texte );

  if((int)icc_betrachter->DD_histogram-> dreiecks_netze .size() <= n)
    icc_betrachter->DD_histogram-> dreiecks_netze .resize( n + 1 );

  std::vector<ICCnetz> *netz = &icc_betrachter->DD_histogram->dreiecks_netze;
  DBG_PROG_V( icc_betrachter->DD_histogram-> dreiecks_netze.size() <<" "<< n )


  if( profile.size() > n && !ncl2_profil )
    netzLese(n, netz);

  DBG_PROG_V( n <<" "<< netz->size() <<" "<< ncl2_profil )

  if(netz->size() && neues_netz)
  {
    if((n == 0 && ncl2_profil)
    || (n == 1 && histogramModus())
    || profile.size() == 1 )
    {
      (*netz)[n].transparenz = 0.25;
      (*netz)[n].grau = false;
    } else {
      (*netz)[n].transparenz = 0.3;
      (*netz)[n].grau = true;
    }

    icc_betrachter->DD_histogram->achsNamen( texte );
  }
  if(ncl2_profil)
    icc_betrachter->DD_histogram->dreiecks_netze[n].transparenz = 1.0;


  frei_ = true;
  DBG_PROG_ENDE
}

void
ICCexamin::histogram ()
{
  DBG_PROG_START
  frei_ = false;


  for(int i = 0; i < profile.size(); ++i)
    histogram(i);

  /*if(icc_betrachter->DD_histogram -> dreiecks_netze.size())
    icc_betrachter->DD_histogram ->
      dreiecks_netze [icc_betrachter->DD_histogram->dreiecks_netze.size()-1]
        . transparenz = 0.7;*/

  DBG_PROG_V( profile.size() )

  frei_ = true;
  DBG_PROG_ENDE
}

void
ICCexamin::histogramModus (int profil)
{
  DBG_PROG_START
  frei_ = false;

  histogram_modus_ = false;
  if(profile.size() && profile.profil()->hasTagName("ncl2"))
    histogram_modus_ = true;

  DBG_PROG_V( histogram_modus_ )

  frei_ = true;
  DBG_PROG_ENDE
}


