/*
 * ICC Examin ist eine ICC Profil Betrachter
 * 
 * Copyright (C) 2004  Kai-Uwe Behrmann 
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


#define cout std::cout
#define endl std::endl

extern int level_PROG;
extern int icc_debug;

#define DBG_UHR_ (double)clock()/(double)CLOCKS_PER_SEC

#define DBG_T_     cout << __FILE__<<":"<<__LINE__ <<" "<< __func__ << "() " << DBG_UHR_ << " ";
#define LEVEL      { for (int i = 0; i < level_PROG; i++) cout << " "; }
#define DBG        if (icc_debug) { LEVEL cout << "        "; DBG_T_ cout << endl; }
#define DBG_S(txt) if (icc_debug) { LEVEL cout << "        "; DBG_T_ cout << txt << endl; }
#define DBG_V(txt) if (icc_debug) { LEVEL cout << "        "; DBG_T_ cout << #txt << " " << txt << endl;}

#if 1
#define DBG_MEM DBG
#define DBG_MEM_S(txt) DBG_S(txt)
#define DBG_MEM_V(txt) DBG_V(txt)
#else
#define DBG_MEM
#define DBG_MEM_S(txt)
#define DBG_MEM_V(txt)
#endif
#if 1
#define DBG_NUM DBG
#define DBG_NUM_S(txt) DBG_S(txt)
#define DBG_NUM_V(txt) DBG_V(txt)
#else
#define DBG_NUM
#define DBG_NUM_S(txt)
#define DBG_NUM_V(txt)
#endif
#if 1
#define DBG_PROG DBG
#define DBG_PROG_START if (icc_debug) {level_PROG++; for (int i = 0; i < level_PROG; i++) cout << "+"; cout << " Start: "; DBG_T_ cout << endl; }
#define DBG_PROG_ENDE if (icc_debug) { for (int i = 0; i < level_PROG; i++) cout << "+"; cout << " Ende:  "; DBG_T_ level_PROG--; cout << endl; }
#define DBG_PROG_S(txt) DBG_S(txt)
#define DBG_PROG_V(txt) DBG_V(txt)
#else
#define DBG_PROG
#define DBG_PROG_START
#define DBG_PROG_ENDE
#define DBG_PROG_S(txt)
#define DBG_PROG_V(txt)
#endif
#define WARN DBG
#define WARN_S(txt) DBG_S(txt)
#define WARN_V(txt) DBG_V(txt)


// mathematische Helfer

#define MIN(a,b)    (((a) <= (b)) ? (a) : (b))
#define MAX(a,b)    (((a) > (b)) ? (a) : (b))
#define HYP(a,b)    pow((a)*(a) + (b)*(b),1.0/2.0)
#define HYP3(a,b,c) pow( (a)*(a) + (b)*(b) + (c)*(c) , 1.0/2.0)
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
      printf ("\tDatei \"%s\" war nicht zu öffnen\n", fname);     // testweise
    };
};


/*void ausn_file_io::report () const throw()
{
    printf ("Ausnahme-Report:\n");
    printf ("\tDatei \"%s\" war nicht zu öffnen\n", fname);	// testweise
}*/


// ==================
// Die FilePtr-Klasse:
// ==================
/*  Konstruktor wirft ausn_file_io bei Mißerfolg.
*/
#if 0
class FilePtr {
    FILE* f;
public:
    FilePtr (const char *fname, const char *mode)
      	{
	   if (!(f = fopen (fname, mode))) {
 	      printf ("%s(): Kann \"%s\" nicht öffnen\n", __func__, fname);
	      throw ausn_file_io (fname);
           }
           printf ("%s(): Öffne \"%s\"\n", __func__, fname);
        }
    FilePtr (FILE* p)			{ f = p; }
    ~FilePtr ()				{ printf ("Schließe Datei...\n");
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
    // Datei öffnen:
    FilePtr fp (__FILE__, "ro");	// wahrsch. erfolgreich
    //FilePtr fp ("gibs_nich", "ro");	// wirft wahrsch. ausn_file_io

    // Datei jetzt offen

    FILE* p = fp;		// Benutzung von fp wie gewöhnliches FILE*.

    // Speicher anfordern...
    // int* buf = new int [100];
    // Angenommen, geht schief und Ausnahme wird geworfen:
    throw std::bad_alloc();	// Ausnahmen-Mechanismus von C++ ruft Destr.
    				// der bis hierin fertigen Objekte auf:
				// --> ~FilePtr() schließt Datei fp

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
       return -1;			// Rückgabe-Kode für Mißerfolg

    // Datei jetzt offen

    // Speicher anfordern...
    if (! (buf = (int*) malloc (100))) {
       fclose (fp);			// explizites Dateischließen nötig
       return -1;			// Rückgabe-Kode für Mißerfolg
    }
    // Daten aus Datei nach buf lesen...

    // Normalfall:
    fclose (fp);			// explizites Dateischließen nötig
    return 0;				// Rückgabe-Kode für Erfolg
}


// ====================
// Fangen von Ausnahmen:
// ====================

int main ()
{
    try {
      f();
    }
    catch (Ausnahme & a) {	// fängt alles von Ausnahme Abstammende
        printf ("Ausnahme aufgetreten: %s\n", a.what());
        a.report();
    }
    catch (std::exception & e) { // fängt alles von exception Abstammende
        printf ("Std-Ausnahme aufgetreten: %s\n", e.what());
    }
    catch (...) {		// fängt alles Übriggebliebene
        printf ("Huch, unbekannte Ausnahme\n");
    }
}
#endif

#endif //ICC_UTILS_H
