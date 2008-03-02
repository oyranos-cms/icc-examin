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

ICCexamin::ICCexamin ()
{
  icc_betrachter = new ICCfltkBetrachter [1];
}

ICCexamin::~ICCexamin ()
{
  delete icc_betrachter;
}

void
ICCexamin::start (int argc, char** argv)
{
  icc_betrachter->start(argc, argv);
}

void
ICCexamin::open (int interaktiv)
{
  icc_betrachter->open( interaktiv );
}

std::string
ICCexamin::selected_tag (int item)
{ DBG_PROG_START

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
      std::vector<double> alle_punkte, punkte;
      std::vector<std::string> alle_texte;
      std::string TagName;
      for (unsigned int i_name = 0; i_name < rgb_tags.size(); i_name++) {
        if (profile[0].hasTagName (rgb_tags[i_name])) {
          punkte = profile[0].getTagCIEXYZ (profile[0].getTagByName(rgb_tags[i_name]));
          for (unsigned int i = 0; i < 3; i++)
            alle_punkte.push_back (punkte[i]);
          TagInfo = profile[0].printTagInfo (profile[0].getTagByName(rgb_tags[i_name]));
          for (unsigned int i = 0; i < 2; i++)
            alle_texte.push_back (TagInfo[i]);
        }
      }
      icc_examin->icc_betrachter->tag_viewer->hinein_punkt( alle_punkte, alle_texte );
    } else if ( TagInfo[1] == "curv"
             || TagInfo[1] == "bfd" ) {
      std::vector<std::vector<double> > kurven;
      std::vector<double> kurve;
      std::vector<std::string> texte;
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
      icc_examin->icc_betrachter->tag_viewer->hinein_punkt( profile[0].getTagCIEXYZ(item), profile[0].getTagText(item) );
    } else if ( TagInfo[1] == "XYZ" ) {
      icc_examin->icc_betrachter->tag_viewer->hinein_punkt( profile[0].getTagCIEXYZ(item), TagInfo );
    } else if ( TagInfo[1] == "mft2"
             || TagInfo[1] == "mft1" ) {
      icc_examin->icc_betrachter->mft_choice->profil_tag (item);
      //mft_text->hinein ( (profile[0].getTagText (item))[0] ); DBG_PROG
    } else if ( TagInfo[1] == "vcgt" ) { DBG_PROG
      icc_examin->icc_betrachter->tag_viewer->hinein_kurven ( profile[0].getTagCurves (item, ICCtag::CURVE_IN),
                                  profile[0].getTagText (item) ); cout << "vcgt "; DBG_PROG

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


