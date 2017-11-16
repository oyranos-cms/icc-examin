/*
 * ICC Examin ist eine ICC Profil Betrachter
 * 
 * Copyright (C) 2004-2014  Kai-Uwe Behrmann 
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
 * The CMS sorter.
 * 
 */

// Date:      25. 11. 2004


#include "icc_formeln.h"
#include "icc_oyranos.h"
#include "icc_profile.h"
#include "icc_utils.h"
#include "icc_fenster.h"
#include "icc_info.h"
#include "config.h"
using namespace icc_examin_ns;

#include <oyranos.h>
#include <oyranos_devices.h>

#include "oyranos_display_helpers.c"

#if defined(__APPLE__)
#include <Carbon/Carbon.h>
#include <IOKit/Graphics/IOGraphicsLib.h>
#endif

#if defined(HAVE_X) && !defined(__APPLE__)
#include <X11/Xlib.h>
#include <X11/extensions/Xfixes.h>
#include <X11/Xcm/Xcm.h>
# ifdef HAVE_FLTK
# include <FL/x.H>
# endif
#endif



Oyranos icc_oyranos;


        /* concepts:
         *   o - oyInit() initialisiert die Bibliothek und erlaubt das lesen von
         *       Zeigern
         *     - oyQuit() beraeumt all diese
         *   o one object per profile (void* cmsOpen(...) ; cmsClose(void*))
         *       this will wait until a CMM architecture is ready
         *   o a C++ wrapper for liboyranos as with Oyranos oyranos
         *       export oyranos with a C++ header
         *   o leave all to the user (C free())
         *       in prototyp here   
         *   o sich auf reine Namensnennung beschraenken --
         */


Oyranos::Oyranos()
{
  DBG_PROG_START
  oy_profile_from_flags = OY_NO_REPAIR;
  DBG_PROG_ENDE
}

Oyranos::~Oyranos()
{
  DBG_PROG_START
  DBG_PROG_ENDE
}

// local types in icc_oyranos.cpp
typedef std::map<std::string,Speicher> Prof_map;
typedef std::map<std::string,Speicher>::iterator Prof_mapIt;
typedef std::pair<std::string,Speicher> Prof_Map_elem;
typedef std::pair<Prof_map::const_iterator,bool> Prof_mapIt_bool;

void
Oyranos::clear()
{
    lab_.clear();
    moni_.clear();
    moni_native_.clear();
    rgb_.clear();
    cmyk_.clear();
    proof_.clear();
}

bool
Oyranos::profil_test_ (const char* profil_name)
{
  DBG_PROG_START
  bool fehler = false;

  if(profil_name && strlen(profil_name))
  {
    Prof_mapIt cmp = pspeicher_.find( profil_name );
    if( cmp == pspeicher_.end() )
    {
      fehler = oyCheckProfile( profil_name, 0 );
      if( !fehler )
      {
        // insert empty block
        Prof_Map_elem teil (profil_name, Speicher());
        Prof_mapIt_bool erg = pspeicher_ .insert( teil );
        if( erg.second == true )
        {
          // get referenz to block
          Speicher *v_block = &pspeicher_[profil_name];
          *v_block = profil_name;
          size_t size;
          char* block = (char*)oyGetProfileBlock( profil_name, &size, malloc);
          DBG_PROG_V( (int*)block <<"|"<< size )
          v_block->ladeUndFreePtr(&block, size);
        }
      }
    } else // profile in list
    {
      //Speicher *v_block = &pspeicher_[profil_name];
      // ...
    }
  }
  
  DBG_NUM_S( "Standard " OY_DEFAULT_ASSUMED_LAB_PROFILE " Profil = "<< *lab_ <<" "<< lab_.size() <<"\n" )

  return fehler;
  DBG_PROG_ENDE
}

void
Oyranos::lab_test_ ()
{
  DBG_PROG_START
  Speicher *v_block = &lab_;
  if( !v_block->size() )
  { DBG_PROG_V( v_block->size() )
    char* profil_name = oyGetDefaultProfileName( oyASSUMED_LAB, myAllocFunc );
    if(profil_name)
      DBG_PROG_V( (int*)profil_name << profil_name );
    if( profil_name &&
        *v_block != profil_name )
    { 
        *v_block = profil_name;

        size_t size = oyGetProfileSize ( profil_name );
        DBG_PROG_V( size )
        if (size)
        { char *block = (char*)oyGetProfileBlock( profil_name, &size, malloc);
          if( oyCheckProfileMem( block, size, 0 ) )
            WARN_S ( _("WARNING: Could not load corrupt or damaged profile.") )
          else {
            DBG_PROG_V( (intptr_t)block <<"|"<< size )
            v_block->ladeUndFreePtr(&block, size);
          }
        } else
          WARN_S(_("WARNING: Could not load profile."));
    }
  }
  
  DBG_NUM_S( "Standard " OY_DEFAULT_ASSUMED_LAB_PROFILE " Profil = "<< *lab_ <<" "<< lab_.size() <<"\n" )

  
  DBG_PROG_ENDE
}

#if defined(__APPLE__) && 0
typedef struct {
  char   *data;
  SInt32  size;
} refcon;

OSErr
MyFlattenProfileProc (
   SInt32 command, 
   SInt32 *size, 
   void *data, 
   void *refCon)
{
  // collect all parts
  if(*size)
  {
    refcon *ref = (refcon*) refCon;

    char* block = (char*)malloc(ref->size+*size);
    // old data
    if(ref->data && ref->size) {
      memcpy(block, ref->data, ref->size);
      free(ref->data);
    }
    // new data
    memcpy( &block[ref->size], data, *size );

    ref->data = block;
    ref->size += *size;
  }
  DBG_PROG_V(command<<" "<<*size)

  return 0;
}

OSErr
MyFlattenProfileProcSize (
   SInt32 command, 
   SInt32 *size, 
   void *data, 
   void *refCon)
{
  // collect all parts
  if(*size)
  {
    refcon *ref = (refcon*) refCon;
    ref->size += *size;
  }
  DBG_PROG_V(command<<" "<<*size)

  return 0;
}

int
oyGetProfileBlockOSX (CMProfileRef prof, char *block, size_t *size, oyAlloc_f allocateFunc)
{
  DBG_PROG_START
    CMProfileLocation loc;
    UInt32 locationSize = sizeof(CMProfileLocation);

    //CMError err = CMGetProfileLocation( prof, &loc );
    NCMGetProfileLocation(prof, &loc, &locationSize);
    switch(loc.locType)
    {
      case cmNoProfileBase:
             DBG_PROG_S("Das Monitorprofil ist ein temporaeres Profil.")
             break;
#if !__LP64__ && !TARGET_OS_WIN32
      case cmFileBasedProfile:
             DBG_PROG_S("Das Monitorprofil ist ein Datei Profil.")
             break;
      case cmHandleBasedProfile:
             DBG_PROG_S("Das Monitorprofil ist ein Haendling Profil.")
             break;
      case cmPtrBasedProfile:
             DBG_PROG_S("Das Monitorprofil ist ein Zeiger Profil.")
             break;
      case cmProcedureBasedProfile:
             DBG_PROG_S("Das Monitorprofil ist ein prozedurales Profil.")
             break;
#endif
      case cmPathBasedProfile:
             DBG_PROG_S("Das Monitorprofil ist ein Pfad Profil.")
             break;
      case cmBufferBasedProfile:
             DBG_PROG_S("Das Monitorprofil ist ein Speicherblock Profil.")
             break;
      default:
             DBG_PROG_S("kein Profil gefunden?")
             break;
    }

#if !__LP64__ && !TARGET_OS_WIN32
    refcon ref = {0,0};
    Boolean bol;
    // only the size
    if(*size == 0) {
      CMError err = CMFlattenProfile ( prof, 0, MyFlattenProfileProcSize, &ref, &bol);
      *size = ref.size;
      return err;
    }
    CMError err = CMFlattenProfile ( prof, 0, MyFlattenProfileProc, &ref, &bol);

    err = 0;
    Str255 str;
    ScriptCode code;
    CMGetScriptProfileDescription(prof, str, &code);
    DBG_PROG_V( (int)str[0] )
	if (prof) CMCloseProfile(prof);
    const unsigned char *profil_name = str; ++profil_name;

    if(ref.size && ref.data)
    {
        *size = ref.size;
        block = (char*) allocateFunc( *size );
        memcpy(block, ref.data, ref.size);
          DBG_MEM_V( size )
    } else
#endif

    if(loc.locType == cmBufferBasedProfile)
    {
        *size = loc.u.bufferLoc.size;
        block = (char*) allocateFunc( *size );
        memcpy(block, loc.u.bufferLoc.buffer, *size);
          DBG_PROG_V( *size )
    }

  DBG_PROG_ENDE
  return 0;
}

#endif

char*
changeScreenName_( const char *display_name, int screen )
{
  char *new_display_name = 0;

  if(!display_name)
    return NULL;

  if( (new_display_name = (char*) new char [strlen(display_name) +24]) == 0)
    return NULL;

  sprintf( new_display_name, "%s", display_name );

  char *ptr = strchr( new_display_name, ':' );
  char *ptr2 = 0;
  if( ptr )
  {
    ptr2 = strchr( ptr, '.' );
    if(ptr2)
      sprintf( ptr2, ".%d", screen );
    else
      sprintf( &ptr[strlen(ptr)], ".%d", screen );
  } else
    sprintf( &ptr[strlen(ptr)], ".%d", screen );

  return new_display_name;
}

void
Oyranos::moni_native_test_ (int x, int y)
{
  DBG_PROG_START
  size_t size = 0;
  Speicher v_block = moni_native_;
  oyProfile_s * oy_moni = 0;

  if(icc_examin_ns::zeitSekunden() > v_block.zeit() + 1.0)
  {
    oy_moni = oyMoni(x,y, 1);
    char* block = (char*) oyProfile_GetMem ( oy_moni, &size, 0, malloc );

    /* monitor name is expensive for non cached profiles */
    const char * vb = v_block;
    if(v_block.size() != size ||
       memcmp( vb, block, size ) != 0 )
    {
      if(block && size)
        v_block.ladeUndFreePtr(&block, size);
      else
        v_block.zeit( icc_examin_ns::zeitSekunden() );

      const char* oy_moni_name = oyProfile_GetFileName( oy_moni, 0 );

      if(oy_moni_name)
        v_block = oy_moni_name;
    }
    if(block && size) {
      free( block ); block = 0; size = 0;
    }

    oyProfile_Release( &oy_moni );
  }

  DBG_PROG_ENDE
  return;
}

void
Oyranos::moni_test_ (int x, int y)
{
  DBG_PROG_START
  size_t size = 0;
  Speicher v_block = moni_;
  oyProfile_s * oy_moni = 0;

  if(icc_examin_ns::zeitSekunden() > v_block.zeit() + 1.0)
  {
    oy_moni = oyMoni(x,y);
    char* block = (char*) oyProfile_GetMem ( oy_moni, &size, 0, malloc );

    /* monitor name is expensive for non cached profiles */
    const char * vb = v_block;
    if(v_block.size() != size ||
       memcmp( vb, block, size ) != 0 )
    {
      if(block && size)
        v_block.ladeUndFreePtr(&block, size);
      else
        v_block.zeit( icc_examin_ns::zeitSekunden() );

      const char* oy_moni_name = oyProfile_GetFileName( oy_moni, 0 );

      if(oy_moni_name)
        v_block = oy_moni_name;
    }
    if(block && size) {
      free( block ); block = 0; size = 0;
    }

    oyProfile_Release( &oy_moni );
  }

  DBG_PROG_ENDE
  return;
}

void
Oyranos::rgb_test_ ()
{
  DBG_PROG_START
  Speicher *v_block = &rgb_;
  if( !v_block->size() )
  { DBG_PROG_V( v_block->size() )
    const char* profil_name = oyGetDefaultProfileName( oyASSUMED_RGB, myAllocFunc );
    if(profil_name)
      DBG_PROG_V( (intptr_t)profil_name << profil_name );
    if( profil_name &&
        *v_block != profil_name )
    { 
        *v_block = profil_name;

        size_t size = oyGetProfileSize ( profil_name );
        DBG_PROG_V( size )
        if (size)
        { char *block = (char*)oyGetProfileBlock( profil_name, &size, malloc);
          if( oyCheckProfileMem( block, size, 0 ) )
            WARN_S(_("WARNING: Could not load profile."))
          else {
            DBG_PROG_V( (int*)block <<"|"<< size )
            v_block->ladeUndFreePtr(&block, size);
          }
        } else
          WARN_S(_("WARNING: Could not load profile."))
    }
  }

  if(rgb_.size())
    DBG_NUM_S( "Standard " OY_DEFAULT_ASSUMED_RGB_PROFILE " Profil = "<< *rgb_ <<" "<< rgb_.size() <<"\n" );
  DBG_PROG_ENDE
}


void
Oyranos::cmyk_test_ ()
{
  DBG_PROG_START
  Speicher *v_block = &cmyk_;
  if( !v_block->size() )
  { DBG_PROG_V( v_block->size() )
    char* profil_name = oyGetDefaultProfileName( oyASSUMED_CMYK, myAllocFunc );
    if(profil_name) {DBG_PROG_V( profil_name );
    } else {         DBG_PROG_V( (intptr_t)profil_name );}

    if( profil_name &&
        *v_block != profil_name )
    { 
        *v_block = profil_name;

        size_t size = oyGetProfileSize ( profil_name );
        DBG_PROG_V( size  )
        if (size)
        { char *block = (char*)oyGetProfileBlock( profil_name, &size, malloc);
          if( oyCheckProfileMem( block, size, 0 ) )
            WARN_S(_("WARNING: Could not load profile."))
          else {
            DBG_PROG_V( (int*)block <<"|"<< size )
            v_block->ladeUndFreePtr(&block, size);
          }
        } else
          WARN_S(_("WARNING: Could not load profile."));
    }
  }

  if(cmyk_.size())
    DBG_NUM_S( "Standard " OY_DEFAULT_ASSUMED_CMYK_PROFILE " Profil = "<< *cmyk_ <<" "<< cmyk_.size() <<"\n" );
  //oy_debug = 0;
  DBG_PROG_ENDE
}

void
Oyranos::proof_test_ ()
{
  DBG_PROG_START
  Speicher *v_block = &proof_;
  char* profil_name = NULL;

  profil_name = oyGetDefaultProfileName( oyPROFILE_PROOF, myAllocFunc);

  if( !v_block->size() )
  { DBG_PROG_V( v_block->size() )
    if(profil_name) {DBG_PROG_V( profil_name );
    } else {         DBG_PROG_V( (intptr_t)profil_name );}

    if( profil_name &&
        *v_block != profil_name )
    { 
        *v_block = profil_name;

        size_t size = oyGetProfileSize ( profil_name );
        DBG_PROG_V( size  )
        if (size)
        { char *block = (char*)oyGetProfileBlock( profil_name, &size, malloc);
          if( oyCheckProfileMem( block, size, 0 ) )
            WARN_S(_("WARNING: Could not load profile."))
          else {
            DBG_PROG_V( (int*)block <<"|"<< size )
            v_block->ladeUndFreePtr(&block, size);
          }
        } else
          WARN_S(_("WARNING: Could not load profile."));
    }
  }

  if(cmyk_.size())
    DBG_NUM_S( "Standard " OY_DEFAULT_PROOF_PROFILE " Profil = "<< *proof_ <<" "<< proof_.size() <<"\n" );
  //oy_debug = 0;
  DBG_PROG_ENDE
}

#if 0
char*
Oyranos::holeMonitorProfil (const char* display_name, size_t* size )
{
  DBG_PROG_START
  char* moni_profil = 0;
  *size = 0;

# if defined(HAVE_X) && !defined(__APPLE__)
  static Display *display=0;
  if(display_name)
    display = XOpenDisplay(display_name);

# ifdef HAVE_FLTK
  if( !display )
    display = fl_display;
# endif
  if( !display )
    display = XOpenDisplay(0);

  display_name = XDisplayString( display );  // gehoert X
  DBG_PROG_V( display_name <<" "<< strlen(display_name) )

# ifndef HAVE_FLTK
    XCloseDisplay( display ); DBG_PROG
# endif

# endif

  moni_profil = oyGetMonitorProfile( display_name, size );

  DBG_PROG_V( *size <<" "<< (int*)moni_profil )

  DBG_PROG_ENDE
  return moni_profil;
}
#endif

int
Oyranos::setzeMonitorProfil (const char* profil_name , int x, int y )
{
  DBG_PROG_START
  int fehler = false;

  DBG_PROG_V( profil_name )
  const char *display_name=0;

# if defined(HAVE_X) && !defined(__APPLE__)
  display_name = XDisplayString( fl_display );  // belongs to X
  DBG_PROG_V( display_name <<" "<< strlen(display_name) )
# endif

  int screen = oyGetScreenFromPosition( display_name, x,y );
  char *new_display_name = changeScreenName_( display_name, screen );

  if(new_display_name)
    fehler = oySetMonitorProfile( new_display_name, profil_name );

  char *neues_profil = oyGetMonitorProfileNameFromDB( display_name,myAllocFunc);
  if(new_display_name) { delete [] new_display_name; new_display_name = 0; }
  DBG_PROG_V( neues_profil )

  if (neues_profil) free (neues_profil);

  DBG_PROG_ENDE
  return fehler;
}

char**
Oyranos::moniInfo (int x, int y, int * num)
{
  DBG_PROG_START
  *num = 0;
  char **infos = 0;

  char *display_name=0;

#if defined(HAVE_X) && !defined(__APPLE__)
  display_name = oyGetDisplayNameFromPosition( 0, x,y, malloc );
#else
  display_name = oyGetDisplayNameFromPosition2( OY_TYPE_STD, "monitor.qarz", 0, x,y, malloc );
#endif
  infos = (char**) new char* [10];
  static char *manufacturer = 0;
  static char *model = 0;
  static char *serial = 0;
  static char *geometry = 0;
  static char *system_port = 0;

  static char * old_screen_name = 0;
//int fehler =
# if OYRANOS_VERSION > 300
  oyConfig_s * device = 0;
  oyOptions_s * options = 0;
  const char * t;

  if(display_name &&
     (!old_screen_name ||
      strcmp(display_name, old_screen_name) != 0))
  {
    oyOptions_SetFromString( &options,
                             "//" OY_TYPE_STD "/config/command",
                             "properties", OY_CREATE_NEW );
    oyDeviceGet( OY_TYPE_STD, "monitor", display_name, options, &device );
    oyOptions_Release( &options );

    t = oyConfig_FindString( device, "manufacturer", 0 );
    if(t) manufacturer = strdup( t );
    t = oyConfig_FindString( device, "model", 0 );
    if(t) model = strdup( t );
    t = oyConfig_FindString( device, "serial", 0 );
    if(t) serial = strdup( t );
    t = oyConfig_FindString( device, "geometry", 0 );
    if(t) geometry = strdup( t );
    t = oyConfig_FindString( device, "system_port", 0 );
    if(t) system_port = strdup( t );

    oyConfig_Release( &device );
    old_screen_name = strdup(display_name);
  } else if(!display_name)
  {
    infos[2 * *num] = icc_strdup_m(_("Screen:"));
    infos[2 * *num + 1] = icc_strdup_m(_("none"));
    *num += 1;
    return infos;
  }
# elif OYRANOS_VERSION > 109
               oyGetMonitorInfo( display_name,
                                 &manufacturer, &model, &serial, 
                                 &system_port, &geometry, 0,
                                 myAllocFunc );
# else
               oyGetMonitorInfo( display_name,
                                 &manufacturer, &model, &serial, 
                                 myAllocFunc );
# endif

  if( manufacturer && strlen( manufacturer ) )
  {
    infos[2 * *num] = icc_strdup_m(_("Manufacturer:"));
    infos[2 * *num + 1] = icc_strdup_m( manufacturer );
    *num += 1;
  }
  if( model && strlen( model ) )
  {
    infos[2 * *num] = icc_strdup_m(_("Model:"));
    infos[2 * *num + 1] = icc_strdup_m( model );
    *num += 1;
  }
  if( serial && strlen( serial ) )
  {
    infos[2 * *num] = icc_strdup_m(_("Serial:"));
    infos[2 * *num + 1] = icc_strdup_m( serial );
    *num += 1;
  }
  if( geometry && strlen( geometry ) )
  {
    infos[2 * *num] = icc_strdup_m(_("Geometry:"));
    infos[2 * *num + 1] = icc_strdup_m( geometry );
    *num += 1;
  }
  if( system_port && strlen( system_port ) )
  {
    infos[2 * *num] = icc_strdup_m(_("Port:"));
    infos[2 * *num + 1] = icc_strdup_m( system_port );
    *num += 1;
  }
    

  if(display_name) { free(display_name); display_name = 0; }

  DBG_PROG_ENDE
  return infos;
}

/** generates a gamut hull */
std::string
Oyranos::netzVonProfil_ (ICCnetz & netz,
                         Speicher & profil,
                         oyOptions_s * options)
{
  DBG_PROG_START
  // a cubus from six squares with the range of the Lab cube
  // will be transformed to a profile colour space and converted to a mesh
  int a = 12; // resolution : 10 - more quick; 20 - more precise
  size_t  size = 4*a*(a+1) + 2*(a-1)*(a-1);
  int     channels_n = 3;
  double *lab = new double [size*channels_n];
  double  min = 0.01, max = 0.99;
  // side quares
  for(int y = 0; y <= a; ++y)
    for(int x = 0; x < 4 * a; ++x)
    {
      int b = 0; // area
      int pos = (y * 4 * a + x) * channels_n;

      // see http://www.oyranos.org/2008/01/gamut-mapping/
      lab[pos + 0] = pow(.9999 - (double)y/(double)a, 2.0) + 0.0001;

      if       (b * a <= x && x < ++b * a) {
        lab[pos + 1] = min + (x - (b - 1) * a)/(double)a * (max-min);
        lab[pos + 2] = min;
      } else if(b * a <= x && x < ++b * a) {
        lab[pos + 1] = max;
        lab[pos + 2] = min + (x - (b - 1) * a)/(double)a * (max-min);
      } else if(b * a <= x && x < ++b * a) {
        lab[pos + 1] = max - (x - (b - 1) * a)/(double)a * (max-min);
        lab[pos + 2] = max;
      } else if(b * a <= x && x < ++b * a) {
        lab[pos + 1] = min;
        lab[pos + 2] = max - (x - (b - 1) * a)/(double)a * (max-min);
      }
    }
  // buttom and top square
  for(int y = 0; y < (a - 1); ++y)
    for(int x = 0; x < 2 * (a - 1); ++x)
    {
      int pos = (4 * a * (a + 1)  +  y * 2 * (a - 1) + x) * channels_n;
      int b = 1; // area
      int x_pos = x + 1, y_pos = y + 1;
      double val = (double)y_pos/(double)a * (max-min);

      if       (0 <= x && x < a - 1) {
        lab[pos + 0] = 1.0;
        lab[pos + 1] = min + (x_pos - (b - 1) * (a - 1))/(double)a * (max-min);
        lab[pos + 2] = min + val;
      } else if(a - 1 <= x && x < 2 * a - 2) {
        ++b;
        lab[pos + 1] = min + (x_pos - (b - 1) * (a - 1))/(double)a * (max-min);
        lab[pos + 2] = min + val;
        // see http://www.oyranos.org/2008/01/gamut-mapping/
        lab[pos + 0] = HYP( lab[pos + 1] - 0.5, lab[pos + 2] - 0.5)/100.;//0.0
      }
    }

  if(wandelLabNachProfilUndZurueck( lab, size, options,
                                    profil ))
    return  std::string("oyranos");
  //double * rgb = wandelLabNachBildschirmFarben( 0,0, lab, size, 0, 0 );

  // collect colour points
  netz.punkte. resize( size );
  for(size_t i = 0; i < size; ++i)
  {
    for(int k = 0; k < channels_n; ++k)
    {
      netz.punkte[i].koord[k] = lab [i*channels_n+k];
      netz.punkte[i].farbe[k] = 0;//rgb [i*channels_n+k];
    }
    netz.punkte[i].farbe[channels_n] = 1.0;
  }
  // build mesh
  char *liste = new char [size];
  memset( liste, 1, size );

  std::pair<double,DreiecksIndexe> index_p;
  for(int y = 0; y < a; ++y)
    for(int x = 0; x < 4 * a; ++x)
    {
      int x_ = (x == 4 * a - 1) ? -1 : x;
      index_p.second.i[0] =  y *  4*a+x; index_p.second.i[1] =  y *  4*a+x_+1;
      index_p.second.i[2] = (y+1)*4*a+x;
      netz.indexe. insert( index_p );

                                         index_p.second.i[0] =  y *  4*a+x_+1;
      index_p.second.i[2] = (y+1)*4*a+x; index_p.second.i[1] = (y+1)*4*a+x_+1;
      netz.indexe. insert( index_p );
    }

  int off = 4 * a * (a + 1);
  // 1 0 0 (L a b)
  index_p.second.i[0] = 4*a-1;   index_p.second.i[1] = off;
  index_p.second.i[2] = 0;
  netz.indexe. insert( index_p );

                                 index_p.second.i[0] = off;
  index_p.second.i[2] = 0;       index_p.second.i[1] = 1;
  netz.indexe. insert( index_p );

  // 0 0 0
  index_p.second.i[1] = off-1;   index_p.second.i[0] = off+a-1;
  index_p.second.i[2] = off-4*a;
  netz.indexe. insert( index_p );

                                 index_p.second.i[1] = off+a-1;
  index_p.second.i[2] = off-4*a; index_p.second.i[0] = off - 4*a+1;
  netz.indexe. insert( index_p );

  // 0 0 1
  index_p.second.i[2] = off-a; index_p.second.i[1] = off-a-1;
                               index_p.second.i[0] = off+2*(a-1)*(a-1)-a+1;
  netz.indexe. insert( index_p );

  index_p.second.i[0] = off-a;
  index_p.second.i[1] = off-a+1; index_p.second.i[2] = off+2*(a-1)*(a-1)-a+1;
  netz.indexe. insert( index_p );

  // 0 1 1
  index_p.second.i[0] = off-2*a+1; index_p.second.i[2] = off-2*a;
  index_p.second.i[1] = off+2*(a-1)*(a-1)-1;
  netz.indexe. insert( index_p );

                                             index_p.second.i[1] = off-2*a;
  index_p.second.i[2] = off+2*(a-1)*(a-1)-1; index_p.second.i[0] = off-2*a-1;
  netz.indexe. insert( index_p );


  // 1 1 1
  index_p.second.i[0] = 2*a-1;   index_p.second.i[2] = 2*a;
  index_p.second.i[1] = off+2*(a-1)*(a-1)-a;
  netz.indexe. insert( index_p );

                                             index_p.second.i[1] = 2*a;
  index_p.second.i[2] = off+2*(a-1)*(a-1)-a; index_p.second.i[0] = 2*a+1;
  netz.indexe. insert( index_p );

  // 1 0 1
  index_p.second.i[2] = 3*a;   index_p.second.i[0] = 3*a-1;
                               index_p.second.i[1] = off+2*(a-1)*(a-1)-2*a+2;
  netz.indexe. insert( index_p );

  index_p.second.i[2] = 3*a;
  index_p.second.i[1] = 3*a+1; index_p.second.i[0] = off+2*(a-1)*(a-1)-2*a+2;
  netz.indexe. insert( index_p );

  // 1 1 0
  index_p.second.i[0] = off+a-2; index_p.second.i[1] = a + 1;
  index_p.second.i[2] = a - 1;
  netz.indexe. insert( index_p );

                                 index_p.second.i[0] = a + 1;
  index_p.second.i[2] = a - 1;   index_p.second.i[1] = a;
  netz.indexe. insert( index_p );

  // 0 1 0
  index_p.second.i[0] = off+2*(a-1)-1;
  index_p.second.i[1] = off-3*a-1; index_p.second.i[2] = off-3*a;
  netz.indexe. insert( index_p );

  index_p.second.i[1] = off+2*(a-1)-1; index_p.second.i[0] = off-3*a+1;
                                       index_p.second.i[2] = off-3*a+0;
  netz.indexe. insert( index_p );

  for(int y = 0; y < a; ++y)
  {
    if(0 < y && y < a - 1)
    {
      // 0 0 .
      index_p.second.i[2] = off-y;  index_p.second.i[0] = off+(y+1)*2*(a-1)-a+1;
      index_p.second.i[1] = off-y-1;
      netz.indexe. insert( index_p );

                                    index_p.second.i[0] = off+(y+0)*2*(a-1)-a+1;
      index_p.second.i[2] = off-y;  index_p.second.i[1] = off+(y+1)*2*(a-1)-a+1;
      netz.indexe. insert( index_p );

      // 0 1 .
      index_p.second.i[1] = off+(y+1)*2*(a-1)-1; index_p.second.i[0] =off-3*a+y+1;
      index_p.second.i[2] = off+(y)*2*(a-1)-1;
      netz.indexe. insert( index_p );

                                               index_p.second.i[1] =off-3*a+y+1;
      index_p.second.i[2] = off+(y)*2*(a-1)-1; index_p.second.i[0] = off-3*a+y;
      netz.indexe. insert( index_p );

      // 1 0 .
                          index_p.second.i[0] = off+2*(a-1)*(a-1)-(y+1)*2*(a-1);
      index_p.second.i[1] = 3*a+y+1; index_p.second.i[2] = off+2*(a-1)*(a-1)-y*2*(a-1);
      netz.indexe. insert( index_p );

      index_p.second.i[0] = 3*a+y+1;  index_p.second.i[2] = off+2*(a-1)*(a-1)-y*2*(a-1);
      index_p.second.i[1] = 3*a+y;
      netz.indexe. insert( index_p );

      // 1 1 .
      index_p.second.i[0] = off+2*(a-1)*(a-1)-(y+1)*2*(a-1)+a-2;
      index_p.second.i[1] = off+2*(a-1)*(a-1)-(y+0)*2*(a-1)+a-2; index_p.second.i[2] = 2*a-y;
      netz.indexe. insert( index_p );

                                         index_p.second.i[0] = 2*a-y-1;
      index_p.second.i[1] = off+2*(a-1)*(a-1)-(y+1)*2*(a-1)+a-2; index_p.second.i[2] = 2*a-y;
      netz.indexe. insert( index_p );
    }

    for(int x = 0; x < 2 * a; ++x)
    {
      int x_ = x + off;

      // lower border
      if( y == 0 )
      {
        if(x == 0) {
        } else if(x == a - 1) {
        } else if(x < a - 1) {
          // 1 . 0
          index_p.second.i[0] = off + x - 1; index_p.second.i[1] = off + x;
          index_p.second.i[2] = x;
          netz.indexe. insert( index_p );

                                         index_p.second.i[0] = off + x;
          index_p.second.i[2] = x;       index_p.second.i[1] = x + 1;
          netz.indexe. insert( index_p );

          // 0 . 1
          index_p.second.i[0] = off-a-x; index_p.second.i[2] = off-a-x-1;
          index_p.second.i[1] = off+2*(a-1)*(a-1)-a+x;
          netz.indexe. insert( index_p );

                                         index_p.second.i[2] = off-a-x-1;
          index_p.second.i[0] = off+2*(a-1)*(a-1)-a+x; index_p.second.i[1] = off+2*(a-1)*(a-1)-a+x+1;
          netz.indexe. insert( index_p );

          // 1 . 1
          index_p.second.i[0] = 3*a - x; index_p.second.i[1] = 3*a - x-1;
          index_p.second.i[2] = off+2*(a-1)*(a-1)-2*(a-1)+x-1;
          netz.indexe. insert( index_p );

                                         index_p.second.i[0] = 3*a - x-1;
          index_p.second.i[2] = off+2*(a-1)*(a-1)-2*(a-1)+x-1; index_p.second.i[1] = off+2*(a-1)*(a-1)-2*(a-1)+x;
          netz.indexe. insert( index_p );

        } else if (x > a + 1) {
          // 0 . 0
          index_p.second.i[0] = off+x-3; index_p.second.i[2] = off+x-3+1;
          index_p.second.i[1] = 4*a*(a+1)-4*a + x-a-1;
          netz.indexe. insert( index_p );

                                         index_p.second.i[1] = off+x-3+1;
          index_p.second.i[2] = 4*a*(a+1)-4*a + x-a-1; index_p.second.i[0] = 4*a*(a+1)-4*a + x-a;
          netz.indexe. insert( index_p );
        }
      // upper border
      } else if( y == a - 1 )
      {
        if(x == 0) {
        }
      } else if(0 <= x && x < a - 1 - 1) {

        // upper middle field (*L=0.0)
        index_p.second.i[0] = (y-1) *  2*(a-1)+x_; index_p.second.i[2] =  (y-1)*2*(a-1)+x_+1;
        index_p.second.i[1] = (y+0)*2*(a-1)+x_;
        netz.indexe. insert( index_p );

                                                index_p.second.i[2] = (y-1)*2*(a-1)+x_+1;
        index_p.second.i[0] = (y+0)*2*(a-1)+x_; index_p.second.i[1] = (y+0)*2*(a-1)+x_+1;
        netz.indexe. insert( index_p );

      } else if(a - 1 <= x && x < 2 * a - 2 - 1) {
        // lower middle field (*L=1.0)
        index_p.second.i[0] = (y-1) *  2*(a-1)+x_; index_p.second.i[1] =  (y-1)*2*(a-1)+x_+1;
        index_p.second.i[2] = (y+0)*2*(a-1)+x_;
        netz.indexe. insert( index_p );

                                                index_p.second.i[0] = (y-1)*2*(a-1)+x_+1;
        index_p.second.i[2] = (y+0)*2*(a-1)+x_; index_p.second.i[1] = (y+0)*2*(a-1)+x_+1;
        netz.indexe. insert( index_p );
      }
    }
  }

  netz.kubus = 1;

  //delete [] rgb;
  delete [] lab;

  DBG_PROG_ENDE
  return std::string("oyranos");
}


#include "icc_vrml.h"
#include "icc_gamut.h"
void
Oyranos::netzVonProfil (ICCprofile & profil, oyOptions_s * options,
                        int native, ICCnetz & netz)
{
  DBG_PROG_START
  Speicher s;

  if(profil.valid()) {
      size_t groesse = 0;
      oyProfile_s * p = profil.oyProfile();
      char * daten = (char*)oyProfile_GetMem( p, &groesse, 0, malloc );
      oyProfile_Release( &p );
      s.ladeUndFreePtr(&daten, groesse);
      DBG_NUM_V( groesse );
  }

  if(s.size())
  {
    std::string vrml;

    double v = profil.getHeader().versionD();

    int intent = 0;
    const char * ri_text = oyOptions_FindString( options, "rendering_intent", 0 );
    if(ri_text)
      sscanf(ri_text, "%d", &intent );


    if(native && v < 4)
      vrml = iccCreateVrml ( s,(int)s.size(), intent );

    icc_examin_ns::ICCThreadList<ICCnetz> netze;

    if(vrml.size())
    {
      netzAusVRML (vrml, netze);
    }

    std::string t;

    if(netze.size())
    {
      netz = netze[0];
    } else {
      t = netzVonProfil_(netz, s, options);
      if(v < 4 && native)
      {
      WARN_S("Fall back to internal hull generation. Argyll is not installed?");
      } else if(native) {
      DBG_NUM_S("Fall back to internal hull generation. "<< v);
      }
    }

    if(netz.punkte.size())
    {
      size_t groesse = 0;
      double* band = icc_examin_ns::iccGrenze( profil, options, groesse );
      DBG_PROG_V( (int*) band <<" "<< groesse )

      netz.umriss.resize( groesse );

      for(int i = 0; i < (int)groesse; ++i)
        for(int j = 0; j < 3; ++j)
          netz.umriss[i].koord[j] = band[i*3+j];
      if(band) delete [] band;
    }
  }
  DBG_PROG_ENDE
}

std::string
Oyranos::vrmlVonProfil (ICCprofile & profil, oyOptions_s * options,
                        int native)
{
  DBG_PROG_START
  Speicher s;
  std::string vrml;
  icc_examin_ns::ICCThreadList<ICCnetz> netze;

  netze.resize(1);

  netzVonProfil( profil, options, native,
                                 netze[0] );

  int p_n = netze[0].punkte.size();
  for(int j = 0; j < p_n; ++j)
  {
        double lab[3];

        for(int k = 0; k < 3 ; ++k)
          lab[k] = netze[0].punkte[j].koord[k];

        double * rgb = icc_oyranos.wandelLabNachBildschirmFarben( 0, 0, NULL,
                                                              lab, 1, options );
        for(int k = 0; k < 3 ; ++k)
          netze[0].punkte[j].farbe[k] = rgb[k];

        free( rgb );
  }

  vrml = netzNachVRML( netze );
  if(vrml.size())
    vrml = writeVRMLbody( vrml );
  else
  if(profil.valid()) {
      size_t groesse = 0;
      char* daten = profil.saveProfileToMem(&groesse); 
      s.ladeUndFreePtr(&daten, groesse);
      DBG_NUM_V( groesse );
  }

  if(s.size())
  {
    double v = profil.getHeader().versionD();

    int intent = 0;
    const char * ri_text = oyOptions_FindString( options, "rendering_intent", 0 );
    if(ri_text)
      sscanf(ri_text, "%d", &intent );

    if(native && v < 4)
      vrml = iccCreateVrml ( s, (int)s.size(), intent );
  }

  DBG_PROG_ENDE
  return vrml;
}

ICClist<double>
Oyranos::bandVonProfil (const Speicher & p, int intent ICC_UNUSED)
{
  DBG_PROG_START
# ifdef DEBUG
  const char* b = p;
#endif
  DBG_MEM_V( (int*)b )
  ICClist<double> band; // TODO
  DBG_PROG_ENDE
  return band;
}


#define PRECALC cmsFLAGS_NOTPRECALC 
#if 0
#define BW_COMP cmsFLAGS_WHITEBLACKCOMPENSATION
#else
#define BW_COMP 0
#endif

void
Oyranos::gamutCheckAbstract(Speicher & s, Speicher & abstract,
                            oyOptions_s * options)
{
  DBG_PROG_START
  size_t size = s.size();
  const char* block = s;

  DBG_MEM_V( (int*) block <<" "<<size )

    double start = fortschritt();

    oyOptions_s * opts = 0,
                * result = 0;
    oyProfile_s * prof = oyProfile_FromMem( size, (void*)block, 0, 0 );
    int error = oyOptions_MoveInStruct( &opts,
                      "//" OY_TYPE_STD "/proofing_profile",
                      (oyStruct_s**)&prof, 0 );

    oyOptions_AppendOpts( opts, options );
    error = oyOptions_Handle( "//" OY_TYPE_STD "/create_profile.icc",
                      opts,"create_profile.icc_profile.proofing_effect",
                      &result );

    if(error)
      WARN_S("create_profile.icc_profile.proofing_effect failed");

    prof = (oyProfile_s*) oyOptions_GetType( result, -1, "icc_profile",
                                             oyOBJECT_PROFILE_S );

    oyOptions_Release( &result );
    oyOptions_Release( &opts );

    fortschritt(0.2, 0.2);
    char * mem = (char*)oyProfile_GetMem( prof, &size, 0, malloc );
    abstract.ladeUndFreePtr (&mem, size);

    if(start <= 0.0)
      fortschritt(1.1);

  oyProfile_Release( &prof );
 
  DBG_PROG_ENDE
}


int
Oyranos::wandelLabNachProfilUndZurueck(double *lab, // 0.0 - 1.0
                                       size_t  size, oyOptions_s * options,
                                       Speicher & p )
{
  DBG_PROG_START

    size_t bsize = p.size();
    const char*  block = p;
    int channels_n;
    int input_ausnahme = 0;
    oyProfile_s * profile = oyProfile_FromMem( bsize, (void*)block, 0, 0 );
    channels_n = oyProfile_GetChannelsCount( profile );
    double * channels = new double [size*channels_n];
    oyProfile_s * lab_profile = oyProfile_FromStd( oyEDITING_LAB, icc_oyranos.icc_profile_flags, 0 );
    oyConversion_s * ctolab = 0, * labtoc = 0;
    oyImage_s * image_lab, * image_tmp;

    {
      // initialising
      DBG_MEM_V( (int*) block <<" "<<bsize )

      image_lab   = oyImage_Create( size, 1,
                         lab ,
                         oyChannels_m(oyProfile_GetChannelsCount(lab_profile)) |
                         oyDataType_m(oyDOUBLE),
                         lab_profile,
                         0 );
      image_tmp   = oyImage_Create( size, 1,
                         channels ,
                         oyChannels_m(oyProfile_GetChannelsCount(profile)) |
                         oyDataType_m(oyDOUBLE),
                         profile,
                         0 );


      if(!bsize)
        WARN_S("no profile found");

      icColorSpaceSignature pcs = (icColorSpaceSignature)
                           oyProfile_GetSignature( profile, oySIGNATURE_PCS );
      if( pcs != icSigXYZData && pcs != icSigLabData )
        return 1;
      icProfileClassSignature device = (icProfileClassSignature)
                           oyProfile_GetSignature( profile, oySIGNATURE_CLASS );
      if( device != icSigInputClass && device != icSigDisplayClass &&
          device != icSigOutputClass && device != icSigAbstractClass )
        return 1;

      /*hLab  = cmsCreateLabProfile(cmsD50_xyY());
      cmsSetColorSpace( hLab, icSigRgbData );
      cmsSetDeviceClass( hLab, icSigInputClass );
      if(!hLab) { WARN_S( "hLab Profil not opened" ); return 1; }
      */
      labtoc = oyConversion_CreateBasicPixels(
                               image_lab, image_tmp, options, 0 );

      if (!labtoc)
      {
        if(device == icSigInputClass && 
           channels_n == 3)
        {
          // use colours instantly
          input_ausnahme = 1;

        } else
        {
          WARN_S( "no transformation found" );
          return 1;
        }
      }
    }

    if(!channels) { WARN_S( "not enough memory available" ); return 1; }

#ifdef DEBUG
    double *cielab_tmp = new double [size * 3];
#endif


    if(!input_ausnahme)
    {
      oyConversion_RunPixels( labtoc, 0 );
      oyConversion_Release( &labtoc );
#ifdef DEBUG
      memcpy( cielab_tmp, lab, size * 3 * sizeof(double));
#endif
    } else {
      memcpy( channels, lab, size * channels_n * sizeof(double));
    }

    ctolab = oyConversion_CreateBasicPixels(
                               image_tmp, image_lab, options, 0 );
    /*cmsSetColorSpace( hLab, icSigLabData );
    cmsSetDeviceClass( hLab, icSigOutputClass );
    form = cmsCreateTransform                 (hProfil, format,
                                               hLab, TYPE_Lab_DBL,
                                               intent,
                                               PRECALC|flags);
    if (!form) { WARN_S( "no transformation found" ); return 1; }

    cmsDoTransform (form, channels, cielab, (unsigned int)size);
    cmsDeleteTransform (form);*/
    oyConversion_RunPixels( ctolab, 0 );
    oyConversion_Release( &ctolab );
    

#ifdef DEBUG
    if(channels_n == 4)
    {
      if(icc_debug > 1)
      {
        oyProfile_s * prof = oyProfile_FromStd( oyEDITING_LAB, icc_oyranos.icc_profile_flags, 0 );
        char * txt = oyDumpColorToCGATS( cielab_tmp, size, prof, malloc,
                                          __FILE__ );
        if(txt)
        {
          saveMemToFile( "icc_oyranos.lab1.cgats", txt, strlen(txt) );
          free(txt);
        }
        txt = oyDumpColorToCGATS( channels, size, profile, malloc,
                                          __FILE__ );
        if(txt)
        {
          saveMemToFile( "icc_oyranos.cmyk.cgats", txt, strlen(txt) );
          free(txt);
        }
        txt = oyDumpColorToCGATS( lab, size, prof, malloc,
                                          __FILE__ );
        if(txt)
        {
          saveMemToFile( "icc_oyranos.lab2.cgats", txt, strlen(txt) );
          free(txt);
        }
        oyProfile_Release( &prof );
      }
    }
    DBG_NUM_V(channels_n)
#endif

    oyProfile_Release( &profile );
    oyProfile_Release( &lab_profile );
    oyImage_Release( &image_lab );
    oyImage_Release( &image_tmp );

    if(channels) { delete [] channels; channels = NULL; }

  DBG_PROG_ENDE
  return 0;
}


oyProfile_s * Oyranos::oyMoni (int x, int y, int native)
{
  char * disp_name = 0;
  oyProfile_s * disp_prof = 0;

  static int x_alt = -1, y_alt = -1;
  static oyProfile_s * prof_alt = 0;

  if(x == x_alt && y == y_alt && prof_alt)
  {
    oyProfile_Copy( prof_alt, 0 );
    return prof_alt;
  }

#if defined(HAVE_X) && !defined(__APPLE__)
  disp_name = oyGetDisplayNameFromPosition( 0, x,y, malloc );
#else
  disp_name = oyGetDisplayNameFromPosition2( OY_TYPE_STD, "monitor.qarz", 0, x,y, malloc );
#endif

  if(disp_name)
  {
    oyOptions_s * options = 0;
    oyConfig_s * device = 0;

    if(native)
    {
      oyOptions_SetFromString( &options,
                             "//" OY_TYPE_STD "/config/command",
                             "list", OY_CREATE_NEW );
      oyOptions_SetFromString( &options,
                 "//" OY_TYPE_STD "/config/icc_profile.x_color_region_target",
                                       "yes", OY_CREATE_NEW );
    }

    
#if defined(HAVE_X) && !defined(__APPLE__)
    oyDeviceGet( OY_TYPE_STD, "monitor", disp_name, 0, &device );
#else
    oyDeviceGet( OY_TYPE_STD, "monitor.qarz", disp_name, 0, &device );
#endif


    oyDeviceGetProfile( device, options, &disp_prof );
    oyOptions_Release( &options );

    if(disp_name)
      free(disp_name);
  }

  if(!disp_prof)
  {
    WARN_S("Could not load profile. Use sRGB instead.")
    disp_prof = oyProfile_FromStd( oyASSUMED_WEB, icc_oyranos.icc_profile_flags, 0 );
  }

  if (disp_prof)
  {
    x_alt = x;
    y_alt = y;
    prof_alt = disp_prof;
  }

  oyProfile_Copy( disp_prof, 0 );
  return disp_prof;
}


double*
Oyranos::wandelLabNachBildschirmFarben(int x, int y, oyProfile_s * profile,
                                       double *Lab_Speicher, // 0.0 - 1.0
                                       size_t  size, oyOptions_s * options)
{
  DBG_5_START

    if(profile)
      return convertLabToProfile( profile, Lab_Speicher, size, options );

    oyProfile_s * prof_disp = oyMoni(x,y);

    double *RGB_Speicher = NULL;

    static oyConversion_s * cc = NULL;
    static oyProfile_s * prof_disp_old = NULL;
    static oyImage_s * image_lab = NULL,
                     * image_disp = NULL;
    static oyOptions_s * options_old = NULL;
    static oyPixelAccess_s * pixel_access = NULL;
    static double * rgb = NULL, * lab = NULL;
    static size_t size_old = 0;

    oyFilterNode_s * node;
    oyFilterPlug_s * plug;

    RGB_Speicher = new double[size*3];
    if(!RGB_Speicher)  WARN_S( "RGB_speicher Speicher not available" )


    if(prof_disp != prof_disp_old ||
       options != options_old ||
       size != size_old)
    {
      oyOptions_Release( &options_old );
      options_old = oyOptions_Copy( options, 0 );

      oyProfile_Release( &prof_disp_old );
      prof_disp_old = oyProfile_Copy( prof_disp, 0 );

      size_old = size;

      if(rgb) {
        free(rgb); rgb = 0;
      }
      rgb = (double*) calloc(sizeof(double),3*size);
      if(lab)
        free(lab);
      lab = (double*) calloc(sizeof(double),3*size);

      oyImage_Release( &image_disp );
      image_disp   = oyImage_Create( size, 1,
                         rgb,
                         oyChannels_m(oyProfile_GetChannelsCount(prof_disp)) |
                         oyDataType_m(oyDOUBLE),
                         prof_disp,
                         0 );

      oyProfile_s * prof_lab = oyProfile_FromStd( oyEDITING_LAB, icc_oyranos.icc_profile_flags, 0 );
      oyImage_Release( &image_lab );
      image_lab   = oyImage_Create( size, 1,
                         lab,
                         oyChannels_m(oyProfile_GetChannelsCount(prof_lab)) |
                         oyDataType_m(oyDOUBLE),
                         prof_lab,
                         0 );

      oyConversion_Release( &cc );
      cc = oyConversion_CreateBasicPixels( image_lab, image_disp, options, 0 );

      oyProfile_Release( &prof_lab );

      oyPixelAccess_Release( &pixel_access );
      node = oyConversion_GetNode( cc, OY_OUTPUT );
      plug = oyFilterNode_GetPlug( node, 0 );
      oyFilterNode_Release( &node );
      pixel_access = oyPixelAccess_Create( 0,0, plug, oyPIXEL_ACCESS_IMAGE, 0 );
      oyFilterPlug_Release( &plug );
    }

    memcpy( lab, Lab_Speicher, sizeof(double)*3*size );
    oyConversion_RunPixels( cc, pixel_access );
    memcpy( RGB_Speicher, rgb, sizeof(double)*3*size );

    oyProfile_Release( &prof_disp );

  DBG_5_ENDE
  return RGB_Speicher;
}

double*  Oyranos::convertLabToProfile    ( oyProfile_s * profile,
                                           double *Lab_Speicher, // 0.0 - 1.0
                                           size_t  size, oyOptions_s * options)
{
  DBG_5_START


    oyProfile_s * prof_disp = profile;

    double *RGB_Speicher = NULL;

    static oyConversion_s * cc = NULL;
    static oyProfile_s * prof_disp_old = NULL;
    static oyImage_s * image_lab = NULL,
                     * image_disp = NULL;
    static oyOptions_s * options_old = NULL;
    static oyPixelAccess_s * pixel_access = NULL;
    static double * rgb = NULL, * lab = NULL;
    static size_t size_old = 0;

    oyFilterNode_s * node;
    oyFilterPlug_s * plug;

    RGB_Speicher = new double[size*3];
    if(!RGB_Speicher)  WARN_S( "RGB_speicher Speicher not available" )


    if(prof_disp != prof_disp_old ||
       options != options_old ||
       size != size_old)
    {
      oyOptions_Release( &options_old );
      options_old = oyOptions_Copy( options, 0 );

      oyProfile_Release( &prof_disp_old );
      prof_disp_old = oyProfile_Copy( prof_disp, 0 );

      size_old = size;

      if(rgb)
        free(rgb);
      rgb = (double*) calloc(sizeof(double),3*size);
      if(lab)
        free(lab);
      lab = (double*) calloc(sizeof(double),3*size);

      oyImage_Release( &image_disp );
      image_disp   = oyImage_Create( size, 1,
                         rgb,
                         oyChannels_m(oyProfile_GetChannelsCount(prof_disp)) |
                         oyDataType_m(oyDOUBLE),
                         prof_disp,
                         0 );

      oyProfile_s * prof_lab = oyProfile_FromStd( oyEDITING_LAB, icc_oyranos.icc_profile_flags, 0 );
      oyImage_Release( &image_lab );
      image_lab   = oyImage_Create( size, 1,
                         lab,
                         oyChannels_m(oyProfile_GetChannelsCount(prof_lab)) |
                         oyDataType_m(oyDOUBLE),
                         prof_lab,
                         0 );

      oyConversion_Release( &cc );
      cc = oyConversion_CreateBasicPixels( image_lab, image_disp, options, 0 );

      oyProfile_Release( &prof_lab );

      oyPixelAccess_Release( &pixel_access );
      node = oyConversion_GetNode( cc, OY_OUTPUT );
      plug = oyFilterNode_GetPlug( node, 0 );
      oyFilterNode_Release( &node );
      pixel_access = oyPixelAccess_Create( 0,0, plug, oyPIXEL_ACCESS_IMAGE, 0 );
      oyFilterPlug_Release( &plug );
    }

    memcpy( lab, Lab_Speicher, sizeof(double)*3*size );
    oyConversion_RunPixels( cc, pixel_access );
    memcpy( RGB_Speicher, rgb, sizeof(double)*3*size );


  DBG_5_ENDE
  return RGB_Speicher;
}


void Oyranos::colourServerRegionSet  ( Fl_Widget         * widget,
                                       oyProfile_s       * p,
                                       oyRectangle_s     * old_rect,
                                       int                 remove )
{
#if defined(HAVE_X) && !defined(__APPLE__)          
  if(!fl_display || !widget->window() || !widget->window()->visible())
    return;

      /* add X11 window and display identifiers to output image */
  Display * dpy = fl_display;
  Window win = fl_xid(widget->window());

  oyBlob_s * b = oyBlob_New(NULL);
  oyOptions_s * opts = oyOptions_New( NULL ),
              * result = NULL;
  oyRectangle_s * r;
  oyProfile_s * prof = NULL;
  int error = 0;

  oyBlob_SetFromStatic( b, (void*)win, 0, "Window" );
  error = oyOptions_MoveInStruct( &opts, "///window_id", (oyStruct_s**)&b,
                          OY_CREATE_NEW );
  b = oyBlob_New(NULL);
  oyBlob_SetFromStatic( b, (void*)dpy, 0, "Display" );
  error = oyOptions_MoveInStruct( &opts, "///display_id", (oyStruct_s**)&b,
                          OY_CREATE_NEW);
  if(remove)
    r = oyRectangle_NewWith( 0, 0, 0, 0, NULL );
  else
    r = oyRectangle_NewWith( widget->x(), widget->y(), widget->w(), widget->h(),
                             NULL );
  error = oyOptions_MoveInStruct( &opts, "///window_rectangle",(oyStruct_s**)&r,
                          OY_CREATE_NEW );
  r = oyRectangle_Copy( old_rect, NULL );
  error = oyOptions_MoveInStruct( &opts, "///old_window_rectangle",
                          (oyStruct_s**)&r, OY_CREATE_NEW );
  if(p)
  {
    prof = oyProfile_Copy( p, NULL );
    error = oyOptions_MoveInStruct( &opts, "///icc_profile",(oyStruct_s**)&prof,
                          OY_CREATE_NEW );
  }

  error = oyOptions_Handle( "//" OY_TYPE_STD "/set_xcm_region",
                                opts,"set_xcm_region",
                                &result );
  if(error)
    WARN_S("\"set_xcm_region\" failed " << error);
  oyOptions_Release( &opts );
#endif                     
}

oyProfile_s * Oyranos::getEditingProfile      ( int early_binding )
{
  static oyProfile_s * editing = NULL;

  if(!editing)
  {
#if defined(XCM_HAVE_X11)
    if(!((colourServerActive() & XCM_COLOR_SERVER_REGIONS) &&
         (colourServerActive() & XCM_COLOR_SERVER_PROFILES)) &&
       !early_binding)
      return oyProfile_FromStd( oyASSUMED_WEB, icc_oyranos.icc_profile_flags, 0 );
#endif

    std::string editing_name("ICC Examin ROMM gamma 2.2 ");

    if(icc_oyranos.icc_profile_flags & OY_ICC_VERSION_2)
      editing_name += "v2";
    else
      editing_name += "v4";

    editing_name += ".icc";

    editing = oyProfile_FromFile( editing_name.c_str(), icc_oyranos.icc_profile_flags, 0 );

    if(editing)
      return editing;

    oyOption_s *matrix = oyOption_FromRegistration("///color_matrix."
              "from_primaries."
              "redx_redy_greenx_greeny_bluex_bluey_whitex_whitey_gamma", NULL );
    /* http://www.color.org/chardata/rgb/rommrgb.xalter
     * original gamma is 1.8, we adapt to typical LCD gamma of 2.2 */
    oyOption_SetFromDouble( matrix, 0.7347, 0, 0);
    oyOption_SetFromDouble( matrix, 0.2653, 1, 0);
    oyOption_SetFromDouble( matrix, 0.1596, 2, 0);
    oyOption_SetFromDouble( matrix, 0.8404, 3, 0);
    oyOption_SetFromDouble( matrix, 0.0366, 4, 0);
    oyOption_SetFromDouble( matrix, 0.0001, 5, 0);
    oyOption_SetFromDouble( matrix, 0.3457, 6, 0);
    oyOption_SetFromDouble( matrix, 0.3585, 7, 0);
    oyOption_SetFromDouble( matrix, 2.2, 8, 0);

    oyOptions_s * opts = oyOptions_New(0),
                * result = 0;

    oyOptions_SetFromInt( &opts, "///icc_profile_flags", icc_oyranos.icc_profile_flags, 0, OY_CREATE_NEW );
    oyOptions_MoveIn( opts, &matrix, -1 );
    oyOptions_Handle( "//" OY_TYPE_STD "/create_profile.icc",
                                opts,"create_profile.icc_profile.color_matrix",
                                &result );

    editing = (oyProfile_s*)oyOptions_GetType( result, -1, "icc_profile",
                                               oyOBJECT_PROFILE_S );
    oyOptions_Release( &result );

    oyProfile_AddTagText( editing, icSigProfileDescriptionTag, editing_name.c_str() );

    if(oy_debug)
    {
      size_t size = 0;
      char * data = (char*) oyProfile_GetMem( editing, &size, 0, malloc );
      saveMemToFile( "ICC Examin ROMM gamma 2.2.icc", data, size );
    }

    oyProfile_Install( editing, oySCOPE_USER, NULL );
  }

  return editing;
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
  // Schluessel erzeugen
  static char schluessel[1024];
  
  sprintf (schluessel, "%s-%d_%s-%d_%d", profilA, ein_bytes, kanaeleA,
                                         profilB, aus_bytes, kanaeleB,
                                         intent, optionen,
                                         proof_profil, intent_p);

  //trafos_.find(schluessel);
  DBG_PROG_ENDE
}
#endif


int      Oyranos::colourServerActive( )
{
  static int active = 0;
#if defined(HAVE_X) && !defined(__APPLE__)
  static double z = 0;
  if(z + 1.0 < icc_examin_ns::zeitSekunden())
  {
    active = XcmColorServerCapabilities( fl_display );
    z = icc_examin_ns::zeitSekunden();
    if(oy_debug)
      printf("colour server active: %d %g\n", active, z);
  }
#endif
  return active;
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
oyranos_pfad_dazu (char* pfad ICC_UNUSED)
{
}

