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



#include "icc_vrml_parser.h"

void
ICCexamin::oeffnen (std::vector<std::string> dateinamen)
{ DBG_PROG_START
  if(!dateinamen.size()) {
    DBG_PROG_ENDE
    return;
  }

  // Laden
  frei_ = false;
  icc_betrachter->DD_histogram->punkte_clear();
  bool weiter = profile.oeffnen(dateinamen);
  if (weiter)
  {
    DBG_PROG
    histogramModus(profile.aktuell());

      // Oberflächenpflege
    tag_browserText ();
    if(icc_betrachter->DD_histogram->visible() &&
       !icc_betrachter->inspekt_html->visible() )
      icc_betrachter->DD_histogram->flush();

    icc_betrachter->measurement( profile.profil()->hasMeasurement() );
    if(histogramModus())
    {
        // Oberflächenpflege
      icc_betrachter->DD_histogram->show();
      icc_waehler_->show();
      icc_betrachter->menueintrag_3D->set();
      histogram_angezeigt_ = true;

      profile.oeffnen(icc_oyranos.moni(),-1);
      profile.oeffnen(icc_oyranos.cmyk(),-1);
    }
  }


      // Sortieren
    if( dateinamen.size() &&
        (dateinamen[0].find( "wrl",  dateinamen[0].find_last_of(".") )
         != std::string::npos) )
    {
      size_t size;
      char *data = ladeDatei (dateinamen[0], &size);
      std::string d (data,size);
      //DBG_NUM_V( d <<" "<< size )
      if(data) free( data ); // übernimmt std::string einfach den Speicherblock?
      std::vector<ICCnetz> netze = extrahiereNetzAusVRML (d);
      if( netze.size() )
      { DBG_NUM_V( netze.size() )
        for(unsigned int n = 0; n< netze.size(); ++n)
        {
          DBG_NUM_V( netze[n].punkte.size() )
          for(unsigned int i = 0; i < 10; ++i) {
           cout << netze[n].punkte[i].koord[0] << " ";
           cout << netze[n].punkte[i].koord[1] << " ";
           cout << netze[n].punkte[i].koord[2] << "  ";
           cout << netze[n].punkte[i].farbe[0] << " ";
           cout << netze[n].punkte[i].farbe[1] << " ";
           cout << netze[n].punkte[i].farbe[2] << " ";
           cout << netze[n].punkte[i].farbe[3] << endl;
          }
          DBG_NUM_V( netze[n].indexe.size()/4.0 )
        }
        for(unsigned int i = 0; i < netze.size(); ++i ) {
          netze[i].transparenz = 0.6;
          netze[i].grau = false;
          netze[i].aktiv = true;
        }
        icc_betrachter->DD_histogram->hineinNetze(netze);
        std::vector<std::string> texte;
        texte.push_back(_("CIE *L"));
        texte.push_back(_("CIE *a"));
        texte.push_back(_("CIE *b"));
        icc_betrachter->DD_histogram->achsNamen(texte);
        icc_betrachter->DD_histogram->punkte_clear();
        icc_betrachter->DD_histogram->auffrischen();
      } else
        WARN_S(_("kein Netz gefunden in VRML Datei"))
    } else {
      DBG_PROG
      //histogram();
      //icc_betrachter->DD_histogram->auffrischen();
    }

    // ICCwaehler
      // erneuern?
    static std::vector<std::string> namen_neu, namen_alt;
    bool namensgleich = false;
    namen_neu = profile;
    if(namen_alt.size() == namen_neu.size()) {
      namensgleich = true;
      DBG_NUM_S( _("Anzahl gabs schon mal") )
      for(int i = 0; i < (int)namen_neu.size(); ++i) {
        if(namen_neu[i] != namen_alt[i]) {
          namensgleich = false;
          namen_alt[i] = namen_neu[i];
        }
      }
    }
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
        const char* name = profile.name(i).c_str();
        if( i >= (int)icc_betrachter->DD_histogram->dreiecks_netze.size() ) {
          WARN_S( _("Gebe irritiert auf. Kein Netz gefunden. Ist Argyll installiert?") )
          break;
        }
        transparenz = icc_betrachter->DD_histogram->dreiecks_netze[i].transparenz;
        DBG_PROG_V( transparenz )
        grau = icc_betrachter->DD_histogram->dreiecks_netze[i].grau;
        icc_waehler_->push_back(name, transparenz, grau , aktiv[i]);
      }
    }

  frei_ = true;
  DBG_PROG_ENDE
}

void
ICCexamin::oeffnen ()
{ DBG_PROG_START

  std::vector<std::string> profilnamen = icc_betrachter->open( profile );
  oeffnen( profilnamen );

  DBG_PROG_ENDE
}

void
ICCexamin::tag_browserText (void)
{
  DBG_PROG_START
  //open and preparing the first selected item

  TagBrowser *b = icc_betrachter->tag_browser;

  std::stringstream s;
  std::string text;
  std::vector<std::string> tag_list = profile.profil()->printTags();
  DBG_PROG_V( tag_list.size() <<" "<< (int*) b )

  #define add_s(stream) s << stream; b->add (s.str().c_str()); s.str("");
  #define add_          s << " ";

  b->clear();
  add_s ("@fDateiname:")
  add_s ("@b    " << profile.profil()->filename() )
  add_s ("")
  if (tag_list.size() == 0) {
    add_s (_("keine Inhalte gefunden für") <<" \"" << profile.profil()->filename() << "\"")
    return;
  } else if ((int)tag_list.size() != profile.profil()->tagCount()*5 ) {
    add_s (_("interner Fehler") )
  }
  add_s ("@B26@tNr. Bezeichner  Typ         GröBeschreibung")
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
    s << *it; for (int i = (*it++).size(); i < 12; i++) {s << " ";} ++anzahl;
    // Typ
    s << *it; for (int i = (*it++).size(); i < 12; i++) {s << " ";} ++anzahl;
    // Größe
    for (int i = (*it).size(); i < 5; i++) {s << " ";} s << *it++; s << " "; ++anzahl;
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

  std::string::size_type pos=0;
  std::string data = profile.profil()->filename(); DBG_NUM_S( data )
  if((pos = data.find_last_of ("/", pos)) != std::string::npos) {
    data.erase (0, pos);
  }

  s.clear(); s << "ICC Details: " << data;
  status ((const char*) s.str().c_str() );
  DBG_PROG_ENDE
}

bool
ICCexamin::berichtSpeichern (void)
{ DBG_PROG_START
  frei_ = false;
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

  dateiwahl->filter(_("HTML Dokumente (*.htm*)")); DBG_PROG
  #ifdef HAVE_FLU
  dateiwahl->cd(".");
  #endif
  dateiwahl->label(_("Bericht Speichern")); DBG_PROG
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

  frei_ = true;
  DBG_PROG_ENDE
  return erfolgreich;
}


