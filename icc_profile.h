// Author:    Kai-Uwe Behrmann
// Copyright: Kai-Uwe Behrmann
// Date:      Mai 2004

#ifndef ICC_PROFILE_H
#define ICC_PROFILE_H

#include <icc34.h>
#include <string>
#include <iostream>
#include <sstream>
#include <map>
#include <vector>
#include <fstream>
#include "icc_utils.h"


using namespace std;

// interne Funktionen

const char* cp_nchar (char* text, int n);

/**
  *   @brief interne ICC Profilstruktur
  **/

class ICCprofile;
class ICCheader;
class ICCtag;

unsigned int            icValue   (icUInt16Number val);
unsigned int            icValue   (icUInt32Number val);
unsigned long           icValue   (icUInt64Number val);
double                  icValueSF (icS15Fixed16Number val);
double                  icValueUF (icU16Fixed16Number val);
int                     icValue   (icInt16Number val);
int                     icValue   (icInt32Number val);
int                     icValue   (icInt64Number val);
icColorSpaceSignature   icValue   (icColorSpaceSignature val);
icPlatformSignature     icValue   (icPlatformSignature val);
icProfileClassSignature icValue   (icProfileClassSignature val);
icTagSignature          icValue   (icTagSignature val);

double*                 XYZto_xyY (double* XYZ);
double*                 XYZto_xyY (std::vector<double> XYZ);

class ICCheader {
  public:
                        ICCheader (); 
  private:
    icHeader            header;
  public:
    const char*         header_raw ()        {return cp_nchar ((char*)&header,
                                                       sizeof (icSignature)); }
    void                header_raw (void* s) {memcpy ((void*)&header, s,
                                                       sizeof (icHeader)); }
    void                load  (void*);

    int                 valid;
    int                 size    ()      {return icValue(header.size); }
    void                cmmName (const char* s){memcpy((char*)&(header.cmmId),s,
                                                       sizeof (icSignature)); }
    const char*         cmmName ()      {return cp_nchar((char*)&(header.cmmId),
                                                      sizeof (icSignature)+1); }
    int                 version ()      {return icValue(header.version); }
    std::string         versionName ();
    icProfileClassSignature deviceClass (){return icValue(header.deviceClass); }
    icColorSpaceSignature colorSpace  (){return icValue(header.colorSpace); }
    icColorSpaceSignature pcs ()        {return icValue(header.pcs); }
    const char*         magicName ()       {return cp_nchar ((char*)&(header.
                                                      magic),
                                                      sizeof (icSignature)+1); }
    std::string         platform ()     {return getPlatformName(icValue(header.platform)); }
    std::string         flags ();
    std::string         attributes ();
    std::string         renderingIntent ();
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

    std::string         getColorSpaceName  ( icColorSpaceSignature color);
    std::string         getDeviceClassName ( icProfileClassSignature cl);
    std::string         getPlatformName    ( icPlatformSignature platform);
};

class ICCtag {
  public:
                        ICCtag             ();
                        ICCtag             (icTag *tag, char* data);
                        ~ICCtag            ();
  private:
    icTagSignature      _sig;
    int                 _size;
    char*               _data;

  public:
    void                load (icTag *tag, char* data);
  public:
    std::string         getTagName()       {return getSigTagName (_sig); }
    std::string         getTypName()       {//cout << _data << " " ; DBG
                                            icTagTypeSignature sig =
                                            ((icTagBase*)_data) ->
                                            sig;
                                            return getSigTypeName(
                                              (icTagTypeSignature)icValue(sig));
                                           }
    int                 getTagByName();
    int                 getSize()          {return _size; }
    std::string         getMore();

    std::vector<double> getCIExy();
    std::vector<double> getCurve();
    std::string         getText();
    std::vector<std::string> getDescription();
    std::string         getVrml();
/*    int                 hasCurve();
    int                 hasCIExy();
    int                 hasText();
    int                 hasVrml();*/
    std::string         getSigTagName( icTagSignature  sig );
    std::string         getSigTypeName( icTagTypeSignature  sig );
    std::string         getSigTechnology( icTechnologySignature sig );
    std::string         getIlluminant( icIlluminant illu );
    std::string         getStandardObserver( icStandardObserver obsv );
    std::string         getMeasurementGeometry( icMeasurementGeometry measgeo );
    std::string         getMeasurementFlare( icMeasurementFlare flare );
//    void                printLut           (   LPLUT           Lut,
//                                               int             sig);
};

class ICCprofile {
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
    icProfile*          _data;
    unsigned int        _size;

    ICCheader           header;
    std::vector<ICCtag>   tags;
    void                write_tagList(std::stringstream);

  public: // Informationen
    const char*         filename ()        {return _filename.c_str(); }
    void                filename (const char* s) {_filename = s; }
    int                 size     ()        {return header.size(); }
    const char*         cmm      ()        {return header.cmmName(); }
    void                cmm      (const char* s) {header.cmmName (s); }
    int                 version  ()        {return (int) header.version(); }
    std::string         printHeader     () {return header.print(); }
    std::string         printLongHeader () {return header.print_long(); }
    std::vector<std::string> printTags  (); // Liste der einzelnen Tags (5)
    std::vector<std::string> printTagInfo (int item); // Name,Typ
    std::string         getTagText      (int item);    // Inhalt
    std::vector<std::string> getTagDescription  (int item);
    std::vector<double> getTagCIExy  (int item);
    std::vector<double> getTagCurve  (int item);
    char*               getProfileInfo  ();
    bool                hasTagName   (std::string name); // Name
    int                 getTagByName (std::string name); // Name

    int                 getTagCount     () {return icValue(_data->count); }
    std::vector<double> getWhitePkt   (void);

    void                saveProfileToFile  (char* filename, char *profile,
                                           int    size);
    int                 checkProfileDevice (char* type,
                                           icProfileClassSignature deviceClass);
};


#endif //ICC_PROFILE_H
