// Author:    Kai-Uwe Behrmann
// Copyright: Kai-Uwe Behrmann <ku.b@gmx.de>
// Date:      03. 08. 2004

#if 1
  #ifndef DEBUG
   #define DEBUG
  #endif
  #define DEBUG_Parser
#endif

#include "icc_profiler.h"
#include "icc_utils.h"

#define _(text) text
#define g_message printf



/**
  *  @brief ICCparser Funktionen
  */ 

Parser::Parser()
{
}

void
Parser::load (void *data)
{
  //DBG
}


std::string
Parser::print()
{
  string s = _("Dateikopf ungültig");
  return s;
}


