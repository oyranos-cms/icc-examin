/*
 * ICC Examin ist eine ICC Profil Betrachter
 * 
 * Copyright (C) 2004-2011  Kai-Uwe Behrmann 
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
const char * selecTpsidLine( int * line );

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

  ICClist<std::string> rgb_tags;
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
    ICClist<std::string> TagInfo = profile.profil()->printTagInfo(item);

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
      ICClist<std::string> texte_l = profile.profil()->getTagText (item);
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
      ICClist<double> punkt;
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
      ICClist<double> kurve;
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
    } else if ( TagInfo[1] == "mAB ")
    {
      std::string choice_text;
      const char * tmp = 0;
      if(TagInfo[1] == "mAB ")
      {
        /* skip the two channel info lines */
        std::string tmp_string = profile.profil()->getTagText(_item)[0];
        tmp = tmp_string.c_str();
        ICClist<std::string> lines = icc_parser::zeilenNachVector( tmp_string );
        choice_text = lines[0];
        choice_text += "\n";

        for(int i = 0; i < 3; ++i)
        {
          tmp = strchr(tmp, '\n');
          if(tmp)
            tmp++;
          else
            break;
        }
      }
      if(tmp)
        choice_text += tmp;

      frei(false);
      icc_betrachter->mft_choice->profilTag( _item, choice_text );
      frei(true);

      waehleMft(_mft_item);
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
      ICClist<double> zahlen = profile.profil()->getTagNumbers (icc_betrachter->tag_nummer, ICCtag::MATRIX);
      cout << zahlen.size() << endl; DBG_PROG
      assert (9 == zahlen.size());
      std::stringstream s;
      s << endl <<
      "  " << zahlen[0] << ", " << zahlen[1] << ", " << zahlen[2] << ", " << endl <<
      "  " << zahlen[3] << ", " << zahlen[4] << ", " << zahlen[5] << ", " << endl <<
      "  " << zahlen[6] << ", " << zahlen[7] << ", " << zahlen[8] << ", " << endl;
      icc_betrachter->tag_text->hinein ( s.str() ); DBG_PROG

      icc_betrachterNeuzeichnen(icc_betrachter->tag_text); */
    } else if ( TagInfo[1] == "ndin" ) {
      profile.frei(false);
      texte [TAG_VIEWER] = profile.profil()->getTagText(item);
      ICClist<double> colorimetry;
      std::string t = texte[TAG_VIEWER][0];
      /* parse from text */
      ICClist<std::string> lines = icc_parser::zeilenNachVector( t ), list;
      int n = lines.size(), i;
      /* odd text lines contain colorimetric data */
      for(i = 3; i < 10; i+=2)
      {
        double x,y;
        std::string line = lines[i];
        ICClist<icc_parser::ZifferWort> xy =
                                     icc_parser::unterscheideZiffernWorte (
                                                                  line, 0, " ");
        if(xy.size() == 2)
        {
          ICClist<double> values;
          x = xy[0].zahl.second;
          y = xy[1].zahl.second;
          values.push_back( x );
          values.push_back( y );
          values.push_back( 1.0 );
          xyYto_XYZ(values);
          for(int j = 0; j < 3; ++j)
            colorimetry.push_back( values[j] );
          std::stringstream s;
          s << lines[i-1] << lines[i];
          list.push_back( s.str() );
        }
      }
      for(i = 10; i < n; ++i)
        list.push_back( lines[i] );
      list.push_back( TagInfo[1] );
      texte [TAG_VIEWER] = list;
      punkte[TAG_VIEWER] = colorimetry;
      profile.frei(true);
      frei(false);
      //texte[TAG_VIEWER].push_back ("XYZ");
      icc_betrachter->tag_viewer->hineinPunkt( punkte[TAG_VIEWER], texte[TAG_VIEWER] );
      frei(true);
      icc_betrachterNeuzeichnen(icc_betrachter->tag_viewer);
    } else {
      frei(false);
      profile.frei(false);
      ICCprofile * pr = profile.profil();
      ICClist<std::string> texte;
      if(pr)
        texte = pr->getTagText (item);
      profile.frei(true);
      if(texte.size())
      {
        if(TagInfo[0] == "ncl2")
        {
            profile.frei(false);
            ICCprofile * pr = profile.profil();
            ICClist<double> p_neu = pr->getTagNumbers(item, ICCtag::MATRIX);
            profile.frei(true);
            int n = p_neu.size()/3;

            ICClist<int> patches;
            patches.resize( n );
            for(int i = 0; i < n; ++i)
            {
              patches[i] = i+5;
            }
          icc_betrachter->tag_text->hinein ( texte[0], patches );

        } else if(TagInfo[0] == "psid")
        {
          icc_betrachter->tag_text->hinein ( texte[0] );
          tagTextsCB_f cb = &selecTpsidLine;
          icc_betrachter->tag_text->cb = cb;

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

void ICCexamin::showData (int item)
{
  ICClist<std::string> TagInfo = profile.profil()->printTagInfo(_item);
  char num[12];
  int n;

  std::string t;

  frei(false);
  if(_mft_item == 0)
  {
    const char * tmp = 0;
    t += _("Intent:");
    t += " ";
    if(TagInfo[0] == "A2B0")
      t += renderingIntentName(0);
    if(TagInfo[0] == "A2B1")
      t += renderingIntentName(1);
    if(TagInfo[0] == "A2B2")
      t += renderingIntentName(2);
    t += "\n";

    tmp = profile.profil()->getTagText (_item)[0].c_str();
    if(tmp)
      tmp = strchr( tmp, '\n' );
    if(tmp)
    {
      tmp++;
      t += tmp;
    }
    icc_betrachter->mft_text->hinein ( t );
    icc_betrachterNeuzeichnen(icc_betrachter->mft_text);
  } else
  {
    int mft_pos = _mft_item-1;
    oyStructList_s * list = profile.profil()->getTagNumbers(_item);
    oyOption_s * opt;
    oyStructList_s * elements;
    int count = oyStructList_Count( list );
    int list_pos = -1;
    for(int i = 0; i < count; ++i)
    {
      elements = 0;
      opt = (oyOption_s*) oyStructList_GetRefType( list, i, oyOBJECT_OPTION_S );
      if(!opt)
        elements = (oyStructList_s*) oyStructList_GetRefType( list, i,
                                                       oyOBJECT_STRUCT_LIST_S );
      if(opt || elements) ++list_pos;

      if(list_pos == mft_pos)
      {
        if(opt && oyFilterRegistrationMatchKey( oyOption_GetRegistration(opt),
                                       "icParametricCurveType", oyOBJECT_NONE ))
        {
          n = oyOption_GetValueDouble( opt, 0 );
          n = 0;
        } else if(elements)
        {
          oyStructList_s * element;
          oyOption_s * element_data;
          const char * element_name;
          ICClist<ICClist<double> > curves;
          ICClist<std::string>      texts;

          n = oyStructList_Count( elements );
          t = "found ";
          sprintf( num, "%d", n );
          t += num;
          t += " elements\n";
          for(int j = 0; j < n; ++j)
          {
            if(j != 0) t += "\n";

            element = (oyStructList_s*) oyStructList_GetRefType( elements, j,
                                             oyOBJECT_STRUCT_LIST_S );
            element_name = oyStructList_GetName( element, 0 );
            if(element_name)
            t += element_name;
            t += " ";

            element_data = (oyOption_s*) oyStructList_GetRefType( element, 1,
                                             oyOBJECT_OPTION_S );
            if(!element_name)
            {
              element_name = oyOption_GetRegistration( element_data );
              if(element_name)
                t += element_name;
            }

            oyStructList_Release( &element );
            oyOption_Release( &element_data );
          }

          if(curves.size())
          {
            icc_betrachter->mft_viewer->hineinKurven ( curves, texts );
            icc_betrachterNeuzeichnen(icc_betrachter->mft_viewer);
          } else
          {
            icc_betrachter->mft_text->hinein ( t );
            icc_betrachterNeuzeichnen(icc_betrachter->mft_text);
          }
        } else
        {
          t = oyStructList_GetName( list, i-1 );
          t += " ";
          t += strrchr( oyOption_GetRegistration(opt), '/' ) + 1;
          icc_betrachter->mft_text->hinein ( t );
          icc_betrachterNeuzeichnen(icc_betrachter->mft_text);
        }
        break;
      }
      oyOption_Release( &opt );
    }
    if(list_pos != mft_pos)
    {
      sprintf( num, "%d", mft_pos );
      t = "no element found: ";
      t += num;
      icc_betrachter->mft_text->hinein ( t );
      icc_betrachterNeuzeichnen(icc_betrachter->mft_text);
    }
  }
  frei(true);
}

void
ICCexamin::waehleMft (int item)
{ DBG_PROG_START
  ICClist<std::string> TagInfo = profile.profil()->printTagInfo(_item);
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
  ICClist<double> zahlen;

  DBG_PROG_V( _mft_item )

  if(TagInfo[1] == "mAB ")
    showData(_mft_item);
  else
  switch (_mft_item) {
  case 0: // overview
    { ICClist<std::string> Info = icc_betrachter->mft_choice->Info;
      //profile.profil()->getTagText (icc_betrachter->tag_nummer)[0];
      for (unsigned int i = 1; i < Info.size(); i++) // leave out first line
        s << Info [i] << endl;
      icc_betrachter->mft_text->hinein ( s.str() ); DBG_PROG_S("show text")
      icc_betrachterNeuzeichnen(icc_betrachter->mft_text);
    } break;
  case 1: // matrix
    profile.frei(false);
    zahlen = profile.profil()->getTagNumbers (icc_betrachter->tag_nummer, ICCtag::MATRIX);
    profile.frei(true);
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
    profile.frei(false);
    kurven[MFT_VIEWER] =  profile.profil()->getTagCurves (icc_betrachter->tag_nummer, ICCtag::CURVE_IN);
    texte[MFT_VIEWER] = profile.profil()->getTagChannelNames (icc_betrachter->tag_nummer, ICCtag::CURVE_IN);
    profile.frei(true);
    icc_betrachter->mft_viewer->hineinKurven ( kurven[MFT_VIEWER], texte[MFT_VIEWER] );
    icc_betrachterNeuzeichnen(icc_betrachter->mft_viewer);
    DBG_PROG
    break;
  case 3: // 3D table
    DBG_PROG_S("show table")
    {
      ICClist<int> channels;
      profile.frei(false);
      ICClist<std::string> nach_farb_namen = profile.profil()->getTagChannelNames (icc_betrachter->tag_nummer, ICCtag::TABLE_OUT);
      ICClist<std::string> from_colour_names = profile.profil()->getTagChannelNames (icc_betrachter->tag_nummer, ICCtag::TABLE_IN);
      icColorSpaceSignature sig_out = profile.profil()->getTag( icc_betrachter->tag_nummer ).colorSpace( ICCtag::TABLE_OUT );
      icColorSpaceSignature sig_in = profile.profil()->getTag( icc_betrachter->tag_nummer ).colorSpace( ICCtag::TABLE_IN );
      ICClist<std::string> nach_farb_snamen =  getChannelNamesShort( sig_out );
      ICClist<std::string> from_colour_snames =  getChannelNamesShort( sig_in );

      int      n = (int)nach_farb_namen.size();
      if(n != (int)nach_farb_snamen.size())
      {
        sig_out = getColorSpaceGeneric( n );
        nach_farb_snamen =  getChannelNamesShort( sig_out );
      }

      channels = icc_examin->icc_betrachter->mft_gl->channels();
      int c_n = channels.size();
      channels.resize(from_colour_names.size());
      for(unsigned int i = c_n; i < channels.size(); ++i)
        channels[i] = 0;
      ICClist<double> clutpoints =  profile.profil()->getTagNumbers(
                                                    icc_betrachter->tag_nummer,
                                                    ICCtag::TABLE );
      for(unsigned int i = 0; i < from_colour_names.size(); ++i)
        if(i < 3)
          channels[i] = -1;
        else
          if(channels[i] >= clutpoints[0])
            channels[i] = (int)clutpoints[0] / 2;

      if(n == (int)nach_farb_snamen.size())
      {
        const char ** pp = new const char* [n];
        const char ** sn = new const char* [n];
        for (int i = 0; i < n; i++)
        {
          pp[i] = nach_farb_namen[i].c_str();
          sn[i] = nach_farb_snamen[i].c_str();
        }
        icc_examin->icc_betrachter->mft_gl_boxAdd( sn, pp, n, icc_betrachter->mft_gl->kanal );
        if(pp) delete [] pp;
        if(sn) delete [] sn;


        int      n = (int)from_colour_names.size();
        pp = new const char* [n];
        sn = new const char* [n];
        for (int i = 0; i < n; i++)
        {
          pp[i] = (const char*)strdup(from_colour_names[i].c_str());
          sn[i] = (const char*)strdup(from_colour_snames[i].c_str());
        }
        if(clutpoints.size())
          icc_examin->icc_betrachter->mft_gl_sliderAdd( pp, sn, channels,
                                                        (int)clutpoints[0] );
        /* We need to keep that around to make FLTK happy */
        /* if(pp) delete [] pp;
        if(sn) delete [] sn;*/

      } else {
        icc_examin->icc_betrachter->mft_gl_boxAdd( NULL, NULL, 0, 0 );
      }

      icc_betrachter->mft_gl->hineinTabelle (
                     profile.profil()->getTagTable (icc_betrachter->tag_nummer,
                                                    ICCtag::TABLE, channels ),
                     from_colour_names,
                     nach_farb_namen, channels ); DBG_PROG_S( "3D table" )
      profile.frei(true);

    }
    icc_betrachterNeuzeichnen(icc_betrachter->mft_gl_group);
    break;
  case 4: // output curves
    DBG_PROG_S("show curves")
    profile.frei(false);
    kurven[MFT_VIEWER] = profile.profil()->getTagCurves (icc_betrachter->tag_nummer, ICCtag::CURVE_OUT);
    texte[MFT_VIEWER] = profile.profil()->getTagChannelNames (icc_betrachter->tag_nummer, ICCtag::CURVE_OUT);
    profile.frei(true);
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

void
ICCexamin::mftChannel ( int channel, int clutplane )
{
    {
      ICClist<int> channels;
      profile.frei(false);
      ICClist<std::string> from_colour_names = profile.profil()->getTagChannelNames (icc_betrachter->tag_nummer, ICCtag::TABLE_IN);
      icColorSpaceSignature sig_out = profile.profil()->getTag( icc_betrachter->tag_nummer ).colorSpace( ICCtag::TABLE_OUT );
      ICClist<std::string> to_colour_names =  getChannelNames( sig_out );


      channels = icc_examin->icc_betrachter->mft_gl->channels();
      channels[channel] = clutplane;

      icc_betrachter->mft_gl->hineinTabelle (
                     profile.profil()->getTagTable (icc_betrachter->tag_nummer,
                                                    ICCtag::TABLE, channels ),
                     from_colour_names,
                     to_colour_names, channels );
      profile.frei(true);
    }
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

    profile.frei(false);
    ICClist<std::string> TagInfo = profile.profil()->printTagInfo(item);
    profile.frei(true);
    if( TagInfo.size() == 2 )
    {
      ICClist<double> v;
      std::string name;
      ICClist<double> lab;
      double l[3];
      double c[32];
      double XYZ[3];
      oyNamedColour_s * colour = 0;
      oyProfile_s * prof = 0;

      profile.frei(false);
      if(profile.profil()->tagBelongsToMeasurement(item) &&
         icc_examin->icc_betrachter->tag_browser->value() > 5)
      {
        colour = profile.profil()->getMeasurement().getPatchLine( i-1,
                                                        TagInfo[0].c_str() );
        if(colour)
        {
          icc_examin->icc_betrachter->DD_farbraum->emphasizePoint( colour );
          name = oyNamedColour_GetName( colour, oyNAME_NICK, 0 );

          // very simple approach, but enough to see the line
          icc_examin->icc_betrachter->inspekt_html->topline( name.c_str() );

          DBG_PROG_S( txt <<" "<< TagInfo[0] <<" "<< TagInfo[1] <<" L "<< lab[0] <<" a "<< lab[1] <<" b "<< lab[2] )

          oyNamedColour_Release( &colour );

        } 

      } else if( profile.profil()->hasTagName("ncl2") &&
                 TagInfo[0] == "ncl2" ) {
          if( icc_examin->icc_betrachter->tag_text->value() > 5 )
          {
            ICClist<std::string> names;
            icc_examin->farbenLese(-(i-5), lab,v,names);
            name = names.size() ? names[0] : 0;
            if( lab.size() )
            {
              for(unsigned int i = 0; i < 3; ++i) l[i] = lab[i];
              for(unsigned int i = 0; i < v.size() && i < 32; ++i) c[i] = v[i];

              LabToCIELab( l, l, 1 );
              oyLab2XYZ( l, XYZ );

              prof = profile.profil()->oyProfile();
              colour = oyNamedColour_CreateWithName( name.c_str(), NULL, NULL,
                                         c, XYZ, NULL,0, prof, 0 );
              oyProfile_Release( &prof );

              icc_examin->icc_betrachter->DD_farbraum->emphasizePoint( colour );
              oyNamedColour_Release( &colour );

            } else
              icc_examin->icc_betrachter->DD_farbraum->emphasizePoint( NULL );
          } else {
            icc_examin->icc_betrachter->DD_farbraum->emphasizePoint( NULL );
          }
      }
      profile.frei(true);
    }
  }

  return txt;
}

// show the psid profile in the status bar
const char *
selecTpsidLine( int * line )
{
  int i = 0;
  const char * txt = "--";

  if(line && *line > 1)
  {
    i = *line;
    txt = icc_examin->icc_betrachter->tag_text->text(i);

    status(_("Loading .."))  Fl::flush();

    profile.frei(false);
      oyProfile_s * prof = profile.profil()->oyProfile();
      const char * name = oyProfile_GetFileName(prof, (i-2)/5);
      if(name)
        status(name)
      else
        status(_("not available"))
      oyProfile_Release( &prof );
    profile.frei(true);
  }

  return txt;
}

