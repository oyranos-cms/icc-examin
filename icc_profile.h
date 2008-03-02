// Author:    Kai-Uwe Behrmann
// Copyright: Kai-Uwe Behrmann
// Date:      Mai 2004

#ifndef ICC_PROFILE_H
#define ICC_PROFILE_H

#include <icc34.h>
#include <string>
#include <iostream>
#include <sstream>
#include <list>
#include <fstream>
#include "icc_utils.h"


using namespace std;

/**
  *   @brief interne ICC Profilstruktur
  **/

class ICCprofile;
class ICCheader;
class ICCtag;

class ICCheader {
  public:
                        ICCheader(); 
    icHeader            header;
    icHeader            header_raw();
    void                header_raw (icHeader);
    std::string         print ();
    std::string         print_long ();
    void                load  (void*);
    int                 valid;
    char*               getCmmName         ();
    char*               getProfileInfo     ();
    char*               getColorSpaceName  ( icColorSpaceSignature color);
    char*               getDeviceClassName ( icProfileClassSignature cl);
};

class ICCtag {
    icTag               tag;
    
  public:
    std::string         get_type();
    std::string         get_vrml();
    std::string         get_text();
//    std::map<int,int>   get_curve();
    char*               getSigTagName      ( icTagSignature  sig );
//    void                printLut           (   LPLUT           Lut,
//                                               int             sig);
};

class ICCprofile {
  public:
    ICCprofile ();
    ICCprofile (const char *filename);
    virtual ~ICCprofile (void);

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
    int                 size     () {return header.header.size; }
    int                 cmm      () {return header.header.cmmId; }
    void                cmm      (icSignature s) {header.header.cmmId = s; }
    int                 version  () {return (int) header.header.version; }
    std::string         print_header () {return header.print(); }
    std::string         print_long_header () {return header.print_long(); }
    void                printProfileTags   ();
    char*               getProfileInfo     ();

    void                saveProfileToFile  (char* filename, char *profile,
                                           int    size);
    int                 checkProfileDevice (char* type,
                                           icProfileClassSignature deviceClass);
};

// sollen noch in den Klassen verschwinden


#endif //ICC_PROFILE_H
