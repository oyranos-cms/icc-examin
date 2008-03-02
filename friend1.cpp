/*
   friend2.cpp

   Über Freundschaften.

   Aufgabe:
   =======
    Die Klasse ICC besitze ein Element (Objekt) vom Typ Header:
   	class ICC {
	    //...
	    Header header;
        };
    Die Klasse Header besitze eine Funktion write() und eine Funktion show():
        class Header {
	    //...
	    void write ();
	    void show ();
	};
    Intern in ICC soll write() und show() aufrufbar sein, aber ein externer
    Anwender eines ICC-Objektes soll zwar Header::show(), aber nicht
    Header::write() aufrufen können. Also

   	ICC icc;

	icc.header.show();	// soll ok sein
	icc.header.write();	// Compiler soll Fehler melden!


    Es gibt (mindestens) zwei Möglichkeiten, dies zu erreichen:

    1.) Header deklariert show() als public und write() als private; damit
        können Externe zwar show(), aber nicht write() aufrufen. ICC als
	"besonderer" Header-Kunde muß aber auch an private Header-Elemente ran.
        Deshalb erklärt Header die Klasse ICC als 'befreundet'.
        ICC legt nun sein Header-Element als public an, so daß ICC-Kunden
	direkt die öffentlichen Header-Funktionen als
	    icc.header.show()
	aufrufen können.

    2.) ICC versteckt sein Element Header als privat vor jedem ICC-Kunden-
        Zugriff und reicht lediglich die Header-Funktionen, die ICC-Kunden
	nutzen können sollen, explizit nach außen durch.
	Header, sodurch vor ICC-Kunden versteckt, kann nun alles, worauf ICC
	Zugriff braucht, public deklarieren. Das erspart ausdrückliche
	Freundschaftsbekundungen in Richtung ICC.
	
    Diese Datei hier führt Variante 1 aus, die Datei "friend2.cpp" die zweite.

    Variante 1 hat den vielleicht Makel, daß ein ICC-Kunde explizit
    eine ICC-Interna, ein ICC-Element namens 'header' in die Hand nehmen muß
    zum Aufruf
    	icc.header.show().
    Möglw. sollten ICC-Kunden syntakisch strikt auf der ersten Ebene bleiben
    können a'la
        icc.show_header().
    Aber das mag eine Geschmacksfrage sein.
*/

#include <cstdio>

#define HEAD_WAS 	printf ("Header::%s():\n",__func__);
#define ICC_WAS		printf ("ICC::%s():\n",__func__);



class ICC;		// Klasse, die Header als Freund nennen will/soll;
			// muß deshalb vor Header *erwähnt* werden;


class Header {		// Hier soll ICC auf Privates zugreifen können

    friend class ICC;	// Header erklärt: ICC ist mein Freund!

private:
    int privat_data;			// privates Datum von Header
    void write ()	{ HEAD_WAS }	// private Methode von Header

public:
    void show ()	{ HEAD_WAS }	// öffentl. Methode von Header
};


class ICC {
public:
    Header header;

    void write ()
       { ICC_WAS
       header.write(); 		// ICC ruft private Header-Methode auf
       header.privat_data = 1;  // ICC beschreibt privates Header-Element
       }			// Dürfte normalerweise niemand außer Header
};


void f ()
{
    ICC icc;			// 'ich' als f() bin ein Anwender von ICC

    icc.write ();		// ok, 'ich' als Nutzer von icc rufe öffentl.
    				// icc-Funktion auf

    icc.header.show ();		// ok, Aufruf einer öffentlichen Funktion
    				// eines öffentlichen icc-Elementes

//  icc.header.write ();	// Fehler: Header::write() ist für 'mich',
    				// den Header nicht als friend erklärt hat,
				// privat.

//  icc.header.privat_data = 1; // Fehler, Header::privat_data ist für 'mich'
    				// privat.
}

int main ()
{
    f();
}

