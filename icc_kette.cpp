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
#include "icc_examin.h"

#if USE_PTHREAD
  #include <pthread.h>
#else
  #if HAVE_FLTK
    #include <FL/Fl.H>
  #endif
#endif

ICCkette profile;

ICCkette::ICCkette  ()
{
  aktuelles_profil_ = -1;
  #if USE_PTHREAD
  waechter(this);
  #else
  Fl::add_timeout( 0.4, /*(void(*)(void*))*/waechter ,(void*)this);
  #endif
}

bool
ICCkette::oeffnen (const Speicher & prof, int pos)
{ DBG_PROG_START
  bool erfolgreich = false;

  // Ist das Profile eventuell schon geladen? -> Abbruch
  for(unsigned int i = 0; i < profile_.size(); ++i)
    if(prof.name() == profile_[i].filename())
      return false;

  if(!prof.size())
    return false;

  // Laden TODO: test auf Existenz der Datei (oyranos?)
  if (pos >= 0 &&
      pos < (int)profile_.size() )
  {
    profile_.insert (profile_.begin()+pos, ICCprofile());
    profile_[pos].load(prof);
    DBG_PROG_V( profile_[pos].size() )
    DBG_PROG_V( profile_[pos].filename() )
    DBG_PROG_V( prof.name() )
    profilnamen_.insert (profilnamen_.begin()+pos, prof.name() );
    aktiv_.insert (aktiv_.begin()+pos, true);
    profil_mzeit_.insert (profil_mzeit_.begin()+pos, (double)prof.zeit() );
    /*Modell::*/benachrichtigen( pos );
  } else {
    profile_.resize(profile_.size()+1);
    profile_[profile_.size()-1].load(prof);
    profilnamen_.push_back( prof.name() );
    aktiv_.push_back(true);
    profil_mzeit_.push_back( (double)prof.zeit() );
    /*Modell::*/benachrichtigen( profile_.size()-1 );
  }


  if( profile_.size() ) {
    aktuelles_profil_ = 0;
    erfolgreich = true;
  } else { DBG_PROG
    icc_examin_ns::status_info(_("File not loaded!"));
  }

  DBG_PROG_ENDE
  return erfolgreich;
}

bool
ICCkette::oeffnen (std::string dateiname, int pos)
{ DBG_PROG_START
  bool erfolgreich = false;
  DBG_PROG_V( dateiname <<" "<< pos )
  // Ist das Profile eventuell schon geladen? -> Abbruch
  for(unsigned int i = 0; i < profile_.size(); ++i)
    if(dateiname == profile_[i].filename()) {
      DBG_PROG_S( _("File allready loaded")<< i <<": "<< dateiname )
      return false;
    }

  // Laden TODO: test auf Existenz der Datei (oyranos?)
  if (pos >= 0 && pos < (int)profile_.size()) {
    profile_.insert (profile_.begin()+pos, ICCprofile());
    profile_[pos].load(dateiname.c_str());
    DBG_PROG_V( dateiname )
    profilnamen_.insert (profilnamen_.begin()+pos, dateiname );
    aktiv_.insert (aktiv_.begin()+pos, true);
    profil_mzeit_.insert (profil_mzeit_.begin()+pos,
                            holeDateiModifikationsZeit( dateiname.c_str() ));
  } else {
    profile_.resize(profile_.size()+1);
    profile_[profile_.size()-1].load(dateiname.c_str());
    profilnamen_.push_back( dateiname );
    aktiv_.push_back(true);
    profil_mzeit_.push_back(holeDateiModifikationsZeit( dateiname.c_str() ));
    pos = profile_.size()-1;
  }


  if( profile_.size() ) {
    aktuelles_profil_ = 0;
    erfolgreich = true;
  } else { DBG_PROG
    icc_examin_ns::status_info(_("File not loaded!"));
  }

  /*Modell::*/benachrichtigen( pos );

  DBG_PROG_ENDE
  return erfolgreich;
}

bool
ICCkette::oeffnen (std::vector<std::string> dateinamen)
{ DBG_PROG_START
  bool erfolgreich = false;

  profile_.clear();
  profilnamen_.clear();
  aktiv_.clear();
  profil_mzeit_.clear();

  for (unsigned int i = 0; i < dateinamen.size(); ++i)
  {
    erfolgreich = oeffnen (dateinamen[i], -1);
    DBG_PROG_V( dateinamen[i] )
    icc_examin_ns::fortschritt (1.0);
    icc_examin_ns::fortschritt (1.1);
  }

  if( profile_.size() )
    aktuelles_profil_ = 0;

  erfolgreich = true;
  DBG_PROG_ENDE
  return erfolgreich;
}

static bool erstes_mal = true;
void
ICCkette::waechter (void* zeiger)
{
  //DBG_PROG_START
  // warte, starte einen pthread , teste alle Profile - ende 
  // Es entsteht eine Kette von threads. Fällt einer aus
  // ist die Überwachung beendet. - etwas labil vielleicht

  ICCkette* obj = (ICCkette*) zeiger;

  if(erstes_mal)
    erstes_mal = false;
  else
  #if USE_PTHREAD
    sleep(1);
  #endif
    ;

  #if USE_PTHREAD
  pthread_t p_t;
  int fehler = pthread_create(&p_t, NULL, &waechter, (void *)zeiger);
  if( fehler == EAGAIN)
  {
    WARN_S( _("Wächter Thread nicht gestartet Fehler: ")  << fehler );
  } else
  if( fehler == PTHREAD_THREADS_MAX )
  {
    WARN_S( _("zu viele Wächter Threads Fehler: ") << fehler );
  } else
  if( fehler != 0 )
  {
    WARN_S( _("unbekannter Fehler beim Start eines Wächter Threads Fehler: ") << fehler );
  }
  #else
  Fl::add_timeout( 0.4, /*(void(*)(void*))*/waechter ,(void*)zeiger);
  #endif

  for(unsigned i = 0; i < obj->profilnamen_.size(); ++i)
  {
    DBG_MEM_V( obj->profilnamen_[i] );
    double m_zeit = holeDateiModifikationsZeit( obj->profilnamen_[i].c_str() );
    DBG_MEM_V( m_zeit )
    if( m_zeit &&
        obj->aktiv_[i] &&
        obj->profil_mzeit_[i] != m_zeit
  #if USE_PTHREAD
     && icc_examin->frei()
  #endif
      )
    {
      obj->profile_[i].load( obj->profilnamen_[i] );
      obj->/*Modell::*/benachrichtigen( i );
      obj->profil_mzeit_[i] = m_zeit;
    }
  }

  //DBG_PROG_ENDE
  #if USE_PTHREAD
  return 0;
  #endif
}




