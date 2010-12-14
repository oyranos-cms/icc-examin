/*
 * ICC Examin ist eine ICC Profil Betrachter
 * 
 * Copyright (C) 2005-2008  Kai-Uwe Behrmann 
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
 * the chain of profiles.
 * 
 */

#include "icc_utils.h"
#include "icc_kette.h"
#include "icc_info.h"
#include "icc_examin.h"
#include "icc_helfer.h"


#if USE_THREADS
# include "threads.h"
#endif

#if HAVE_FLTK
# include <FL/Fl.H>
#endif

ICCkette profile;

ICCkette::ICCkette  ()
{ DBG_PROG_START
  DBG_PROG_ENDE
}

void
ICCkette::init ()
{ DBG_PROG_START
  aktuelles_profil_ = -1;
# if USE_THREADS
  // There are three basic threads for ICC Examin.
  // In the main thread runs ICCexamin.
  // The next one starts a while loop to observe the opened files
  // A additional thread loads new files.

  int fehler = fl_create_thread( getThreadId(THREAD_WACHE), &waechter, (void *)this );
  if(!fehler)
    DBG_PROG_S( "new thread" );

  if( fehler == EAGAIN)
  {
    WARN_S( "Observer thread not started. Error: "  << fehler );
  } else
# if !APPLE && !WIN32 && PTHREAD_THREADS_MAX
  if( fehler == (int)PTHREAD_THREADS_MAX )
  {
    WARN_S( "Too many observer threads. Error: " << fehler );
  } else
# endif
  if( fehler != 0 )
  {
    WARN_S( "Unknown error at start of observer threads. Error: " << fehler );
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
  frei(false);

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

  int p = pos;

  // Load TODO: test for correctnes of a profile (oyranos?)
  if (pos < 0 ||
      pos >= (int)profile_.size() )
  {
    p = (int)profile_.size(); DBG_PROG
    //profile_.resize (profile_.size()+1 ); DBG_PROG
    profile_.push_back( ICCprofile() ); DBG_PROG
    for( unsigned int i = 0; i < profile_.size(); ++i)
      profile_[i].measurementReparent();
    profilnamen_.resize (profilnamen_.size()+1 );
    aktiv_.resize (aktiv_.size()+1 );
    profil_mzeit_.resize (profil_mzeit_.size()+1 );
    std::string prof_name = prof.name();
    DBG_PROG_V( p<<" "<<prof_name )
  }
  DBG_PROG_V( pos )

  //profile_[p] = ICCprofile();
# ifdef DEBUG
  ICCprofile::ICCDataType type =
#endif
  profile_[p].load(prof);
  profile_[p].filename( prof.name().c_str() );
  DBG_PROG_V( type )
# ifdef DEBUG
  ICCprofile::ICCDataType dtype = profile_[0].data_type;
#endif
  DBG_PROG_V( dtype )

  int extra_benachrichtigen = -1;
  // measurement data should be appended after the first profile, as normal
  if(profile_[0].data_type == ICCprofile::ICCprofileDATA)
  {
    //ICCmeasurement m;
    //m.load( profile.profil() , (const char*) prof, prof.size() );
    for (unsigned int i = 1; i < profile_.size(); ++i)
    {
      if(profile_[i].data_type == ICCprofile::ICCmeasurementDATA &&
         i == (unsigned int) aktuell() + 1)
      {
        ICCmeasurement & m = profile_[0].getMeasurement();
        int tag_n = profile_[i].getTagIDByName( "targ" );
        ICCtag & tag = profile_[i].getTag( tag_n );
        m.load( &profile_[0], tag );
        if( !profile_[0].hasTagName( "targ" ) )
        {
          profile_[0].addTag( tag );

          // the profile is done: erase it
          unsigned int c = 0;
          for(int j = 0; j < (int)profile_.size(); ++j)
          {
            if(i == c)
            {
              profile_.erase ( &profile_[i] );
              profilnamen_.erase ( &profilnamen_[i] );
              aktiv_.erase( &aktiv_[i] );
              profil_mzeit_.erase( &profil_mzeit_[i] );
              break;
            }
          }
          // we dont add the measurement to the profiles list
          return erfolg;
        }
        extra_benachrichtigen = 0;
        continue;
      }
    }
  }
  DBG_PROG_V( profile_[p].size() )
  DBG_PROG_V( profile_[p].filename() )
  DBG_PROG_V( prof.name() )
  std::string name = _("noName");
  if(prof.name().size())
    name = prof.name();
  else
    if(profile_[p].hasTagName("desc"))
      name = profile_[p].getTagText( profile_[p].getTagIDByName("desc"))[0];
  profilnamen_[p] = name ;

  aktiv_[p] = true;
  profil_mzeit_[p] = holeDateiModifikationsZeit( name.c_str() );
  DBG_PROG

  if( profile_.size() ) {
    aktuelles_profil_ = 0;
    DBG_PROG_V( aktuell() )
    erfolg = true;
  } else { DBG_PROG
    icc_examin_ns::status_info(_("File not loaded!"), 0);
  }


  frei(true);
  //icc_examin_ns::lock(__FILE__,__LINE__);
  //if(pos >= 0)
  {
    benachrichtigen( p );
    if( extra_benachrichtigen >= 0 )
      benachrichtigen( extra_benachrichtigen );
  }
  //else
    ///*Modell::*/benachrichtigen( p );
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
  registerThreadId( iccThreadSelf(), THREAD_WACHE );

  DBG_PROG_START
  ICCkette* obj = (ICCkette*) zeiger;
  // Haupt Thread freigeben
  //icc_examin_ns::unlock(0,__FILE__,__LINE__);

# if USE_THREADS
  while(1)
# endif
  {
    if(icc_examin_ns::laeuft())
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
          // load in LADEN and wait for result
          icc_examin->erneuern(i);
          DBG_PROG_S(name)
        }
        obj->profil_mzeit_[i] = m_zeit;
      }
    }
    DBG_THREAD
    icc_examin_ns::sleep(1.0/10.0);
  }

# if USE_THREADS
  icc_examin_ns::wait( 0.0, true );
# endif

  DBG_PROG_ENDE
  return
# if USE_THREADS
         0
# endif
          ;
}




