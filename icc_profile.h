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

#ifndef ICC_PROFILE_H
#define ICC_PROFILE_H


#include <icc34.h>
#include <string>
#include <iostream>
#include <sstream>
#include <map>
#include <vector>
#include <fstream>
#include "icc_utils.h"
#include "icc_formeln.h"
#include "icc_helfer.h"
#include "icc_speicher.h"

#include "icc_measurement.h"
#include "icc_profile_header.h"
#include "icc_profile_tags.h"


/**
  *   @brief interne ICC Profilstruktur
  **/

class ICCprofile;


// definiert in icc_measurement.cpp
class ICCmeasurement {
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
    icTagSignature      _sig;
    size_t              size_;
    char*               data_;

    ICCprofile*         _profil;
    int                 _channels;
    int                 _isMatrix;

    int                 _nFelder;

    bool                _XYZ_measurement;
    bool                _RGB_measurement;
    bool                _CMYK_measurement;
    // Messwerte
    std::vector<XYZ>    _XYZ_Satz;
    std::vector<Lab>    _Lab_Satz;
    std::vector<RGB>    _RGB_Satz;
    std::vector<CMYK>   _CMYK_Satz;
    // Profilwerte
    std::vector<std::string> _Feldnamen;
    std::vector<XYZ>    _XYZ_Ergebnis;
    std::vector<Lab>    _Lab_Ergebnis;
    std::vector<RGB>    _RGB_MessFarben;
    std::vector<RGB>    _RGB_ProfilFarben;
    // Ergebnisse
    std::vector<double> _Lab_Differenz;
    double              _Lab_Differenz_max;
    double              _Lab_Differenz_min;
    double              _Lab_Differenz_Durchschnitt;
    std::vector<double> _DE00_Differenz;
    double              _DE00_Differenz_max;
    double              _DE00_Differenz_min;
    double              _DE00_Differenz_Durchschnitt;

    std::vector<std::vector<std::string> > _reportTabelle;
    std::vector<int>    layout;
  // I/O
  public:
    void                load (ICCprofile* profil , ICCtag& tag);
    void                load (ICCprofile* profil , char *data, size_t size);
  public:
    // grundlegende Infos
    bool                has_data (void)    {DBG_PROG return (_XYZ_Satz.size() ||
                                                    ( data_ && size_ ) ); }
    bool                valid (void)       {DBG_PROG return (_XYZ_measurement
                                                 && (_RGB_measurement
                                                  || _CMYK_measurement)); }
    bool                hasRGB ()          {DBG_PROG return _RGB_measurement; }
    bool                hasCMYK ()         {DBG_PROG return _CMYK_measurement; }
    bool                hasXYZ ()          {DBG_PROG return _XYZ_measurement; }
    size_t              getSize()          {DBG_PROG return size_; }
    int                 getPatchCount()    {DBG_PROG return _nFelder; }
    // Werte
    std::vector<double> getMessRGB (int patch); // Darstellungsfarben
    std::vector<double> getCmmRGB (int patch);  // Darstellungsfarben
    std::vector<double> getMessLab (int patch);
    std::vector<double> getCmmLab (int patch);
    std::vector<XYZ>    getMessXYZ ()      {DBG_PROG return _XYZ_Satz; }
    std::vector<Lab>    getMessLab ()      {DBG_PROG return _Lab_Satz; }
    std::vector<RGB>    getMessRGB ()      {DBG_PROG return _RGB_Satz; }
    std::vector<CMYK>   getMessCMYK ()     {DBG_PROG return _CMYK_Satz; }
    std::vector<std::string> getFeldNamen () {DBG_PROG return _Feldnamen; }

    // Report
    std::vector<std::vector<std::string> > getText ();
    std::vector<std::string> getDescription();
    std::string         getHtmlReport (bool export_ausserhalb);
    std::vector<int>    getLayout ()       {DBG_PROG return layout; }
    std::string         getCGATS    ();
    std::string         getMaxCGATS ();

    // Herkunft
    std::string         getTagName()       {return getSigTagName (_sig); }
    std::string         getInfo()          {DBG_PROG return getSigTagDescription(_sig); }

    // Schalter
    bool                export_farben;

};


// definiert in icc_profile.cpp
class ICCprofile {
  friend class ICCtag;
  friend class ICCmeasurement;

    ICCprofile &        copy_      ( const ICCprofile & p );
  public:
                        ICCprofile ();
                        ICCprofile (const char *filename);
                        ICCprofile ( const ICCprofile & p );
    ICCprofile &        operator=  (const ICCprofile & );
    virtual             ~ICCprofile (void);
    void                clear (void);

    void                load (std::string filename);
    void                load (char* filename);
    void                load (const Speicher & profil);
    bool                changing()             { return changing_; }

  private:
    bool                changing_;
    void                fload ();
    std::string         filename_;

    // icc34.h Definitionen
    char*               data_;
    size_t              size_;

    ICCheader           header;
    std::vector<ICCtag> tags;
  private: // cgats via lcms
    ICCmeasurement      measurement;

  public: // Informationen
    const char*         filename ();
    void                filename (const char* s);
    size_t              size     ();
    //const char*         cmm      ()        {DBG_PROG return header.cmmName(); }
    //void                cmm      (const char* s) {DBG_PROG header.cmmName (s); }
    //int                 version  ()        {DBG_PROG return (int) header.version(); }
    //const char*         creator  ()        {DBG_PROG return header.creatorName(); }
    int                 intent   ()     {return icValue(((const icHeader*)header
                                           .header_raw())-> renderingIntent ); }
    icColorSpaceSignature colorSpace()  {return header.colorSpace(); }

    std::string         printHeader     ();
    std::string         printLongHeader ();
    std::vector<std::string> getPCSNames();

    // Tag Infos
    int                      tagCount();
    std::vector<std::string> printTags  (); // Liste der einzelnen Tags (5)
    std::vector<std::string> printTagInfo      (int item); // Name,Typ
    std::vector<std::string> getTagText        (int item);    // Inhalt
    std::vector<std::string> getTagDescription (int item);

    std::vector<double>      getTagCIEXYZ      (int item);
    std::vector<double>      getTagCurve       (int item);
    std::vector<std::vector<double> >
                             getTagCurves      (int item, ICCtag::MftChain typ);
    std::vector<std::vector<std::vector<std::vector<double> > > >
                             getTagTable       (int item, ICCtag::MftChain typ);
    std::vector<double>      getTagNumbers     (int item, ICCtag::MftChain typ);
    std::vector<std::string> getTagChannelNames(int item, ICCtag::MftChain typ);
    bool                hasTagName   (std::string name); // Name
    int                 getTagByName (std::string name); // Name
    int                 getTagCount     (); 

    // Profil Infos
    char*               getProfileInfo  ();
    std::vector<double> getWhitePkt   (void);
    int                 getColourChannelsCount ();
    int                 hasCLUT ();
    bool                valid ();
  public: // Datei I/O
    int                 checkProfileDevice (char* type,
                                           icProfileClassSignature deviceClass);
 
  public: // Messwertinfos
    bool                hasMeasurement ();
    std::string         report         (bool auss);
    ICCmeasurement&     getMeasurement ();
    std::string         cgats          ();
    std::string         cgats_max      ();

  public: // Profilerstellung
    void                setHeader          (void* h);
    void                addTag             (ICCtag tag);
    ICCtag&             getTag             (int item);
    void                removeTag          (int item);
    void                removeTagByName    (std::string name);
    void                saveProfileToFile  (const char* filename);
    size_t              getProfileSize     (void);
    char*               saveProfileToMem   (size_t* size);
  private:
    void                writeTags     (void);
    void                writeHeader   (void);
    void                writeTagTable (void);
};


#endif //ICC_PROFILE_H
