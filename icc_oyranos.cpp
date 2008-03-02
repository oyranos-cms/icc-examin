/*
 * ICC Examin ist eine ICC Profil Betrachter
 * 
 * Copyright (C) 2004-2006  Kai-Uwe Behrmann 
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

void* myAllocFunc(size_t size)
{ 
  return new char [size];
}

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
  DBG_PROG_ENDE
}

Oyranos::~Oyranos()
{
  DBG_PROG_START
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
        // leeren Block einfügen
        Prof_Map_elem teil (profil_name, Speicher());
        Prof_mapIt_bool erg = pspeicher_ .insert( teil );
        if( erg.second = true )
        {
          // Referenz auf Block holen
          Speicher *v_block = &pspeicher_[profil_name];
          *v_block = profil_name;
          size_t size;
          char* block = (char*)oyGetProfileBlock( profil_name, &size, myAllocFunc);
          DBG_PROG_V( (int*)block <<"|"<< size )
          v_block->ladeNew(block, size);
        }
      }
    } else // Profil in Liste
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
        { char *block = (char*)oyGetProfileBlock( profil_name, &size, myAllocFunc);
          if( oyCheckProfileMem( block, size, 0 ) )
            WARN_S ( _("WARNING: Could not load corrupt or damaged profile.") )
          else {
            DBG_PROG_V( (intptr_t)block <<"|"<< size )
            v_block->ladeNew(block, size);
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
  // Alle Bestandteile einsammeln
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
};

OSErr
MyFlattenProfileProcSize (
   SInt32 command, 
   SInt32 *size, 
   void *data, 
   void *refCon)
{
  // Alle Bestandteile einsammeln
  if(*size)
  {
    refcon *ref = (refcon*) refCon;
    ref->size += *size;
  }
  DBG_PROG_V(command<<" "<<*size)

  return 0;
};

typedef  void* (oyAllocFunc_t)(size_t size);

int
oyGetProfileBlockOSX (CMProfileRef prof, char *block, size_t *size, oyAllocFunc_t allocate_func)
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
# if HAVE_OY
  {
    size_t size = 0;

    const char *display_name = 0;
    
    display_name = XDisplayString( fl_display );  // gehoert X
    DBG_PROG_V( display_name )
    int screen = oyGetScreenFromPosition( display_name, x,y );
    char *new_display_name = changeScreenName_( display_name, screen );
    char* moni_profil = oyGetMonitorProfile( new_display_name, &size, myAllocFunc );
    if(new_display_name) { delete [] new_display_name; new_display_name = 0; }

    Speicher v_block = moni_;
      DBG_MEM_V( v_block.size() )
    const char *profil_name=_("Monitor Profile");
      DBG_PROG_V( (int*)profil_name <<" "<< profil_name )
    // Wir holen das Profil wenn es sich geaendert hat.
    // a) neuer Name  - schwierig zu identifizieren ueber oyDeviceProfil
    // b) Benachrichtigung  - vorausgesetzt Oyranos (X?) aendert das Profil
    { 
        if (size)
        {
          char* block = moni_profil;
          if( oyCheckProfileMem( block, size, 0 ) )
            WARN_S(_("WARNING: Could not load corrupt or damaged profile."))
          else {
              DBG_MEM_V( (int*)block <<"|"<< size )
            v_block.ladeNew(block, size);
          }
        } else
          WARN_S(_("WARNING: Could not load profile."))
        v_block = (const char*)profil_name;
        DBG_MEM
    }
  }
  DBG_NUM_S( "Monitorprofil = "<< moni_.name() <<" "<< moni_.size() <<"\n" )

# else
#   ifdef APPLE
    CMProfileRef prof=NULL;
    DisplayIDType screenID=0;
    GDHandle device = DMGetFirstScreenDevice(true); //GetDeviceList();
    DMGetDisplayIDByGDevice(device, &screenID, false);
    CMGetProfileByAVID(screenID, &prof);
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
    CMError err = CMFlattenProfile ( prof, 0, MyFlattenProfileProc, &ref, &bol);
    err = 0;
    Str255 str;
    ScriptCode code;
    CMGetScriptProfileDescription(prof, str, &code);
    //CFStringRef cfstring;
    //CFStringGetPascalString( cfstring, str, 255, kCFStringEncodingASCII);
    //cfstring = CFStringCreateWithPascalString( NULL, str, kCFStringEncodingASCII);
    DBG_PROG_V( (int)str[0] )
	if (prof) CMCloseProfile(prof);
    //const char *profil_name = CFStringGetCStringPtr(cfstring, kCFStringEncodingASCII);
    const char *profil_name = str; ++profil_name;
    DBG_PROG_V( screenID )

    //char* profil_name = (char*)malloc(strlen(str));
    //sprintf(profil_name, str);
    Speicher v_block = moni_;
      DBG_MEM_V( v_block.size() )
      DBG_PROG_V( (int*)profil_name <<" "<< profil_name )
    if( profil_name &&
        v_block != profil_name )
    { 
        size_t size = ref.size;;
          DBG_MEM_V( size )
        if (size)
        {
#         ifdef HAVE_OY
          if( oyCheckProfileMem( ref.data, size, 0 ) )
            WARN_S(_("WARNING: Could not load profile."))
          else
#         endif
          {
              DBG_MEM_V( (int*)ref.data <<"|"<< size )
            v_block.lade((const char*)ref.data, size);
          }
        } else
          WARN_S(_("WARNING: Could not load profile."))
        v_block = profil_name;
        //if(profil_name) free(profil_name);
        DBG_PROG_V( v_block.name() )

        DBG_MEM
    }
    //CFRelease(cfstring);
#   endif
# endif
  DBG_PROG_ENDE
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
        { char *block = (char*)oyGetProfileBlock( profil_name, &size, myAllocFunc);
          if( oyCheckProfileMem( block, size, 0 ) )
            WARN_S(_("WARNING: Could not load profile."))
          else {
            DBG_PROG_V( (int*)block <<"|"<< size )
            v_block->ladeNew(block, size);
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
  oyGetProfileBlockOSX(prof, block, &groesse, myAllocFunc);
  if(groesse) {
    block = (char*)malloc(groesse);
    oyGetProfileBlockOSX(prof, block, &groesse, myAllocFunc);
  }
  Speicher *v_block = &rgb_;
  v_block->lade(block,groesse);
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
        { char *block = (char*)oyGetProfileBlock( profil_name, &size, myAllocFunc);
          if( oyCheckProfileMem( block, size, 0 ) )
            WARN_S(_("WARNING: Could not load profile."))
          else {
            DBG_PROG_V( (int*)block <<"|"<< size )
            v_block->ladeNew(block, size);
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
        { char *block = (char*)oyGetProfileBlock( profil_name, &size, myAllocFunc);
          if( oyCheckProfileMem( block, size, 0 ) )
            WARN_S(_("WARNING: Could not load profile."))
          else {
            DBG_PROG_V( (int*)block <<"|"<< size )
            v_block->ladeNew(block, size);
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

  display_name = XDisplayString( fl_display );  // gehoert X
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

  display_name = XDisplayString( fl_display );  // gehoert FLTK
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
    infos[2 * *num] = strdup(_("Manufacturer:"));
    infos[2 * *num + 1] = strdup( manufacturer );
    *num += 1;
  }
  if( model && strlen( model ) )
  {
    infos[2 * *num] = strdup(_("Model:"));
    infos[2 * *num + 1] = strdup( model );
    *num += 1;
  }
  if( serial && strlen( serial ) )
  {
    infos[2 * *num] = strdup(_("Serial:"));
    infos[2 * *num + 1] = strdup( serial );
    *num += 1;
  }
    

  if(new_display_name) { delete [] new_display_name; new_display_name = 0; }

# endif

  DBG_PROG_ENDE
  return infos;
}

#include "icc_vrml.h"
#include "icc_gamut.h"
std::vector<ICCnetz>
Oyranos::netzVonProfil (ICCprofile & profil, int intent)
{
  DBG_PROG_START
  Speicher s;
  std::vector<ICCnetz> netz;

  if(profil.valid()) {
      size_t groesse = 0;
      char* daten = profil.saveProfileToMem(&groesse);
      DBG_NUM_V( groesse );
      s.ladeUndFreePtr(&daten, groesse);
  }

  if(s.size())
  {
    std::string vrml;
    vrml = icc_create_vrml ( s,s.size(), intent );
    netz = netzAusVRML (vrml);

    if(netz.size())
    {
      size_t groesse = 0;
      double* band = icc_examin_ns::iccGrenze( profil, intent, groesse );
      DBG_PROG_V( (int*) band <<" "<< groesse )

      netz[0].umriss.resize( groesse );

      for(int i = 0; i < (int)groesse; ++i)
        for(int j = 0; j < 3; ++j)
          netz[0].umriss[i].koord[j] = band[i*3+j];
      if(band) delete [] band;
    }
  }
  DBG_PROG_ENDE
  return netz;
}

std::string
Oyranos::vrmlVonProfil (ICCprofile & profil, int intent)
{
  DBG_PROG_START
  Speicher s;
  std::string vrml;

  if(profil.valid()) {
      size_t groesse = 0;
      char* daten = profil.saveProfileToMem(&groesse); 
      s.ladeUndFreePtr(&daten, groesse);
      DBG_NUM_V( groesse );
  }

  if(s.size())
  {
    vrml = icc_create_vrml ( s, s.size(), intent );
  }
  DBG_PROG_ENDE
  return vrml;
}

std::vector<double>
Oyranos::bandVonProfil (const Speicher & p, int intent)
{
  DBG_PROG_START
# ifdef DEBUG
  const char* b = p;
#endif
  DBG_MEM_V( (int*)b )
  std::vector<double> band; // TODO
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

      fortschritt(0.2);
      hLab  = cmsCreateLabProfile(cmsD50_xyY());
      if(!hLab)  WARN_S( "hLab Profil nicht geoeffnet" )

      fortschritt(0.3);
      profil = cmsOpenProfileFromMem(const_cast<char*>(block), groesse);
      cmsHTRANSFORM tr1 = cmsCreateProofingTransform  (hLab, TYPE_Lab_DBL,
                                               hLab, TYPE_Lab_DBL,
                                               profil,
                                               intent,
                                               INTENT_RELATIVE_COLORIMETRIC,
                                               flags|cmsFLAGS_HIGHRESPRECALC);
     
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

      // Wir berechnen die Farbhuellwarnung fuer ein abstraktes Profil
      cmsHPROFILE tmp = cmsTransform2DeviceLink(tr1,0);
      fortschritt(0.4);
      LPLUT gmt_lut = cmsAllocLUT(),
            lut = cmsReadICCLut( tmp, icSigAToB0Tag);
      cmsAlloc3DGrid( gmt_lut, lut->cLutPoints, 3, 3);
      DBG_PROG_V( lut->cLutPoints )
      cmsSample3DGrid( gmt_lut, icc_examin_ns::gamutCheckSampler, tr1, 0 );

      fortschritt(0.5);
      cmsHPROFILE gmt = _cmsCreateProfilePlaceholder();
      cmsSetDeviceClass( gmt, icSigAbstractClass );
      cmsSetColorSpace( gmt, icSigLabData );
      cmsSetPCS( gmt, icSigLabData );
      cmsAddTag( gmt, icSigProfileDescriptionTag,  (char*)"GamutCheck");
      cmsAddTag( gmt, icSigAToB0Tag, gmt_lut );
      _cmsSaveProfileToMem ( gmt, 0, &groesse );
      char* mem = (char*) calloc( sizeof(char), groesse);
      _cmsSaveProfileToMem ( gmt, mem, &groesse );
      abstract.ladeUndFreePtr (&mem, groesse); DBG
      fortschritt(1.);
      if(gmt) cmsCloseProfile( gmt );
      if(hLab) cmsCloseProfile( hLab );
      if(tmp) cmsCloseProfile( tmp );
      if(tr1) cmsDeleteTransform( tr1 );
      if(gmt_lut) cmsFreeLUT( gmt_lut );
      if(lut) cmsFreeLUT( lut );
      fortschritt(1.1);
 
  DBG_PROG_ENDE
}


double*
Oyranos::wandelLabNachBildschirmFarben(double *Lab_Speicher, // 0.0 - 1.0
                                       size_t  size, int intent, int flags)
{
  DBG_PROG_START

  DBG_PROG_V( size <<" "<< intent <<" "<< flags )

    // lcms Typen
    cmsHPROFILE hMoni = 0,
                hLab = 0,
                hProof = 0;
    static cmsHTRANSFORM hLabtoRGB = 0;
    static cmsHTRANSFORM h_lab_to_RGB_teuer = 0;
    cmsHTRANSFORM form = 0;
    double *RGB_Speicher = 0;
    static  char digest[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, dig[16];
    static int flags_ = 0;

    size_t groesse = 0;
    char*  block = (char*) moni(0,0, groesse);

    oyProfileGetMD5(block, groesse, dig);

    if(flags & cmsFLAGS_GAMUTCHECK)
      form = h_lab_to_RGB_teuer;
    else
      form = hLabtoRGB;


    if(flags_ != (flags & ~cmsFLAGS_GAMUTCHECK) ||
       memcmp(digest, dig, 16) != 0 ||
       !form )
    {
      flags_ = flags & ~cmsFLAGS_GAMUTCHECK;
      memcpy( digest, dig, 16 );

      fortschritt(0.05);

      if (form)
      {
        cmsDeleteTransform(form);
        form = 0;
      }

      // Initialisierung fuer lcms
      DBG_MEM_V( (int*) block <<" "<<groesse )

      fortschritt(0.2);
      if(groesse)
        hMoni = cmsOpenProfileFromMem(block, groesse);
      else
        hMoni = cmsCreate_sRGBProfile();
      if(!hMoni) WARN_S( "hMoni Profil nicht geoeffnet" )
      hLab  = cmsCreateLabProfile(cmsD50_xyY());
      if(!hLab)  WARN_S( "hLab Profil nicht geoeffnet" )

      if(flags & cmsFLAGS_GAMUTCHECK)
      {
        block = const_cast<char*>( proof(groesse) );
        hProof = cmsOpenProfileFromMem(block, groesse);
      }

      fortschritt(0.5);
      form = cmsCreateProofingTransform  (hLab, TYPE_Lab_DBL,
                                               hMoni, TYPE_RGB_DBL,
                                               hProof, // Simulationsprofil
                                               intent,
                                               INTENT_RELATIVE_COLORIMETRIC,
                                               PRECALC|BW_COMP|flags);
      if (!form) WARN_S( "keine hXYZtoRGB Transformation gefunden" )

      if(flags & cmsFLAGS_GAMUTCHECK)
        h_lab_to_RGB_teuer = form;
      else
        hLabtoRGB = form;
    }


    fortschritt(0.9);

    RGB_Speicher = new double[size*3];
    if(!RGB_Speicher)  WARN_S( "RGB_speicher Speicher nicht verfuegbar" )

    double *cielab = (double*) malloc (sizeof(double)*3*size);
    LabToCIELab (Lab_Speicher, cielab, size);

    cmsDoTransform (form, cielab, RGB_Speicher, size);

    if(hMoni)     cmsCloseProfile(hMoni);
    if(hLab)      cmsCloseProfile(hLab);

    if(cielab)    free (cielab);

  fortschritt(1.1);

  DBG_PROG_ENDE
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
  // Schlüssel erzeugen
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


