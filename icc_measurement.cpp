/*
 * ICC Examin ist eine ICC Profil Betrachter
 * 
 * Copyright (C) 2004-2008  Kai-Uwe Behrmann 
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
 * qualtity check from measurements, possibly from inside profile itself
 * 
 */

// Date:      20. 08. 2004

#if 0
# ifndef DEBUG
#  define DEBUG
# endif
# define DEBUG_ICCMEASUREMENT
# define DBG_MESS_START DBG_PROG_START
# define DBG_MESS_ENDE DBG_PROG_ENDE
# define DBG_MESS_V(t) DBG_NUM_V(t)
#else
# define DBG_MESS_START
# define DBG_MESS_ENDE
# define DBG_MESS_V(t)
#endif


#include "icc_utils.h"
#include "icc_profile.h"
#include "icc_oyranos.h"
#include "icc_examin_version.h"
#include "icc_helfer.h"
#include "icc_cgats_filter.h"
#include "icc_info.h"
using namespace icc_examin_ns;


/**
  *  @brief ICCmeasurement functions
  */

int icc_measurement_id_ = 0;

ICCmeasurement::ICCmeasurement (ICCprofile* profil)
{ DBG_PROG_START
  id_ = icc_measurement_id_++;
  DBG_MEM_V( id_ <<" "<< profil )
  defaults();
  profile_ = profil;
  DBG_PROG_ENDE
}

ICCmeasurement::ICCmeasurement (ICCprofile* profil, ICCtag &tag)
{ DBG_PROG_START
  id_ = icc_measurement_id_++;
  DBG_MEM_V( id_ <<" "<< profil )
  ICCmeasurement::load (profil, tag); 
  DBG_PROG_ENDE
}

ICCmeasurement::ICCmeasurement     (const ICCmeasurement& m)
{
  id_ = icc_measurement_id_++;
  DBG_MEM_V( id_ <<" "<< profile_ )
  DBG_PROG
  copy (m);
}

ICCmeasurement::ICCmeasurement     ()
{
  DBG_PROG_START
  WARN_S( "-----------------------  dont use the default constructor --------------------------" )
  id_ = icc_measurement_id_++;
  DBG_MEM_V( id_ )
  defaults();
  DBG_PROG_ENDE
}

ICCmeasurement::~ICCmeasurement ()
{
  DBG_PROG_S("::~ICCmeasurement")
  clear();
  DBG_MEM_S( "::~ICCmeasurement id_ "<< id_ <<" "<< profile_ )
}

void
ICCmeasurement::defaults ()
{
  DBG_PROG_START
  DBG_MEM_V( id_ <<" "<< profile_ )
  sig_ = (icTagSignature)0xFFFFFFFF;
  size_ = 0;
  data_ = NULL;

  nFelder_ = 0;

  channels_ = 0;
  isICCDisplay_ = 0;
  profile_ = NULL;
  XYZ_measurement_ = false;
  LAB_measurement_ = false;
  RGB_measurement_ = false;
  CMYK_measurement_ = false;
  Lab_Differenz_max_ = -1000;
  Lab_Differenz_min_ = 1000;
  Lab_Differenz_Durchschnitt_ = 0;
  DE00_Differenz_max_ = -1000;
  DE00_Differenz_min_ = 1000;
  DE00_Differenz_Durchschnitt_ = 0;
  export_farben = false;
  int i;
  for(i=0;i<6;i+=2) { range_XYZ[i] = 0.f; range_XYZ[i+1] = 100.f; }
  range_Lab[0] = 0.f; range_Lab[1] = 100.f;
  for(i=2;i<6;i+=2) { range_Lab[i] = -128.f; range_Lab[i+1] = 127.f; }
  for(i=0;i<6;i+=2) { range_RGB[i] = 0.f; range_RGB[i+1] = 255.f; }
  for(i=0;i<8;i+=2) { range_CMYK[i] = 0.f; range_CMYK[i+1] = 100.f; }

  XYZmax[0] = XYZmax[1] = XYZmax[2] = -100000;
  XYZmin[0] = XYZmin[1] = XYZmin[2] = 100000;

  XYZWP[0] = X_D50;
  XYZWP[1] = Y_D50;
  XYZWP[2] = Z_D50;
  XYZBP[0] = XYZBP[1] = XYZBP[2] = 0;

  minFeld = -1;
  maxFeld = -1;
  cgats = 0;
  options_ = 0;
  DBG_PROG_ENDE
}

void
ICCmeasurement::copy (const ICCmeasurement& m)
{
  DBG_PROG_START
  DBG_MEM_V( id_ <<" "<< m.id_ <<" "<< m.profile_ )
  sig_ = m.sig_;
  size_ = m.size_;
  if (size_ && m.data_) {
    data_ = (char*)calloc(sizeof(char),size_+1);
    memcpy (data_ , m.data_ , size_);
    DBG_MEM_S((int*)m.data_ << " -> " << (int*)data_)
  } else {
    data_ = NULL;
    size_ = 0;
  }

  nFelder_ = m.nFelder_;
  channels_ = m.channels_;
  isICCDisplay_ = m.isICCDisplay_;
  profile_ = m.profile_;
  LAB_measurement_ = m.LAB_measurement_;
  XYZ_measurement_ = m.XYZ_measurement_;
  RGB_measurement_ = m.RGB_measurement_;
  CMYK_measurement_ = m.CMYK_measurement_;
  // measurments
  XYZ_Satz_ = m.XYZ_Satz_;
  Lab_Satz_ = m.Lab_Satz_;
  RGB_Satz_ = m.RGB_Satz_;
  CMYK_Satz_ = m.CMYK_Satz_;
  // profile values
  Feldnamen_ = m.Feldnamen_;
  XYZ_Ergebnis_ = m.XYZ_Ergebnis_;
  Lab_Ergebnis_ = m.Lab_Ergebnis_;
  RGB_MessFarben_ = m.RGB_MessFarben_;
  RGB_ProfilFarben_ = m.RGB_ProfilFarben_;
  patch_src_lines_ = m.patch_src_lines_;
  // results
  Lab_Differenz_ = m.Lab_Differenz_;
  Lab_Differenz_max_ = m.Lab_Differenz_max_;
  Lab_Differenz_min_ = m.Lab_Differenz_min_;
  Lab_Differenz_Durchschnitt_ = m.Lab_Differenz_Durchschnitt_;
  DE00_Differenz_ = m.DE00_Differenz_;
  DE00_Differenz_max_ = m.DE00_Differenz_max_;
  DE00_Differenz_min_ = m.DE00_Differenz_min_;
  DE00_Differenz_Durchschnitt_ = m.DE00_Differenz_Durchschnitt_;

  memcpy( range_XYZ, m.range_XYZ, 6*sizeof(double) );
  memcpy( range_Lab, m.range_Lab, 6*sizeof(double) );
  memcpy( range_RGB, m.range_RGB, 6*sizeof(double) );
  memcpy( range_CMYK, m.range_CMYK, 8*sizeof(double) );

  memcpy( XYZmax, m.XYZmax, 3*sizeof(double) );
  memcpy( XYZmin, m.XYZmin, 3*sizeof(double) );
  memcpy( XYZWP, m.XYZWP, 3*sizeof(double) );
  memcpy( XYZBP, m.XYZBP, 3*sizeof(double) );
  minFeld = m.minFeld;
  maxFeld = m.maxFeld;

  export_farben = m.export_farben;
  if(m.cgats)
    cgats = new CgatsFilter(*(m.cgats));

  if(m.options_)
    options_ = oyOptions_Copy( m.options_, m.options_->oy_ );
  else
    options_ = 0;
  
  DBG_PROG_ENDE
}

void
ICCmeasurement::clear (void)
{
  DBG_PROG_START
  DBG_MEM_V( id_ <<" "<< profile_ )
  if (data_ != NULL) free(data_);
  defaults();
  XYZ_Satz_.clear();
  Lab_Satz_.clear();
  RGB_Satz_.clear();
  CMYK_Satz_.clear();
  Feldnamen_.clear();
  XYZ_Ergebnis_.clear();
  Lab_Ergebnis_.clear();
  RGB_MessFarben_.clear();
  RGB_ProfilFarben_.clear();
  patch_src_lines_.clear();
  Lab_Differenz_.clear();
  DE00_Differenz_.clear();
  reportTabelle_.clear();
  layout.clear();
  if(cgats) delete cgats;
  oyOptions_Release( &options_ );

  DBG_PROG_ENDE
}

ICCmeasurement &
ICCmeasurement::operator =          (const ICCmeasurement& m)
{
  DBG_PROG copy (m); 
  return *this;
}

void
ICCmeasurement::load                ( ICCprofile *profil,
                                      ICCtag&     tag )
{ DBG_PROG_START
  profile_ = profil;
  if (!profile_) WARN_S( "cant initialise, profile referenz is missed" )

  sig_    = tag._sig;
  if(tag.size_)
    size_   = tag.size_ - 8;
  else
    size_ = 0;
  DBG_PROG_V( size_ )
  // simply exchange
  if (data_ != NULL) { free (data_); data_ = NULL; }
  if(size_)
  {
    data_ = (char*) calloc ( size_+1 , sizeof (char) );
    memcpy ( data_ , &(tag.data_)[8] , size_ );
  }

  DBG_PROG_ENDE
}

void
ICCmeasurement::load                ( ICCprofile *profil,
                                      const char *data,
                                      size_t      size )
{ DBG_PROG_START
  profile_ = profil;
  if (!profile_) WARN_S( "cant initialise, profile referenz is missed" )

  if (sig_ != (icTagSignature)0xFFFFFFFF)
    sig_ = icSigCharTargetTag;

  size_   = size;
  // simply exchange
  if (!data_) free (data_);
  data_ = (char*) calloc ( size_+1 , sizeof (char) );
  memcpy ( data_ , data , size_ );

  DBG_PROG_ENDE
}

void
ICCmeasurement::leseTag (void)
{ DBG_PROG_START

  //DBG_PROG_V( data_ )

  if(!data_ || !size_)
    return;

  cgats->lade( data_, size_ );
  std::string data = cgats->lcms_gefiltert (); DBG_NUM_V( (int*)data_ <<" "<< size_ )

  
  // locale - differentiate commas (Attention: CgatsFilter changes LC_NUMERIC)
  doLocked_m( std::string loc_alt = setlocale(LC_NUMERIC, NULL);,NULL) //getenv("LANG");
  if(loc_alt.size()) {
    DBG_NUM_V( loc_alt )
  } else {
    DBG_NUM_S( "LANG variable not found" )
  }
  doLocked_m( setlocale(LC_NUMERIC,"C");,NULL);

  if(data.size())
  {  
    // correct CGATS data -> data_
    if (data_ != NULL) { free (data_); data_ = NULL; }

    data_ = (char*) calloc (sizeof(char), data.size()+1);
    size_ = data.size();
    memcpy (data_, data.c_str(), size_); DBG_NUM_V( (int*)data_ )

    if(strstr(data.c_str(), "\nDESCRIPTOR \"Argyll Calibration Target chart"))
    {
      range_RGB[0] = range_RGB[2] = range_RGB[4] = 0.f;
      range_RGB[1] = range_RGB[3] = range_RGB[5] = 100.f;
    } else
    if(strstr(data.c_str(), "ORIGINATOR \"Monaco Systems, Inc\""))
    {
      range_RGB[0] = range_RGB[2] = range_RGB[4] = 0.f;
      range_RGB[1] = range_RGB[3] = range_RGB[5] = 1.f;
    } else {
      range_RGB[0] = range_RGB[2] = range_RGB[4] = 0.f;
      range_RGB[1] = range_RGB[3] = range_RGB[5] = 255.f;
    }

#   if 0
    // lcms liest ein
    lcms_parse();
#   else

    //LCMSHANDLE _lcms_it8 = cmsIT8LoadFromMem ( data_, size_ ); DBG_MEM_V( (int*)data_)

    int ps = patch_src_lines_.size();
    patch_src_lines_.resize( ps + 1 );
    patch_src_lines_[ps].first = getSigTagName( sig_ );

    const char **SampleNames = 0;
    int m = 0; // actual measurement
    int m_n = cgats->messungen.size();
    int _nKanaele = 0;
    bool _sample_name = false;
    bool _sample_id = false;
    bool _id_vor_name = false;

    if(cgats && cgats->messungen.size() && cgats->messungen[m].felder.size())
      _nKanaele = (int)cgats->messungen[m].felder[0].size();

    for(m = 0; m < m_n; ++m)
    {
      int n   = cgats->messungen[m].block_zeilen,
          start = nFelder_,
          end = nFelder_ + n;

      if(cgats->messungen[m].felder.size() != 1)
      {
        WARN_S( "There are unadequate field declarations: "
                << cgats->messungen[m].felder.size() )
      }

      // we want valid patchsets only
      // heuristic: skip patches sets with fewer patches
      if(m > 0 && n &&
         _nKanaele >  (int)cgats->messungen[m].felder[0].size())
        continue;

      // measurement spot number
      if (nFelder_ == 0 ||
          nFelder_ ||
          (m_n > 1) )
      {
        nFelder_ += n;
        patch_src_lines_[ps].second.resize( nFelder_ );

      } else {

        WARN_S( "number of measurements should correspond! " << nFelder_ << "|" << (int)cgats->messungen[m].block_zeilen )
        clear();
        goto finish;
      }

      if(cgats->messungen[m].felder.size())
      {
        _nKanaele = (int)cgats->messungen[m].felder[0].size();
        SampleNames = (const char**) new const char* [_nKanaele];
        for (int i = 0; i < _nKanaele; i++)
          SampleNames[i] = cgats->messungen[m].felder[0][i].c_str();
      }



      // What is all here? Do we want do exchange the names later?
      if(cgats->messungen[m].felder.size())
      {
        _sample_name = false;
        _sample_id = false;
        _id_vor_name = false;
        for (int i = 0; i < _nKanaele; i++)
        {
          if (strstr(cgats->messungen[m].felder[0][i].c_str(),"SAMPLE_ID") != 0)
            _sample_id = true;
          if (strstr(cgats->messungen[m].felder[0][i].c_str(),"SAMPLE_NAME") != 0
           && _sample_id) {
            _sample_name = true;
            _id_vor_name = true;
          }
#   ifdef DEBUG_ICCMEASUREMENT
          DBG_NUM_S( SampleNames[i] << " _sample_name " << _sample_name <<
                " _sample_id" << _sample_id << " _id_vor_name " << _id_vor_name)
#   endif
        }
      }

      // reding and parsing
      ICClist<std::string> farbkanaele;
      // locals !
      bool has_Lab = false;
      bool has_XYZ = false;
      bool has_CMYK = false;
      bool has_RGB = false;
      bool has_xyY = false;
      for (int i = 0; i < _nKanaele; i++) {

        if ((strstr (SampleNames[i], "LAB_L") != 0)
         || (strstr (SampleNames[i], "LAB_A") != 0)
         || (strstr (SampleNames[i], "LAB_B") != 0)) {
          DBG_PROG_S( "Lab data " )
          has_Lab = true;
          farbkanaele.push_back(SampleNames[i]);
          } else if ((strstr (SampleNames[i], "XYZ_X") != 0)
                || (strstr (SampleNames[i], "XYZ_Y") != 0)
                || (strstr (SampleNames[i], "XYZ_Z") != 0)) {
          DBG_PROG_S( "XYZ data " )
          has_XYZ = true;
          farbkanaele.push_back(SampleNames[i]);
        } else if ((strstr (SampleNames[i], "CMYK_C") != 0)
                || (strstr (SampleNames[i], "CMYK_M") != 0)
                || (strstr (SampleNames[i], "CMYK_Y") != 0)
                || (strstr (SampleNames[i], "CMYK_K") != 0)) {
          DBG_PROG_S( "CMYK data " )
          has_CMYK = true;
          farbkanaele.push_back(SampleNames[i]);
        } else if ((strstr (SampleNames[i], "RGB_R") != 0)
                || (strstr (SampleNames[i], "RGB_G") != 0)
                || (strstr (SampleNames[i], "RGB_B") != 0)) {
          DBG_PROG_S( "RGB data " )
          has_RGB = true;
          farbkanaele.push_back(SampleNames[i]);
        } else if ((strstr (SampleNames[i], "XYY_X") != 0)
                || (strstr (SampleNames[i], "XYY_Y") != 0)
                || (strstr (SampleNames[i], "XYY_CAPY") != 0)) {
          DBG_PROG_S( "xyY data " )
          has_xyY = true;
          farbkanaele.push_back(SampleNames[i]);
        } else {
          farbkanaele.push_back(SampleNames[i]);
        }

      } DBG_PROG

      // variables
      int farben = 0;
      if (has_Lab) {farben++; LAB_measurement_ = true; }
      if (has_XYZ) {farben++; XYZ_measurement_ = true; }
      if (has_RGB) {farben++; RGB_measurement_ = true; }
      if (has_CMYK){farben++; CMYK_measurement_ = true; }
      if (has_xyY) farben++;


      // list lcms colour names
        Feldnamen_.resize(end);
        DBG_PROG_V( nFelder_ )
        for (int k = 0; k < n; k++) {
          if (_id_vor_name
           && (getTagName() != "DevD")) {// ignore names
            char *text = (char*) calloc (sizeof(char), 12);
            sprintf (text, "%d", start+k+1);
            Feldnamen_[start+k] = text;
            free(text);
          } else if(_sample_id || _sample_name) {
            const char *constr = cgats->messungen[m].block[k][0].c_str();
            Feldnamen_[start+k] = constr;
          } else {
            char n[8];
            snprintf(n,8,"%d",start+k+1);
            Feldnamen_[start+k] = n;
          }
          patch_src_lines_[ps].second[start+k] = cgats->messungen[m].line[k];
        }
      if(nFelder_)
        DBG_NUM_S (Feldnamen_[0] << " bis " << Feldnamen_[nFelder_-1])

      DBG_NUM_V( has_XYZ << has_Lab << has_RGB << has_CMYK )

      // read colours
      int c = 0;
      const char * d_text = 0;
      if (has_XYZ)
      { DBG_PROG // no calculation required
        XYZ_Satz_.resize(nFelder_);
        for (int i = 0; i < n; i++)
        {
          if((int)cgats->messungen[m].block.size() > i)
          for (int j = 0; j < _nKanaele; ++j)
          {
            if((int)cgats->messungen[m].block[i].size() <= j)
              continue;

            d_text = cgats->messungen[m].block[i][j].c_str();
            c = 0;
            if( strcmp(SampleNames[j], "XYZ_X") == 0 )
              XYZ_Satz_[start+i].X = (atof(d_text) - range_XYZ[c]) /
                                     (range_XYZ[c+1] - range_XYZ[c]);
            c+=2;
            if( strcmp(SampleNames[j], "XYZ_Y") == 0 )
              XYZ_Satz_[start+i].Y = (atof(d_text) - range_XYZ[c]) /
                                     (range_XYZ[c+1] - range_XYZ[c]);
            c+=2;
            if( strcmp(SampleNames[j], "XYZ_Z") == 0 )
              XYZ_Satz_[start+i].Z = (atof(d_text) - range_XYZ[c]) /
                                     (range_XYZ[c+1] - range_XYZ[c]);
          }
        }
      }
      if (has_Lab)
      {
        DBG_PROG // no calculation required
        Lab_Satz_.resize(nFelder_);
        for (int i = 0; i < n; i++)
        {
          if((int)cgats->messungen[m].block.size() > i)
          for (int j = 0; j < _nKanaele; ++j)
          {
            if((int)cgats->messungen[m].block[i].size() <= j)
              continue;

            d_text = cgats->messungen[m].block[i][j].c_str();
            c = 0;
            if( strcmp(SampleNames[j], "LAB_L") == 0 )
              Lab_Satz_[start+i].L = (atof(d_text) - range_Lab[c]) /
                                     (range_Lab[c+1] - range_Lab[c]);
            c+=2;
            if( strcmp(SampleNames[j], "LAB_A") == 0 )
              Lab_Satz_[start+i].a = (atof(d_text) - range_Lab[c]) /
                                     (range_Lab[c+1] - range_Lab[c]);
            c+=2;
            if( strcmp(SampleNames[j], "LAB_B") == 0 )
              Lab_Satz_[start+i].b = (atof(d_text) - range_Lab[c]) /
                                     (range_Lab[c+1] - range_Lab[c]);
          }
        }
      }
      if (has_RGB) { DBG_PROG // no calculation required
        RGB_Satz_.resize(nFelder_);
        for (int i = 0; i < n; i++)
        {
          if((int)cgats->messungen[m].block.size() > i)
          for (int j = 0; j < _nKanaele; ++j)
          {
            if((int)cgats->messungen[m].block[i].size() <= j)
              continue;

            d_text = cgats->messungen[m].block[i][j].c_str();
            c = 0;
            if( strcmp(SampleNames[j], "RGB_R") == 0 )
              RGB_Satz_[start+i].R = (atof(d_text) - range_RGB[c]) /
                                     (range_RGB[c+1] - range_RGB[c]);
            c+=2;
            if( strcmp(SampleNames[j], "RGB_G") == 0 )
              RGB_Satz_[start+i].G = (atof(d_text) - range_RGB[c]) /
                                     (range_RGB[c+1] - range_RGB[c]);
            c+=2;
            if( strcmp(SampleNames[j], "RGB_B") == 0 )
              RGB_Satz_[start+i].B = (atof(d_text) - range_RGB[c]) /
                                     (range_RGB[c+1] - range_RGB[c]);
          }
        }
      }
      if (has_CMYK) { DBG_PROG // no calculation required
        CMYK_Satz_.resize(nFelder_);
        for (int i = 0; i < n; i++)
        {
          if((int)cgats->messungen[m].block.size() > i)
          for (int j = 0; j < _nKanaele; ++j)
          {
            if((int)cgats->messungen[m].block[i].size() <= j)
              continue;

            d_text = cgats->messungen[m].block[i][j].c_str();
            c = 0;
            if( strcmp(SampleNames[j], "CMYK_C") == 0 )
              CMYK_Satz_[start+i].C = (atof(d_text) - range_CMYK[c]) /
                                      (range_CMYK[c+1] - range_CMYK[c]);
            c+=2;
            if( strcmp(SampleNames[j], "CMYK_M") == 0 )
              CMYK_Satz_[start+i].M = (atof(d_text) - range_CMYK[c]) /
                                      (range_CMYK[c+1] - range_CMYK[c]);
            c+=2;
            if( strcmp(SampleNames[j], "CMYK_Y") == 0 )
              CMYK_Satz_[start+i].Y = (atof(d_text) - range_CMYK[c]) /
                                      (range_CMYK[c+1] - range_CMYK[c]);
            c+=2;
            if( strcmp(SampleNames[j], "CMYK_K") == 0 )
              CMYK_Satz_[start+i].K = (atof(d_text) - range_CMYK[c]) /
                                      (range_CMYK[c+1] - range_CMYK[c]);
          }
        } DBG_PROG
      }


      // colour names of taste
      if (_id_vor_name) {
        for (int i = 0; i < n; i++) {
          for (int j = 0; j < (int)cgats->messungen[m].block[i].size(); ++j) {
            if( strcmp(SampleNames[j], "SAMPLE_NAME") == 0 )
              Feldnamen_[start+i] = cgats->messungen[m].block[i][j].c_str();
          }
        } DBG_NUM_S (Feldnamen_[0] <<" to "<< Feldnamen_[nFelder_-1] <<" "<< nFelder_)
      }

    }

    DBG_NUM_V( XYZ_Satz_.size() )
    DBG_NUM_V( RGB_Satz_.size() )
    DBG_NUM_V( CMYK_Satz_.size() )

  #   endif
  }

  finish:
  if(loc_alt.size())
    doLocked_m( setlocale(LC_NUMERIC,loc_alt.c_str()) , NULL);

  DBG_PROG_ENDE
}

void
ICCmeasurement::init (void)
{ DBG_PROG_START DBG_MEM_V( (int*)data_ )
  if (valid())
    return;

  if( profile_->data_type == ICCprofile::ICCcorruptedprofileDATA )
    return;

  if(!cgats)
    cgats = new CgatsFilter;

  if (!profile_) WARN_S( "cant initialise, profile referenz missed; id: "<<id_<<" profile: "<< (int*)profile_ )

  if (profile_->hasTagName("targ")) {
    load (profile_, profile_->getTag(profile_->getTagIDByName("targ")));
    leseTag ();
  }
  else if (profile_->hasTagName("b015")) {
    load (profile_, profile_->getTag(profile_->getTagIDByName("b015")));
    leseTag ();
  }
  else if (profile_->hasTagName("TGL2")) {
    load (profile_, profile_->getTag(profile_->getTagIDByName("TGL2")));
    leseTag ();
  }
  else if (profile_->hasTagName("DevD") && (profile_->hasTagName("CIED"))) {
    load (profile_, profile_->getTag(profile_->getTagIDByName("DevD")));
    leseTag ();
    
    // We start again at position zero.
    nFelder_ = 0;
    load (profile_, profile_->getTag(profile_->getTagIDByName("CIED")));
    leseTag ();
  }

  if (RGB_MessFarben_.size() != 0)
    DBG_NUM_V( RGB_MessFarben_.size() );

  if (profile_ &&
      profile_->data_type == ICCprofile::ICCprofileDATA )
  {
    channels_ = profile_->getColourChannelsCount();
    isICCDisplay_ = /*!(profile_->hasCLUT()) &&*/
                profile_->getHeader().deviceClass() == icSigDisplayClass;
  }

  init_umrechnen();
  DBG_PROG_ENDE
}

#if 0
void
ICCmeasurement::lcms_parse                   (void)
{ DBG_PROG_START
  LCMSHANDLE _lcms_it8 = cmsIT8LoadFromMem ( data_, size_ ); DBG_MEM_V( (int*)data_)

  char **SampleNames; DBG_MEM

  // Messfeldanzahl
  if (nFelder_ == 0
   || nFelder_ == (int)cmsIT8GetPropertyDbl(_lcms_it8, "NUMBER_OF_SETS")) { DBG_NUM
    nFelder_ = (int)cmsIT8GetPropertyDbl(_lcms_it8, "NUMBER_OF_SETS"); DBG_NUM
  } else {
    WARN_S( "Messfeldanzahl sollte schon uebereinstimmen! " << nFelder_ << "|" << (int)cmsIT8GetPropertyDbl(_lcms_it8, "NUMBER_OF_SETS") )
    clear();
    return;
  }

  int _nKanaele = cmsIT8EnumDataFormat(_lcms_it8, &SampleNames);
  bool _sample_name = false;
  bool _sample_id = false;
  bool _id_vor_name = false;

  // Was ist alles da? Wollen wir spaeter die Namen tauschen?
  for (int i = 0; i < _nKanaele; i++) {
    if (strstr((char*)SampleNames[i],"SAMPLE_ID") != 0)
      _sample_id = true;
    if (strstr((char*)SampleNames[i],"SAMPLE_NAME") != 0
     && _sample_id) {
      _sample_name = true;
      _id_vor_name = true;
    }
#   ifdef DEBUG_ICCMEASUREMENT
    DBG_NUM_S( (char*)SampleNames[i] << " _sample_name " << _sample_name <<
           " _sample_id" << _sample_id << " _id_vor_name " << _id_vor_name) 
#   endif
  }

  // Auslesen und Aufbereiten
  ICClist<std::string> farbkanaele;
  // muessen lokal bleiben !
  bool has_Lab = false;
  bool has_XYZ = false;
  bool has_CMYK = false;
  bool has_RGB = false;
  bool has_xyY = false;
  for (int i = 0; i < _nKanaele; i++) {

    if ((strstr (SampleNames[i], "LAB_L") != 0)
     || (strstr (SampleNames[i], "LAB_A") != 0)
     || (strstr (SampleNames[i], "LAB_B") != 0)) {
      cout << "Lab Daten ";
      has_Lab = true;
      farbkanaele.push_back(SampleNames[i]);
    } else if ((strstr (SampleNames[i], "XYZ_X") != 0)
            || (strstr (SampleNames[i], "XYZ_Y") != 0)
            || (strstr (SampleNames[i], "XYZ_Z") != 0)) {
      cout << "XYZ Daten ";
      has_XYZ = true;
      farbkanaele.push_back(SampleNames[i]);
    } else if ((strstr (SampleNames[i], "CMYK_C") != 0)
            || (strstr (SampleNames[i], "CMYK_M") != 0)
            || (strstr (SampleNames[i], "CMYK_Y") != 0)
            || (strstr (SampleNames[i], "CMYK_K") != 0)) {
      cout << "CMYK Daten ";
      has_CMYK = true;
      farbkanaele.push_back(SampleNames[i]);
    } else if ((strstr (SampleNames[i], "RGB_R") != 0)
            || (strstr (SampleNames[i], "RGB_G") != 0)
            || (strstr (SampleNames[i], "RGB_B") != 0)) {
      cout << "RGB Daten ";
      has_RGB = true;
      farbkanaele.push_back(SampleNames[i]);
    } else if ((strstr (SampleNames[i], "XYY_X") != 0)
            || (strstr (SampleNames[i], "XYY_Y") != 0)
            || (strstr (SampleNames[i], "XYY_CAPY") != 0)) {
      cout << "xyY Daten ";
      has_xyY = true;
      farbkanaele.push_back(SampleNames[i]);
    } else {
      farbkanaele.push_back(SampleNames[i]);
    }

  } DBG_PROG

  // Variablen
  int farben = 0;
  if (has_Lab) farben++;
  if (has_XYZ) {farben++; XYZ_measurement_ = true; }
  if (has_RGB) {farben++; RGB_measurement_ = true; }
  if (has_CMYK) {farben++; CMYK_measurement_ = true; }
  if (has_xyY) farben++;


  // vorlaeufige lcms Farbnamen listen
    Feldnamen_.resize(nFelder_);
    DBG_PROG_V( nFelder_ )
    for (int k = 0; k < nFelder_; k++) {
      if (_id_vor_name
       && (getTagName() != "DevD")) {// Name ignorieren
        char *text = (char*) calloc (sizeof(char), 12);
        sprintf (text, "%d", k+1);
        Feldnamen_[k] = text;
        free(text);
      } else {
        const char *constr = cmsIT8GetPatchName (_lcms_it8, k, NULL);
        Feldnamen_[k] = constr;
      }
    }
  if(nFelder_)
    DBG_NUM_S (Feldnamen_[0] << " bis " << Feldnamen_[nFelder_-1])

  DBG_NUM_V( has_XYZ << has_RGB << has_CMYK )
 
  // Farben auslesen
  if (has_XYZ) { DBG_PROG // keine Umrechnung noetig
    XYZ_Satz_.resize(nFelder_);
    for (int i = 0; i < nFelder_; i++) {
        XYZ_Satz_[i].X = cmsIT8GetDataDbl (_lcms_it8, Feldnamen_[i].c_str(),
                                           "XYZ_X") / 100.0;
        XYZ_Satz_[i].Y = cmsIT8GetDataDbl (_lcms_it8, Feldnamen_[i].c_str(),
                                           "XYZ_Y") / 100.0;
        XYZ_Satz_[i].Z = cmsIT8GetDataDbl (_lcms_it8, Feldnamen_[i].c_str(),
                                           "XYZ_Z") / 100.0;
    }
  }
  if (has_RGB) { DBG_PROG // keine Umrechnung noetig
    RGB_Satz_.resize(nFelder_);
    for (int i = 0; i < nFelder_; i++) {
        RGB_Satz_[i].R = cmsIT8GetDataDbl (_lcms_it8, Feldnamen_[i].c_str(),
                                           "RGB_R") / 255.0;
        RGB_Satz_[i].G = cmsIT8GetDataDbl (_lcms_it8, Feldnamen_[i].c_str(),
                                           "RGB_G") / 255.0;
        RGB_Satz_[i].B = cmsIT8GetDataDbl (_lcms_it8, Feldnamen_[i].c_str(),
                                           "RGB_B") / 255.0;
    }
  }
  if (has_CMYK) { DBG_PROG // keine Umrechnung noetig
    CMYK_Satz_.resize(nFelder_);
    for (int i = 0; i < nFelder_; i++) {
        CMYK_Satz_[i].C = cmsIT8GetDataDbl (_lcms_it8, Feldnamen_[i].c_str(),
                                           "CMYK_C") /100.0;
        CMYK_Satz_[i].M = cmsIT8GetDataDbl (_lcms_it8, Feldnamen_[i].c_str(),
                                           "CMYK_M") /100.0;
        CMYK_Satz_[i].Y = cmsIT8GetDataDbl (_lcms_it8, Feldnamen_[i].c_str(),
                                           "CMYK_Y") /100.0;
        CMYK_Satz_[i].K = cmsIT8GetDataDbl (_lcms_it8, Feldnamen_[i].c_str(),
                                           "CMYK_K") /100.0;
    } DBG_PROG
  }


  // Farbnamen nach Geschmack (unmittelbar vor cmsIT8Free !)
  if (_id_vor_name) {
    for (int i = 0; i < nFelder_; i++) {
        Feldnamen_[i] = cmsIT8GetData (_lcms_it8, Feldnamen_[i].c_str(),
                                       "SAMPLE_NAME");
    } DBG_NUM_S (Feldnamen_[0] <<" bis "<< Feldnamen_[nFelder_-1] <<" "<< nFelder_)
  }

  // lcms's cgats Leser wird nicht mehr gebraucht
  cmsIT8Free (_lcms_it8);
  _lcms_it8 = NULL;
  DBG_NUM_V( XYZ_Satz_.size() )
  DBG_NUM_V( RGB_Satz_.size() )
  DBG_NUM_V( CMYK_Satz_.size() )
  DBG_PROG_ENDE
}
#endif

void
ICCmeasurement::init_umrechnen                     (void)
{ DBG_PROG_START

  if( profile_->data_type == ICCprofile::ICCcorruptedprofileDATA )
    return;

  Lab_Differenz_max_ = -1000.0;
  Lab_Differenz_min_ = 1000.0;
  Lab_Differenz_Durchschnitt_ = 0.0;
  DE00_Differenz_max_ = -1000.0;
  DE00_Differenz_min_ = 1000.0;
  DE00_Differenz_Durchschnitt_ = 0.0;
# define PRECALC cmsFLAGS_NOTPRECALC // No memory overhead, VERY
                                      // SLOW ON TRANSFORMING, very fast on creating transform.
                                      // Maximum accurancy.

  XYZmax[0] = XYZmax[1] = XYZmax[2] = -100000;
  XYZmin[0] = XYZmin[1] = XYZmin[2] = 100000;
  { int maxFeld=0, minFeld=0;
    const char *maxFN=0, *minFN=0;
    if (nFelder_ != (int)XYZ_Satz_.size()) {
      DBG_PROG_S("divergine measurement count");
    }

    int m = nFelder_ < (int)XYZ_Satz_.size() ? nFelder_ : (int)XYZ_Satz_.size();
    if(!m && isICCDisplay_)
      WARN_S("No XYZ data available. Dont support this display profile?");
    DBG_PROG_S( "fields: " << m )

    for (int i = 0; i < m; i++)
    { 
      if (XYZmax[1] < XYZ_Satz_[i].Y)
      { XYZmax[0] = XYZ_Satz_[i].X;
        XYZmax[1] = XYZ_Satz_[i].Y;
        XYZmax[2] = XYZ_Satz_[i].Z;
        maxFeld = i;
        maxFN = Feldnamen_[i].c_str();
      }
      if (XYZmin[1] > XYZ_Satz_[i].Y)
      { XYZmin[0] = XYZ_Satz_[i].X;
        XYZmin[1] = XYZ_Satz_[i].Y;
        XYZmin[2] = XYZ_Satz_[i].Z;
        minFeld = i;
        minFN = Feldnamen_[i].c_str();
      }
    }
    if( maxFN ) {
      DBG_PROG_S( maxFN << " Nr. " << maxFeld << endl << " X_max = "<< XYZmax[0] <<" Y_max = "<< XYZmax[1] <<" Z_max = "<< XYZmax[2] );
    }
    if( minFN ) {
      DBG_PROG_S( minFN << " Nr. " << minFeld << endl << " X_min = "<< XYZmin[0] <<" Y_min = "<< XYZmin[1] <<" Z_min = "<< XYZmin[2] );
    }
  }

  {
    int bkpt_pos = -1;
    ICClist<double> wp, bp;
    if (profile_)
    {
      wp = profile_->getWhitePkt();
      bkpt_pos = profile_->getTagIDByName("bkpt");
      if(bkpt_pos >= 0)
        bp = profile_->getTagCIEXYZ(bkpt_pos);
    }
    if (wp.size() == 3)
    { for (int i = 0; i < 3; i++)
        XYZWP[i] = wp[i];
    } else
    { XYZWP[0] = X_D50;
      XYZWP[1] = Y_D50;
      XYZWP[2] = Z_D50;
    }

    if (bp.size() == 3 &&
        bp[0] != 0.0 && bp[1] != 0.0 && bp[2] != 0.0)
      for (int i = 0; i < 3; i++)
        XYZBP[i] = 0.0;
    else
      for (int i = 0; i < 3; i++)
        XYZBP[i] = XYZmin[i];
  }


  if ((RGB_measurement_ ||
       CMYK_measurement_) || (XYZ_measurement_ || LAB_measurement_))
  {
    double start = fortschritt();


    if (getColorSpaceName(profile_->header.colorSpace()) != "Rgb"
     && getColorSpaceName(profile_->header.colorSpace()) != "Cmyk")
    {
      WARN_S("different mesurement- and profile colour space ")
      DBG_PROG_V( getColorSpaceName(profile_->header.colorSpace()) )
    }

    oyProfile_s * profile = 0,
                * profile_rgb = 0,
                * profile_xyz = oyProfile_FromStd( oyEDITING_XYZ, 0 ),
                * profile_lab = oyProfile_FromStd( oyEDITING_LAB, 0 );

    // select a fitting monitor- / displayprofile
    if(!export_farben)
    {
      int x = 0;
      int y = 0;

      profile_rgb = icc_oyranos.oyMoni( x,y, 0 );

      DBG_PROG_S( icc_oyranos.moni_name( x,y ) << " Farben" )
    } else
    {
      profile_rgb = oyProfile_FromStd( oyASSUMED_WEB, 0 );
      DBG_PROG_S( "Export colours" );
    }

    double Farbe[64], RGB[3], XYZ[3], Lab[3];
    oyConversion_s * ctorgb=0, * ctoxyz=0, * ctolab=0, * labtorgb=0;
    oyOptions_s * abscol=0, * ui_options=0;

    ui_options = oyOptions_FromBoolean( options_, 0, oyBOOLEAN_UNION, NULL );

    /* default to absolute colorimetric  */
    if(oyOptions_FindString( ui_options, "rendering_intent", NULL) == NULL)
      oyOptions_SetFromText( &ui_options, OY_BEHAVIOUR_STD "rendering_intent",
                             "3", OY_CREATE_NEW );

    if ((RGB_measurement_ ||
         CMYK_measurement_))
    {
      if(RGB_measurement_)
        channels_ = 3;
      else
        channels_ = 4;

#   define TYPE_nCOLOUR_DBL (COLORSPACE_SH(PT_ANY)|CHANNELS_SH(channels_)|BYTES_SH(0))
      if( profile_->size() &&
          profile_->data_type == ICCprofile::ICCprofileDATA )
        profile = oyProfile_FromMem( profile_->size_, const_cast<char*>(profile_->data_), 0,0 );

      if(!profile)
      { // alternative
        if( CMYK_measurement_ )
          profile = oyProfile_FromStd( oyEDITING_CMYK,0 );
        else
        if( RGB_measurement_ )
          profile = oyProfile_FromStd( oyEDITING_RGB, 0 );

        if( !profile ) {
          WARN_S("no suitable default profile found")
          goto Kein_Profil; //TODO
        }
      }

      if( !profile )
        WARN_S("hCOLOUR is empty")

      fortschritt(0.1 , 0.2);
      oyOptions_SetFromText( &abscol, OY_BEHAVIOUR_STD "rendering_intent", "3",
                             OY_CREATE_NEW );
      // How sees the profile the measurement colour? -> XYZ
      ctoxyz= oyConversion_CreateBasicPixelsFromBuffers(
                                        profile, Farbe, oyDOUBLE,
                                        profile_xyz, XYZ, oyDOUBLE,
                                        abscol, 1 );
      /*hCOLOURtoXYZ =  cmsCreateTransform (hCOLOUR, TYPE_nCOLOUR_DBL,
                                    hXYZ, TYPE_XYZ_DBL,
                                    INTENT_ABSOLUTE_COLORIMETRIC,
                                    PRECALC|BW_COMP);*/
      fortschritt(0.1, 0.2);
      // How sees the profile the measurement colour? -> Lab
      ctolab= oyConversion_CreateBasicPixelsFromBuffers(
                                        profile, Farbe, oyDOUBLE,
                                        profile_lab, Lab, oyDOUBLE,
                                        abscol, 1 );
      /*hCOLOURtoLab =  cmsCreateTransform (hCOLOUR, TYPE_nCOLOUR_DBL,
                                    hLab, TYPE_Lab_DBL,
                                    INTENT_ABSOLUTE_COLORIMETRIC,
                                    PRECALC|BW_COMP);*/
      oyOptions_Release( &abscol );
      fortschritt(0.15, 0.2);
      // How sees the profile the measurement colour? -> monitor
      ctorgb= oyConversion_CreateBasicPixelsFromBuffers(
                                        profile, Farbe, oyDOUBLE,
                                        profile_rgb, RGB, oyDOUBLE,
                                        ui_options, 1 );
      /*hCOLOURtoRGB =  cmsCreateProofingTransform (hCOLOUR, TYPE_nCOLOUR_DBL,
                                    hsRGB, TYPE_RGB_DBL,
                                    hProof,
                                    intent,
                                    INTENT_RELATIVE_COLORIMETRIC,
                                    PRECALC|flags);*/
      fortschritt(0.3, 0.2);

      if(!ctoxyz || !ctolab || !ctorgb)
        return;
    }
    Kein_Profil:
    if (XYZ_measurement_ || LAB_measurement_)
    {
      // How sees the CMM the measurement colour? -> monitor
      labtorgb = oyConversion_CreateBasicPixelsFromBuffers(
                                           profile_lab, Lab, oyDOUBLE,
                                           profile_rgb, RGB, oyDOUBLE,
                                           ui_options, 1 );

      /*hLabtoRGB = cmsCreateProofingTransform (hLab, TYPE_Lab_DBL,
                                    hsRGB, TYPE_RGB_DBL,
                                    hProof,
                                    intent,
                                    INTENT_RELATIVE_COLORIMETRIC,
                                    PRECALC|flags); */
      if(!labtorgb)
        return;
    }
    oyOptions_Release( &ui_options );

    fortschritt(0.5,0.2);
    bool vcgt = false;
    ICClist<ICClist<double> > vcgt_kurven;
    //TODO
    if (profile_ && profile_->hasTagName ("vcgt")) {
      vcgt = true;
      vcgt_kurven = profile_->getTagCurves( profile_->getTagIDByName("vcgt"),
                                                                ICCtag::MATRIX);
    }

    {
      if (XYZ_measurement_ || LAB_measurement_)
      {
        RGB_MessFarben_.resize(nFelder_);
        if(!XYZ_measurement_)
        {
          XYZ_Satz_.resize(nFelder_);
          for(int i = 0; i < nFelder_; ++i)
          {
            XYZ_Satz_[i].X = 0;
            XYZ_Satz_[i].Y = 0;
            XYZ_Satz_[i].Z = 0;
          }
        }
        if(!LAB_measurement_)
          Lab_Satz_.resize(nFelder_);
        Lab_Differenz_.resize(nFelder_); DBG_NUM_V( Lab_Differenz_.size() )
        DE00_Differenz_.resize(nFelder_);
      }
      Lab_Ergebnis_.resize(nFelder_);
      XYZ_Ergebnis_.resize(nFelder_);
      RGB_ProfilFarben_.resize(nFelder_);

      if( (int)XYZ_Satz_.size() != nFelder_ )
        DBG_PROG_S("XYZ_Satz_.size() and nFelder_ are unequal");
      if( (int)Lab_Satz_.size() != nFelder_ )
        DBG_PROG_S("Lab_Satz_.size() and nFelder_ are unequal");
      if( RGB_Satz_.size() && (int)RGB_Satz_.size() != nFelder_ )
        WARN_S("RGB_Satz_.size() and nFelder_ are unequal");
      if( CMYK_Satz_.size() && (int)CMYK_Satz_.size() != nFelder_ )
        WARN_S("CMYK_Satz_.size() and nFelder_ are unequal");
      for (int i = 0; i < nFelder_; i++)
      {
        if (XYZ_measurement_ || LAB_measurement_)
        {
          if (XYZ_measurement_)
          {
          if (isICCDisplay_) {
            double CIEXYZ[3];
            FarbeZuDouble( CIEXYZ, XYZ_Satz_[i] );
            // scale measurement to ICC white and black
            oyCIEabsXYZ2ICCrelXYZ( CIEXYZ, XYZ, XYZBP, XYZmax, XYZWP);
          } else
            FarbeZuDouble( &XYZ[0], XYZ_Satz_[i] );

          }

          if(LAB_measurement_ && !isICCDisplay_)
          {
            //LabToCIELab( Lab_Satz_[i], &Lab[0] );
            Lab[0] = Lab_Satz_[i].L;
            Lab[1] = Lab_Satz_[i].a;
            Lab[2] = Lab_Satz_[i].b;
          } else {
            XYZtoLab (&XYZ[0], &Lab[0], 1);
            FarbeZuDouble( &Lab_Satz_[i], &Lab[0] );
          }

          if (!XYZ_measurement_)
            LabtoXYZ( Lab_Satz_[i], XYZ_Satz_[i]);

          oyConversion_RunPixels( labtorgb, 0 );
          FarbeZuDouble( &RGB_MessFarben_[i], RGB );
        }

        
        if ((RGB_measurement_ ||
             CMYK_measurement_))
        {

          // profile colours
          if (RGB_measurement_) {
            //for (int n = 0; n < channels_; n++)
            Farbe[0] = RGB_Satz_[i].R; DBG_MESS_V( RGB_Satz_[i].R )
            Farbe[1] = RGB_Satz_[i].G;
            Farbe[2] = RGB_Satz_[i].B;
          } else {
            Farbe[0] = CMYK_Satz_[i].C; DBG_MESS_V( CMYK_Satz_[i].C )
            Farbe[1] = CMYK_Satz_[i].M;
            Farbe[2] = CMYK_Satz_[i].Y;
            Farbe[3] = CMYK_Satz_[i].K;
          }

          if(ctoxyz)
            oyConversion_RunPixels( ctoxyz, 0 );
          XYZ[0] *= 2.0;
          XYZ[1] *= 2.0;
          XYZ[2] *= 2.0;
          FarbeZuDouble ( &XYZ_Ergebnis_[i], &XYZ[0] );

          if(ctolab)
            oyConversion_RunPixels( ctolab, 0 );
          Lab_Ergebnis_[i].L = Lab[0];
          Lab_Ergebnis_[i].a = Lab[1];
          Lab_Ergebnis_[i].b = Lab[2];

          if(ctorgb)
            oyConversion_RunPixels( ctorgb, 0 );
          FarbeZuDouble ( &RGB_ProfilFarben_[i], &RGB[0] );

          if(Lab_Satz_.size())
          {
            // geometric colour differenze - dE CIE*Lab
            double cie_erg[3], cie_satz[3];
            LabToCIELab( Lab_Ergebnis_[i], cie_erg );
            LabToCIELab( Lab_Satz_[i], cie_satz );
            Lab_Differenz_[i] = HYP3( cie_erg[0] - cie_satz[0] ,
                                      cie_erg[1] - cie_satz[1] ,
                                      cie_erg[2] - cie_satz[2]  );
            if (Lab_Differenz_max_ < Lab_Differenz_[i])
              Lab_Differenz_max_ = Lab_Differenz_[i];
            if (Lab_Differenz_min_ > Lab_Differenz_[i])
              Lab_Differenz_min_ = Lab_Differenz_[i];
            // dE2000
            DE00_Differenz_[i] = 
#            if 0
             cmsCIE2000DeltaE( (cmsCIELab*)&Lab_Ergebnis_[i], (cmsCIELab*)&Lab_Satz_[i] , 1.0, 1.0, 1.0);
#            else
             // 2 1 1 is a recomendation of Marti
               dE2000(Lab_Ergebnis_[i], Lab_Satz_[i] , 2.0, 1.0, 1.0);
#            endif
            if (DE00_Differenz_max_ < DE00_Differenz_[i])
              DE00_Differenz_max_ = DE00_Differenz_[i];
            if (DE00_Differenz_min_ > DE00_Differenz_[i])
              DE00_Differenz_min_ = DE00_Differenz_[i];
          }
        }
      }
    }

    if(start <= 0.0)
      fortschritt(1.1);

    oyProfile_Release( &profile );
    oyProfile_Release( &profile_rgb );
    oyProfile_Release( &profile_xyz );
    oyProfile_Release( &profile_lab );

    oyConversion_Release( &ctoxyz );
    oyConversion_Release( &ctolab );
    oyConversion_Release( &ctorgb );
    oyConversion_Release( &labtorgb );
  } else
    WARN_S("no RGB/CMYK and XYZ measurements found")

  for (unsigned int i = 0; i < Lab_Differenz_.size(); i++) {
    Lab_Differenz_Durchschnitt_ += Lab_Differenz_[i];
  }
  Lab_Differenz_Durchschnitt_ /= (double)Lab_Differenz_.size();
  for (unsigned int i = 0; i < DE00_Differenz_.size(); i++) {
    DE00_Differenz_Durchschnitt_ += DE00_Differenz_[i];
  }
  DE00_Differenz_Durchschnitt_ /= (double)DE00_Differenz_.size();
  DBG_NUM_V( Lab_Satz_.size() )
  DBG_NUM_V( Lab_Ergebnis_.size() )
  DBG_NUM_V( Lab_Differenz_.size() )
  DBG_NUM_V( RGB_Satz_.size() )
  DBG_NUM_V( CMYK_Satz_.size() )
  DBG_NUM_V( RGB_MessFarben_.size() )
  DBG_PROG_ENDE
}

void     ICCmeasurement::options     ( oyOptions_s       * opts )
{
  oyOptions_Release( &options_ );
  options_ = oyOptions_Copy( opts, 0 );
}

std::string
ICCmeasurement::getHtmlReport        ( bool                aussen )
{ DBG_PROG_START
  char SF[] = "#cccccc";  // standard background colours
  char HF[] = "#aaaaaa";  // emphasised
# define LAYOUTFARBE  if (layout[l++] == true) \
                        html << HF; \
                      else \
                        html << SF; // select colour after layout option
  int l = 0;
  std::stringstream html; DBG_NUM_V( RGB_MessFarben_.size() )

  bool html_export = aussen;
  if (RGB_MessFarben_.size() == 0 || aussen) { DBG_PROG
    export_farben = aussen;
    init ();
  }


  if (reportTabelle_.size() == 0)
    reportTabelle_ = getText();
  ICClist<int> layout = getLayout(); DBG_PROG

  html << "<!DOCTYPE html PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\">";
  html << "<html><head>" << endl;
  html << "<title>" << _("Report about ICC profile") << "</title>\n";
  html << "<meta http-equiv=\"content-type\" content=\"text/html; charset=ISO-8859-1\">" << endl;
  html << "<meta name=\"description\" content=\"icc_examin ICC Profil Report\">\n";
  html << "<meta name=\"author\" content=\"automatic generated by icc_examin-" << ICC_EXAMIN_V << "\">\n";
  html << "</head><body bgcolor=\"" << SF << "\" text=\"#000000\">" << endl << endl;

  int kopf = (int)reportTabelle_.size() - nFelder_;  // head over table
  if (kopf < 0) kopf = (int)reportTabelle_.size();
  int tkopf = 1;  // tablen head
  if (reportTabelle_.size() <= 1)
    tkopf = 0;
  // common informations
  html << dateiName( profile_->filename() ) << ":<br>";
  for (int i = 0; i < kopf - tkopf ; i++) { DBG_NUM_S (nFelder_<<"|"<<kopf<<"|"<<i)
    //if (i == 0) html << "<h2>";

    html << reportTabelle_[i][0];
    //if (i == 0) html << "</h2>";
    html <<     "<br>\n\n";
  } DBG_NUM_V( nFelder_ )
  if (!nFelder_)
    return html.str();

  html <<       "<table align=left cellpadding=\"2\" cellspacing=\"0\" border=\"0\" width=\"90%\" bgcolor=\"" << SF << "\">\n";
  html <<       "<thead> \n";
  html <<       "  <tr> \n";
  // tablen head
  int s = 0;           // cols
  int f = 0;           // cols for colours
  if (XYZ_Satz_.size() && RGB_MessFarben_.size() == XYZ_Satz_.size()) {
    f = 2;
  } DBG_PROG_V( reportTabelle_.size() )
  DBG_PROG_V( reportTabelle_[reportTabelle_.size()-1][0]<<" "<<kopf<<" "<<tkopf)
  l = 0;
  if(tkopf)            // wothout table head no table
  for (s = 0; s < (int)reportTabelle_  [kopf - tkopf].size() + f; s++) {
    if (s < f) {
      if (s == 0) {
        html <<   "    <th width=\"25\">" << _("Meas colour") << "</th>";
      } else {
        html <<   "    <th width=\"25\">" << _("Profile colour") << "</th>";
      }
    } else {
      html <<   "    <th bgcolor=\""; LAYOUTFARBE
      html << "\">" << reportTabelle_ [kopf - tkopf][s - f] << "</th>\n";
    }
  } DBG_PROG
  html <<       "  </tr>\n";
  html <<       "</thead>\n<tbody>\n";

  // measurements fields
# define  NACH_HTML(satz,kanal) \
          sprintf (farbe, "%x", (int)(satz[z].kanal*mult+0.5)); \
          if (strlen (farbe) == 1) \
            html << "0"; \
          else if (strstr (farbe, "100") != 0) \
            sprintf (farbe, "ff"); \
          html << farbe;

  char farbe[17];
  double mult = 256.0;
  if(tkopf)            // without table head no table
  for (int z = 0; z < nFelder_; z++) {
    html <<     "  <tr>\n";
    l = 0;
    for (s = 0; s < (int)reportTabelle_[kopf - tkopf].size() + f; s++)
    {
      if (s < f)
      { // colour representation
        char * tmp = 0;
        const char * t;
        int len = 0, c = 0;
        if(s == 0)
        {
          if(Feldnamen_[z].c_str())
          {
            /* tokenise uppercase as HTML tags have to be case insensitive */
            t = Feldnamen_[z].c_str();
            len = strlen(t);
            tmp = (char*) malloc(len*2 + 1);
            for(int i = 0; i < len; ++i)
              if(isupper(t[i]))
              {
                sprintf( &tmp[c++], "%c_", t[i] ); c++;
              } else
                sprintf( &tmp[c++], "%c", t[i] );
            html << "    <a name=\"" << Feldnamen_[z] << "\">\n";
          } 
        }
        html << "    <td width=\"20\" bgcolor=\"#"; 
        farbe[0] = 0;

        if (s == 0) {
          NACH_HTML (RGB_MessFarben_, R)
          NACH_HTML (RGB_MessFarben_, G)
          NACH_HTML (RGB_MessFarben_, B)
        } else {
          NACH_HTML (RGB_ProfilFarben_, R)
          NACH_HTML (RGB_ProfilFarben_, G)
          NACH_HTML (RGB_ProfilFarben_, B)
        }
        html << "\"></td>\n";
      } else {
        html << "    <td bgcolor=\""; LAYOUTFARBE
        html << "\">" << reportTabelle_ [kopf + z][s - f] << "</td>\n";
      }
    }
    html <<     "  </tr>\n";
  }

  html <<       "</tbody>\n</table>\n\n<br>\n</body></html>\n";
  //DBG_NUM_S(html.str() )

  if (html_export) {
    export_farben = false;
    init ();
  }

  DBG_PROG_ENDE
  return html.str();
}

ICClist<ICClist<std::string> >
ICCmeasurement::getText                     (void)
{ DBG_PROG_START
  DBG_NUM_V( RGB_MessFarben_.size() )
  if (RGB_MessFarben_.size() == 0)
    init ();

  // push_back is too slow
  ICClist<ICClist<std::string> > tabelle (1);
  std::stringstream s;
  int z = 0; // rows

  tabelle[0].resize(1);
  tabelle[0][0] = _("no measurment data or correct profile conversion available");

  if ((CMYK_measurement_ || RGB_measurement_)
       && (XYZ_measurement_ || LAB_measurement_) &&
      Lab_Differenz_.size() && Lab_Satz_.size()) {
    tabelle.resize(nFelder_+5); // push_back is too slow
    // table head line
    tabelle[0].resize(1);
    tabelle[0][0] =    _("Measurment- and profile colours from <b>"); 
    if (RGB_measurement_)
      tabelle[0][0] += _("RGB");
    else
      tabelle[0][0] += _("CMYK");
    tabelle[0][0] +=   _("</b> Measurement");
    z++; tabelle[z].resize(1);
    tabelle[z][0] = _("CMM: <b>lcms</b>");
    z++; tabelle[z].resize(1);
    s << _("averaging deviations (dE CIE*Lab):<b> ") << Lab_Differenz_Durchschnitt_ << _("</b>  maximum: ") << Lab_Differenz_max_ << _("  minimum: ") << Lab_Differenz_min_ << _("  (dE CIE 2000) averaging: ") << DE00_Differenz_Durchschnitt_ << _("  maximum: ") << DE00_Differenz_max_ << _("  minimum: ") << DE00_Differenz_min_;
    tabelle[z][0] = s.str();
    z++; tabelle[z].resize(1);
    s.str("");
    s << "___________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________";
    tabelle[z][0] = s.str();
    z++;
    // table head
    int spalten, sp = 0, xyz_erg_sp = 0;
    int h = false;
    if (XYZ_Ergebnis_.size() == XYZ_Satz_.size())
      xyz_erg_sp = 3;
#   define HI (h == true) ? h-- : h++ // invert
    layout.clear();
    layout.push_back (HI); // measurement
    layout.push_back (HI); // dE Lab
    layout.push_back (HI); // dE2000
    layout.push_back (h); layout.push_back (h); layout.push_back (HI); // Lab
    layout.push_back (h); layout.push_back (h); layout.push_back (HI); // Lab'
    layout.push_back (h); layout.push_back (h); layout.push_back (HI); // XYZ
    if (xyz_erg_sp)
      layout.push_back (h); layout.push_back (h); layout.push_back (HI);//XYZ'
    if (RGB_measurement_) {
      layout.push_back (h); layout.push_back (h); layout.push_back (HI);//RGB
    } else {
      layout.push_back (h); layout.push_back (h); layout.push_back (h);
      layout.push_back (HI); // CMYK
    }
    spalten = (int) layout.size();
    tabelle[z].resize( spalten ); DBG_NUM_S( tabelle[z].size() )
    tabelle[z][sp++] = _("Measurement patch");
    tabelle[z][sp++]=_("dE Lab");
    tabelle[z][sp++]=_("dE2000");
    tabelle[z][sp++]=_("L");
    tabelle[z][sp++]=_("a");
    tabelle[z][sp++]=_("b");
    tabelle[z][sp++]=_("L'");
    tabelle[z][sp++]=_("a'");
    tabelle[z][sp++]=_("b'");
    tabelle[z][sp++]=_("X");
    tabelle[z][sp++]=_("Y");
    tabelle[z][sp++]=_("Z");
    if (XYZ_Ergebnis_.size() == XYZ_Satz_.size()) {
      tabelle[z][sp++] = _("X'");
      tabelle[z][sp++] = _("Y'");
      tabelle[z][sp++] = _("Z'");
    }
    if (RGB_measurement_) {
      tabelle[z][sp++] = _("R");
      tabelle[z][sp++] = _("G");
      tabelle[z][sp++] = _("B");
    } else if (CMYK_measurement_) {
      tabelle[z][sp++] = _("C");
      tabelle[z][sp++] = _("M");
      tabelle[z][sp++] = _("Y");
      tabelle[z][sp++] = _("K");
    } DBG_PROG_V( z <<" "<< nFelder_ <<" "<< tabelle.size() )
    z++;
    // measurements
    s.str("");
#   define DBG_TAB_V(txt)
    for (int i = 0; i < nFelder_; i++) { 
      sp = 0;
      tabelle[z+i].resize( spalten );
      tabelle[z+i][sp++] =  Feldnamen_[i]; DBG_TAB_V ( z <<" "<< sp <<" "<< Lab_Differenz_.size() )
      s << Lab_Differenz_[i]; tabelle[z+i][sp++] = s.str().c_str(); s.str("");
      s << DE00_Differenz_[i]; tabelle[z+i][sp++] = s.str().c_str(); s.str("");
      s << Lab_Satz_[i].L * (range_Lab[1]-range_Lab[0]) + range_Lab[0]; tabelle[z+i][sp++] = s.str().c_str(); s.str("");
      s << Lab_Satz_[i].a * (range_Lab[3]-range_Lab[2]) + range_Lab[2]; tabelle[z+i][sp++] = s.str().c_str(); s.str("");
      s << Lab_Satz_[i].b * (range_Lab[5]-range_Lab[4]) + range_Lab[4]; tabelle[z+i][sp++] = s.str().c_str(); s.str("");
      s << Lab_Ergebnis_[i].L * (range_Lab[1]-range_Lab[0]) + range_Lab[0]; tabelle[z+i][sp++] = s.str().c_str(); s.str("");
      s << Lab_Ergebnis_[i].a * (range_Lab[3]-range_Lab[2]) + range_Lab[2]; tabelle[z+i][sp++] = s.str().c_str(); s.str("");
      s << Lab_Ergebnis_[i].b * (range_Lab[5]-range_Lab[4]) + range_Lab[4]; tabelle[z+i][sp++] = s.str().c_str(); s.str("");
      s << XYZ_Satz_[i].X * (range_XYZ[5]-range_XYZ[4]) + range_XYZ[0]; tabelle[z+i][sp++] = s.str().c_str(); s.str("");
      s << XYZ_Satz_[i].Y * (range_XYZ[3]-range_XYZ[2]) + range_XYZ[2]; tabelle[z+i][sp++] = s.str().c_str(); s.str("");
      s << XYZ_Satz_[i].Z * (range_XYZ[5]-range_XYZ[4]) + range_XYZ[4]; tabelle[z+i][sp++] = s.str().c_str(); s.str("");
      if (xyz_erg_sp) {
      s << XYZ_Ergebnis_[i].X * (range_XYZ[1]-range_XYZ[0]) + range_XYZ[0]; tabelle[z+i][sp++]=s.str().c_str();s.str("");
      s << XYZ_Ergebnis_[i].Y * (range_XYZ[3]-range_XYZ[2]) + range_XYZ[2]; tabelle[z+i][sp++]=s.str().c_str();s.str("");
      s << XYZ_Ergebnis_[i].Z * (range_XYZ[5]-range_XYZ[4]) + range_XYZ[4]; tabelle[z+i][sp++]=s.str().c_str();s.str("");
      } DBG_TAB_V( i )
      if (RGB_measurement_) {
        s << RGB_Satz_[i].R * (range_RGB[1]-range_RGB[0]) + range_RGB[0]; tabelle[z+i][sp++]= s.str().c_str(); s.str("");
        s << RGB_Satz_[i].G * (range_RGB[3]-range_RGB[2]) + range_RGB[2]; tabelle[z+i][sp++]= s.str().c_str(); s.str("");
        s << RGB_Satz_[i].B * (range_RGB[5]-range_RGB[4]) + range_RGB[4]; tabelle[z+i][sp++]= s.str().c_str(); s.str("");
      } else {
        s << CMYK_Satz_[i].C * (range_CMYK[1]-range_CMYK[0]) + range_CMYK[0]; tabelle[z+i][sp++]=s.str().c_str(); s.str("");
        s << CMYK_Satz_[i].M * (range_CMYK[3]-range_CMYK[2]) + range_CMYK[2]; tabelle[z+i][sp++]=s.str().c_str(); s.str("");
        s << CMYK_Satz_[i].Y * (range_CMYK[5]-range_CMYK[4]) + range_CMYK[4]; tabelle[z+i][sp++]=s.str().c_str(); s.str("");
        s << CMYK_Satz_[i].K * (range_CMYK[7]-range_CMYK[6]) + range_CMYK[6]; tabelle[z+i][sp++]=s.str().c_str(); s.str("");
      }
    }
  }

  DBG_PROG_ENDE
  return tabelle;
}

std::string
ICCmeasurement::getCGATS              (void)
{ DBG_PROG_START
  std::string text =  "";

  if(has_data())
    text = cgats_korrigieren(data_,size_);
  else
    text = "";

  DBG_PROG_ENDE
  return text;
}

std::string
ICCmeasurement::getMaxCGATS           (void)
{ DBG_PROG_START
  std::string text =  "";

  if(has_data())
    text = cgats_max_korrigieren(data_,size_);
  else
    text = "";

  DBG_PROG_ENDE
  return text;
}

ICClist<std::string>
ICCmeasurement::getDescription              (void)
{ DBG_PROG_START
  ICClist<std::string> texte;
  std::string text =  "";

# ifdef DEBUG_ICCMEASUREMENT
# endif

  DBG_PROG_ENDE
  return texte;
}

ICClist<double>
ICCmeasurement::getMessRGB                  (int patch)
{ DBG_MESS_START
  ICClist<double> punkte(3);

  if (RGB_MessFarben_.size() == 0)
    init ();

  if (patch > nFelder_) {
    WARN_S( "Patch No: " << patch << " outside the measurment set" )
    DBG_MESS_ENDE
    return punkte;
  }

  punkte[0] = RGB_MessFarben_[patch].R; DBG_MESS_V( RGB_MessFarben_[patch].R <<  punkte[0] )
  punkte[1] = RGB_MessFarben_[patch].G; DBG_MESS_V( RGB_MessFarben_[patch].G )
  punkte[2] = RGB_MessFarben_[patch].B; DBG_MESS_V( RGB_MessFarben_[patch].B )

  DBG_MESS_ENDE
  return punkte;
}

ICClist<double>
ICCmeasurement::getCmmRGB                   (int patch)
{ DBG_MESS_START
  ICClist<double> punkte (3) ;

  if (RGB_ProfilFarben_.size() == 0)
    init ();

  if (patch > nFelder_) {
    WARN_S( "Patch No: " << patch << " outside the measurement set" )
    DBG_MESS_ENDE
    return punkte;
  }

  punkte[0] = RGB_ProfilFarben_[patch].R;
  punkte[1] = RGB_ProfilFarben_[patch].G;
  punkte[2] = RGB_ProfilFarben_[patch].B;

  DBG_MESS_ENDE
  return punkte;
}

ICClist<double>
ICCmeasurement::getMessLab                  (int patch)
{ DBG_MESS_START
  ICClist<double> punkte (3) ;

  if (Lab_Satz_.size() == 0)
    init ();

  if (patch > nFelder_) {
    WARN_S( "Patch No: " << patch << " ausserhalb des Messfarbsatzes" )
    DBG_MESS_ENDE
    return punkte;
  }

  punkte[0] = Lab_Satz_[patch].L;
  punkte[1] = Lab_Satz_[patch].a;
  punkte[2] = Lab_Satz_[patch].b;

  DBG_MESS_ENDE
  return punkte;
}

ICClist<double>
ICCmeasurement::getCmmLab                   (int patch)
{ DBG_MESS_START
  ICClist<double> punkte (3) ;

  if (Lab_Ergebnis_.size() == 0)
    init ();

  if (patch > nFelder_) {
    WARN_S( "Patch Nr: " << patch << " outside the measurement set" )
    DBG_MESS_ENDE
    return punkte;
  }

  punkte[0] = Lab_Ergebnis_[patch].L;
  punkte[1] = Lab_Ergebnis_[patch].a;
  punkte[2] = Lab_Ergebnis_[patch].b;

  DBG_MESS_ENDE
  return punkte;
}

/** @func  getCmmColour
 *  @brief get a measured colour as Oyranos struct
 *
 *  The XYZ colour is calculated from the contained original channels over 
 *  the profile.
 *
 *  @since ICC Examin 0.45
 *  @date  28 december 2007 (ICC Examin 0.45)
 */
oyNamedColour_s *
ICCmeasurement::getCmmColour (int patch)
{ DBG_MESS_START
  oyNamedColour_s * nc = 0;
  int i = patch;
  double * channels = 0, * XYZ = 0;

  if (XYZ_Ergebnis_.size() == 0)
    init ();

  if (XYZ_Ergebnis_.size() == 0) {
    WARN_S( "Could not initialise measurements" )
    DBG_MESS_ENDE
    return nc;
  }

  if (patch > nFelder_) {
    WARN_S( "Patch Nr: " << patch << " outside the measurement set" )
    DBG_MESS_ENDE
    return nc;
  }

  oyProfile_s * prof = oyProfile_Copy( profile_->profile_, 0 );

  if(!prof)
  {
    if(profile_->data_type == ICCprofile::ICCprofileDATA)
      prof = oyProfile_FromFile( profile_->filename(), 0, 0 );
    else
    {
      if(CMYK_measurement_)
        prof = oyProfile_FromStd( oyEDITING_CMYK, 0 );
      else
        prof = oyProfile_FromStd( oyEDITING_RGB, 0 );
    }
  }

  if(CMYK_measurement_)
    channels = (double*)&CMYK_Satz_[i];
  else if(RGB_measurement_)
    channels = (double*)&RGB_Satz_[i];

  XYZ = (double*)&XYZ_Ergebnis_[i];

  nc = oyNamedColour_CreateWithName( Feldnamen_[i].c_str(),0,0,
                                     channels, XYZ, 0,0, prof, 0 );
  oyProfile_Release( &prof );
  DBG_MESS_ENDE
  return nc;
}

/** @func  getMessColour
 *  @brief get a measured colour as Oyranos struct
 *
 *  The XYZ colour and the device channels are preserved from the measurement.
 *  The profile is provided informational.
 *
 *  @since ICC Examin 0.45
 *  @date  28 december 2007 (ICC Examin 0.45)
 */
oyNamedColour_s *
ICCmeasurement::getMessColour (int patch)
{ DBG_MESS_START
  oyNamedColour_s * nc = 0;
  int i = patch;
  double * channels = 0;
  const double * CIEXYZ = 0,
               * XYZ = 0;
  double ICCXYZ[3];

  if (XYZ_Satz_.size() == 0)
    init ();

  if (XYZ_Satz_.size() == 0) {
    WARN_S( "Could not initialise measurements" )
    DBG_MESS_ENDE
    return nc;
  }

  if (patch > nFelder_) {
    WARN_S( "Patch Nr: " << patch << " outside the measurement set" )
    DBG_MESS_ENDE
    return nc;
  }

  oyProfile_s * prof = oyProfile_Copy( profile_->profile_, 0 );

  if(!prof)
  {
    if(profile_->data_type == ICCprofile::ICCprofileDATA)
      prof = oyProfile_FromFile( profile_->filename(), 0, 0 );
    else
    {
      if(CMYK_measurement_)
        prof = oyProfile_FromStd( oyEDITING_CMYK, 0 );
      else
        prof = oyProfile_FromStd( oyEDITING_RGB, 0 );
    }
  }

  if(CMYK_measurement_)
    channels = (double*)&CMYK_Satz_[i];
  else if(RGB_measurement_)
    channels = (double*)&RGB_Satz_[i];

  CIEXYZ = (double*)&XYZ_Satz_[i];
  if (isICCDisplay_)
  {
    oyCIEabsXYZ2ICCrelXYZ( CIEXYZ, ICCXYZ, XYZBP, XYZmax, XYZWP);
    XYZ = ICCXYZ;
  } else
    XYZ = CIEXYZ;

  nc = oyNamedColour_CreateWithName( Feldnamen_[i].c_str(),0,0,
                                     channels, XYZ, 0,0, prof, 0 );
  oyProfile_Release( &prof );
  DBG_MESS_ENDE
  return nc;
}


/** get infos about a CGATS tag

    The info becomes available during parsing a CGATS tag

    @param[in]   tag_name    take the according tag : CIED <-> DevD

    @return                  list of patch lines, size() is patch count
 */
ICClist<int>
ICCmeasurement::getPatchLines              ( const char       * tag_name )
{ DBG_MESS_START

  if (Lab_Ergebnis_.size() == 0)
    init ();

  ICClist<int> patches;

  if (Lab_Ergebnis_.size() == 0)
    return patches;

  for(int j = 0; j < (int)patch_src_lines_.size(); ++j)
  {
    if( patch_src_lines_[j].first == tag_name )
    {
      int n = patch_src_lines_[j].second.size();
      patches.resize( n );

      for(int i = 0; i < nFelder_; ++i)
        patches[ i ] = patch_src_lines_[j].second[i];
    }
  }

  DBG_MESS_ENDE
  return patches;
}


/** Get information about a text line in a measurement tag.
 */
oyNamedColour_s *
ICCmeasurement::getPatchLine  ( int line, const char * tag_name )
{ DBG_MESS_START
  oyNamedColour_s * nc = 0;
  int patch = -1;

  if (Lab_Ergebnis_.size() == 0)
    init ();

  for(int j = 0; j < (int)patch_src_lines_.size(); ++j)
  {
    if( patch_src_lines_[j].first == tag_name )
    {
      for(int i = 0; i < nFelder_; ++i)
      {
        if( patch_src_lines_[j].second[i] == line )
        {
          patch = i;
          break;
        }
      }
      break;
    }
  }

  if (patch > nFelder_) {
    WARN_S( "Patch Nr: " << patch << " outside the measurement set" )
    DBG_MESS_ENDE
    return nc;
  }
  if( patch == -1 ) {
    DBG_PROG_S( "Line Nr: " << line << " outside the measurement size" )
    DBG_MESS_ENDE
    return nc;
  }

  if( std::string(tag_name) == "DevD" ||
      ( !Lab_Satz_.size() &&
        Lab_Ergebnis_.size() ) )
    nc = getCmmColour( patch );
  else if (Lab_Satz_.size() && RGB_MessFarben_.size())
    nc = getMessColour( patch );

  DBG_MESS_ENDE
  return nc;
}


