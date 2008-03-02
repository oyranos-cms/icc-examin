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
 * Der CMS Sortierer.
 * 
 */

// Date:      25. 11. 2004


#ifdef HAVE_OY
#include "oyranos/oyranos.h"
#include "oyranos/oyranos_monitor.h"
using namespace oyranos;
#endif
#include "icc_oyranos.h"
#include "icc_utils.h"


Oyranos icc_oyranos;


        /* Konzepte:
         *   o - oyInit() initialisiert die Bibliothek und erlaubt das lesen von
         *       Zeigern
         *     - oyQuit() beräumt all diese
         *   o ein Objekt pro Profil (void* cmsOpen(...) ; cmsClose(void*))
         *       dies bleibt ein CMM Architektur vorbehalten
         *   o ein C++ Wrapper für liboyranos wie mit Oyranos oyranos
         *       oyranos als C++ Header exportieren
         *   o alles dem Benutzer überlassen (C free())
         *       im Prototyp hier   
         *   o sich auf reine Namensnennung beschränken --
         */


Oyranos::Oyranos()
{
  DBG_PROG_START
  #if HAVE_OY
  oyOpen();
  #endif
  DBG_PROG_ENDE
}

Oyranos::~Oyranos()
{
  DBG_PROG_START
  #if HAVE_OY
  oyClose();
  #endif
  DBG_PROG_ENDE
}

// nur in icc_oyranos.cpp zu verwendende Typen
typedef std::map<std::string,Speicher> Prof_map;
typedef std::map<std::string,Speicher>::iterator Prof_mapIt;
typedef std::pair<std::string,Speicher> Prof_Map_elem;
typedef std::pair<Prof_map::const_iterator,bool> Prof_mapIt_bool;

bool
Oyranos::profil_test_ (const char* profil_name)
{
  DBG_PROG_START
  #if HAVE_OY

  if(profil_name && strlen(profil_name))
  {
    Prof_mapIt cmp = pspeicher_.find( profil_name );
    if( cmp == pspeicher_.end() )
    {
      bool fehler = oyCheckProfile( profil_name, 0 );
      if( !fehler )
      {
        // leeren Block einfügen
        Prof_Map_elem teil (profil_name, Speicher());
        Prof_mapIt_bool erg = pspeicher_ .insert( teil );
        if( erg.second = true )
        {
          // Referenz auf Block holen
          Speicher *v_block = &pspeicher_[profil_name];
          *v_block = profil_name;
          int size;
          char* block = (char*)oyGetProfileBlock( profil_name, &size);
          DBG_PROG_V( (int)block <<"|"<< size )
          v_block->lade(block, size);
        }
      }
    } else // Profil in Liste
    {
      Speicher *v_block = &pspeicher_[profil_name];
      // ...
    }
  }
  
  DBG_NUM_S( "Standard " OY_DEFAULT_LAB_PROFILE " Profil = "<< *lab_ <<" "<< lab_.size() <<"\n" )

  #endif
  DBG_PROG_ENDE
}

void
Oyranos::lab_test_ ()
{
  DBG_PROG_START
  Speicher *v_block = &lab_;
  char* block;
  #if HAVE_OY
  if( !v_block->size() )
  { DBG_PROG_V( v_block->size() )
    char* profil_name = oyGetDefaultLabProfileName();
    DBG_PROG_V( (int)profil_name << oyGetDefaultLabProfileName() )
    if( profil_name &&
        *v_block != profil_name )
    { 
        *v_block = profil_name;

        int size = oyGetProfileSize ( profil_name );
        DBG_PROG_V( size )
        if (size)
        { block = (char*)oyGetProfileBlock( profil_name, &size);
          if( oyCheckProfileMem( block, size, 0 ) )
            WARN_S ( _("Profil konnte nicht geladen werden") )
          else {
            DBG_PROG_V( (int)block <<"|"<< size )
            v_block->lade(block, size);
          }
        }
    }
  }
  
  DBG_NUM_S( "Standard " OY_DEFAULT_LAB_PROFILE " Profil = "<< *lab_ <<" "<< lab_.size() <<"\n" )

  
  #endif
  DBG_PROG_ENDE
}

void
Oyranos::moni_test_ ()
{
  DBG_PROG_START
  #if HAVE_OY
  {
    char* block;
    const char *display_name = 0;
    char* profil_name =
     oyGetMonitorProfileName (display_name);
    Speicher v_block = moni_;
      DBG_MEM_V( v_block.size() )
      DBG_PROG_V( (int*)profil_name << profil_name )
    if( profil_name &&
        v_block != profil_name )
    { 
        int size = oyGetProfileSize ( profil_name );
          DBG_MEM_V( size )
        if (size)
        {
          block = (char*)
           oyGetProfileBlock( profil_name, &size);
          if( oyCheckProfileMem( block, size, 0 ) )
            WARN_S ( _("Profil konnte nicht geladen werden") )
          else {
              DBG_MEM_V( (int*)block <<"|"<< size )
            v_block.lade(block, size);
          }
        }
        v_block = profil_name;

      if(profil_name) free(profil_name);
        DBG_MEM
    }
  }
  
  DBG_NUM_S( "Monitorprofil = "<< *moni_ <<" "<< moni_.size() <<"\n" )

  #endif
  DBG_PROG_ENDE
}

void
Oyranos::rgb_test_ ()
{
  DBG_PROG_START
  Speicher *v_block = &rgb_;
  char* block;
  #if HAVE_OY
  if( !v_block->size() )
  { DBG_PROG_V( v_block->size() )
    char* profil_name = oyGetDefaultRGBProfileName();
    DBG_PROG_V( (int)profil_name << oyGetDefaultRGBProfileName() )
    if( profil_name &&
        *v_block != profil_name )
    { 
        *v_block = profil_name;

        int size = oyGetProfileSize ( profil_name );
        DBG_PROG_V( size )
        if (size)
        { block = (char*)oyGetProfileBlock( profil_name, &size);
          if( oyCheckProfileMem( block, size, 0 ) )
            WARN_S ( _("Profil konnte nicht geladen werden") )
          else {
            DBG_PROG_V( (int)block <<"|"<< size )
            v_block->lade(block, size);
          }
        }
    }
  }

  DBG_NUM_S( "Standard " OY_DEFAULT_RGB_PROFILE " Profil = "<< *rgb_ <<" "<< rgb_.size() <<"\n" )
  #endif
  DBG_PROG_ENDE
}

extern int oy_debug;
void
Oyranos::cmyk_test_ ()
{
  DBG_PROG_START
  //oy_debug = 1;
  Speicher *v_block = &cmyk_;
  char* block;
  #if HAVE_OY
  if( !v_block->size() )
  { DBG_PROG_V( v_block->size() )
    char* profil_name = oyGetDefaultCmykProfileName();
    if(profil_name) DBG_PROG_V( profil_name )
    else            DBG_PROG_V( (int)profil_name )

    if( profil_name &&
        *v_block != profil_name )
    { 
        *v_block = profil_name;

        int size = oyGetProfileSize ( profil_name );
        DBG_PROG_V( size  )
        if (size)
        { block = (char*)oyGetProfileBlock( profil_name, &size);
          if( oyCheckProfileMem( block, size, 0 ) )
            WARN_S ( _("Profil konnte nicht geladen werden") )
          else {
            v_block->lade(block, size);
            DBG_PROG_V( (int)block <<"|"<< size <<" "<<(int) (*cmyk_) )
          }
        }
    }
  }

  if(cmyk_.size())
    DBG_NUM_S( "Standard " OY_DEFAULT_CMYK_PROFILE " Profil = "<< *cmyk_ <<" "<< cmyk_.size() <<"\n" )
  #endif
  //oy_debug = 0;
  DBG_PROG_ENDE
}

#ifdef HAVE_X
#include <X11/Xlib.h>
#endif
#ifdef HAVE_FLTK
//#include <FL/x.H>
#endif

int
Oyranos::setzeMonitorProfil (const char* profil_name )
{
  DBG_PROG_START
  int fehler = false;

  DBG_PROG_V( profil_name )
  #if HAVE_OY
  const char *display_name=0;

  #ifdef HAVE_X
  static Display *display=0;

  #ifdef HAVE_FLTK
  if( !display )
    ;//display = fl_display;
  #endif
  if( !display )
    display = XOpenDisplay(0);

  display_name = XDisplayString( display );  // gehört X
  DBG_PROG_V( display_name <<" "<< strlen(display_name) )

  #ifndef HAVE_FLTK
    XCloseDisplay( display ); DBG_PROG
  #endif

  #endif

  fehler = oySetMonitorProfile( display_name, profil_name );

  char *neues_profil = oyGetMonitorProfileName( display_name );
  DBG_PROG_V( neues_profil )

  if (neues_profil) free (neues_profil);
  #endif

  DBG_PROG_ENDE
  return fehler;
}

#include "icc_vrml.h"
std::vector<ICCnetz>
Oyranos::netzVonProfil (const Speicher & p, int intent)
{
  DBG_PROG_START
  std::string vrml;
  const char* b = p;
  DBG_PROG_V( (int*)b )
  vrml = icc_create_vrml ( p,p.size(), intent );
  std::vector<ICCnetz> netz = extrahiereNetzAusVRML (vrml);
  DBG_PROG_ENDE
  return netz;
}



#if 0
const Speicher&
Oyranos::zeigTrafo           ( const char *profilA, int ein_bytes, int kanaeleA,
                               const char *profilB, int aus_bytes, int kanaeleB,
                               int intent, int optionen,
                               const char *proof_profil, int intent_p )
{
  DBG_PROG_START
  Speicher   *v_block = &cmyk_;
  const char *block;
  // Schlüssel erzeugen
  static char schluessel[1024];
  
  sprintf (schluessel, "%s-%d_%s-%d_%d", profilA, ein_bytes, kanaeleA,
                                         profilB, aus_bytes, kanaeleB,
                                         intent, optionen,
                                         proof_profil, intent_p);

  //trafos_.find(schluessel);
  #if HAVE_OY
  #endif
  DBG_PROG_ENDE
}
#endif

void
oyranos_pfade_loeschen()
{
}

void
oyranos_pfade_auffrischen()
{
}

void
oyranos_pfade_einlesen()
{
}

void
oyranos_pfad_dazu (char* pfad)
{
  #ifdef HAVE_OY

  #endif
}


