/*
   friend3.cpp

   siehe friend1.cpp

*/

#include <cstdio>

#define HEAD_WAS 	printf ("Header::%s():\n",__func__);
#define ICC_WAS		printf ("ICC::%s():\n",__func__);



class Header {
private:
    int privat_data;			// privates Datum von Header

public:
    void write ()	{ HEAD_WAS }	// öffentl. Methode von Header
    void show ()	{ HEAD_WAS }	// öffentl. Methode von Header
};


class ICC {
    Header header;			// ICC deklariert header als
    					// *privates* Element.
					// Damit ist es für ICC-*Anwender*
					// unzugänglich
public:

    void write ()		// z.B schreibt ICC-Profil
       { ICC_WAS
       header.write(); 		// ok, ICC ruft öffentl. Header-Methode auf
       // ...
//     header.privat_data = 1;  // Fehler: ICC kann als gewöhnl. Anwender von
       }			// Header auf private Header-Elemente nicht
				// zugreifen
    void show_header ()
       { ICC_WAS
       header.show();		// ICC reicht Header-Funktion an ICC-Anwender
       }			// durch, veröffentlicht diese
};


void f ()
{
    ICC icc;			// 'ich' als f() bin ein Anwender von ICC

    icc.write();		// ok, 'ich' als Nutzer von icc rufe öffentl.
    				// icc-Funktion auf

    icc.show_header();		// ok, öffentl. Funktion von icc

//  icc.header.show();		// Fehler: header ist privates icc-Element,
    				// für 'mich' als f() nicht zugänglich
//  icc.header.write();		// dito
//  icc.header.privat_data = 1; // dito

}

int main ()
{
    f();
}

