// Author:    Kai-Uwe Behrmann
// Copyright: Kai-Uwe Behrmann

#ifndef ICC_PROFILE_H
#define ICC_PROFILE_H

#include <icc34.h>
#include <string>
#include <iostream>
#include <sstream>
#include <list>

/**
  *   @brief interne ICC Profilstruktur
  **/

class ICCprofile;
class ICCheader;
class ICCtag;

class ICCheader {
  public:
    icHeader            header;
    icHeader            header_raw();
    void                header_raw (icHeader);
    std::string         print();
};

class ICCtag {
    icTag               tag;
    
  public:
    std::string         get_type();
    std::string         get_vrml();
    std::string         get_text();
//    std::map<int,int>   get_curve();
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

  private:
    std::string         _filename;
    void                read_header();

    // icc34.h Definitionen
    icProfile           profile;

    ICCheader           header;
    std::list<ICCtag>   tags;
    void                write_tagList(std::stringstream);

  public:
    int                 size     () {return header.header.size; }
    std::string         cmm      ();
    void                cmm      (std::string);
    int                 version  () {return (int) header.header.version; }
    std::string         print_header () {return header.print(); }
};

#endif
