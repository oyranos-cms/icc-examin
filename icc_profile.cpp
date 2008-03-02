/*
 * ICC Examin ist eine ICC Profil Betrachter
 * 
 * Copyright (C) 2004-2005  Kai-Uwe Behrmann 
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

// Date:      04. 05. 2004

#if 0
  #ifndef DEBUG
   #define DEBUG
  #endif
  #define DEBUG_ICCPROFILE
#endif

#include "icc_profile.h"
#include "icc_utils.h"

#include <cmath>

#define g_message printf



/**
  *  @brief ICCprofile Funktionen
  */

ICCprofile::ICCprofile (void)
{ DBG_PROG_START
  _data = NULL;
  _size = 0;
  DBG_PROG_ENDE
}

ICCprofile::ICCprofile (const char *filename)
  : _filename (filename)
{ DBG_PROG_START
  if (_data && _size) free(_data);//delete [] _data;
  _data = NULL;
  _size = 0;

  // delegieren
  _filename = filename;
  fload ();
  DBG_PROG_ENDE
}

ICCprofile::~ICCprofile (void)
{ DBG_PROG_START
  this->clear();

  #ifdef DEBUG_PROFILE
  DBG_S ( "~ICCprofile beendet" )
  #endif
  DBG_PROG_ENDE
}
 
void
ICCprofile::clear (void)
{ DBG_PROG_START
  DBG_PROG_S( "Profil wird geleert" )

  if (_data && _size) free(_data);
  _data = NULL;
  _size = 0;

  _filename = "";
  header.load(NULL);

  tags.clear();
  measurement.clear();

  DBG_NUM_S( "_data, tags und measurement gelöscht" )
  DBG_PROG_ENDE
}

void
ICCprofile::load (std::string filename)
{ DBG_PROG_START
  // delegieren
  _filename = filename;
  fload ();
  DBG_PROG_ENDE
}

void
ICCprofile::load (char* filename)
{ DBG_PROG_START
  // delegieren
  _filename = filename;
  fload();
  DBG_PROG_ENDE
}

void
ICCprofile::fload ()
{ DBG_PROG_START // ICC Profil laden
  std::string file = _filename;

  try {
    _data = ladeDatei (file, &_size);
    DBG_MEM
  }
    catch (Ausnahme & a) {	// fängt alles von Ausnahme Abstammende
        printf ("Ausnahme aufgetreten: %s\n", a.what());
        a.report();
        _filename = "";
    }
    catch (std::exception & e) { // fängt alles von exception Abstammende
        printf ("Std-Ausnahme aufgetreten: %s\n", e.what());
        _filename = "";
    }
    catch (...) {		// fängt alles Übriggebliebene
        printf ("Huch, unbekannte Ausnahme\n");
        _filename = "";
    }

  DBG_MEM_V( (int*)_data <<" "<< _size )

  if (_data && _size) {
    WARN_S( _("!!!! Profil wird wiederbenutzt !!!! ") )
    clear();
    // zweites mal Laden nach clear() ; könnte optimiert werden
    _data = ladeDatei (file, &_size);
    _filename = file;
  }

  // Test   > 132 byte
  if (_size < 132) {
    WARN_S( _("Kein Profil")<<_(" Größe ")<<_size )
    measurement.load( this, _data, _size );
    DBG_PROG_ENDE
    return;
  }

  DBG_PROG
  //Kopf
  header.load ((void*)_data); DBG_PROG

  // Test acsp
  char magic[5];
  memcpy( magic, header.magicName(), 4); magic[4] = 0;
  if (strstr(magic, "acsp") == 0)
  {
    WARN_S( _("Kein Profil") )
    header.clear();

    // "targ"  Messdaten als Block hinzufügen
    int groesse = 8 + _size + 1;
    char* tag_block = (char*) calloc (sizeof (char), groesse);
    icTag ic_tag;
    ICCtag tag;

    ic_tag.size = icValue ((icUInt32Number)groesse); DBG_V( groesse )
    ic_tag.offset = 0;
    ic_tag.sig = icValue (icSigCharTargetTag);

    char sig[] = "text";
    memcpy (&tag_block[0], &sig, 4); DBG_S( tag_block )
    memcpy (&tag_block[8], _data, _size);

    tag.load( this, &ic_tag, tag_block );
    addTag( tag );

    free (tag_block);

    measurement.load( this, tag );
    DBG_PROG_ENDE
    return;
  }
   
  //Profilabschnitte
  // TagTabelle bei 132 abholen
  icTag *tagList = (icTag*)&((char*)_data)[132];
  //(icTag*) new char ( getTagCount() * sizeof (icTag));
  //memcpy (tagList , &((char*)_data)[132], sizeof (icTag) * getTagCount());
  DBG_PROG
  tags.resize(getTagCount()); DBG_PROG
  for (int i = 0 ; i < getTagCount() ; i++) { DBG_PROG
    tags[i].load( this, &tagList[i] ,
              &((char*)_data)[ icValue(tagList[i].offset) ]); DBG_PROG
    #ifdef DEBUG_ICCPROFILE
    cout << " sig: " << tags[i].getTagName() << " " << i << " "; DBG_PROG
    #endif

    // bekannte Tags mit Messdaten
    if (tags[i].getTagName() == ("targ")
     || tags[i].getTagName() == ("DevD")
     || tags[i].getTagName() == ("CIED")) {
      #ifdef DEBUG_ICCPROFILE
      DBG_S( "Messdaten gefunden " << tags[i].getTagName() )
      #endif
      measurement.load( this, tags[i] );
    }
  }
  #ifdef DEBUG_ICCPROFILE
  DBG_S( "TagCount: " << getTagCount() << " / " << tags.size() )
  #endif

  DBG_NUM_V( _filename )

  DBG_PROG_ENDE
}

std::vector<std::string>
ICCprofile::printTagInfo         (int item)
{ DBG_PROG_START
  std::vector<std::string> liste;

  liste.push_back( tags.at(item).getTagName() );
  liste.push_back( tags.at(item).getTypName() );

  DBG_PROG_ENDE
  return liste;
}

std::vector<std::string>
ICCprofile::printTags            ()
{ DBG_PROG_START
  std::vector<std::string> StringList;
  std::string text;
  std::stringstream s;

  if (!tags.size()) { DBG_PROG_ENDE
    return StringList;
  } DBG_MEM

  int count = 0;
  for (std::vector<ICCtag>::iterator it = tags.begin(); it != tags.end(); it++){
    s.str("");
    s << count; count++;           StringList.push_back(s.str()); s.str("");
    s.str((*it).getTagName());     StringList.push_back(s.str()); s.str("");
    s.str((*it).getTypName());     StringList.push_back(s.str()); s.str("");
    s << (*it).getSize();          StringList.push_back(s.str()); s.str("");
    s.str((*it).getInfo()); StringList.push_back(s.str()); s.str("");
  #ifdef DEBUG_ICCPROFILE
    DBG_S( (*it).getTagName() << " " << count )
  #endif
  }
  DBG_PROG_ENDE
  return StringList;
}

std::vector<std::string>
ICCprofile::getTagText                                  (int item)
{ DBG_PROG_START
  // Prüfen
  std::string name = tags[item].getTypName();
  std::string leer = name + " Typ - keine Textausgabe";
  std::vector<std::string> v;
  v.push_back( leer );
#if 0
  if (name != "chrm"
   && name != "cprt?"
   && name != "dtim"
   && name != "meas"
   && name != "mft1"
   && name != "mft2"
   && name != "sig"
   && name != "text"
   && name != "vcgt"
   && name != "XYZ")
    return v;
#endif
  DBG_PROG_ENDE
  return tags.at(item).getText();
}

std::vector<std::string>
ICCprofile::getTagChannelNames                          (int item,
                                                         ICCtag::MftChain typ)
{ DBG_PROG_START
  // Prüfen
  std::string leer = tags[item].getTypName() + " Typ - keine Textausgabe";
  std::vector<std::string> v;
  v.push_back( leer );

  // Prüfen
  if (tags[item].getTypName() != "mft2"
   && tags[item].getTypName() != "mft1")
  { DBG_PROG_ENDE
    return v;
  }

  DBG_PROG_ENDE
  return tags.at(item).getText(typ);
}

std::vector<std::string>
ICCprofile::getTagDescription                    (int item)
{ DBG_PROG_START
  // Prüfen
  std::vector<std::string> leer;
  if (tags[item].getTypName() != "desc") { DBG_PROG_ENDE return leer; }

  DBG_PROG_ENDE
  return tags.at(item).getDescription();
}

std::vector<double>
ICCprofile::getTagCIEXYZ                         (int item)
{ DBG_PROG_START
  // Prüfen
  std::vector<double> XYZ;

  if ( tags[item].getTypName() == "XYZ"
    || tags[item].getTypName() == "chrm")
    XYZ = tags.at(item).getCIEXYZ();

  DBG_PROG_ENDE
  return XYZ;
}

std::vector<double>
ICCprofile::getTagCurve                          (int item)
{ DBG_PROG_START
  // Prüfen
  std::vector<double> leer;
  if (tags[item].getTypName() != "curv")
  {
    #ifdef DEBUG_ICCPROFILE
    DBG_S( tags[item].getTypName() )
    #endif
    DBG_PROG_ENDE
    return leer;
  }

  DBG_PROG_ENDE
  return tags.at(item).getCurve();
}

std::vector<std::vector<double> >
ICCprofile::getTagCurves                         (int item,ICCtag::MftChain typ)
{ DBG_PROG_START
  // Prüfen
  std::vector<std::vector<double> > leer;
  if (tags[item].getTypName() != "mft2"
   && tags[item].getTypName() != "mft1"
   && tags[item].getTypName() != "vcgt")
  {
    #ifdef DEBUG_ICCPROFILE
    DBG_S( "gibt nix für " << tags[item].getTypName() )
    #endif
    DBG_PROG_ENDE
    return leer;
  }

  DBG_PROG_ENDE
  return tags.at(item).getCurves(typ);
}

std::vector<std::vector<std::vector<std::vector<double> > > >
ICCprofile::getTagTable                         (int item,ICCtag::MftChain typ)
{ DBG_PROG_START
  // Prüfen
  std::vector<std::vector<std::vector<std::vector<double> > > > leer;
  if (tags[item].getTypName() != "mft2"
   && tags[item].getTypName() != "mft1")
  {
    #ifdef DEBUG_ICCPROFILE
    DBG_S( "gibt nix für " << tags[item].getTypName() )
    #endif
    DBG_PROG_ENDE
    return leer;
  }

  DBG_PROG_ENDE
  return tags.at(item).getTable(typ);
}

std::vector<double>
ICCprofile::getTagNumbers                        (int item,ICCtag::MftChain typ)
{ DBG_PROG_START
  // Prüfen
  std::vector<double> leer;
  if (tags[item].getTypName() != "mft2"
   && tags[item].getTypName() != "mft1")
  {
    #ifdef DEBUG_ICCPROFILE
    DBG_S( tags[item].getTypName() )
    #endif
    DBG_PROG_ENDE
    return leer;
  }
  DBG_PROG_ENDE
  return tags.at(item).getNumbers(typ);
}

int
ICCprofile::getTagByName            (std::string name)
{ DBG_PROG_START
  if (!tags.size()) { DBG_PROG_ENDE 
    return -1;
  } DBG_MEM

  int item = 0;
  for (std::vector<ICCtag>::iterator it = tags.begin(); it != tags.end(); it++){
    if ( (*it).getTagName() == name
      && (*it).getSize()            ) {
      #ifdef DEBUG_ICCPROFILE
      DBG_S( item << "=" << (*it).getTagName() << " gefunden" )
      #endif
      DBG_PROG_ENDE
      return item;
    }
    item++;
  }
  DBG_PROG_ENDE
  return -1;
}

bool
ICCprofile::hasTagName            (std::string name)
{ DBG_PROG_START
  if (!tags.size()) { DBG_PROG_ENDE
    DBG_PROG_ENDE
    return false;
  } DBG_PROG

  int item = 0;
  for (std::vector<ICCtag>::iterator it = tags.begin(); it != tags.end(); it++){
    if ( (*it).getTagName() == name
      && (*it).getSize()            ) {
      #ifdef DEBUG_ICCPROFILE
      DBG_S( (*it).getTagName() << " gefunden" )
      #endif
      DBG_PROG_ENDE
      return true;
    }
    item++;
  }
  DBG_PROG_ENDE
  return false;
}

int
ICCprofile::hasCLUT(void)
{ DBG_PROG_START
  int has_LUTS = false;

       if(this->hasTagName("A2B0"))
    has_LUTS = true;
  else if(this->hasTagName("A2B1"))
    has_LUTS = true;
  else if(this->hasTagName("A2B2"))
    has_LUTS = true;
  else if(this->hasTagName("B2A0"))
    has_LUTS = true;
  else if(this->hasTagName("B2A1"))
    has_LUTS = true;
  else if(this->hasTagName("B2A2"))
    has_LUTS = true;
 
  DBG_PROG_ENDE
  return has_LUTS;
}

int
ICCprofile::getColourChannelsCount(void)
{ DBG_PROG_START
  int channels = 0;
       if(this->hasTagName("A2B0"))
    channels = (int)tags[getTagByName("A2B0")].getNumbers(ICCtag::TABLE_IN)[0];
  else if(this->hasTagName("A2B1"))
    channels = (int)tags[getTagByName("A2B1")].getNumbers(ICCtag::TABLE_IN)[0];
  else if(this->hasTagName("A2B2"))
    channels = (int)tags[getTagByName("A2B2")].getNumbers(ICCtag::TABLE_IN)[0];
  else if(this->hasTagName("B2A0"))
    channels = (int)tags[getTagByName("B2A0")].getNumbers(ICCtag::TABLE_OUT)[0];
  else if(this->hasTagName("B2A1"))
    channels = (int)tags[getTagByName("B2A1")].getNumbers(ICCtag::TABLE_OUT)[0];
  else if(this->hasTagName("B2A2"))
    channels = (int)tags[getTagByName("B2A2")].getNumbers(ICCtag::TABLE_OUT)[0];
  else if(this->hasTagName("kTRC"))
    channels = 1;
  else if(this->hasTagName("rTRC") &&
          this->hasTagName("gTRC") &&
          this->hasTagName("bTRC"))
    channels = 3;
 
  DBG_PROG_ENDE
  return channels;
}

std::vector<double>
ICCprofile::getWhitePkt           (void)
{ DBG_PROG_START
  std::vector<double> XYZ;
  if (hasTagName ("wtpt"))
    XYZ = getTagCIEXYZ (getTagByName ("wtpt"));
 
  DBG_PROG_ENDE
  return XYZ;
}

void
ICCprofile::saveProfileToFile  (char* filename)
{ DBG_PROG_START
  if (_data && _size) free(_data);//delete []_data;
  _size = sizeof (icHeader) + sizeof (icUInt32Number); DBG_MEM_V(_size <<" "<<sizeof (icProfile))
  _data = (char*)calloc (sizeof (char) , _size); //new char (sizeof(icHeader) );
  writeTagTable ();
  writeTags ();
  header.size(_size); DBG_V (_size )
  writeHeader ();

  std::ofstream f ( filename,  std::ios::out );
  f.write ( (char*)_data, _size );
  f.close();
  DBG_PROG_ENDE
}

int
ICCprofile::getProfileSize  ()
{ DBG_PROG_START
  if (_data && _size) free(_data);//delete []_data;
  _size = sizeof (icHeader) + sizeof (icUInt32Number); DBG_V(_size <<" "<<sizeof (icProfile))
  _data = (char*)calloc (sizeof (char) , _size); //new char (sizeof(icHeader) );
  writeTagTable ();
  writeTags ();
  header.size(_size); DBG_V (_size )
  writeHeader ();

  DBG_MEM_V( _size )
  DBG_PROG_ENDE
  return _size;
}

char*
ICCprofile::saveProfileToMem  (int *size)
{ DBG_PROG_START
  if (_data && _size) free(_data);//delete []_data;
  _size = sizeof (icHeader) + sizeof (icUInt32Number); DBG_V(_size <<" "<<sizeof (icProfile))
  _data = (char*)calloc (sizeof (char) , _size); //new char (sizeof(icHeader) );
  writeTagTable ();
  writeTags ();
  header.size(_size); DBG_V (_size )
  writeHeader ();

  char *block = (char*)calloc (sizeof (char) , _size);
  memcpy (block, _data, _size);
  *size = _size;
  DBG_MEM_V( _size )
  DBG_PROG_ENDE
  return block;
}

void
ICCprofile::writeTags (void)
{ DBG_PROG_START
  unsigned int i;
  for (i = 0; i < tags.size(); i++) {
    DBG_NUM_S ( i << ": " << tags[i].getTypName() )
    int size;
    const char* data = tags[i].write(&size);
    icTagList* list = (icTagList*)&((char*)_data)[128];
    
    list->tags[i].sig = icValue((icTagSignature)tags[i].getSignature());
    list->tags[i].offset = icValue((icUInt32Number)_size); DBG_MEM_V (icValue(list->tags[i].offset))
    list->tags[i].size = icValue((icUInt32Number)size); DBG_MEM_V(_size)
    char* temp = (char*) calloc (sizeof(char), _size + size + 
                                               (size%4 ? 4 - size%4 : 0));
    memcpy (temp, _data, _size); DBG_V( _size<<" "<< size<<" "<<size%4 )
    memcpy (&temp[_size], data, size);
    _size = _size + size + (size%4 ? 4 - size%4 : 0);
    free(_data);//delete [] _data;
    _data = temp;
    list = (icTagList*)&temp[128]; DBG_V (icValue(list->tags[i].offset))
    DBG_MEM_V (icValue(list->tags[i].sig))
    DBG_MEM_V (icValue(list->tags[i].size) << " " << (int)&_data[0])
  }
  DBG_MEM_V( _size )
  DBG_PROG_ENDE
}

void
ICCprofile::writeTagTable (void)
{ DBG_PROG_START
  icProfile* p = (icProfile*) _data;
  p->count = icValue((icUInt32Number)tags.size());
  int size = sizeof (icTag) * tags.size();
  char* data = (char*) calloc (sizeof(char), size + _size);
  memcpy (data, _data, _size);
  _size = _size + size;
  free(_data);//delete [] _data;
  _data = data;
  DBG_PROG_ENDE
}

void
ICCprofile::writeHeader (void)
{ DBG_PROG_START
  memcpy (_data, header.header_raw(), 128);
  DBG_PROG_ENDE
}



int
ICCprofile::checkProfileDevice (char* type, icProfileClassSignature deviceClass)
{ DBG_PROG_START
  int check = true;

  if ((strcmp(type, _("Work Space"))) == 0) {
      // test for device class
      switch (deviceClass)
        {
        case icSigInputClass:
        case icSigDisplayClass:
        case icSigOutputClass:
        case icSigLinkClass:
        case icSigAbstractClass:
        case icSigColorSpaceClass:
        case icSigNamedColorClass:
          // should be good
          break;
        default:
          g_message ("%s \"%s %s: %s",_("Your"), type,
                     _("Profile\" is designed for an other device"),
                     getDeviceClassName(deviceClass).c_str());
          check = false;
          break;
        }
  } else if ((strcmp(type, _("Separation"))) == 0) {
      switch (deviceClass)
        {
//        case icSigInputClass:
//        case icSigDisplayClass:
        case icSigOutputClass:
        case icSigLinkClass:
        case icSigAbstractClass:
        case icSigColorSpaceClass:
        case icSigNamedColorClass:
          // should be good
          break;
        default:
          g_message ("%s - %s - %s \"%s %s",_("Device class"),
                     getDeviceClassName(deviceClass).c_str(),
                     _("is not valid for an"),
                     type,
                     _("Profile\"."));
          check = false;
          break;
        if (icSigCmykData   != header.colorSpace())
          check = false;
        }
  } else if ((strcmp(type, _("Linearisation"))) == 0) {
      switch (deviceClass)
        {
//        case icSigInputClass:
//        case icSigDisplayClass:
//        case icSigOutputClass:
        case icSigLinkClass:
//        case icSigAbstractClass:
//        case icSigColorSpaceClass:
//        case icSigNamedColorClass:
          // should be good
          break;
        default:
          g_message ("%s - %s - %s \"%s %s",_("Device class"),
                     getDeviceClassName(deviceClass).c_str(),
                     _("is not valid for an"),
                     type,
                     _("Profile\"."));
          check = false;
          break;
        if (icSigCmykData   != header.colorSpace())
          g_message ("%s - %s - %s \"%s %s",_("Color space"),
                     getColorSpaceName(header.colorSpace()).c_str(),
                     _("is not valid for an"),
                     type,
                     _("Profile at the moment\"."));
          check = false;
        }
  }
  DBG_PROG_ENDE
  return check;
}

void
ICCprofile::saveMemToFile (char* filename, char *block, int size)
{ DBG_PROG_START
  FILE *fp=NULL;
  int   pt = 0;

  if ((fp=fopen(filename, "w")) != NULL) {
    do {
      fputc ( block[pt++] , fp);
    } while (--size);
  }

  fclose (fp);
  DBG_PROG_ENDE
}

void
ICCprofile::removeTag (int item)
{ DBG_PROG_START
  if (item >= (int)tags.size() ) { DBG_PROG_ENDE return; }

  std::vector <ICCtag> t(tags.size()-1); DBG_PROG
  DBG_PROG_V (tags.size())
  int i = 0,
      zahl = 0; DBG_PROG
  for (; i < (int)tags.size(); i++)
    if (i != item) { DBG_PROG
      t[zahl] = tags[i]; DBG_PROG_S("i: " << i << " -> zahl: " << zahl)
      zahl++; DBG_PROG
    }

  DBG_PROG
  #if 0
  tags.resize(t.size());
  for (int i = 0; i < (int)t.size(); i++)
    tags[i].copy (t[i]);
  #else
  tags = t;
  #endif
  DBG_MEM_V( i << " " << tags.size())
  DBG_PROG_ENDE
}

/**
  *  allgemeine Funktionen
  */

void
lcms_error (int ErrorCode, const char* ErrorText)
{ DBG_PROG_START
   g_message ("LCMS error:%d %s", ErrorCode, ErrorText);
  DBG_PROG_ENDE
}

const char* cp_nchar (char* text, int n)
{ DBG_MEM_START
  static char string[1024];

/*  for (int i = 0; i < 1024 ; i++)
    string[i] = '\000';*/

  if (n < 1024)
    snprintf(&string[0], n, text);
  string[1023] = '\000';

  #ifdef DEBUG
  DBG_MEM_V( n << " Buchstaben kopieren " <<  (int)text << " " << string)
  #endif
  DBG_MEM_ENDE
  return string;
}


