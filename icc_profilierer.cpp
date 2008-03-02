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
RGB* RGB_Differenz (ICCprofile profil, XYZ mess_xyz);

/**
  *  @brief Profilierer Funktionen
  */ 

Profilierer::Profilierer()
{ DBG
  
}

void
Profilierer::load (ICCprofile& profil)
{ DBG
  _measurement = profil.getMeasurement();
  DBG
}


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
  std::vector<std::map<double,XYZ> > tonwertskalen;
  // Primärtags + Weiss und Schwarz schreiben
  tonwertskalen = RGB_Tags ();
  // vorläufiges Gamma 1.0
  RGB_Gamma_schreiben (1.0);
  // endgültiges Gamma berechnen und schreiben
  RGB_Gamma_anpassen (tonwertskalen);

  schreibMessTag (&profile);  // TODO bedient sich bei icc_examin

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

  std::map<double,XYZ>::const_iterator i;

  int zahl = 0;
  double korr_Y, Y, min_Y;
  int kanal = 0;

  for (; kanal < 3; kanal++) {
    i = tonwertskalen[kanal].begin();
    min_Y = i->second.Y;
    i = tonwertskalen[kanal].end(); i--;
    korr_Y = 1.0/(i->second.Y - min_Y);
    for (i= tonwertskalen[kanal].begin(); tonwertskalen[kanal].end() != i; i++){
      DBG_V( i->first ) DBG_V( min_Y ) DBG_V( korr_Y ) DBG_V( i->second.Y )
      Y = (i->second.Y - min_Y)* korr_Y;
      kurveTag->curve.data[zahl] = icValue( (icUInt16Number)(Y*65535.0+0.5) );
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
      if (rgb[i].R == 255.0) {
        rot.X = xyz[i].X/100.0; DBG_S( "rot" )
        rot.Y = xyz[i].Y/100.0;
        rot.Z = xyz[i].Z/100.0;
      }
    }
    if (rgb[i].R == 0.0 && rgb[i].B == 0.0) {
      tonwertskalen[1][rgb[i].G] = xyz[i];
      if (rgb[i].G == 255.0) {
        gruen.X = xyz[i].X/100.0; DBG_S( "grün" )
        gruen.Y = xyz[i].Y/100.0;
        gruen.Z = xyz[i].Z/100.0;
      }
    }
    if (rgb[i].R == 0.0 && rgb[i].G == 0.0) {
      tonwertskalen[2][rgb[i].B] = xyz[i];
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


