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
 * ICC Helfer
 * 
 */

// Date:      Mai 2004

#ifndef ICC_HELFER_H
#define ICC_HELFER_H

#include <string>
#include <vector>

#include "icc_utils.h"
#include "icc_icc.h"

// I18N
void initialiseI18N();

// File macros / teilweise aus config.h
#ifdef __WIN32__
#define PATH_SHELLSTRING ""
#else
#define PATH_SHELLSTRING "export PATH=$PATH:" PREFIX "/bin:~/bin; "
#endif

#if HAVE_FLTK
#include <FL/Fl_Menu_Item.H>
void        menue_translate( Fl_Menu_Item *menueleiste );
void        dbgFltkEvents  ( int           event);
std::string dbgFltkEvent   ( int           event);
#endif

// interne Funktionen

const char* cp_nchar (char* text, int n);


// Helferfunktionen
// ICC Helfer definiert in icc_helfer.cpp
icUInt16Number          icValue   (icUInt16Number val);
icUInt32Number          icValue   (icUInt32Number val);
unsigned long           icValue   (icUInt64Number val);
double                  icSFValue (icS15Fixed16Number val);
double                  icUFValue (icU16Fixed16Number val);
icS15Fixed16Number      icValueSF (double val);
icU16Fixed16Number      icValueUF (double val);
icInt16Number                     icValue   (icInt16Number val);
icInt32Number                     icValue   (icInt32Number val);
int                     icValue   (icInt64Number val);
icColorSpaceSignature   icValue   (icColorSpaceSignature val);
icPlatformSignature     icValue   (icPlatformSignature val);
icProfileClassSignature icValue   (icProfileClassSignature val);
icTagSignature          icValue   (icTagSignature val);
void                    icValueXYZ(icXYZNumber*, double X, double Y, double Z);

double*                 XYZto_xyY (double* XYZ);
double*                 XYZto_xyY (std::vector<double> XYZ);

std::string         renderingIntentName ( int intent);
std::string         getColorSpaceName   ( icColorSpaceSignature color);
std::vector<std::string> getChannelNames( icColorSpaceSignature color);
std::string         getDeviceClassName  ( icProfileClassSignature cl);
std::string         getPlatformName     ( icPlatformSignature platform);
std::string         getSigTagName( icTagSignature  sig );
std::string         getSigTagDescription( icTagSignature  sig );
std::string         getSigTypeName( icTagTypeSignature  sig );
std::string         getSigTechnology( icTechnologySignature sig );
std::string         getIlluminant( icIlluminant illu );
std::string         getStandardObserver( icStandardObserver obsv );
std::string         getMeasurementGeometry( icMeasurementGeometry measgeo );
std::string         getMeasurementFlare( icMeasurementFlare flare );
std::string         printDatum( icDateTimeNumber date );

namespace icc_examin_ns {
  // Zeit / Uhr
  void sleep(double Sekunden);
  // Fl_Thread's sperren / freigeben
  void lock(const char *file, int line);
  void unlock(void *widget, const char *file, int line);
}

// Datendarstellung
std::string         zeig_bits_bin      (const void* speicher, int groesse);

// Datei E/A
char*     ladeDatei                  ( std::string dateiname,
                                       size_t     *size );
void      saveMemToFile              ( const char *filename,
                                       const char *block,
                                       int         size );
#include "icc_speicher.h"
Speicher  dateiNachSpeicher          ( const std::string & dateiname );
void      dateiNachSpeicher          ( Speicher & s,
                                       const std::string & dateiname );
void      speicherNachDatei          ( std::string & dateiname,
                                       Speicher & s );

double    holeDateiModifikationsZeit ( const char *fullFileName );


// Texthelfer
namespace icc_parser {
  extern const char *alnum;
  extern const char *alpha;
  extern const char *numerisch;
  extern const char *ziffer;
  extern const char *leer_zeichen;

  int               suchenErsetzen    ( std::string            &text,
                                        std::string            &suchen,
                                        std::string            &ersetzen,
                                        std::string::size_type  pos );
  int               suchenErsetzen    ( std::string            &text,
                                        const char             *suchen,
                                        const char             *ersetzen,
                                        std::string::size_type  pos );
  std::vector<std::string> zeilenNachVector (std::string &text);
  std::string::size_type sucheWort    ( std::string            &text,
                                        std::string             wort,
                                        std::string::size_type  pos );

  struct ZifferWort {
    std::pair<bool,std::string> wort;
    std::pair<bool,double>      zahl;
    std::pair<bool,int>         ganz_zahl;
  };

  std::vector<ZifferWort>       unterscheideZiffernWorte ( std::string &zeile,
                                                 bool anfuehrungsstriche_setzen,
                                                 const char *trennzeichen );
}

// Callback Struktur
typedef struct StructVoidInt
{
  void* data;
  int   wert;
};

#endif //ICC_HELFER_H
