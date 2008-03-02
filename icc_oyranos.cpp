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
#endif
#include "icc_oyranos.h"
#include "icc_utils.h"

Oyranos oyranos;


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
        v_block->name != profil_name )
    { 
        v_block->name = profil_name;

        int size = oyGetProfileSize ( profil_name );
        DBG_PROG_V( size )
        if (size)
        { block = (char*)oyGetProfileBlock( profil_name, &size);
          if( oyCheckProfileMem( block, size, 0 ) )
            WARN_S ( _("Profil konnte nicht geladen werden,") )
          else {
            DBG_PROG_V( (int)block <<"|"<< size )
            v_block->lade(block, size);
          }
        }
    }
  }
  
  DBG_NUM_S( "Standard " OY_DEFAULT_LAB_PROFILE " Profil = "<< lab_.name <<" "<< lab_.size() <<"\n" )

  
  #endif
  DBG_PROG_ENDE
}

void
Oyranos::moni_test_ ()
{
  DBG_PROG_START
  Speicher *v_block = &moni_;
  char* block;
  #if HAVE_OY
  if( !v_block->size() )
  { DBG_PROG_V( v_block->size() )
    const char *display_name = 0;
    char* profil_name =
      oyGetMonitorProfileName (display_name);
    DBG_PROG_V( (int)profil_name << profil_name )
    if( profil_name &&
        v_block->name != profil_name )
    { 
        v_block->name = profil_name;

        int size = oyGetProfileSize ( profil_name );
        DBG_PROG_V( size )
        if (size)
        { block = (char*)oyGetProfileBlock( profil_name, &size);
          if( oyCheckProfileMem( block, size, 0 ) )
            WARN_S ( _("Profil konnte nicht geladen werden,") )
          else {
            DBG_PROG_V( (int)block <<"|"<< size )
            v_block->lade(block, size);
          }
        }
    }
  }
  
  DBG_NUM_S( "Monitorprofil = "<< moni_.name <<" "<< moni_.size() <<"\n" )

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
        v_block->name != profil_name )
    { 
        v_block->name = profil_name;

        int size = oyGetProfileSize ( profil_name );
        DBG_PROG_V( size )
        if (size)
        { block = (char*)oyGetProfileBlock( profil_name, &size);
          if( oyCheckProfileMem( block, size, 0 ) )
            WARN_S ( _("Profil konnte nicht geladen werden,") )
          else {
            DBG_PROG_V( (int)block <<"|"<< size )
            v_block->lade(block, size);
          }
        }
    }
  }

  DBG_NUM_S( "Standard " OY_DEFAULT_RGB_PROFILE " Profil = "<< rgb_.name <<" "<< rgb_.size() <<"\n" )
  #endif
  DBG_PROG_ENDE
}

void
Oyranos::cmyk_test_ ()
{
  DBG_PROG_START
  Speicher *v_block = &cmyk_;
  char* block;
  #if HAVE_OY
  if( !v_block->size() )
  { DBG_PROG_V( v_block->size() )
    char* profil_name = oyGetDefaultCmykProfileName();
    DBG_PROG_V( (int)profil_name << oyGetDefaultCmykProfileName() )
    if( profil_name &&
        v_block->name != profil_name )
    { 
        v_block->name = profil_name;

        int size = oyGetProfileSize ( profil_name );
        DBG_PROG_V( size )
        if (size)
        { block = (char*)oyGetProfileBlock( profil_name, &size);
          if( oyCheckProfileMem( block, size, 0 ) )
            WARN_S ( _("Profil konnte nicht geladen werden,") )
          else {
            DBG_PROG_V( (int)block <<"|"<< size )
            v_block->lade(block, size);
          }
        }
    }
  }

  DBG_NUM_S( "Standard " OY_DEFAULT_CMYK_PROFILE " Profil = "<< cmyk_.name <<" "<< cmyk_.size() <<"\n" )
  #endif
  DBG_PROG_ENDE
}

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

  #if 0
  KeySet myConfig;
  ksInit(&myConfig);
  kdbOpen();

  int rc = kdbGetChildKeys("user/sw/oyanos/paths", &myConfig, KDB_O_RECURSIVE);
	
  // Close the Key database 
  kdbClose();

  Key *current;
	
  for (current=myConfig.start; current; current=current->next) {
    char keyName[200];
    char value[300];
		
    keyGetFullName(current,keyName,sizeof(keyName));
    keyGetString(current,value,sizeof(value));
		
    printf("Key %s was %s. ", keyName, value);
		
    // Add "- modified" to the end of the string
    //strcat(value,"- modified");
    //sprintf (value, pfad);
		
    // change the key value
    keySetString(current,value);
		
    // reget it, just as an example
    keyGetString(current,value,sizeof(value));
		
    printf("Now is %s\n", value);
  }

  kdbClose();
  #endif
  #endif
}


