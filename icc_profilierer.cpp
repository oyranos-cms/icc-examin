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

  std::vector<double> r,g,b;
  std::vector<XYZ> r_xyz, g_xyz, b_xyz;
  // Kanaltests
  for (int i = 0; i < (int)rgb.size(); i++) {
    if (rgb[i].G == 0.0 && rgb[i].B == 0.0) {
      r.push_back(rgb[i].R);
      r_xyz.push_back (xyz[i]);
    }
    if (rgb[i].R == 0.0 && rgb[i].B == 0.0) {
      g.push_back(rgb[i].G);
      g_xyz.push_back (xyz[i]);
    }
    if (rgb[i].R == 0.0 && rgb[i].G == 0.0) {
      b.push_back(rgb[i].B);
      b_xyz.push_back (xyz[i]);
    }
   } DBG_V( r.size() )
  DBG_V( g.size() )
  DBG_V( b.size() )

  std::map<std::vector<double>,std::vector<XYZ> > r_map;

  // Sortieren 
  /*XYZ temp;
  double demp, min, max;
  int i, begin = 0, ende = r.size() - 1, pos_min = 0, pos_max = 0;
  bool sortieren = true;
  while (sortieren) {
    min  = +100000; max = -100000;
    for (i = begin; i <= ende; i++) {
      if (r[i] < min) {min = r[i]; pos_min = i;  DBG_S(begin<<"-"<<pos_min<<"-"<<ende << " : " <<r[i]) }
    }
    // Tauschen
    demp = r[begin]; DBG_V(demp)
    temp = r_xyz[begin];
    r[begin] = r[pos_min]; DBG_V(r[begin])
    r_xyz[begin] = r_xyz[pos_min];
    r[pos_min] = demp; DBG_V(r[pos_min]) 
    r_xyz[pos_min] = temp; 
    for (i = begin; i <= ende; i++) {
      if (r[i] > max) {max = r[i]; pos_max = i; DBG_S(begin<<"-"<<pos_max<<"-"<<ende << " : " <<r[i]) }
    }
    demp = r[ende]; DBG_V(demp)
    temp = r_xyz[ende];
    r[ende] = r[pos_max]; DBG_V(r[ende])
    r_xyz[ende] = r_xyz[pos_max];
    r[pos_max] = demp; DBG_V(r[pos_max])
    r_xyz[pos_max] = temp;
    // Suchbereich einengen
    begin ++; ende --;
    // Fertig?
    if (begin >= ende) sortieren = false;
  }*/

  

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
  for (int i = 0; i < (int) r.size(); i++) { DBG_S (i)
    kurveTag->curve.data[i] = icValue( (icUInt16Number)((double)r[i]*257.0+0.5) ); DBG_V ( i << " " << kurveTag->curve.data[i] << " " << r[i] )
  }
  // Tagbeschreibung mitgeben
  ic_tag.sig = icValue (icSigRedTRCTag);
  ic_tag.size = icValue (size);
  // Tag kreieren
  ICCtag rTRC; DBG
  DBG_V( &_profil )
  DBG_V( icValue(ic_tag.size) << &ic_tag )
  DBG_V( kurveTag )
  rTRC.load( &_profil, &ic_tag, (char*)kurveTag );
  // hinzufügen
  _profil.addTag( rTRC );

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


