// Author:    Kai-Uwe Behrmann
// Copyright: Kai-Uwe Behrmann <ku.b@gmx.de>
// Date:      20. 08. 2004

#if 1
  #ifndef DEBUG
   #define DEBUG
  #endif
  #define DEBUG_ICCMEASUREMENT
#endif

#include "icc_profile.h"
#include "icc_examin.h"

#define _(text) text
#define g_message printf


/**
  *  @brief ICCmeasurement Funktionen
  */

ICCmeasurement::ICCmeasurement ()
{
  _sig = icMaxEnumTag;
  _data = NULL;
  _profil = NULL;
  _lcms_it8 = NULL;
}

ICCmeasurement::ICCmeasurement (ICCprofile* profil, ICCtag &tag)
{
  ICCmeasurement::load (profil, tag); DBG
}

ICCmeasurement::~ICCmeasurement ()
{
  _sig = icMaxEnumTag;
  _size = 0;
  if (_lcms_it8 != NULL) cmsIT8Free (_lcms_it8); // lcms
  if (_data != NULL) free (_data);
  DBG
}

void
ICCmeasurement::load                ( ICCprofile *profil,
                                      ICCtag&     tag )
{ DBG
  _profil = profil;

  _sig    = tag._sig;
  _size   = tag._size - 8;
  // einfach austauschen
  if (_data != NULL) free (_data); 
  _data = (char*) calloc ( _size , sizeof (char) );
  memcpy ( _data , &(tag._data)[8] , _size );

  init_meas();
  DBG
}

void
ICCmeasurement::clear               (void)
{
  if (_lcms_it8 != NULL) cmsIT8Free (_lcms_it8); // lcms
  if (_data != NULL) free (_data);
  _sig = icMaxEnumTag;
  _size = 0;
  _data = NULL;

  _lcms_it8 = NULL;
  _nFelder = 0;

  _profil = NULL;
  _XYZ_measurement = false;
  _RGB_measurement = false;
  _CMYK_measurement = false;
  _XYZ_Satz.clear();
  _RGB_Satz.clear();
  _CMYK_Satz.clear();
  _Feldnamen.clear();
  _XYZ_Ergebnis.clear();
  _RGB_MessFarben.clear();
  _RGB_ProfilFarben.clear();
  _reportTabelle.clear();

  #ifdef DEBUG_ICCMEASUREMENT
  DBG
  #endif
}

void
ICCmeasurement::load                ( ICCprofile *profil,
                                      char       *data,
                                      size_t      size )
{ DBG
  _profil = profil;

  if (_sig != icMaxEnumTag)
    _sig = icSigCharTargetTag;

  _size   = size;
  // enfach austauschen
  if (!_data) free (_data);
  _data = (char*) calloc ( _size , sizeof (char) );
  memcpy ( _data , data , _size );
  DBG

  init_meas();

  #ifdef DEBUG_ICCMEASUREMENT
  DBG
  #endif
}

void
ICCmeasurement::init_meas (void)
{ //DBG_S (_data)

  // Reparieren
  // LF FF
  char* ptr = 0;
  while (strchr(_data, 13) > 0) { // \r 013 0x0d
      ptr = strchr(_data, 13);
      if (ptr > 0) {
        if (*(ptr+1) == '\n')
          *ptr = ' ';
        else
          *ptr = '\n';
      }
  };

  // char* -> std::string
  std::string data (_data, 0, _size);

  // reparieren: Sample_Name , SampleID, "" , LF FF
  std::string::size_type pos=0;
  std::string::size_type ende;
  while ((pos = data.find ("SampleID", pos)) != std::string::npos) {
      data.replace (pos, 8, "SAMPLE_ID"); DBG_S( "SampleID ersetzt" )
  }
  pos = 0;
  while ((pos = data.find ("Sample_Name", pos)) != std::string::npos) {
      data.replace (pos, strlen("Sample_Name"), "SAMPLE_NAME"); DBG_S( "Sample_Name ersetzt" )
  }
  pos = 0;
  while ((pos = data.find ("Date:", pos)) != std::string::npos) {
      data.replace (pos, strlen("Date:"), "CREATED #"); DBG_S( "Date: ersetzt" )
  }
  pos = 0;
  while ((pos = data.find ("\"\"", pos)) != std::string::npos) {
      data.replace (pos, strlen("\"\""), "\""); DBG_S( "\"\" ersetzt" )
  }
  // fehlendes SAMPLE_ID einführen und jeder Zeile einen Zähler voransetzen
  int count;
  if ((data.find ("SAMPLE_ID", 0)) == std::string::npos) {
    pos = data.find ("BEGIN_DATA_FORMAT\n", 0);
    data.insert (pos+strlen("BEGIN_DATA_FORMAT\n"), "SAMPLE_ID   ");
    pos = data.find ("BEGIN_DATA\n", 0); DBG_S (pos)
    count = 1; pos++;
    while ((pos = data.find ("\n", pos)) != std::string::npos
         && (ende = data.find ("END_DATA\n", pos+2)) != std::string::npos) {
      static char zahl[12];
      sprintf (&zahl[0], "%d   ", count); count++; DBG_S(count << " " << pos)
      data.insert (pos+1, &zahl[0]);
      pos += strlen (&zahl[0]);
      //DBG_S( data )
    }
  }
  // NUMBER_OF_FIELDS reparieren
  if ((data.find ("NUMBER_OF_FIELDS", 0)) == std::string::npos) {
    pos = data.find ("BEGIN_DATA_FORMAT\n", 0); DBG_S (pos)
    count = 0; pos++;
    if (data.find ("SAMPLE_ID", 0) != std::string::npos) count ++;
    if (data.find ("SAMPLE_NAME", 0) != std::string::npos) count ++;
    if (data.find ("CMYK_C", 0) != std::string::npos) count ++;
    if (data.find ("CMYK_M", 0) != std::string::npos) count ++;
    if (data.find ("CMYK_Y", 0) != std::string::npos) count ++;
    if (data.find ("CMYK_K", 0) != std::string::npos) count ++;
    if (data.find ("RGB_R", 0) != std::string::npos) count ++;
    if (data.find ("RGB_G", 0) != std::string::npos) count ++;
    if (data.find ("RGB_B", 0) != std::string::npos) count ++;
    if (data.find ("XYZ_X", 0) != std::string::npos) count ++;
    if (data.find ("XYZ_Y", 0) != std::string::npos) count ++;
    if (data.find ("XYZ_Z", 0) != std::string::npos) count ++;
    if (data.find ("XYY_X", 0) != std::string::npos) count ++;
    if (data.find ("XYY_Y", 0) != std::string::npos) count ++;
    if (data.find ("XYY_CAPY", 0) != std::string::npos) count ++;
    if (data.find ("LAB_L", 0) != std::string::npos) count ++;
    if (data.find ("LAB_A", 0) != std::string::npos) count ++;
    if (data.find ("LAB_B", 0) != std::string::npos) count ++;
    static char zahl[64];
    pos = data.find ("BEGIN_DATA_FORMAT\n", 0);
    sprintf (&zahl[0], "NUMBER_OF_FIELDS %d\n", count);
    data.insert (pos, &zahl[0]);
    DBG_S( "NUMBER_OF_FIELDS " << count << " eingefügt" )
  }
  // NUMBER_OF_SETS reparieren
  if ((data.find ("NUMBER_OF_SETS", 0)) == std::string::npos) {
    pos = data.find ("BEGIN_DATA\n", 0); DBG_S (pos)
    count = 0; pos++;
    while ((pos = data.find ("\n", pos)) != std::string::npos
         && (ende = data.find ("END_DATA\n", pos+2)) != std::string::npos) {
      count ++; DBG_S( pos << " " << count)
      pos++;
    }
    static char zahl[64];
    pos = data.find ("BEGIN_DATA\n", 0);
    sprintf (&zahl[0], "NUMBER_OF_SETS %d\n", count);
    data.insert (pos, &zahl[0]);
    DBG_S( "NUMBER_OF_SETS " << count << " eingefügt" )
  }
  // Signatur reparieren
  pos = 0;
  int pos_alt = 0, diff = 0;
  count = 0;
  while (diff <= 1) {
    count ++;
    pos = data.find ("\n", pos);
    diff = pos - pos_alt;
    pos_alt = pos;
    if (count == 12) // 12 ist länger als die erlaubten 7 Zeichen
      diff = count;
    DBG_S (diff)
  }
  if (diff != 8) {
  //if (getTagName() == "CIED") {
      data.insert (0, "ICCEXAM\n"); DBG_S( "Beschreibung eingeführt" )
      //DBG_S ( data );
  }

  DBG_S (data)
  #if 0
  clear();
  return;
  #endif

  // lcms liest ein
  if (_lcms_it8 != NULL) cmsIT8Free (_lcms_it8);
  if (_data != NULL) free (_data);
  _data = (char*) calloc (sizeof(char), data.size());
  _size = data.size();
  memcpy (_data, data.c_str(), _size);
  _lcms_it8 = cmsIT8LoadFromMem ( _data, _size ); DBG

  char **SampleNames; DBG

  // Messfeldanzahl
  if (_nFelder == 0
   || _nFelder == (int)cmsIT8GetPropertyDbl(_lcms_it8, "NUMBER_OF_SETS")) { DBG
    _nFelder = (int)cmsIT8GetPropertyDbl(_lcms_it8, "NUMBER_OF_SETS"); DBG
  } else {
    DBG_S( "Messfeldanzahl sollte schon übereinstimmen! " << _nFelder << "|" << (int)cmsIT8GetPropertyDbl(_lcms_it8, "NUMBER_OF_SETS") )
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
    #ifdef DEBUG_ICCMEASUREMENT
    DBG_S( (char*)SampleNames[i] << " _sample_name " << _sample_name <<
           " _sample_id" << _sample_id << " _id_vor_name " << _id_vor_name) 
    #endif
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

  } DBG

  // Variablen
  int farben = 0;
  if (has_Lab) farben++;
  if (has_XYZ) {farben++; _XYZ_measurement = true; }
  if (has_RGB) {farben++; _RGB_measurement = true; }
  if (has_CMYK) {farben++; _CMYK_measurement = true; }
  if (has_xyY) farben++;


  // vorläufige lcms Farbnamen listen
  if (farben > 0) {
    _Feldnamen.resize(_nFelder);
    const char* constr = (const char*) calloc (sizeof (char), 12);
    DBG
    for (int k = 0; k < _nFelder; k++) {
      if (_id_vor_name
       && (getTagName() != "DevD")) {// Name ignorieren
        char *text = (char*) calloc (sizeof(char), 12);
        sprintf (text, "%d", k+1);
        _Feldnamen[k] = text;
      } else {
        constr = cmsIT8GetPatchName (_lcms_it8, k, NULL);
        _Feldnamen[k] = constr;
      }
      //cout << _Feldnamen[k] << " " << k << " "; DBG
    }
  } DBG_S (_Feldnamen[0] << " bis " << _Feldnamen[_nFelder-1])
 
  // Farben auslesen
  if (has_XYZ) { DBG // keine Umrechnung nötig
    _XYZ_Satz.resize(_nFelder);
    for (int i = 0; i < _nFelder; i++) {
        _XYZ_Satz[i].X = cmsIT8GetDataDbl (_lcms_it8, _Feldnamen[i].c_str(),
                                           "XYZ_X")/100.0;
        _XYZ_Satz[i].Y = cmsIT8GetDataDbl (_lcms_it8, _Feldnamen[i].c_str(),
                                           "XYZ_Y")/100.0;
        _XYZ_Satz[i].Z = cmsIT8GetDataDbl (_lcms_it8, _Feldnamen[i].c_str(),
                                           "XYZ_Z")/100.0;
    }
  }
  if (has_RGB) { DBG // keine Umrechnung nötig
    _RGB_Satz.resize(_nFelder);
    for (int i = 0; i < _nFelder; i++) {
        _RGB_Satz[i].R = cmsIT8GetDataDbl (_lcms_it8, _Feldnamen[i].c_str(),
                                           "RGB_R")/256.0;
        _RGB_Satz[i].G = cmsIT8GetDataDbl (_lcms_it8, _Feldnamen[i].c_str(),
                                           "RGB_G")/256.0;
        _RGB_Satz[i].B = cmsIT8GetDataDbl (_lcms_it8, _Feldnamen[i].c_str(),
                                           "RGB_B")/256.0;
    }
  }
  if (has_CMYK) { DBG // keine Umrechnung nötig
    _CMYK_Satz.resize(_nFelder);
    for (int i = 0; i < _nFelder; i++) { //cout << _Feldnamen[i] << " " << i << " "; DBG
        _CMYK_Satz[i].C = cmsIT8GetDataDbl (_lcms_it8, _Feldnamen[i].c_str(),
                                           "CMYK_C")/100.0;
        _CMYK_Satz[i].M = cmsIT8GetDataDbl (_lcms_it8, _Feldnamen[i].c_str(),
                                           "CMYK_M")/100.0;
        _CMYK_Satz[i].Y = cmsIT8GetDataDbl (_lcms_it8, _Feldnamen[i].c_str(),
                                           "CMYK_Y")/100.0;
        _CMYK_Satz[i].K = cmsIT8GetDataDbl (_lcms_it8, _Feldnamen[i].c_str(),
                                           "CMYK_K")/100.0;
    } DBG
  }


  // Farbnamen nach Geschmack (unmittelbar vor cmsIT8Free !)
  if (_id_vor_name) {
    for (int i = 0; i < _nFelder; i++) {
        _Feldnamen[i] = cmsIT8GetData (_lcms_it8, _Feldnamen[i].c_str(),
                                       "SAMPLE_NAME");
    } DBG_S (_Feldnamen[0] <<" bis "<< _Feldnamen[_nFelder-1] <<" "<< _nFelder)
  }

  // lcms's cgats Leser wird nicht mehr gebraucht
  cmsIT8Free (_lcms_it8);
  _lcms_it8 = NULL;

  #ifdef DEBUG_ICCMEASUREMENT
  // Infos für die Konsole
  DBG_S( "Anzahl Messfelder: " << getPatchCount() << " Samples " << _nKanaele )

  if (_RGB_measurement && _XYZ_measurement) {
    for (int i = 0; i < _nFelder; i++) {
      cout << _Feldnamen[i] << ": " << _XYZ_Satz[i].X << ", " << _XYZ_Satz[i].Y << ", " << _XYZ_Satz[i].Z << ", "  << _RGB_Satz[i].R << ", "  << _RGB_Satz[i].G << ", "  << _RGB_Satz[i].B << endl;
    } DBG
  } else if (_CMYK_measurement && _XYZ_measurement) {
    DBG_S( _Feldnamen.size() << " | " << _CMYK_Satz.size() )
    for (int i = 0; i < _nFelder; i++) {
      cout << _Feldnamen[i] << ": " << _XYZ_Satz[i].X << ", " << _XYZ_Satz[i].Y << ", " << _XYZ_Satz[i].Z << ", "  << _CMYK_Satz[i].C << ", "  << _CMYK_Satz[i].M << ", "  << _CMYK_Satz[i].Y << ", "  << _CMYK_Satz[i].K << endl;
    } DBG
  }

  if (farben > 1
   && ((has_CMYK || has_RGB)
       && (has_Lab || has_XYZ || has_xyY))) {
    DBG_S( "sieht gut aus" )
  }
  #endif
}

void
ICCmeasurement::init_umrechnen                     (void)
{
  if (_RGB_measurement && _XYZ_measurement) { DBG // keine Umrechnung nötig
    cmsHTRANSFORM hRGBtoSRGB, hXYZtoSRGB, hRGBtoXYZ;
    cmsHPROFILE hRGB, hsRGB, hXYZ;
    hRGB = cmsOpenProfileFromMem (_profil->_data, _profil->_size);
    hsRGB = cmsCreate_sRGBProfile ();
    hXYZ = cmsCreateXYZProfile ();
    // Wie sieht das Profil die Messfarbe? -> XYZ
    hRGBtoXYZ =  cmsCreateTransform (hRGB, TYPE_RGB_DBL,
                                    hXYZ, TYPE_XYZ_DBL,
                                    INTENT_RELATIVE_COLORIMETRIC,
                                    cmsFLAGS_NOTPRECALC);
    // Wie sieht das Profil die Messfarbe? -> Bildschirmdarstellung
    hRGBtoSRGB = cmsCreateTransform (hRGB, TYPE_RGB_DBL,
                                    hsRGB, TYPE_RGB_DBL,
                                    INTENT_RELATIVE_COLORIMETRIC,
                                    cmsFLAGS_NOTPRECALC);
    // Wie sieht die CMM die Messfarbe? -> Bildschirmdarstellung
    hXYZtoSRGB = cmsCreateTransform (hXYZ, TYPE_XYZ_DBL,
                                    hsRGB, TYPE_RGB_DBL,
                                    INTENT_RELATIVE_COLORIMETRIC,
                                    cmsFLAGS_NOTPRECALC);
    double RGB[3], sRGB[3], XYZ[3];

    _RGB_MessFarben.resize(_nFelder);
    _RGB_ProfilFarben.resize(_nFelder);
    _XYZ_Ergebnis.resize(_nFelder);
    for (int i = 0; i < _nFelder; i++) {
        XYZ[0] = _XYZ_Satz[i].X;
        XYZ[1] = _XYZ_Satz[i].Y;
        XYZ[2] = _XYZ_Satz[i].Z;
        cmsDoTransform (hXYZtoSRGB, &XYZ[0], &sRGB[0], 1);
        _RGB_MessFarben[i].R = sRGB[0];
        _RGB_MessFarben[i].G = sRGB[1];
        _RGB_MessFarben[i].B = sRGB[2];

        RGB[0] = _RGB_Satz[i].R;
        RGB[1] = _RGB_Satz[i].G;
        RGB[2] = _RGB_Satz[i].B;
        cmsDoTransform (hRGBtoSRGB, &RGB[0], &sRGB[0], 1);
        _RGB_ProfilFarben[i].R = sRGB[0];
        _RGB_ProfilFarben[i].G = sRGB[1];
        _RGB_ProfilFarben[i].B = sRGB[2];

        cmsDoTransform (hRGBtoXYZ, &RGB[0], &XYZ[0], 1);
        _XYZ_Ergebnis[i].X = XYZ[0];
        _XYZ_Ergebnis[i].Y = XYZ[1];
        _XYZ_Ergebnis[i].Z = XYZ[2];
    }
    cmsDeleteTransform (hRGBtoSRGB);
    cmsDeleteTransform (hXYZtoSRGB);
    cmsDeleteTransform (hRGBtoXYZ);
    cmsCloseProfile (hRGB);
    cmsCloseProfile (hsRGB);
    cmsCloseProfile (hXYZ);
  }
}

std::string
ICCmeasurement::getHtmlReport                     (void)
{ DBG
  std::stringstream html;
  if (_RGB_MessFarben.size() == 0)
    init_umrechnen ();

  if (_reportTabelle.size() == 0)
    _reportTabelle = getText();

  html << "<!DOCTYPE html PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\">";
  html << "<html><head>" << endl;
  html << "<title>" << _("Prüfung des Farbprofiles") << "</title>\n";
  html << "<meta http-equiv=\"content-type\" content=\"text/html; charset=ISO-8859-1\">" << endl;
  html << "</head><body>" << endl << endl;

  int kopf = (int)_reportTabelle.size() - _nFelder;
  // Allgemeine Informationen
  for (int i = 0; i < kopf - 1 ; i++) {
    if (i == 0) html << "<h2>";
    html << _reportTabelle[i][0];
    if (i == 0) html << "</h2>";
    html << "<br>\n\n";
  }
  html << "<table align=left cellpadding=\"2\" cellspacing=\"0\" border=\"1\" width=\"100%\" bgcolor=\"#cccccc\">\n\
  <tbody> \n";
  html << "  <tr> \n";
  // Tabellenkopf
  for (int s = 0; s < (int)_reportTabelle [kopf - 1].size(); s++)
    html << "    <td>" << _reportTabelle[kopf - 1][s] << "</td>\n";
  html << "  </tr>\n";
  for (int z = 0; z < _nFelder; z++) {
    html << "  <tr>\n"; 
    for (int s = 0; s < (int)_reportTabelle [kopf - 1].size(); s++)
      html << "    <td>" << _reportTabelle[kopf + z][s] << "</td>\n";
    html << "  </tr>\n";
  }

  html << "</tbody>\n</table> \n\n<br>\n</body></html>\n";

  return html.str();
}

std::vector<std::vector<std::string> >
ICCmeasurement::getText                     (void)
{ DBG
  std::vector<std::vector<std::string> > tabelle;
  tabelle.resize(_nFelder+2); // push_back ist zu langsam

  tabelle[0].resize(1);
  tabelle[0][0] = _("keine Messdaten verfügbar");

  if ((_CMYK_measurement || _RGB_measurement)
       && _XYZ_measurement) {
    // Tabellenüberschrift
    tabelle[0].resize(1);
    tabelle[0][0] =    _("Farbtabelle mit "); 
    if (_RGB_measurement)
      tabelle[0][0] += _("RGB");
    else
      tabelle[0][0] += _("CMYK");
    tabelle[0][0] +=   _(" Messdaten");
    // Tabellenkopf
    tabelle[1].resize(1 + 3 + (_RGB_measurement ? 3 : 4));
    DBG_S((1 + 3 + (_RGB_measurement ? 3 : 4)))
    tabelle[1][0] = _("Messfeld");
    tabelle[1][1] = _("X"); tabelle[1][2] = _("Y"); tabelle[1][3] = _("Z");
    if (_RGB_measurement) {
      tabelle[1][4] = _("R"); tabelle[1][5] =_("G"); tabelle[1][6] = _("B");
    } else {
      tabelle[1][4] = _("C"); tabelle[1][5] =_("M"); tabelle[1][6] = _("Y");
      tabelle[1][7] = _("K");
    }
    // Messwerte
    std::stringstream s;
    for (int i = 0; i < _nFelder; i++) {
      tabelle[2+i].resize(1 + 3 + (_RGB_measurement ? 3 : 4));
      tabelle[2+i][0] =  _Feldnamen[i];
      s << _XYZ_Satz[i].X; tabelle[2+i][1] = s.str().c_str(); s.str("");
      s << _XYZ_Satz[i].Y; tabelle[2+i][2] = s.str().c_str(); s.str("");
      s << _XYZ_Satz[i].Z; tabelle[2+i][3] = s.str().c_str(); s.str("");
      if (_RGB_measurement) {
        s << _RGB_Satz[i].R; tabelle[2+i][4] = s.str().c_str(); s.str("");
        s << _RGB_Satz[i].G; tabelle[2+i][5] = s.str().c_str(); s.str("");
        s << _RGB_Satz[i].B; tabelle[2+i][6] = s.str().c_str(); s.str("");
      } else {
        s << _CMYK_Satz[i].C; tabelle[2+i][4] = s.str().c_str(); s.str("");
        s << _CMYK_Satz[i].M; tabelle[2+i][5] = s.str().c_str(); s.str("");
        s << _CMYK_Satz[i].Y; tabelle[2+i][6] = s.str().c_str(); s.str("");
        s << _CMYK_Satz[i].K; tabelle[2+i][7] = s.str().c_str(); s.str("");
      }
    }
  }

  return tabelle;
}

std::vector<std::string>
ICCmeasurement::getDescription              (void)
{ DBG
  std::vector<std::string> texte;
  std::string text =  "";

  #ifdef DEBUG_ICCMEASUREMENT
  #endif
  return texte;
}

std::vector<double>
ICCmeasurement::getMessRGB                  (int patch)
{
  std::vector<double> punkte;
  punkte.resize(3);

  if (_RGB_MessFarben.size() == 0)
    init_umrechnen ();

  if (patch > _nFelder)
    return punkte;

  punkte[0] = _RGB_MessFarben[patch].R;
  punkte[1] = _RGB_MessFarben[patch].G;
  punkte[2] = _RGB_MessFarben[patch].B;

  return punkte;
}

std::vector<double>
ICCmeasurement::getCmmRGB                   (int patch)
{
  std::vector<double> punkte;
  punkte.resize(3);

  if (_RGB_MessFarben.size() == 0)
    init_umrechnen ();

  if (patch > _nFelder)
    return punkte;

  punkte[0] = _RGB_ProfilFarben[patch].R;
  punkte[1] = _RGB_ProfilFarben[patch].G;
  punkte[2] = _RGB_ProfilFarben[patch].B;

  return punkte;
}


