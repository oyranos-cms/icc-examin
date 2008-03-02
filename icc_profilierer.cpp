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
  RGB_TRC_Kurven ();

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

  return _profil;
}

void
Profilierer::RGB_TRC_Kurven (void)
{ DBG
  std::vector<XYZ> xyz = _measurement.getMessXYZ();
  std::vector<RGB> rgb = _measurement.getMessRGB();

  std::map<double,XYZ> r_map;
  std::map<double,XYZ> g_map;
  std::map<double,XYZ> b_map;
  // Kanaltests
  for (int i = 0; i < (int)rgb.size(); i++) {
    if (rgb[i].G == 0.0 && rgb[i].B == 0.0) {
      r_map[rgb[i].R] = xyz[i];
    }
    if (rgb[i].R == 0.0 && rgb[i].B == 0.0) {
      g_map[rgb[i].G] = xyz[i];
    }
    if (rgb[i].R == 0.0 && rgb[i].G == 0.0) {
      b_map[rgb[i].B] = xyz[i];
    }
   } DBG_V( r_map.size() )
  DBG_V( g_map.size() )
  DBG_V( b_map.size() )
  

  icTag ic_tag;
  ic_tag.offset = 0;
  icCurveType* kurveTag = NULL; //icCurveType = icTagBase + icCurve
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
  ic_tag.size = icValue (size);
  // Tag kreieren
  ICCtag TRC;
  DBG_V( &_profil ) DBG_V( icValue(ic_tag.size) << &ic_tag ) DBG_V( kurveTag )
  TRC.load( &_profil, &ic_tag, (char*)kurveTag );
  // hinzufügen
  _profil.addTag( TRC ); TRC.clear(); 

  for (i = g_map.begin(); g_map.end() != i; i++) { DBG_V( i->first )
    kurveTag->curve.data[zahl] = icValue( (icUInt16Number)(i->second.Y*655.36));
    zahl++;
  } zahl = 0;
  ic_tag.sig = icValue (icSigGreenTRCTag);
  TRC.load( &_profil, &ic_tag, (char*)kurveTag );
  _profil.addTag( TRC ); TRC.clear();

  for (i = b_map.begin(); b_map.end() != i; i++) {
    kurveTag->curve.data[zahl] = icValue( (icUInt16Number)(i->second.Y*655.36));
    zahl++;
  } zahl = 0;
  ic_tag.sig = icValue (icSigBlueTRCTag);
  TRC.load( &_profil, &ic_tag, (char*)kurveTag );
  _profil.addTag( TRC );

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


