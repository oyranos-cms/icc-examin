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

// Date:      04. 05. 2004

#if 0
# ifndef DEBUG
#  define DEBUG
# endif
# define DEBUG_ICCHEADER
#endif

#include "icc_profile_header.h"
#include "icc_utils.h"

#include <cmath>
#include <sstream>

#define g_message printf

/**
  *  @brief ICCheader functions
  */ 

ICCheader::ICCheader()
{ DBG_PROG_START
  valid = false;
  DBG_PROG_ENDE
}

void
ICCheader::load (void *data)
{ DBG_PROG_START
  if (data == NULL)
  {
    for (int i = 0; i < 128; i++)
      ((char*)&header)[i] = 0;
    valid = false;
    DBG_PROG_ENDE
    return;
  }

  memcpy ((void*)&header, data, sizeof (icHeader));
# ifdef DEBUG_ICCHEADER
  DBG_NUM_S( sizeof (icHeader) << " genommen" )
# endif
  if (header.size > 0) {
    valid = true;
# ifdef DEBUG_ICCHEADER
    DBG_NUM_V( size() )
# endif
  } else {
    valid = false;
  }
  DBG_PROG_ENDE
}


void
ICCheader::setCurrentDate (void)
{ DBG_PROG_START
  struct tm *tm_;
  time_t t = time(0);
  tm_ = localtime(&t);
  header.date.day = icValue((icUInt16Number)tm_->tm_mday);
  DBG_PROG_V(tm_->tm_mday)
  header.date.month = icValue((icUInt16Number)(tm_->tm_mon+1));
  DBG_PROG_V(tm_->tm_mon+1)
  header.date.year = icValue((icUInt16Number)(tm_->tm_year+1900));
  DBG_PROG_V(tm_->tm_year+1900)
  header.date.hours = icValue((icUInt16Number)tm_->tm_hour);
  DBG_PROG_V(tm_->tm_hour)
  header.date.minutes = icValue((icUInt16Number)tm_->tm_min);
  DBG_PROG_V(tm_->tm_min)
  header.date.seconds = icValue((icUInt16Number)tm_->tm_sec);
  DBG_PROG_V(tm_->tm_sec)
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
    s << _("Transparent, ");
  else
    s << _("Reflective, ");

  if (f[0] & maske2)  
    s << _("matte, ");
  else
    s << _("glossy, ");

  if (f[0] & maske3)  
    s << _("negative, ");
  else
    s << _("positive, ");

  if (f[0] & maske4)  
    s << _("black/white");
  else
    s << _("colour");


# ifdef DEBUG
  DBG_PROG_S( (long)header.attributes )
  if (icc_debug)
  {
    LEVEL cout << "           ";
    char* ptr = (char*) &(header.attributes);
    for (int i = 0; i < 8 ; i++)
      cout << (int)ptr[i] << " ";
    cout << endl;
  }
# endif
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
    s << _("Profile is embedded and ");
  else
    s << _("Profile is not embedded and ");

  if (f[0] & maske2)  
    s << _("can not be used independently from image.");
  else
    s << _("can be used independently from image.");

# ifdef DEBUG
  DBG_NUM_S( (int)f[0] << " " << (long)header.flags )
  if (icc_debug)
  { LEVEL cout << "           ";
    char* ptr = (char*) &(header.flags);
    for (int i = 0; i < 8 ; i++)
      cout << (int)ptr[i] << " ";
    cout << endl;
  }
# endif
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
ICCheader::dateTime (void) const
{ DBG_PROG_START
  std::string s;
  
  s = printDatum(header.date);

  DBG_PROG_ENDE
  return s;
}

std::string
ICCheader::print_long() const
{ DBG_PROG_START
# ifdef DEBUG_ICCHEADER
  cout << sizeof (icSignature) << " " << sizeof (icUInt32Number)<< endl;
# endif
  std::string cm = cmmName();
  std::string mg = magicName();
  std::string ma = manufacturerName();
  std::string mo = modelName();
  std::string cr = creatorName();
  std::stringstream s; DBG_PROG
  s << _("found no header"); DBG_PROG
  
  //cout << "char icSignature ist " << sizeof (icSignature) << endl;
  if (valid) { DBG_PROG
    s.str("");
    s << _("ICC profile header") << ":\n"<< endl \
      <<  "    " << _("Size:        ") <<
                       size() << " " << _("bytes") << endl \
      <<  "    " << _("CMM:         ") << cm << endl \
      <<  "    " << _("Version:     ") << versionName() << endl \
      <<  "    " << _("Class:       ") <<
                       getDeviceClassName(deviceClass()) << endl \
      <<  "    " << _("Colour Space:")  <<
                       getColorSpaceName(colorSpace()) << endl \
      <<  "    " << _("PCS:         ") <<
                       getColorSpaceName(pcs()) << endl \
      <<  "    " << _("Date:        ") << printDatum(header.date) << endl\
      <<  "    " << _("Magic:       ") << mg << endl \
      <<  "    " << _("Platform:    ") << platform() << endl \
      <<  "    " << _("Flags:       ") << flags() << endl \
      <<  "    " << _("Manufacturer:") << ma << endl \
      <<  "    " << _("Model:       ") << mo << endl \
      <<  "    " << _("Attributes:  ") <<
                       attributes() << endl \
      <<  "    " << _("Intent:      ") <<
                       renderingIntent() << endl \
      <<  "    " << _("Illuminant:  ") << "X=" <<
                       icSFValue(header.illuminant.X) << ", Y=" << \
                       icSFValue(header.illuminant.Y) << ", Z=" << \
                       icSFValue(header.illuminant.Z) << endl \
      <<  "    " << _("created by:  ") << cr << endl ;
  } DBG_PROG_ENDE
  return s.str();
}

std::string
ICCheader::print() const
{ DBG_PROG_START
  std::string s = _("File header invalid");
  if (valid)
    // format sensible, as this string is the header in the tag browser table
    s = _("    head  head    128 File header");
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


