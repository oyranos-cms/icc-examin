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


#include "icc_kette.h"
#include "icc_examin.h"
#include "icc_betrachter.h"
#include "icc_fenster.h"
#include "icc_draw.h"
#include "icc_gl.h"
#include "agviewer.h"
#include "icc_helfer_ui.h"
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


ICCexamin::ICCexamin ()
{ DBG_PROG_START
  icc_betrachter = new ICCfltkBetrachter [1];
  icc_waehler_ = new  ICCwaehler(485, 186, _("Ansichtsw\212hler"));
  if(!icc_waehler_) WARN_S( _("icc_waehler_ nicht reservierbar") )
  icc_waehler_->hide();
  _item = -1;
  _mft_item = -1;
  _zeig_histogram = 0;
  statlabel = "";
  status_ = false;
  DBG_PROG_ENDE
}

void
ICCexamin::quit ()
{ DBG_PROG_START
  delete icc_betrachter;
  agviewers.resize(0);
  profile.clear();
  DBG_PROG_ENDE
  exit(0);
}

int
tastatur(int e)
{ //DBG_PROG_START
  int gefunden = 0;
  if( e == FL_SHORTCUT )
  {
      if(Fl::event_key() == FL_Escape) {
        gefunden = 1;
        DBG_NUM_S("FL_Escape")
      } else
      if(Fl::event_key() == 'q'
       && Fl::event_state() == FL_CTRL) {
        DBG_NUM_S("FL_CTRL+Q")
        icc_examin->quit();
        gefunden = 1;
      }
  }
  icc_examin->icc_betrachter->DD_histogram->tastatur(e);
  //DBG_PROG_ENDE
  return gefunden;
}

void
ICCexamin::start (int argc, char** argv)
{ DBG_PROG_START

  kurven.resize(MAX_VIEWER);
  punkte.resize(MAX_VIEWER);
  texte.resize(MAX_VIEWER);
  kurve_umkehren.resize(MAX_VIEWER);

  icc_betrachter->init();

  // Die TagViewers registrieren und ihre Variablen initialisieren
  icc_betrachter->tag_viewer->id = TAG_VIEWER;
  icc_betrachter->mft_viewer->id = MFT_VIEWER;
  icc_betrachter->vcgt_viewer->id = VCGT_VIEWER;

  DBG_PROG

  #if HAVE_X// || HAVE_OSX
  icc_betrachter->menueintrag_vcgt->show();
  #endif
  if(!icc_debug)
    icc_betrachter->menueintrag_testkurven->hide();

  status(_(""));
  DBG_PROG

      if (argc>1) {
        statlabel = argv[1];
        statlabel.append (" ");
        statlabel.append (_("geladen"));
        status(statlabel.c_str());
        std::vector<std::string>profilnamen;
        profilnamen.resize(argc-1);
        for (int i = 1; i < argc; i++) {
          DBG_PROG_V( i ) profilnamen[i-1] = argv[i];
        }
        oeffnen (profilnamen);
      } else {
        status(_("Bereit"));
      }

  Fl::add_handler(tastatur);

  modellDazu( /*ICCkette*/&profile ); // wird in nachricht ausgewertet

  // zur Benutzung freigeben
  status_ = 1;
  frei_ = true;

  icc_betrachter->run();

  DBG_PROG_ENDE
}

#include "icc_vrml_parser.h"

void
ICCexamin::oeffnen (std::vector<std::string> dateinamen)
{ DBG_PROG_START
  if(!dateinamen.size())
    return;
  // Laden
  frei_ = false;
  icc_betrachter->DD_histogram->punkte_clear();
  bool weiter = profile.oeffnen(dateinamen);
  if (weiter) { DBG_PROG
    icc_betrachter->tag_browser->reopen ();
    icc_betrachter->measurement( profile.profil()->hasMeasurement() );
    if(profile.profil()->hasTagName("ncl2"))
    {
      icc_betrachter->DD_histogram->zeigen();
      icc_betrachter->menueintrag_3D->set();
      _zeig_histogram = true;
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
        for(unsigned int i = 0; i < netze.size(); ++i )
          netze[i].transparenz = 0.6;
        icc_betrachter->DD_histogram->hineinNetze(netze);
        std::vector<std::string> texte;
        texte.push_back(_("CIE *L"));
        texte.push_back(_("CIE *a"));
        texte.push_back(_("CIE *b"));
        icc_betrachter->DD_histogram->achsNamen(texte);
        icc_betrachter->DD_histogram->punkte_clear();
        if(icc_betrachter->DD_histogram->beruehrt())
          icc_betrachter->DD_histogram->auffrischen();
      } else
        WARN_S(_("kein Netz gefunden in VRML Datei"))
    } else {
      if (icc_betrachter->DD_histogram->beruehrt())
      { DBG_PROG
        histogram();
        icc_betrachter->DD_histogram->auffrischen();
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

std::string
ICCexamin::waehleTag (int item)
{ DBG_PROG_START
  frei_ = false;

  DBG_PROG_V( item )

  std::string text = _("Leer");

  if(!profile.size()) {
    DBG_PROG_ENDE
    return text;
  }

  kurven[TAG_VIEWER].clear();
  punkte[TAG_VIEWER].clear();
  texte[TAG_VIEWER].clear();

  std::vector<std::string> rgb_tags;
  rgb_tags.push_back("rXYZ");
  rgb_tags.push_back("gXYZ");
  rgb_tags.push_back("bXYZ");

  if (item < 0) {
    icc_betrachter->tag_browser->select(5);
    text = profile.profil()->printLongHeader(); DBG_PROG
    icc_betrachter->tag_text->hinein(text);
    neuzeichnen(icc_betrachter->tag_text);

    text = "";
  } else if (item >= 0) {
    std::vector<std::string> TagInfo = profile.profil()->printTagInfo(item);
    //if(_item == item)
      //return TagInfo[0];

    kurve_umkehren[TAG_VIEWER] = false;

    _item = item;

    DBG_PROG_S( TagInfo.size() << " " << TagInfo[0] << " " << TagInfo[1] )

    if        ( TagInfo[1] == "text"
             || TagInfo[1] == "cprt?"
             || TagInfo[1] == "meas"
             || TagInfo[1] == "sig"
             || TagInfo[1] == "dtim") {
      icc_betrachter->tag_text->hinein ( (profile.profil()->getTagText (item))[0] ); DBG_PROG
      neuzeichnen(icc_betrachter->tag_text);
    } else if ( TagInfo[1] == "desc" ) {
      icc_betrachter->tag_text->hinein( (profile.profil()->getTagDescription (item))[0] ); DBG_PROG
      neuzeichnen(icc_betrachter->tag_text);
    } else if ( TagInfo[0] == "rXYZ" || TagInfo[0] == "gXYZ" || TagInfo[0] == "bXYZ" ) {
      std::string TagName;
      std::vector<double> punkt;
      for (unsigned int i_name = 0; i_name < rgb_tags.size(); i_name++) {
        if (profile.profil()->hasTagName (rgb_tags[i_name])) {
          punkt = profile.profil()->getTagCIEXYZ (profile.profil()->getTagByName(rgb_tags[i_name]));
          for (unsigned int i = 0; i < 3; i++)
            punkte[TAG_VIEWER].push_back (punkt[i]);
          TagInfo = profile.profil()->printTagInfo (profile.profil()->getTagByName(rgb_tags[i_name]));
          for (unsigned int i = 0; i < 2; i++)
            texte[TAG_VIEWER].push_back (TagInfo[i]);
        }
      }
      icc_betrachter->tag_viewer->hineinPunkt( punkte[TAG_VIEWER], texte[TAG_VIEWER] );
      neuzeichnen(icc_betrachter->tag_viewer);
    } else if ( TagInfo[1] == "curv"
             || TagInfo[1] == "bfd" ) {
      std::vector<double> kurve;
      std::string TagName;
      for (int i_name = 0; i_name < profile.profil()->tagCount(); i_name++) {
        if ( (profile.profil()->printTagInfo(i_name))[1] == "curv"
          || (profile.profil()->printTagInfo(i_name))[1] == "bfd" ) {
          kurve = profile.profil()->getTagCurve (i_name);
          kurven[TAG_VIEWER].push_back (kurve);
          TagInfo = profile.profil()->printTagInfo (i_name);
          //for (unsigned int i = 0; i < 2; i++)
          texte[TAG_VIEWER].push_back (TagInfo[0]);
        }
      }
      texte[TAG_VIEWER].push_back ("curv");
      icc_betrachter->tag_viewer->hineinKurven( kurven[TAG_VIEWER], texte[TAG_VIEWER] );
      neuzeichnen(icc_betrachter->tag_viewer);
    } else if ( TagInfo[1] == "chrm" ) {
      punkte[TAG_VIEWER] = profile.profil()->getTagCIEXYZ(item);
      texte [TAG_VIEWER] = profile.profil()->getTagText(item);
      icc_betrachter->tag_viewer->hineinPunkt( punkte[TAG_VIEWER], texte[TAG_VIEWER] );
      neuzeichnen(icc_betrachter->tag_viewer);
    } else if ( TagInfo[1] == "XYZ" ) {
      punkte[TAG_VIEWER] = profile.profil()->getTagCIEXYZ(item);
      texte[TAG_VIEWER] = TagInfo;
      icc_betrachter->tag_viewer->hineinPunkt( punkte[TAG_VIEWER], texte[TAG_VIEWER] );
      neuzeichnen(icc_betrachter->tag_viewer);
    } else if ( TagInfo[1] == "mft2"
             || TagInfo[1] == "mft1" ) { DBG_PROG_S("mft1/2")
      std::string t = profile.profil()->getTagText (item)[0];
      icc_betrachter->mft_choice->profilTag (item, t);
      waehleMft (_mft_item);
    } else if ( TagInfo[1] == "vcgt" ) { DBG_PROG_S("vcgt")
      kurve_umkehren[TAG_VIEWER] = true;
      kurven[TAG_VIEWER] = profile.profil()->getTagCurves (item, ICCtag::CURVE_IN);
      texte[TAG_VIEWER] = profile.profil()->getTagText (item);
      icc_betrachter->tag_viewer->hineinKurven ( kurven[TAG_VIEWER], texte[TAG_VIEWER] );
      icc_betrachter->tag_viewer->kurve_umkehren = true;
      neuzeichnen(icc_betrachter->tag_viewer);
      DBG_PROG_S( "vcgt" )

    /*} else if ( TagInfo[1] == "chad" ) {
      std::vector<int> zahlen = profile.profil()->getTagNumbers (tag_nummer, ICCtag::MATRIX);
      cout << zahlen.size() << endl; DBG_PROG
      assert (9 == zahlen.size());
      s << endl <<
      "  " << zahlen[0] << ", " << zahlen[1] << ", " << zahlen[2] << ", " << endl <<
      "  " << zahlen[3] << ", " << zahlen[4] << ", " << zahlen[5] << ", " << endl <<
      "  " << zahlen[6] << ", " << zahlen[7] << ", " << zahlen[8] << ", " << endl;
      icc_betrachter->tag_text->hinein ( s.str() ); DBG_PROG
*/
    } else {
      icc_betrachter->tag_text->hinein ( (profile.profil()->getTagText (item))[0] ); DBG_PROG
      neuzeichnen(icc_betrachter->tag_text);
    }

    text = TagInfo[0];
  }

  frei_ = true;
  DBG_PROG_ENDE
  return text;
}


#include "icc_gl.h"
void
ICCexamin::waehleMft (int item)
{ DBG_PROG_START
  frei_ = false;
  //Auswahl aus mft_choice

  kurven[MFT_VIEWER].clear();
  punkte[MFT_VIEWER].clear();
  texte[MFT_VIEWER].clear();

  if (item < 1)
    _mft_item = 0;
  else
    _mft_item = item;

  kurve_umkehren[MFT_VIEWER] = false;

  status("")

  std::stringstream s;
  std::vector<double> zahlen;

  DBG_PROG_V( _mft_item )
  switch (_mft_item) {
  case 0: // Überblick
    { std::vector<std::string> Info = icc_betrachter->mft_choice->Info;
      //profile.profil()->getTagText (icc_betrachter->tag_nummer)[0];
      for (unsigned int i = 1; i < Info.size(); i++) // erste Zeile weglassen
        s << Info [i] << endl;
      icc_betrachter->mft_text->hinein ( s.str() ); DBG_PROG_S("Text anzeigen")
      neuzeichnen(icc_betrachter->mft_text);
    } break;
  case 1: // Matrix
    zahlen = profile.profil()->getTagNumbers (icc_betrachter->tag_nummer, ICCtag::MATRIX);
    DBG_PROG_S("Zahlen in mft_text anzeigen")
    assert (9 == zahlen.size());
    s << endl <<
    "  " << zahlen[0] << ", " << zahlen[1] << ", " << zahlen[2] << ", " << endl <<
    "  " << zahlen[3] << ", " << zahlen[4] << ", " << zahlen[5] << ", " << endl <<
    "  " << zahlen[6] << ", " << zahlen[7] << ", " << zahlen[8] << ", " << endl;
    icc_betrachter->mft_text->hinein ( s.str() ); DBG_PROG
    neuzeichnen(icc_betrachter->mft_text);
    break;
  case 2: // Eingangskurven
    DBG_PROG_S("Kurven in anzeigen")
    kurven[MFT_VIEWER] =  profile.profil()->getTagCurves (icc_betrachter->tag_nummer, ICCtag::CURVE_IN);
    texte[MFT_VIEWER] = profile.profil()->getTagChannelNames (icc_betrachter->tag_nummer, ICCtag::CURVE_IN);
    icc_betrachter->mft_viewer->hineinKurven ( kurven[MFT_VIEWER], texte[MFT_VIEWER] );
    neuzeichnen(icc_betrachter->mft_viewer);
    DBG_PROG
    break;
  case 3: // 3D Tabelle
    DBG_PROG_S("Tabelle in anzeigen")
    icc_betrachter->mft_gl->hineinTabelle (
                     profile.profil()->getTagTable (icc_betrachter->tag_nummer, ICCtag::TABLE),
                     profile.profil()->getTagChannelNames (icc_betrachter->tag_nummer, ICCtag::TABLE_IN),
                     profile.profil()->getTagChannelNames (icc_betrachter->tag_nummer, ICCtag::TABLE_OUT) ); DBG_PROG
    neuzeichnen(icc_betrachter->mft_gl);
    break;
  case 4: // Ausgangskurven
    DBG_PROG_S("Kurven in anzeigen")
    kurven[MFT_VIEWER] = profile.profil()->getTagCurves (icc_betrachter->tag_nummer, ICCtag::CURVE_OUT);
    texte[MFT_VIEWER] = profile.profil()->getTagChannelNames (icc_betrachter->tag_nummer, ICCtag::CURVE_OUT);
    icc_betrachter->mft_viewer->hineinKurven ( kurven[MFT_VIEWER], texte[MFT_VIEWER] );
    neuzeichnen(icc_betrachter->mft_viewer);
    DBG_PROG
    break;
  }

  icc_betrachter->mft_choice->gewaehlter_eintrag = item;
  frei_ = true;
  DBG_PROG_ENDE
}

void
ICCexamin::zeigPrueftabelle ()
{ DBG_PROG_START
  neuzeichnen(icc_betrachter->inspekt_html);
  DBG_PROG_ENDE
}

void
ICCexamin::zeigCGATS()
{
  DBG_PROG_START
  // CGATS in Fenster präsentieren
  icc_examin_ns::nachricht(profile.profil()->cgats_max());
  DBG_PROG_ENDE
}

// virtual aus icc_examin_ns::Beobachter::
void
ICCexamin::nachricht( Modell* modell , int infos )
{
  DBG_PROG_START
  frei_ = false;
  DBG_PROG_V( infos )
  // Modell identifizieren
  ICCkette* k = dynamic_cast<ICCkette*>(modell);
  if(k)
  {
    DBG_PROG_S( _("Nachricht von ICCkette") )
    if(infos>=0 && !(*k)[infos]->changing())
    { DBG_PROG
      histogram (infos);
    }
    DBG_PROG_S( _("Auffrischen von Profil Nr.: ") << infos )

    // ICCwaehler
    icc_waehler_->clear();
    int anzahl = k->size();
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
      DBG_PROG
      grau = icc_betrachter->DD_histogram->dreiecks_netze[i].grau;
      icc_waehler_->push_back(name, transparenz, grau , aktiv[i]);
    }
  }
  Beobachter::nachricht(modell, infos);
  frei_ = true;
  DBG_PROG_ENDE
}

void
ICCexamin::messwertLese (int n,
                         std::vector<double> & p,
                         std::vector<float>  & f,
                         std::vector<std::string> & namen)
{
  DBG_PROG_START
  if(profile.size() > n &&
     profile.aktuell() == n &&
     profile[n]->hasMeasurement() &&
     profile[n]->getMeasurement().hasXYZ() )
    { DBG_NUM_S( "nutze Messdaten" )
      ICCmeasurement messung = profile[n]->getMeasurement();

      if(messung.valid() && profile[n]->size())
        icc_betrachter->DD_histogram->zeig_punkte_als_messwert_paare = true;
      else
        icc_betrachter->DD_histogram->zeig_punkte_als_messwert_paare = false;
      DBG_NUM_V( icc_betrachter->DD_histogram->zeig_punkte_als_messwert_paare )
      icc_betrachter->DD_histogram->zeig_punkte_als_messwerte = true;
      DBG_NUM_V( icc_betrachter->DD_histogram->zeig_punkte_als_messwerte )

      unsigned int j;
      int n = messung.getPatchCount(); DBG_PROG_V( messung.getPatchCount() )
      for (j = 0; j < (unsigned) n; ++j)
      { // zuerst die Messwerte ...
        std::vector<double> daten = messung.getMessLab(j);
        for (unsigned i = 0; i < daten.size(); ++i)
          p.push_back(daten[i]);
        // ... dann die über das Profil errechneten Lab Werte
        if (icc_betrachter->DD_histogram->zeig_punkte_als_messwert_paare) {
          daten = messung.getCmmLab(j);
          for (unsigned i = 0; i < daten.size(); ++i)
            p.push_back(daten[i]);
        } 

        daten = messung.getMessRGB(j);
        for (unsigned i = 0; i < daten.size(); ++i) {
          f.push_back((float)daten[i]);
        }
        f.push_back(1.0);
        if (icc_betrachter->DD_histogram->zeig_punkte_als_messwert_paare)
        { daten = messung.getCmmRGB(j);
          for (unsigned i = 0; i < daten.size(); ++i)
            f.push_back(daten[i]);
          f.push_back(1.0);
        } 
      }
      namen = messung.getFeldNamen();
    }

  DBG_PROG_ENDE
}

void
ICCexamin::netzLese (int n,
                     std::vector<ICCnetz> & netz)
{
  DBG_PROG_START
  std::vector<ICCnetz> netz_temp;
  Speicher s;
  if(profile.size() > n)
    if(profile[n]->valid())
      s.lade(profile[n]->saveProfileToMem(0),
             profile[n]->getProfileSize());

  if(s.size())
  {
    DBG_PROG
    netz_temp = icc_oyranos. netzVonProfil( s );
    if(netz_temp.size())
    {
      if(n+1 > (int)netz.size())
        netz.resize( n+1 );
      netz[n] = netz_temp[0];
      netz[n].name = profile[n]->filename();
      DBG_NUM_V( netz[n].transparenz )
    }
  }

  DBG_PROG_ENDE
}

void
ICCexamin::farbenLese (int n,
                       std::vector<double> & p,
                       std::vector<float>  & f)
{
  DBG_PROG_START
  // benannte Farben darstellen
  if( profile.size() > n &&
      profile[n]->getTagByName("ncl2") >= 0 )
  {
    DBG_PROG
    p = profile[n]->getTagNumbers (profile[n]->getTagByName("ncl2"),
                                         ICCtag::MATRIX);
    DBG_NUM_V( p[0] )
    f.resize( (int)p[0] * 4);
    DBG_NUM_V( f.size() )
    for(unsigned i = 0; i < f.size(); ++i)
      f[i] = 1.0;
    p.erase( p.begin() );
    icc_betrachter->DD_histogram->zeig_punkte_als_messwert_paare = false;
    icc_betrachter->DD_histogram->zeig_punkte_als_messwerte = false;
  }

  DBG_PROG_ENDE
}

void
ICCexamin::histogram (int n)
{
  DBG_PROG_START
  frei_ = false;
  std::vector<std::string> texte, namen;

  texte.push_back(_("CIE *L"));
  texte.push_back(_("CIE *a"));
  texte.push_back(_("CIE *b"));

  std::vector<double> p;
  std::vector<float>  f;
  DBG_PROG_V( n <<" "<< profile.size() )
  DBG_PROG_V( profile[n]->filename() )

  if(profile.size() > n &&
     profile.aktuell() == n &&
     profile[n]->hasMeasurement() &&
     profile[n]->getMeasurement().hasXYZ() )
    {
      DBG_PROG
      messwertLese(n, p,f,namen);
    }

  // benannte Farben darstellen
  if( profile.size() > n &&
      profile[n]->getTagByName("ncl2") >= 0 )
  {
    DBG_PROG
    farbenLese(n, p,f);
  }

  if(p.size())
    icc_betrachter->DD_histogram->hineinPunkte( p, f, namen, texte );

  std::vector<ICCnetz> netz;
  if( profile.size() > n )
    netzLese(n, netz);

  DBG_PROG
  if(netz.size())
  {
    if(icc_betrachter->DD_histogram->dreiecks_netze.size() < netz.size())
    {
      icc_betrachter->DD_histogram-> dreiecks_netze .resize( netz.size() );
      netz[n].transparenz = 0.3;
      netz[n].grau = true;
    } else {
      netz[n].transparenz = icc_betrachter->DD_histogram->dreiecks_netze[n].
                            transparenz;
      netz[n].grau = false;
    }

    icc_betrachter->DD_histogram->dreiecks_netze[n] = netz[n];
    icc_betrachter->DD_histogram->achsNamen( texte );
    icc_betrachter->DD_histogram->draw();
  }


  frei_ = true;
  DBG_PROG_ENDE
}

void
ICCexamin::histogram ()
{
  DBG_PROG_START
  frei_ = false;

  for(int i = 0; i < profile.size(); ++i)
    histogram(i);

  if(icc_betrachter->DD_histogram -> dreiecks_netze.size())
    icc_betrachter->DD_histogram ->
      dreiecks_netze [icc_betrachter->DD_histogram->dreiecks_netze.size()-1]
        . transparenz = 0.7;

  DBG_PROG_V( profile.size() )

  frei_ = true;
  DBG_PROG_ENDE
}

void
ICCexamin::testZeigen ()
{ DBG_PROG_START

  #if HAVE_X || HAVE_OSX
  std::vector<std::vector<std::pair<double,double> > > kurven2;
  kurven2.resize(8);
  kurven2[0].resize(4);
  kurven2[1].resize(3);
  kurven2[2].resize(56);
  kurven2[3].resize(56);
  kurven2[4].resize(56);
  kurven2[5].resize(56);
  kurven2[6].resize(56);
  kurven2[7].resize(56);
  for(unsigned int i = 0; i < kurven2.size(); ++i)
    for(unsigned int j = 0; j < kurven2[i].size(); ++j) {
      kurven2[i][j].first = sin(i) * 3.2 - 0.5* (cos(j*2)+0.1);
      kurven2[i][j].second = i * -0.2 + 0.05 * (sin(j/10.0)+2.7);
    }
  std::vector<std::string> txt;
  txt.resize(8);
  txt[0] = "ein Bild";
  txt[1] = "Gemälde";
  txt[2] = "fast HDR";
  txt[3] = "2 fast HDR";
  txt[4] = "3 fast HDR";
  txt[5] = "4 fast HDR";
  txt[6] = "5 fast HDR";
  txt[7] = "6 fast HDR";
  icc_betrachter->vcgt_viewer->hide();
  icc_betrachter->vcgt_viewer->show();
  icc_betrachter->vcgt_viewer->hineinDaten ( kurven2, txt );
  icc_betrachter->vcgt_viewer->kurve_umkehren = true;
  #endif

  // TODO: osX
  DBG_PROG_ENDE
}

void
ICCexamin::vcgtZeigen ()
{ DBG_PROG_START
  frei_ = false;
  kurve_umkehren[VCGT_VIEWER] = true;

  #if HAVE_X// || HAVE_OSX
  std::string display_name = "";
  kurven[VCGT_VIEWER] = leseGrafikKartenGamma (display_name,texte[VCGT_VIEWER]);
  if (kurven[VCGT_VIEWER].size()) {
    icc_betrachter->vcgt_viewer->hide();
    icc_betrachter->vcgt_viewer->show();
    icc_betrachter->vcgt_viewer->hineinKurven( kurven[VCGT_VIEWER],
                                               texte [VCGT_VIEWER] );
    icc_betrachter->vcgt_viewer->kurve_umkehren = true;
  } else {
    WARN_S(_("Keine Kurve gefunden")) //TODO kleines Fenster
    icc_betrachter->vcgt_viewer->hide();
  }
  #endif

  frei_ = true;
  // TODO: osX
  DBG_PROG_ENDE
}

void
ICCexamin::moniSetzen ()
{ DBG_PROG_START
  frei_ = false;
  if( profile.size() ) { DBG_PROG
    icc_oyranos.setzeMonitorProfil( profile.profil()->filename() );
    vcgtZeigen();
  }
  frei_ = true;
  DBG_PROG_ENDE
}

void
ICCexamin::standardGamma ()
{ DBG_PROG_START
  frei_ = false;

  #if HAVE_X
  system("xgamma -gamma 1.0");
  vcgtZeigen();
  #endif

  // TODO: osX
  frei_ = true;
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

void
ICCexamin::neuzeichnen (void* z)
{ DBG_PROG_START
  Fl_Widget *wid = (Fl_Widget*)z;
  static int item;

  DBG_PROG_V( dynamic_cast<Fl_Widget*>(icc_betrachter->DD_histogram)->visible())
  DBG_PROG_V( dynamic_cast<Fl_Widget*>(icc_betrachter->inspekt_html)->visible())
  DBG_PROG_V( dynamic_cast<Fl_Widget*>(icc_betrachter->tag_browser)->visible() )
  DBG_PROG_V( dynamic_cast<Fl_Widget*>(icc_betrachter->tag_text)->visible() )
  DBG_PROG_V( dynamic_cast<Fl_Widget*>(icc_betrachter->tag_viewer)->visible() )
  DBG_PROG_V( dynamic_cast<Fl_Widget*>(icc_betrachter->mft_choice)->visible() )
  DBG_PROG_V( dynamic_cast<Fl_Widget*>(icc_betrachter->mft_text)->visible() )
  DBG_PROG_V( dynamic_cast<Fl_Widget*>(icc_betrachter->mft_gl)->visible() )
  DBG_PROG_V( dynamic_cast<Fl_Widget*>(icc_betrachter->mft_viewer)->visible() )

  DBG_PROG_V(_zeig_histogram << icc_betrachter->menueintrag_3D->value() )
  if (icc_betrachter->menueintrag_3D->value() && !_zeig_histogram)
  { DBG_PROG_S( "3D Histogramm zeigen" )
    if(!icc_betrachter->DD_histogram->visible())
      icc_betrachter->DD_histogram->show();

    icc_betrachter->DD_histogram->zeigen();
    icc_waehler_->show();
    _zeig_histogram = true;
  } else if (_zeig_histogram) { DBG_PROG_S( "3D hist ausschalten" )
    icc_betrachter->DD_histogram->verstecken();
    _zeig_histogram = false;
    icc_waehler_->iconize();

    if(!icc_betrachter->menueintrag_inspekt->value()) {
      waehleTag(_item);
      return;
    }
  } 

  if (icc_betrachter->menueintrag_inspekt->value() &&
      !icc_betrachter->inspekt_html->visible())
  { DBG_PROG
    icc_betrachter->inspekt_html->show();
  } else if (!icc_betrachter->menueintrag_inspekt->value() &&
             icc_betrachter->inspekt_html->visible())
  { DBG_PROG
    icc_betrachter->inspekt_html->hide();
    waehleTag(_item);
    return;
  }
DBG_PROG
  if(icc_betrachter->menueintrag_inspekt->value() ||
     icc_betrachter->menueintrag_3D->value()) {
    icc_betrachter->tag_browser->hide(); DBG_PROG
  } else {
    icc_betrachter->tag_browser->show(); DBG_PROG
  }
DBG_PROG
  if (wid == icc_betrachter->tag_viewer ||
      wid == icc_betrachter->mft_viewer) {
    wid->clear_visible(); DBG_PROG_V( item << _item )
  }
DBG_PROG
  if (wid == icc_betrachter->mft_text ||
      wid == icc_betrachter->mft_gl ||
      wid == icc_betrachter->mft_viewer)
  { icc_betrachter->mft_choice->show(); DBG_PROG_S( "mft_choice zeigen" ) 
  } else
    icc_betrachter->mft_choice->hide();
DBG_PROG
  if (wid != icc_betrachter->mft_gl)
    icc_betrachter->mft_gl->verstecken();
  else
    icc_betrachter->mft_gl->zeigen();
DBG_PROG
  #define zeig(widget) \
  { Fl_Widget *w = dynamic_cast<Fl_Widget*> (icc_betrachter->widget); \
    if (w != wid && w->visible()) { DBG_PROG_S( #widget << " verstecken" ) \
      w->hide(); \
    } else if(w == wid) { DBG_PROG_S( #widget << " zeigen" ) \
      w->show(); \
      item = _item; \
    } \
  }

  zeig(mft_viewer)
  zeig(mft_text)
  zeig(tag_viewer)
  zeig(tag_text)

  DBG_PROG_V( dynamic_cast<Fl_Widget*>(icc_betrachter->DD_histogram)->visible())
  DBG_PROG_V( dynamic_cast<Fl_Widget*>(icc_betrachter->inspekt_html)->visible())
  DBG_PROG_V( dynamic_cast<Fl_Widget*>(icc_betrachter->tag_browser)->visible() )
  DBG_PROG_V( dynamic_cast<Fl_Widget*>(icc_betrachter->tag_text)->visible() )
  DBG_PROG_V( dynamic_cast<Fl_Widget*>(icc_betrachter->tag_viewer)->visible() )
  DBG_PROG_V( dynamic_cast<Fl_Widget*>(icc_betrachter->mft_choice)->visible() )
  DBG_PROG_V( dynamic_cast<Fl_Widget*>(icc_betrachter->mft_text)->visible() )
  DBG_PROG_V( dynamic_cast<Fl_Widget*>(icc_betrachter->mft_gl)->visible() )
  DBG_PROG_V( dynamic_cast<Fl_Widget*>(icc_betrachter->mft_viewer)->visible() )

  DBG_PROG_ENDE
}

void
ICCexamin::fortschritt(double f)
{ DBG_PROG_START

  if(0.0 < f && f <= 1.0)
    icc_betrachter->load_progress-> value(f);
  else if (1.0 < f)
    icc_betrachter->load_progress-> hide();
  else
    icc_betrachter->load_progress-> show();

  DBG_PROG_ENDE
}

void
ICCexamin::statusAktualisieren()
{ DBG_PROG_START
  icc_betrachter->box_stat->label(statlabel.c_str());
  DBG_PROG_ENDE
}

void
ICCexamin::initReihenfolgeGL_Ansicht(GL_Ansicht* gl_ansicht)
{ DBG_PROG_START
  if (!icc_betrachter->mft_gl->beruehrt()) {
    icc_betrachter->mft_gl->show();
    icc_betrachter->mft_gl->init(1);
  }
  if (gl_ansicht == icc_betrachter->DD_histogram) {
    if (!icc_betrachter->DD_histogram->visible()) {
      WARN_S("DD_histogram war nicht sichtbar")
      icc_betrachter->DD_histogram->show();
    }
    icc_betrachter->DD_histogram->init(2);
    if(profile.size())
      nachricht(&profile,0);
  }
  DBG_PROG_ENDE
}

#if 0
void
ICCexamin::glAnsicht (GL_Ansicht* dazu)
{ DBG_PROG_START
  bool vorhanden = false; DBG_PROG_V( _gl_ansichten.size() )
  if (dazu != icc_betrachter->mft_gl && // zuerst mft_gl
      _gl_ansichten.size() < 1)
  { DBG_PROG
    icc_betrachter->mft_gl->zeigen(); DBG_PROG
  }
  for (unsigned i = 0; i < _gl_ansichten.size(); i++)
  if (dazu == _gl_ansichten[i])
  { DBG_PROG
    vorhanden = true;
    _gl_ansicht = dazu->id();
  }
  if(!vorhanden) { DBG_PROG
    _gl_ansicht = dazu->id();
    _gl_ansichten.resize( _gl_ansichten.size() +1 );
    _gl_ansichten[_gl_ansichten.size() -1] = dazu;
  }
  DBG_PROG_ENDE
}

GL_Ansicht*
ICCexamin::glAnsicht(int id)
{ DBG_EXAMIN_START
  if(id>0)
  { DBG_EXAMIN_V( id )
    std::vector<GL_Ansicht*>::iterator it;
    for (it = _gl_ansichten.begin() ; it != _gl_ansichten.end(); ++it)
      if ((*it)->id() == id)
      { DBG_EXAMIN_ENDE
        return *it;
      }
  }
  DBG_EXAMIN_ENDE
  return 0;
}

#endif
