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
 * the central class. colour space parts
 * 
 */


#include "icc_examin.h"
#include "icc_betrachter.h"
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

#ifdef DEBUG_
#define MARK(x) DBG_S( #x ) x
#else
#define MARK(x) x
#endif


void
ICCexamin::messwertLese (int n,
                         oyNamedColours_s ** list)
{
  DBG_PROG_START
  if(profile.size() > n &&
     profile[n]->hasMeasurement() )
    { DBG_NUM_S( "nutze Messdaten" )
      ICCmeasurement messung = profile[n]->getMeasurement();

      if(messung.validHalf() && profile[n]->size())
        icc_betrachter->DD_farbraum->show_points_as_pairs = true;
      else
        icc_betrachter->DD_farbraum->show_points_as_pairs = false;

        DBG_NUM_V( icc_betrachter->DD_farbraum->show_points_as_pairs )
      icc_betrachter->DD_farbraum->show_points_as_measurements = true;
        DBG_NUM_V( icc_betrachter->DD_farbraum->show_points_as_measurements )

      if(profile.profil()->data_type == ICCprofile::ICCmeasurementDATA || 
         profile.profil()->hasTagName("ncl2") )
        icc_betrachter->DD_farbraum->show_points_as_measurements = true;
      else
        icc_betrachter->DD_farbraum->show_points_as_measurements = false;

        DBG_NUM_V( icc_betrachter->DD_farbraum->show_points_as_measurements )

      unsigned int j;
      unsigned int n_farben = messung.getPatchCount(); DBG_PROG_V( messung.getPatchCount() )
      oyNamedColour_s * c = 0;
      oyNamedColours_s * nl = 0;

      if(messung.validHalf())
      {
        for (j = 0; j < n_farben; ++j)
        { // first the measurments ...
          if(messung.hasXYZ() || messung.hasLab())
            c = messung.getMessColour(j);
          else
            c = messung.getCmmColour(j);

          if (c)
            nl = oyNamedColours_MoveIn( nl, &c, -1 );
        }

        unsigned int n_old = oyNamedColours_Count( *list );

        if(n_old != n_farben * 2)
          oyNamedColours_Release( list );

        if(icc_betrachter->DD_farbraum->show_points_as_pairs)
        {
          if(n_old == n_farben * 2 && *list)
          {
            for(unsigned int i = 0; i < n_farben; ++i)
            {
              if(profile[n]->data_type == ICCprofile::ICCmeasurementDATA)
                c = oyNamedColours_Get( *list, i );
              else
                c = messung.getCmmColour(i);

              if(c)
                nl = oyNamedColours_MoveIn( nl, &c, -1 );
            }
          }
          else
          {
            for(unsigned int i = 0; i < n_farben; ++i)
            {
              if(profile[n]->data_type == ICCprofile::ICCmeasurementDATA)
                c = oyNamedColours_Get( nl, i );
              else
                c = messung.getCmmColour(i);

              if(c)
                nl = oyNamedColours_MoveIn( nl, &c, -1 );
            }
          }
        }

        oyNamedColours_Release( list );

        *list = nl; nl = 0;

      }
    }

  DBG_PROG_ENDE
}

void
ICCexamin::netzLese (int n,
                     icc_examin_ns::ICCThreadList<ICCnetz> *netz)
{
  DBG_PROG_START
  if( profile.size() == 0 && farbraumModus()) {
    netz->resize(1);
    return;
  }

  ICCnetz & netz_temp = (*netz)[n];

  
  {
    int intent = intentGet(NULL);
    oyOptions_s * opts = oyOptions_FromBoolean( options(), NULL,
                                                oyBOOLEAN_UNION, NULL );
    if(farbraumModus())
      intent = profile[n]->intent();
    if( intent < 0 || 3 < intent ) /* check range */
      intent = 3;

    char num[4];
    sprintf(num, "%d", intent);
    oyOptions_SetFromText( &opts, "rendering_intent", num, OY_CREATE_NEW );

    if(profile[n]->data_type == ICCprofile::ICCprofileDATA)
      icc_oyranos.netzVonProfil( *(profile[n]), opts, nativeGamut(),
                                 netz_temp );
    oyOptions_Release( &opts );

    if(netz_temp.punkte.size())
    {
      netz_temp.name = profile[n]->filename();
      // extract the file name
      std::string & dateiname = netz_temp.name;
      if( dateiname.find_last_of(ICC_DIR_SEPARATOR) != std::string::npos)
        dateiname = dateiname.substr( dateiname.find_last_of(ICC_DIR_SEPARATOR)+1,
                                    dateiname.size() );
      DBG_NUM_V( netz_temp.undurchsicht <<" "<< netz_temp.umriss.size() )
    }
  }
  /*for(int i = 0; i < (int)netz->size(); ++i)
    DBG_PROG_V( (*netz)[n].aktiv <<" "<< (*netz)[i].undurchsicht <<" "<< (*netz)[i].umriss.size() );*/
  DBG_PROG_ENDE
}

/** 
   @param[in]  n       < 0 - single patch; >= 0 all patches from profile n
   @param[out] p       lab positions
   @param[out] f       colour
   @param[out] names   according names
   @return             nothing
 */
void
ICCexamin::farbenLese  ( int n,
                         ICClist<double> & p,
                         ICClist<double>  & f,
                         ICClist<std::string> & names )
{
  DBG_PROG_START
  // show named colours

  ICCprofile * pr = NULL;
  int single = (n < 0);
  int patch = single ? -n - 1 : -1;
  int s;

  if( single )
    pr = profile.profil();
  else
    pr = profile[n];

  if( (n >= 0 && profile.size() > n) ||
      single )
  {
    DBG_PROG

    int item = pr->getTagIDByName("ncl2");
    ICClist<double> p_neu = pr->getTagNumbers (item, ICCtag::MATRIX);
    int channels_n = p_neu[1]+0.5;
    unsigned int n_farben = p_neu.size()/(3+channels_n);

    if(n_farben != p_neu[0])
      WARN_S( "wrong patch count" << n_farben<<">"<< p_neu[0] )

    names = pr->getTag(item).getText("ncl2_names");

    int mult = 1;
    int neu = 0;

    if( single )
    {
      if( patch > (int)p_neu.size() )
        WARN_S( "inadequate patch requested" << patch<<">"<< p_neu.size() )
      n_farben = 1;
    }

    if(n_farben != p.size() / 3 / mult)
    {
      DBG_PROG_S( "resize " << n_farben <<" "<<
              (p_neu.size()-2) / 3 <<" "<< p.size() / 3 / mult )
      p.resize( n_farben * 3 * mult );
      f.resize( n_farben * channels_n * mult );
      neu = 1;
    }

    if( single )
    {
      p[0] = p_neu[patch*3+2+0];
      p[1] = p_neu[patch*3+2+1];
      p[2] = p_neu[patch*3+2+2];

      names[0] = names[patch];
      names.resize(1);

    } else {

      for(size_t i = 0; i < n_farben; ++i)
      {
        s = i*mult*3;
        if(mult == 2 && !neu)
        {
          // copy old colours
          p[s+3] = p[s+0];
          p[s+4] = p[s+1];
          p[s+5] = p[s+2];
        }
        p[s+0] = p_neu[i*3+2+0];
        p[s+1] = p_neu[i*3+2+1];
        p[s+2] = p_neu[i*3+2+2];
        if(mult == 2 && neu)
        {
          // double of new colours
          p[s+3] = p[s+0];
          p[s+4] = p[s+1];
          p[s+5] = p[s+2];
        }
      }
    }

    if(channels_n && mult &&
       n_farben != f.size() / channels_n / mult)
      neu = 1;
    else
      neu = 0;

    DBG_NUM_V( f.size() )
    for(unsigned i = 0; i < n_farben; ++i)
    {
      s = i*mult*channels_n;
      if(mult == 2 && !neu)
        for(int j = 0; j < channels_n; ++j)
          f[s+channels_n + j] = f[s + j];

      for(int j = 0; j < channels_n; ++j)
        f[s + j] = p_neu[3*n_farben + i*channels_n +2+ j];

      if(mult == 2 && neu)
        for(int j = 0; j < channels_n; ++j)
          f[s + channels_n + j] = f[i + j];
    }
  }

  DBG_PROG_ENDE
}

void
ICCexamin::farbenLese  ( int n,
                         oyNamedColours_s ** list )
{
  DBG_PROG_START
  int single = (n < 0);
  oyNamedColours_s * nl = 0;

  ICClist<double> p;
  ICClist<double>  f;  // colour
  ICClist<std::string> names;
  oyNamedColour_s * c = 0;
  oyProfile_s * prof = oyProfile_FromFile( profile[n]->filename(), 0, 0 );
  int channels_n = oyProfile_GetChannelsCount( prof );


  farbenLese(n, p, f, names);
  unsigned int n_farben = p.size()/3;
  unsigned int n_old = oyNamedColours_Count( *list );

  if(n_old != n_farben * 2)
    oyNamedColours_Release( list );

  if( !single )
  {
    icc_betrachter->DD_farbraum->show_points_as_measurements = true;
    icc_betrachter->DD_farbraum->show_points_as_pairs = true;
  } else {
    n_farben = 1;
  }

  for (unsigned int i = 0; i < n_farben; ++i)
  {
    double XYZ[3];
    double cielab[3];
    const char * name = 0;
    unsigned int names_n = names.size();
    double * channels = NULL;
    if(channels_n*i < f.size())
      channels = &f[channels_n*i];

    if(i >= names_n)
    {
      if(names_n*2 == n_farben)
        name = names[i-names_n].c_str();
    } else
      name = names[i].c_str();

    LabToCIELab( &p[3*i], cielab, 1 );
    oyLab2XYZ( cielab, XYZ );

    c = oyNamedColour_CreateWithName( name,0,0, channels, XYZ, 0,0, prof, 0 );

    if(c)
      nl = oyNamedColours_MoveIn( nl, &c, -1 );
  }
  oyProfile_Release( &prof );

  if(n_old == n_farben * 2 && *list)
    for(unsigned int i = 0; i < n_farben; ++i)
    {
      c = oyNamedColours_Get( *list, i );
      if(c)
        nl = oyNamedColours_MoveIn( nl, &c, -1 );
    }
  else
    for(unsigned int i = 0; i < n_farben; ++i)
    {
      c = oyNamedColours_Get( nl, i );
      if(c)
        nl = oyNamedColours_MoveIn( nl, &c, -1 );
    }

  oyNamedColours_Release( list );

  *list = nl;

  DBG_PROG_ENDE
}

void
ICCexamin::farbraum (int n)
{
  DBG_PROG_START

  ICClist<std::string> texte, namen;

  texte.push_back(_("CIE *L"));
  texte.push_back(_("CIE *a"));
  texte.push_back(_("CIE *b"));

  oyStructList_s * namedColours = icc_betrachter->DD_farbraum->namedColours();
  oyNamedColours_s * ncl = 0;
  if(oyStructList_Count( namedColours ) > n)
    ncl = (oyNamedColours_s*) oyStructList_GetRefType( namedColours, n,
                                   oyOBJECT_NAMED_COLOURS_S );

  DBG_PROG_V( n <<" "<< profile.size()<<" "<<profile.aktuell() )
  DBG_PROG_V( profile[n]->filename() )

  // measurements
  int messwerte=false;
  bool has_mess = profile[n]->hasMeasurement();
  MARK( frei(false); )
  if(profile.size() > n &&
     has_mess &&
     !(profile[n]->data_type == ICCprofile::ICCprofileDATA &&
       profile.aktuell() != n))
    {
      DBG_PROG
      messwertLese(n, &ncl);
      messwerte = true;
    }
  MARK( frei(true); )

  int ncl2_profil = profile[n]->hasTagName("ncl2");

  // open
  if(lade())
    farbraumModus( profile.aktuell() );

  // show named colours
  if( profile.size() > n && ncl2_profil )
  {
    DBG_PROG
    farbenLese(n, &ncl );
  }

  bool neues_netz = false;
  if( n >= (int)icc_betrachter->DD_farbraum->triangle_nets.size() )
    neues_netz = true;


  //if((int)icc_betrachter->DD_farbraum->triangle_nets.size() > n)
    //icc_betrachter->DD_farbraum->triangle_nets[n].undurchsicht = 1.0;
  if(oyStructList_Count( namedColours ) > n && !collect_changing_points)
    oyStructList_ReleaseAt( namedColours, n );

  if(ncl)
  {
    if(!namedColours)
      namedColours = oyStructList_New(0);
    oyStructList_MoveIn( namedColours, (oyStruct_s**)&ncl, n, 0 );
  }

  icc_betrachter->DD_farbraum->namedColours( namedColours );
  oyStructList_Release( &namedColours );


  do {
    icc_examin_ns::sleep(0.05);
  } while(!icc_betrachter->DD_farbraum->frei());

  if(profile[0]->data_type != ICCprofile::ICCvrmlDATA)
  {
  //MARK( icc_betrachter->DD_farbraum->frei(false); )

    icc_betrachter->DD_farbraum->triangle_nets.frei(false);
    if((int)icc_betrachter->DD_farbraum->triangle_nets .size() <= n)
      icc_betrachter->DD_farbraum->triangle_nets .resize( n + 1 );

    icc_examin_ns::ICCThreadList<ICCnetz> *netze = &icc_betrachter->DD_farbraum->triangle_nets;
    DBG_PROG_V( icc_betrachter->DD_farbraum->triangle_nets.size() <<" "<< n )


    if( profile.size() > n && !ncl2_profil )
    {
      //icc_betrachter->DD_farbraum->frei(true);
      netzLese(n, netze);
      //icc_betrachter->DD_farbraum->frei(false);
    }

    DBG_PROG_V( n <<" "<< netze->size() <<" "<< ncl2_profil )

    // set some standard values
    if(netze->size() && neues_netz)
    {
      if((messwerte || ncl2_profil) && !(*netze)[n].punkte.size())
      {
        (*netze)[n].undurchsicht = 1.0;
        if(n == 0)
          (*netze)[n].grau = false;
        else
          (*netze)[n].grau = true;
      }
      else
      if((n == 0 && ncl2_profil) &&
         !messwerte )
      {
        (*netze)[n].undurchsicht = 0.15;
        (*netze)[n].grau = false;
      }
      else
      if ( farbraumModus() && !messwerte )
      {
        if(n == 1)
          (*netze)[n].undurchsicht = 0.15;
        else
          (*netze)[n].undurchsicht = 0.10;
        (*netze)[n].grau = true;
      }
      else
      if ( n == 0 )
      {
        (*netze)[n].undurchsicht = 0.25;
        (*netze)[n].grau = false;
      }
      else
      {
        (*netze)[n].undurchsicht = 0.3;
        (*netze)[n].grau = true;
      }

      if(profile[n]->filename() == icc_examin->moniName() && 
         netze->size() > 1)
      {
        profile.passiv(n);
        (*netze)[n].active( false );
      }


      icc_betrachter->DD_farbraum->achsNamen( texte );

      DBG_PROG_V( n <<" "<< profile.aktiv(n) )
    }

    if(icc_betrachter->DD_farbraum->triangle_nets[n].name == "")
    {
      icc_betrachter->DD_farbraum->triangle_nets[n].name =
                                                         profile[n]->filename();
      // extract file name
      std::string & dateiname = icc_betrachter->DD_farbraum->triangle_nets[n].name;
      if( dateiname.find_last_of("/") != std::string::npos)
        dateiname = dateiname.substr( dateiname.find_last_of("/")+1,
                                    dateiname.size() );
      DBG_PROG_V( icc_betrachter->DD_farbraum->triangle_nets[n].name )
    }

#if 0
    if( profile.size() > n && !ncl2_profil )
      icc_betrachter->DD_farbraum->hineinNetze(
                                  icc_betrachter->DD_farbraum->triangle_nets );
#endif
    icc_betrachter->DD_farbraum->triangle_nets.frei(true);

  //MARK( icc_betrachter->DD_farbraum->frei(true); )
  }

  DBG_PROG_ENDE
}

void
ICCexamin::farbraum ()
{
  DBG_PROG_START
  MARK( frei(false); )

  MARK( icc_betrachter->DD_farbraum->triangle_nets.frei(false); )
  if((int)icc_betrachter->DD_farbraum->triangle_nets.size() > profile.size())
    icc_betrachter->DD_farbraum->triangle_nets.resize(profile.size());
  MARK( icc_betrachter->DD_farbraum->triangle_nets.frei(true); )
  DBG_PROG_V( icc_betrachter->DD_farbraum->triangle_nets.size() )
  MARK( frei(true); )

  for(int i = 0; i < profile.size(); ++i)
  {
    while(!profile[i] || profile[i]->changing())
      icc_examin_ns::sleep(0.05);
    farbraum(i);
  }
  icc_betrachter->DD_farbraum->clearNet();

  /*if(icc_betrachter->DD_farbraum -> triangle_nets.size())
    icc_betrachter->DD_farbraum ->
      triangle_nets [icc_betrachter->DD_farbraum->triangle_nets.size()-1]
        . undurchsicht = 0.7;*/

  DBG_PROG_V( profile.size() )

  DBG_PROG_ENDE
}

void
ICCexamin::farbraumModus (int profil)
{
  DBG_PROG_START
  MARK( frei(false); )

  farbraum_modus_ = false;
  if(profile.size() && profile.profil()->hasTagName("ncl2"))
  {
    farbraum_modus_ = true;
    static int test = 1;
    if(lade() && test)
      intentNoUpdate( profile.profil()->intent() );
    else
      intent( -1, 0 );

    DBG_PROG_S( "set colour space mode" )
  } else if(!profile.size()) {
    WARN_S( "too early with " << profile.size() << " profiles" )
  }

  DBG_PROG_V( farbraum_modus_ )

  MARK( frei(true); )
  DBG_PROG_ENDE
}


