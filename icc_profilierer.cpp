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
{ DBG_PROG_START
  std::string s = _("Dateikopf ungültig");
  DBG_PROG_ENDE
  return s;
}

const ICCprofile&
Profilierer::matrix()
{ DBG_PROG_START
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
  // Gamma 1.8
  double gamma = 2.2;
  bool vcgt = true;

  if (vcgt)
    RGB_Gamma_schreiben (gamma);
  else
    RGB_Gamma_schreiben (1.0);

  // endgültiges Gamma berechnen und schreiben
  RGB_Gamma_anpassen (tonwertskalen, gamma, vcgt);

  // TODO Optimierung : Kurven fit, Itterierung oder Vorausberechnung
  // TODO eigene Oberfläche
  // TODO Messdateien lesen aus Text und Profilen
  // TODO Auswertung dE Lab aus fertigem Profil

  schreibMessTag (&profile);  // TODO bedient sich bei icc_examin

  DBG_PROG_ENDE
  return _profil;
}

void
weisspunktanpassung (XYZ& korrektur, XYZ *f)
{ DBG_PROG_START
  f->X = f->X * korrektur.X;
  f->Y = f->Y * korrektur.Y;
  f->Z = f->Z * korrektur.Z;
  DBG_PROG_ENDE
}

void
Profilierer::RGB_Gamma_schreiben (double gamma)
{ DBG_PROG_START
  schreibKurveTag (icSigRedTRCTag, gamma);
  schreibKurveTag (icSigGreenTRCTag, gamma);
  schreibKurveTag (icSigBlueTRCTag, gamma);
  DBG_PROG_ENDE
}

double*
Profilierer::XYZnachRGB (XYZ mess_xyz)
{ DBG_PROG_START
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
  DBG_PROG_ENDE
  return &rgb[0];
}

// Tabellentyp von vcgt
typedef struct VCGT_Tag {
  icTagSignature name;
  icUInt32Number leer;
  icUInt32Number parametrisch; // 8
  icUInt16Number nkurven;      // 12
  icUInt16Number segmente;     // 14
  icUInt16Number byte;         // 16
  icUInt16Number kurve[1];     // 18
};

void
Profilierer::RGB_Gamma_anpassen (std::vector<std::map<double,XYZ> > tonwertskalen, double gamma, bool vcgt)
{ DBG_PROG_START
  icTag ic_tag;
  VCGT_Tag *vcgtTag = NULL;
  icTag vcgt_tag;
  ic_tag.offset = 0;
  vcgt_tag.offset = 0;
  icCurveType* kurveTag = NULL; //icCurveType = icTagBase + icCurve

  int k_size = 0, v_size = 0;

  if (kurveTag && k_size)
    { free (kurveTag); k_size = 0; }
  k_size = 8 + 4 + tonwertskalen[0].size() * 2;
  kurveTag = (icCurveType*) calloc (sizeof (char), k_size);

  kurveTag->base.sig = (icTagTypeSignature)icValue( icSigCurveType );
  // Werte eintragen
  kurveTag->curve.count = icValue( tonwertskalen[0].size() );
  ic_tag.size = icValue ((icUInt32Number)k_size);

  if (vcgt) {
    vcgt_tag.size= icValue ((icUInt32Number)18 + 2 * tonwertskalen[0].size()*3);
    vcgt_tag.sig = icValue ((icTagSignature)1986226036);
    v_size = 18 + sizeof (icUInt16Number) * 3 * tonwertskalen[0].size();
    vcgtTag = (VCGT_Tag*)calloc (sizeof (char), v_size);
    vcgtTag->name = icValue ((icTagSignature)1986226036);
    vcgtTag->leer = 0;
    vcgtTag->parametrisch = icValue ((icUInt32Number)0);
    vcgtTag->nkurven = icValue ((icUInt16Number)3);
    vcgtTag->segmente = icValue ((icUInt16Number)tonwertskalen[0].size());
    vcgtTag->byte = icValue ((icUInt16Number)2);
  }

  int zahl = 0, v_zahl = 0;
  int kanal = 0;
  double *dRGB;

  std::map<double,XYZ>::const_iterator i;

  for (; kanal < 3; kanal++) {
    for (i= tonwertskalen[kanal].begin(); i != tonwertskalen[kanal].end(); i++){
      dRGB = XYZnachRGB (i->second);

      DBG_NUM_V( i->first ) DBG_NUM_V( dRGB[kanal] )

      // Soll
      double soll = i->first; DBG_NUM_V( soll <<" "<< zahl <<" "<< dRGB[kanal])
      // Differenz
      double diff = (double)zahl/(tonwertskalen[kanal].size()-1)
                    + dRGB[kanal] - soll;
      DBG_NUM_V( (double)zahl/(tonwertskalen[kanal].size()-1) <<" "<< diff )

      if (vcgt) {
        vcgtTag->kurve[v_zahl] = icValue( (icUInt16Number)(diff*65535.0 + 0.5));
        DBG_NUM_V( icValue( vcgtTag->kurve[v_zahl] ) <<" "<< v_zahl )
      } else {
        kurveTag->curve.data[zahl] = icValue( (icUInt16Number)
                                              (diff * 65535.0 + 0.5) );
        DBG_NUM_V( icValue( kurveTag->curve.data[zahl] ) )
      }
      zahl++; v_zahl++;
    }
    zahl = 0;
    // Tagbeschreibung mitgeben
    switch (kanal) {
      case 0: ic_tag.sig = icValue (icSigRedTRCTag);
              if (!vcgt)
                _profil.removeTagByName("rTRC");
              break;
      case 1: ic_tag.sig = icValue (icSigGreenTRCTag);
              if (!vcgt)
                _profil.removeTagByName("gTRC");
              break;
      case 2: ic_tag.sig = icValue (icSigBlueTRCTag);
              if (!vcgt)
                _profil.removeTagByName("bTRC");
              break;
    }
    // Tag kreieren
    if (!vcgt) {
      ICCtag Tag;
      DBG_V(&_profil)  DBG_V( icValue(ic_tag.size) << &ic_tag )  DBG_V(kurveTag)
      Tag.load( &_profil, &ic_tag, (char*)kurveTag );
      // Tag hinzufügen
      _profil.addTag( Tag ); Tag.clear();
    }
  }

  if (vcgt) {
    ICCtag Tag;
    Tag.load( &_profil, &vcgt_tag, (char*)vcgtTag ); 
    _profil.addTag( Tag );
    _testProfil.clear();
  }

  DBG_PROG_ENDE
}

std::vector<std::map<double,XYZ> >
Profilierer::RGB_Tags (void)
{ DBG_PROG_START
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
      if (rgb[i].R == 1.0) {
        rot.X = xyz[i].X; DBG_S( "rot" )
        rot.Y = xyz[i].Y;
        rot.Z = xyz[i].Z;
      }
    }
    if (rgb[i].R == 0.0 && rgb[i].B == 0.0) {
      tonwertskalen[1][rgb[i].G] = xyz[i];
      if (rgb[i].G == 1.0) {
        gruen.X = xyz[i].X; DBG_S( "grün" )
        gruen.Y = xyz[i].Y;
        gruen.Z = xyz[i].Z;
      }
    }
    if (rgb[i].R == 0.0 && rgb[i].G == 0.0) {
      tonwertskalen[2][rgb[i].B] = xyz[i];
      if (rgb[i].B == 1.0) {
        blau.X = xyz[i].X; DBG_S( "blau" )
        blau.Y = xyz[i].Y;
        blau.Z = xyz[i].Z;
      }
    }
    if (rgb[i].R == 0.0 && rgb[i].G == 0.0 && rgb[i].B == 0.0) {
      schwarz.X = xyz[i].X; DBG_S( "schwarz" )
      schwarz.Y = xyz[i].Y;
      schwarz.Z = xyz[i].Z;
    }
    if (rgb[i].R == 1.0 && rgb[i].G == 1.0 && rgb[i].B == 1.0) {
      weiss.X = xyz[i].X; DBG_S( "weiss" )
      weiss.Y = xyz[i].Y;
      weiss.Z = xyz[i].Z;
    }
  }
  DBG_NUM_V( tonwertskalen[0].size() )
  DBG_NUM_V( tonwertskalen[1].size() )
  DBG_NUM_V( tonwertskalen[2].size() )
  DBG_NUM_V( weiss.X<<" "<<weiss.Y<<" "<<weiss.Z )

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
  DBG_NUM_V( weiss.X <<" "<< weiss.Y <<" "<< weiss.Z )


  schreibXYZTag (icSigMediaWhitePointTag, weiss.X, weiss.Y, weiss.Z );
  schreibXYZTag (icSigMediaBlackPointTag, schwarz.X, schwarz.Y, schwarz.Z );
  schreibXYZTag (icSigRedColorantTag, rot.X, rot.Y, rot.Z );
  schreibXYZTag (icSigGreenColorantTag, gruen.X, gruen.Y, gruen.Z );
  schreibXYZTag (icSigBlueColorantTag, blau.X, blau.Y, blau.Z );

  DBG_PROG_ENDE
  return tonwertskalen;
}

void
Profilierer::schreibKurveTag (icTagSignature name, double gamma)
{ DBG_PROG_START
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
  DBG_PROG_ENDE
}

void
Profilierer::schreibXYZTag (icTagSignature name, double X, double Y, double Z)
{ DBG_PROG_START
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
  DBG_PROG_ENDE
}

void
Profilierer::schreibTextTag (icTagSignature name, std::string text)
{ DBG_PROG_START
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
  DBG_PROG_ENDE
}

void
Profilierer::schreibMessTag (ICCprofile *profil)
{ DBG_PROG_START
  if (profil->hasTagName("CIED"))
    _profil.addTag( profil->getTag(profil->getTagByName("CIED")) );
  if (profil->hasTagName("DevD"))
    _profil.addTag( profil->getTag(profil->getTagByName("DevD")) );
  if (profil->hasTagName("targ"))
    _profil.addTag( profil->getTag(profil->getTagByName("targ")) );
  DBG_PROG_ENDE
}

void
Profilierer::gemeinsamerHeader (ICCheader* header)
{ DBG_PROG_START
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
  DBG_PROG_ENDE
}


