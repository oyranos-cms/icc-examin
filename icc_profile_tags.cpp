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
 * profile interpretation
 * 
 */

// Date:      04. 05. 2004

#if 0
# ifndef DEBUG
#  define DEBUG
# endif
# define DEBUG_ICCTAG
#endif

#include "icc_profile_tags.h"
#include "icc_profile.h"
#include "icc_utils.h"
#include "icc_helfer.h"
#include <oyranos/oyranos_alpha.h>

#include <cmath>
#include <sstream>

#define g_message printf

struct Ncl2Farbe {
  char name[32];
  icUInt16Number pcsfarbe[3]; // PCS Lab oder XYZ
  icUInt16Number geraetefarbe[16];
};

struct Ncl2 {
  char vendor_flag[4];
  icUInt32Number anzahl;
  icUInt32Number koord;
  char vorname[32];
  char nachname[32];
  Ncl2Farbe *farben;
};


/**
  *  @brief ICCtag functions
  */

ICCtag::ICCtag                      (ICCprofile* profil, icTag* tag, char* data)
{ DBG_PROG_START
  DBG_PROG_S("ICCtag::ICCtag ICCprofile* , icTag* , char*  - beginn")
  ICCtag::load (profil, tag, data); DBG_NUM_S("ICCtag::ICCtag ICCprofile* , icTag* , char*  - fertig")
  DBG_PROG_ENDE
}

void
ICCtag::copy                        ( const ICCtag& tag)
{
  _sig = tag._sig;
  size_ = tag.size_;
  DBG_PROG_S("ICCtag::ICCtag <- Kopie size_: " << size_ )
  if (size_ && tag.data_) {
    data_ = (char*)calloc(sizeof(char),size_);
    memcpy (data_ , tag.data_ , size_);
    DBG_MEM_S((int*)tag.data_ << " -> " << (int*)data_)
  } else {
    data_ = NULL;
    size_ = 0;
  }

  _intent = tag._intent;
  _color_in = tag._color_in;
  _color_out = tag._color_out;

  _profil = tag._profil;
}

void
ICCtag::defaults ()
{
  DBG_PROG
  _sig = icMaxEnumTag;
  size_ = 0;
  data_ = NULL;
  _intent = 0;
  _color_in = icMaxEnumData;
  _color_out = icMaxEnumData;
  _profil = NULL;
  if (size_) DBG_MEM_V ((int*)data_) ;
}

void
ICCtag::clear              ()
{
  DBG_PROG
  if (data_ && size_) {
    DBG_MEM_S("erase: "<<(int*)data_)
    free (data_);
  }
  defaults();
}

void
ICCtag::load                        ( ICCprofile *profil,
                                      icTag      *tag,
                                      char       *data )
{ DBG_MEM_START
  DBG_MEM_V( profil )
  DBG_MEM_V( tag )
  DBG_MEM_V( data )
  _profil = profil;
  _sig    = icValue(tag->sig); //DBG_PROG_S( getSigTagName(_sig) )
  switch (_sig) {
  case icSigAToB0Tag:
  case icSigBToA0Tag:
  case icSigPreview0Tag:
    _intent = 0; break;
  case icSigAToB1Tag:
  case icSigBToA1Tag:
  case icSigPreview1Tag:
    _intent = 1; break;
  case icSigAToB2Tag:
  case icSigBToA2Tag:
  case icSigPreview2Tag:
    _intent = 2; break;
  default:
    _intent = -1;
    break;
  }

  switch (_sig) {
  case icSigAToB0Tag:
  case icSigAToB1Tag:
  case icSigAToB2Tag:
    _color_in = _profil->header.colorSpace();
    _color_out = _profil->header.pcs();
    _to_pcs = true;
    break;
  case icSigBToA0Tag:
  case icSigBToA1Tag:
  case icSigBToA2Tag:
    _color_in = _profil->header.pcs();
    _color_out = _profil->header.colorSpace();
    _to_pcs = false;
    break;
  case icSigGamutTag:
    _color_in = _profil->header.pcs(); _color_out = (icColorSpaceSignature)0;
    _to_pcs = false;
    break;
  case icSigPreview0Tag:
  case icSigPreview1Tag:
  case icSigPreview2Tag:
    _color_in = _profil->header.pcs(); _color_out = _profil->header.pcs();
    _to_pcs = true;
    break;
  default:
    _color_in = (icColorSpaceSignature)0; _color_out = (icColorSpaceSignature)0;
    _to_pcs = true;
    break;
  }

  DBG_MEM

  if (data_ != NULL && size_) { DBG_MEM
    DBG_MEM_S( "ICCtag reused: " << (char*)tag->sig << " " )
    DBG_MEM
    free(data_); DBG_MEM // delete [] data_;
    data_ = NULL;
    DBG_MEM_S( "ICCtag reused: " << (char*)tag->sig << " " )
  } DBG_MEM

  size_   = icValue(tag->size); DBG_MEM_V( size_ )
  if(size_ < 0 || size_ > 100000000) {
    WARN_S( "tag tried to allocate " << size_ << " Byte: ignored ");
    size_ = 0;
    data_ = NULL;
  } else {
    if(size_ && data)
    {
      data_ = (char*) calloc(sizeof(char),size_); // new char (size_);
      memcpy ( data_ , data , size_ );
    } else {
      size_ = 0;
      data_ = NULL;
    }
  }
  DBG_MEM_V((int*)data_)

# ifdef DEBUG_ICCTAG_
  char* text = data_;
  DBG_MEM_S( _sig << "=" << tag->sig << " offset " << icValue(tag->offset) << " size " << size_ << " next tag " << size_ + icValue(tag->offset) << " " << text << " " )
# endif
  DBG_MEM_ENDE
}

std::vector<std::string>
ICCtag::getText                     (void)
{ DBG_PROG_START
  std::vector<std::string> texte;
  std::string type = getTypName();

  texte = this->getText (type);
  DBG_PROG_ENDE
  return texte;
}

std::vector<std::string>
ICCtag::getText                     (std::string text)
{ DBG_PROG_START
  std::vector<std::string> texte;

  if (data_ == NULL || !size_)
  { DBG_PROG_ENDE
    return texte;
  }

  if (text == "sig") {

    if (size_ < 12) { DBG_PROG_ENDE return texte; }
    icTechnologySignature tech;
    memcpy (&tech, &data_[8] , 4);
    text = getSigTechnology( (icTechnologySignature) icValue(tech) );
    texte.push_back( text );

  } else if (text == "dtim") {

    if (size_ < 20) { DBG_PROG_ENDE return texte; }
    DBG_PROG
    icDateTimeNumber date;
    memcpy (&date, &data_[8] , 12);
    texte.push_back( printDatum(date) );

  } else if (text == "meas") {

    if (size_ < 36) { DBG_PROG_ENDE return texte; }
    std::stringstream s;
    icMeasurement meas;
    memcpy (&meas, &data_[8] , 28);
    s << _("Standard Observer") << ": " <<
    getStandardObserver( icValue( meas.stdObserver) ) <<endl
      // "Back side" or Background
      << _("Back side") << ": X = " << icSFValue(meas.backing.X)
                        << ", Y = " << icSFValue(meas.backing.Y)
                        << ", Z = " << icSFValue(meas.backing.Z) << endl
      << _("Geometrie") << ": "<< 
    getMeasurementGeometry (icValue(meas.geometry))<<endl
      << _("Flare")     << ": "<< 
    getMeasurementFlare (icValue(meas.flare)) << endl
      << _("Illuminant Type") << ": " <<
    getIlluminant (icValue(meas.illuminant)) <<endl;
    texte.push_back( s.str() );

  } else if (text == "mft2") {

    icLut16* lut16 = (icLut16*) &data_[8];
    int inputChan, outputChan, clutPoints, inputEnt, outputEnt;

    inputChan = (int)lut16->inputChan;
    outputChan = (int)lut16->outputChan;
    clutPoints = (int)lut16->clutPoints;
    inputEnt = icValue(lut16->inputEnt);
    outputEnt = icValue(lut16->outputEnt);
    std::stringstream s;
    s << _("Conversion table with 16-bit precission:") << endl <<
         _("Intent:") << " " << renderingIntentName(_intent) << endl <<
         _("Number of input channels") << " (" << getColorSpaceName(_color_in)  << "): " << (int)inputChan << endl <<
         _("Number of output channels") << " (" << getColorSpaceName(_color_out) << "): " << (int)outputChan << endl <<
         _("Matrix") << endl <<
         _("Linear input curve") << " " << _("with") << " " << (int)inputEnt << " " << _("steps") << endl <<
         _("3D Lockup table with") << " " <<  (int)clutPoints << " " << _("points length per side") << endl <<
         _("Linear output curve") << " " << _("with") << " " << (int)outputEnt << " " << _("steps") << endl;
    texte.push_back( s.str() );

  } else if (text == "mft1") {

    icLut8* lut8 = (icLut8*) &data_[8];
    int inputChan, outputChan, clutPoints;//, inputEnt, outputEnt;
    inputChan = (int)lut8->inputChan;
    outputChan = (int)lut8->outputChan;
    clutPoints = (int)lut8->clutPoints;

    std::stringstream s;
    s << _("Conversion table with 8-bit precission:") << endl <<
         _("Intent:") << " " << renderingIntentName(_intent) << endl <<
         _("Number of input channels") << " (" << getColorSpaceName(_color_in)  << "): " << (int)inputChan << endl <<
         _("Number of output channels") << " (" << getColorSpaceName(_color_out) << "): " << (int)outputChan << endl <<
         _("Matrix") << endl <<
         _("3D Lockup table with") << " " <<  (int)clutPoints << " " << _("points length per side") << endl;
    texte.push_back( s.str() );

  } else if (((icTagBase*)&data_[0])->sig == (icTagTypeSignature)icValue( icSigChromaticityType )) {

    int count = icValue(*(icUInt16Number*)&data_[8]);
    if (count == 0)
      count = 3;
#   ifdef DEBUG_ICCTAG
    DBG_NUM_S( count )
#   endif
    for (int i = 0; i < count ; ++i) { // Table 35 -- chromaticityType encoding
      std::stringstream s;
      s << _("Channel") << " "<< i;
      texte.push_back( s.str() );
#     ifdef DEBUG_ICCTAG
      DBG_NUM_S(  cout << s.str() )
#     endif
    }
    std::vector<double> xy = getCIEXYZ(); XYZto_xyY(xy);
    if( icValue(*(icUInt16Number*)&data_[10]) ) 
      texte.push_back( getChromaticityColorantType( icValue(*(icUInt16Number*)&data_[10])) );
    else if(!(xy.size()%3))
    {
      std::stringstream s;
      for(unsigned int i = 0; i < xy.size()/3; ++i) {
        s << texte[i] << " " << "xy: "<< xy[i*3+0] <<" "<< xy[i*3+1];
        texte.push_back(s.str()); s.str("");
      }
    }
    DBG_V( xy.size()%3 )
    texte.push_back( "chrm" );

  } else  if (text == "text"
           || text == "cprt?" ) { // text

    text = ""; DBG_PROG

    int count = 0;
# if 1
    char* txt = (char*)calloc (size_-8, sizeof(char));
    memcpy (txt, &data_[8], size_ - 8);
    char* pos = 0;
#   ifdef DEBUG_ICCTAG
    DBG_NUM_S ((int)strchr(txt, 13))
#   endif
    while (strchr(txt, 13) > 0) { // \r 013 0x0d
      pos = strchr(txt, 13);
#     ifdef DEBUG_ICCTAG
      //cout << (int)pos << " "; DBG
#     endif
      if (pos > 0) {
        if (*(pos+1) == '\n')
          *pos = ' ';
        else
          *pos = '\n';
      }
      count++;
    };
    text = txt;
    free (txt);
# else
    text.append (&data_[8], size_ - 8);
    int pos = 0;
#   ifdef DEBUG_ICCTAG
    DEB_S( (int)text.find('\r') )
#   endif
    while ((int)text.find('\r') > 0) { // \r 013 0x0d
      pos = (int)text.find('\r');
#     ifdef DEBUG_ICCTAG
      DBG_NUM_V( pos )
#     endif
      if (pos > 0)
        //text.erase (pos);
        text.replace (pos, 1, ' ', 1); 
      count++;
    };
# endif

    texte.push_back( text );

  } else if ( text == "desc" ) {

    text =  "";
    icUInt32Number count = *(icUInt32Number*)(data_+8);
    count = icValue(count);
    DBG_PROG_V( count <<" "<< data_+12 )

    if((int)count > size_- 8)
    {
      int diff = count - size_ - 8;
      char nt[128];
      snprintf( nt, 128, "%d", diff );

      text.append (_("Error in ICC profile tag found!"));
      text.append ("\n Wrong \"desc\" tag count. Difference is :\n   ");
      text.append (nt);
      text.append ("\n Try ordinary tag length instead (?):\n   ");
      char * txt = new char [size_ - 8];
      memcpy (txt, &data_[12], size_ - 12);
      txt[size_ - 12] = 0;
      text.append (txt);
      delete [] txt;
    }
    else
      text.append ((const char*)(data_+12), count);
# ifdef DEBUG_ICCTAG
    DBG_NUM_S ( &data_[12] << "|" << "|" << text )
# endif

    texte.push_back( text );

  } else if ( text == "mluc" ) { // i18n

    int anzahl = icValue( *(icUInt32Number*)&data_[8] );
    int groesse = icValue( *(icUInt32Number*)&data_[12] ); // 12
    char a = 'e';
    char b = 'n';
    doLocked_m(const char *locale = getenv("LANG");,NULL)
    if(locale)
      DBG_NUM_V( locale )
    else
      DBG_NUM_S( "keine LANG Variable gefunden" )
    
    if(locale && strlen(locale) >= 2 )
    {
      a = locale[0];
      b = locale[1];
    }

    for (int i = 0; i < anzahl; i++)
    {
      char c = data_[16+ i*groesse],
           d = data_[17+ i*groesse];
      int  g = icValue(*(icUInt32Number*)&data_[20+ i*groesse]);
      DBG_PROG_V( c << d << g )

      if (c == a && d == b)
      {
        int dversatz = icValue(*(icUInt32Number*)&data_[24+ i*groesse]);
        char *t = (char*) new char [g];
        int n, j;
        size_t size = 0;
        icUInt16Number * uni16be = (icUInt16Number*) &data_[dversatz];
        wchar_t *wc = (wchar_t*) new wchar_t [g];
#if 1
        for( j = 1; j < g/2; ++j)
          wc[j] = icValue( uni16be[j] );
        wc[j] = 0;
        size = wcstombs( t, (const wchar_t*)uni16be, g );
        DBG_V( size<<" "<<&data_[dversatz]<<" "<<&data_[dversatz+1]<<" "<< t );

        for (n = 0; n < g ; n = n+2)
          t[n/2] = (char)icValue( *(icUInt16Number*)&data_[dversatz + n] );
        t[n/2] = 0;
#endif
        texte.push_back( t );
        delete [] t; delete [] wc; DBG_PROG_V( g <<" "<< dversatz )
      }
    }
    if (!texte.size()) // first entry
    { int g =        icValue(*(icUInt32Number*)&data_[20]),
          dversatz = icValue(*(icUInt32Number*)&data_[24]);
      char *t = (char*) new char [g];
      int n;
      for (n = 1; n < g ; n = n+2)
        t[n/2] = data_[dversatz + n];
      t[n/2] = 0;
      texte.push_back( t );
      delete [] t; DBG_PROG_V( g <<" "<< dversatz )
    }

  } else if ( text == "vcgt" ) {

    texte.push_back( _("Red") );
    texte.push_back( _("Green") );
    texte.push_back( _("Blue") );
    texte.push_back( "gamma_start_ende" );

  } else if ( text == "sf32" ) {

    texte .resize(1);
    texte[0].append ("\n\n  ",4);
    char t[16];
    for (int i = 0; i < size_-8; i = i + 4)
    { sprintf (t, "%f ", icSFValue (*(icS15Fixed16Number*)(&data_[8+i]) ) );
      if (t[0] != '-')
        texte[0].append (" ", 1);
      texte[0].append (t, strlen(t));
      if (!((i/4+1)%3))
        texte[0].append ("\n  ", 3);
      else
        texte[0].append (" ", 1);
    }

  } else if ( text == "ncl2" ||
              text == "ncl2_names" ) {

    Ncl2 *ncl2 = (Ncl2*) &data_[8];
    std::stringstream s;

    int farben_n        = icValue(ncl2->anzahl);
    int geraetefarben_n = icValue(ncl2->koord);
    if( text == "ncl2" )
    {
      texte.resize(1);
      s << "\n\n   " <<
         _("Number of colours:") << icValue(ncl2->anzahl) << "\n" <<
           "   " << _("Name") << "    " << _("CIE*Lab") <<
           " / " << _("Device Colours") << "\n\n";
      texte[0] = s.str();
    }
    DBG_MEM_V( texte[0] )
    DBG_MEM_V( sizeof(Ncl2)+icValue(ncl2->anzahl)*sizeof(Ncl2Farbe) )
    DBG_MEM_V( sizeof(Ncl2Farbe) )
    DBG_MEM_V( sizeof(Ncl2) )
    for (int i = 0; i < farben_n; ++i)
    {
      Ncl2Farbe *f = (Ncl2Farbe*) ((char*)ncl2 + 76 + // base site of Ncl2
                     (i * (38 +                 // base size of Ncl2Farbe
                           geraetefarben_n      // number of device colours
                           * sizeof(icUInt16Number))));//Ncl2Farbe::geraetefarbe
      DBG_MEM_V( sizeof(icUInt16Number) <<"|"<< geraetefarben_n )
      DBG_MEM_V( i <<" "<<(int*)f <<" "<< (int*)ncl2  )
      s << "" <<
           ncl2->vorname << f->name << ncl2->nachname;// max 31 byte
      if( text == "ncl2" )
      {
        s <<" ";
        s << icValue(f->pcsfarbe[0]) << " " <<
             icValue(f->pcsfarbe[1]) << " " <<
             icValue(f->pcsfarbe[2]) << " | ";
        for(int j=0; j < geraetefarben_n; ++j)
          s << icValue(f->geraetefarbe[j]) << " ";
        s << "\n";
      } else {
        texte.push_back(s.str());
        s.str("");
      }
    }
    if( text == "ncl2" )
      texte[0] = s.str();

  } else {

    oyProfileTag_s * tag_ = oyProfileTag_New( 0 );
    icTagBase *base  = (icTagBase*)(&data_[0]);
    icTagTypeSignature tag_type = icValue( base->sig );
    oySTATUS_e status = oyOK;
    int error = oyProfileTag_Set( tag_, _sig, tag_type,
                                  status, size_, base );
    oyChar **texts = 0;
    int    texts_n = 0;
    int    profiles_n = 0;

    sprintf( tag_->required_cmm, "oyra" );

    if(!error)
      texts = oyProfileTag_GetText( tag_,&texts_n,0,0,0,0 );

    texte .resize(1);

    for (int i = 0; i < texts_n; ++i)
    {
      int line_break = 1;

      if(i == 0)
      {
        const char * t = _("Profiles where originally involved.");
        profiles_n = atoi(texts[i]);
        texte[0].append (texts[i], strlen(texts[i]));
        texte[0].append (" ", 1);
        texte[0].append (t, strlen(t));
        texte[0].append (":", 1);
      }
      else
      {
        if(((i-1)%7))
        {
          texte[0].append ("  ", 2);

          if((((i-1)%7)-1)%2)
            texte[0].append ("    ", 4);
          else
            line_break = 1;
        }

        if(texts[i])
          texte[0].append (texts[i], strlen(texts[i]));
      }
      if(line_break)
        texte[0].append ("\n", 1);
    }

    if(!texts_n || !texte[0].size())
    {
      texte[0].append ("\n\n",2);

      for (int i = 0; i < size_-8; i = i + 4)
      {
        texte[0].append (" ", 1);
        text = zeig_bits_bin(&data_[8+i], MIN(4,size_-8-i));
        texte[0].append (text.data(), text.size());
        for (int k = 0; k <  MIN(4,size_-8-i); k++)
          if (isprint(data_[8+i+k]))
            text[k] = data_[8+i+k];
          else
            text[k] = '.';
        //text[MIN(4,size_-8-i)] = 0;
        texte[0].append (text.data(), MIN(4,size_-8-i));//text.size());
        texte[0].append ("\n", 1);
      }
    }
    DBG_PROG
    //char c[5]; sprintf (c, "%s", "mluc"); printf ("%d\n",icValue(*(int*)c));
  }
    
# ifdef DEBUG_ICCTAG
  DBG_NUM_S( count << " Ersetzungen " << "|" << getTypName() << "|" << text )
# endif

  DBG_PROG_ENDE
  return texte;
}


/*
std::vector<std::string>
ICCtag::getDescription              (void)
{ DBG_PROG_START
  std::vector<std::string> texte;
  std::string text =  "";
  icUInt32Number count = *(icUInt32Number*)(data_+8);

  count = icValue(count);

  text.append ((const char*)(data_+12), count);
  texte.push_back (text);
# ifdef DEBUG_ICCTAG
  DBG_NUM_S ( &data_[12] << "|" << "|" << text )
# endif
  DBG_PROG_ENDE
  return texte;
}*/

std::vector<double>
ICCtag::getCIEXYZ                                 (void)
{ DBG_PROG_START
  std::vector<double> punkte;
  icTagBase *base  = (icTagBase*)(&data_[0]);

  if ((base->sig) == (icTagTypeSignature)icValue( icSigChromaticityType )) {
    int count = icValue(*(icUInt16Number*)&data_[8]);
#   ifdef DEBUG_ICCTAG
    DBG_NUM_S( count )
#   endif
    DBG_S(getChromaticityColorantType( icValue(*(icUInt16Number*)&data_[10])))
    for (int i = 0; i < count ; ++i) { // Table 35 -- chromaticityType encoding
      // TODO lcms needs a 16 Byte Offset (instead of 12 Byte ?)
      icU16Fixed16Number* channel = (icU16Fixed16Number*)&data_[12+(8*i)];
      double xyY[3] = { icUFValue( channel[0] ),
                        icUFValue( channel[1] ),
                        1.0 };
      punkte.push_back( xyY[0] );
      punkte.push_back( xyY[1] );
      punkte.push_back( xyY[2] );
#     ifdef DEBUG_ICCTAG
      DBG_NUM_S( xyY[0] << ", " << xyY[1] << ", " << xyY[2] )
#     endif
      DBG_S( xyY[0] << ", " << xyY[1] << ", " << xyY[2] )
    }
    DBG_V( punkte.size() )
    xyYto_XYZ(punkte);
  } else if (base->sig == (icTagTypeSignature)icValue( icSigXYZType )) {
    icXYZType *daten = (icXYZType*) &data_[0];
    punkte.push_back( icSFValue( (daten->data.data[0].X) ) );
    punkte.push_back( icSFValue( (daten->data.data[0].Y) ) );
    punkte.push_back( icSFValue( (daten->data.data[0].Z) ) );
  }

  DBG_PROG_ENDE
  return punkte;
}

std::vector<double>
ICCtag::getCurve                                  (void)
{ DBG_PROG_START
  std::vector<double> punkte;
  icCurveType *daten = (icCurveType*) &data_[0];
  int count = icValue(daten->curve.count);

  if (count == 1) { // icU16Fixed16Number
    punkte.push_back (icValue(daten->curve.data[0])/256.0);
  } else {
    for (int i = 0; i < count; i++)
      punkte.push_back (icValue(daten->curve.data[i])/65536.0);
  }

  DBG_PROG_ENDE
  return punkte;
}

std::vector<std::vector<double> >
ICCtag::getCurves                                 (MftChain typ)
{ DBG_PROG_START
  std::vector<double> kurve;
  std::vector<std::vector<double> > kurven; DBG_PROG
  // Who are we?
  if (getTypName() == "mft2") {
    icLut16* lut16 = (icLut16*) &data_[8];
    int inputChan, outputChan, clutPoints, inputEnt, outputEnt;
    inputChan = (int)lut16->inputChan;
    outputChan = (int)lut16->outputChan;
    clutPoints = (int)lut16->clutPoints;
    inputEnt = icValue(lut16->inputEnt);
    outputEnt = icValue(lut16->outputEnt);
    int feldPunkte = (int)pow((double)clutPoints, inputChan);
#   ifdef DEBUG_ICCTAG
    DBG_NUM_S( feldPunkte << " array points " << clutPoints << " clutPoints" )
#   endif
    int start = 52,
        byte  = 2;
    double div   = 65536.0;
    DBG_PROG
    // What is requested?
    switch (typ) {
    case MATRIX:
         break;
    case CURVE_IN: DBG_PROG
         for (int j = 0; j < inputChan; j++)
         { kurve.clear();
#          ifdef DEBUG_ICCTAG
           DBG_NUM_S( kurve.size() << " Start" )
#          endif
           for (int i = inputEnt * j; i < inputEnt * (j+1); i++) {
             kurve.push_back( (double)icValue (*(icUInt16Number*)&data_[start + byte*i])
                              / div );
#            ifdef DEBUG_ICCTAG
             DBG_NUM_S( icValue (*(icUInt16Number*)&data_[start + byte*i]) )
#            endif
           }
           kurven.push_back (kurve);
#          ifdef DEBUG_ICCTAG
           DBG_NUM_S( kurve.size() << " entries" )
#          endif
         } DBG_PROG
         break;
    case TABLE_IN:
    case TABLE_OUT:
    case TABLE: DBG_PROG
         start += (inputChan * inputEnt) * byte;
         for (int i = 0; i < feldPunkte * outputChan; i++)
           kurve.push_back( (double)icValue (*(icUInt16Number*)&data_[start + byte*i])
                            / div );
         break;
    case CURVE_OUT: DBG_PROG
         start += (inputChan * inputEnt + feldPunkte * outputChan) * byte;
         for (int j = 0; j < outputChan; j++)
         { kurve.clear();
           for (int i = outputEnt * j; i < outputEnt * (j+1); i++)
             kurve.push_back( (double)icValue (*(icUInt16Number*)&data_[start + byte*i])
                              / div );
           kurven.push_back (kurve);
#          ifdef DEBUG_ICCTAG
           DBG_NUM_S( kurve.size() << "|" << outputEnt << " entries" )
#          endif
         }
         break;
    } 
  } else if (getTypName() == "mft1") {
    icLut8* lut8 = (icLut8*) &data_[8];
    int inputChan, outputChan, clutPoints, inputEnt=256, outputEnt=256;
    inputChan = (int)lut8->inputChan;
    outputChan = (int)lut8->outputChan;
    clutPoints = (int)lut8->clutPoints;
    int feldPunkte = (int)pow((double)clutPoints, inputChan);
    int start = 48,
        byte  = 1;
    double div   = 255.0;

    // What is requested?
    switch (typ) {
    case MATRIX:
         break;
    case CURVE_IN:
         for (int j = 0; j < inputChan; j++)
         { kurve.clear();
#          ifdef DEBUG_ICCTAG
           DBG_NUM_S( kurve.size() << " Start" )
#          endif
           for (int i = inputEnt * j; i < inputEnt * (j+1); i++)
             kurve.push_back( (double) *(icUInt8Number*)&data_[start + byte*i]
                              / div );
           kurven.push_back (kurve);
#          ifdef DEBUG_ICCTAG
           DBG_NUM_S( kurve.size() << " entries" )
#          endif
         }
         break;
    case TABLE_IN:
    case TABLE_OUT:
    case TABLE:
         start += (inputChan * inputEnt) * byte;
         for (int i = 0; i < feldPunkte * outputChan; i++)
           kurve.push_back( (double) *(icUInt8Number*)&data_[start + byte*i]
                            / div );
         break;
    case CURVE_OUT:
         start += (inputChan * inputEnt + feldPunkte * outputChan) * byte;
         for (int j = 0; j < outputChan; j++)
         { kurve.clear();
#          ifdef DEBUG_ICCTAG
           DBG_NUM_S( kurve.size() << " Start" )
#          endif
           for (int i = outputEnt * j; i < outputEnt * (j+1); i++)
             kurve.push_back( (double) *(icUInt8Number*)&data_[start + byte*i]
                              / div );
           kurven.push_back (kurve);
#          ifdef DEBUG_ICCTAG
           DBG_NUM_S( kurve.size() << " entries" )
#          endif
         }
         break;
    } 
  } else if (getTypName() == "vcgt") {
    int parametrisch        = icValue(*(icUInt32Number*) &data_[8]);
    icUInt16Number nkurven  = icValue(*(icUInt16Number*) &data_[12]);
    icUInt16Number segmente = icValue(*(icUInt16Number*) &data_[14]);
    icUInt16Number byte     = icValue(*(icUInt16Number*) &data_[16]);
    
#   ifdef DEBUG_ICCTAG
    DBG_NUM_S( data_ << " parametric " << parametrisch << " ncurves " << nkurven << " segments " << segmente << " byte " << byte )
#   endif

    if (parametrisch) { //icU16Fixed16Number
      double r_gamma = 1.0/icValue(*(icUInt32Number*)&data_[12])*65536.0;
      double start_r = icValue(*(icUInt32Number*)&data_[16])/65536.0;
      double ende_r = icValue(*(icUInt32Number*)&data_[20])/65536.0;
      double g_gamma = 1.0/icValue(*(icUInt32Number*)&data_[24])*65536.0;
      double start_g = icValue(*(icUInt32Number*)&data_[28])/65536.0;
      double ende_g = icValue(*(icUInt32Number*)&data_[32])/65536.0;
      double b_gamma = 1.0/icValue(*(icUInt32Number*)&data_[36])*65536.0;
      double start_b = icValue(*(icUInt32Number*)&data_[40])/65536.0;
      double ende_b = icValue(*(icUInt32Number*)&data_[44])/65536.0;
      kurve.clear();
      kurve.push_back(r_gamma);
      kurve.push_back(start_r);
      kurve.push_back(ende_r); kurven.push_back (kurve); kurve.clear();
      kurve.push_back(g_gamma);
      kurve.push_back(start_g);
      kurve.push_back(ende_g); kurven.push_back (kurve); kurve.clear();
      kurve.push_back(b_gamma);
      kurve.push_back(start_b);
      kurve.push_back(ende_b); kurven.push_back (kurve); kurve.clear();
    } else {
      int start = 18;
      double div   = 65536.0;
           for (int j = 0; j < nkurven; j++)
           { kurve.clear();
#            ifdef DEBUG_ICCTAG
             DBG_NUM_S( kurve.size() << " Start" )
#            endif
             for (int i = segmente * j; i < segmente * (j+1); i++)
               kurve.push_back( (double) icValue (*(icUInt16Number*)&data_[start + byte*i])
                                / div );
             kurven.push_back (kurve);
             //#ifdef DEBUG_ICCTAG
             DBG_NUM_S( kurve.size() << " entries" )
             //#endif
           }
    }
  }

# ifdef DEBUG_ICCTAG
  DBG_NUM_V( kurven.size() )
# endif
  DBG_PROG_ENDE
  return kurven;
}

std::vector<std::vector<std::vector<std::vector<double> > > >
ICCtag::getTable                                 (MftChain typ)
{ DBG_PROG_START
  std::vector<std::vector<std::vector<std::vector<double> > > > Tabelle;
  std::vector<double> Farbe; DBG_PROG
  // Wer sind wir?
  if (getTypName() == "mft2") {
    icLut16* lut16 = (icLut16*) &data_[8];
    int inputChan, outputChan, clutPoints, inputEnt, outputEnt;
    inputChan = (int)lut16->inputChan;
    outputChan = (int)lut16->outputChan;
    clutPoints = (int)lut16->clutPoints;
    inputEnt = icValue(lut16->inputEnt);
    outputEnt = icValue(lut16->outputEnt);
#   ifdef DEBUG_ICCTAG
    int feldPunkte = (int)pow((double)clutPoints, inputChan);
    DBG_NUM_S( feldPunkte << " array points " << clutPoints << " clutPoints" )
#   endif
    int start = 52,
        byte  = 2;
    double div   = 65536.0;
    DBG_PROG
    // What is requested?
    if (inputChan == 3)
    {
      switch (typ) {
      case TABLE_IN:
      case TABLE_OUT:
      case TABLE: { DBG_PROG
           start += (inputChan * inputEnt) * byte;
           Tabelle.resize(clutPoints);
           for (int i = 0; i < clutPoints; i++) {
             Tabelle[i].resize(clutPoints);
             for (int j = 0; j < clutPoints; j++) {
               Tabelle[i][j].resize(clutPoints);
               for (int k = 0; k < clutPoints; k++)
                 Tabelle[i][j][k].resize(outputChan);
             }
           }
           int n = 0;
           for (int i = 0; i < clutPoints; i++) {
             Tabelle[i].resize(clutPoints);
             for (int j = 0; j < clutPoints; j++) {
               Tabelle[i][j].resize(clutPoints);
               for (int k = 0; k < clutPoints; k++) {
                 Tabelle[i][j][k].resize(outputChan);
                 for (int l = 0; l < outputChan; l++) {
                   Tabelle[i][j][k][l] = (double)icValue (*(icUInt16Number*)&data_[start + byte*n++])
                              / div;
                 }
               }
             }
           }
         }
         break;
      case MATRIX:
      case CURVE_IN:
      case CURVE_OUT: DBG_PROG
         break;
      }
    } 
  } else if (getTypName() == "mft1") {
    icLut8* lut8 = (icLut8*) &data_[8];
    int inputChan, outputChan, clutPoints, inputEnt=256;//, outputEnt=256;
    inputChan = (int)lut8->inputChan;
    outputChan = (int)lut8->outputChan;
    clutPoints = (int)lut8->clutPoints;
#   ifdef DEBUG_ICCTAG
    int feldPunkte = (int)pow((double)clutPoints, inputChan);
    DBG_NUM_S( feldPunkte << " array points " << clutPoints << " clutPoints" )
#   endif
    int start = 48,
        byte  = 1;
    double div= 255.0;

    // What is requested?
    switch (typ) {
    case TABLE_IN:
    case TABLE_OUT:
    case TABLE: DBG_PROG
         if (inputChan == 3)
         {
           start += (inputChan * inputEnt) * byte;
           Tabelle.resize(clutPoints);
           for (int i = 0; i < clutPoints; i++) {
             Tabelle[i].resize(clutPoints);
             for (int j = 0; j < clutPoints; j++) {
               Tabelle[i][j].resize(clutPoints);
               for (int k = 0; k < clutPoints; k++)
                 Tabelle[i][j][k].resize(outputChan);
             }
           }
           int n = 0;
           for (int i = 0; i < clutPoints; i++) {
             Tabelle[i].resize(clutPoints);
             for (int j = 0; j < clutPoints; j++) {
               Tabelle[i][j].resize(clutPoints);
               for (int k = 0; k < clutPoints; k++) {
                 Tabelle[i][j][k].resize(outputChan);
                 for (int l = 0; l < outputChan; l++) {
                   Tabelle[i][j][k][l] = (double) *(icUInt8Number*)&data_[start + byte*n++]
                              / div;
                 }
               }
             }
           }
         }
         break;
    case MATRIX:
    case CURVE_IN:
    case CURVE_OUT:
         break;
    } 
  }

# ifdef DEBUG_ICCTAG
  DBG_NUM_S( Tabelle.size() )
# endif
  DBG_PROG_ENDE
  return Tabelle;
}

std::vector<double>
ICCtag::getNumbers                                 (MftChain typ)
{ DBG_PROG_START
  std::vector<double> nummern;
  // Who are we?
  if (getTypName() == "mft2") {
    icLut16* lut16 = (icLut16*) &data_[8];
    int inputChan, outputChan, clutPoints, inputEnt, outputEnt;
    inputChan = (int)lut16->inputChan;
    outputChan = (int)lut16->outputChan;
    clutPoints = (int)lut16->clutPoints;
    inputEnt = icValue(lut16->inputEnt);
    outputEnt = icValue(lut16->outputEnt);

    // What is requested?
    switch (typ) {
    case MATRIX:
         for (int i = 0; i < 9; i++) {
           icS15Fixed16Number *n = (icS15Fixed16Number*)&data_[12 + 4*i];
           nummern.push_back( icSFValue (*n) );
         }
         break;
    case TABLE:
         break;
    case CURVE_IN:
    case TABLE_IN:
         nummern.push_back( inputChan );
         break;
    case CURVE_OUT:
    case TABLE_OUT:
         nummern.push_back( outputChan );
         break;
         break;
    } 
  } else if (getTypName() == "mft1") {
    icLut8* lut8 = (icLut8*) &data_[8];
    int inputChan, outputChan, clutPoints;//, inputEnt, outputEnt;
    inputChan = (int)lut8->inputChan;
    outputChan = (int)lut8->outputChan;
    clutPoints = (int)lut8->clutPoints;

    // What is requested?
    switch (typ) {
    case MATRIX:
         for (int i = 0; i < 9; i++) {
           icS15Fixed16Number *n = (icS15Fixed16Number*)&data_[12 + 4*i];
           nummern.push_back( icSFValue (*n) );
         }
         break;
    case CURVE_IN:
    case TABLE:
    case TABLE_IN:
         nummern.push_back( inputChan );
         break;
    case TABLE_OUT:
         nummern.push_back( outputChan );
         break;
    case CURVE_OUT:
         break;
    } 

  } else if ( getTypName() == "ncl2" ) {

    // 0: number of colours
    // 1...n: CIE*Lab colour values
    // n = 3 * colour number

    Ncl2 *ncl2 = (Ncl2*) &data_[8];

    int farben_n        = icValue(ncl2->anzahl);
    int geraetefarben_n = icValue(ncl2->koord);
    if(farben_n)
    {
      nummern .resize(farben_n * (3+geraetefarben_n));
      nummern[0] = farben_n;
      DBG_PROG_V( nummern[0] )
      // append device colours
      for (int i = 0; i < farben_n; ++i)
      {
        Ncl2Farbe *f = (Ncl2Farbe*) ((char*)ncl2 + 76 + // base size of Ncl2
                       (i * (38 +                 // base size of Ncl2Farbe
                             geraetefarben_n      // n Ncl2Farbe::geraetefarbe
                             * sizeof(icUInt16Number))));
        nummern[i*3 +0] = icValue(f->pcsfarbe[0])/65280.0;
        nummern[i*3 +1] = icValue(f->pcsfarbe[1])/65535.0;
        nummern[i*3 +2] = icValue(f->pcsfarbe[2])/65535.0;
        for(int j=0; j < geraetefarben_n; ++j)
          nummern[farben_n*3+i*geraetefarben_n +j] = icValue(f->geraetefarbe[j])/65535.0;
      }
    }
  }

# ifdef DEBUG_ICCTAG
  DBG_NUM_S( nummern.size() )
# endif
  DBG_PROG_ENDE
  return nummern;
}

std::vector<std::string>
ICCtag::getText                     (MftChain typ)
{ DBG_PROG_START
  std::vector<std::string> texte;
  std::vector<double> kanaele;
  char n[6];


  // TODO: check for icColorSpaceSignature <-> channel count
    // What is requested?
    switch (typ) {
    case TABLE:
    case MATRIX:
         for (int i = 0; i < 9; i++) {
           texte.push_back( "" );
         }
         break;
    case CURVE_IN:
    case TABLE_IN:
         kanaele = getNumbers (TABLE_IN);
         texte = getChannelNames (_color_in);
         break;
    case TABLE_OUT:
    case CURVE_OUT:
         texte = getChannelNames (_color_out);
         kanaele = getNumbers (TABLE_OUT);
         break;
    }

  if (kanaele.size()
   && (texte.size() < (unsigned int)kanaele[0]))
  {
    // in case no colour is expected
           if (texte.size() == 1)
           {
             sprintf(n,"%d",1);
             texte[0] = _("Colour");
             texte[0] = texte[0] + n;
           }
    // fill
           for (int i = texte.size(); i < kanaele[0]; i++)
           {
             sprintf(n,"%d",i+1);
             texte.push_back(_("Colour"));
             texte[i] = texte[i] + n;
           }
  }

  DBG_PROG_ENDE
  return texte;
}

icColorSpaceSignature
ICCtag::colorSpace               (MftChain typ)
{ DBG_PROG_START

  // TODO: check for icColorSpaceSignature <-> channel count
    // What is requested?
    switch (typ) {
    case TABLE:
    case MATRIX:
         return (icColorSpaceSignature)0;
         break;
    case CURVE_IN:
    case TABLE_IN:
         return _color_in;
         break;
    case TABLE_OUT:
    case CURVE_OUT:
         return _color_out;
         break;
    }

  DBG_PROG_ENDE
  return (icColorSpaceSignature)0;
}



