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
 * Qualtitätsprüfung anhand von Messdaten welche im Profil vorhanden sind.
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


#include <lcms.h> // für CGATS lesen
#include "icc_profile.h"
#include "icc_oyranos.h"
#include "icc_examin_version.h"
#include "icc_helfer.h"
#include "icc_cgats_filter.h"
#include "icc_examin.h"

#define g_message printf


/**
  *  @brief ICCmeasurement Funktionen
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
  WARN_S( "-----------------------  den default Konstruktor nicht benutzen --------------------------" )
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
  sig_ = icMaxEnumTag;
  size_ = 0;
  data_ = NULL;

  nFelder_ = 0;

  channels_ = 0;
  isMatrix_ = 0;
  profile_ = NULL;
  XYZ_measurement_ = false;
  RGB_measurement_ = false;
  CMYK_measurement_ = false;
  Lab_Differenz_max_ = -1000;
  Lab_Differenz_min_ = 1000;
  Lab_Differenz_Durchschnitt_ = 0;
  DE00_Differenz_max_ = -1000;
  DE00_Differenz_min_ = 1000;
  DE00_Differenz_Durchschnitt_ = 0;
  export_farben = false;
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
  isMatrix_ = m.isMatrix_;
  profile_ = m.profile_;
  XYZ_measurement_ = m.XYZ_measurement_;
  RGB_measurement_ = m.RGB_measurement_;
  CMYK_measurement_ = m.CMYK_measurement_;
  // Messwerte
  XYZ_Satz_ = m.XYZ_Satz_;
  Lab_Satz_ = m.Lab_Satz_;
  RGB_Satz_ = m.RGB_Satz_;
  CMYK_Satz_ = m.CMYK_Satz_;
  // Profilwerte
  Feldnamen_ = m.Feldnamen_;
  XYZ_Ergebnis_ = m.XYZ_Ergebnis_;
  Lab_Ergebnis_ = m.Lab_Ergebnis_;
  RGB_MessFarben_ = m.RGB_MessFarben_;
  RGB_ProfilFarben_ = m.RGB_ProfilFarben_;
  // Ergebnisse
  Lab_Differenz_ = m.Lab_Differenz_;
  Lab_Differenz_max_ = m.Lab_Differenz_max_;
  Lab_Differenz_min_ = m.Lab_Differenz_min_;
  Lab_Differenz_Durchschnitt_ = m.Lab_Differenz_Durchschnitt_;
  DE00_Differenz_ = m.DE00_Differenz_;
  DE00_Differenz_max_ = m.DE00_Differenz_max_;
  DE00_Differenz_min_ = m.DE00_Differenz_min_;
  DE00_Differenz_Durchschnitt_ = m.DE00_Differenz_Durchschnitt_;
  export_farben = m.export_farben;
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
  Lab_Differenz_.clear();
  DE00_Differenz_.clear();
  reportTabelle_.clear();
  layout.clear();
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
  if (!profile_) WARN_S( "kann nicht initialisieren, Profilreferenz fehlt" )
  //DBG_MEM_V( profile_->hasTagName("targ") << profile_->printLongHeader() )

  sig_    = tag._sig;
  size_   = tag.size_ - 8;
  DBG_PROG_V( size_ )
  // einfach austauschen
  if (data_ != NULL) free (data_); 
  data_ = (char*) calloc ( size_+1 , sizeof (char) );
  memcpy ( data_ , &(tag.data_)[8] , size_ );

  DBG_PROG_ENDE
}

void
ICCmeasurement::load                ( ICCprofile *profil,
                                      const char *data,
                                      size_t      size )
{ DBG_PROG_START
  profile_ = profil;
  if (!profile_) WARN_S( "kann nicht initialisieren, Profilreferenz fehlt" )

  if (sig_ != icMaxEnumTag)
    sig_ = icSigCharTargetTag;

  size_   = size;
  // einfach austauschen
  if (!data_) free (data_);
  data_ = (char*) calloc ( size_+1 , sizeof (char) );
  memcpy ( data_ , data , size_ );

  DBG_PROG_ENDE
}

void
ICCmeasurement::leseTag (void)
{ DBG_PROG_START

  DBG_PROG_V( data_ )

  CgatsFilter cgats;
  cgats.lade( data_, size_ );
  std::string data = cgats.lcms_gefiltert (); DBG_NUM_V( (int*)data_ )

  // korrigierte CGATS Daten -> data_
  if (data_ != NULL) free (data_);
  data_ = (char*) calloc (sizeof(char), data.size()+1);
  size_ = data.size();
  memcpy (data_, data.c_str(), size_); DBG_NUM_V( (int*)data_ )

  // lcms liest ein
  lcms_parse();
  DBG_PROG_ENDE
}

void
ICCmeasurement::init (void)
{ DBG_PROG_START DBG_MEM_V( (int*)data_ )
  if (valid())
    return;

  if (!profile_) WARN_S( "kann nicht initialisieren, Profilreferenz fehlt; id: "<<id_<<" profil: "<< (int*)profile_ )

  if (profile_->hasTagName("targ")) {
    load (profile_, profile_->getTag(profile_->getTagByName("targ")));
    leseTag ();
  }
  else if (profile_->hasTagName("DevD") && (profile_->hasTagName("CIED"))) {
    load (profile_, profile_->getTag(profile_->getTagByName("DevD")));
    leseTag ();
    
    load (profile_, profile_->getTag(profile_->getTagByName("CIED")));
    leseTag ();
  }
  if (RGB_MessFarben_.size() != 0)
    DBG_NUM_V( RGB_MessFarben_.size() )

  if (profile_)
  {
    channels_ = profile_->getColourChannelsCount();
    isMatrix_ = !(profile_->hasCLUT());
  }

  init_umrechnen();
  DBG_PROG_ENDE
}

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
    WARN_S( "Messfeldanzahl sollte schon übereinstimmen! " << nFelder_ << "|" << (int)cmsIT8GetPropertyDbl(_lcms_it8, "NUMBER_OF_SETS") )
    clear();
    return;
  }

  int _nKanaele = cmsIT8EnumDataFormat(_lcms_it8, &SampleNames);
  bool _sample_name = false;
  bool _sample_id = false;
  bool _id_vor_name = false;

  // Was ist alles da? Wollen wir später die Namen tauschen?
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
  std::vector<std::string> farbkanaele;
  // müssen lokal bleiben !
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


  // vorläufige lcms Farbnamen listen
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
  if (has_XYZ) { DBG_PROG // keine Umrechnung nötig
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
  if (has_RGB) { DBG_PROG // keine Umrechnung nötig
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
  if (has_CMYK) { DBG_PROG // keine Umrechnung nötig
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

void
ICCmeasurement::init_umrechnen                     (void)
{ DBG_PROG_START
  Lab_Differenz_max_ = -1000.0;
  Lab_Differenz_min_ = 1000.0;
  Lab_Differenz_Durchschnitt_ = 0.0;
  DE00_Differenz_max_ = -1000.0;
  DE00_Differenz_min_ = 1000.0;
  DE00_Differenz_Durchschnitt_ = 0.0;
# define PRECALC cmsFLAGS_NOTPRECALC // No memory overhead, VERY
                                      // SLOW ON TRANSFORMING, very fast on creating transform.
                                      // Maximum accurancy.

  double max[3], min[3], WP[3];
  max[0] = max[1] = max[2] = 0;
  min[0] = min[1] = min[2] = 100;
  {
    std::vector<double> wp;
    if (profile_) wp = profile_->getWhitePkt();
    if (wp.size() == 3)
    { for (int i = 0; i < 3; i++)
        WP[i] = wp[i];
    } else
    { WP[0] = X_D50;
      WP[1] = Y_D50;
      WP[2] = Z_D50;
    }
  }
  { int maxFeld=0, minFeld=0;
    const char *maxFN=0, *minFN=0;
    if (nFelder_ != (int)XYZ_Satz_.size()) {
      DBG_PROG_S("Messfeldanzahl divergiert");
    }
    int m = nFelder_ < (int)XYZ_Satz_.size() ? nFelder_ : (int)XYZ_Satz_.size();
    DBG_PROG_S( "Felder: " << m )
    for (int i = 0; i < m; i++)
    { 
      if (max[1] < XYZ_Satz_[i].Y)
      { max[0] = XYZ_Satz_[i].X;
        max[1] = XYZ_Satz_[i].Y;
        max[2] = XYZ_Satz_[i].Z;
        maxFeld = i;
        maxFN = Feldnamen_[i].c_str();
      }
      if (min[1] > XYZ_Satz_[i].Y)
      { min[0] = XYZ_Satz_[i].X;
        min[1] = XYZ_Satz_[i].Y;
        min[2] = XYZ_Satz_[i].Z;
        minFeld = i;
        minFN = Feldnamen_[i].c_str();
      }
    }
    if( maxFN ) {
      DBG_PROG_S( maxFN << " Nr. " << maxFeld << endl << " X_max = "<< max[0] <<" Y_max = "<< max[1] <<" Z_max = "<< max[2] );
    }
    if( minFN ) {
      DBG_PROG_S( minFN << " Nr. " << minFeld << endl << " X_min = "<< min[0] <<" Y_min = "<< min[1] <<" Z_min = "<< min[2] );
    }
  }


  if ((RGB_measurement_ ||
       CMYK_measurement_) || XYZ_measurement_)
  {
    cmsHTRANSFORM hCOLOURtoRGB=0, hXYZtoRGB=0, hCOLOURtoXYZ=0, hXYZtoLab=0,
                  hCOLOURtoLab=0;
    cmsHPROFILE   hCOLOUR=0, hsRGB=0, hLab=0, hXYZ=0;


    if (getColorSpaceName(profile_->header.colorSpace()) != "Rgb"
     || getColorSpaceName(profile_->header.colorSpace()) != "Cmyk")
    {
      WARN_S("unterschiedliche Messdaten und Profilfarbraum ")
      DBG_PROG_V( getColorSpaceName(profile_->header.colorSpace()) )
    }

    // ein passendes Bildschirm- / Darstellungsprofil aussuchen
    if(!export_farben)
    {
#     ifdef HAVE_OY
      size_t groesse = 0;
      const char* block = 0;
      int x = 0;
      int y = 0;

      block = icc_oyranos.moni( x,y, groesse );
      if(groesse)
        hsRGB = cmsOpenProfileFromMem(const_cast<char*>(block), groesse);
      DBG_PROG_S( icc_oyranos.moni_name( x,y ) << " Farben" )
#     endif
    } else { DBG_PROG_S( "Export Farben" ); }
    if(!hsRGB)
      hsRGB = cmsCreate_sRGBProfile ();
    hLab = cmsCreateLabProfile (cmsD50_xyY());
    hXYZ = cmsCreateXYZProfile ();
#   if 0
#   define BW_COMP cmsFLAGS_WHITEBLACKCOMPENSATION
#   else
#   define BW_COMP 0
#   endif
#   define TYPE_nCOLOUR_DBL (COLORSPACE_SH(PT_ANY)|CHANNELS_SH(channels_)|BYTES_SH(0))
    if ((RGB_measurement_ ||
         CMYK_measurement_))
    {
      if( profile_->size() )
        hCOLOUR = cmsOpenProfileFromMem (const_cast<char*>(profile_->data_),
                                         profile_->size_);
      else { // Alternative
        size_t groesse = 0;
        const char* block = 0;
#       ifdef HAVE_OY
        if( CMYK_measurement_ )
          block = icc_oyranos.cmyk(groesse);
        else
        if( RGB_measurement_ )
          block = icc_oyranos.rgb(groesse);
#       endif
        DBG_PROG_V( groesse )

        if( !groesse ) {
          WARN_S(_("kein passendes voreingestelltes Profil gefunden"))
          goto Kein_Profil; //TODO
        } else
          hCOLOUR = cmsOpenProfileFromMem(const_cast<char*>(block), groesse);
      }
      if( !hCOLOUR )
        WARN_S(_("hCOLOUR ist leer"))

      // Wie sieht das Profil die Messfarbe? -> XYZ
      hCOLOURtoXYZ =  cmsCreateTransform (hCOLOUR, TYPE_nCOLOUR_DBL,
                                    hXYZ, TYPE_XYZ_DBL,
                                    INTENT_ABSOLUTE_COLORIMETRIC,
                                    PRECALC|BW_COMP);
      // Wie sieht das Profil die Messfarbe? -> Lab
      hCOLOURtoLab =  cmsCreateTransform (hCOLOUR, TYPE_nCOLOUR_DBL,
                                    hLab, TYPE_Lab_DBL,
                                    INTENT_ABSOLUTE_COLORIMETRIC,
                                    PRECALC|BW_COMP);
      // Wie sieht das Profil die Messfarbe? -> Bildschirmdarstellung
      hCOLOURtoRGB =  cmsCreateProofingTransform (hCOLOUR, TYPE_nCOLOUR_DBL,
                                    hsRGB, TYPE_RGB_DBL,
                                    hsRGB,
                                    INTENT_ABSOLUTE_COLORIMETRIC,
                                    INTENT_ABSOLUTE_COLORIMETRIC,
                                    icc_examin->gamutwarn() ?
                                    cmsFLAGS_GAMUTCHECK : 0  |
                                    PRECALC|BW_COMP);
    }
    Kein_Profil:
    if (XYZ_measurement_)
    {
      if( !hXYZ ) WARN_S(_("hXYZ ist leer"))
      if( !hsRGB ) WARN_S(_("hsRGB ist leer"))

      // Wie sieht das Messgerät die Messfarbe? -> Lab
      hXYZtoLab = cmsCreateTransform (hXYZ, TYPE_XYZ_DBL,
                                    hLab, TYPE_Lab_DBL,
                                    INTENT_ABSOLUTE_COLORIMETRIC,
                                    PRECALC|BW_COMP);
      // Wie sieht die CMM die Messfarbe? -> Bildschirmdarstellung
      hXYZtoRGB = cmsCreateProofingTransform (hXYZ, TYPE_XYZ_DBL,
                                    hsRGB, TYPE_RGB_DBL,
                                    hsRGB,
                                    INTENT_ABSOLUTE_COLORIMETRIC,
                                    INTENT_ABSOLUTE_COLORIMETRIC,
                                    icc_examin->gamutwarn() ?
                                    cmsFLAGS_GAMUTCHECK : 0  |
                                    PRECALC|BW_COMP);
    }
    double Farbe[channels_], RGB[3], XYZ[3], Lab[3];
    bool vcgt = false;
    std::vector<std::vector<double> > vcgt_kurven;
    //TODO
    if (profile_ && profile_->hasTagName ("vcgt")) {
      vcgt = true;
      vcgt_kurven = profile_->getTagCurves( profile_->getTagByName("vcgt"),
                                                                ICCtag::MATRIX);
    }

    if (XYZ_measurement_)
    {
      RGB_MessFarben_.resize(nFelder_);
      RGB_ProfilFarben_.resize(nFelder_);
      XYZ_Ergebnis_.resize(nFelder_);
      Lab_Satz_.resize(nFelder_);
      Lab_Ergebnis_.resize(nFelder_);
      Lab_Differenz_.resize(nFelder_); DBG_NUM_V( Lab_Differenz_.size() )
      DE00_Differenz_.resize(nFelder_);

      if( (int)XYZ_Satz_.size() != nFelder_ )
        WARN_S(_("XYZ_Satz_.size() und nFelder_ sind ungleich"))
      if( RGB_Satz_.size() && (int)RGB_Satz_.size() != nFelder_ )
        WARN_S(_("RGB_Satz_.size() und nFelder_ sind ungleich"))
      if( CMYK_Satz_.size() && (int)CMYK_Satz_.size() != nFelder_ )
        WARN_S(_("CMYK_Satz_.size() und nFelder_ sind ungleich"))
      for (int i = 0; i < nFelder_; i++)
      {
        if (isMatrix_) {
          // Messfarben auf Weiss und Schwarz addaptiert
          XYZ[0] = (XYZ_Satz_[i].X-min[0])/(max[0]-min[0])*WP[0];
          XYZ[1] = (XYZ_Satz_[i].Y-min[1])/(max[1]-min[1])*WP[1];
          XYZ[2] = (XYZ_Satz_[i].Z-min[2])/(max[2]-min[2])*WP[2];
        } else {
          XYZ[0] = XYZ_Satz_[i].X;
          XYZ[1] = XYZ_Satz_[i].Y;
          XYZ[2] = XYZ_Satz_[i].Z;
        }
        cmsDoTransform (hXYZtoLab, &XYZ[0], &Lab[0], 1);
        Lab_Satz_[i].L = Lab[0]; DBG_MESS_V( Lab_Satz_[i].L )
        Lab_Satz_[i].a = Lab[1]; DBG_MESS_V( Lab_Satz_[i].a )
        Lab_Satz_[i].b = Lab[2]; DBG_MESS_V( Lab_Satz_[i].b )

        cmsDoTransform (hXYZtoRGB, &XYZ[0], &RGB[0], 1);
        RGB_MessFarben_[i].R = RGB[0]; DBG_MESS_V( RGB_MessFarben_[i].R )
        RGB_MessFarben_[i].G = RGB[1]; DBG_MESS_V( RGB_MessFarben_[i].G )
        RGB_MessFarben_[i].B = RGB[2]; DBG_MESS_V( RGB_MessFarben_[i].B )

        
        if ((RGB_measurement_ ||
             CMYK_measurement_))
        {

          // Profilfarben
          if (RGB_measurement_) {
            //for (int n = 0; n < channels_; n++)
            Farbe[0] = RGB_Satz_[i].R*100.0; DBG_MESS_V( RGB_Satz_[i].R )
            Farbe[1] = RGB_Satz_[i].G*100.0;
            Farbe[2] = RGB_Satz_[i].B*100.0;
          } else {
            Farbe[0] = CMYK_Satz_[i].C*100.0; DBG_MESS_V( CMYK_Satz_[i].C )
            Farbe[1] = CMYK_Satz_[i].M*100.0;
            Farbe[2] = CMYK_Satz_[i].Y*100.0;
            Farbe[3] = CMYK_Satz_[i].K*100.0;
          }

          cmsDoTransform (hCOLOURtoXYZ, &Farbe[0], &XYZ[0], 1);
          XYZ_Ergebnis_[i].X = XYZ[0]; DBG_MESS_V( XYZ_Ergebnis_[i].X )
          XYZ_Ergebnis_[i].Y = XYZ[1]; DBG_MESS_V( XYZ_Ergebnis_[i].Y )
          XYZ_Ergebnis_[i].Z = XYZ[2]; DBG_MESS_V( XYZ_Ergebnis_[i].Z )

          cmsDoTransform (hCOLOURtoLab, &Farbe[0], &Lab[0], 1);
          Lab_Ergebnis_[i].L = Lab[0]; DBG_MESS_V( Lab_Ergebnis_[i].L )
          Lab_Ergebnis_[i].a = Lab[1]; DBG_MESS_V( Lab_Ergebnis_[i].a )
          Lab_Ergebnis_[i].b = Lab[2]; DBG_MESS_V( Lab_Ergebnis_[i].b )
        
          cmsDoTransform (hCOLOURtoRGB, &Farbe[0], &RGB[0], 1);
          RGB_ProfilFarben_[i].R = RGB[0]; DBG_MESS_V( RGB_ProfilFarben_[i].R )
          RGB_ProfilFarben_[i].G = RGB[1]; DBG_MESS_V( RGB_ProfilFarben_[i].G )
          RGB_ProfilFarben_[i].B = RGB[2]; DBG_MESS_V( RGB_ProfilFarben_[i].B )

          // geometrische Farbortdifferenz - dE CIE*Lab
          Lab_Differenz_[i] = HYP3( Lab_Ergebnis_[i].L - Lab_Satz_[i].L ,
                                    Lab_Ergebnis_[i].a - Lab_Satz_[i].a ,
                                    Lab_Ergebnis_[i].b - Lab_Satz_[i].b  );
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
    if (XYZ_measurement_) {
      if(hXYZtoRGB) cmsDeleteTransform (hXYZtoRGB);
      if(hXYZtoLab) cmsDeleteTransform (hXYZtoLab);
    }
    if ((RGB_measurement_ ||
         CMYK_measurement_)) {
      if(hCOLOURtoXYZ) cmsDeleteTransform (hCOLOURtoXYZ);
      if(hCOLOURtoLab) cmsDeleteTransform (hCOLOURtoLab);
      if(hCOLOURtoRGB) cmsDeleteTransform (hCOLOURtoRGB);
      if(hCOLOUR) cmsCloseProfile (hCOLOUR);
    }
    if(hsRGB) cmsCloseProfile (hsRGB);
    if(hLab) cmsCloseProfile (hLab);
    if(hXYZ) cmsCloseProfile (hXYZ);
  } else
    WARN_S("keine RGB/CMYK und XYZ Messdaten gefunden")

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

std::string
ICCmeasurement::getHtmlReport                     (bool aussen)
{ DBG_PROG_START
  char SF[] = "#cccccc";  // standard Hintergrundfarbe
  char HF[] = "#aaaaaa";  // hervorgehoben
# define LAYOUTFARBE  if (layout[l++] == true) \
                        html << HF; \
                      else \
                        html << SF; //Farbe nach Layoutoption auswählen
  int l = 0;
  std::stringstream html; DBG_NUM_V( RGB_MessFarben_.size() )

  bool html_export = aussen;
  if (RGB_MessFarben_.size() == 0 || aussen) { DBG_PROG
    export_farben = aussen;
    init ();
  }


  if (reportTabelle_.size() == 0)
    reportTabelle_ = getText();
  std::vector<int> layout = getLayout(); DBG_PROG

  html << "<!DOCTYPE html PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\">";
  html << "<html><head>" << endl;
  html << "<title>" << _("Report about ICC profile") << "</title>\n";
  html << "<meta http-equiv=\"content-type\" content=\"text/html; charset=ISO-8859-1\">" << endl;
  html << "<meta name=\"description\" content=\"icc_examin ICC Profil Report\">\n";
  html << "<meta name=\"author\" content=\"automatic generated by icc_examin-" << ICC_EXAMIN_V << "\">\n";
  html << "</head><body bgcolor=\"" << SF << "\" text=\"#000000\">" << endl << endl;

  int kopf = (int)reportTabelle_.size() - nFelder_;  // Kopf über Tabelle
  if (kopf < 0) kopf = (int)reportTabelle_.size();
  int tkopf = 1;  // Tabellenkopf
  if (reportTabelle_.size() <= 1)
    tkopf = 0;
  // Allgemeine Informationen
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
  // Tabellenkopf
  int s = 0;           // Spalten
  int f = 0;           // Spalten für Farben
  if (XYZ_Satz_.size() && RGB_MessFarben_.size() == XYZ_Satz_.size()) {
    f = 2;
  } DBG_PROG_V( reportTabelle_.size() )
  DBG_PROG_V( reportTabelle_[reportTabelle_.size()-1][0]<<" "<<kopf<<" "<<tkopf)
  l = 0;
  if(tkopf)            // ohne Tabellenkopf keine Tabelle
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

  // Messfelder
# define  NACH_HTML(satz,kanal) \
          sprintf (farbe, "%x", (int)(satz[z].kanal*mult+0.5)); \
          if (strlen (farbe) == 1) \
            html << "0"; \
          else if (strstr (farbe, "100") != 0) \
            sprintf (farbe, "ff"); \
          html << farbe;

  char farbe[17];
  double mult = 256.0;
  if(tkopf)            // ohne Tabellenkopf keine Tabelle
  for (int z = 0; z < nFelder_; z++) {
    html <<     "  <tr>\n";
    l = 0;
    for (s = 0; s < (int)reportTabelle_[kopf - tkopf].size() + f; s++) {
      if (s < f) { // Farbdarstellung
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

std::vector<std::vector<std::string> >
ICCmeasurement::getText                     (void)
{ DBG_PROG_START
  DBG_NUM_V( RGB_MessFarben_.size() )
  if (RGB_MessFarben_.size() == 0)
    init ();

  // push_back ist zu langsam
  std::vector<std::vector<std::string> > tabelle (1);
  std::stringstream s;
  int z = 0; // Zeilen

  tabelle[0].resize(1);
  tabelle[0][0] = _("no measurment data available");

  if ((CMYK_measurement_ || RGB_measurement_)
       && XYZ_measurement_) {
    tabelle.resize(nFelder_+5); // push_back ist zu langsam
    // Tabellenüberschrift
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
    // Tabellenkopf
    int spalten, sp = 0, xyz_erg_sp = 0;
    int h = false;
    if (XYZ_Ergebnis_.size() == XYZ_Satz_.size())
      xyz_erg_sp = 3;
#   define HI (h == true) ? h-- : h++ // invertieren
    layout.clear();
    layout.push_back (HI); // Messfeld
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
    // Messwerte
    s.str("");
#   define DBG_TAB_V(txt)
    for (int i = 0; i < nFelder_; i++) { 
      sp = 0;
      tabelle[z+i].resize( spalten );
      tabelle[z+i][sp++] =  Feldnamen_[i]; DBG_TAB_V ( z <<" "<< sp <<" "<< Lab_Differenz_.size() )
      s << Lab_Differenz_[i]; tabelle[z+i][sp++] = s.str().c_str(); s.str("");
      s << DE00_Differenz_[i]; tabelle[z+i][sp++] = s.str().c_str(); s.str("");
      s << Lab_Satz_[i].L; tabelle[z+i][sp++] = s.str().c_str(); s.str("");
      s << Lab_Satz_[i].a; tabelle[z+i][sp++] = s.str().c_str(); s.str("");
      s << Lab_Satz_[i].b; tabelle[z+i][sp++] = s.str().c_str(); s.str("");
      s << Lab_Ergebnis_[i].L; tabelle[z+i][sp++] = s.str().c_str(); s.str("");
      s << Lab_Ergebnis_[i].a; tabelle[z+i][sp++] = s.str().c_str(); s.str("");
      s << Lab_Ergebnis_[i].b; tabelle[z+i][sp++] = s.str().c_str(); s.str("");
      s << XYZ_Satz_[i].X*100; tabelle[z+i][sp++] = s.str().c_str(); s.str("");
      s << XYZ_Satz_[i].Y*100; tabelle[z+i][sp++] = s.str().c_str(); s.str("");
      s << XYZ_Satz_[i].Z*100; tabelle[z+i][sp++] = s.str().c_str(); s.str("");
      if (xyz_erg_sp) {
      s << XYZ_Ergebnis_[i].X*100; tabelle[z+i][sp++]=s.str().c_str();s.str("");
      s << XYZ_Ergebnis_[i].Y*100; tabelle[z+i][sp++]=s.str().c_str();s.str("");
      s << XYZ_Ergebnis_[i].Z*100; tabelle[z+i][sp++]=s.str().c_str();s.str("");
      } DBG_TAB_V( i )
      if (RGB_measurement_) {
        s << RGB_Satz_[i].R*255; tabelle[z+i][sp++]= s.str().c_str(); s.str("");
        s << RGB_Satz_[i].G*255; tabelle[z+i][sp++]= s.str().c_str(); s.str("");
        s << RGB_Satz_[i].B*255; tabelle[z+i][sp++]= s.str().c_str(); s.str("");
      } else {
        s << CMYK_Satz_[i].C*100; tabelle[z+i][sp++]=s.str().c_str(); s.str("");
        s << CMYK_Satz_[i].M*100; tabelle[z+i][sp++]=s.str().c_str(); s.str("");
        s << CMYK_Satz_[i].Y*100; tabelle[z+i][sp++]=s.str().c_str(); s.str("");
        s << CMYK_Satz_[i].K*100; tabelle[z+i][sp++]=s.str().c_str(); s.str("");
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

std::vector<std::string>
ICCmeasurement::getDescription              (void)
{ DBG_PROG_START
  std::vector<std::string> texte;
  std::string text =  "";

# ifdef DEBUG_ICCMEASUREMENT
# endif

  DBG_PROG_ENDE
  return texte;
}

std::vector<double>
ICCmeasurement::getMessRGB                  (int patch)
{ DBG_MESS_START
  std::vector<double> punkte(3);

  if (RGB_MessFarben_.size() == 0)
    init ();

  if (patch > nFelder_) {
    WARN_S( "Patch Nr: " << patch << " ausserhalb des Messfarbsatzes" )
    DBG_MESS_ENDE
    return punkte;
  }

  punkte[0] = RGB_MessFarben_[patch].R; DBG_MESS_V( RGB_MessFarben_[patch].R <<  punkte[0] )
  punkte[1] = RGB_MessFarben_[patch].G; DBG_MESS_V( RGB_MessFarben_[patch].G )
  punkte[2] = RGB_MessFarben_[patch].B; DBG_MESS_V( RGB_MessFarben_[patch].B )

  DBG_MESS_ENDE
  return punkte;
}

std::vector<double>
ICCmeasurement::getCmmRGB                   (int patch)
{ DBG_MESS_START
  std::vector<double> punkte (3) ;

  if (RGB_MessFarben_.size() == 0)
    init ();

  if (patch > nFelder_) {
    WARN_S( "Patch Nr: " << patch << " ausserhalb des Messfarbsatzes" )
    DBG_MESS_ENDE
    return punkte;
  }

  punkte[0] = RGB_ProfilFarben_[patch].R;
  punkte[1] = RGB_ProfilFarben_[patch].G;
  punkte[2] = RGB_ProfilFarben_[patch].B;

  DBG_MESS_ENDE
  return punkte;
}

std::vector<double>
ICCmeasurement::getMessLab                  (int patch)
{ DBG_MESS_START
  std::vector<double> punkte (3) ;

  if (Lab_Satz_.size() == 0)
    init ();

  if (patch > nFelder_) {
    WARN_S( "Patch Nr: " << patch << " ausserhalb des Messfarbsatzes" )
    DBG_MESS_ENDE
    return punkte;
  }

  punkte[0] =  Lab_Satz_[patch].L          / 100.0;
  punkte[1] = (Lab_Satz_[patch].a + 128.0) / 255.0;
  punkte[2] = (Lab_Satz_[patch].b + 128.0) / 255.0;

  DBG_MESS_ENDE
  return punkte;
}

std::vector<double>
ICCmeasurement::getCmmLab                   (int patch)
{ DBG_MESS_START
  std::vector<double> punkte (3) ;

  if (Lab_Ergebnis_.size() == 0)
    init ();

  if (patch > nFelder_) {
    WARN_S( "Patch Nr: " << patch << " ausserhalb des Messfarbsatzes" )
    DBG_MESS_ENDE
    return punkte;
  }

  punkte[0] =  Lab_Ergebnis_[patch].L          / 100.0;
  punkte[1] = (Lab_Ergebnis_[patch].a + 128.0) / 255.0;
  punkte[2] = (Lab_Ergebnis_[patch].b + 128.0) / 255.0;

  DBG_MESS_ENDE
  return punkte;
}


