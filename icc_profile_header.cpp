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

// Date:      04. 05. 2004

#if 0
  #ifndef DEBUG
   #define DEBUG
  #endif
  #define DEBUG_ICCHEADER
#endif

#include "icc_profile.h"
#include "icc_utils.h"

#include <cmath>

#define g_message printf

/**
  *  @brief ICCheader Funktionen
  */ 

ICCheader::ICCheader()
{ DBG_PROG_START
  valid = false;
  DBG_PROG_ENDE
}

void
ICCheader::load (void *data)
{ DBG_PROG_START
  if (data == NULL) {
    for (int i = 0; i < 128; i++)
      ((char*)&header)[i] = 0;
    valid = false;
    DBG_PROG_ENDE
    return;
  }

  memcpy ((void*)&header, data, sizeof (icHeader));
  #ifdef DEBUG_ICCHEADER
  DBG_NUM_S( sizeof (icHeader) << " genommen" )
  #endif
  if (header.size > 0) {
    valid = true;
  #ifdef DEBUG_ICCHEADER
    DBG_NUM_V( size() )
  #endif
  } else {
    valid = false;
  }
  DBG_PROG_ENDE
}


void
ICCheader::set_current_date (void)
{ DBG_PROG_START // TODO Zeit setzen
  header.date.day = icValue((icUInt16Number)1);
  header.date.month = icValue((icUInt16Number)9);
  header.date.year = icValue((icUInt16Number)2004);
  header.date.hours = icValue((icUInt16Number)12);
  header.date.minutes = icValue((icUInt16Number)0);
  header.date.seconds = icValue((icUInt16Number)0);
  DBG_PROG_ENDE
}

std::string
ICCheader::attributes (void) const
{ DBG_PROG_START
  std::stringstream s;
  char* f = (char*) &(header.attributes);
  unsigned char         maske1 = 0x80;
  unsigned char         maske2 = 0x40;
  unsigned char         maske3 = 0x20;
  unsigned char         maske4 = 0x10;

  if (f[0] & maske1)
    s << _("Durchsicht, ");
  else
    s << _("Aufsicht, ");

  if (f[0] & maske2)  
    s << _("matt, ");
  else
    s << _("glänzend, ");

  if (f[0] & maske3)  
    s << _("negativ, ");
  else
    s << _("positiv, ");

  if (f[0] & maske4)  
    s << _("schwarz/weiss");
  else
    s << _("farbig");


  #ifdef DEBUG
  DBG_PROG_S( (long)header.attributes )
  if (icc_debug)
  {
    LEVEL cout << "           ";
    char* ptr = (char*) &(header.attributes);
    for (int i = 0; i < 8 ; i++)
      cout << (int)ptr[i] << " ";
    cout << endl;
  }
  #endif
  DBG_PROG_ENDE
  return s.str();
}

std::string
ICCheader::flags (void) const
{ DBG_PROG_START
  std::stringstream s;
  char* f = (char*) &(header.flags);
  unsigned char         maske1 = 0x80;
  unsigned char         maske2 = 0x40;

  if (f[0] & maske1)
    s << _("Farbprofil ist eingebettet und ");
  else
    s << _("Farbprofil ist nicht eingebettet und ");

  if (f[0] & maske2)  
    s << _("kann nicht unabhängig vom Bild verwendet werden.");
  else
    s << _("kann unabhängig vom Bild verwendet werden.");

  #ifdef DEBUG
  DBG_NUM_S( (int)f[0] << " " << (long)header.flags )
  if (icc_debug)
  { LEVEL cout << "           ";
    char* ptr = (char*) &(header.flags);
    for (int i = 0; i < 8 ; i++)
      cout << (int)ptr[i] << " ";
    cout << endl;
  }
  #endif
  DBG_PROG_ENDE
  return s.str();
}

std::string
ICCheader::versionName (void) const
{ DBG_PROG_START
  std::stringstream s;
  char* v = (char*)&(header.version);
  
  s << (int)v[0] << "." << (int)v[1]/16 << "." << (int)v[1]%16;

  DBG_PROG_ENDE
  return s.str();
}

std::string
ICCheader::print_long() const
{ DBG_PROG_START
  #ifdef DEBUG_ICCHEADER
  cout << sizeof (icSignature) << " " << sizeof (icUInt32Number)<< endl;
  #endif
  std::string cm = cmmName();
  std::string mg = magicName();
  std::string ma = manufacturerName();
  std::string mo = modelName();
  std::string cr = creatorName();
  std::stringstream s; DBG_PROG
  s << "kein Dateikopf gefunden"; DBG_PROG
  
  //cout << "char icSignature ist " << sizeof (icSignature) << endl;
  if (valid) { DBG_PROG
    s.str("");
    s << "ICC Dateikopf:\n"<< endl \
      <<  "    " << _("Größe") << ":       " <<
                       size() << " " << _("bytes") << endl \
      <<  "    " << _("CMM") << ":         " << cm << endl \
      <<  "    " << _("Version") << ":     " << versionName() << endl \
      <<  "    " << _("Klasse") << ":      " <<
                       getDeviceClassName(deviceClass()) << endl \
      <<  "    " << _("Farbraum") << ":    " <<
                       getColorSpaceName(colorSpace()) << endl \
      <<  "    " << _("PCS") << ":         " <<
                       getColorSpaceName(pcs()) << endl \
      <<  "    " << printDatum(header.date) << endl \
      <<  "    " << _("Magic") << ":       " << mg << endl \
      <<  "    " << _("Plattform") << ":   " << platform() << endl \
      <<  "    " << _("Flags") << ":       " << flags() << endl \
      <<  "    " << _("Hersteller") << ":  " << ma << endl \
      <<  "    " << _("Model") << ":       " << mo << endl \
      <<  "    " << _("Attribute") << ":   " <<
                       attributes() << endl \
      <<  "    " << _("Übertragung") << ": " <<
                       renderingIntent() << endl \
      <<  "    " << _("Beleuchtung") << ": X=" <<
                       icSFValue(header.illuminant.X) << ", Y=" << \
                       icSFValue(header.illuminant.Y) << ", Z=" << \
                       icSFValue(header.illuminant.Z) << endl \
      <<  "    " << _("erzeugt von") << ": " << cr << endl ;
  } DBG_PROG_ENDE
  return s.str();
}

std::string
ICCheader::print() const
{ DBG_PROG_START
  std::string s = _("Dateikopf ungültig");
  if (valid)
    s = _("    Kopf        Dateikopf     128 Dateikopf");
  DBG_PROG_ENDE
  return s;
}

void
ICCheader::clear()
{ DBG_PROG_START
  char *zeiger = (char*) &header;
  for(unsigned i = 0; i < sizeof(icHeader); ++i)
    zeiger[i] = 0;
  DBG_PROG_ENDE
}


