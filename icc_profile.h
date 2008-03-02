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
double                  icSFValue (icS15Fixed16Number val);
double                  icUFValue (icU16Fixed16Number val);
icS15Fixed16Number      icValueSF (double val);
icU16Fixed16Number      icValueUF (double val);
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
std::string         printDatum( icDateTimeNumber date );

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
    const char*         header_raw ()        {DBG return /*cp_nchar (*/(char*)&header/*,
                                                       sizeof (icHeader))*/; }
    void                header_raw (void* h) {DBG memcpy ((void*)&header, h,
                                                       sizeof (icHeader)); }
    void                load  (void*);

    int                 valid;
    int                 size    ()      {DBG return icValue(header.size); }
    void                size    (icUInt32Number size)   {DBG header.size =
                                                (icUInt32Number)icValue(size); }
    void                cmmName (const char* s){DBG memcpy((char*)&(header.cmmId),s,
                                                       sizeof (icSignature)); }
    const char*         cmmName ()      {DBG return cp_nchar((char*)&(header.cmmId),
                                                      sizeof (icSignature)+1); }
    int                 version ()      {DBG return icValue(header.version); }
    void                version (icUInt32Number v) {DBG header.version= icValue(v);}
    std::string         versionName ();
    icProfileClassSignature deviceClass ()
                                        {DBG return icValue(header.deviceClass); }
    void                deviceClass (icProfileClassSignature d)
                                        {DBG header.deviceClass = icValue(d); }
    icColorSpaceSignature colorSpace () {DBG return icValue(header.colorSpace); }
    void                colorSpace (icColorSpaceSignature color)
                                        {DBG header.colorSpace = icValue(color); }
    icColorSpaceSignature pcs ()        {DBG return icValue(header.pcs); }
    void                pcs (icColorSpaceSignature pcs)
                                        {DBG header.pcs = icValue(pcs); }
    void                set_current_date ();
    const char*         magicName ()    {DBG return cp_nchar ((char*)&(header.
                                                      magic),
                                                      sizeof (icSignature)+1); }
    void                set_magic ()    {DBG char* m = {"acsp"};
                                         header.magic = *(icSignature*)m; }
    std::string         platform ()     {DBG return getPlatformName(icValue(header.platform)); }
    void                set_platform () {DBG header.platform = (icValue(icSigSGI));}
    std::string         flags ();
    void                set_embedded_flag()   {DBG ((char*)&header.flags)[0] =
                                              ((char*)&header.flags)[0] | 0x80;}
    void                unset_embedded_flag() {DBG ((char*)&header.flags)[0] =
                                              ((char*)&header.flags)[0] & 0x7f;}
    void                set_dependent_flag()  {DBG ((char*)&header.flags)[0] =
                                              ((char*)&header.flags)[0] | 0x40;}
    void                unset_dependent_flag() {DBG ((char*)&header.flags)[0] =
                                              ((char*)&header.flags)[0] & 0xbf;}
    const char*         manufacturerName() {DBG return cp_nchar ((char*)&(header.
                                                      manufacturer),
                                                      sizeof (icSignature)+1); }
    void                set_manufacturer (){DBG char* m = {"none"};
                                         header.manufacturer= *(icSignature*)m;}
    const char*         modelName ()       {DBG return cp_nchar ((char*)&(header.
                                                      model),
                                                      sizeof (icSignature)+1); }
    void                set_model ()       {DBG char* m = {"none"};
                                         header.model= *(icSignature*)m;}
    std::string         attributes ();
    void                set_reflective_attr() {DBG ((char*)&header.attributes)[0] =
                                              ((char*)&header.attributes)[0]
                                                & 0x7f; }
    void                set_transparency_attr() {DBG ((char*)&header.attributes)[0] =
                                              ((char*)&header.attributes)[0]
                                                | 0x80; }
    void                set_glossy_attr() {DBG ((char*)&header.attributes)[0] =
                                              ((char*)&header.attributes)[0]
                                                & 0xbf; }
    void                set_matte_attr() {DBG ((char*)&header.attributes)[0] =
                                              ((char*)&header.attributes)[0]
                                                | 0x40; }
    void                set_positive_attr() {DBG ((char*)&header.attributes)[0] =
                                              ((char*)&header.attributes)[0]
                                                & 0xdf; }
    void                set_negative_attr() {DBG ((char*)&header.attributes)[0] =
                                              ((char*)&header.attributes)[0]
                                                | 0x20; }
    void                set_color_attr() {DBG ((char*)&header.attributes)[0] =
                                              ((char*)&header.attributes)[0]
                                                & 0xef; }
    void                set_gray_attr() {DBG ((char*)&header.attributes)[0] =
                                              ((char*)&header.attributes)[0]
                                                | 0x10; }
    std::string         renderingIntent () {DBG return renderingIntentName( icValue(
                                            header.renderingIntent ) ); }
    void                set_renderingIntent () {DBG header.renderingIntent =icValue(
                                                0 ); }
    void                set_illuminant ()  {DBG 
                                        header.illuminant.X = icValueSF(0.9642);
                                        header.illuminant.Y = icValueSF(1.0000);
                                        header.illuminant.Z = icValueSF(0.8249);
                                           } 
    const char*         creatorName ()     {DBG return cp_nchar ((char*)&(header.
                                                      creator),
                                                      sizeof (icSignature)+1); }
    void                set_creator ()     {DBG char* m = {"SB  "};
                                            header.creator= *(icSignature*)m; }
    void                setID ()           {/*char* m = ((char*)&header)[84];
                                            *m = "-";*/ }
    std::string         print ();
    std::string         print_long ();
};

class ICCtag {
    friend class ICCmeasurement;

    void                copy               (const ICCtag& tag);
  public:
                        ICCtag             ();
                        ICCtag             (ICCprofile* profil,
                                            icTag* tag, char* data);
                        ICCtag             (const ICCtag& tag)
                                                          {DBG copy (tag); }
    ICCtag&             operator=          (const ICCtag& tag)
                                                          {DBG copy (tag); 
                                                           return *this; }
    void                clear              () {DBG _sig = icMaxEnumTag;
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
    icTagSignature      getSignature ()    {DBG return _sig; }
    std::string         getTagName()       {DBG return getSigTagName (_sig); }
    std::string         getInfo()          {DBG return getSigTagDescription(_sig); }
    std::string         getTypName()       {DBG icTagTypeSignature sig =
                                            ((icTagBase*)_data) ->
                                            sig;
                                            return getSigTypeName(
                                              (icTagTypeSignature)icValue(sig));
                                           }
    int                 getSize()          {DBG return _size; }

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
    const char*         write(int* size)   {DBG *size = _size;
                                            return (const char*)_data; }
};


// definiert in icc_measurement.cpp
class ICCmeasurement {
  public:
                        ICCmeasurement     ();
                        ICCmeasurement     (ICCprofile* profil , ICCtag& tag);
                        ICCmeasurement     (ICCprofile* profil,
                                            icTag& tag, char* data);
    void                clear(void);
                        ~ICCmeasurement    () {clear(); }
    void                init (void);

  private:
    // laden und auswerten
    void                leseTag ();
    // Berechnen aller Mess- und Profilwerte
    void                init_umrechnen (void);
    // ??
    void                pruefen (void);
    // vorbereiten -> cgats
    std::string         ascii_korrigieren ();
    // lcms cgats Leser
    void                lcms_parse ();

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
    bool                has_data (void)    {DBG return (_XYZ_Satz.size() ||
                                                    ( _data && _size ) ); }
    bool                valid (void)       {DBG return (_XYZ_measurement
                                                 && (_RGB_measurement
                                                  || _CMYK_measurement)); }
    bool                hasRGB ()          {DBG return _RGB_measurement; }
    bool                hasCMYK ()         {DBG return _CMYK_measurement; }
    bool                hasXYZ ()          {DBG return _XYZ_measurement; }
    int                 getSize()          {DBG return _size; }
    int                 getPatchCount()    {DBG return _nFelder; }
    // Werte
    std::vector<double> getMessRGB (int patch);
    std::vector<double> getCmmRGB (int patch);
    std::vector<XYZ>    getMessXYZ ()      {DBG return _XYZ_Satz; }
    std::vector<RGB>    getMessRGB ()      {DBG return _RGB_Satz; }
    std::vector<CMYK>   getMessCMYK ()     {DBG return _CMYK_Satz; }

    // Report
    std::vector<std::vector<std::string> > getText ();
    std::vector<std::string> getDescription();
    std::string         getHtmlReport ();
    std::vector<int>    getLayout ()       {DBG return layout; }

    // Herkunft
    std::string         getTagName()       {DBG return getSigTagName (_sig); }
    std::string         getInfo()          {DBG return getSigTagDescription(_sig); }

};


// definiert in icc_profile.cpp
class ICCprofile {
  friend class ICCtag;
  friend class ICCmeasurement;
  public:
                        ICCprofile ();
                        ICCprofile (const char *filename);
    virtual             ~ICCprofile (void);
    void                clear (void);

  public:
    int                 tagCount()         {DBG return tags.size(); }
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
    const char*         filename ()        {DBG return _filename.c_str(); }
    void                filename (const char* s) {DBG _filename = s; }
    int                 size     ()        {DBG return header.size(); }
    const char*         cmm      ()        {DBG return header.cmmName(); }
    void                cmm      (const char* s) {DBG header.cmmName (s); }
    int                 version  ()        {DBG return (int) header.version(); }
    const char*         creator  ()        {DBG return header.creatorName(); }

    std::string         printHeader     () {DBG return header.print(); }
    std::string         printLongHeader () {DBG return header.print_long(); }

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
    int                 getTagCount     () {DBG return icValue(((icProfile*)_data)->
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
    bool                hasMeasurement () {DBG return (hasTagName("targ") ||
                                    (hasTagName("CIED")&&hasTagName("DevD"))); }
    std::string         report ()         {DBG return measurement.getHtmlReport(); }
    ICCmeasurement&     getMeasurement () {DBG if (hasMeasurement())
                                                 measurement.init();
                                               return measurement; }

  public: // Profilerstellung
    void                setHeader (void* h) {DBG header.header_raw(h); }
    void                addTag (ICCtag tag)  {DBG tags.push_back(tag); }
    ICCtag&             getTag (int item)  {return tags[item]; }
    void                removeTag (int item);
    void                saveProfileToFile  (char* filename);
  private:
    void                writeTags (void);
    void                writeHeader (void);
    void                writeTagTable (void);
};


#endif //ICC_PROFILE_H
