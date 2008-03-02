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

#include "icc_utils.h"
#include "icc_kette.h"
#include "icc_info.h"
#include "icc_examin.h"
#include "icc_helfer.h"


#if USE_THREADS
# include "threads.h"
# if HAVE_FLTK
#   include <FL/Fl.H>
# endif
#endif

ICCkette profile;

ICCkette::ICCkette  ()
{ DBG_PROG_START
  frei_ = true;
  DBG_PROG_ENDE
}

void
ICCkette::init ()
{ DBG_PROG_START
  aktuelles_profil_ = -1;
# if USE_THREADS
  // Es gibt drei threads.
  // Der erste Neben-thread started eine while Schleife zum Beobachten
  // der göffneten Dateien.
  // Im Haupthread laeuft ICCexamin.
  // Ein weiterer Thread übernimmt das Laden von neuen Daten.

  static Fl_Thread fl_t;
  int fehler = fl_create_thread( fl_t, &waechter, (void *)this );
# if HAVE_PTHREAD_H
  DBG_THREAD_V( fl_t <<" "<< pthread_self () )
  icc_thread_liste[THREAD_WACHE] = fl_t; DBG_THREAD
# endif
  if(!fehler)
    DBG_PROG_S( "neuer Thread" );

  if( fehler == EAGAIN)
  {
    WARN_S( _("Waechter Thread nicht gestartet Fehler: ")  << fehler );
  } else
# if !APPLE && !WIN32 && PTHREAD_THREADS_MAX
  if( fehler == PTHREAD_THREADS_MAX )
  {
    WARN_S( _("zu viele Waechter Threads Fehler: ") << fehler );
  } else
# endif
  if( fehler != 0 )
  {
    WARN_S( _("unbekannter Fehler beim Start eines Waechter Threads Fehler: ") << fehler );
  }

# else
  Fl::add_timeout( 0.4, /*(void(*)(void*))*/waechter ,(void*)this);
# endif
  DBG_PROG_ENDE
}

bool
ICCkette::einfuegen (const Speicher & prof, int pos)
{ DBG_PROG_START
  bool erfolg = false;

# if 0
  if(!frei())
  {
    int erfolg = false;
    while(!erfolg) {
      if(frei()) {
        erfolg = true;
      } else
        // kurze Pause 
        icc_examin_ns::sleep(0.05);
    }
  }
# endif
  while(!frei()) icc_examin_ns::sleep(0.05); frei(false);

  // Ist das Profile eventuell schon geladen? -> Abbruch
  /*
  for(unsigned int i = 0; i < profile_.size(); ++i)
    if((icc_examin->erneuern() != (int)i) &&
       prof.name() == profile_[i].filename()) {
      DBG_S( "- abgebrochen" )
      frei(true);
      return erfolg;
    }
  */
  if(!prof.size()) {
    frei(true);
    return false;
  }

  DBG_PROG_V( pos )

  // Laden TODO: test auf Korrektheit des Profiles (oyranos?)
  if (pos < 0 ||
      pos >= (int)profile_.size() )
  {
    pos = profile_.size(); DBG_PROG
    //profile_.resize (profile_.size()+1 ); DBG_PROG
    profile_.push_back( ICCprofile() ); DBG_PROG
    for( unsigned int i = 0; i < profile_.size(); ++i)
      profile_[i].measurementReparent();
    profilnamen_.resize (profilnamen_.size()+1 );
    aktiv_.resize (aktiv_.size()+1 );
    profil_mzeit_.resize (profil_mzeit_.size()+1 );
    DBG_PROG_V( pos<<" "<<prof.name() )
  }
  DBG_PROG_V( pos )

  //profile_[pos] = ICCprofile();
  ICCprofile::ICCDataType type = profile_[pos].load(prof);
  profile_[pos].filename( prof.name().c_str() );
  DBG_PROG_V( type )
  if(type == ICCprofile::ICCmeasurementDATA && pos != 0)
  {
    //ICCmeasurement m;
    //m.load( profile.profil() , (const char*) prof, prof.size() );
    DBG_PROG_V( profile.profil()->hasMeasurement() )
  }
  DBG_PROG_V( profile_[pos].size() )
  DBG_PROG_V( profile_[pos].filename() )
  DBG_PROG_V( prof.name() )
  std::string name = _("noName");
  if(prof.name().size())
    name = prof.name();
  else
    if(profile_[pos].hasTagName("desc"))
      name = profile_[pos].getTagText( profile_[pos].getTagByName("desc"))[0];
  profilnamen_[pos] = name ;

  aktiv_[pos] = true;
  profil_mzeit_[pos] = holeDateiModifikationsZeit( name.c_str() );
  DBG_PROG

  if( profile_.size() ) {
    aktuelles_profil_ = 0;
    DBG_PROG_V( aktuell() )
    erfolg = true;
  } else { DBG_PROG
    icc_examin_ns::status_info(_("File not loaded!"));
  }


  frei(true);
  //icc_examin_ns::lock(__FILE__,__LINE__);
  /*Modell::*/benachrichtigen( pos );
  //icc_examin_ns::unlock(icc_examin, __FILE__,__LINE__);
  DBG_PROG_ENDE
  return erfolg;
}

#if USE_THREADS
void*
#else
void
#endif
ICCkette::waechter (void* zeiger)
{
  // TODO
  //icc_examin_ns::sleep(1.0);
  //cout << (int*)level_PROG_ << endl;
  DBG_PROG_START
  ICCkette* obj = (ICCkette*) zeiger;
  // Haupt Thread freigeben
  //icc_examin_ns::unlock(0,__FILE__,__LINE__);

  while(1)
  {
    for(unsigned i = 0; i < obj->profilnamen_.size(); ++i)
    {
      const char* name = obj->profilnamen_[i].c_str();
      DBG_MEM_V( name );
      double m_zeit = holeDateiModifikationsZeit( name );
      DBG_MEM_V( m_zeit <<" "<< obj->profil_mzeit_[i] << name )
      if( m_zeit &&
          obj->aktiv_[i] &&
          obj->profil_mzeit_[i] != m_zeit
#      if USE_THREADS
       && obj->frei()
#      endif
        )
      {
        DBG_MEM_V( obj->profil_mzeit_[i] )
        if( obj->profil_mzeit_[i] != 0 ) {
          // lade in LADEN und warte auf Ergebnis
          icc_examin->erneuern(i);
        }
        obj->profil_mzeit_[i] = m_zeit;
      }
    }
    DBG_THREAD
    icc_examin_ns::sleep(1.0/10.0);
  }

# if USE_THREADS
  Fl::wait();
# endif

  DBG_PROG_ENDE
  return
# if USE_THREADS
         0
# endif
          ;
}




