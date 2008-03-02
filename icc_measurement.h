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
 * Profilinterpretation
 * 
 */

// Date:      Mai 2004

#ifndef ICC_MEASUREMENT_H
#define ICC_MEASUREMENT_H


#include <string>
#include <vector>
#include "icc_utils.h"

#include "icc_profile_tags.h"

/**
  *   @brief liest CGATS aus einem Profil 
  *
  *   Typischerweise wird ICCmeasurement einem ICCprofile zugeordnet.
  *   CIE* Lab, CIE* XYZ, RGB und CMYK werden unterstützt.
  **/


class ICCmeasurement {
                        ICCmeasurement     (ICCprofile* profil);
    int                 id_;
  friend class ICCprofile;
    void                copy (const ICCmeasurement& m);
    void                defaults ();

  public:
                        ICCmeasurement     (); 
                        ICCmeasurement     (ICCprofile* profil , ICCtag& tag);
                        ICCmeasurement     (ICCprofile* profil,
                                            icTag& tag, char* data);
    void                clear (void);
                        ~ICCmeasurement();
    void                init (void);
                        ICCmeasurement     (const ICCmeasurement& m);
    ICCmeasurement&     operator=          (const ICCmeasurement& m);

  private:
    // laden und auswerten
    void                leseTag ();
    // Berechnen aller Mess- und Profilwerte
    void                init_umrechnen (void);
    // ??
    void                pruefen (void);
    // lcms cgats Leser
    void                lcms_parse ();

  private:
    icTagSignature      sig_;
    size_t              size_;
    char*               data_;

    ICCprofile*         profile_;
    int                 channels_;
    int                 isMatrix_;

    int                 nFelder_;

    //! wird nur gesetzt wenn in CGATS vorhanden
    bool                XYZ_measurement_;
    bool                RGB_measurement_;
    bool                CMYK_measurement_;
    // Messwerte
    std::vector<XYZ_s>    XYZ_Satz_;
    std::vector<Lab_s>    Lab_Satz_;
    std::vector<RGB_s>    RGB_Satz_;
    std::vector<CMYK_s>   CMYK_Satz_;
    // Profilwerte
    std::vector<std::string> Feldnamen_;
    std::vector<XYZ_s>    XYZ_Ergebnis_;
    std::vector<Lab_s>    Lab_Ergebnis_;
    std::vector<RGB_s>    RGB_MessFarben_;
    std::vector<RGB_s>    RGB_ProfilFarben_;
    // Ergebnisse
    std::vector<double> Lab_Differenz_;
    double              Lab_Differenz_max_;
    double              Lab_Differenz_min_;
    double              Lab_Differenz_Durchschnitt_;
    std::vector<double> DE00_Differenz_;
    double              DE00_Differenz_max_;
    double              DE00_Differenz_min_;
    double              DE00_Differenz_Durchschnitt_;

    std::vector<std::vector<std::string> > reportTabelle_;
    std::vector<int>    layout;
  // I/O
  public:
    void                load (ICCprofile* profil , ICCtag& tag);
    void                load (ICCprofile* profil ,
                              const char *data, size_t size);
  public:
    // grundlegende Infos
    bool                has_data (void)    {DBG_PROG return (XYZ_Satz_.size() ||
                                                    ( data_ && size_ ) ); }
    bool                valid (void)       {DBG_PROG return (XYZ_measurement_
                                                 && (RGB_measurement_
                                                  || CMYK_measurement_)); }
    bool                validHalf (void)   {DBG_PROG return (XYZ_measurement_
                                                 || RGB_measurement_
                                                 || CMYK_measurement_); }
    bool                hasRGB ()          {DBG_PROG return RGB_measurement_; }
    bool                hasCMYK ()         {DBG_PROG return CMYK_measurement_; }
    bool                hasXYZ ()          {DBG_PROG return XYZ_measurement_; }
    size_t              getSize()          {DBG_PROG return size_; }
    int                 getPatchCount()    {DBG_PROG return nFelder_; }
    // Werte
    std::vector<double> getMessRGB (int patch); //!< Darstellungsfarben
    std::vector<double> getCmmRGB (int patch);  //!< Darstellungsfarben
    std::vector<double> getMessLab (int patch);
    std::vector<double> getCmmLab (int patch);
    std::vector<XYZ_s>    getMessXYZ ()      {DBG_PROG return XYZ_Satz_; }
    std::vector<Lab_s>    getMessLab ()      {DBG_PROG return Lab_Satz_; }
    std::vector<RGB_s>    getMessRGB ()      {DBG_PROG return RGB_Satz_; }
    std::vector<CMYK_s>   getMessCMYK ()     {DBG_PROG return CMYK_Satz_; }
    std::vector<std::string> getFeldNamen () {DBG_PROG return Feldnamen_; }

    // Report
    std::vector<std::vector<std::string> > getText ();
    std::vector<std::string> getDescription();
    std::string         getHtmlReport (bool export_ausserhalb);
    std::vector<int>    getLayout ()       {DBG_PROG return layout; }
    std::string         getCGATS    ();
    std::string         getMaxCGATS ();

    // Herkunft
    std::string         getTagName()       {return getSigTagName (sig_); }
    std::string         getInfo()          {DBG_PROG return getSigTagDescription(sig_); }

    // Schalter
    bool                export_farben;

};



#endif //ICC_MEASUREMENT_H
