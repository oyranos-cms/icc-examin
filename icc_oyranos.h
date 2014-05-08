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
 * the CMS sorter
 * 
 */

// Date:      25. 11. 2004


#ifndef ICC_OYRANOS_H
#define ICC_OYRANOS_H

#include "icc_speicher.h"
#include "icc_vrml_parser.h"


#include "icc_oyranos_extern.h"

#include <oyConversion_s.h>
#include <oyObject_s.h>
#include <oyProfile_s.h>
#include <oyRectangle_s.h>
#include <oyNamedColor_s.h>
#include <oyNamedColors_s.h>
#include <oyranos_color.h>
#include <oyranos_image.h>

#include <string>
#include <list>
#include <map>
#include <cstring>
class Oyranos;
class Speicher;
class ICCprofile;



class Oyranos
{
  public:
    Oyranos() ;
    ~Oyranos();
    void    init() {; }

    // standard profiles
    std::string lab  ()                { lab_test_();
                                         return lab_.name(); }
    const char* lab  (size_t &g)       { lab_test_(); g = lab_.size();
                                         return lab_; }
    std::string rgb  ()                { rgb_test_(); return rgb_.name(); }
    const char* rgb  (size_t &g)       { rgb_test_(); g = rgb_.size();
                                         return rgb_; }
    std::string cmyk_name ()           { cmyk_test_(); return cmyk_.name(); }
    const char* cmyk (size_t &g)       { cmyk_test_(); g = cmyk_.size();
                                         return cmyk_; }
    const Speicher & cmyk ()           { cmyk_test_(); return cmyk_; }

    const char* proof (size_t &g)      { proof_test_(); g = proof_.size();
                                         return proof_; }

    // device profiles
    std::string moni_name (int x,int y){ moni_test_(x,y); return moni_.name(); }
    const char* moni (int x, int y, size_t &g)
                                       { moni_test_(x,y); g = moni_.size();
                                         return moni_; }
    const Speicher & moni (int x,int y){ moni_test_(x,y); return moni_; }
    const Speicher & moniNative (int x,int y){ moni_native_test_(x,y); return moni_native_; }
    oyProfile_s * oyMoni (int x, int y, int native = 0);
    int         setzeMonitorProfil     (const char *name, int x, int y );
    char**      moniInfo (int x, int y, int *num);
    // common profiles
    std::string profil (const char* n) { if(profil_test_(n)) return profil_(n);}
    const char* profil (const char* n, size_t &g) { return profil_(n,g); }

//    char*       holeMonitorProfil      (const char *display_name, size_t *size );
    void        netzAusVRML   (std::string & vrml, icc_examin_ns::ICCThreadList<ICCnetz> & netz)
                                { netz = extrahiereNetzAusVRML (vrml); }
    void        netzVonProfil (ICCprofile & p, oyOptions_s * options,
                               int native, ICCnetz & netz);
  private:
    std::string netzVonProfil_      (ICCnetz  & netz,
                                     Speicher & profil,
                                     oyOptions_s * options );
  public:
    ICClist<double>  bandVonProfil (const Speicher & p, int intent);

  private:
    void lab_test_();
    void moni_test_( int x, int y );
    void moni_native_test_(int x, int y);
    void rgb_test_();
    void cmyk_test_();
    void proof_test_();
    Speicher lab_;
    Speicher moni_;
    Speicher moni_native_;
    Speicher rgb_;
    Speicher cmyk_;
    Speicher proof_;
    std::map<std::string,Speicher> pspeicher_;
    bool profil_test_   (const char* profil_name);
    std::string profil_ (const char* profil_name);  
    const char* profil_ (const char* profil_name, size_t &g);

  public:
    void     clear();
    // intermediate
    int      wandelLabNachProfilUndZurueck(double *lab, // 0.0 - 1.0
                                           size_t  size, oyOptions_s * options,
                                           Speicher & profil);
    double*  wandelLabNachBildschirmFarben(int x, int y, oyProfile_s * profile, 
                                           double *Lab_Speicher, // 0.0 - 1.0
                                           size_t  size, oyOptions_s * options);
    double*  convertLabToProfile         ( oyProfile_s * profile,
                                           double *Lab_Speicher, // 0.0 - 1.0
                                           size_t  size, oyOptions_s * options);
    oyProfile_s * getEditingProfile      ( );
    // Create an abstract profile containing GamutCheck + Proof
    // from a device profile; write in the gamut profile in abstract
    void     gamutCheckAbstract(  Speicher &  profil,
                                  Speicher &  abstract,
                                  oyOptions_s*options );
    // Create VMRL from Profile
    std::string vrmlVonProfil (   ICCprofile &profil,
                                  oyOptions_s*options,
                                  int         native );

    int      colourServerActive( );
    void     colourServerRegionSet   ( Fl_Widget         * widget,
                                       oyProfile_s       * profile,
                                       oyRectangle_s     * old_rect,
                                       int                 remove );

    // primary profile selection flags
    int      oy_profile_from_flags;
    // secondary help profile selection flags
    int      icc_profile_flags;
};

extern Oyranos icc_oyranos;

// user GUI helpers
void	oyranos_pfade_einlesen();
void	oyranos_pfade_auffrischen();
void	oyranos_pfade_loeschen();
void	oyranos_pfad_dazu();

extern "C" {
int  oyColorConvert_ ( oyProfile_s       * p_in,
                        oyProfile_s       * p_out,
                        oyPointer           buf_in,
                        oyPointer           buf_out,
                        oyDATATYPE_e        buf_type_in,
                        oyDATATYPE_e        buf_type_out,
                        oyOptions_s       * options,
                        int                 count );
char   *       oyDumpColorToCGATS   ( const double      * channels,
                                       size_t              n,
                                       oyProfile_s       * prof,
                                       oyAlloc_f           allocateFunc,
                                       const char        * DESCRIPTOR );
}


#endif //ICC_OYRANOS_H
