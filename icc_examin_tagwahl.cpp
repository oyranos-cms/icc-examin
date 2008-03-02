/*
 * ICC Examin ist eine ICC Profil Betrachter
 * 
 * Copyright (C) 2004-2007  Kai-Uwe Behrmann 
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
 * the centrale class. tag selection part
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

const char * selectTextsLine( int * line );


std::string
ICCexamin::waehleTag (int item)
{ DBG_PROG_START

  DBG_PROG_V( item )

  std::string text = _("empty");

  if(!profile.size() ||
     !icc_betrachter->examin->visible_r() ||
     !icc_betrachter->examin->visible() ||
     !profile.profil() )
  {
    DBG_PROG_ENDE
    return text;
  }

  if( item >= (int)profile.profil()->tagCount() )
  {
    frei(false);
    icc_betrachter->tag_text->hinein(text);
    frei(true);
    icc_betrachterNeuzeichnen(icc_betrachter->tag_text);
    DBG_PROG_ENDE
    return text;
  }

  if(!icc_betrachter->tag_text->cb)
  {
    tagTextsCB_f cb = &selectTextsLine;
    icc_betrachter->tag_text->cb = cb;
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
             || TagInfo[1] == "dtim"
             || TagInfo[1] == "desc"
              ) {

      frei(false);
      std::vector<std::string> texte_l = profile.profil()->getTagText (item);
      if( texte_l.size() )
      {
        std::string text_l = texte_l[0];
        if( profile.profil()->tagBelongsToMeasurement(item) )
        {
          icc_betrachter->tag_text->hinein ( text_l,
          profile.profil()->getMeasurement().getPatchLines(TagInfo[0].c_str()));

        } else
          icc_betrachter->tag_text->hinein ( text_l );
      }
      frei(true);

      icc_betrachterNeuzeichnen(icc_betrachter->tag_text);
    } else if ( TagInfo[0] == "rXYZ"
             || TagInfo[0] == "gXYZ"
             || TagInfo[0] == "bXYZ" ) {
      std::string TagName;
      std::vector<double> punkt;
      for (unsigned int i_name = 0; i_name < rgb_tags.size(); i_name++) {
        if (profile.profil()->hasTagName (rgb_tags[i_name])) {
          punkt = profile.profil()->getTagCIEXYZ (profile.profil()->getTagIDByName(rgb_tags[i_name]));
          for (unsigned int i = 0; i < 3; i++)
            punkte[TAG_VIEWER].push_back (punkt[i]);
          TagInfo = profile.profil()->printTagInfo (profile.profil()->getTagIDByName(rgb_tags[i_name]));
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
      {
        if(TagInfo[0] == "ncl2")
        {
            ICCprofile * pr = profile.profil();
            std::vector<double> p_neu = pr->getTagNumbers(item, ICCtag::MATRIX);
            int n = p_neu.size()/3;

            std::vector<int> patches;
            patches.resize( n );
            for(int i = 0; i < n; ++i)
            {
              patches[i] = i+5;
            }
          icc_betrachter->tag_text->hinein ( texte[0], patches );

        } else
          icc_betrachter->tag_text->hinein ( texte[0] );

      } else
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
  //selection from mft_choice

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
  case 0: // overview
    { std::vector<std::string> Info = icc_betrachter->mft_choice->Info;
      //profile.profil()->getTagText (icc_betrachter->tag_nummer)[0];
      for (unsigned int i = 1; i < Info.size(); i++) // leave out first line
        s << Info [i] << endl;
      icc_betrachter->mft_text->hinein ( s.str() ); DBG_PROG_S("show text")
      icc_betrachterNeuzeichnen(icc_betrachter->mft_text);
    } break;
  case 1: // matrix
    zahlen = profile.profil()->getTagNumbers (icc_betrachter->tag_nummer, ICCtag::MATRIX);
    DBG_PROG_S("show numbers in mft_text")
    assert (9 == zahlen.size());
    s << endl <<
    "  " << zahlen[0] << ", " << zahlen[1] << ", " << zahlen[2] << ", " << endl <<
    "  " << zahlen[3] << ", " << zahlen[4] << ", " << zahlen[5] << ", " << endl <<
    "  " << zahlen[6] << ", " << zahlen[7] << ", " << zahlen[8] << ", " << endl;
    icc_betrachter->mft_text->hinein ( s.str() ); DBG_PROG
    icc_betrachterNeuzeichnen(icc_betrachter->mft_text);
    break;
  case 2: // input curves
    DBG_PROG_S("show curves")
    kurven[MFT_VIEWER] =  profile.profil()->getTagCurves (icc_betrachter->tag_nummer, ICCtag::CURVE_IN);
    texte[MFT_VIEWER] = profile.profil()->getTagChannelNames (icc_betrachter->tag_nummer, ICCtag::CURVE_IN);
    icc_betrachter->mft_viewer->hineinKurven ( kurven[MFT_VIEWER], texte[MFT_VIEWER] );
    icc_betrachterNeuzeichnen(icc_betrachter->mft_viewer);
    DBG_PROG
    break;
  case 3: // 3D table
    DBG_PROG_S("show table")
    {
      std::vector<std::string> nach_farb_namen = profile.profil()->getTagChannelNames (icc_betrachter->tag_nummer, ICCtag::TABLE_OUT);
      icColorSpaceSignature sig_out = profile.profil()->getTag( icc_betrachter->tag_nummer ).colorSpace( ICCtag::TABLE_OUT );
      std::vector<std::string> nach_farben_snamen =  getChannelNamesShort( sig_out );

      icc_betrachter->mft_gl->hineinTabelle (
                     profile.profil()->getTagTable (icc_betrachter->tag_nummer, ICCtag::TABLE),
                     profile.profil()->getTagChannelNames (icc_betrachter->tag_nummer, ICCtag::TABLE_IN),
                     nach_farb_namen ); DBG_PROG_S( "3D table" )

      int      n = (int)nach_farb_namen.size();
      if(n != (int)nach_farben_snamen.size())
      {
        sig_out = getColorSpaceGeneric( n );
        nach_farben_snamen =  getChannelNamesShort( sig_out );
      }

      if(n == (int)nach_farben_snamen.size())
      {
        const char ** pp = new const char* [n];
        const char ** sn = new const char* [n];
        for (int i = 0; i < n; i++)
        {
          pp[i] = nach_farb_namen[i].c_str();
          sn[i] = nach_farben_snamen[i].c_str();
        }
        icc_examin->icc_betrachter->mft_gl_boxAdd( sn, pp, n, icc_betrachter->mft_gl->kanal );

      } else {
        icc_examin->icc_betrachter->mft_gl_boxAdd( NULL, NULL, 0, 0 );
      }
    }
    icc_betrachterNeuzeichnen(icc_betrachter->mft_gl_group);
    break;
  case 4: // output curves
    DBG_PROG_S("show curves")
    kurven[MFT_VIEWER] = profile.profil()->getTagCurves (icc_betrachter->tag_nummer, ICCtag::CURVE_OUT);
    texte[MFT_VIEWER] = profile.profil()->getTagChannelNames (icc_betrachter->tag_nummer, ICCtag::CURVE_OUT);
    icc_betrachter->mft_viewer->hineinKurven ( kurven[MFT_VIEWER], texte[MFT_VIEWER] );
    icc_betrachterNeuzeichnen(icc_betrachter->mft_viewer);
    DBG_PROG
    break;
  }

  icc_betrachter->mft_choice->gewaehlter_eintrag = item;
  if(item <= 0)
    icc_betrachter->mft_choice->value(0);
  DBG_NUM_V( item )

  frei(true);
  DBG_PROG_ENDE
}

const char *
selectTextsLine( int * line )
{
  int i = 0;
  const char * txt = "--";

  if(line)
  {
    int item = icc_examin->tag_nr();
    i = *line;
    //*line = 1;
    txt = icc_examin->icc_betrachter->tag_text->text(i);

    std::vector<std::string> TagInfo = profile.profil()->printTagInfo(item);
    if( TagInfo.size() == 2 )
    {
      std::vector<float> v;
      std::string name;
      std::vector<double> lab;
      double l[3];
      double c[32];

      if(profile.profil()->tagBelongsToMeasurement(item) &&
         icc_examin->icc_betrachter->tag_browser->value() > 5)
      {
        lab = profile.profil()->getMeasurement().getPatchLine( i-1,
                                                        TagInfo[0].c_str(),
                                                        v, name );
        for(unsigned int i = 0; i < 3; ++i) l[i] = lab[i];
        for(unsigned int i = 0; i < v.size() && i < 32; ++i) c[i] = v[i];

        oyNamedColour_s * colour = oyNamedColourCreate(
                              l, c,
                              profile.profil()->colorSpace(), 0, 
                              name.c_str(), 0, name.c_str(),
                              0,0, profile.profil()->filename(), malloc, free );
        if(lab.size() == 3)
        {
          icc_examin->icc_betrachter->DD_farbraum->emphasizePoint( colour );
          // very simple approach, but enough to see the line
          icc_examin->icc_betrachter->inspekt_html->topline( name.c_str() );

          DBG_PROG_S( txt <<" "<< TagInfo[0] <<" "<< TagInfo[1] <<" L "<< lab[0] <<" a "<< lab[1] <<" b "<< lab[2] )
        } else
          icc_examin->icc_betrachter->DD_farbraum->emphasizePoint( colour );

        oyNamedColourRelease( colour );

      } else if( profile.profil()->hasTagName("ncl2") &&
                 TagInfo[0] == "ncl2" ) {
          oyNamedColour_s * colour = 0;
          if( icc_examin->icc_betrachter->tag_text->value() > 5 )
          {
            std::vector<std::string> names;
            icc_examin->farbenLese(-(i-5), lab,v,names);
            name = names.size() ? names[0] : 0;
            for(unsigned int i = 0; i < 3; ++i) l[i] = lab[i];
            for(unsigned int i = 0; i < v.size() && i < 32; ++i) c[i] = v[i];
          
            colour = oyNamedColourCreate(
                              l, c,
                              profile.profil()->colorSpace(), 0, 
                              name.c_str(), 0, name.c_str(),
                              0,0, profile.profil()->filename(), malloc, free );
            icc_examin->icc_betrachter->DD_farbraum->emphasizePoint( colour );
            oyNamedColourRelease( colour );
          } else {
            icc_examin->icc_betrachter->DD_farbraum->emphasizePoint( NULL );
          }
      }
    }
  }

  return txt;
}

