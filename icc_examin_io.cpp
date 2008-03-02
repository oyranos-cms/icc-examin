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
ICCexamin::oeffnen (std::vector<Speicher> profil_vect)
{ DBG_PROG_START
  if(!profil_vect.size()) {
    DBG_PROG_ENDE
    return;
  }

  // Laden
  frei_ = false;
  icc_betrachter->DD_farbraum->punkte_clear();
  profile.clear();
  for (int i = 0; i < (int)profil_vect.size(); ++i)
    profile.oeffnen( profil_vect[i], 1 );

  if (profile.size())
  {
      // Oberflaechenpflege
    tag_browserText ();
    if(icc_betrachter->DD_farbraum->visible() &&
       !icc_betrachter->inspekt_html->visible() )
      icc_betrachter->DD_farbraum->flush();
    icc_betrachter->menueintrag_gamut_speichern->activate();

    icc_betrachter->measurement( profile.profil()->hasMeasurement() );
    if(farbraumModus())
    {
        // Oberflaechenpflege
      icc_betrachter->menueintrag_3D->set();
      icc_betrachter->menueintrag_huelle->set();
      icc_betrachter->widget_oben = ICCfltkBetrachter::WID_3D;
      farbraum_angezeigt_ = true;
      neuzeichnen(icc_betrachter->DD_farbraum);
      DBG_PROG_S("neuzeichnen DD_farbraum")

      profile.oeffnen(icc_oyranos.moni(),-1);
      profile.oeffnen(icc_oyranos.cmyk(),-1);
    }
  }

    // ICCwaehler
      // erneuern?
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
    DBG_NUM_V( "####### " << namensgleich << " ########")
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
        // Datainame extahieren
        std::string dateiname;
        if( profile.name(i).find_last_of("/") != std::string::npos)
          dateiname = profile.name(i).substr( profile.name(i).find_last_of("/")+1, profile.name(i).size() );
        else
          dateiname = profile.name(i);

        if( i >= (int)icc_betrachter->DD_farbraum->dreiecks_netze.size() ) {
          WARN_S( _("Gebe irritiert auf. Kein Netz gefunden. Ist Argyll installiert?") )
          break;
        }
        transparenz = icc_betrachter->DD_farbraum->dreiecks_netze[i].transparenz;
        DBG_PROG_V( transparenz )
        grau = icc_betrachter->DD_farbraum->dreiecks_netze[i].grau;
        icc_waehler_->push_back(dateiname.c_str(), transparenz, grau , aktiv[i]);
      }
    }

  // Fenstername setzen
  {
    detaillabel = "ICC Examin: ";
    std::string dateiname = profile.name(profile.aktuell());
    if( dateiname.find_last_of("/") != std::string::npos)
      detaillabel.insert( detaillabel.size(), dateiname.substr( dateiname.find_last_of("/")+1, dateiname.size()) );
    else
      detaillabel.insert( detaillabel.size(), dateiname );
    icc_betrachter->details->label(detaillabel.c_str());
  }

  frei_ = true;
  DBG_PROG_ENDE
}

void
ICCexamin::oeffnen (std::vector<std::string> dateinamen)
{ DBG_PROG_START
  if(!dateinamen.size()) {
    DBG_PROG_ENDE
    return;
  }

  // Laden
  frei_ = false;
  icc_betrachter->DD_farbraum->punkte_clear();
  bool weiter = profile.oeffnen(dateinamen);
  if (weiter)
  {
      // Oberflaechenpflege
    tag_browserText ();
    if(icc_betrachter->DD_farbraum->visible() &&
       !icc_betrachter->inspekt_html->visible() )
      icc_betrachter->DD_farbraum->flush();
    icc_betrachter->menueintrag_gamut_speichern->activate();

    icc_betrachter->measurement( profile.profil()->hasMeasurement() );
    if(farbraumModus())
    {
        // Oberflaechenpflege
      icc_betrachter->menueintrag_3D->set();
      icc_betrachter->menueintrag_huelle->set();
      icc_betrachter->widget_oben = ICCfltkBetrachter::WID_3D;
      farbraum_angezeigt_ = true;
      neuzeichnen(icc_betrachter->DD_farbraum);
      DBG_PROG_S("neuzeichnen DD_farbraum")

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
        icc_betrachter->DD_farbraum->hineinNetze(netze);
        std::vector<std::string> texte;
        texte.push_back(_("CIE *L"));
        texte.push_back(_("CIE *a"));
        texte.push_back(_("CIE *b"));
        icc_betrachter->DD_farbraum->achsNamen(texte);
        icc_betrachter->DD_farbraum->punkte_clear();
        icc_betrachter->DD_farbraum->auffrischen();
      } else
        WARN_S(_("kein Netz gefunden in VRML Datei"))
    } else {
      DBG_PROG
      //farbraum();
      //icc_betrachter->DD_farbraum->auffrischen();
    }

    // ICCwaehler
      // erneuern?
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
        // Datainame extahieren
        std::string dateiname;
        if( profile.name(i).find_last_of("/") != std::string::npos)
          dateiname = profile.name(i).substr( profile.name(i).find_last_of("/")+1, profile.name(i).size() );
        else
          dateiname = profile.name(i);

        if( i >= (int)icc_betrachter->DD_farbraum->dreiecks_netze.size() ) {
          WARN_S( _("Gebe irritiert auf. Kein Netz gefunden. Ist Argyll installiert?") )
          break;
        }
        transparenz = icc_betrachter->DD_farbraum->dreiecks_netze[i].transparenz;
        DBG_PROG_V( transparenz )
        grau = icc_betrachter->DD_farbraum->dreiecks_netze[i].grau;
        icc_waehler_->push_back(dateiname.c_str(), transparenz, grau , aktiv[i]);
      }
    }

  // Fenstername setzen
  if( dateinamen.size() ) {
    detaillabel = "ICC Examin: ";
    if( dateinamen[0].find_last_of("/") != std::string::npos)
      detaillabel.insert( detaillabel.size(), dateinamen[0].substr( dateinamen[0].find_last_of("/")+1,dateinamen[0].size()) );
    else
      detaillabel.insert( detaillabel.size(), dateinamen[0] );
    icc_betrachter->details->label(detaillabel.c_str());
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

  dateiwahl->filter(_("HTML Documents (*.htm*)")); DBG_PROG
  #ifdef HAVE_FLU
  dateiwahl->cd(".");
  #endif
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

  frei_ = true;
  DBG_PROG_ENDE
  return erfolgreich;
}

bool
ICCexamin::gamutSpeichern (void)
{
  DBG_PROG_START
  frei_ = false;
  bool erfolgreich = true;
  std::string dateiname = profile.name();  DBG_PROG_V( dateiname )

  // Profilnamen ersetzen
  std::string::size_type pos=0;
  if ((pos = dateiname.find_last_of(".", dateiname.size())) != std::string::npos)
  { DBG_PROG
    dateiname.replace (pos, 5, "_proof.icc"); DBG_NUM_S( "_proof.icc gesetzt" )
  } DBG_PROG_V( dateiname )

  // FLTK Dateidialog aufrufen
  DBG_PROG_V( dateiwahl->filter() )

  std::string muster = dateiwahl->filter(); DBG_PROG
  std::string datei;
  if (dateiwahl->value())
    datei = dateiwahl->value(); DBG_PROG
  std::string titel = dateiwahl->label(); DBG_PROG

  dateiwahl->filter(_("ICC colour profiles (*.ic*)")); DBG_PROG
  #ifdef HAVE_FLU
  dateiwahl->cd(".");
  #endif
  dateiwahl->label(_("Save Gamut as Profile")); DBG_PROG
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
    frei_ = true;
    DBG_PROG_ENDE
    return false;
  }

  // Gamutprofil erzeugen
  Speicher profil;
  size_t groesse = 0;
  char* daten = profile.profil()->saveProfileToMem( &groesse );
  //char* block = (char*) calloc (sizeof (char), groesse );
  //memcpy( daten, daten, groesse);
  profil.lade( daten, groesse );
  Speicher abstract;
  icc_oyranos.gamutCheckAbstract( profil, abstract,
                                  profile.profil()->intent(),
                                  /*cmsFLAGS_GAMUTCHECK |*/ cmsFLAGS_SOFTPROOFING );

  // Speichern
  std::ofstream f ( dateiname.c_str(),  std::ios::out );
  f.write ( (const char*)abstract, abstract.size() );
  f.close();
  profil.clear(); DBG
  abstract.clear(); DBG

  frei_ = true;
  DBG_PROG_ENDE
  return erfolgreich;
}

