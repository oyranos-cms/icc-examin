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
 */

/** @brief  thread locking for Oyranos
 * 
 */

 //! @date      2008/01/22


#ifndef ICC_THREADS_H
#define ICC_THREADS_H

#include "icc_utils.h"
#include "icc_helfer.h"
#include <oyranos_alpha.h>

namespace icc_examin_ns {

  oyranos::oyPointer iccStruct_LockCreate ( oyranos::oyStruct_s * obj );
  void       iccLockRelease          ( oyranos::oyPointer  lock,
                                       const char        * marker,
                                       int                 line );
  void       iccLock                 ( oyranos::oyPointer  lock,
                                       const char        * marker,
                                       int                 line );
  void       iccUnLock               ( oyranos::oyPointer  look,
                                       const char        * marker,
                                       int                 line );

}

#endif //ICC_THREADS_H
