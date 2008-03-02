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
 * Werkzeuge
 * 
 */

// Date:      August 2004

#ifndef ICC_UTILS_H
#define ICC_UTILS_H

#include <cstdio>		// printf()
#include <exception>		// class expeption
#include <new>			// bad_alloc()
#include <iostream>
#include <sstream>

#include "config.h"

#define USE_GETTEXT
#ifdef USE_GETTEXT
  #include <libintl.h>
  #define _(text) gettext(text)
#else
  #define _(text) text
#endif

// ByteOrder on Solaris
#ifndef BIG_ENDIAN
  #define BIG_ENDIAN 4321
#endif
#ifndef LITTLE_ENDIAN
  #define LITTLE_ENDIAN 1234
#endif
#ifdef _BIG_ENDIAN
  #ifdef BYTE_ORDER
    #undef BYTE_ORDER
  #endif
  #define BYTE_ORDER BIG_ENDIAN
#endif
#ifdef _LITTLE_ENDIAN
  #ifdef BYTE_ORDER
    #undef BYTE_ORDER
  #endif
  #define BYTE_ORDER LITTLE_ENDIAN
#endif

void dbgWriteF (std::stringstream & ss);
extern std::stringstream debug_s;
#define dbgWrite(ss) { debug_s.str(""); debug_s << ss; dbgWriteF(debug_s); }
// look in icc_utils.cpp for the WRITE_DBG definition

#define cout std::cout
#define endl std::endl

extern int level_PROG;
extern int icc_debug;

/*  icc_debug wird mit der Umgebungsvariable ICCEXAMIN_DEBUG in main() gesetzt
 *  Niveaus:
 *   0: DBG              // nur zwischenzeitlich verwenden
 *   1: DBG_NUM
 *   2: DBG_PROG
 *   3: DBG_MEM
 *   4: Datei E/A
 *   9: DBG_PROG_START & DBG_PROG_ENDE
 *
 *   [1,2,...,9]    diese und alle kleineren Kategorien
 *   10+[1,2,...,9] einzig diese Kategorie , z.B. ICCEXAMIN_DEBUG=13 wählt alle
 *                                                Speicherinformationen aus
 *
 */


#define DBG_UHR_ (double)clock()/(double)CLOCKS_PER_SEC

#define DBG_T_     dbgWrite ( __FILE__<<":"<<__LINE__ <<" "<< __func__ << "() " << DBG_UHR_ << " ");
#define LEVEL      { for (int i = 0; i < level_PROG; i++) dbgWrite (" "); }
#define DBG_       { LEVEL dbgWrite ("        "); DBG_T_ dbgWrite (endl); }
#define DBG_S_(txt){ LEVEL dbgWrite ("        "); DBG_T_ dbgWrite (txt << endl); }
#define DBG_V_(txt){ LEVEL dbgWrite ("        "); DBG_T_ dbgWrite (#txt << " " << txt << endl);}
#define DBG        DBG_
#define DBG_START  {level_PROG++; for (int i = 0; i < level_PROG; i++) dbgWrite ( "+"); dbgWrite (" Start: "); DBG_T_ dbgWrite (endl); }
#define DBG_ENDE   { for (int i = 0; i < level_PROG; i++) dbgWrite ("+"); dbgWrite (" Ende:  "); DBG_T_ level_PROG--; dbgWrite (endl); }
#define DBG_S(txt) DBG_S_(txt)
#define DBG_V(txt) DBG_V_(txt)

#ifdef DEBUG
#define DBG_BED(n) if (icc_debug >= n && icc_debug < 10 || icc_debug == 1##n)
#define DBG_BED2(n1,n2) if (icc_debug >= n1 && icc_debug < 10 || icc_debug == 1##n1 || icc_debug >= n2 && icc_debug < 10 || icc_debug == 1##n2 )
#define DBG_NUM        DBG_BED(1) DBG
#define DBG_NUM_S(txt) DBG_BED(1) DBG_S(txt)
#define DBG_NUM_V(txt) DBG_BED(1) DBG_V(txt)
#else
#define DBG_NUM ;
#define DBG_NUM_S(txt) ;
#define DBG_NUM_V(txt) ;
#endif
#ifdef DEBUG
#define DBG_PROG        DBG_BED(2) DBG
#define DBG_PROG_START  DBG_BED2(2,9) DBG_START
#define DBG_PROG_ENDE   DBG_BED2(2,9) DBG_ENDE
#define DBG_PROG_S(txt) DBG_BED(2) DBG_S(txt)
#define DBG_PROG_V(txt) DBG_BED(2) DBG_V(txt)
#else
#define DBG_PROG ;
#define DBG_PROG_START ;
#define DBG_PROG_ENDE ;
#define DBG_PROG_S(txt) ;
#define DBG_PROG_V(txt) ;
#endif
#ifdef DEBUG
#define DBG_MEM        DBG_BED(3) DBG
#define DBG_MEM_START  DBG_BED(3) DBG_START
#define DBG_MEM_ENDE   DBG_BED(3) DBG_ENDE
#define DBG_MEM_S(txt) DBG_BED(3) DBG_S(txt)
#define DBG_MEM_V(txt) DBG_BED(3) DBG_V(txt)
#else
#define DBG_MEM ;
#define DBG_MEM_START ;
#define DBG_MEM_ENDE ;
#define DBG_MEM_S(txt) ;
#define DBG_MEM_V(txt) ;
#endif
#define WARN { dbgWrite (_("!!! Warnung !!!")); DBG_ }
#define WARN_S(txt) { dbgWrite (_("!!! Warnung !!!")); DBG_S_(txt) }
#define WARN_V(txt) { dbgWrite (_("!!! Warnung !!!")); DBG_V_(txt) }


// mathematische Helfer

#define MIN(a,b)    (((a) <= (b)) ? (a) : (b))
#define MAX(a,b)    (((a) > (b)) ? (a) : (b))
#define HYP(a,b)    sqrt( (a)*(a) + (b)*(b) )
#define HYP3(a,b,c) sqrt( (a)*(a) + (b)*(b) + (c)*(c) )
#define RUND(a)     ((a) + 0.5)



// ============================================================
// Provisorische Ausnahme-Klasse; von std::exception abstammend:
// ============================================================
class Ausnahme : public std::exception {
public:
    virtual void report () const throw() {}  // oder = 0;
};


class ausn_file_io : public Ausnahme {
public:
    const char *fname;

    ausn_file_io (const char *fn) throw()	{ fname = fn; }
    virtual ~ausn_file_io () throw()		{ DBG_PROG_S ("Destruktor von " << __func__ << endl); }
    virtual const char* what() const throw()	{ return "Datei I/O"; }
    virtual void report () const throw() {
      DBG_PROG_S ("Ausnahme-Report:\n");
      DBG_PROG_S ("\tDatei \""<< fname <<"\" war nicht zu öffnen\n");//testweise
    };
};


#endif //ICC_UTILS_H
