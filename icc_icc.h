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
 * ICC
 * 
 */

/* Date:      Mai 2004 */

#ifndef ICC_ICC_H
#define ICC_ICC_H

#if defined(WIN32)
#define LCMS_WIN_TYPES_ALREADY_DEFINED 1
typedef char *LPSTR;
typedef void *LPVOID;
typedef void* LCMSHANDLE;

#define cdecl
#endif

#include <lcms.h>

/* additionals - not defined in icc34.h */

#ifndef icSigProfileSequenceIdentifierTag
#define icSigProfileSequenceIdentifierTag 0x70736964 /* psid */
#endif

#ifndef icSigChromaticityType
#define icSigChromaticityType 0x6368726D /* chrm */
#endif

#ifndef icSigMCH5Data
#define icSigMCH5Data 1296255029 /* MCH5 */
#endif
#ifndef icSigMCH6Data
#define icSigMCH6Data 1296255030 /* MCH6 */
#endif
#ifndef icSigMCH7Data
#define icSigMCH7Data 1296255031 /* MCH7 */
#endif
#ifndef icSigMCH8Data
#define icSigMCH8Data 1296255032 /* MCH8 */
#endif
#ifndef icSigMCH9Data
#define icSigMCH9Data 1296255033 /* MCH9 */
#endif
#ifndef icSigMCHAData
#define icSigMCHAData 1296255041 /* MCHA */
#endif
#ifndef icSigMCHBData
#define icSigMCHBData 1296255042 /* MCHB */
#endif
#ifndef icSigMCHCData
#define icSigMCHCData 1296255043 /* MCHC */
#endif
#ifndef icSigMCHDData
#define icSigMCHDData 1296255044 /* MCHD */
#endif
#ifndef icSigMCHEData
#define icSigMCHEData 1296255045 /* MCHE */
#endif
#ifndef icSigMCHFData
#define icSigMCHFData 1296255046 /* MCHF */
#endif
#ifndef icSiglcms
#define icSiglcmsData 1818455411 /* lcms */
#endif
#ifndef icSigLuvKData
#define icSigLuvKData 1282766411 /* LuvK */
#endif
#ifndef icSigJab1Data
#define icSigJab1Data 1247896113 /* Jab1 */
#endif
#ifndef icSigJab2Data
#define icSigJab2Data 1247896114 /* Jab2 */
#endif
#ifndef icSigJcj2Data
#define icSigJcj2Data 1248029234 /* Jcj2 */
#endif
#ifndef icSigChromaticityTag
#define icSigChromaticityTag 1667789421 /* chrm */
#endif
#ifndef icSigHPGamutDescTag
#define icSigHPGamutDescTag 1735226433 /* gmtA */
#endif

#endif /* ICC_ICC_H */
