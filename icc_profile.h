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

// definiert in icc_profile.cpp
class ICCprofile {
  friend class ICCtag;
  friend class ICCmeasurement;

    ICCprofile &        copy_      ( const ICCprofile & p );
  public:
  typedef enum {
    ICCnullDATA,
    ICCprofileDATA,
    ICCmeasurementDATA
  } ICCDataType;

                        ICCprofile ();
                        ICCprofile ( const Speicher & s);
                        ICCprofile ( const ICCprofile & p );
    ICCprofile &        operator=  (const ICCprofile & );
    virtual             ~ICCprofile (void);
    void                clear (void);

    ICCDataType         load (const Speicher & profil);
    bool                changing()             { return changing_; }

  private:
    bool                changing_;
    std::string         filename_;

    // icc34.h Definitionen
    char*               data_;
    size_t              size_;

    ICCheader           header;
    std::vector<ICCtag> tags;
  private: // cgats via lcms
    ICCmeasurement      measurement;
  public:
    void                measurementReparent() { measurement.profile_ = this; }

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
    size_t              getProfileSize     (void);
    char*               saveProfileToMem   (size_t* size);
  private:
    void                writeTags     (void);
    void                writeHeader   (void);
    void                writeTagTable (void);
};


#endif //ICC_PROFILE_H
