// Author:    Kai-Uwe Behrmann
// Copyright: Kai-Uwe Behrmann
// Date:      Mai 2004

#ifndef ICC_PROFILE_H
#define ICC_PROFILE_H

#define ICC_EXAMIN_V 0.12

#include <icc34.h>
#include <string>
#include <iostream>
#include <sstream>
#include <map>
#include <vector>
#include <fstream>
#include "icc_utils.h"

#include <lcms.h> // für it8 E/A


// Zusätze - nicht definiert in icc34.h

#ifndef icSigChromaticityType
#define icSigChromaticityType 0x6368726D
#endif

// interne Funktionen

const char* cp_nchar (char* text, int n);


// interne Typen
typedef struct {
    double X;
    double Y;
    double Z;
} XYZ;

typedef struct {
    double L;
    double a;
    double b;
} Lab;

typedef struct {
    double R;
    double G;
    double B;
} RGB;

typedef struct {
    double C;
    double M;
    double Y;
    double K;
} CMYK;

// Farbfunktionen
// definiert in icc_formeln.cpp
double        dE2000 (Lab Lab1, Lab Lab2, double kL, double kC, double kH);

// Helferfunktionen
// definiert in icc_helfer.cpp
icUInt16Number          icValue   (icUInt16Number val);
icUInt32Number          icValue   (icUInt32Number val);
unsigned long           icValue   (icUInt64Number val);
double                  icValueSF (icS15Fixed16Number val);
double                  icValueUF (icU16Fixed16Number val);
icInt16Number                     icValue   (icInt16Number val);
icInt32Number                     icValue   (icInt32Number val);
int                     icValue   (icInt64Number val);
icColorSpaceSignature   icValue   (icColorSpaceSignature val);
icPlatformSignature     icValue   (icPlatformSignature val);
icProfileClassSignature icValue   (icProfileClassSignature val);
icTagSignature          icValue   (icTagSignature val);

double*                 XYZto_xyY (double* XYZ);
double*                 XYZto_xyY (std::vector<double> XYZ);

std::string         renderingIntentName ( int intent);
std::string         getColorSpaceName   ( icColorSpaceSignature color);
std::vector<std::string> getChannelNames( icColorSpaceSignature color);
std::string         getDeviceClassName  ( icProfileClassSignature cl);
std::string         getPlatformName     ( icPlatformSignature platform);
std::string         getSigTagName( icTagSignature  sig );
std::string         getSigTagDescription( icTagSignature  sig );
std::string         getSigTypeName( icTagTypeSignature  sig );
std::string         getSigTechnology( icTechnologySignature sig );
std::string         getIlluminant( icIlluminant illu );
std::string         getStandardObserver( icStandardObserver obsv );
std::string         getMeasurementGeometry( icMeasurementGeometry measgeo );
std::string         getMeasurementFlare( icMeasurementFlare flare );

/**
  *   @brief interne ICC Profilstruktur
  **/

class ICCprofile;
class ICCheader;
class ICCtag;
class ICCmeasurement;

// definiert in icc_profile.cpp
class ICCheader {
  public:
                        ICCheader (); 
  private:
    icHeader            header;
  public:
    const char*         header_raw ()        {return /*cp_nchar (*/(char*)&header/*,
                                                       sizeof (icHeader))*/; }
    void                header_raw (void* s) {memcpy ((void*)&header, s,
                                                       sizeof (icHeader)); }
    void                load  (void*);

    int                 valid;
    int                 size    ()      {return icValue(header.size); }
    void                size    (icUInt32Number size)   {header.size =
                                                (icUInt32Number)icValue(size); }
    void                cmmName (const char* s){memcpy((char*)&(header.cmmId),s,
                                                       sizeof (icSignature)); }
    const char*         cmmName ()      {return cp_nchar((char*)&(header.cmmId),
                                                      sizeof (icSignature)+1); }
    int                 version ()      {return icValue(header.version); }
    std::string         versionName ();
    icProfileClassSignature deviceClass (){return icValue(header.deviceClass); }
    icColorSpaceSignature colorSpace    (){return icValue(header.colorSpace); }
    icColorSpaceSignature pcs ()          {return icValue(header.pcs); }
    const char*         magicName ()      {return cp_nchar ((char*)&(header.
                                                      magic),
                                                      sizeof (icSignature)+1); }

    std::string         platform ()        {return getPlatformName(icValue(header.platform)); }
    std::string         flags ();
    std::string         attributes ();
    std::string         renderingIntent () {return renderingIntentName( icValue(
                                            header.renderingIntent ) ); }
    const char*         modelName ()       {return cp_nchar ((char*)&(header.
                                                      model),
                                                      sizeof (icSignature)+1); }
    const char*         manufacturerName() {return cp_nchar ((char*)&(header.
                                                      manufacturer),
                                                      sizeof (icSignature)+1); }
    const char*         creatorName ()     {return cp_nchar ((char*)&(header.
                                                      creator),
                                                      sizeof (icSignature)+1); }
    std::string         print ();
    std::string         print_long ();
};

class ICCtag {
    friend class ICCmeasurement;

  public:
                        ICCtag             ();
                        ICCtag             (ICCprofile* profil,
                                            icTag* tag, char* data);
                        ICCtag             (const ICCtag& tag)
                                                          {ICCtag::copy (tag); }
    void                copy               (const ICCtag& tag);
    void                clear              () {_sig = icMaxEnumTag;
                                               DBG_S((int*)_data)
                                               if (_data && _size) free (_data);
                                               _size = 0;
                                               _data = NULL;
                                               _intent = 0;
                                               _color_in = icMaxEnumData;
                                               _color_out = icMaxEnumData;
                                               _profil = NULL; DBG
                                               }
                        ~ICCtag            ();
  private:
    icTagSignature      _sig;
    int                 _size;
    char*               _data;

    int                 _intent; // für mft1/2
    icColorSpaceSignature _color_in;
    icColorSpaceSignature _color_out;

    ICCprofile*         _profil;

  public:
    void                load (ICCprofile* profil ,icTag* tag, char* data);
  public:
    icTagSignature      getSignature ()    {return _sig; }
    std::string         getTagName()       {return getSigTagName (_sig); }
    std::string         getInfo()          {return getSigTagDescription(_sig); }
    std::string         getTypName()       {icTagTypeSignature sig =
                                            ((icTagBase*)_data) ->
                                            sig;
                                            return getSigTypeName(
                                              (icTagTypeSignature)icValue(sig));
                                           }
    int                 getSize()          {return _size; }

    std::vector<double> getCIEXYZ();
    std::vector<double> getCurve();
    typedef enum {
      MATRIX,
      CURVE_IN,
      TABLE,
      CURVE_OUT
    } MftChain;
    std::vector<std::vector<double> >
                        getCurves    (MftChain typ);
    std::vector<double> getNumbers   (MftChain typ);

    std::vector<std::string> getText ();
    std::vector<std::string> getText (MftChain typ);
    std::vector<std::string> getDescription();
    std::string         getVrml();
  public:  // I/O
    const char*         write(int* size)   {*size = _size;
                                            return (const char*)_data; }
};


// definiert in icc_measurement.cpp
class ICCmeasurement {
  public:
                        ICCmeasurement     ();
                        ICCmeasurement     (ICCprofile* profil , ICCtag& tag);
                        ICCmeasurement     (ICCprofile* profil,
                                            icTag& tag, char* data);
                        ~ICCmeasurement    ();
    void                clear(void);

  private:
    // laden und auswerten
    void                init_meas (void);
    // Berechnen aller Mess- und Profilwerte
    void                init_umrechnen (void);
    // ??
    void                pruefen (void);
    // vorbereiten -> cgats
    std::string         ascii_korrigieren ();
    // lcms cgats Leser
    void                lcms_parse (std::string data);

  private:
    icTagSignature      _sig;
    int                 _size;
    char*               _data;

    int                 _nFelder;

    ICCprofile*         _profil;
    bool                _XYZ_measurement;
    bool                _RGB_measurement;
    bool                _CMYK_measurement;
    // Messwerte
    std::vector<XYZ>    _XYZ_Satz;
    std::vector<Lab>    _Lab_Satz;
    std::vector<RGB>    _RGB_Satz;
    std::vector<CMYK>   _CMYK_Satz;
    // Profilwerte
    std::vector<std::string> _Feldnamen;
    std::vector<XYZ>    _XYZ_Ergebnis;
    std::vector<Lab>    _Lab_Ergebnis;
    std::vector<RGB>    _RGB_MessFarben;
    std::vector<RGB>    _RGB_ProfilFarben;
    // Ergebnisse
    std::vector<double> _Lab_Differenz;
    double              _Lab_Differenz_max;
    double              _Lab_Differenz_min;
    double              _Lab_Differenz_Durchschnitt;
    std::vector<double> _DE00_Differenz;
    double              _DE00_Differenz_max;
    double              _DE00_Differenz_min;
    double              _DE00_Differenz_Durchschnitt;

    std::vector<std::vector<std::string> > _reportTabelle;
    std::vector<int>    layout;
  // I/O
  public:
    void                load (ICCprofile* profil , ICCtag& tag);
    void                load (ICCprofile* profil , char *data, size_t size);
  public:
    // grundlegende Infos
    bool                valid (void)       {return (_XYZ_measurement
                                                 && (_RGB_measurement
                                                  || _CMYK_measurement)); }
    bool                hasRGB ()          {return _RGB_measurement; }
    bool                hasCMYK ()         {return _CMYK_measurement; }
    bool                hasXYZ ()          {return _XYZ_measurement; }
    int                 getSize()          {return _size; }
    int                 getPatchCount()    {return _nFelder; }
    // einzelne Werte
    std::vector<double> getMessRGB (int patch);
    std::vector<double> getCmmRGB (int patch);

    // Report
    std::vector<std::vector<std::string> > getText ();
    std::vector<std::string> getDescription();
    std::string         getHtmlReport ();
    std::vector<int>    getLayout ()       {return layout; }

    // Herkunft
    std::string         getTagName()       {return getSigTagName (_sig); }
    std::string         getInfo()          {return getSigTagDescription(_sig); }

};


// definiert in icc_profile.cpp
class ICCprofile {
  friend class ICCtag;
  friend class ICCmeasurement;
  public:
                        ICCprofile ();
                        ICCprofile (const char *filename);
    virtual             ~ICCprofile (void);

  public:
    int                 tagCount()         {return tags.size(); }
    void                load (std::string filename);
    void                load (char* filename);

  private:
    void                fload ();
    std::string         _filename;

    // icc34.h Definitionen
    char*               _data;
    unsigned int        _size;

    ICCheader           header;
    std::vector<ICCtag> tags;
  private: // cgats via lcms
    ICCmeasurement      measurement;

  public: // Informationen
    const char*         filename ()        {return _filename.c_str(); }
    void                filename (const char* s) {_filename = s; }
    int                 size     ()        {return header.size(); }
    const char*         cmm      ()        {return header.cmmName(); }
    void                cmm      (const char* s) {header.cmmName (s); }
    int                 version  ()        {return (int) header.version(); }
    const char*         creator  ()        {return header.creatorName(); }

    std::string         printHeader     () {return header.print(); }
    std::string         printLongHeader () {return header.print_long(); }

    // Tag Infos
    std::vector<std::string> printTags  (); // Liste der einzelnen Tags (5)
    std::vector<std::string> printTagInfo      (int item); // Name,Typ
    std::vector<std::string> getTagText        (int item);    // Inhalt
    std::vector<std::string> getTagDescription (int item);

    std::vector<double>      getTagCIEXYZ      (int item);
    std::vector<double>      getTagCurve       (int item);
    std::vector<std::vector<double> >
                             getTagCurves      (int item, ICCtag::MftChain typ);
    std::vector<double>      getTagNumbers     (int item, ICCtag::MftChain typ);
    std::vector<std::string> getTagChannelNames(int item, ICCtag::MftChain typ);
    bool                hasTagName   (std::string name); // Name
    int                 getTagByName (std::string name); // Name
    int                 getTagCount     () {return icValue(((icProfile*)_data)->
                                                   count); }
    // Profil Infos
    char*               getProfileInfo  ();
    std::vector<double> getWhitePkt   (void);

  public: // Datei I/O
    int                 checkProfileDevice (char* type,
                                           icProfileClassSignature deviceClass);
    void                saveProfileToFile  (char* filename, char *profile,
                                           int    size);
 
  public: // Messwertinfos
    bool                hasMeasurement () {return measurement.valid(); }
    std::string         report ()         {return measurement.getHtmlReport(); }
    ICCmeasurement      getMeasurement () {return measurement; }

  public: // Profilerstellung
    void                addTag (ICCtag tag)  {tags.push_back(tag); }
    void                removeTag (int item);
    void                saveProfileToFile  (char* filename);
  private:
    void                writeTags (void);
    void                writeHeader (void);
    void                writeTagTable (void);
};


#endif //ICC_PROFILE_H
