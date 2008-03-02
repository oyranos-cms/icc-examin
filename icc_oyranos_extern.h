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
  double       lab[3];     /*!< Lab  L: 0...1  a/b: -1.28...1.28 */
  double       channels[32];    /*!< eigther parsed or calculated otherwise */
  double       moni_rgb[3];     /*!< monitor colours */
  icColorSpaceSignature sig;    /*!< ICC colour space signature */
  char * names_chan[32];        /*!< user visible channel description */
  char * name;                  /*!< normal user visible name (A1-MySys) */
  char * name_long;        /*!< full user description (A1-MySys from Oyranos) */
  char * nick_name;             /*!< few letters for mass representation (A1) */
  char * blob;                  /*!< advanced CGATS / ICC ? */
  size_t blob_len;              /*!< advanced CGATS / ICC ? */
  char * ref_file;              /*!< ICC */
  oyAllocFunc_t allocateFunc;
  oyDeAllocFunc_t deallocateFunc;
} oyNamedColour_s;

oyNamedColour_s* oyNamedColourCreate ( double      * lab,
                                       double      * chan,
                                       icColorSpaceSignature sig,
                                       const char ** names_chan,
                                       const char  * name,
                                       const char  * name_long,
                                       const char  * nick,
                                       const char  * blob,
                                       int           blob_len,
                                       const char  * icc_ref,
                                       oyAllocFunc_t allocateFunc,
                                       oyDeAllocFunc_t deallocateFunc);
void             oyNamedColourRelease( oyNamedColour_s ** colour );
oyNamedColour_s* oyNamedColourCopy   ( oyNamedColour_s  * colour,
                                       oyAllocFunc_t      allocateFunc,
                                       oyDeAllocFunc_t    deallocateFunc);

void             oyNamedColourSetLab ( oyNamedColour_s * colour,
                                       double * lab );
void             oyNamedColourGetLab ( oyNamedColour_s * colour,
                                       double * lab );
const char *     oyNamedColourGetName( oyNamedColour_s * colour );
void             oyNamedColourSetName( oyNamedColour_s * colour );
const char *     oyNamedColourGetNick( oyNamedColour_s * colour );
void             oyNamedColourSetNick( oyNamedColour_s * colour );
const char *     oyNamedColourGetDescription( oyNamedColour_s * colour );
void             oyNamedColourSetDescription( oyNamedColour_s * colour );

void             oyCopyColour            ( double * from, double * to, int n,
                                           icColorSpaceSignature sig );

/* convenient functions */
int              oyColourSpaceGetChannelCount ( icColorSpaceSignature sig );
const char *     oyColourSpaceGetName( icColorSpaceSignature sig );


#ifdef __cplusplus
} /* namespace oyranos */
} /* extern "C" */
#endif /* __cplusplus */

#endif /* ICC_OYRANOS_EXTERN_H */
