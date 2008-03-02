/*
 * ICC Examin ist eine ICC Profil Betrachter
 * 
 * Copyright (C) 2004  Kai-Uwe Behrmann 
 *
 * Autor: Kai-Uwe Behrmann <ku.b@gmx.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 * 
 * -----------------------------------------------------------------------------
 *
 * Profilinterpretation
 * 
 */

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
#include "icc_formeln.h"
#include "icc_helfer.h"



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
    const char*         header_raw ()        {DBG_PROG return /*cp_nchar (*/(char*)&header/*,
                                                       sizeof (icHeader))*/; }
    void                header_raw (void* h) {DBG_PROG memcpy ((void*)&header, h,
                                                       sizeof (icHeader)); }
    void                load  (void*);

    int                 valid;
    int                 size    ()      {DBG_PROG return icValue(header.size); }
    void                size    (icUInt32Number size)   {DBG_PROG header.size =
                                                (icUInt32Number)icValue(size); }
    void                cmmName (const char* s){DBG_PROG memcpy((char*)&(header.cmmId),s,
                                                       sizeof (icSignature)); }
    const char*         cmmName ()      {DBG_PROG return cp_nchar((char*)&(header.cmmId),
                                                      sizeof (icSignature)+1); }
    int                 version ()      {DBG_PROG return icValue(header.version); }
    void                version (icUInt32Number v) {DBG_PROG header.version= icValue(v);}
    std::string         versionName ();
    icProfileClassSignature deviceClass ()
                                        {DBG_PROG return icValue(header.deviceClass); }
    void                deviceClass (icProfileClassSignature d)
                                        {DBG_PROG header.deviceClass = icValue(d); }
    icColorSpaceSignature colorSpace () {DBG_PROG return icValue(header.colorSpace); }
    void                colorSpace (icColorSpaceSignature color)
                                        {DBG_PROG header.colorSpace = icValue(color); }
    icColorSpaceSignature pcs ()        {DBG_PROG return icValue(header.pcs); }
    void                pcs (icColorSpaceSignature pcs)
                                        {DBG_PROG header.pcs = icValue(pcs); }
    void                set_current_date ();
    const char*         magicName ()    {DBG_PROG return cp_nchar ((char*)&(header.
                                                      magic),
                                                      sizeof (icSignature)+1); }
    void                set_magic ()    {DBG_PROG char* m = {"acsp"};
                                         header.magic = *(icSignature*)m; }
    std::string         platform ()     {DBG_PROG return getPlatformName(icValue(header.platform)); }
    void                set_platform () {DBG_PROG header.platform = (icValue(icSigSGI));}
    std::string         flags ();
    void                set_embedded_flag()   {DBG_PROG ((char*)&header.flags)[0] =
                                              ((char*)&header.flags)[0] | 0x80;}
    void                unset_embedded_flag() {DBG_PROG ((char*)&header.flags)[0] =
                                              ((char*)&header.flags)[0] & 0x7f;}
    void                set_dependent_flag()  {DBG_PROG ((char*)&header.flags)[0] =
                                              ((char*)&header.flags)[0] | 0x40;}
    void                unset_dependent_flag() {DBG_PROG ((char*)&header.flags)[0] =
                                              ((char*)&header.flags)[0] & 0xbf;}
    const char*         manufacturerName() {DBG_PROG return cp_nchar ((char*)&(header.
                                                      manufacturer),
                                                      sizeof (icSignature)+1); }
    void                set_manufacturer (){DBG_PROG char* m = {"none"};
                                         header.manufacturer= *(icSignature*)m;}
    const char*         modelName ()       {DBG_PROG return cp_nchar ((char*)&(header.
                                                      model),
                                                      sizeof (icSignature)+1); }
    void                set_model ()       {DBG_PROG char* m = {"none"};
                                         header.model= *(icSignature*)m;}
    std::string         attributes ();
    void                set_reflective_attr() {DBG_PROG ((char*)&header.attributes)[0] =
                                              ((char*)&header.attributes)[0]
                                                & 0x7f; }
    void                set_transparency_attr() {DBG_PROG ((char*)&header.attributes)[0] =
                                              ((char*)&header.attributes)[0]
                                                | 0x80; }
    void                set_glossy_attr() {DBG_PROG ((char*)&header.attributes)[0] =
                                              ((char*)&header.attributes)[0]
                                                & 0xbf; }
    void                set_matte_attr() {DBG_PROG ((char*)&header.attributes)[0] =
                                              ((char*)&header.attributes)[0]
                                                | 0x40; }
    void                set_positive_attr() {DBG_PROG ((char*)&header.attributes)[0] =
                                              ((char*)&header.attributes)[0]
                                                & 0xdf; }
    void                set_negative_attr() {DBG_PROG ((char*)&header.attributes)[0] =
                                              ((char*)&header.attributes)[0]
                                                | 0x20; }
    void                set_color_attr() {DBG_PROG ((char*)&header.attributes)[0] =
                                              ((char*)&header.attributes)[0]
                                                & 0xef; }
    void                set_gray_attr() {DBG_PROG ((char*)&header.attributes)[0] =
                                              ((char*)&header.attributes)[0]
                                                | 0x10; }
    std::string         renderingIntent () {DBG_PROG return renderingIntentName( icValue(
                                            header.renderingIntent ) ); }
    void                set_renderingIntent () {DBG_PROG header.renderingIntent =icValue(
                                                0 ); }
    void                set_illuminant ()  {DBG_PROG icValueXYZ (&header.illuminant,
                                                  0.9642, 1.0000,0.8249); }
    const char*         creatorName ()     {DBG_PROG return cp_nchar((char*)&(header.
                                                      creator),
                                                      sizeof (icSignature)+1); }
    void                set_creator ()     {DBG_PROG char* m = {"SB  "};
                                            header.creator= *(icSignature*)m; }
    void                setID ()           {/*char* m = ((char*)&header)[84];
                                            *m = "-";*/ }
    std::string         print ();
    std::string         print_long ();
};

class ICCtag {
    friend class ICCmeasurement;

    void                copy               (const ICCtag& tag)
                        {    _sig = tag._sig;
                             _size = tag._size;
                             DBG_PROG_S("ICCtag::ICCtag <- Kopie _size: " << _size )
                             if (_size && tag._data) {
                               _data = (char*)calloc(sizeof(char),_size);
                               memcpy (_data , tag._data , _size);
                               DBG_MEM_S((int*)tag._data << " -> " << (int*)_data)
                             } else {
                               _data = NULL;
                               _size = 0;
                             }

                             _intent = tag._intent;
                             _color_in = tag._color_in;
                             _color_out = tag._color_out;

                             _profil = tag._profil;
                        }
    void                defaults ()
                        {    DBG_PROG
                             _sig = icMaxEnumTag;
                             _size = 0;
                             _data = NULL;
                             _intent = 0;
                             _color_in = icMaxEnumData;
                             _color_out = icMaxEnumData;
                             _profil = NULL;
                             if (_size) DBG_MEM_V ((int*)_data) ;
                        }
  public:
                        ICCtag             ()             {DBG_PROG defaults(); }
                        ICCtag             (ICCprofile* profil,
                                            icTag* tag, char* data);
                        ICCtag             (const ICCtag& tag)
                                                          {DBG_PROG copy (tag);}
    ICCtag&             operator=          (const ICCtag& tag)
                                                          {DBG_PROG copy (tag); 
                                                           return *this; }
    void                clear              () { DBG_PROG
                                              if (_data && _size) {
                                                DBG_MEM_S("lösche: "<<(int*)_data)
                                                free (_data);
                                              } defaults(); }
                        ~ICCtag            () {DBG_PROG_S("::ICCtag~") clear();}
  private:
    icTagSignature      _sig;
    int                 _size;
    char*               _data;

    int                 _intent; // für mft1/2
    icColorSpaceSignature _color_in;
    icColorSpaceSignature _color_out;
    bool                _to_pcs;

    ICCprofile*         _profil;

  public:
    void                load (ICCprofile* profil ,icTag* tag, char* data);
  public:
    icTagSignature      getSignature ()    {DBG_PROG return _sig; }
    std::string         getTagName()       {/*DBG_PROG*/ return getSigTagName (_sig); }
    std::string         getInfo()          {DBG_PROG return getSigTagDescription(_sig); }
    std::string         getTypName()       {DBG_PROG icTagTypeSignature sig =
                                            ((icTagBase*)_data) ->
                                            sig;
                                            return getSigTypeName(
                                              (icTagTypeSignature)icValue(sig));
                                           }
    int                 getSize()          {DBG_PROG return _size; }

    std::vector<double> getCIEXYZ();
    std::vector<double> getCurve();
    typedef enum {
      MATRIX,
      CURVE_IN,
      TABLE,
      TABLE_IN,
      TABLE_OUT,
      CURVE_OUT
    } MftChain;
    std::vector<std::vector<double> >
                        getCurves    (MftChain typ);
    std::vector<std::vector<std::vector<std::vector<double> > > >
                        getTable    (MftChain typ);
    std::vector<double> getNumbers   (MftChain typ);

    std::vector<std::string> getText ();
    std::vector<std::string> getText (MftChain typ);
    std::vector<std::string> getDescription();
    std::string         getVrml();
  public:  // I/O
    const char*         write(int* size)   {DBG_PROG *size = _size;
                                            return (const char*)_data; }
};


// definiert in icc_measurement.cpp
class ICCmeasurement {
    void                copy (const ICCmeasurement& m)
                        {    _sig = m._sig; DBG_PROG_START
                             _size = m._size;
                             if (_size && m._data) {
                               _data = (char*)calloc(sizeof(char),_size);
                               memcpy (_data , m._data , _size);
                               DBG_MEM_S((int*)m._data << " -> " << (int*)_data)
                             } else {
                               _data = NULL;
                               _size = 0;
                             }

                             _nFelder = m._nFelder;
                             _channels = m._channels;
                             _isMatrix = m._isMatrix;
                             _profil = m._profil;
                             _XYZ_measurement = m._XYZ_measurement;
                             _RGB_measurement = m._RGB_measurement;
                             _CMYK_measurement = m._CMYK_measurement;
                             // Messwerte
                             _XYZ_Satz = m._XYZ_Satz;
                             _Lab_Satz = m._Lab_Satz;
                             _RGB_Satz = m._RGB_Satz;
                             _CMYK_Satz = m._CMYK_Satz;
                             // Profilwerte
                             _Feldnamen = m._Feldnamen;
                             _XYZ_Ergebnis = m._XYZ_Ergebnis;
                             _Lab_Ergebnis = m._Lab_Ergebnis;
                             _RGB_MessFarben = m._RGB_MessFarben;
                             _RGB_ProfilFarben = m._RGB_ProfilFarben;
                             // Ergebnisse
                             _Lab_Differenz = m._Lab_Differenz;
                             _Lab_Differenz_max = m._Lab_Differenz_max;
                             _Lab_Differenz_min = m._Lab_Differenz_min;
                             _Lab_Differenz_Durchschnitt = m._Lab_Differenz_Durchschnitt;
                             _DE00_Differenz = m._DE00_Differenz;
                             _DE00_Differenz_max = m._DE00_Differenz_max;
                             _DE00_Differenz_min = m._DE00_Differenz_min;
                             _DE00_Differenz_Durchschnitt = m._DE00_Differenz_Durchschnitt;
                             DBG_PROG_ENDE
                        }
    void                defaults ()
                        {    DBG_PROG
                            _sig = icMaxEnumTag;
                            _size = 0;
                            _data = NULL;

                            _nFelder = 0;

                            _channels = 0;
                            _isMatrix = 0;
                            _profil = NULL;
                            _XYZ_measurement = false;
                            _RGB_measurement = false;
                            _CMYK_measurement = false;
                            _Lab_Differenz_max = -1000;
                            _Lab_Differenz_min = 1000;
                            _Lab_Differenz_Durchschnitt = 0;
                            _DE00_Differenz_max = -1000;
                            _DE00_Differenz_min = 1000;
                            _DE00_Differenz_Durchschnitt = 0;
                        }
  public:
                        ICCmeasurement     () { DBG_PROG defaults(); }
                        ICCmeasurement     (ICCprofile* profil , ICCtag& tag);
                        ICCmeasurement     (ICCprofile* profil,
                                            icTag& tag, char* data);
    void                clear (void)
                        {   DBG_PROG
                            if (_data != NULL) free(_data);
                            defaults();
                            _XYZ_Satz.clear();
                            _Lab_Satz.clear();
                            _RGB_Satz.clear();
                            _CMYK_Satz.clear();
                            _Feldnamen.clear();
                            _XYZ_Ergebnis.clear();
                            _Lab_Ergebnis.clear();
                            _RGB_MessFarben.clear();
                            _RGB_ProfilFarben.clear();
                            _Lab_Differenz.clear();
                            _DE00_Differenz.clear();
                            _reportTabelle.clear();
                            layout.clear();
                        }
                        ~ICCmeasurement() {DBG_PROG_S("::~ICCmeasurement") clear(); }
    void                init (void);
                        ICCmeasurement     (const ICCmeasurement& m)
                                                          {DBG_PROG copy (m); }
    ICCmeasurement&     operator=          (const ICCmeasurement& m)
                                                          {DBG_PROG copy (m); 
                                                           return *this; }

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

    ICCprofile*         _profil;
    int                 _channels;
    int                 _isMatrix;

    int                 _nFelder;

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
    bool                has_data (void)    {DBG_PROG return (_XYZ_Satz.size() ||
                                                    ( _data && _size ) ); }
    bool                valid (void)       {DBG_PROG return (_XYZ_measurement
                                                 && (_RGB_measurement
                                                  || _CMYK_measurement)); }
    bool                hasRGB ()          {DBG_PROG return _RGB_measurement; }
    bool                hasCMYK ()         {DBG_PROG return _CMYK_measurement; }
    bool                hasXYZ ()          {DBG_PROG return _XYZ_measurement; }
    int                 getSize()          {DBG_PROG return _size; }
    int                 getPatchCount()    {DBG_PROG return _nFelder; }
    // Werte
    std::vector<double> getMessRGB (int patch);
    std::vector<double> getCmmRGB (int patch);
    std::vector<XYZ>    getMessXYZ ()      {DBG_PROG return _XYZ_Satz; }
    std::vector<RGB>    getMessRGB ()      {DBG_PROG return _RGB_Satz; }
    std::vector<CMYK>   getMessCMYK ()     {DBG_PROG return _CMYK_Satz; }

    // Report
    std::vector<std::vector<std::string> > getText ();
    std::vector<std::string> getDescription();
    std::string         getHtmlReport ();
    std::vector<int>    getLayout ()       {DBG_PROG return layout; }

    // Herkunft
    std::string         getTagName()       {DBG_PROG return getSigTagName (_sig); }
    std::string         getInfo()          {DBG_PROG return getSigTagDescription(_sig); }

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
    int                 tagCount()         {DBG_PROG return tags.size(); }
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
    const char*         filename ()        {DBG_PROG return _filename.c_str(); }
    void                filename (const char* s) {DBG_PROG _filename = s; }
    int                 size     ()        {DBG_PROG return header.size(); }
    //const char*         cmm      ()        {DBG_PROG return header.cmmName(); }
    //void                cmm      (const char* s) {DBG_PROG header.cmmName (s); }
    //int                 version  ()        {DBG_PROG return (int) header.version(); }
    //const char*         creator  ()        {DBG_PROG return header.creatorName(); }

    std::string         printHeader     () {DBG_PROG return header.print(); }
    std::string         printLongHeader () {DBG_PROG return header.print_long(); }
    std::vector<std::string> getPCSNames       () {DBG_PROG return
                                                getChannelNames(header.pcs()); }

    // Tag Infos
    std::vector<std::string> printTags  (); // Liste der einzelnen Tags (5)
    std::vector<std::string> printTagInfo      (int item); // Name,Typ
    std::vector<std::string> getTagText        (int item);    // Inhalt
    std::vector<std::string> getTagDescription (int item);

    std::vector<double>      getTagCIEXYZ      (int item);
    std::vector<double>      getTagCurve       (int item);
    std::vector<std::vector<double> >
                             getTagCurves      (int item, ICCtag::MftChain typ);
    std::vector<std::vector<std::vector<std::vector<double> > > >
                             getTagTable       (int item, ICCtag::MftChain typ);
    std::vector<double>      getTagNumbers     (int item, ICCtag::MftChain typ);
    std::vector<std::string> getTagChannelNames(int item, ICCtag::MftChain typ);
    bool                hasTagName   (std::string name); // Name
    int                 getTagByName (std::string name); // Name
    int                 getTagCount     () {DBG_PROG return icValue(((icProfile*)_data)->
                                                   count); }
    // Profil Infos
    char*               getProfileInfo  ();
    std::vector<double> getWhitePkt   (void);
    int                 getColourChannelsCount ();
    int                 hasCLUT ();

  public: // Datei I/O
    int                 checkProfileDevice (char* type,
                                           icProfileClassSignature deviceClass);
    void                saveMemToFile  (char* filename, char *block,
                                           int size);
 
  public: // Messwertinfos
    bool                hasMeasurement() {DBG_PROG return (hasTagName("targ") ||
                                    (hasTagName("CIED")&&hasTagName("DevD"))); }
    std::string         report ()         {DBG_PROG return measurement.getHtmlReport(); }
    ICCmeasurement&     getMeasurement () {DBG_PROG if (hasMeasurement())
                                                 measurement.init();
                                               return measurement; }

  public: // Profilerstellung
    void                setHeader (void* h) {DBG_PROG header.header_raw(h); }
    void                addTag (ICCtag tag)  {DBG_PROG tags.push_back(tag); }
    ICCtag&             getTag (int item)  {return tags[item]; }
    void                removeTag (int item);
    void                removeTagByName (std::string name) { DBG_PROG
                          if (hasTagName(name))
                            removeTag (getTagByName(name)); }
    void                saveProfileToFile  (char* filename);
    int                 getProfileSize  (void);
    char*               saveProfileToMem  (int* size);
  private:
    void                writeTags (void);
    void                writeHeader (void);
    void                writeTagTable (void);
};


#endif //ICC_PROFILE_H
