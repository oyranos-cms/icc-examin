// Author:    Kai-Uwe Behrmann
// Copyright: Kai-Uwe Behrmann <ku.b@gmx.de>
// Date:      03. 08. 2004

#if 1
  #ifndef DEBUG
   #define DEBUG
  #endif
  #define DEBUG_PROFILIERER
#endif

#include "icc_profilierer.h"
#include "icc_examin.h"
#include "icc_utils.h"

#define _(text) text
#define g_message printf

// locale Funktionen

void weisspunktanpassung (XYZ& korrektur, XYZ *f);

/**
  *  @brief Profilierer Funktionen
  */ 


std::string
Profilierer::print()
{ DBG
  std::string s = _("Dateikopf ungültig");
  return s;
}

const ICCprofile&
Profilierer::matrix()
{ DBG
  _profil.clear();
  // Profilkopf schreiben
  ICCheader header;
  gemeinsamerHeader (&header);
  header.deviceClass (icSigDisplayClass);
  header.colorSpace (icSigRgbData);
  header.pcs (icSigXYZData);
  header.set_transparency_attr ();
  header.set_matte_attr ();
  header.set_positive_attr ();
  header.set_color_attr ();
  _profil.setHeader ((void*)header.header_raw());

  std::stringstream s;
  s << "SB v" << ICC_PROFILIERER_VERSION << " Test";
  schreibTextTag (icSigProfileDescriptionTag, s.str());
  schreibTextTag (icSigCopyrightTag, "Lesen und Anwenden sind verboten. Veröffentlichen ist Hochverrat und wir mit Enthauptung nach Patentrecht bestraft!!!");

  // Primärtags + Weiss und Schwarz schreiben
  std::vector<std::map<double,XYZ> > tonwertskalen =
  RGB_Tags ();
  // vorläufiges Gamma 1.0
  RGB_Gamma_schreiben (1.0);
  // endgültiges Gamma berechnen und schreiben
  RGB_Gamma_anpassen (tonwertskalen);

  schreibMessTag (&profile);  // TODO bedient sich bei icc_examin

  return _profil;
}

void
weisspunktanpassung (XYZ& korrektur, XYZ *f)
{
  f->X = f->X * korrektur.X;
  f->Y = f->Y * korrektur.Y;
  f->Z = f->Z * korrektur.Z;
}

void
Profilierer::RGB_Gamma_schreiben (double gamma)
{
  schreibKurveTag (icSigRedTRCTag, gamma);
  schreibKurveTag (icSigGreenTRCTag, gamma);
  schreibKurveTag (icSigBlueTRCTag, gamma);
}

double*
Profilierer::XYZnachRGB (XYZ mess_xyz)
{ DBG
  static double rgb[3], xyz[3];
  DBG_S( "XYZ: " << mess_xyz.X << " " << mess_xyz.Y << " " << mess_xyz.Z )

  if (_testProfil.size() == 0) {
    int size = _profil.getProfileSize(); DBG_V( size )
    _testProfil.assign( _profil.saveProfileToMem( &size ), size );
  }
  if (_testProfil.size() == 0)
    DBG_S( "!!! _testProfile war leer !!!" )
  char* const_ptr = (char*) _testProfil.c_str();

  #define PRECALC cmsFLAGS_NOTPRECALC // No memory overhead, VERY
                                      // SLOW ON TRANSFORMING, very fast on creating transform.
  cmsHPROFILE testRGB = cmsOpenProfileFromMem (const_ptr, _testProfil.size() ),
              hXYZ;
  cmsHTRANSFORM hXYZtoRGB;
  hXYZ = cmsCreateXYZProfile ();
  hXYZtoRGB = cmsCreateTransform   (hXYZ, TYPE_XYZ_DBL,
                                    testRGB, TYPE_RGB_DBL,
                                    INTENT_ABSOLUTE_COLORIMETRIC,
                                    PRECALC|cmsFLAGS_WHITEBLACKCOMPENSATION);
  FarbeZuDouble( &xyz[0], mess_xyz );
  DBG_S( "xyz: " << xyz[0] << " " << xyz[1] << " " << xyz[2] )
  cmsDoTransform (hXYZtoRGB, &xyz[0], &rgb[0], 1);

  cmsDeleteTransform (hXYZtoRGB);
  cmsCloseProfile (testRGB);
  DBG_S( "RGB: " << rgb[0] << " " << rgb[1] << " " << rgb[2] )
  return &rgb[0];
}

void
Profilierer::RGB_Gamma_anpassen (std::vector<std::map<double,XYZ> > tonwertskalen)
{
  icTag ic_tag;
  ic_tag.offset = 0;
  icCurveType* kurveTag = NULL; //icCurveType = icTagBase + icCurve

  int size = 0;

  if (kurveTag && size)
    { free (kurveTag); size = 0; }
  size = 8 + 4 + tonwertskalen[0].size() * 2;
  kurveTag = (icCurveType*) calloc (sizeof (char), size);

  kurveTag->base.sig = (icTagTypeSignature)icValue( icSigCurveType );
  // Werte eintragen
  kurveTag->curve.count = icValue( tonwertskalen[0].size() );
  ic_tag.size = icValue ((icUInt32Number)size);

  int zahl = 0;
  double korr, kFarbe, min;
  int kanal = 0;
  double *dRGB;

  std::map<double,XYZ>::const_iterator i;

  for (; kanal < 3; kanal++) {
    i = tonwertskalen[kanal].begin();
    dRGB = XYZnachRGB (i->second);
    min = dRGB[kanal];
    i = tonwertskalen[kanal].end(); i--;
    dRGB = XYZnachRGB (i->second); DBG_V( dRGB[0] << dRGB[1] << dRGB[2] )
    korr = 1.0/(dRGB[kanal]/* - min*/);
    for (i= tonwertskalen[kanal].begin(); i != tonwertskalen[kanal].end(); i++){
      dRGB = XYZnachRGB (i->second);

      DBG_V( i->first ) DBG_V( min ) DBG_V( korr ) DBG_V( dRGB[kanal] )

      kFarbe = (dRGB[kanal]/* - min*/)* korr;
      kurveTag->curve.data[zahl] = icValue( (icUInt16Number)(kFarbe*65535.0+0.5) );
      zahl++; DBG_V( icValue( kurveTag->curve.data[zahl] ) )
    } zahl = 0;
    // Tagbeschreibung mitgeben
    switch (kanal) {
      case 0: ic_tag.sig = icValue (icSigRedTRCTag);
              _profil.removeTagByName("rTRC");  break;
      case 1: ic_tag.sig = icValue (icSigGreenTRCTag);
              _profil.removeTagByName("gTRC");  break;
      case 2: ic_tag.sig = icValue (icSigBlueTRCTag);
              _profil.removeTagByName("bTRC");  break;
    }
    // Tag kreieren
    ICCtag Tag;
    DBG_V( &_profil ) DBG_V( icValue(ic_tag.size) << &ic_tag ) DBG_V( kurveTag )
    Tag.load( &_profil, &ic_tag, (char*)kurveTag );
    // hinzufügen
    _profil.addTag( Tag ); Tag.clear(); 
  }
  _testProfil.clear();
}

std::vector<std::map<double,XYZ> >
Profilierer::RGB_Tags (void)
{ DBG
  std::vector<XYZ> xyz = _measurement.getMessXYZ();
  std::vector<RGB> rgb = _measurement.getMessRGB();

  std::vector<std::map<double,XYZ> > tonwertskalen (3);
  XYZ weiss, schwarz, rot, gruen, blau, // gefundene Farben
      weiss_ziel,                       // Weisspunkt
      weiss_D50,                        // Umrechnungsweisspunkt
      korr;                             // Korrekturwerte für alle Farben

  // Kanaltests
  for (std::vector<XYZ>::iterator i = xyz.begin(); i != xyz.end(); i++) {
    DBG_V ( i->X <<" "<< i->Y <<" "<< i->Z )
  }
  for (int i = 0; i < (int)rgb.size(); i++) {
    if (rgb[i].G == 0.0 && rgb[i].B == 0.0) {
      tonwertskalen[0][rgb[i].R] = xyz[i];
      tonwertskalen[0][rgb[i].R].X /= 100.0;
      tonwertskalen[0][rgb[i].R].Y /= 100.0;
      tonwertskalen[0][rgb[i].R].Z /= 100.0;
      if (rgb[i].R == 255.0) {
        rot.X = xyz[i].X/100.0; DBG_S( "rot" )
        rot.Y = xyz[i].Y/100.0;
        rot.Z = xyz[i].Z/100.0;
      }
    }
    if (rgb[i].R == 0.0 && rgb[i].B == 0.0) {
      tonwertskalen[1][rgb[i].G] = xyz[i];
      tonwertskalen[1][rgb[i].G].X /= 100.0;
      tonwertskalen[1][rgb[i].G].Y /= 100.0;
      tonwertskalen[1][rgb[i].G].Z /= 100.0;
      if (rgb[i].G == 255.0) {
        gruen.X = xyz[i].X/100.0; DBG_S( "grün" )
        gruen.Y = xyz[i].Y/100.0;
        gruen.Z = xyz[i].Z/100.0;
      }
    }
    if (rgb[i].R == 0.0 && rgb[i].G == 0.0) {
      tonwertskalen[2][rgb[i].B] = xyz[i];
      tonwertskalen[2][rgb[i].B].X /= 100.0;
      tonwertskalen[2][rgb[i].B].Y /= 100.0;
      tonwertskalen[2][rgb[i].B].Z /= 100.0;
      if (rgb[i].B == 255.0) {
        blau.X = xyz[i].X/100.0; DBG_S( "blau" )
        blau.Y = xyz[i].Y/100.0;
        blau.Z = xyz[i].Z/100.0;
      }
    }
    if (rgb[i].R == 0.0 && rgb[i].G == 0.0 && rgb[i].B == 0.0) {
      schwarz.X = xyz[i].X/100.0; DBG_S( "schwarz" )
      schwarz.Y = xyz[i].Y/100.0;
      schwarz.Z = xyz[i].Z/100.0;
    }
    if (rgb[i].R == 255.0 && rgb[i].G == 255.0 && rgb[i].B == 255.0) {
      weiss.X = xyz[i].X/100.0; DBG_S( "weiss" )
      weiss.Y = xyz[i].Y/100.0;
      weiss.Z = xyz[i].Z/100.0;
    }
  }
  DBG_V( tonwertskalen[0].size() )
  DBG_V( tonwertskalen[1].size() )
  DBG_V( tonwertskalen[2].size() )
  DBG_V( weiss.X<<" "<<weiss.Y<<" "<<weiss.Z )

  // Korrekturen weiss -> D50
  weiss_D50.X = 0.9642; weiss_D50.Y = 1.0; weiss_D50.Z = 0.8249;
  // natürlicher Weisspunkt
  weiss_ziel.X = weiss.X * 1.0/weiss.Y; DBG_V( weiss.X )
  weiss_ziel.Y = weiss.Y * 1.0/weiss.Y; DBG_V( weiss.Y )
  weiss_ziel.Z = weiss.Z * 1.0/weiss.Y; DBG_V( weiss.Z )
  korr.X = weiss_D50.X/weiss.X;
  korr.Y = weiss_D50.Y/weiss.Y;
  korr.Z = weiss_D50.Z/weiss.Z;
  weisspunktanpassung (korr, &weiss_ziel);
  weisspunktanpassung (korr, &schwarz);
  weisspunktanpassung (korr, &rot);
  weisspunktanpassung (korr, &gruen);
  weisspunktanpassung (korr, &blau);
  for (int kanal = 0; kanal < 3 ; kanal++) {
    std::map<double,XYZ>::iterator i;
    for (i= tonwertskalen[kanal].begin(); i != tonwertskalen[kanal].end(); i++){
      weisspunktanpassung (korr, &(i->second));
    }
  }
  DBG_V( weiss.X <<" "<< weiss.Y <<" "<< weiss.Z )


  schreibXYZTag (icSigMediaWhitePointTag, weiss.X, weiss.Y, weiss.Z );
  schreibXYZTag (icSigMediaBlackPointTag, schwarz.X, schwarz.Y, schwarz.Z );
  schreibXYZTag (icSigRedColorantTag, rot.X, rot.Y, rot.Z );
  schreibXYZTag (icSigGreenColorantTag, gruen.X, gruen.Y, gruen.Z );
  schreibXYZTag (icSigBlueColorantTag, blau.X, blau.Y, blau.Z );

  return tonwertskalen;
}

void
Profilierer::schreibKurveTag (icTagSignature name, double gamma)
{
  icTag ic_tag;
  ic_tag.offset = 0;
  icCurveType* kurveTag = NULL; //icCurveType = icTagBase + icCurve

  int size = 0;

  if (kurveTag && size)
    { free (kurveTag); size = 0; }
  size = 8 + 4 + 1 * 2;
  kurveTag = (icCurveType*) calloc (sizeof (char), size);

  kurveTag->base.sig = (icTagTypeSignature)icValue( icSigCurveType );
  // Werte eintragen
  kurveTag->curve.count = icValue( (icUInt32Number) 1 );
  kurveTag->curve.data[0] = icValue( (icUInt16Number)(gamma*256.0));
  // Tagbeschreibung mitgeben
  ic_tag.sig = icValue (name);
  ic_tag.size = icValue ((icUInt32Number)size);
  // Tag kreieren
  ICCtag Tag;
  DBG_V( &_profil ) DBG_V( icValue(ic_tag.size) << &ic_tag ) DBG_V( kurveTag )
  Tag.load( &_profil, &ic_tag, (char*)kurveTag );
  // hinzufügen
  _profil.addTag( Tag ); Tag.clear(); 
}

void
Profilierer::schreibXYZTag (icTagSignature name, double X, double Y, double Z)
{
  icXYZType xyzTag;
  memset ((char*)&xyzTag, 0, sizeof(icXYZType));
  icTag ic_tag;
  ICCtag Tag;

  ic_tag.size = icValue ((icUInt32Number)sizeof(icXYZType));
  ic_tag.offset = 0;
  ic_tag.sig = icValue (name);

  xyzTag.base.sig = (icTagTypeSignature)icValue( icSigXYZType );
  icValueXYZ (&xyzTag.data.data[0], X, Y, Z );

  Tag.load( &_profil, &ic_tag, (char*)&xyzTag );
  _profil.addTag( Tag );
}

void
Profilierer::schreibTextTag (icTagSignature name, std::string text)
{
  int groesse = 8 + text.size() + 1;
  char* tag_block = (char*) calloc (sizeof (char), groesse);
  icTag ic_tag;
  ICCtag Tag;

  ic_tag.size = icValue ((icUInt32Number)groesse); DBG_V( groesse )
  ic_tag.offset = 0;
  ic_tag.sig = icValue (name);

  char sig[] = "text";
  memcpy (&tag_block[0], &sig, 4); DBG_S( tag_block )
  memcpy (&tag_block[8], text.c_str(), text.size());

  Tag.load( &_profil, &ic_tag, tag_block );
  _profil.addTag( Tag );

  free (tag_block);
}

void
Profilierer::schreibMessTag (ICCprofile *profil)
{
  if (profil->hasTagName("CIED"))
    _profil.addTag( profil->getTag(profil->getTagByName("CIED")) );
  if (profil->hasTagName("DevD"))
    _profil.addTag( profil->getTag(profil->getTagByName("DevD")) );
  if (profil->hasTagName("targ"))
    _profil.addTag( profil->getTag(profil->getTagByName("targ")) );
}

void
Profilierer::gemeinsamerHeader (ICCheader* header)
{ DBG
  header->cmmName ("lcms");
  header->version (0x02300000);
  header->set_current_date ();
  header->set_magic ();
  header->set_platform ();
  header->unset_embedded_flag ();
  header->unset_dependent_flag ();
  header->set_manufacturer ();
  header->set_model ();
  header->set_renderingIntent ();
  header->set_illuminant ();
  header->set_creator ();
  //header->setID(); // TODO MD5
}


