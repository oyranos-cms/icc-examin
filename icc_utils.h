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


#define _(text) text

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
 *
 *   [1,2,...,9]    diese und alle kleineren Kategorien
 *   10+[1,2,...,9] einzig diese Kategorie , z.B. ICCEXAMIN_DEBUG=13 w�hlt alle
 *                                                Speicherinformationen aus
 */


#define DBG_UHR_ (double)clock()/(double)CLOCKS_PER_SEC

#define DBG_T_     cout << __FILE__<<":"<<__LINE__ <<" "<< __func__ << "() " << DBG_UHR_ << " ";
#define LEVEL      { for (int i = 0; i < level_PROG; i++) cout << " "; }
#define DBG_       { LEVEL cout << "        "; DBG_T_ cout << endl; }
#define DBG_S_(txt){ LEVEL cout << "        "; DBG_T_ cout << txt << endl; }
#define DBG_V_(txt){ LEVEL cout << "        "; DBG_T_ cout << #txt << " " << txt << endl;}
#define DBG        DBG_
#define DBG_START  {level_PROG++; for (int i = 0; i < level_PROG; i++) cout << "+"; cout << " Start: "; DBG_T_ cout << endl; }
#define DBG_ENDE   { for (int i = 0; i < level_PROG; i++) cout << "+"; cout << " Ende:  "; DBG_T_ level_PROG--; cout << endl; }
#define DBG_S(txt) DBG_S_(txt)
#define DBG_V(txt) DBG_V_(txt)

#if DEBUG
#define DBG_BED(n) if (icc_debug >= n && icc_debug < 10 || icc_debug == 1##n)
#define DBG_NUM        DBG_BED(1) DBG
#define DBG_NUM_S(txt) DBG_BED(1) DBG_S(txt)
#define DBG_NUM_V(txt) DBG_BED(1) DBG_V(txt)
#else
#define DBG_NUM
#define DBG_NUM_S(txt)
#define DBG_NUM_V(txt)
#endif
#if DEBUG
#define DBG_PROG        DBG_BED(2) DBG
#define DBG_PROG_START  DBG_BED(2) DBG_START
#define DBG_PROG_ENDE   DBG_BED(2) DBG_ENDE
#define DBG_PROG_S(txt) DBG_BED(2) DBG_S(txt)
#define DBG_PROG_V(txt) DBG_BED(2) DBG_V(txt)
#else
#define DBG_PROG
#define DBG_PROG_START
#define DBG_PROG_ENDE
#define DBG_PROG_S(txt)
#define DBG_PROG_V(txt)
#endif
#if DEBUG
#define DBG_MEM        DBG_BED(3) DBG
#define DBG_MEM_START  DBG_BED(3) DBG_START
#define DBG_MEM_ENDE   DBG_BED(3) DBG_ENDE
#define DBG_MEM_S(txt) DBG_BED(3) DBG_S(txt)
#define DBG_MEM_V(txt) DBG_BED(3) DBG_V(txt)
#else
#define DBG_MEM
#define DBG_MEM_START
#define DBG_MEM_ENDE
#define DBG_MEM_S(txt)
#define DBG_MEM_V(txt)
#endif
#define WARN { cout << _("!!! Warnung !!!"); DBG_ }
#define WARN_S(txt) { cout << _("!!! Warnung !!!"); DBG_S_(txt) }
#define WARN_V(txt) { cout << _("!!! Warnung !!!"); DBG_V_(txt) }


// mathematische Helfer

#define MIN(a,b)    (((a) <= (b)) ? (a) : (b))
#define MAX(a,b)    (((a) > (b)) ? (a) : (b))
#define HYP(a,b)    sqrt( (a)*(a) + (b)*(b) )
#define HYP3(a,b,c) sqrt( (a)*(a) + (b)*(b) + (c)*(c) )
#define RUND(a)     ((a) + 0.5)

class Wert
{
    double _wert;
  public:
    Wert & operator = (const Wert   & wert)
                       {_wert =         wert._wert; return *this; }
    Wert & operator = (const double & wert)
                       {_wert =         wert;       return *this; }
    Wert & operator = (const float  & wert)
                       {_wert = (double)wert;       return *this; }
    Wert & operator = (const int    & wert)
                       {_wert = (double)wert;       return *this; }
    Wert & operator = (const unsigned int    & wert)
                       {_wert = (double)wert;       return *this; }
  public:
    operator double ()  {return _wert; }
    operator float  ()  {return (float)_wert; }
    operator int    ()  {return (int)(_wert + 0.5); }
    operator unsigned int ()  {return (unsigned int)(_wert + 0.5); }
};

class Int {
public:
// TODO: byteswap + Groesse + Vorzeichen
#if BYTE_ORDER == BIG_ENDIAN
//  int             operator=(const int _n)     {return _n; }
  //icUInt32Number  operator=(const int _n)     {return _n; }
#endif
#if BYTE_ORDER == LITTLE_ENDIAN
  int             operator=(const int _n)     {return _n; }
  //icUInt32Number  operator=(const int _n)     {return _n; }
#endif
};


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
    virtual ~ausn_file_io () throw()		{ printf ("Destruktor von %s\n", __func__); }
    virtual const char* what() const throw()	{ return "Datei I/O"; }
    virtual void report () const throw() {
      printf ("Ausnahme-Report:\n");
      printf ("\tDatei \"%s\" war nicht zu �ffnen\n", fname);     // testweise
    };
};


/*void ausn_file_io::report () const throw()
{
    printf ("Ausnahme-Report:\n");
    printf ("\tDatei \"%s\" war nicht zu �ffnen\n", fname);	// testweise
}*/


// ==================
// Die FilePtr-Klasse:
// ==================
/*  Konstruktor wirft ausn_file_io bei Mi�erfolg.
*/
#if 0
class FilePtr {
    FILE* f;
public:
    FilePtr (const char *fname, const char *mode)
      	{
	   if (!(f = fopen (fname, mode))) {
 	      printf ("%s(): Kann \"%s\" nicht �ffnen\n", __func__, fname);
	      throw ausn_file_io (fname);
           }
           printf ("%s(): �ffne \"%s\"\n", __func__, fname);
        }
    FilePtr (FILE* p)			{ f = p; }
    ~FilePtr ()				{ printf ("Schlie�e Datei...\n");
                                          if (f) fclose (f); }
    operator FILE*()			{ return f; }
};
#endif


// ==========================================================
// Beispiel einer doppelten Ressourcenanforderung im C++-Stil:
// ==========================================================
#if 0
void f ()
{
    // Datei �ffnen:
    FilePtr fp (__FILE__, "ro");	// wahrsch. erfolgreich
    //FilePtr fp ("gibs_nich", "ro");	// wirft wahrsch. ausn_file_io

    // Datei jetzt offen

    FILE* p = fp;		// Benutzung von fp wie gew�hnliches FILE*.

    // Speicher anfordern...
    // int* buf = new int [100];
    // Angenommen, geht schief und Ausnahme wird geworfen:
    throw std::bad_alloc();	// Ausnahmen-Mechanismus von C++ ruft Destr.
    				// der bis hierin fertigen Objekte auf:
				// --> ~FilePtr() schlie�t Datei fp

    // Daten aus Datei nach buf lesen...

    // Normalfall:
    // Blockende von fp --> Destruktor --> Datei fp wird geschlossen
}


// ======================================
// Zum Vergleich diese Funktion im C-Stil:
// ======================================
#include <cstdlib>	// malloc()

int f_C ()
{
    FILE* fp;
    int*  buf;

    if (! (fp = fopen ("datei","ro")))
       return -1;			// R�ckgabe-Kode f�r Mi�erfolg

    // Datei jetzt offen

    // Speicher anfordern...
    if (! (buf = (int*) malloc (100))) {
       fclose (fp);			// explizites Dateischlie�en n�tig
       return -1;			// R�ckgabe-Kode f�r Mi�erfolg
    }
    // Daten aus Datei nach buf lesen...

    // Normalfall:
    fclose (fp);			// explizites Dateischlie�en n�tig
    return 0;				// R�ckgabe-Kode f�r Erfolg
}


// ====================
// Fangen von Ausnahmen:
// ====================

int main ()
{
    try {
      f();
    }
    catch (Ausnahme & a) {	// f�ngt alles von Ausnahme Abstammende
        printf ("Ausnahme aufgetreten: %s\n", a.what());
        a.report();
    }
    catch (std::exception & e) { // f�ngt alles von exception Abstammende
        printf ("Std-Ausnahme aufgetreten: %s\n", e.what());
    }
    catch (...) {		// f�ngt alles �briggebliebene
        printf ("Huch, unbekannte Ausnahme\n");
    }
}
#endif

#endif //ICC_UTILS_H
