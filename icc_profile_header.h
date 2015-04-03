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

#ifndef ICC_PROFILE_HEADER_H
#define ICC_PROFILE_HEADER_H

#include "icc_utils.h"
#include "icc_helfer.h"



#include <icc34.h>

#include <string>


/**
  *   @brief internal ICC profile strukture
  **/


// defined in icc_profile.cpp
class ICCheader {
  public:
                ICCheader ();
    void        clear();
  private:
    icHeader    header;               //!<brief icc34.h definition
  public:
    const char* headerRaw () const
                  { DBG_PROG return /*cp_nchar (*/(const char*)&header/*,
                    sizeof (icHeader))*/; }
    void        headerRaw (const void* h)
                  { DBG_PROG memcpy ((void*)&header, h, sizeof (icHeader)); }
    void        load  (void*);

    int         valid;
    int         size    () const
                  { DBG_PROG  return icValue(header.size); }
    void        size    (icUInt32Number size)
                  { DBG_PROG  header.size = (icUInt32Number)icValue(size); }
    void        cmmName (const char*s)
                  { DBG_PROG  memcpy((char*)&(header.cmmId),s,
                                     sizeof (icSignature)); }
    const char* cmmName () const
                  { DBG_PROG  return oyICCCmmDescription( icValue( header.cmmId ) ); }
    double      versionD() const;
    int         version () const
                  { DBG_PROG  return icValue(header.version); }
    void        version (icUInt32Number v)
                  { DBG_PROG  header.version= icValue(v);}
    std::string versionName () const;
    icProfileClassSignature deviceClass () const
                  { DBG_PROG  return icValue(header.deviceClass); }
    void        deviceClass (icProfileClassSignature d)
                  { DBG_PROG  header.deviceClass = icValue(d); }
    icColorSpaceSignature colorSpace () const
                  { DBG_PROG  return icValue(header.colorSpace); }
    void        colorSpace (icColorSpaceSignature color)
                  { DBG_PROG  header.colorSpace = icValue(color); }
    icColorSpaceSignature pcs () const
                  { DBG_PROG  return icValue(header.pcs); }
    void        pcs (icColorSpaceSignature pcs)
                  { DBG_PROG  header.pcs = icValue(pcs); }
    void        setCurrentDate ();
    std::string dateTime() const;
    const char* magicName () const
                  { DBG_PROG  return cp_nchar ((char*)&(header.  magic),
                                                      sizeof (icSignature)); }
    void        set_magic ()
                  { DBG_PROG const char* m= {"acsp"};
                                                header.magic= *(icSignature*)m;}
    std::string platform () const
                  { DBG_PROG  return getPlatformName(icValue(header.platform));}
    void        set_platform ()
                  { DBG_PROG  header.platform = (icValue(icSigSGI));}
    std::string flags () const;
    void        set_embedded_flag()
                  { DBG_PROG  ((char*)&header.flags)[0] =
                                             ((char*)&header.flags)[0] | 0x80; }
    void        unset_embedded_flag()
                  { DBG_PROG  ((char*)&header.flags)[0] =
                                              ((char*)&header.flags)[0] & 0x7f;}
    void        set_dependent_flag()
                  { DBG_PROG  ((char*)&header.flags)[0] =
                                              ((char*)&header.flags)[0] | 0x40;}
    void        unset_dependent_flag()
                  { DBG_PROG  ((char*)&header.flags)[0] =
                                              ((char*)&header.flags)[0] & 0xbf;}
    const char* manufacturerName() const
                  { DBG_PROG  return cp_nchar ((char*)&(header. manufacturer),
                                                      sizeof (icSignature)); }
    void        set_manufacturer () 
                  { DBG_PROG  const char* m= {"none"};
                    header.manufacturer= *(icSignature*)m; }
    const char* modelName () const
                  { DBG_PROG  return cp_nchar ((char*)&(header.  model),
                                                sizeof (icSignature)); }
    void        set_model ()
                  { DBG_PROG  const char* m={"none"}; header.model= *(icSignature*)m;}
    std::string attributes () const;
    void        set_reflective_attr()
                  { DBG_PROG  ((char*)&header.attributes)[0] =
                                        ((char*)&header.attributes)[0] & 0x7f; }
    void        set_transparency_attr()
                  { DBG_PROG  ((char*)&header.attributes)[0] =
                                        ((char*)&header.attributes)[0] | 0x80; }
    void        set_glossy_attr()
                  { DBG_PROG  ((char*)&header.attributes)[0] =
                                        ((char*)&header.attributes)[0] & 0xbf; }
    void        set_matte_attr()
                  { DBG_PROG  ((char*)&header.attributes)[0] =
                                        ((char*)&header.attributes)[0] | 0x40; }
    void        set_positive_attr()
                  { DBG_PROG  ((char*)&header.attributes)[0] =
                                        ((char*)&header.attributes)[0] & 0xdf; }
    void        set_negative_attr()
                  { DBG_PROG  ((char*)&header.attributes)[0] =
                                        ((char*)&header.attributes)[0] | 0x20; }
    void        set_color_attr()
                  { DBG_PROG  ((char*)&header.attributes)[0] =
                                        ((char*)&header.attributes)[0] & 0xef; }
    void        set_gray_attr()
                  { DBG_PROG  ((char*)&header.attributes)[0] =
                                        ((char*)&header.attributes)[0] | 0x10; }
    std::string renderingIntent () const
                  { DBG_PROG  return renderingIntentName( icValue(
                                            header.renderingIntent ) ); }
    void        set_renderingIntent ()
                  { DBG_PROG  header.renderingIntent =icValue( 0 ); }
    void        set_renderingIntent (icUInt32Number intent)
                  { DBG_PROG  header.renderingIntent =icValue( intent ); }
    void        set_illuminant ()
                  { DBG_PROG  icValueXYZ (&header.illuminant,
                                                  0.9642, 1.0000,0.8249); }
    const char* creatorName () const
                  { DBG_PROG return cp_nchar((char*)&(header.  creator),
                                                      sizeof (icSignature)); }
    void        set_creator ()
                  { DBG_PROG const char* m = {"Oyra"};
                    header.creator = *(icSignature*)m; }
    void        setID ()
                  {/*char* m = ((char*)&header)[84]; *m = "-";*/ } // v4
    std::string print () const;
    std::string print_long () const;
};



#endif //ICC_PROFILE_HEADER_H
