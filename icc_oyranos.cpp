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


#include <lcms.h>

#ifdef HAVE_OY
#include "oyranos/oyranos.h"
#include "oyranos/oyranos_monitor.h"
using namespace oyranos;
#endif

#include "icc_formeln.h"
#include "icc_oyranos.h"
#include "icc_profile.h"
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
  bool fehler = false;
  #if HAVE_OY

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
          char* block = (char*)oyGetProfileBlock( profil_name, &size);
          DBG_PROG_V( (int)block <<"|"<< size )
          v_block->lade(block, size);
        }
      }
    } else // Profil in Liste
    {
      //Speicher *v_block = &pspeicher_[profil_name];
      // ...
    }
  }
  
  DBG_NUM_S( "Standard " OY_DEFAULT_LAB_INPUT_PROFILE " Profil = "<< *lab_ <<" "<< lab_.size() <<"\n" )

  #endif
  return fehler;
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
    char* profil_name = oyGetDefaultLabInputProfileName();
    DBG_PROG_V( (int)profil_name << oyGetDefaultLabInputProfileName() )
    if( profil_name &&
        *v_block != profil_name )
    { 
        *v_block = profil_name;

        size_t size = oyGetProfileSize ( profil_name );
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
  
  DBG_NUM_S( "Standard " OY_DEFAULT_LAB_INPUT_PROFILE " Profil = "<< *lab_ <<" "<< lab_.size() <<"\n" )

  
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
    DBG_V( display_name )
    Speicher v_block = moni_;
      DBG_MEM_V( v_block.size() )
      DBG_PROG_V( (int*)profil_name << profil_name )
    if( profil_name &&
        v_block != profil_name )
    { 
        size_t size = oyGetProfileSize ( profil_name );
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
    char* profil_name = oyGetDefaultRGBInputProfileName();
    DBG_PROG_V( (int)profil_name << oyGetDefaultRGBInputProfileName() )
    if( profil_name &&
        *v_block != profil_name )
    { 
        *v_block = profil_name;

        size_t size = oyGetProfileSize ( profil_name );
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

  DBG_NUM_S( "Standard " OY_DEFAULT_RGB_INPUT_PROFILE " Profil = "<< *rgb_ <<" "<< rgb_.size() <<"\n" )
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
    char* profil_name = oyGetDefaultCmykInputProfileName();
    if(profil_name) {DBG_PROG_V( profil_name );
    } else {         DBG_PROG_V( (int)profil_name );}

    if( profil_name &&
        *v_block != profil_name )
    { 
        *v_block = profil_name;

        size_t size = oyGetProfileSize ( profil_name );
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
    DBG_NUM_S( "Standard " OY_DEFAULT_CMYK_INPUT_PROFILE " Profil = "<< *cmyk_ <<" "<< cmyk_.size() <<"\n" );
  #endif
  //oy_debug = 0;
  DBG_PROG_ENDE
}

#ifdef HAVE_X
#include <X11/Xlib.h>
#endif
#ifdef HAVE_FLTK
#include <FL/x.H>
#endif

char*
Oyranos::holeMonitorProfil (const char* display_name, size_t* size )
{
  DBG_PROG_START
  char* moni_profil = 0;
  *size = 0;

  #if HAVE_OY

  #ifdef HAVE_X
  static Display *display=0;

  #ifdef HAVE_FLTK
  if( !display )
    display = fl_display;
  #endif
  if( !display )
    display = XOpenDisplay(0);

  display_name = XDisplayString( display );  // gehört X
  DBG_PROG_V( display_name <<" "<< strlen(display_name) )

  #ifndef HAVE_FLTK
    XCloseDisplay( display ); DBG_PROG
  #endif

  #endif

  moni_profil = oyGetMonitorProfile( display_name, size );

  #endif
  DBG_PROG_V( *size <<" "<< (int*)moni_profil )

  DBG_PROG_ENDE
  return moni_profil;
}

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
      s.lade(daten, groesse);
      DBG_NUM_V( groesse );
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
      s.lade(daten, groesse);
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
  const char* b = p;
  DBG_MEM_V( (int*)b )
  std::vector<double> band;
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

      hLab  = cmsCreateLabProfile(cmsD50_xyY());
      if(!hLab)  WARN_S( _("hLab Profil nicht geoeffnet") )

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
      LPLUT gmt_lut = cmsAllocLUT(),
            lut = cmsReadICCLut( tmp, icSigAToB0Tag);
      cmsAlloc3DGrid( gmt_lut, lut->cLutPoints, 3, 3);
      DBG_V( lut->cLutPoints )
      cmsSample3DGrid( gmt_lut, icc_examin_ns::gamutCheckSampler, tr1, 0 );

      cmsHPROFILE gmt = _cmsCreateProfilePlaceholder();
      cmsSetDeviceClass( gmt, icSigAbstractClass );
      cmsSetColorSpace( gmt, icSigLabData );
      cmsSetPCS( gmt, icSigLabData );
      cmsAddTag( gmt, icSigProfileDescriptionTag,  (char*)"GamutCheck");
      cmsAddTag( gmt, icSigAToB0Tag, gmt_lut );
      _cmsSaveProfileToMem ( gmt, 0, &groesse );
      char* mem = (char*) calloc( sizeof(char), groesse);
      _cmsSaveProfileToMem ( gmt, mem, &groesse );
      abstract.lade (mem, groesse); DBG
      if(gmt) cmsCloseProfile( gmt );
      if(hLab) cmsCloseProfile( hLab );
      if(tmp) cmsCloseProfile( tmp );
      if(tr1) cmsDeleteTransform( tr1 );
      if(gmt_lut) cmsFreeLUT( gmt_lut );
      if(lut) cmsFreeLUT( lut );
  
  DBG_PROG_ENDE
}


double*
Oyranos::wandelLabNachBildschirmFarben(double *Lab_Speicher, // 0.0 - 1.0
                                       size_t  size, int intent, int flags)
{
  DBG_PROG_START

  DBG_PROG_V( size <<" "<< intent <<" "<< flags )

    // lcms Typen
    cmsHPROFILE hsRGB = 0,
                hLab = 0;
    static cmsHTRANSFORM hLabtoRGB = 0;
    double *RGB_Speicher = 0;
    
    static int flags_ = 0;

    if(flags_ != flags && hLabtoRGB) {
      cmsDeleteTransform(hLabtoRGB);
      hLabtoRGB = 0;
      flags_ = flags;
    }

    // Initialisierung für lcms
    if(!hLabtoRGB)
    {
      size_t groesse = 0;
      char* block = 0;
      block = (char*) moni(groesse);
      DBG_MEM_V( (int*) block <<" "<<groesse )

      if(groesse)
        hsRGB = cmsOpenProfileFromMem(const_cast<char*>(block), groesse);
      else
        hsRGB = cmsCreate_sRGBProfile();
      if(!hsRGB) WARN_S( _("hsRGB Profil nicht geoeffnet") )
      hLab  = cmsCreateLabProfile(cmsD50_xyY());
      if(!hLab)  WARN_S( _("hLab Profil nicht geoeffnet") )

      hLabtoRGB = cmsCreateProofingTransform  (hLab, TYPE_Lab_DBL,
                                               hsRGB, TYPE_RGB_DBL,
                                               hsRGB,
                                               intent,
                                               INTENT_RELATIVE_COLORIMETRIC,
                                               PRECALC|BW_COMP|flags);

      if (!hLabtoRGB) WARN_S( _("keine hXYZtoRGB Transformation gefunden") )
    }

    RGB_Speicher = new double[size*3];
    if(!RGB_Speicher)  WARN_S( _("RGB_speicher Speicher nicht verfuegbar") )

    double *cielab = (double*) alloca (sizeof(double)*3*size);
    LabToCIELab (Lab_Speicher, cielab, size);

    cmsDoTransform (hLabtoRGB, cielab, RGB_Speicher, size);

    //if(hLabtoRGB) cmsDeleteTransform(hLabtoRGB);
    if(hsRGB)     cmsCloseProfile(hsRGB);
    if(hLab)      cmsCloseProfile(hLab);

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


