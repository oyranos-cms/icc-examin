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
{
  
}

void
Profilierer::load (ICCprofile& profil)
{
  _measurement = profil.getMeasurement();
  DBG
}


std::string
Profilierer::print()
{
  std::string s = _("Dateikopf ungültig");
  return s;
}

const ICCprofile&
Profilierer::matrix()
{
  _profil.clear();
  RGB_TRC_Kurven ();

  // Profilkopf schreiben
  ICCheader header;
  gemeinsamerHeader (header);
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
{
  std::vector<XYZ> xyz = _measurement.getMessXYZ();
  std::vector<RGB> rgb = _measurement.getMessRGB();

  std::vector<double> r,g,b;
  // Kanaltests
  for (int i = 0; i < (int)rgb.size(); i++) {
    if (rgb[i].G == 0.0 && rgb[i].B == 0.0) r.push_back(rgb[i].R);
    if (rgb[i].R == 0.0 && rgb[i].B == 0.0) g.push_back(rgb[i].G);
    if (rgb[i].R == 0.0 && rgb[i].G == 0.0) b.push_back(rgb[i].B);
  }

  icTag ic_tag;
  ic_tag.offset = 0;
  icCurveType* kurveTag = NULL; //icCurveType = icTagBase + icCurve
  int size = 0;

  if (kurveTag && size)
    { free (kurveTag); size = 0; }
  size = 8 + 4 + r.size() * 2;
  kurveTag = (icCurveType*) calloc (sizeof (char), size);

  kurveTag->base.sig = (icTagTypeSignature)icValue( icSigCurveType );
  // Werte eintragen
  kurveTag->curve.count = icValue( r.size() );
  for (int i = 0; i < (int) r.size(); i++) {
    kurveTag->curve.data[i] = icValue( (int)(r[i]+0.5) );
  }
  // Tagbeschreibung mitgeben
  ic_tag.sig = icValue (icSigRedTRCTag);
  ic_tag.size = size;
  // Tag kreieren
  ICCtag rTRC( &_profil, &ic_tag, (char*)kurveTag );
  // hinzufügen
  _profil.addTag( rTRC );

}

void
Profilierer::gemeinsamerHeader (ICCheader& header)
{
  header.cmmName ("lcms");
  header.version (0x02300000);
  header.set_current_date ();
  header.set_magic ();
  header.set_platform ();
  header.unset_embedded_flag ();
  header.unset_dependent_flag ();
  header.set_manufacturer ();
  header.set_model ();
  header.set_renderingIntent ();
  header.set_illuminant ();
  header.set_creator ();
  //header.setID(); // TODO MD5
}


