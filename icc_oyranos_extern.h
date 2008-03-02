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
 * CMS
 * 
 */

/* Date:      25. 11. 2004 */


#ifndef ICC_OYRANOS_EXTERN_H
#define ICC_OYRANOS_EXTERN_H

#include "config.h"
#include <oyranos/oyranos.h>
#include <oyranos/oyranos_icc.h>
#include <oyranos/oyranos_alpha.h>
#include "icc_icc.h"

#include "icc_utils.h"

#ifdef __cplusplus
extern "C" {
namespace oyranos
{
#endif /* __cplusplus */

#ifndef HAVE_OY
typedef void* (*oyAllocFunc_t)         (size_t size);
typedef void  (*oyDeAllocFunc_t)       (void *data);
#endif
void* myCAllocFunc(size_t size);
void  myCDeAllocFunc(void * buf);

icUInt16Number          oyValueUInt16   (icUInt16Number val);
icUInt32Number          oyValueUInt32   (icUInt32Number val);
unsigned long           oyValueUInt64   (icUInt64Number val);
icColorSpaceSignature   oyValueCSpaceSig(icColorSpaceSignature val);
icPlatformSignature     oyValuePlatSig  (icPlatformSignature val);
icProfileClassSignature oyValueProfCSig (icProfileClassSignature val);
icTagSignature          oyValueTagSig   (icTagSignature val);


typedef enum {
  oyOBJECT_TYPE_NONE,
  oyOBJECT_TYPE_DISPLAY_S,
  oyOBJECT_TYPE_NAMED_COLOUR_S,
  oyOBJECT_TYPE_NAMED_COLOURS_S,
  oyOBJECT_TYPE_COLOUR_PROFILE
} oyOBJECT_TYPE;


/** @brief colour patch with meta informations
 *
 *  Data management on library side.
 *  User can control memory management at creation time.
 *
 *  It has the complexity of a object, and should not be accessed directly.
 *
 *  since: (ICC Examin: version 0.45)
 *
 *  TODO: needs to be Xatom compatible
 */
typedef struct {
  oyOBJECT_TYPE type;           /*!< internal struct type */
  double *     channels;        /*!< eigther parsed or calculated otherwise */
  int          channels_n;      /*!< number of channels */
  char   **    names_chan;      /*!< user visible channel description */
  char * name;                  /*!< normal user visible name (A1-MySys) */
  char * name_long;        /*!< full user description (A1-MySys from Oyranos) */
  char * nick_name;             /*!< few letters for mass representation (A1) */
  char * blob;                  /*!< advanced CGATS / ICC ? */
  size_t blob_len;              /*!< advanced CGATS / ICC ? */
  oyProfile_s *profile;         /*!< ICC */
  oyAllocFunc_t allocateFunc;
  oyDeAllocFunc_t deallocateFunc;
  oyPointer * backdoor;
} oyNamedColour_s;

oyNamedColour_s*  oyNamedColourCreate ( const double      * chan,
                                        int           channels_n,
                                        const char  * name,
                                        const char  * name_long,
                                        const char  * nick,
                                        const char  * blob,
                                        int           blob_len,
                                        oyProfile_s * profile_ref,
                                        oyAllocFunc_t allocateFunc,
                                        oyDeAllocFunc_t deallocateFunc);
void              oyNamedColourRelease( oyNamedColour_s ** colour );
oyNamedColour_s*  oyNamedColourCopy   ( const oyNamedColour_s  * colour,
                                        oyAllocFunc_t      allocateFunc,
                                        oyDeAllocFunc_t    deallocateFunc);

oyProfile_s*      oyNamedColourGetSpace( oyNamedColour_s * colour );
int               oyNamedColourSetSpace( oyNamedColour_s * colour,
                                        oyProfile_s      * profile_ref );
void              oyNamedColourSetChannels( oyNamedColour_s * colour,
                                        const double     * channels,
                                        int                channels_n );
const double *    oyNamedColourGetChannelsConst(oyNamedColour_s * colour);
int               oyNamedColourGetChannelsCount(oyNamedColour_s * colour);
void              oyNamedColourSetChannelNames( oyNamedColour_s * colour,
                                        const char      ** names_chan );
const char **     oyNamedColourGetChannelNames( const oyNamedColour_s * colour);
const char *      oyNamedColourGetName( oyNamedColour_s  * colour );
void              oyNamedColourSetName( oyNamedColour_s  * colour );
const char *      oyNamedColourGetNick( oyNamedColour_s  * colour );
void              oyNamedColourSetNick( oyNamedColour_s  * colour );
const char *      oyNamedColourGetDescription( oyNamedColour_s * colour );
void              oyNamedColourSetDescription( oyNamedColour_s * colour );

/** @brief list of colour patches
 *
 *  Data management on library side.
 *  User can control memory management at creation time.
 *
 *  It has the complexity of a object, and should not be accessed directly.
 *
 *  since: (ICC Examin: version 0.45)
 *
 *  TODO: make the object non visible
 */
typedef struct {
  oyOBJECT_TYPE type;           /*!< internal struct type */
  char * name;                  /*!< normal user visible name (autumn) */
  char * name_long;        /*!< full user description (My autumn swatches ) */
  int    size;                  /*!< list entries */
  oyNamedColour_s ** colours;   /*!< colour list */
  oyAllocFunc_t allocateFunc;
  oyDeAllocFunc_t deallocateFunc;
  oyPointer * backdoor;
} oyNamedColours_s;

oyNamedColours_s* oyNamedColoursCreate( const char  * name,
                                        const char  * name_long,
                                        oyAllocFunc_t allocateFunc,
                                        oyDeAllocFunc_t deallocateFunc);
int               oyNamedColoursSize  ( oyNamedColours_s * swatch );
oyNamedColour_s*  oyNamedColoursGet   ( oyNamedColours_s * swatch,
                                        int           position );
int               oyNamedColoursAdd   ( oyNamedColours_s * swatch,
                                        oyNamedColour_s  * patch,
                                        int           position );
int               oyNamedColoursRemove( oyNamedColours_s * swatch,
                                        int           position );
int               oyNamedColoursRelease( oyNamedColours_s ** swatch );
oyNamedColours_s* oyNamedColoursCopy  ( const oyNamedColours_s * colours,
                                        oyAllocFunc_t      allocateFunc,
                                        oyDeAllocFunc_t    deallocateFunc);
const char *      oyNamedColoursGetName( const oyNamedColours_s * colours );
void              oyNamedColoursSetName( oyNamedColours_s * colours );
const char *      oyNamedColoursGetDescription(const oyNamedColours_s *colours);
void              oyNamedColoursSetDescription( oyNamedColours_s * colours );


void              oyCopyColour        ( const double * from, double * to, int n,
                                        oyProfile_s       * profile_ref,
                                        int                 channels_n );

/* ICC functions */
int              oyColourSpaceGetChannelCount ( oyProfile_s * profile_ref );
const char *     oyColourSpaceGetName ( oyProfile_s       * profile_ref );
const char *     oyColourSpaceGetChannelName( oyProfile_s * profile_ref,
                                        int                 channel );


#ifdef __cplusplus
} /* namespace oyranos */
} /* extern "C" */
#endif /* __cplusplus */

#endif /* ICC_OYRANOS_EXTERN_H */
