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

#define oyIGNORE_DEPRECATED 108

#include "config.h"
#include <oyranos.h>
#include <oyranos_icc.h>
#include <alpha/oyranos_alpha.h>

#define HAVE_LCMS 1

#include "icc_utils.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifndef HAVE_OY
typedef void* (*oyAllocFunc_t)         (size_t size);
typedef void  (*oyDeAllocFunc_t)       (void *data);
#endif
void* myCAllocFunc(size_t size);
void  myCDeAllocFunc(void * buf);


#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* ICC_OYRANOS_EXTERN_H */
