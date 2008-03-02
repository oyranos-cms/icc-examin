/*
 * ICC Examin ist eine ICC Profil Betrachter
 * 
 * Copyright (C) 2004  Kai-Uwe Behrmann 
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
 * Der CMS Sortierer
 * 
 */

// Date:      25. 11. 2004


#ifndef ICC_OYRANOS_H
#define ICC_OYRANOS_H
#define _(text) text

#include <string>
#include <list>

class Oyranos;

struct ColourTransformKey
{
  friend class Oyranos;
    const char* show() { return text.data(); }
  private:
    std::string text; // Schluessel
};

class Speicher
{
    char*  zeiger_;
    size_t groesse_;
  public:
    Speicher       () { zeiger_ = 0; groesse_ = 0; }
    ~Speicher      () { if( zeiger_ ) free (zeiger_); }
    Speicher       (const Speicher& s) { zeiger_ = 0; groesse_ = 0; }
    Speicher& operator = (const Speicher& s) { zeiger_ = 0; groesse_ = 0;
                        return *this; }

    void        lade     (char* zeiger, int groesse)
                            { zeiger_ = zeiger;
                              groesse_ = groesse; }
                Speicher (char* zeiger, int groesse) { lade (zeiger, groesse); }

    size_t      size     () { return groesse_; }
    std::string name;
    int         anzahl;

    operator const char* () { return zeiger_; }
    operator std::string () { return name; }
    operator size_t      () { return groesse_; }
};

class Oyranos
{
  public:
    Oyranos() ;
    ~Oyranos();
    void    init() {; }

    // Standard Profile
    std::string lab  ()                { lab_test_(); return lab_.name; }
    const char* lab  (size_t &g)       { lab_test_(); g = lab_.size();
                                         return lab_; }
    std::string rgb  ()                { rgb_test_(); return rgb_.name; }
    const char* rgb  (size_t &g)       { rgb_test_(); g = rgb_.size();
                                         return rgb_; }
    std::string cmyk ()                { cmyk_test_(); return cmyk_.name; }
    const char* cmyk (size_t &g)       { cmyk_test_(); g = cmyk_.size();
                                         return cmyk_; }
    // Geräte Profile
    std::string moni ()                { moni_test_(); return moni_.name; }
    const char* moni (size_t &g)       { moni_test_(); g = moni_.size();
                                         return moni_; }
    int         setzeMonitorProfil (const char* name );
  private:
    void lab_test_();
    void moni_test_();
    void rgb_test_();
    void cmyk_test_();
    Speicher lab_;
    Speicher moni_;
    Speicher rgb_;
    Speicher cmyk_;

  public:
    // Farbtransformationen
    ColourTransformKey erzeugeTrafo (
                                  const char* eingangs_profil__geraet,
                                  int         byte,
                                  int         kanaele,
                                  const char* ausgangs_profil__geraet,
                                  int         byte,
                                  int         kanaele,
                                  int         farb_intent,
                                  int         cmm_optionen); // BPC, Präzission
    ColourTransformKey erzeugeTrafo (
                                  const char* eingangs_profil__geraet,
                                  int         byte,
                                  int         kanaele,
                                  const char* ausgangs_profil__geraet,
                                  int         byte,
                                  int         kanaele,
                                  int         farb_intent,
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

// Benutzeroberflächenfunktionen
void	oyranos_pfade_einlesen();
void	oyranos_pfade_auffrischen();
void	oyranos_pfade_loeschen();
void	oyranos_pfad_dazu();



#endif //ICC_OYRANOS_H
