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

#ifndef ICC_PROFILE_H
#define ICC_PROFILE_H

#include "icc_utils.h"
#include "icc_formeln.h"
//#include "icc_helfer.h"
#include "icc_speicher.h"

#include "icc_measurement.h"
#include "icc_profile_header.h"
#include "icc_profile_tags.h"

#include <icc34.h>
#include <string>
#include <iostream>
#include <sstream>
#include <map>
#include <fstream>

/**
  *   @brief internal ICC profile struktur
  **/

class ICCprofile;

// defined in icc_profile.cpp
class ICCprofile {
  friend class ICCtag;
  friend class ICCmeasurement;

    ICCprofile &        copy_      ( const ICCprofile & p );
  public:
  typedef enum {
    ICCnullDATA,
    ICCprofileDATA,
    ICCcorruptedprofileDATA,
    ICCmeasurementDATA,
    ICCvrmlDATA
  } ICCDataType;

                        ICCprofile ();
                        ICCprofile ( const Speicher & s);
                        ICCprofile ( const ICCprofile & p );
    ICCprofile &        operator=  (const ICCprofile & );
    virtual             ~ICCprofile (void);
    void                clear (void);

    ICCDataType         load (const Speicher & profil);
    ICCDataType         data_type;  //!< is set at least at load time
    bool                changing()             { return changing_; }

  private:
    bool                changing_;
    std::string         filename_;

    // icc34.h definitions
    char*               data_;
    size_t              size_;
    oyProfile_s       * profile_;
  public:
    oyProfile_s       * oyProfile() { return oyProfile_Copy( profile_, 0 ); }

  private:
    ICCheader           header;
    ICClist<ICCtag> tags;
  private: // cgats via lcms
    ICCmeasurement      measurement;
  public:
    void                measurementReparent() { measurement.profile_ = this; }

  public: // informations
    const char*         filename ();
    void                filename (const char* s);
    size_t              size     ();
    //const char*         cmm      ()        {DBG_PROG return header.cmmName(); }
    //void                cmm      (const char* s) {DBG_PROG header.cmmName (s); }
    //int                 version  ()        {DBG_PROG return (int) header.version(); }
    //const char*         creator  ()        {DBG_PROG return header.creatorName(); }
    //! rendering intent; see as well @see: ICCexamin.intent()
    int                 intent   ()     {return icValue(((const icHeader*)header
                                           .headerRaw())-> renderingIntent ); }
    icColorSpaceSignature colorSpace()  {return header.colorSpace(); }

    std::string         printHeader     ();
    std::string         printLongHeader ();
    ICClist<std::string> getPCSNames();

    // tag infos
    int                      tagCount();
    ICClist<std::string> printTags  (); //!< list of tags (5)
    ICClist<std::string> printTagInfo      (int item); //!< name,typ
    ICClist<std::string> getTagText        (int item); //!< content
    ICClist<std::string> getTagDescription (int item);

    ICClist<double>      getTagCIEXYZ      (int item);
    ICClist<double>      getTagCurve       (int item);
    ICClist<ICClist<double> >
                             getTagCurves      (int item, ICCtag::MftChain typ);
    ICClist<ICClist<ICClist<ICClist<double> > > >
                         getTagTable ( int                 item,
                                       ICCtag::MftChain    typ,
                                       ICClist<int>        channels );
    ICClist<double>      getTagNumbers     (int item, ICCtag::MftChain typ);
    oyStructList_s *     getTagNumbers     (int item);
    ICClist<std::string> getTagChannelNames(int item, ICCtag::MftChain typ);
    bool                hasTagName   (std::string name); //!< name
    int                 getTagIDByName (std::string name); //!< name
    int                 getTagCount     (); 

    //! profile infos
    //char*               getProfileInfo  ();
    char*               getProfileDescription  ();
    ICClist<double> getWhitePkt   (void);
    int                 getColourChannelsCount ();
    int                 hasCLUT ();
    bool                valid ();
  public: // file I/O
    int                 checkProfileDevice (char* type,
                                           icProfileClassSignature deviceClass);
 
  public: // measurement infos
    bool                hasMeasurement ();
    bool                tagBelongsToMeasurement (int tag);
    std::string         report       ( bool                export_ausserhalb, 
                                       oyOptions_s       * opts );
    ICCmeasurement&     getMeasurement ();
    std::string         cgats          ();
    std::string         cgats_max      ();

  public: // profile generation
    void                setHeader          (const void* h);
    ICCheader           getHeader          ()      { return header; };
    void                addTag             (ICCtag & tag);
    ICCtag&             getTag             (int item);
    ICCtag&             getTag             (std::string name);
    int                 removeTag          (int item);
    int                 removeTag          (std::string name);
    size_t              getProfileSize     ();
    char*               saveProfileToMem   (size_t* size);
  private:
    void                writeTags     (void);
    void                writeHeader   (void);
    void                writeTagTable (void);
};


ICCprofile::ICCDataType guessFileType( const char        * filename );


#endif //ICC_PROFILE_H
