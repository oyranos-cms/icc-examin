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
//#include "icc_examin.h"
#include "icc_betrachter.h"
#include "icc_draw.h"
#include "icc_gl.h"

ICCexamin::ICCexamin ()
{ DBG_PROG_START
  icc_betrachter = new ICCfltkBetrachter [1];
  _item = -1;
  _mft_item = -1;
  _zeig_prueftabelle = 0;
  _zeig_histogram = 0;
  _gl_ansicht = -1;
  statlabel = "";
  DBG_PROG_ENDE
}

void
ICCexamin::quit ()
{ DBG_PROG_START
  delete icc_betrachter;
  for (unsigned i = 0; i < _gl_ansichten.size(); i++)
    delete _gl_ansichten[i];
  agviewers.resize(0);
  DBG_PROG_ENDE
  exit(0);
}

void
ICCexamin::start (int argc, char** argv)
{ DBG_PROG_START

  icc_betrachter->init();
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

  DBG_PROG
  icc_betrachter->run();

  DBG_PROG_ENDE
}

//#include "icc_vrml.h"
void
ICCexamin::oeffnen (std::vector<std::string> dateinamen)
{ DBG_PROG_START
  // Laden
  profile.oeffnen(dateinamen);
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

  std::string text = _("Leer");

  icc_examin->icc_betrachter->mft_choice->hide();

  std::vector<std::string> rgb_tags;
  rgb_tags.push_back("rXYZ");
  rgb_tags.push_back("gXYZ");
  rgb_tags.push_back("bXYZ");

  if (item < 0) {
    icc_betrachter->tag_browser->select(5);
    text = profile.profil()->printLongHeader(); DBG_PROG
    icc_examin->icc_betrachter->tag_text->hinein(text);
    neuzeichnen(icc_betrachter->tag_text);

    text = "";
  } else if (item >= 0) {
    std::vector<std::string> TagInfo = profile.profil()->printTagInfo(item);
    //if(_item == item)
      //return TagInfo[0];

    kurven.clear();
    punkte.clear();
    texte.clear();
    kurve_umkehren = false;

    _item = item;

    DBG_PROG_S( TagInfo.size() << " " << TagInfo[0] << " " << TagInfo[1] )

    if        ( TagInfo[1] == "text"
             || TagInfo[1] == "cprt?"
             || TagInfo[1] == "meas"
             || TagInfo[1] == "sig"
             || TagInfo[1] == "dtim") {
      icc_examin->icc_betrachter->tag_text->hinein ( (profile.profil()->getTagText (item))[0] ); DBG_PROG
      neuzeichnen(icc_betrachter->tag_text);
    } else if ( TagInfo[1] == "desc" ) {
      icc_examin->icc_betrachter->tag_text->hinein( (profile.profil()->getTagDescription (item))[0] ); DBG_PROG
      neuzeichnen(icc_betrachter->tag_text);
    } else if ( TagInfo[0] == "rXYZ" || TagInfo[0] == "gXYZ" || TagInfo[0] == "bXYZ" ) {
      std::string TagName;
      std::vector<double> punkt;
      for (unsigned int i_name = 0; i_name < rgb_tags.size(); i_name++) {
        if (profile.profil()->hasTagName (rgb_tags[i_name])) {
          punkt = profile.profil()->getTagCIEXYZ (profile.profil()->getTagByName(rgb_tags[i_name]));
          for (unsigned int i = 0; i < 3; i++)
            punkte.push_back (punkt[i]);
          TagInfo = profile.profil()->printTagInfo (profile.profil()->getTagByName(rgb_tags[i_name]));
          for (unsigned int i = 0; i < 2; i++)
            texte.push_back (TagInfo[i]);
        }
      }
      icc_examin->icc_betrachter->tag_viewer->hinein_punkt( punkte, texte );
      neuzeichnen(icc_betrachter->tag_viewer);
    } else if ( TagInfo[1] == "curv"
             || TagInfo[1] == "bfd" ) {
      std::vector<double> kurve;
      std::string TagName;
      for (int i_name = 0; i_name < profile.profil()->tagCount(); i_name++) {
        if ( (profile.profil()->printTagInfo(i_name))[1] == "curv"
          || (profile.profil()->printTagInfo(i_name))[1] == "bfd" ) {
          kurve = profile.profil()->getTagCurve (i_name);
          kurven.push_back (kurve);
          TagInfo = profile.profil()->printTagInfo (i_name);
          //for (unsigned int i = 0; i < 2; i++)
          texte.push_back (TagInfo[0]);
        }
      }
      texte.push_back ("curv");
      icc_examin->icc_betrachter->tag_viewer->hinein_kurven( kurven, texte );
      neuzeichnen(icc_betrachter->tag_viewer);
    } else if ( TagInfo[1] == "chrm" ) {
      punkte = profile.profil()->getTagCIEXYZ(item);
      texte = profile.profil()->getTagText(item);
      icc_examin->icc_betrachter->tag_viewer->hinein_punkt( punkte, texte );
      neuzeichnen(icc_betrachter->tag_viewer);
    } else if ( TagInfo[1] == "XYZ" ) {
      punkte = profile.profil()->getTagCIEXYZ(item);
      texte = TagInfo;
      icc_examin->icc_betrachter->tag_viewer->hinein_punkt( punkte, texte );
      neuzeichnen(icc_betrachter->tag_viewer);
    } else if ( TagInfo[1] == "mft2"
             || TagInfo[1] == "mft1" ) { DBG_PROG
      std::string t = profile.profil()->getTagText (item)[0];
      icc_examin->icc_betrachter->mft_choice->profil_tag (item, t);
      icc_examin->icc_betrachter->mft_choice->show();
      neuzeichnen(icc_betrachter->mft_choice);
    } else if ( TagInfo[1] == "vcgt" ) { DBG_PROG
      kurve_umkehren = true;
      kurven = profile.profil()->getTagCurves (item, ICCtag::CURVE_IN);
      texte = profile.profil()->getTagText (item);
      icc_examin->icc_betrachter->tag_viewer->hinein_kurven ( kurven, texte );
      neuzeichnen(icc_betrachter->tag_viewer);
      cout << "vcgt "; DBG_PROG

    /*} else if ( TagInfo[1] == "chad" ) {
      std::vector<int> zahlen = profile.profil()->getTagNumbers (tag_nummer, ICCtag::MATRIX);
      cout << zahlen.size() << endl; DBG_PROG
      assert (9 == zahlen.size());
      s << endl <<
      "  " << zahlen[0] << ", " << zahlen[1] << ", " << zahlen[2] << ", " << endl <<
      "  " << zahlen[3] << ", " << zahlen[4] << ", " << zahlen[5] << ", " << endl <<
      "  " << zahlen[6] << ", " << zahlen[7] << ", " << zahlen[8] << ", " << endl;
      icc_examin->icc_betrachter->tag_text->hinein ( s.str() ); DBG_PROG
*/
    } else {
      icc_examin->icc_betrachter->tag_text->hinein ( (profile.profil()->getTagText (item))[0] ); DBG_PROG
      neuzeichnen(icc_betrachter->tag_text);
    }

    text = TagInfo[0];
  }

  DBG_PROG_ENDE
  return text;
}


#include "icc_gl.h"
void
ICCexamin::waehleMft (int item)
{ DBG_PROG_START
  //Auswahl aus mft_choice

  _mft_item = item;

  kurven.clear();
  punkte.clear();
  texte.clear();
  kurve_umkehren = false;

  status("")


  std::stringstream s;
  std::vector<double> zahlen;


  switch (item) {
  case 0: // Überblick
    { std::vector<std::string> Info = icc_examin->icc_betrachter->mft_choice->Info;
      //profile.profil()->getTagText (icc_examin->icc_betrachter->tag_nummer)[0];
      for (unsigned int i = 1; i < Info.size(); i++) // erste Zeile weglassen
        s << Info [i] << endl;
      icc_examin->icc_betrachter->mft_text->hinein ( s.str() ); DBG_PROG // anzeigen
      neuzeichnen(icc_betrachter->mft_text);
    } break;
  case 1: // Matrix
    zahlen = profile.profil()->getTagNumbers (icc_examin->icc_betrachter->tag_nummer, ICCtag::MATRIX);
    cout << zahlen.size() << endl; DBG_PROG
    assert (9 == zahlen.size());
    s << endl <<
    "  " << zahlen[0] << ", " << zahlen[1] << ", " << zahlen[2] << ", " << endl <<
    "  " << zahlen[3] << ", " << zahlen[4] << ", " << zahlen[5] << ", " << endl <<
    "  " << zahlen[6] << ", " << zahlen[7] << ", " << zahlen[8] << ", " << endl;
    icc_examin->icc_betrachter->mft_text->hinein ( s.str() ); DBG_PROG
      neuzeichnen(icc_betrachter->mft_text);
    break;
  case 2: // Eingangskurven
    DBG_PROG
    kurven =  profile.profil()->getTagCurves (icc_examin->icc_betrachter->tag_nummer, ICCtag::CURVE_IN);
    texte = profile.profil()->getTagChannelNames (icc_examin->icc_betrachter->tag_nummer, ICCtag::CURVE_IN);
    icc_examin->icc_betrachter->mft_viewer->hinein_kurven ( kurven, texte );
    neuzeichnen(icc_betrachter->mft_viewer);
    DBG_PROG
    break;
  case 3: // 3D Tabelle
    DBG_PROG
    icc_examin->icc_betrachter->mft_gl->hinein_tabelle (
                     profile.profil()->getTagTable (icc_examin->icc_betrachter->tag_nummer, ICCtag::TABLE),
                     profile.profil()->getTagChannelNames (icc_examin->icc_betrachter->tag_nummer, ICCtag::TABLE_IN),
                     profile.profil()->getTagChannelNames (icc_examin->icc_betrachter->tag_nummer, ICCtag::TABLE_OUT) ); DBG_PROG
    neuzeichnen(icc_betrachter->mft_gl);
    break;
  case 4: // Ausgangskurven
    kurven = profile.profil()->getTagCurves (icc_examin->icc_betrachter->tag_nummer, ICCtag::CURVE_OUT);
    texte = profile.profil()->getTagChannelNames (icc_examin->icc_betrachter->tag_nummer, ICCtag::CURVE_OUT);
    icc_examin->icc_betrachter->mft_viewer->hinein_kurven ( kurven, texte );
    neuzeichnen(icc_betrachter->mft_viewer);
    DBG_PROG
    break;
  }

  icc_examin->icc_betrachter->mft_choice->gewaehlter_eintrag = item;
  DBG_PROG_ENDE
}

void
ICCexamin::zeig_prueftabelle ()
{ DBG_PROG_START
  neuzeichnen(icc_betrachter->inspekt_html);
  DBG_PROG_ENDE
}

void
ICCexamin::histogram ()
{ DBG_PROG_START
  //std::vector<double> v;
  std::vector<std::vector<double> >v;
  std::vector<std::vector<float> > farben;
  std::vector<std::string> texte, namen;

  namen.push_back("Netz A");
  v.resize(1);
  farben.resize(1);
  for (int i = 0; i <= 20; i++)
  {
      v[0].push_back( i/20.0 );
      v[0].push_back( 0.5 );
      v[0].push_back( 0.5 );
      v[0].push_back( i/20.0 );
      v[0].push_back( 0.6 );
      v[0].push_back( 0.5 );
      if (i%2) {
        farben[0].push_back(.2);
        farben[0].push_back(.5);
        farben[0].push_back(1.0);
        farben[0].push_back(.5);
        farben[0].push_back(.2);
        farben[0].push_back(.2);
        farben[0].push_back(1.0);
        farben[0].push_back(.5);
      } else {
        farben[0].push_back(1.0);
        farben[0].push_back(.5);
        farben[0].push_back(.2);
        farben[0].push_back(.5);
        farben[0].push_back(1.0);
        farben[0].push_back(.2);
        farben[0].push_back(.2);
        farben[0].push_back(.5);
      }
  }

  texte.push_back(_("CIE *L"));
  texte.push_back(_("CIE *a"));
  texte.push_back(_("CIE *b"));

  //glAnsicht()->hinein_punkte( v, farben, texte );
  glAnsicht()->hinein_netze( v, farben, namen, texte );
  DBG_PROG_ENDE
}

void
ICCexamin::neuzeichnen (void* z)
{ DBG_PROG_START
  Fl_Widget *wid = (Fl_Widget*)z;
  static int item;


  if        (_zeig_histogram &&
             !icc_betrachter->menueintrag_3D->value())
  { DBG_PROG 
    icc_betrachter->DD_histogram->stop();
    icc_betrachter->DD_histogram->verstecken();
    _zeig_histogram = false;
  } else if (!_zeig_histogram &&
             icc_betrachter->menueintrag_3D->value())
  { DBG_PROG
    _zeig_histogram = true;
    icc_betrachter->DD_histogram->zeigen();
  } else if (icc_betrachter->menueintrag_inspekt->value())
  { DBG_PROG
    icc_betrachter->inspekt_html->show();
    _zeig_prueftabelle = true;
  } else if (!icc_betrachter->menueintrag_inspekt->value() &&
             _zeig_prueftabelle )
  { DBG_PROG
    _zeig_prueftabelle = false;
    icc_betrachter->inspekt_html->hide();
    waehleTag(_item);
    /*item = _item;
    _item = -2;
    static int nochmal;
    if (nochmal)
      nochmal = false;
    else*/
  }

  #define zeig(widget) { DBG_PROG_S( #widget ) \
  Fl_Widget *w = dynamic_cast<Fl_Widget*> (icc_betrachter->widget); \
  if (w == icc_betrachter->tag_viewer) \
    w->clear_visible(); DBG_PROG_V( item << _item ) \
  if (w != wid && w->visible()) { DBG_PROG \
    w->hide(); \
  } else if(w == wid /*&& \
            (!w->visible() || (_item != item))*/) { DBG_PROG_V( _item )\
    w->show(); \
    item = _item; \
  } \
  }

    zeig(mft_viewer)
    zeig(mft_text)
    zeig(tag_viewer)
    zeig(tag_text)
    zeig(inspekt_html)

  DBG_PROG_ENDE
}


