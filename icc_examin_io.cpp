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
 * Die zentrale Klasse.
 * 
 */


#include "icc_examin.h"
#include "icc_betrachter.h"
#include "icc_waehler.h"

using namespace icc_examin_ns;

#if USE_THREADS
#include "threads.h"
#else
// TODO: beseitige Hack
static int frei_tuen = 0;
#define frei_ frei_tuen
#endif

//#define DEBUG_EXAMIN
#ifdef DEBUG_EXAMIN
#define DBG_EXAMIN_START DBG_PROG_START
#define DBG_EXAMIN_ENDE DBG_PROG_ENDE
#define DBG_EXAMIN_V( texte ) DBG_NUM_V( texte )
#define DBG_EXAMIN_S( texte ) DBG_NUM_S( texte )
#else
#define DBG_EXAMIN_START
#define DBG_EXAMIN_ENDE
#define DBG_EXAMIN_V( texte )
#define DBG_EXAMIN_S( texte )
#endif

const char* dateiName(const char* name);
const char* dateiName(std::string name);


#include "icc_vrml_parser.h"

void
ICCexamin::oeffnenThread_ ()
{ DBG_PROG_START
  if(wandelThreadId(pthread_self()) != THREAD_LADEN) WARN_S("THREAD_LADEN???");

  if(!speicher_vect_.size()) {
    fortschritt( 1.1 );
    DBG_PROG_ENDE
    return;
  }

# if 0
  for (float f = 0.; f < 1.0 ; f += 0.1) {
    fortschrittThreaded( f );
    icc_examin_ns::sleep(.5);
  }
# endif

  // Laden
  icc_betrachter->DD_farbraum->punkte_clear();
  profile.clear();
  fortschrittThreaded( -.1 );
  for (unsigned int i = 0; i < speicher_vect_.size(); ++i)
  {
    DBG_PROG_V( speicher_vect_[i].size()<<" "<<speicher_vect_[i].name() )
    fortschrittThreaded( 1./3.+ (double)(i)/speicher_vect_.size()/3.0 );
    profile.einfuegen( speicher_vect_[i], -1 );
    fortschrittThreaded( 1./3.+ (double)(i+1)/speicher_vect_.size()/3.0 );
    DBG_THREAD
  }

  std::vector<std::string> dateinamen = profile;

  if (profile.size())
  {
      // Oberflaechenpflege
    icc_examin_ns::lock(__FILE__,__LINE__);
    erneuerTagBrowserText_ ();
    if(icc_betrachter->DD_farbraum->visible() &&
       !icc_betrachter->inspekt_html->visible() )
      icc_betrachter->DD_farbraum->damage(FL_DAMAGE_ALL);
    icc_betrachter->menueintrag_gamut_speichern->activate();
    icc_betrachter->menueintrag_gamut_vrml_speichern->activate();

#   if 0
    icc_examin_ns::unlock(this, __FILE__,__LINE__);
    fortschrittThreaded( 2./3.+ 1./3. );
    icc_examin_ns::sleep(0.04);
    icc_examin_ns::lock(__FILE__,__LINE__);
#   else
    fortschritt( 2./3.+ 1./3. );
#   endif
    icc_betrachter->measurement( profile.profil()->hasMeasurement() );
    if(farbraumModus())
    {
        // Oberflaechenpflege
      // siehe ICCexamin::farbraum(int n)
      //gamutAnsichtZeigen();
    }
    icc_examin_ns::unlock(this, __FILE__,__LINE__);

      // Sortieren
    if( dateinamen.size() &&
        (dateinamen[0].find( "wrl",  dateinamen[0].find_last_of(".") )
         != std::string::npos) )
    {
      std::string d (speicher_vect_[0], speicher_vect_[0].size());
      //DBG_NUM_V( d <<" "<< size )
      std::vector<ICCnetz> netze = extrahiereNetzAusVRML (d);
      if( netze.size() )
      { DBG_NUM_V( netze.size() )
        for(unsigned int i = 0; i < netze.size(); ++i ) {
          netze[i].transparenz = 0.6;
          netze[i].grau = false;
          netze[i].aktiv = true;
        }
        icc_examin_ns::lock(__FILE__,__LINE__);
        icc_betrachter->DD_farbraum->hineinNetze(netze);
        std::vector<std::string> texte;
        texte.push_back(_("CIE *L"));
        texte.push_back(_("CIE *a"));
        texte.push_back(_("CIE *b"));
        icc_betrachter->DD_farbraum->achsNamen(texte);
        icc_betrachter->DD_farbraum->punkte_clear();
        icc_betrachter->DD_farbraum->auffrischen();
        gamutAnsichtZeigen();
        icc_examin_ns::unlock(this, __FILE__,__LINE__);
      } else
        WARN_S(_("kein Netz gefunden in VRML Datei"))
    } else {
      DBG_PROG
      //farbraum();
      //icc_betrachter->DD_farbraum->auffrischen();
    }

    fortschrittThreaded( 2./3.+ 2./3. );
    // ICCwaehler
      // erneuern
    static std::vector<std::string> namen_neu, namen_alt;
    bool namensgleich = false;
    namen_neu = profile;
    DBG_PROG_V( namen_neu.size() <<" "<< namen_alt.size() )
    if(namen_alt.size() == namen_neu.size()) {
      namensgleich = true;
      DBG_NUM_S( _("Anzahl gabs schon mal") )
      for(int i = 0; i < (int)namen_neu.size(); ++i)
        if(namen_neu[i] != namen_alt[i])
          namensgleich = false;
    }
    namen_alt = namen_neu;
    DBG_NUM_V( "#################### " << namensgleich << " ##############")
    if(!namensgleich)
    {
      icc_waehler_->clear();
      int anzahl = profile.size();
      DBG_PROG_V( anzahl )
      double transparenz;
      bool grau;
      std::vector<int> aktiv = profile.aktiv();
      DBG_PROG_V( aktiv.size() )
      for(int i = 0; i < anzahl; ++i) {
        DBG_PROG_V( i )

        if( i >= (int)icc_betrachter->DD_farbraum->dreiecks_netze.size() ) {
          WARN_S( _("Kein Netz gefunden. Ist Argyll installiert?") )
          break;
        }

        transparenz= icc_betrachter->DD_farbraum->dreiecks_netze[i].transparenz;
        DBG_PROG_V( transparenz )
        grau = icc_betrachter->DD_farbraum->dreiecks_netze[i].grau;
        icc_waehler_->push_back(dateiName(profile.name(i)),
                                transparenz, grau , aktiv[i]);
      }
    }

    // Fenstername setzen
    {
      detaillabel = "ICC Examin: ";
      detaillabel.insert( detaillabel.size(), dateiName(dateinamen[0]) );
      icc_betrachter->details->label(detaillabel.c_str());
      DBG_PROG_V( dateinamen[0] <<" "<< detaillabel )
    }

  }
  DBG_PROG_ENDE
}

const char*
dateiName(const char* name)
{
  const char* dateiname=0;
        // Datainame extahieren
        if(name)
          dateiname = strrchr(name,'/');

        if(!dateiname)
          dateiname = name;
        else
          ++dateiname;
  return dateiname;
}

const char*
dateiName(std::string name)
{
  return dateiName(name.c_str());
}

void
ICCexamin::lade (std::vector<Speicher> & neu)
{
  if(kannLaden())
  {
    speicher_vect_ = neu;
    lade_ = true;
  } else {
    DBG_THREAD_S( "muss warten" )
  }
}

// Dieses Spalten in Threads fuer eine fluessige Oberflaeche
#if USE_THREADS
void*
#else
void
#endif
ICCexamin::oeffnenStatisch_ (void* ie)
{
  // TODO
  //icc_examin_ns::sleep(0.1);
  DBG_PROG_START

  // Laufzeitfehler erkennen
  {
    static int erster = true;
    if(!erster)
      WARN_S("Programmierfehler: " <<__func__<<" thread darf\n" <<
             "nur einmal gestartet werden.")
    erster = false;
  }

  // Haupt Thread freigeben
  //icc_examin_ns::unlock(ie, __FILE__,__LINE__);

  // Bezug herstellen
  ICCexamin* examin = (ICCexamin*) ie;

  // Schleife starten die diesen thread laufen lässt
  while(1) {
    if(examin->lade_) {
      examin->oeffnenThread_();
      examin->lade_ = false;
    } else
      // kurze Pause 
      icc_examin_ns::sleep(0.01);
  }

  DBG_PROG_ENDE
# if USE_THREADS
  return 0;
# endif
}

void
ICCexamin::oeffnen (std::vector<std::string> dateinamen)
{ DBG_PROG_START
# if 1
  if(!dateinamen.size()) {
    WARN_S("keine Dateinamen angegeben")
    fortschritt( 1.1 );
    DBG_PROG_ENDE
    return;
  }
# endif

  fortschritt( 0.0 );

  // Laden
  std::vector<Speicher> ss;
  // resize benutzt copy, und erzeugt damit Referenzen auf das
  // selbe Objekt ; wir benoetigen aber neue Objekte
  // ss.resize(dateinamen.size());
  for (unsigned int i = 0; i < dateinamen.size(); ++i)
  {
    ss.push_back(Speicher());

    ss[i] = dateiNachSpeicher(dateinamen[i]);
    if(ss[i].size() == 0) {
      status( _("Stop loading ") << dateinamen[i] )
      fortschritt( 1.1 );
      return;
    }
  }
  DBG_PROG

# if 0
  if(!kannLaden())
  {
    int erfolg = false;
    while(!erfolg) {
      if(kannLaden()) {
        erfolg = true;
      } else {
        // kurze Pause 
        DBG_THREAD_S( "muss warten" )
        icc_examin_ns::sleep(0.05);
      }
    }
  }
# endif
  lade (ss);

  DBG_PROG_ENDE
}

void
ICCexamin::oeffnen ()
{ DBG_PROG_START
  fortschritt(0.01);
  std::vector<std::string> profilnamen = icc_betrachter->open( profile );
  oeffnen( profilnamen );
  neu_laden_ = true;
  DBG_PROG_ENDE
}

void
ICCexamin::erneuerTagBrowserText_ (void)
{
  DBG_PROG_START
  //open and preparing the first selected item

  TagBrowser *b = icc_betrachter->tag_browser;

  std::stringstream s;
  std::string text;
  std::vector<std::string> tag_list = profile.profil()->printTags();
  DBG_PROG_V( tag_list.size() <<" "<< (int*) b )

# define add_s(stream) s << stream; b->add (s.str().c_str()); s.str("");
# define add_          s << " ";

  b->clear();
  add_s ("@f" << _("Filename") << ":" )
  add_s ("@b    " << profile.profil()->filename() )
  add_s ("")
  if (tag_list.size() == 0) {
    add_s (_("found no content for") <<" \"" << profile.profil()->filename() << "\"")
    return;
  } else if ((int)tag_list.size() != profile.profil()->tagCount()*5 ) {
    add_s (_("Internal error") )
  }
  add_s ("@B26@t" << _("No. Tag   Type   Size Description") )
  add_s ("@t" << profile.profil()->printHeader() )
  std::vector<std::string>::iterator it;
  for(int i = 0; i < (int)tag_list.size(); ++i)
    ;//DBG_PROG_V( i <<" "<< tag_list[i] )
  int anzahl = 0;
  for (it = tag_list.begin() ; it != tag_list.end(); ++it) {
    s << "@t";
    // Nummer
    int Nr = atoi((*it).c_str()) + 1;
    std::stringstream t; t << Nr;
    for (int i = t.str().size(); i < 3; i++) {s << " ";} s << Nr; *it++; ++anzahl; s << " ";
    // Name/Bezeichnung
    s << *it; for (int i = (*it++).size(); i < 6; i++) {s << " ";} ++anzahl;
    // Typ
    s << *it; for (int i = (*it++).size(); i < 5; i++) {s << " ";} ++anzahl;
    // Größe
    for (int i = (*it).size(); i < 6; i++) {s << " ";} s << *it++; s << " "; ++anzahl;
    // Beschreibung
    add_s (*it)
  }
  DBG_PROG_V( anzahl )
  if (b->value())
    b->selectItem (b->value()); // Anzeigen
  else
    b->selectItem (1);

  if (profile.profil()->hasTagName (b->selectedTagName)) {
    int item = profile.profil()->getTagByName (b->selectedTagName) + 6;
    b->selectItem (item);
    b->value(item);
  }

  status ( dateiName( profile.profil()->filename() ) << " " << _("loaded")  )
  DBG_PROG_ENDE
}

bool
ICCexamin::berichtSpeichern (void)
{ DBG_PROG_START
  bool erfolgreich = true;
  std::string dateiname = profile.name();  DBG_PROG_V( dateiname )

  // Profilnamen ersetzen
  std::string::size_type pos=0;
  if ((pos = dateiname.find_last_of(".", dateiname.size())) != std::string::npos)
  { DBG_PROG
    dateiname.replace (pos, 5, ".html"); DBG_NUM_S( ".html gesetzt" )
  } DBG_PROG_V( dateiname )

  // FLTK Dateidialog aufrufen
  DBG_PROG_V( dateiwahl->filter() )

  std::string muster = dateiwahl->filter(); DBG_PROG
  std::string datei;
  if (dateiwahl->value())
    datei = dateiwahl->value(); DBG_PROG
  std::string titel = dateiwahl->label(); DBG_PROG

  dateiwahl->filter(_("HTML Documents (*.htm*)")); DBG_PROG
# ifdef HAVE_FLU
  dateiwahl->cd(".");
# endif
  dateiwahl->label(_("Save Report")); DBG_PROG
  dateiwahl->value(dateiname.c_str()); DBG_PROG

  dateiwahl->show(); DBG_PROG
  while( dateiwahl->shown() )
    Fl::wait( 0.01 );

  DBG_PROG_V( dateiwahl->filter() )
  if (dateiwahl->value())
    dateiname = dateiwahl->value();
  else
    dateiname = "";
  DBG_PROG

  dateiwahl->filter(muster.c_str()); DBG_PROG
  dateiwahl->value(datei.c_str()); DBG_PROG
  dateiwahl->label(titel.c_str()); DBG_PROG
  DBG_PROG_V( dateiwahl->filter() )

  DBG_PROG_V( dateiname )

  if (dateiname == "" ||
      dateiname == profile.name())
  { DBG_PROG_V( dateiwahl->count() << dateiname )
    fortschritt (1.1);
    DBG_PROG_ENDE
    return false;
  }

  // Bericht erzeugen
  bool export_html = true;
  std::string bericht = profile.profil()->report(export_html);
  // Speichern
  std::ofstream f ( dateiname.c_str(),  std::ios::out );
  f.write ( bericht.c_str(), bericht.size() );
  f.close();

  DBG_PROG_ENDE
  return erfolgreich;
}

bool
ICCexamin::gamutSpeichern (IccGamutFormat format)
{
  DBG_PROG_START
  bool erfolgreich = true;
  std::string dateiname = profile.name();  DBG_PROG_V( dateiname )

  // Profilnamen ersetzen
  std::string::size_type pos=0;
  if ((pos = dateiname.find_last_of(".", dateiname.size())) != std::string::npos)
  { DBG_PROG
    if(format == ICC_ABSTRACT) {
      dateiname.replace (pos, 5, "_proof.icc"); DBG_NUM_S( "_proof.icc gesetzt")
    } else if(format == ICC_VRML) {
      dateiname.replace (pos, 5, ".wrl"); DBG_NUM_S( ".wrl gesetzt")
    }
  } DBG_PROG_V( dateiname )

  // FLTK Dateidialog aufrufen
  DBG_PROG_V( dateiwahl->filter() )

  std::string muster = dateiwahl->filter(); DBG_PROG
  std::string datei;
  if (dateiwahl->value())
    datei = dateiwahl->value(); DBG_PROG
  std::string titel = dateiwahl->label(); DBG_PROG

# ifdef HAVE_FLU
  dateiwahl->cd(".");
# endif
  if(format == ICC_ABSTRACT) {
    dateiwahl->filter(_("ICC colour profiles (*.ic*)")); DBG_PROG
    dateiwahl->label(_("Save Gamut as Profile")); DBG_PROG
  } else if(format == ICC_VRML) {
    dateiwahl->filter(_("VRML Files (*.wrl)")); DBG_PROG
    dateiwahl->label(_("Save Gamut as VRML")); DBG_PROG
  }
  dateiwahl->value(dateiname.c_str()); DBG_PROG

  dateiwahl->show(); DBG_PROG
  while( dateiwahl->shown() )
    Fl::wait( 0.01 );

  DBG_PROG_V( dateiwahl->filter() )
  if (dateiwahl->value())
    dateiname = dateiwahl->value();
  else
    dateiname = "";
  DBG_PROG

  dateiwahl->filter(muster.c_str()); DBG_PROG
  dateiwahl->value(datei.c_str()); DBG_PROG
  dateiwahl->label(titel.c_str()); DBG_PROG
  DBG_PROG_V( dateiwahl->filter() )

  DBG_PROG_V( dateiname )

  if (dateiname == "" ||
      dateiname == profile.name())
  { DBG_PROG_V( dateiwahl->count() << dateiname )
    fortschritt (1.1);
    DBG_PROG_ENDE
    return false;
  }

  Speicher profil;
  size_t groesse = 0;
  char* daten = 0;
  daten = profile.profil()->saveProfileToMem( &groesse );
  profil.lade( daten, groesse );
  if(format == ICC_ABSTRACT) {
    // Gamutprofil erzeugen
    Speicher speicher;
    icc_oyranos.gamutCheckAbstract( profil, speicher,
                                    profile.profil()->intent(),
                                    /*cmsFLAGS_GAMUTCHECK |*/ cmsFLAGS_SOFTPROOFING );

    // Speichern
    saveMemToFile ( dateiname.c_str(), (const char*)speicher, speicher.size() );
    speicher.clear(); DBG
  } else if(format == ICC_VRML) {
    std::string vrml;
    vrml = icc_oyranos.vrmlVonProfil ( *profile.profil(), profile.profil()->intent() );
    // Speichern
    saveMemToFile ( dateiname.c_str(), vrml.c_str(), vrml.size() );
  }
  profil.clear(); DBG

  DBG_PROG_ENDE
  return erfolgreich;
}

