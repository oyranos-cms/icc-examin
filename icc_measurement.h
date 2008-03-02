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
 * profile interpretation
 * 
 */

// Date:      Mai 2004

#ifndef ICC_MEASUREMENT_H
#define ICC_MEASUREMENT_H


#include <string>
#include <vector>
#include "icc_utils.h"

#include "icc_profile_tags.h"
#include "icc_oyranos.h"

/**
  *   @brief reads CGATS from a profile 
  *
  *   typicaly a ICCmeasurement is associated to a ICCprofile.
  *   CIE* Lab, CIE* XYZ, RGB and CMYK are supported. Spectal can be added.
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
    // load and analyse
    void                leseTag ();
    // calculate of all measurement- and profile values
    void                init_umrechnen (void);
    // ??
    void                pruefen (void);
    // lcms cgats reader
    void                lcms_parse ();

  private:
    icTagSignature      sig_;
    size_t              size_;
    char*               data_;

    ICCprofile*         profile_;
    int                 channels_;
    int                 isICCDisplay_;

    int                 nFelder_;

    //! is only set when in CGATS available
    bool                XYZ_measurement_;
    bool                LAB_measurement_;
    bool                RGB_measurement_;
    bool                CMYK_measurement_;
    // measurement values
    std::vector<XYZ_s>    XYZ_Satz_;
    std::vector<Lab_s>    Lab_Satz_;
    std::vector<RGB_s>    RGB_Satz_;
    std::vector<CMYK_s>   CMYK_Satz_;
    // profile values
    std::vector<std::string> Feldnamen_;
    std::vector<XYZ_s>    XYZ_Ergebnis_;
    std::vector<Lab_s>    Lab_Ergebnis_;
    std::vector<RGB_s>    RGB_MessFarben_;
    std::vector<RGB_s>    RGB_ProfilFarben_;
    std::vector<std::pair<std::string,std::vector<int> > > patch_src_lines_;
    // results
    std::vector<double> Lab_Differenz_;
    double              Lab_Differenz_max_;
    double              Lab_Differenz_min_;
    double              Lab_Differenz_Durchschnitt_;
    std::vector<double> DE00_Differenz_;
    double              DE00_Differenz_max_;
    double              DE00_Differenz_min_;
    double              DE00_Differenz_Durchschnitt_;
    // input range normalisation
    double              range_XYZ[6];
    double              range_Lab[6];
    double              range_RGB[6];
    double              range_CMYK[8];
    // maintain CIE absolute intent
    int                 minFeld;
    int                 maxFeld;
    double              XYZmax[3];      /**< CIE absolute black point */
    double              XYZmin[3];      /**< CIE absolute media white */
    double              XYZWP[3];       /**< media white */
    double              XYZBP[3];       /**< media black */

    std::vector<std::vector<std::string> > reportTabelle_;
    std::vector<int>    layout;
  // I/O
  public:
    void                load (ICCprofile* profil , ICCtag& tag);
    void                load (ICCprofile* profil ,
                              const char *data, size_t size);
  public:
    // basic infos
    bool                has_data (void)    {DBG_PROG return (XYZ_Satz_.size() ||
                                                    ( data_ && size_ ) ); }
    bool                valid (void)       {DBG_PROG return ((XYZ_measurement_
                                                  || LAB_measurement_) 
                                                 && (RGB_measurement_
                                                  || CMYK_measurement_)); }
    bool                validHalf (void)   {DBG_PROG return (XYZ_measurement_
                                                 || LAB_measurement_
                                                 || RGB_measurement_
                                                 || CMYK_measurement_); }
    bool                hasRGB ()          {DBG_PROG return RGB_measurement_; }
    bool                hasCMYK ()         {DBG_PROG return CMYK_measurement_; }
    bool                hasXYZ ()          {DBG_PROG return XYZ_measurement_; }
    bool                hasLab ()          {DBG_PROG return LAB_measurement_; }
    size_t              getSize()          {DBG_PROG return size_; }
    int                 getPatchCount()    {DBG_PROG return nFelder_; }
    // values
    std::vector<double> getMessRGB (int patch); //!< displaying colours
    std::vector<double> getCmmRGB (int patch);  //!< displaying colours
    std::vector<double> getMessLab (int patch);
    std::vector<double> getCmmLab (int patch);
    oyNamedColour_s *   getMessColour (int patch);
    oyNamedColour_s *   getCmmColour (int patch);
    std::vector<int>    getPatchLines ( const char       * tag_name );
    oyNamedColour_s *   getPatchLine  ( int line, const char * tag_name );
    std::vector<XYZ_s>    getMessXYZ ()      {DBG_PROG return XYZ_Satz_; }
    std::vector<Lab_s>    getMessLab ()      {DBG_PROG return Lab_Satz_; }
    std::vector<RGB_s>    getMessRGB ()      {DBG_PROG return RGB_Satz_; }
    std::vector<CMYK_s>   getMessCMYK ()     {DBG_PROG return CMYK_Satz_; }
    void                  getTargRGB  (int patch, RGB_s * rgb) 
                                             { *rgb = RGB_Satz_[patch]; }
    void                  getTargCMYK (int patch, CMYK_s * cmyk)
                                             { *cmyk = CMYK_Satz_[patch]; }
    std::vector<std::string> getFeldNamen () {DBG_PROG return Feldnamen_; }
    std::string           getFieldName (int p) {DBG_PROG return Feldnamen_[p]; }
    std::vector<Lab_s>    getProfileLab ()   {DBG_PROG return Lab_Ergebnis_; }

    // report
    std::vector<std::vector<std::string> > getText ();
    std::vector<std::string> getDescription();
    std::string         getHtmlReport (bool export_ausserhalb);
    std::vector<int>    getLayout ()       {DBG_PROG return layout; }
    std::string         getCGATS    ();
    std::string         getMaxCGATS ();

    // source
    std::string         getTagName()       {return getSigTagName (sig_); }
    std::string         getInfo()          {DBG_PROG return getSigTagDescription(sig_); }

    // switch
    bool                export_farben;

};



#endif //ICC_MEASUREMENT_H
