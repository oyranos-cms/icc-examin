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
#include "icc_utils.h"

#define _(text) text
#define g_message printf



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
  RGB_Tags ();

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
Profilierer::RGB_Tags (void)
{ DBG
  std::vector<XYZ> xyz = _measurement.getMessXYZ();
  std::vector<RGB> rgb = _measurement.getMessRGB();

  std::map<double,XYZ> r_map;
  std::map<double,XYZ> g_map;
  std::map<double,XYZ> b_map;
  XYZ weiss, schwarz, rot, gruen, blau;
  // Kanaltests
  for (std::vector<XYZ>::iterator i = xyz.begin(); i != xyz.end(); i++) {
    DBG_V ( i->X <<" "<< i->Y <<" "<< i->Z )
  }
  for (int i = 0; i < (int)rgb.size(); i++) {
    if (rgb[i].G == 0.0 && rgb[i].B == 0.0) {
      r_map[rgb[i].R] = xyz[i];
      if (rgb[i].R == 0.0) {
        schwarz.X = xyz[i].X/100.0; DBG_S( "schwarz" )
        schwarz.Y = xyz[i].Y/100.0;
        schwarz.Z = xyz[i].Z/100.0;
      }
      if (rgb[i].R == 255.0) {
        rot.X = xyz[i].X/100.0; DBG_S( "rot" )
        rot.Y = xyz[i].Y/100.0;
        rot.Z = xyz[i].Z/100.0;
      }
    }
    if (rgb[i].R == 0.0 && rgb[i].B == 0.0) {
      g_map[rgb[i].G] = xyz[i];
      if (rgb[i].G == 255.0) {
        gruen.X = xyz[i].X/100.0; DBG_S( "grün" )
        gruen.Y = xyz[i].Y/100.0;
        gruen.Z = xyz[i].Z/100.0;
      }
    }
    if (rgb[i].R == 0.0 && rgb[i].G == 0.0) {
      b_map[rgb[i].B] = xyz[i];
      if (rgb[i].B == 255.0) {
        blau.X = xyz[i].X/100.0; DBG_S( "blau" )
        blau.Y = xyz[i].Y/100.0;
        blau.Z = xyz[i].Z/100.0;
      }
    }
    if (rgb[i].R == 255.0 && rgb[i].G == 255.0 && rgb[i].B == 255.0) {
      weiss.X = xyz[i].X/100.0; DBG_S( "weiss" )
      weiss.Y = xyz[i].Y/100.0;
      weiss.Z = xyz[i].Z/100.0;
    }
  } DBG_V( r_map.size() )
  DBG_V( g_map.size() )
  DBG_V( b_map.size() )
  DBG_V( weiss.X<<" "<<weiss.Y<<" "<<weiss.Z )
  DBG_V( schwarz.Y )
  

  icTag ic_tag;
  ic_tag.offset = 0;
  icCurveType* kurveTag = NULL; //icCurveType = icTagBase + icCurve
/*
  int size = 0;

  if (kurveTag && size)
    { free (kurveTag); size = 0; }
  size = 8 + 4 + r_map.size() * 2;
  kurveTag = (icCurveType*) calloc (sizeof (char), size);

  kurveTag->base.sig = (icTagTypeSignature)icValue( icSigCurveType );
  // Werte eintragen
  kurveTag->curve.count = icValue( r_map.size() );
  std::map<double,XYZ>::const_iterator i;
  int zahl = 0;
  for (i = r_map.begin(); r_map.end() != i; i++) {
    kurveTag->curve.data[zahl] = icValue( (icUInt16Number)(i->second.Y*655.36));
    zahl++;
  } zahl = 0;
  // Tagbeschreibung mitgeben
  ic_tag.sig = icValue (icSigRedTRCTag);
  ic_tag.size = icValue ((icUInt32Number)size);
  // Tag kreieren
  ICCtag Tag;
  DBG_V( &_profil ) DBG_V( icValue(ic_tag.size) << &ic_tag ) DBG_V( kurveTag )
  Tag.load( &_profil, &ic_tag, (char*)kurveTag );
  // hinzufügen
  _profil.addTag( Tag ); Tag.clear(); 

  for (i = g_map.begin(); g_map.end() != i; i++) { DBG_V( i->first )
    kurveTag->curve.data[zahl] = icValue( (icUInt16Number)(i->second.Y*655.36));
    zahl++;
  } zahl = 0;
  ic_tag.sig = icValue (icSigGreenTRCTag);
  Tag.load( &_profil, &ic_tag, (char*)kurveTag );
  _profil.addTag( Tag ); Tag.clear();

  for (i = b_map.begin(); b_map.end() != i; i++) {
    kurveTag->curve.data[zahl] = icValue( (icUInt16Number)(i->second.Y*655.36));
    zahl++;
  } zahl = 0;
  ic_tag.sig = icValue (icSigBlueTRCTag);
  Tag.load( &_profil, &ic_tag, (char*)kurveTag );
  _profil.addTag( Tag );
*/
  schreibXYZTag (icSigMediaWhitePointTag, weiss.X, weiss.Y, weiss.Z );
  schreibXYZTag (icSigMediaBlackPointTag, schwarz.X, schwarz.Y, schwarz.Z );
  schreibXYZTag (icSigRedColorantTag, rot.X, rot.Y, rot.Z );
  schreibXYZTag (icSigGreenColorantTag, gruen.X, gruen.Y, gruen.Z );
  schreibXYZTag (icSigBlueColorantTag, blau.X, blau.Y, blau.Z );

  //free (kurveTag);
}

void
Profilierer::schreibXYZTag (icTagSignature name, double X, double Y, double Z)
{
  icXYZType xyzTag;
  icTag ic_tag;
  ICCtag Tag;

  ic_tag.size = icValue ((icUInt32Number)sizeof(icXYZType));
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
  ic_tag.sig = icValue (name);

  char sig[] = "text";
  memcpy (&tag_block[0], &sig, 4); DBG_S( tag_block )
  memcpy (&tag_block[8], text.c_str(), text.size());

  Tag.load( &_profil, &ic_tag, tag_block );
  _profil.addTag( Tag );

  free (tag_block);
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


