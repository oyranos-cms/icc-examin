/*
 * ICC Examin ist eine ICC Profil Betrachter
 * 
 * Copyright (C) 2004-2007  Kai-Uwe Behrmann 
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

void* myAllocFunc(size_t size)
{
  return new char [size];
}

void myDeAllocFunc(void * buf)
{
  delete [] (char*)buf;
}

/** @brief number of channels in a colour space
 *
 *  since: (ICC Examin: version 0.45)
 */
int
oyColourSpaceGetChannelCount ( icColorSpaceSignature color )
{
  int n;

  switch (color) {
    case icSigXYZData: n = 3; break;
    case icSigLabData: n = 3; break;
    case icSigLuvData: n = 3; break;
    case icSigYCbCrData: n = 3; break;
    case icSigYxyData: n = 3; break;
    case icSigRgbData: n = 3; break;
    case icSigGrayData: n = 1; break;
    case icSigHsvData: n = 3; break;
    case icSigHlsData: n = 3; break;
    case icSigCmykData: n = 4; break;
    case icSigCmyData: n = 3; break;
    case icSig2colorData: n = 2; break;
    case icSig3colorData: n = 3; break;
    case icSig4colorData: n = 4; break;
    case icSig5colorData:
    case icSigMCH5Data:
          n = 5; break;
    case icSig6colorData:
    case icSigMCH6Data:
         n = 6; break;
    case icSig7colorData:
    case icSigMCH7Data:
         n = 7; break;
    case icSig8colorData:
    case icSigMCH8Data:
         n = 8; break;
    case icSig9colorData:
    case icSigMCH9Data:
         n = 9; break;
    case icSig10colorData:
    case icSigMCHAData:
         n = 10; break;
    case icSig11colorData:
    case icSigMCHBData:
         n = 11; break;
    case icSig12colorData:
    case icSigMCHCData:
         n = 12; break;
    case icSig13colorData:
    case icSigMCHDData:
         n = 13; break;
    case icSig14colorData:
    case icSigMCHEData:
         n = 14; break;
    case icSig15colorData:
    case icSigMCHFData:
         n = 15; break;
    default: n = 0; break;
  }
  return n;
}

/** @brief name of a colour space
 *
 *  since: (ICC Examin: version 0.45)
 */
const char *
oyColourSpaceGetName( icColorSpaceSignature sig )
{
  const char * text;

  switch (color) {
    case icSigXYZData: text =_("XYZ"); break;
    case icSigLabData: text =_("Lab"); break;
    case icSigLuvData: text =_("Luv"); break;
    case icSigYCbCrData: text =_("YCbCr"); break;
    case icSigYxyData: text =_("Yxy"); break;
    case icSigRgbData: text =_("Rgb"); break;
    case icSigGrayData: text =_("Gray"); break;
    case icSigHsvData: text =_("Hsv"); break;
    case icSigHlsData: text =_("Hls"); break;
    case icSigCmykData: text =_("Cmyk"); break;
    case icSigCmyData: text =_("Cmy"); break;
    case icSig2colorData: text =_("2color"); break;
    case icSig3colorData: text =_("3color"); break;
    case icSig4colorData: text =_("4color"); break;
    case icSigMCH5Data:
    case icSig5colorData: text =_("5color"); break;
    case icSigMCH6Data:
    case icSig6colorData: text =_("6color"); break;
    case icSigMCH7Data:
    case icSig7colorData: text =_("7color"); break;
    case icSigMCH8Data:
    case icSig8colorData: text =_("8color"); break;
    case icSigMCH9Data:
    case icSig9colorData: text =_("9color"); break;
    case icSigMCHAData:
    case icSig10colorData: text =_("10color"); break;
    case icSigMCHBData:
    case icSig11colorData: text =_("11color"); break;
    case icSigMCHCData:
    case icSig12colorData: text =_("12color"); break;
    case icSigMCHDData:
    case icSig13colorData: text =_("13color"); break;
    case icSigMCHEData:
    case icSig14colorData: text =_("14color"); break;
    case icSigMCHFData:
    case icSig15colorData: text =_("15color"); break;
    default: { icUInt32Number i = icValue(color);
               char t[8];
               memcpy (t,(char*)&i, 4);
               t[4] = '?';
               t[5] = 0;
               text = &t[0];
               break;
             }
  }
  return text;
}

/** @brief copy pure colours

    handle colour only, without from set to -1 default
 *
 *  since: (ICC Examin: version 0.45)
 */
void
oyCopyColour ( double * from, double * to, int n, icColorSpaceSignature sig )
{
  int i, j;
  int c = oyColourSpaceGetChannelCount( sig );

  if(!n || !to)
    return;

  if(from)
    memcpy( to, from, sizeof(double) * n * c );
  else
    switch(sig)
    {
      case icSigLabData:
      case icSigLuvData:
      case icSigHsvData:
      case icSigHlsData:
      case icSigYCbCrData:
           for( i = 0; i < n; ++i )
             for( j = 0; j < c; ++j )
               to[i*c+0] = -1;
               to[i*c+1] = 0;
               to[i*c+2] = 0;
           break;
      case icSigRgbData:
      default:
           for( i = 0; i < n*c; ++i )
             to[i] = -1;
           break;
    }
}

/** @brief manage complex oyNamedColour_s inside Oyranos
 *
 *  since: (ICC Examin: version 0.45)
 */
oyNamedColour_s*
oyNamedColourCreate( double * lab, double * chan, icColorSpaceSignature sig,
                     const char **names_chan, const char *name,
                     const char *name_long, char *nick,
                     const char  *blob, int blob_len, const char *ref_file,
                     oyAllocFunc_t allocateFunc,
                     oyDeAllocFunc_t deallocateFunc )
{
  oyNamedColour_s * colour = (oyNamedColour_s*)allocateFunc(sizeof(oyNamedColour_s));
  int i;

  if(!colour)
  {
    WARN_S("MEM Error.")
    return NULL;
  }

  colour->sig = sig;

  if(deallocateFunc)
    colour->deallocateFunc = deallocateFunc;
  else
    colour->deallocateFunc = myDeAllocFunc;

  if(allocateFunc)
    colour->allocateFunc = allocateFunc;
  else
    colour->allocateFunc = myAllocFunc;

  oyCopyColour( lab, &colour->lab[0], 1, icSigLabData );

  return colour;
}

void
oyNamedColourGetLab ( oyNamedColour_s * colour,
                      double * lab )
{
  int i;
  if(lab)
    for(i=0;i<3;++i) lab[i] = colour->lab[i];
}

void
oyNamedColourSetLab ( oyNamedColour_s * colour,
                      double * lab )
{
  int i;
  if(lab)
  {
    for(i=0;i<3;++i)
    {
      colour->lab[i] = lab[i];
      colour->moni_rgb[i] = -1f;
    }
    for(i=0;i<32;++i)
      colour->channels[i] = -1f;
  }
}

/** @brief get associated colour name
 *
 *
 *  @param[in]    Oyranos colour struct pointer
 *
 *  since: (ICC Examin: version 0.45)
 */
const char *
oyNamedColourGetName( oyNamedColour_s * colour )
{
  icColorSpaceSignature sig = colour->sig;

  if(colour->name)
    return colour->name;

  colour->name = colour->allocateFunc(80);
  snprintf( colour->name, 80, "%s: %.02f %.02f %.02f",
            oyColourSpaceGetName(sig),
            colour->lab[0], colour->lab[1], colour->lab[2] );
    
  return colour->name;
}

/** @brief get associated colour nick name
 *
 *
 *  @param[in]    Oyranos colour struct pointer
 *
 *  since: (ICC Examin: version 0.45)
 */
const char *
oyNamedColourGetNick( oyNamedColour_s * colour )
{
  return colour->nick_name;
}

/** @brief get associated colour description
 *
 *
 *  @param[in]    Oyranos colour struct pointer
 *
 *  since: (ICC Examin: version 0.45)
 */
const char *
oyNamedColourGetDescription( oyNamedColour_s * colour )
{
  return colour->name_long;
}

/** @brief release correctly
 *
 *  set pointer to zero
 *
 *  @param[in]    adress of Oyranos colour struct pointer
 *
 *  since: (ICC Examin: version 0.45)
 */
void
oyNamedColourRelease( oyNamedColour_s ** colour )
{
  //TODO
  *colour->deallocate_func( *colour );
  *colour = NULL;
}



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
          char* block = (char*)oyGetProfileBlock( profil_name, &size, myAllocFunc);
          DBG_PROG_V( (int*)block <<"|"<< size )
          v_block->ladeNew(block, size);
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
    if(!allocateFunc) {
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
        block = allocateFunc( *size );
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
# if HAVE_OY && !defined(APPLE)
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
    // We take the profile only if it had changed.
    // a) new name  - difficult to identify with oyDeviceProfil
    // b) notification  - provided Oyranos (X?) changes the profile
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
          DBG_S("Could not load profile.")
        v_block = (const char*)profil_name;
        DBG_MEM
    }
  }
  DBG_NUM_S( "monitor profile = "<< moni_.name() <<" "<< moni_.size() <<"\n" )

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
    const char *profil_name = (const char*) str; ++profil_name;
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
        // "Monitor Profile" will be handled special on other places
        v_block = _("Monitor Profile");//profil_name;
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

/** generates a gamut hull */
std::string
Oyranos::netzVonProfil_ (std::vector<ICCnetz> & netze,
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

      lab[pos + 0] = 1.0 - (double)y/(double)a;

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
        lab[pos + 0] = 0.0;
        lab[pos + 1] = min + (x_pos - (b - 1) * (a - 1))/(double)a * (max-min);
        lab[pos + 2] = min + val;
      }
    }

  if(wandelProfilNachLabUndZurueck( lab, size, intent, bpc, profil ))
    return  std::string("oyranos");
  double * rgb = wandelLabNachBildschirmFarben( lab, size, 0, 0 );

  // initialise mesh
  netze.resize(1);
  // collect colour points
  netze[0].punkte. resize( size );
  for(size_t i = 0; i < size; ++i)
  {
    for(int k = 0; k < kanaele; ++k)
    {
      netze[0].punkte[i].koord[k] = lab [i*kanaele+k];
      netze[0].punkte[i].farbe[k] = rgb [i*kanaele+k];
    }
    netze[0].punkte[i].farbe[kanaele] = 1.0;
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
      netze[0].indexe. insert( index_p );

                                         index_p.second.i[0] =  y *  4*a+x_+1;
      index_p.second.i[2] = (y+1)*4*a+x; index_p.second.i[1] = (y+1)*4*a+x_+1;
      netze[0].indexe. insert( index_p );
    }

  for(int y = 0; y < a; ++y)
  {
    int off = 4 * a * (a + 1);
    if(0 < y && y < a - 1)
    {
      // 0 0 .
      index_p.second.i[2] = off-y;  index_p.second.i[0] = off+(y+1)*2*(a-1)-a+1;
      index_p.second.i[1] = off-y-1;
      netze[0].indexe. insert( index_p );

                                    index_p.second.i[0] = off+(y+0)*2*(a-1)-a+1;
      index_p.second.i[2] = off-y;  index_p.second.i[1] = off+(y+1)*2*(a-1)-a+1;
      netze[0].indexe. insert( index_p );

      // 0 1 .
      index_p.second.i[1] = off+(y+1)*2*(a-1)-1; index_p.second.i[0] =off-3*a+y+1;
      index_p.second.i[2] = off+(y)*2*(a-1)-1;
      netze[0].indexe. insert( index_p );

                                               index_p.second.i[1] =off-3*a+y+1;
      index_p.second.i[2] = off+(y)*2*(a-1)-1; index_p.second.i[0] = off-3*a+y;
      netze[0].indexe. insert( index_p );

      // 1 0 .
                          index_p.second.i[0] = off+2*(a-1)*(a-1)-(y+1)*2*(a-1);
      index_p.second.i[1] = 3*a+y+1; index_p.second.i[2] = off+2*(a-1)*(a-1)-y*2*(a-1);
      netze[0].indexe. insert( index_p );

      index_p.second.i[0] = 3*a+y+1;  index_p.second.i[2] = off+2*(a-1)*(a-1)-y*2*(a-1);
      index_p.second.i[1] = 3*a+y;
      netze[0].indexe. insert( index_p );

      // 1 1 .
      index_p.second.i[0] = off+2*(a-1)*(a-1)-(y+1)*2*(a-1)+a-2;
      index_p.second.i[1] = off+2*(a-1)*(a-1)-(y+0)*2*(a-1)+a-2; index_p.second.i[2] = 2*a-y;
      netze[0].indexe. insert( index_p );

                                         index_p.second.i[0] = 2*a-y-1;
      index_p.second.i[1] = off+2*(a-1)*(a-1)-(y+1)*2*(a-1)+a-2; index_p.second.i[2] = 2*a-y;
      netze[0].indexe. insert( index_p );
    }

    for(int x = 0; x < 2 * a; ++x)
    {
      int x_ = x + off;
      int b = 0; // area

          // 1 0 0 (L a b)
          index_p.second.i[0] = 4*a-1;   index_p.second.i[1] = off;
          index_p.second.i[2] = 0;
          netze[0].indexe. insert( index_p );

                                         index_p.second.i[0] = off;
          index_p.second.i[2] = 0;       index_p.second.i[1] = 1;
          netze[0].indexe. insert( index_p );

          // 0 0 0
          index_p.second.i[1] = off-1;   index_p.second.i[0] = off+a-1;
          index_p.second.i[2] = off-4*a;
          netze[0].indexe. insert( index_p );

                                         index_p.second.i[1] = off+a-1;
          index_p.second.i[2] = off-4*a; index_p.second.i[0] = off - 4*a+1;
          netze[0].indexe. insert( index_p );

          // 0 0 1
          index_p.second.i[2] = off-a; index_p.second.i[1] = off-a-1;
                                    index_p.second.i[0] = off+2*(a-1)*(a-1)-a+1;
          netze[0].indexe. insert( index_p );

          index_p.second.i[0] = off-a;
          index_p.second.i[1] = off-a+1; index_p.second.i[2] = off+2*(a-1)*(a-1)-a+1;
          netze[0].indexe. insert( index_p );

          // 0 1 1
          index_p.second.i[0] = off-2*a+1; index_p.second.i[2] = off-2*a;
          index_p.second.i[1] = off+2*(a-1)*(a-1)-1;
          netze[0].indexe. insert( index_p );

                                         index_p.second.i[1] = off-2*a;
          index_p.second.i[2] = off+2*(a-1)*(a-1)-1; index_p.second.i[0] = off-2*a-1;
          netze[0].indexe. insert( index_p );


          // 1 1 1
          index_p.second.i[0] = 2*a-1;   index_p.second.i[2] = 2*a;
          index_p.second.i[1] = off+2*(a-1)*(a-1)-a;
          netze[0].indexe. insert( index_p );

                                         index_p.second.i[1] = 2*a;
          index_p.second.i[2] = off+2*(a-1)*(a-1)-a;index_p.second.i[0] = 2*a+1;
          netze[0].indexe. insert( index_p );

          // 1 0 1
          index_p.second.i[2] = 3*a;   index_p.second.i[0] = 3*a-1;
                                  index_p.second.i[1] = off+2*(a-1)*(a-1)-2*a+2;
          netze[0].indexe. insert( index_p );

          index_p.second.i[2] = 3*a;
          index_p.second.i[1] = 3*a+1; index_p.second.i[0] = off+2*(a-1)*(a-1)-2*a+2;
          netze[0].indexe. insert( index_p );

          // 1 1 0
          index_p.second.i[0] = off+a-2; index_p.second.i[1] = a + 1;
          index_p.second.i[2] = a - 1;
          netze[0].indexe. insert( index_p );

                                         index_p.second.i[0] = a + 1;
          index_p.second.i[2] = a - 1;   index_p.second.i[1] = a;
          netze[0].indexe. insert( index_p );

          // 0 1 0
          index_p.second.i[0] = off+2*(a-1)-1;
          index_p.second.i[1] = off-3*a-1; index_p.second.i[2] = off-3*a;
          netze[0].indexe. insert( index_p );

          index_p.second.i[1] = off+2*(a-1)-1; index_p.second.i[0] = off-3*a+1;
                                         index_p.second.i[2] = off-3*a+0;
          netze[0].indexe. insert( index_p );

      // lower border
      if( y == 0 )
      {
        if(x == 0) {
        } else if(x == a - 1) {
        } else if(x < a - 1) {
          // 1 . 0
          index_p.second.i[0] = off + x - 1; index_p.second.i[1] = off + x;
          index_p.second.i[2] = x;
          netze[0].indexe. insert( index_p );

                                         index_p.second.i[0] = off + x;
          index_p.second.i[2] = x;       index_p.second.i[1] = x + 1;
          netze[0].indexe. insert( index_p );

          // 0 . 1
          index_p.second.i[0] = off-a-x; index_p.second.i[2] = off-a-x-1;
          index_p.second.i[1] = off+2*(a-1)*(a-1)-a+x;
          netze[0].indexe. insert( index_p );

                                         index_p.second.i[2] = off-a-x-1;
          index_p.second.i[0] = off+2*(a-1)*(a-1)-a+x; index_p.second.i[1] = off+2*(a-1)*(a-1)-a+x+1;
          netze[0].indexe. insert( index_p );

          // 1 . 1
          index_p.second.i[0] = 3*a - x; index_p.second.i[1] = 3*a - x-1;
          index_p.second.i[2] = off+2*(a-1)*(a-1)-2*(a-1)+x-1;
          netze[0].indexe. insert( index_p );

                                         index_p.second.i[0] = 3*a - x-1;
          index_p.second.i[2] = off+2*(a-1)*(a-1)-2*(a-1)+x-1; index_p.second.i[1] = off+2*(a-1)*(a-1)-2*(a-1)+x;
          netze[0].indexe. insert( index_p );

        } else if (x > a + 1) {
          // 0 . 0
          index_p.second.i[0] = off+x-3; index_p.second.i[2] = off+x-3+1;
          index_p.second.i[1] = 4*a*(a+1)-4*a + x-a-1;
          netze[0].indexe. insert( index_p );

                                         index_p.second.i[1] = off+x-3+1;
          index_p.second.i[2] = 4*a*(a+1)-4*a + x-a-1; index_p.second.i[0] = 4*a*(a+1)-4*a + x-a;
          netze[0].indexe. insert( index_p );
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
        netze[0].indexe. insert( index_p );

                                                index_p.second.i[2] = (y-1)*2*(a-1)+x_+1;
        index_p.second.i[0] = (y+0)*2*(a-1)+x_; index_p.second.i[1] = (y+0)*2*(a-1)+x_+1;
        netze[0].indexe. insert( index_p );

      } else if(b * (a - b) <= x && x < ++b * a - b - 1) {
        // lower middle field (*L=1.0)
        index_p.second.i[0] = (y-1) *  2*(a-1)+x_; index_p.second.i[1] =  (y-1)*2*(a-1)+x_+1;
        index_p.second.i[2] = (y+0)*2*(a-1)+x_;
        netze[0].indexe. insert( index_p );

                                                index_p.second.i[0] = (y-1)*2*(a-1)+x_+1;
        index_p.second.i[2] = (y+0)*2*(a-1)+x_; index_p.second.i[1] = (y+0)*2*(a-1)+x_+1;
        netze[0].indexe. insert( index_p );
      }
    }
  }

  netze[0].kubus = 1;

  delete [] rgb;
  delete [] lab;

  DBG_PROG_ENDE
  return std::string("oyranos");
}


#include "icc_vrml.h"
#include "icc_gamut.h"
std::vector<ICCnetz>
Oyranos::netzVonProfil (ICCprofile & profil, int intent, int bpc)
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
    std::string t;
    if(!netz.size())
    {
      t = netzVonProfil_(netz, s, intent, bpc);
      WARN_S("Fall back to internal hull generation. Argyll is not installed?");
    }

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

    double start = fortschritt();


      fortschritt(0.2, 0.2);
      hLab  = cmsCreateLabProfile(cmsD50_xyY());
      if(!hLab)  WARN_S( "hLab profil not opened" )

      fortschritt(0.2, 0.2);
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
      abstract.ladeUndFreePtr (&mem, groesse); DBG
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
Oyranos::wandelProfilNachLabUndZurueck(double *lab, // 0.0 - 1.0
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

    {
      flags_ = flags & ~cmsFLAGS_GAMUTCHECK;

      // initialising for lcms
      DBG_MEM_V( (int*) block <<" "<<groesse )

      if(groesse)
        hProfil = cmsOpenProfileFromMem(const_cast<char*>(block), groesse);
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
      if(!hLab) { WARN_S( "hLab Profil not opened" ); return 1; }

      kanaele = oyColourSpaceGetChannelCount( cmsGetColorSpace( hProfil ) );
      format = COLORSPACE_SH(PT_ANY) |
               CHANNELS_SH(kanaele) |
               BYTES_SH(0); // lcms_bytes;

      form = cmsCreateTransform               (hLab, TYPE_Lab_DBL,
                                               hProfil, format,
                                               intent,
                                               PRECALC|flags);
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

    LabToCIELab (lab, cielab, size);

    if(!input_ausnahme)
    {
      cmsDoTransform (form, cielab, farben, size);
      cmsDeleteTransform (form);
    } else {
      memcpy( farben, cielab, size * kanaele * sizeof(double)); // why cielab?
    }

    form = cmsCreateTransform                 (hProfil, format,
                                               hLab, TYPE_Lab_DBL,
                                               intent,
                                               PRECALC|flags);
    if (!form) { WARN_S( "no transformation found" ); return 1; }

    cmsDoTransform (form, farben, cielab, size);
    cmsDeleteTransform (form);

    CIELabToLab (cielab, lab, size);

    if(hProfil)   cmsCloseProfile(hProfil);
    if(hLab)      cmsCloseProfile(hLab);

    if(cielab)    delete [] cielab;
    if(farben)    delete [] farben;

  DBG_PROG_ENDE
  return 0;
}


double*
Oyranos::wandelLabNachBildschirmFarben(double *Lab_Speicher, // 0.0 - 1.0
                                       size_t  size, int intent, int flags)
{
  DBG_PROG_START

  DBG_PROG_V( size <<" "<< intent <<" "<< flags )

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

    size_t groesse = 0;
    char*  block = (char*) moni(0,0, groesse);
#ifndef OYRANOS_VERSION
#define OYRANOS_VERSION 0
#endif
#if OYRANOS_VERSION > 106
    static  char digest[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, dig[16];
    if(block && groesse)
      oyProfileGetMD5(block, groesse, dig);
#endif

    if(flags & cmsFLAGS_GAMUTCHECK)
      form = &h_lab_to_RGB_teuer;
    else
      form = &hLabtoRGB;

    double start = fortschritt();

    if(flags_ != (flags & ~cmsFLAGS_GAMUTCHECK) ||
       intent_ != intent ||
#if OYRANOS_VERSION > 106
       memcmp(digest, dig, 16) != 0 ||
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
      DBG_MEM_V( (int*) block <<" "<<groesse )

      fortschritt(0.2,0.2);
      if(groesse)
        hMoni = cmsOpenProfileFromMem(block, groesse);
      else
        hMoni = cmsCreate_sRGBProfile();
      if(!hMoni) WARN_S( "hMoni profile nicht opened" )
      hLab  = cmsCreateLabProfile(cmsD50_xyY());
      if(!hLab)  WARN_S( "hLab profile not opened" )

      if(flags & cmsFLAGS_GAMUTCHECK)
      {
        block = const_cast<char*>( proof(groesse) );
        hProof = cmsOpenProfileFromMem(block, groesse);
      }

      fortschritt(0.5,0.2);
      *form = cmsCreateProofingTransform  (hLab, TYPE_Lab_DBL,
                                               hMoni, TYPE_RGB_DBL,
                                               hProof, // simulation profile
                                               intent,
                                               INTENT_RELATIVE_COLORIMETRIC,
                                               PRECALC|BW_COMP|flags);
      if (!*form) WARN_S( "no hXYZtoRGB transformation found" )

      if(flags & cmsFLAGS_GAMUTCHECK)
        h_lab_to_RGB_teuer = *form;
      else
        hLabtoRGB = *form;

    }


    RGB_Speicher = new double[size*3];
    if(!RGB_Speicher)  WARN_S( "RGB_speicher Speicher not available" )

    double *cielab = (double*) malloc (sizeof(double)*3*size);
    LabToCIELab (Lab_Speicher, cielab, size);

    cmsDoTransform (*form, cielab, RGB_Speicher, size);

    if(start <= 0.0)
      fortschritt(1.1);

    if(hMoni)     cmsCloseProfile(hMoni);
    if(hLab)      cmsCloseProfile(hLab);

    if(cielab)    free (cielab);

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


