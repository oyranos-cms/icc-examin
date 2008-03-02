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
#include "icc_draw.h"
#include "icc_gl.h"

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



std::string
ICCexamin::waehleTag (int item)
{ DBG_PROG_START

  DBG_PROG_V( item )

  std::string text = _("Leer");

  if(!profile.size()) {
    DBG_PROG_ENDE
    return text;
  }


  frei(false);
  kurven[TAG_VIEWER].clear();
  punkte[TAG_VIEWER].clear();
  texte[TAG_VIEWER].clear();
  frei(true);

  std::vector<std::string> rgb_tags;
  rgb_tags.push_back("rXYZ");
  rgb_tags.push_back("gXYZ");
  rgb_tags.push_back("bXYZ");

  icc_examin_ns::lock(__FILE__,__LINE__);
  if (item < 0) {
    icc_betrachter->tag_browser->select(5);
    text = profile.profil()->printLongHeader(); DBG_PROG
    frei(false);
    icc_betrachter->tag_text->hinein(text);
    frei(true);

    if(farbraumModus())
      _item = item;

    text = "";
    icc_betrachterNeuzeichnen(icc_betrachter->tag_text);
  } else if (item >= 0) {
    std::vector<std::string> TagInfo = profile.profil()->printTagInfo(item);

    kurve_umkehren[TAG_VIEWER] = false;

    _item = item;

    DBG_PROG_S( TagInfo.size() << " " << TagInfo[0] << " " << TagInfo[1] )

    if        ( TagInfo[1] == "text"
             || TagInfo[1] == "cprt?"
             || TagInfo[1] == "meas"
             || TagInfo[1] == "sig"
             || TagInfo[1] == "dtim") {
      frei(false);
      icc_betrachter->tag_text->hinein ( (profile.profil()->getTagText (item))[0] );
      frei(true);
      icc_betrachterNeuzeichnen(icc_betrachter->tag_text);
    } else if ( TagInfo[1] == "desc" ) {
      frei(false);
      icc_betrachter->tag_text->hinein( (profile.profil()->getTagDescription (item))[0] ); DBG_PROG
      frei(true);
      icc_betrachterNeuzeichnen(icc_betrachter->tag_text);
    } else if ( TagInfo[0] == "rXYZ"
             || TagInfo[0] == "gXYZ"
             || TagInfo[0] == "bXYZ" ) {
      std::string TagName;
      std::vector<double> punkt;
      for (unsigned int i_name = 0; i_name < rgb_tags.size(); i_name++) {
        if (profile.profil()->hasTagName (rgb_tags[i_name])) {
          punkt = profile.profil()->getTagCIEXYZ (profile.profil()->getTagByName(rgb_tags[i_name]));
          for (unsigned int i = 0; i < 3; i++)
            punkte[TAG_VIEWER].push_back (punkt[i]);
          TagInfo = profile.profil()->printTagInfo (profile.profil()->getTagByName(rgb_tags[i_name]));
          texte[TAG_VIEWER].push_back (TagInfo[0]);
        }
      }
      texte[TAG_VIEWER].push_back (_("RGB primaries"));
      texte[TAG_VIEWER].push_back ("XYZ");
      frei(false);
      icc_betrachter->tag_viewer->hineinPunkt( punkte[TAG_VIEWER], texte[TAG_VIEWER] );
      frei(true);
      icc_betrachterNeuzeichnen(icc_betrachter->tag_viewer);
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
          texte[TAG_VIEWER].push_back (TagInfo[0]);
        }
      }
      texte[TAG_VIEWER].push_back ("curv");
      frei(false);
      icc_betrachter->tag_viewer->hineinKurven( kurven[TAG_VIEWER], texte[TAG_VIEWER] );
      frei(true);
      icc_betrachterNeuzeichnen(icc_betrachter->tag_viewer);
    } else if ( TagInfo[1] == "chrm" ) {
      punkte[TAG_VIEWER] = profile.profil()->getTagCIEXYZ(item);
      texte [TAG_VIEWER] = profile.profil()->getTagText(item);
      frei(false);
      icc_betrachter->tag_viewer->hineinPunkt( punkte[TAG_VIEWER], texte[TAG_VIEWER] );
      frei(true);
      icc_betrachterNeuzeichnen(icc_betrachter->tag_viewer);
    } else if ( TagInfo[1] == "XYZ" ) {
      punkte[TAG_VIEWER] = profile.profil()->getTagCIEXYZ(item);
      texte[TAG_VIEWER] = TagInfo;
      frei(false);
      icc_betrachter->tag_viewer->hineinPunkt( punkte[TAG_VIEWER], texte[TAG_VIEWER] );
      frei(true);
      icc_betrachterNeuzeichnen(icc_betrachter->tag_viewer);
    } else if ( TagInfo[1] == "mft2"
             || TagInfo[1] == "mft1" ) { DBG_PROG_S("mft1/2")
      std::string t = profile.profil()->getTagText (item)[0];
      frei(false);
      icc_betrachter->mft_choice->profilTag (item, t);
      frei(true);
      waehleMft (_mft_item);
    } else if ( TagInfo[1] == "vcgt" ) { DBG_PROG_S("vcgt")
      kurve_umkehren[TAG_VIEWER] = true;
      kurven[TAG_VIEWER] = profile.profil()->getTagCurves (item, ICCtag::CURVE_IN);
      texte[TAG_VIEWER] = profile.profil()->getTagText (item);
      frei(false);
      icc_betrachter->tag_viewer->hineinKurven ( kurven[TAG_VIEWER], texte[TAG_VIEWER] );
      icc_betrachter->tag_viewer->kurve_umkehren = true;
      frei(true);
      DBG_PROG_S( "vcgt" )

      icc_betrachterNeuzeichnen(icc_betrachter->tag_viewer);
/*    } else if ( TagInfo[1] == "chad" ) {
      std::vector<double> zahlen = profile.profil()->getTagNumbers (icc_betrachter->tag_nummer, ICCtag::MATRIX);
      cout << zahlen.size() << endl; DBG_PROG
      assert (9 == zahlen.size());
      std::stringstream s;
      s << endl <<
      "  " << zahlen[0] << ", " << zahlen[1] << ", " << zahlen[2] << ", " << endl <<
      "  " << zahlen[3] << ", " << zahlen[4] << ", " << zahlen[5] << ", " << endl <<
      "  " << zahlen[6] << ", " << zahlen[7] << ", " << zahlen[8] << ", " << endl;
      icc_betrachter->tag_text->hinein ( s.str() ); DBG_PROG

      icc_betrachterNeuzeichnen(icc_betrachter->tag_text); */
    } else {
      frei(false);
      std::vector<std::string> texte = profile.profil()->getTagText (item);
      if(texte.size())
        icc_betrachter->tag_text->hinein ( texte[0] );
      else
        icc_betrachter->tag_text->hinein ( "" );
        
      frei(true);
      icc_betrachterNeuzeichnen(icc_betrachter->tag_text);
    }

    text = TagInfo[0];
  }

  icc_examin_ns::unlock(this, __FILE__,__LINE__);
  DBG_PROG_ENDE
  return text;
}


void
ICCexamin::waehleMft (int item)
{ DBG_PROG_START
  frei(false);
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
      icc_betrachterNeuzeichnen(icc_betrachter->mft_text);
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
    icc_betrachterNeuzeichnen(icc_betrachter->mft_text);
    break;
  case 2: // Eingangskurven
    DBG_PROG_S("Kurven in anzeigen")
    kurven[MFT_VIEWER] =  profile.profil()->getTagCurves (icc_betrachter->tag_nummer, ICCtag::CURVE_IN);
    texte[MFT_VIEWER] = profile.profil()->getTagChannelNames (icc_betrachter->tag_nummer, ICCtag::CURVE_IN);
    icc_betrachter->mft_viewer->hineinKurven ( kurven[MFT_VIEWER], texte[MFT_VIEWER] );
    icc_betrachterNeuzeichnen(icc_betrachter->mft_viewer);
    DBG_PROG
    break;
  case 3: // 3D Tabelle
    DBG_PROG_S("Tabelle in anzeigen")
    icc_betrachter->mft_gl->hineinTabelle (
                     profile.profil()->getTagTable (icc_betrachter->tag_nummer, ICCtag::TABLE),
                     profile.profil()->getTagChannelNames (icc_betrachter->tag_nummer, ICCtag::TABLE_IN),
                     profile.profil()->getTagChannelNames (icc_betrachter->tag_nummer, ICCtag::TABLE_OUT) ); DBG_PROG_S( "3D Tabelle" )
    icc_betrachterNeuzeichnen(icc_betrachter->mft_gl);
    break;
  case 4: // Ausgangskurven
    DBG_PROG_S("Kurven in anzeigen")
    kurven[MFT_VIEWER] = profile.profil()->getTagCurves (icc_betrachter->tag_nummer, ICCtag::CURVE_OUT);
    texte[MFT_VIEWER] = profile.profil()->getTagChannelNames (icc_betrachter->tag_nummer, ICCtag::CURVE_OUT);
    icc_betrachter->mft_viewer->hineinKurven ( kurven[MFT_VIEWER], texte[MFT_VIEWER] );
    icc_betrachterNeuzeichnen(icc_betrachter->mft_viewer);
    DBG_PROG
    break;
  }

  icc_betrachter->mft_choice->gewaehlter_eintrag = item;
  frei(true);
  DBG_PROG_ENDE
}


