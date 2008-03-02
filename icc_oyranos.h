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
 * the CMS sorter
 * 
 */

// Date:      25. 11. 2004


#ifndef ICC_OYRANOS_H
#define ICC_OYRANOS_H

#include "icc_speicher.h"
#include "icc_vrml_parser.h"


#include "icc_oyranos_extern.h"
using namespace oyranos;

#include <string>
#include <list>
#include <map>
class Oyranos;
class Speicher;
class ICCprofile;


struct ColourTransformKey
{
    const char* show() const   { return text; }
    int    eq(const char* cmp) { return !strcmp(cmp, text); }
    char  *text; // key
};

struct ColourTransform
{
    int    eq(ColourTransformKey *cmp) { return !strcmp(cmp->text, text); }
    const char* show() const   { return text; }
    char  *text; // key
    char  *cmm;
    char **props; // properties
    char  *block;
    size_t size;
};

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
    int         setzeMonitorProfil     (const char *name, int x, int y );
    char**      moniInfo (int x, int y, int *num);
    // common profiles
    std::string profil (const char* n) { if(profil_test_(n)) return profil_(n);}
    const char* profil (const char* n, size_t &g) { return profil_(n,g); }

//    char*       holeMonitorProfil      (const char *display_name, size_t *size );
    std::vector<ICCnetz> netzAusVRML   (std::string & vrml)
                                { return extrahiereNetzAusVRML (vrml); }
    std::vector<ICCnetz> netzVonProfil (ICCprofile & p, int intent, int bpc);
  private:
    std::string netzVonProfil_      (std::vector<ICCnetz> & netz,
                                     Speicher & profil,
                                     int intent, int bpc);
  public:
    std::vector<double>  bandVonProfil (const Speicher & p, int intent);

  private:
    void lab_test_();
    void moni_test_( int x, int y );
    void rgb_test_();
    void cmyk_test_();
    void proof_test_();
    Speicher lab_;
    Speicher moni_;
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
    int      wandelProfilNachLabUndZurueck(double *lab, // 0.0 - 1.0
                                           size_t  size, int intent, int flags,
                                           Speicher & profil);
    double*  wandelLabNachBildschirmFarben(double *Lab_Speicher, // 0.0 - 1.0
                                           size_t  size, int intent, int flags);
    // Create an abstract profile containing GamutCheck + Proof
    // from a device profile; write in the gamut profile in abstract
    void     gamutCheckAbstract(  Speicher &  profil,
                                  Speicher &  abstract,
                                  int         intent,
                                  int         flags);
    // Create VMRL from Profile
    std::string vrmlVonProfil (   ICCprofile &profil,
                                  int         intent);

    // colour transformations
    ColourTransformKey erzeugeTrafo (
                                  const char* eingangs_profil__geraet,
                                  int         byte,
                                  int         kanaele,
                                  const char* ausgangs_profil__geraet,
                                  int         byte,
                                  int         kanaele,
                                  int         farb_intent,
                                  const char* cmm, // 4 bytes 'lcms' 'APPL'
                                  int         cmm_optionen); // BPC, precission
    ColourTransformKey erzeugeTrafo (
                                  const char* eingangs_profil__geraet,
                                  int         byte,
                                  int         kanaele,
                                  const char* ausgangs_profil__geraet,
                                  int         byte,
                                  int         kanaele,
                                  int         farb_intent,
                                  const char* cmm, // 4 bytes 'lcms' 'APPL'
                                  int         cmm_optionen,
                                  std::list<const char*> &profile );
    ColourTransformKey erzeugeTrafo (
                                  const char* eingangs_profil__geraet,
                                  int         byte,
                                  int         kanaele,
                                  const char* ausgangs_profil__geraet,
                                  int         byte,
                                  int         kanaele,
                                  int         farb_intent,
                                  const char* cmm, // 4 bytes 'lcms' 'APPL'
                                  int         cmm_optionen,
                                  const char* simulations_profil,
                                  int         simulations_intent );
    // TODO: callback, cmm Auswahl, 
    void               wandelFarben (
                                  void* block,
                                  ColourTransformKey& transformation );
    std::string cmm;
  private:
    ColourTransformKey erzeugeSchluessel_ (
                                  const char* eingangs_profil__geraet,
                                  int         byte,
                                  int         kanaele,
                                  const char* ausgangs_profil__geraet,
                                  int         byte,
                                  int         kanaele,
                                  int         farb_intent,
                                  int         cmm_optionen,
                                  const char* simulations_profil,
                                  int         simulations_intent,
                                  std::list<const char*> &profile );
    std::list<std::pair<ColourTransformKey, Speicher> > trafos_;
};

extern Oyranos icc_oyranos;

// user GUI helpers
void	oyranos_pfade_einlesen();
void	oyranos_pfade_auffrischen();
void	oyranos_pfade_loeschen();
void	oyranos_pfad_dazu();



#endif //ICC_OYRANOS_H
