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
 * Die zentrale Klasse.
 * 
 */


#include "icc_examin.h"
#include "icc_betrachter.h"
#include "icc_draw.h"

ICCexamin::ICCexamin ()
{ DBG_PROG_START
  icc_betrachter = new ICCfltkBetrachter [1];
  DBG_PROG_ENDE
}

ICCexamin::~ICCexamin ()
{ DBG_PROG_START
  delete icc_betrachter;
  DBG_PROG_ENDE
}

void
ICCexamin::start (int argc, char** argv)
{ DBG_PROG_START

  icc_betrachter->init();
  DBG_PROG

      if (argc>1) {
        statlabel = argv[1];
        statlabel.append (" ");
        statlabel.append (_("geladen"));
        status(statlabel.c_str());
        profilnamen.resize(argc-1);
        for (int i = 1; i < argc; i++) {
          DBG_PROG_V( i ) profilnamen[i-1] = argv[i];
        }
        oeffnen (profilnamen);
      } else {
        status(_("Konnte Datei nicht laden!"));
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
  profile.resize(dateinamen.size());
  for (unsigned int i = 0; i < dateinamen.size(); i++)
    profile[i].load (dateinamen[i]);

  if (dateinamen.size()) { DBG_PROG
    for (unsigned int i = 0; i < profile.size(); i++) {
      //create_vrml ( dateiname.c_str(), "/usr/share/color/icc/sRGB.icm", &vrmlDatei[0]);

      icc_betrachter->load_progress->value (0.8);

      std::vector<std::string> url;
      std::vector<std::string> param;
      //url.push_back (&vrmlDatei[0]);
      //browser->load_url(url, param);
        statlabel = dateinamen[i].c_str();
        statlabel.append (" ");
        statlabel.append (_("geladen"));
        status(statlabel.c_str());
    }
    icc_betrachter->load_progress->value (1.0);
    icc_betrachter->load_progress->value (0.0);
    icc_betrachter->load_progress->hide();
    DBG_PROG

    icc_betrachter->tag_browser->reopen ();

    icc_betrachter->measurement( profile[0].hasMeasurement() );
  } else {
    status(_("Datei nicht geladen!"));
  } DBG_PROG

  DBG_PROG_ENDE
}

void
ICCexamin::oeffnen ()
{ DBG_PROG_START

  profilnamen = icc_betrachter->open( profilnamen );
  oeffnen( profilnamen );

  DBG_PROG_ENDE
}

std::string
ICCexamin::selectTag (int item)
{ DBG_PROG_START

  kurven.clear();
  punkte.clear();
  texte.clear();
  kurve_umkehren = false;

  std::string text = _("Leer");

  std::vector<std::string> rgb_tags;
  rgb_tags.push_back("rXYZ");
  rgb_tags.push_back("gXYZ");
  rgb_tags.push_back("bXYZ");

  if (item < 0) {
    icc_betrachter->tag_browser->select(5);
    text = profile[0].printLongHeader(); DBG_PROG
    icc_examin->icc_betrachter->tag_text->hinein(text);

    text = "";
  } else if (item >= 0) {
    std::vector<std::string> TagInfo = profile[0].printTagInfo(item);
    DBG_PROG_S( TagInfo.size() << " " << TagInfo[0] << " " << TagInfo[1] )

    if        ( TagInfo[1] == "text"
             || TagInfo[1] == "cprt?"
             || TagInfo[1] == "meas"
             || TagInfo[1] == "sig"
             || TagInfo[1] == "dtim") {
      icc_examin->icc_betrachter->tag_text->hinein ( (profile[0].getTagText (item))[0] ); DBG_PROG
    } else if ( TagInfo[1] == "desc" ) {
      icc_examin->icc_betrachter->tag_text->hinein( (profile[0].getTagDescription (item))[0] ); DBG_PROG
    } else if ( TagInfo[0] == "rXYZ" || TagInfo[0] == "gXYZ" || TagInfo[0] == "bXYZ" ) {
      std::string TagName;
      std::vector<double> punkte;
      for (unsigned int i_name = 0; i_name < rgb_tags.size(); i_name++) {
        if (profile[0].hasTagName (rgb_tags[i_name])) {
          punkte = profile[0].getTagCIEXYZ (profile[0].getTagByName(rgb_tags[i_name]));
          for (unsigned int i = 0; i < 3; i++)
            punkte.push_back (punkte[i]);
          TagInfo = profile[0].printTagInfo (profile[0].getTagByName(rgb_tags[i_name]));
          for (unsigned int i = 0; i < 2; i++)
            texte.push_back (TagInfo[i]);
        }
      }
      icc_examin->icc_betrachter->tag_viewer->hinein_punkt( punkte, texte );
    } else if ( TagInfo[1] == "curv"
             || TagInfo[1] == "bfd" ) {
      std::vector<double> kurve;
      std::string TagName;
      for (int i_name = 0; i_name < profile[0].tagCount(); i_name++) {
        if ( (profile[0].printTagInfo(i_name))[1] == "curv"
          || (profile[0].printTagInfo(i_name))[1] == "bfd" ) {
          kurve = profile[0].getTagCurve (i_name);
          kurven.push_back (kurve);
          TagInfo = profile[0].printTagInfo (i_name);
          //for (unsigned int i = 0; i < 2; i++)
          texte.push_back (TagInfo[0]);
        }
      }
      texte.push_back ("curv");
      icc_examin->icc_betrachter->tag_viewer->hinein_kurven( kurven, texte );
    } else if ( TagInfo[1] == "chrm" ) {
      punkte = profile[0].getTagCIEXYZ(item);
      texte = profile[0].getTagText(item);
      icc_examin->icc_betrachter->tag_viewer->hinein_punkt( punkte, texte );
    } else if ( TagInfo[1] == "XYZ" ) {
      punkte = profile[0].getTagCIEXYZ(item);
      texte = TagInfo;
      icc_examin->icc_betrachter->tag_viewer->hinein_punkt( punkte, texte );
    } else if ( TagInfo[1] == "mft2"
             || TagInfo[1] == "mft1" ) {
      icc_examin->icc_betrachter->mft_choice->profil_tag (item);
      //mft_text->hinein ( (profile[0].getTagText (item))[0] ); DBG_PROG
    } else if ( TagInfo[1] == "vcgt" ) { DBG_PROG
      kurve_umkehren = true;
      kurven = profile[0].getTagCurves (item, ICCtag::CURVE_IN);
      texte = profile[0].getTagText (item);
      icc_examin->icc_betrachter->tag_viewer->hinein_kurven ( kurven, texte );
      cout << "vcgt "; DBG_PROG

    /*} else if ( TagInfo[1] == "chad" ) {
      std::vector<int> zahlen = profile[0].getTagNumbers (tag_nummer, ICCtag::MATRIX);
      cout << zahlen.size() << endl; DBG_PROG
      assert (9 == zahlen.size());
      s << endl <<
      "  " << zahlen[0] << ", " << zahlen[1] << ", " << zahlen[2] << ", " << endl <<
      "  " << zahlen[3] << ", " << zahlen[4] << ", " << zahlen[5] << ", " << endl <<
      "  " << zahlen[6] << ", " << zahlen[7] << ", " << zahlen[8] << ", " << endl;
      icc_examin->icc_betrachter->tag_text->hinein ( s.str() ); DBG_PROG
*/
    } else {
      icc_examin->icc_betrachter->tag_text->hinein ( (profile[0].getTagText (item))[0] ); DBG_PROG
    }

    text = TagInfo[0];
  }

  DBG_PROG_ENDE
  return text;
}

void
ICCexamin::drawKurve   (int x,int y,int w,int h)
{ DBG_PROG_START
      if (kurven.size())
      {
        wiederholen = false;
        draw_kurve   (x,y,w,h,kurve_umkehren);
      } else {
        if (wiederholen)
        { draw_cie_shoe(x,y,w,h,false);
          Fl::add_timeout( 1.2, (void(*)(void*))d_haendler ,(void*)this);
        } else {
          draw_cie_shoe(x,y,w,h,true);
        }
        wiederholen = true;
      }
      DBG_PROG;
  DBG_PROG_ENDE
}

