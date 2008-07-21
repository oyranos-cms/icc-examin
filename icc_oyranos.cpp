/*
 * ICC Examin ist eine ICC Profil Betrachter
 * 
 * Copyright (C) 2004-2008  Kai-Uwe Behrmann 
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

#define BOOL LCMS_BOOL
#include <lcms.h>
#undef BOOL

#ifdef HAVE_OY
#include "oyranos/oyranos.h"
#include "oyranos/oyranos_monitor.h"
using namespace oyranos;
#endif

#ifdef APPLE
#include <Carbon/Carbon.h>
#endif

#ifdef HAVE_X
#include <X11/Xlib.h>
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
    rgb_.clear();
    cmyk_.clear();
    proof_.clear();
}

bool
Oyranos::profil_test_ (const char* profil_name)
{
  DBG_PROG_START
  bool fehler = false;
# if HAVE_OY

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
        if( erg.second = true )
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

# endif
  return fehler;
  DBG_PROG_ENDE
}

void
Oyranos::lab_test_ ()
{
  DBG_PROG_START
# if HAVE_OY
  Speicher *v_block = &lab_;
  if( !v_block->size() )
  { DBG_PROG_V( v_block->size() )
    char* profil_name = oyGetDefaultProfileName( oyranos::oyASSUMED_LAB, myAllocFunc );
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

  
# endif
  DBG_PROG_ENDE
}

#ifdef APPLE
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

#ifndef HAVE_OY
typedef  void* (oyAllocFunc_t)(size_t size);
#endif

int
oyGetProfileBlockOSX (CMProfileRef prof, char *block, size_t *size, oyAllocFunc_t allocateFunc)
{
  DBG_PROG_START
    CMProfileLocation loc;
    CMGetProfileLocation(prof, &loc);
    switch(loc.locType)
    {
      case cmNoProfileBase:
             DBG_PROG_S("Das Monitorprofil ist ein temporaeres Profil.")
             break;
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
    }
  DBG_PROG_ENDE
  return 0;
}

#endif

char*
changeScreenName_( const char *display_name, int screen )
{
  char *new_display_name = 0;
  if( (new_display_name = (char*) new char [strlen(display_name) +24]) == 0)
    return NULL;

  sprintf( new_display_name, "%s", display_name );

  char *ptr = strchr( new_display_name, ':' );
  char *ptr2 = 0;
  if( ptr )
  {
    ptr2 = strchr( ptr, '.' );
    sprintf( ptr2, ".%d", screen );
  } else
    sprintf( &ptr[strlen(ptr)], ".%d", screen );

  return new_display_name;
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
    if(block && size)
      v_block.ladeUndFreePtr(&block, size);
    else
      v_block.zeit( icc_examin_ns::zeitSekunden() );

    const char* oy_moni_name = oyProfile_GetFileName( oy_moni, 0 );

    if(oy_moni_name)
      v_block = oy_moni_name;
    oyProfile_Release( &oy_moni );
  }

  DBG_PROG_ENDE
  return;
}

void
Oyranos::rgb_test_ ()
{
  DBG_PROG_START
# if HAVE_OY
  Speicher *v_block = &rgb_;
  if( !v_block->size() )
  { DBG_PROG_V( v_block->size() )
    const char* profil_name = oyGetDefaultProfileName( oyranos::oyASSUMED_RGB, myAllocFunc );
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
# else
# if APPLE
  CMProfileRef prof=NULL;
  char *block = 0;
  size_t groesse = 0;
  oyGetProfileBlockOSX(prof, block, &groesse, malloc);
  if(groesse) {
    block = (char*)malloc(groesse);
    oyGetProfileBlockOSX(prof, block, &groesse, malloc);
  }
  Speicher *v_block = &rgb_;
  v_block->ladeUndFreePtr(&block,groesse);
# endif
# endif
  DBG_PROG_ENDE
}


void
Oyranos::cmyk_test_ ()
{
  DBG_PROG_START
# if HAVE_OY
  Speicher *v_block = &cmyk_;
  if( !v_block->size() )
  { DBG_PROG_V( v_block->size() )
    char* profil_name = oyGetDefaultProfileName( oyranos::oyASSUMED_CMYK, myAllocFunc );
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
# else
# if APPLE
# endif
# endif
  //oy_debug = 0;
  DBG_PROG_ENDE
}

void
Oyranos::proof_test_ ()
{
  DBG_PROG_START
# if HAVE_OY
  Speicher *v_block = &proof_;
  char* profil_name = NULL;

  profil_name = oyGetDefaultProfileName( oyranos::oyPROFILE_PROOF, myAllocFunc);

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
# else
# if APPLE
# endif
# endif
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

# if HAVE_OY

# ifdef HAVE_X
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

# endif
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
# if HAVE_OY
  const char *display_name=0;

# ifdef HAVE_X
  static Display *display=0;

# ifdef HAVE_FLTK
  if( !display )
    ;//display = fl_display;
# endif
  //if( !display )
    //display = XOpenDisplay(0);

  display_name = XDisplayString( fl_display );  // belongs to X
  DBG_PROG_V( display_name <<" "<< strlen(display_name) )

# ifndef HAVE_FLTK
    //XCloseDisplay( display ); DBG_PROG
# endif

# endif

  int screen = oyGetScreenFromPosition( display_name, x,y );
  char *new_display_name = changeScreenName_( display_name, screen );

  fehler = oySetMonitorProfile( new_display_name, profil_name );

  char *neues_profil = oyGetMonitorProfileName( display_name, myAllocFunc );
  if(new_display_name) { delete [] new_display_name; new_display_name = 0; }
  DBG_PROG_V( neues_profil )

  if (neues_profil) free (neues_profil);
# endif

  DBG_PROG_ENDE
  return fehler;
}

char**
Oyranos::moniInfo (int x, int y, int *num)
{
  DBG_PROG_START
  *num = 0;
  char **infos = 0;

# if HAVE_OY
  const char *display_name=0;

# ifdef HAVE_X
  //static Display *display=0;

  display_name = XDisplayString( fl_display );  // belongs to FLTK
  DBG_PROG_V( display_name <<" "<< strlen(display_name) )
# endif
  infos = (char**) new char* [6];
  char *manufacturer = 0;
  char *model = 0;
  char *serial = 0;

  int screen = oyGetScreenFromPosition( display_name, x,y );
  char *new_display_name = changeScreenName_( display_name, screen );
//int fehler =
               oyGetMonitorInfo( new_display_name,
                                 &manufacturer, &model, &serial,
                                 myAllocFunc );

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
    

  if(new_display_name) { delete [] new_display_name; new_display_name = 0; }

# endif

  DBG_PROG_ENDE
  return infos;
}

/** generates a gamut hull */
std::string
Oyranos::netzVonProfil_ (ICCnetz & netz,
                         Speicher & profil,
                         int intent, int bpc)
{
  DBG_PROG_START
  // a cubus from six squares with the range of the Lab cube
  // will be transformed to a profile colour space and converted to a mesh
  int a = 12; // resolution : 10 - more quick; 20 - more precise
  size_t  size = 4*a*(a+1) + 2*(a-1)*(a-1);
  int     kanaele = 3;
  double *lab = new double [size*kanaele];
  double  min = 0.01, max = 0.99;
  // side quares
  for(int y = 0; y <= a; ++y)
    for(int x = 0; x < 4 * a; ++x)
    {
      int b = 0; // area
      int pos = (y * 4 * a + x) * kanaele;

      // see http://www.behrmann.name/wind/oyranos/icc_examin_2008.01.21.html
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
      int pos = (4 * a * (a + 1)  +  y * 2 * (a - 1) + x) * kanaele;
      int b = 0; // area
      int x_pos = x + 1, y_pos = y + 1;
      double val = (double)y_pos/(double)a * (max-min);

      if       (b * (a - b) <= x && x < ++b * a - b) {
        lab[pos + 0] = 1.0;
        lab[pos + 1] = min + (x_pos - (b - 1) * (a - 1))/(double)a * (max-min);
        lab[pos + 2] = min + val;
      } else if(b * (a - b) <= x && x < ++b * a - b) {
        lab[pos + 1] = min + (x_pos - (b - 1) * (a - 1))/(double)a * (max-min);
        lab[pos + 2] = min + val;
        // see http://www.behrmann.name/wind/oyranos/icc_examin_2008.01.21.html
        lab[pos + 0] = HYP( lab[pos + 1] - 0.5, lab[pos + 2] - 0.5)/100.;//0.0
      }
    }

  if(wandelLabNachProfilUndZurueck( lab, size, intent, bpc, profil ))
    return  std::string("oyranos");
  //double * rgb = wandelLabNachBildschirmFarben( 0,0, lab, size, 0, 0 );

  // collect colour points
  netz.punkte. resize( size );
  for(size_t i = 0; i < size; ++i)
  {
    for(int k = 0; k < kanaele; ++k)
    {
      netz.punkte[i].koord[k] = lab [i*kanaele+k];
      netz.punkte[i].farbe[k] = 0;//rgb [i*kanaele+k];
    }
    netz.punkte[i].farbe[kanaele] = 1.0;
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
      int b = 0; // area

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
      } else if(b * (a - b) <= x && x < ++b * a - b - 1) {

        // upper middle field (*L=0.0)
        index_p.second.i[0] = (y-1) *  2*(a-1)+x_; index_p.second.i[2] =  (y-1)*2*(a-1)+x_+1;
        index_p.second.i[1] = (y+0)*2*(a-1)+x_;
        netz.indexe. insert( index_p );

                                                index_p.second.i[2] = (y-1)*2*(a-1)+x_+1;
        index_p.second.i[0] = (y+0)*2*(a-1)+x_; index_p.second.i[1] = (y+0)*2*(a-1)+x_+1;
        netz.indexe. insert( index_p );

      } else if(b * (a - b) <= x && x < ++b * a - b - 1) {
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
Oyranos::netzVonProfil (ICCprofile & profil, int intent, int bpc,
                        int native, ICCnetz & netz)
{
  DBG_PROG_START
  Speicher s;

  if(profil.valid()) {
      size_t groesse = 0;
      char* daten = profil.saveProfileToMem(&groesse);
      s.ladeUndFreePtr(&daten, groesse);
      DBG_NUM_V( groesse );
  }

  if(s.size())
  {
    std::string vrml;

    double v = profil.getHeader().versionD();

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
      t = netzVonProfil_(netz, s, intent, bpc);
#ifdef USE_ARGYLL
      DBG_PROG_S("Fall back to internal hull generation.");
#else
      if(v < 4 && native)
      {
      WARN_S("Fall back to internal hull generation. Argyll is not installed?");
      } else if(native) {
      DBG_NUM_S("Fall back to internal hull generation. "<< v);
      }
#endif
    }

    if(netz.punkte.size())
    {
      size_t groesse = 0;
      double* band = icc_examin_ns::iccGrenze( profil, intent, groesse );
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
Oyranos::vrmlVonProfil (ICCprofile & profil, int intent, int bpc,
                        int native)
{
  DBG_PROG_START
  Speicher s;
  std::string vrml;
  icc_examin_ns::ICCThreadList<ICCnetz> netze;

  netze.resize(1);

  netzVonProfil( profil, intent, bpc, native,
                                 netze[0] );

  int p_n = netze[0].punkte.size();
  for(int j = 0; j < p_n; ++j)
  {
        double lab[3];

        for(int k = 0; k < 3 ; ++k)
          lab[k] = netze[0].punkte[j].koord[k];

        double * rgb = icc_oyranos.wandelLabNachBildschirmFarben( 0, 0,
                                 lab, 1, 3,
                                 0);
        for(int k = 0; k < 3 ; ++k)
          netze[0].punkte[j].farbe[k] = rgb[k];

        delete [] rgb;
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

    if(native && v < 4)
      vrml = iccCreateVrml ( s, (int)s.size(), intent );
  }
  DBG_PROG_ENDE
  return vrml;
}

ICClist<double>
Oyranos::bandVonProfil (const Speicher & p, int intent)
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

namespace icc_examin_ns {
int
gamutCheckSampler(register WORD In[],
                      register WORD Out[],
                      register LPVOID Cargo)
{
  cmsCIELab Lab1, Lab2;

  cmsLabEncoded2Float(&Lab1, In);
  cmsDoTransform( Cargo, &Lab1, &Lab2, 1 );
  cmsFloat2LabEncoded(Out, &Lab2);

  return TRUE;
}
}

void
Oyranos::gamutCheckAbstract(Speicher & s, Speicher & abstract,
                            int intent, int flags)
{
  DBG_PROG_START
  cmsHPROFILE profil = 0,
              hLab = 0;
  size_t groesse = s.size();
  const char* block = s;

  DBG_MEM_V( (int*) block <<" "<<groesse )

    double start = fortschritt();


      fortschritt(0.2, 0.2);
      hLab  = cmsCreateLabProfile(cmsD50_xyY());
      if(!hLab)  WARN_S( "hLab profil not opened" )

      fortschritt(0.2, 0.2);
      profil = cmsOpenProfileFromMem(const_cast<char*>(block), (DWORD)groesse);
      cmsHTRANSFORM tr1 = cmsCreateProofingTransform  (hLab, TYPE_Lab_DBL,
                                               hLab, TYPE_Lab_DBL,
                                               profil,
                                               intent,
                                               INTENT_RELATIVE_COLORIMETRIC,
                                               PRECALC|flags|cmsFLAGS_HIGHRESPRECALC);
     
#if 0 // Gamut tag
      LPLUT lut = _cmsPrecalculateGamutCheck( tr1 ); DBG
      cmsHPROFILE gmt = _cmsCreateProfilePlaceholder();
      cmsSetDeviceClass( gmt, icSigOutputClass );
      cmsSetColorSpace( gmt, icSigLabData );
      cmsSetPCS( gmt, icSigCmykData );
      _cmsAddLUTTag( gmt, icSigGamutTag, lut ); DBG
      cmsAddTag( gmt, icSigProfileDescriptionTag,  (char*)"GamutCheck");
      _cmsSaveProfile ( gmt,"proof_gamut.icc"); DBG
#endif

      // We calculate the gamut warning for a abstract profile
      cmsHPROFILE tmp = cmsTransform2DeviceLink(tr1,0);
      fortschritt(0.2,0.2);
      LPLUT gmt_lut = cmsAllocLUT(),
            lut = cmsReadICCLut( tmp, icSigAToB0Tag);
      cmsAlloc3DGrid( gmt_lut, lut->cLutPoints, 3, 3);
      DBG_PROG_V( lut->cLutPoints )
      cmsSample3DGrid( gmt_lut, icc_examin_ns::gamutCheckSampler, tr1, 0 );

      fortschritt(0.5,0.2);
      cmsHPROFILE gmt = _cmsCreateProfilePlaceholder();
      cmsSetDeviceClass( gmt, icSigAbstractClass );
      cmsSetColorSpace( gmt, icSigLabData );
      cmsSetPCS( gmt, icSigLabData );
      cmsAddTag( gmt, icSigProfileDescriptionTag,  (char*)"GamutCheck");
      cmsAddTag( gmt, icSigAToB0Tag, gmt_lut );
      _cmsSaveProfileToMem ( gmt, 0, &groesse );
      char* mem = (char*) calloc( sizeof(char), groesse);
      _cmsSaveProfileToMem ( gmt, mem, &groesse );
      abstract.ladeUndFreePtr (&mem, groesse);
      if(gmt) cmsCloseProfile( gmt );
      if(hLab) cmsCloseProfile( hLab );
      if(tmp) cmsCloseProfile( tmp );
      if(tr1) cmsDeleteTransform( tr1 );
      if(gmt_lut) cmsFreeLUT( gmt_lut );
      if(lut) cmsFreeLUT( lut );
    if(start <= 0.0)
      fortschritt(1.1);
 
  DBG_PROG_ENDE
}


int
Oyranos::wandelLabNachProfilUndZurueck(double *lab, // 0.0 - 1.0
                                       size_t  size, int intent, int flags,
                                       Speicher & p )
{
  DBG_PROG_START

  DBG_PROG_V( size <<" "<< intent <<" "<< flags )

    // lcms types
    cmsHPROFILE hProfil = 0,
                hLab = 0;
    cmsHTRANSFORM form = 0;

    size_t groesse = p.size();
    const char*  block = p;
    int flags_ = 0;
    int kanaele, format;
    int input_ausnahme = 0;
    oyProfile_s * profile = oyProfile_FromMem( groesse, (void*)block, 0, 0 );
    oyProfile_s * lab_profile = oyProfile_FromStd( oyEDITING_LAB, 0 );
    

    {
      flags_ = flags & ~cmsFLAGS_GAMUTCHECK;

      // initialising for lcms
      DBG_MEM_V( (int*) block <<" "<<groesse )

      if(groesse)
        hProfil = cmsOpenProfileFromMem(const_cast<char*>(block), (DWORD)groesse);
      else
        WARN_S("no profile found");

      icColorSpaceSignature pcs = cmsGetPCS( hProfil );
      if( pcs != icSigXYZData && pcs != icSigLabData )
        return 1;
      icProfileClassSignature device = cmsGetDeviceClass( hProfil );
      if( device != icSigInputClass && device != icSigDisplayClass &&
          device != icSigOutputClass && device != icSigAbstractClass )
        return 1;

      hLab  = cmsCreateLabProfile(cmsD50_xyY());
      cmsSetColorSpace( hLab, icSigRgbData );
      cmsSetDeviceClass( hLab, icSigInputClass );
      if(!hLab) { WARN_S( "hLab Profil not opened" ); return 1; }

      kanaele = _cmsChannelsOf( cmsGetColorSpace( hProfil ) );
      format = COLORSPACE_SH(PT_ANY) |
               CHANNELS_SH(kanaele) |
               BYTES_SH(0); // lcms_bytes;

#     if HAVE_EXCEPTION
      try {
#     endif
      form = cmsCreateTransform               (hLab, TYPE_RGB_DBL,
                                               hProfil, format,
                                               intent,
                                               PRECALC|flags);
#     if HAVE_EXCEPTION
      }
      catch (std::exception & e) { // catches all from exception
        WARN_S (_("Std-exception occured: ") << e.what());
      }
      catch (...) {       // catches all from exception
        WARN_S (_("Huch, unknown exception"));
      }
#endif

      if (!form) {
        if(device == icSigInputClass && 
           kanaele == 3)
        {
          // use colours instantly
          input_ausnahme = 1;

        } else {
          WARN_S( "no transformation found" );
          return 1;
        }
      }
    }

    double *farben = new double [size * kanaele];
    if(!farben) { WARN_S( "not enough memory available" ); return 1; }

    double *cielab = new double [size * 3];
#ifdef DEBUG
    double *cielab_tmp = new double [size * 3];
#endif


    if(!input_ausnahme)
    {
      cmsDoTransform (form, lab, farben, (unsigned int)size);
      cmsDeleteTransform (form);
#ifdef DEBUG
      memcpy( cielab_tmp, lab, size * 3 * sizeof(double));
#endif
    } else {
      LabToCIELab (lab, cielab, (int)size);
      memcpy( farben, cielab, size * kanaele * sizeof(double)); // why cielab?
    }

    cmsSetColorSpace( hLab, icSigLabData );
    cmsSetDeviceClass( hLab, icSigOutputClass );
    form = cmsCreateTransform                 (hProfil, format,
                                               hLab, TYPE_Lab_DBL,
                                               intent,
                                               PRECALC|flags);
    if (!form) { WARN_S( "no transformation found" ); return 1; }

    cmsDoTransform (form, farben, cielab, (unsigned int)size);
    cmsDeleteTransform (form);

#ifdef DEBUG
    if(kanaele == 4)
    {
      if(icc_debug > 1)
      {
        oyProfile_s * prof = oyProfile_FromStd( oyEDITING_LAB, 0 );
        char * txt = oyDumpColourToCGATS( cielab_tmp, size, prof, malloc,
                                          __FILE__ );
        if(txt)
        {
          saveMemToFile( "icc_oyranos.lab1.cgats", txt, strlen(txt) );
          free(txt);
        }
        txt = oyDumpColourToCGATS( farben, size, profile, malloc,
                                          __FILE__ );
        if(txt)
        {
          saveMemToFile( "icc_oyranos.cmyk.cgats", txt, strlen(txt) );
          free(txt);
        }
        txt = oyDumpColourToCGATS( cielab, size, prof, malloc,
                                          __FILE__ );
        if(txt)
        {
          saveMemToFile( "icc_oyranos.lab2.cgats", txt, strlen(txt) );
          free(txt);
        }
        oyProfile_Release( &prof );
      }
    }
    DBG_NUM_V(kanaele)
#endif

    CIELabToLab (cielab, lab, (int)size);

    if(hProfil)   cmsCloseProfile(hProfil);
    if(hLab)      cmsCloseProfile(hLab);
    oyProfile_Release( &profile );
    oyProfile_Release( &lab_profile );

    if(cielab)    delete [] cielab;
    if(farben)    delete [] farben;

  DBG_PROG_ENDE
  return 0;
}


oyProfile_s * Oyranos::oyMoni (int x, int y)
{
  char * disp_name = 0;
  oyProfile_s * disp_prof = 0;

  static int x_alt = -1, y_alt = -1;
  static oyProfile_s * prof_alt = 0;

  if(x == x_alt && y == y_alt && prof_alt)
    return oyProfile_Copy( prof_alt, 0 );

  disp_name = oyGetDisplayNameFromPosition( 0, x,y, malloc );

  if(disp_name)
  {
    char * moni_profile_name = oyGetMonitorProfileName(disp_name, myAllocFunc);
    if(moni_profile_name)
    {
      disp_prof = oyProfile_FromFile( moni_profile_name, 0, 0 );

    } else {

      size_t size = 0;
      char * buf = oyGetMonitorProfile( disp_name, &size, malloc );
      if(size && buf)
      {
        disp_prof = oyProfile_FromMem( size, buf, 0, 0 );
        free(buf); size = 0;
      } else
      {
        WARN_S("Could not load profile. Use sRGB instead.")
        disp_prof = oyProfile_FromStd( oyASSUMED_WEB, 0 );
      }
    }
    if(disp_name)
      free(disp_name);
    if(moni_profile_name)
      delete [] moni_profile_name;
  }

  if (disp_prof)
  {
    x_alt = x;
    y_alt = y;
    prof_alt = disp_prof;
  }

  return oyProfile_Copy( disp_prof, 0 );
}


double*
Oyranos::wandelLabNachBildschirmFarben(int x, int y,
                                       double *Lab_Speicher, // 0.0 - 1.0
                                       size_t  size, int intent, int flags)
{
  DBG_5_START

  DBG_5_V( size <<" "<< intent <<" "<< flags )

    // lcms types
    cmsHPROFILE hMoni = 0,
                hLab = 0,
                hProof = 0;
    static cmsHTRANSFORM hLabtoRGB = 0;
    static cmsHTRANSFORM h_lab_to_RGB_teuer = 0;
    cmsHTRANSFORM *form = 0;
    double *RGB_Speicher = 0;
    static int flags_ = 0;
    static int intent_ = 0;
    static int x_ = 0;
    static int y_ = 0;

    size_t groesse = 0;
    char*  block = 0;
    static size_t groesse_ = 0;
    static char * block_ = 0;

#ifndef OYRANOS_VERSION
#define OYRANOS_VERSION 0
#endif
#if OYRANOS_VERSION > 106
    static
#if OYRANOS_VERSION > 107
           unsigned
#endif
                     char digest[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
                          dig[16];
#endif

    if(x != x_ || y != y_ || !block_ || !groesse_ || size > 1)
    {
      block = block_ = (char*) moni(x,y, groesse_);
      x_ = x;
      y_ = y;
      groesse = groesse_;
      
      if(block && groesse)
        oyProfileGetMD5(block, groesse, dig);
    }

    if(flags & cmsFLAGS_GAMUTCHECK)
      form = &h_lab_to_RGB_teuer;
    else
      form = &hLabtoRGB;

    double start = fortschritt();

    if(flags_ != (flags & ~cmsFLAGS_GAMUTCHECK) ||
       intent_ != intent ||
#if OYRANOS_VERSION > 106
       (size > 1 && memcmp(digest, dig, 16) != 0) ||
#endif
       !*form )
    {
      flags_ = flags & ~cmsFLAGS_GAMUTCHECK;
      intent_ = intent;
#if OYRANOS_VERSION > 106
      memcpy( digest, dig, 16 );
#endif

      fortschritt(0.05,0.2);

      if (*form)
      {
        cmsDeleteTransform(*form);
        *form = 0;
      }

      // initialising for lcms
      DBG_5_V( (int*) block <<" "<<groesse )

      fortschritt(0.2,0.2);
      if(groesse)
        hMoni = cmsOpenProfileFromMem(block, (DWORD)groesse);
      else
        hMoni = cmsCreate_sRGBProfile();
      if(!hMoni) WARN_S( "hMoni profile nicht opened" )
      hLab  = cmsCreateLabProfile(cmsD50_xyY());
      if(!hLab)  WARN_S( "hLab profile not opened" )

      if(flags & cmsFLAGS_GAMUTCHECK)
      {
        block = const_cast<char*>( proof(groesse) );
        hProof = cmsOpenProfileFromMem(block, (DWORD)groesse);
      }

      fortschritt(0.5,0.2);
      *form = cmsCreateProofingTransform  (hLab, TYPE_Lab_DBL,
                                               hMoni, TYPE_RGB_DBL,
                                               hProof, // simulation profile
                                               intent,
                                               INTENT_RELATIVE_COLORIMETRIC,
                                               PRECALC|BW_COMP|flags);
      if (!*form)
      {
        WARN_S( "no hXYZtoRGB transformation found" )
        DBG_5_ENDE
        return RGB_Speicher;
      }

      if(flags & cmsFLAGS_GAMUTCHECK)
        h_lab_to_RGB_teuer = *form;
      else
        hLabtoRGB = *form;

    }


    RGB_Speicher = new double[size*3];
    if(!RGB_Speicher)  WARN_S( "RGB_speicher Speicher not available" )

    double *cielab = (double*) malloc (sizeof(double)*3*size);
    LabToCIELab (Lab_Speicher, cielab, (int)size);

    cmsDoTransform (*form, cielab, RGB_Speicher, (unsigned int)size);

    if(start <= 0.0)
      fortschritt(1.1);

    if(hMoni)     cmsCloseProfile(hMoni);
    if(hLab)      cmsCloseProfile(hLab);

    if(cielab)    free (cielab);

  DBG_5_ENDE
  return RGB_Speicher;
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
# if HAVE_OY
# endif
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
# ifdef HAVE_OY

# endif
}


