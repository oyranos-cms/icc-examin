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
#include <list>
#include <fstream>
#include "icc_utils.h"


using namespace std;

// interne Funktionen

const char* cp_char (char* text);
const char* cp_nchar (char* text, int n);

/**
  *   @brief interne ICC Profilstruktur
  **/

class ICCprofile;
class ICCheader;
class ICCtag;

int                     icValue   (icUInt32Number val);
long                    icValue   (icUInt64Number val);
double                  icValue   (icS15Fixed16Number val);

class ICCheader {
  public:
                        ICCheader(); 
  private:
    icHeader            header;
    map<string,string>  cmm_map;
  public:
    const char*         header_raw ()        {return cp_nchar ((char*)&header,
                                                       sizeof (icSignature)); }
    void                header_raw (void* s) {memcpy ((void*)&header, s,
                                                       sizeof (icHeader)); }
    void                load  (void*);

    int                 valid;
    int                 size    ()      {return icValue(header.size); }
//    void                size    (int size)      {header.size = inbyteswap (size); }
//    void                Cmm     (string s)      {memcpy((char*)&header.cmmId,
//                                                       cmm_map.find(s),
//                                                       sizeof (icSignature)); }
    void                CmmName (const char* s) {memcpy((char*)&header.cmmId, s,
                                                       sizeof (icSignature)); }
    const char*         CmmName ()       {return cp_nchar ((char*)&header.cmmId,
                                                      sizeof (icSignature)+1); }
    int                 version ()       {return icValue(header.version); }
    icProfileClassSignature deviceClass ()   {return header.deviceClass; }
    icColorSpaceSignature colorSpace  ()   {return header.colorSpace; }
    icColorSpaceSignature pcs ()           {return header.pcs; }
    const char*         platform ()        {return getPlatformName(header.platform); }
    std::string         print ();
    std::string         print_long ();
    //char*               getProfileInfo  ();

    const char*         getColorSpaceName  ( icColorSpaceSignature color);
    const char*         getDeviceClassName ( icProfileClassSignature cl);
    const char*         getPlatformName    ( icPlatformSignature platform);
};

class ICCtag {
  public:
                        ICCtag             ();
                        ICCtag             (icTag *tag, char* data);
                        ~ICCtag            ();
  private:
    icTag               _tag;
    char*               _data;

  public:
    void                load               (icTag *tag, char* data);
  public:
    std::string         getTagName()       {return getSigTagName (_tag.sig); }
    //std::string         getTypeName()      {return getSigTypeName (_tag.sig); }
    std::string         get_vrml();
    std::string         get_text();
//    std::map<int,int>   get_curve();
    std::string         getSigTagName      ( icTagSignature  sig );
//    void                printLut           (   LPLUT           Lut,
//                                               int             sig);
};

class ICCprofile {
  public:
                        ICCprofile ();
                        ICCprofile (const char *filename);
    virtual             ~ICCprofile (void);

  public:
    int                 tag_count();
    void                profile_name (std::string filename);
    char*               profile_name ();
    void                load (std::string filename);
    void                load (char* filename);

  private:
    void                fload ();
    std::string         _filename;
    //void                read_header();

    // icc34.h Definitionen
    icProfile          *_data;
    unsigned int        _size;

    ICCheader           header;
    std::list<ICCtag>   tags;
    void                write_tagList(std::stringstream);

  public: // Informationen
    const char*         filename () {return _filename.c_str(); }
    void                filename (const char* s) {_filename = s; }
    int                 size     () {return header.size(); }
    const char*         cmm      () {return header.CmmName(); }
    void                cmm      (const char* s) {header.CmmName (s); }
    int                 version  () {return (int) header.version(); }
    std::string         print_header       () {return header.print(); }
    std::string         print_long_header  () {return header.print_long(); }
    std::list<std::string> printTags       ();
    std::string         printLongTag       (int   item);
    char*               getProfileInfo     ();

    int                 getTagCount        () {return _data->count; }

    void                saveProfileToFile  (char* filename, char *profile,
                                           int    size);
    int                 checkProfileDevice (char* type,
                                           icProfileClassSignature deviceClass);
};


#endif //ICC_PROFILE_H
