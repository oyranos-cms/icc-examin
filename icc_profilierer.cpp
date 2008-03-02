// Author:    Kai-Uwe Behrmann
// Copyright: Kai-Uwe Behrmann <ku.b@gmx.de>
// Date:      03. 08. 2004

#if 1
  #ifndef DEBUG
   #define DEBUG
  #endif
  #define DEBUG_PROFILIERER
#endif

#include "icc_profilierer.h"
#include "icc_utils.h"

#define _(text) text
#define g_message printf



/**
  *  @brief Profilierer Funktionen
  */ 

Profilierer::Profilierer()
{
  
}

void
Profilierer::load (ICCprofile& profil)
{
  _measurement = profil.getMeasurement();
  DBG
}


std::string
Profilierer::print()
{
  std::string s = _("Dateikopf ungültig");
  return s;
}

ICCprofile
Profilierer::matrix()
{
  ICCprofile p;
  return p;
}


