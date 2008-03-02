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
 * profile interpretation
 * 
 */

// Date:      Mai 2004

#ifndef ICC_PROFILE_TAGS_H
#define ICC_PROFILE_TAGS_H


#include "icc_utils.h"
#include "icc_helfer.h"


// lcms.h for BSD compiling
#include <lcms.h>
#include <icc34.h>
#include <string>
#include <vector>


/**
  *   @brief internal ICC profile structure
  **/

class ICCprofile;


class ICCtag {
    friend class ICCmeasurement;

    void                copy               (const ICCtag& tag);
    void                defaults           ();
  public:
                        ICCtag             ()
                          { DBG_PROG defaults(); }
                        ICCtag             (ICCprofile* profil,
                                            icTag* tag, char* data);
                        ICCtag             (const ICCtag& tag)
                          { DBG_PROG copy (tag); }
    ICCtag&             operator =         (const ICCtag& tag)
                          { DBG_PROG copy (tag); return *this; }
    void                clear              ();
                        ~ICCtag            ()
                          { DBG_PROG_S("::ICCtag~") clear(); }
  private:
    icTagSignature      _sig;
    int                 size_;
    char*               data_;

    int                 _intent; // for mft1/2
    icColorSpaceSignature _color_in;
    icColorSpaceSignature _color_out;
    bool                _to_pcs;

    ICCprofile*         _profil;

  public:
    icTagSignature      getSignature ()    {DBG_MEM return _sig; }
    std::string         getTagName ()
                          { DBG_MEM return getSigTagName (_sig); }
    std::string         getInfo ()
                          { DBG_MEM  return getSigTagDescription(_sig); }
    std::string         getTypName()
                          { icTagTypeSignature sig;
                            if( ((icTagBase*)data_) )
                              sig = ((icTagBase*)data_) -> sig;
                            else
                              return "";
                            DBG_MEM
                            return getSigTypeName(
                                            (icTagTypeSignature)icValue(sig)); }
    int                 getSize()
                          { DBG_MEM  return size_; }

    std::vector<double> getCIEXYZ();
    std::vector<double> getCurve();
    typedef enum {
      MATRIX,
      CURVE_IN,
      TABLE,
      TABLE_IN,
      TABLE_OUT,
      CURVE_OUT
    } MftChain;
    std::vector<std::vector<double> >
                        getCurves    (MftChain typ);
    std::vector<std::vector<std::vector<std::vector<double> > > >
                        getTable     (MftChain typ);
    std::vector<double> getNumbers   (MftChain typ);

    std::vector<std::string> getText ();
    std::vector<std::string> getText (MftChain typ);
    //std::vector<std::string> getDescription(); deprecated
    std::string         getVrml();
  public:  // I/O
    void                load (ICCprofile* profil ,icTag* tag, char* data);
    const char*         write(int* size)
                          { DBG_PROG  *size = size_;
                            return (const char*)data_; }
};


#endif //ICC_PROFILE_TAGS_H
