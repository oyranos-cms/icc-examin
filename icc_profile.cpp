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

#if 1
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
  data_ = NULL;
  size_ = 0;
  DBG_PROG_ENDE
}

ICCprofile &
ICCprofile::copy_ ( const ICCprofile & p )
{ DBG_PROG_START
  data_ = (char*) calloc (sizeof(char), p.size_);
  size_ = p.size_;
  memcpy(data_, p.data_, size_);
  _filename = p._filename;
  changing_ = p.changing_;
  measurement = p.measurement;
  measurement._profil = this;
  tags = p.tags;
  header = p.header;
  
  DBG_PROG_ENDE
  return *this;
}

ICCprofile::ICCprofile ( const ICCprofile & p )
{ DBG_PROG_START
  copy_(p);
  DBG_PROG_ENDE
}

ICCprofile &
ICCprofile::operator=  ( const ICCprofile & p )
{ DBG_PROG_START
  copy_(p);
  DBG_PROG_ENDE
  return *this;
}

ICCprofile::ICCprofile (const char *filename)
  : _filename (filename)
{ DBG_PROG_START
  if (data_ && size_) free(data_);//delete [] data_;
  data_ = NULL;
  size_ = 0;

  // delegieren
  _filename = filename;
  fload ();
  DBG_PROG_ENDE
}

ICCprofile::~ICCprofile (void)
{ DBG_PROG_START
  this->clear();

  #ifdef DEBUG_PROFILE
  DBG__PROG_S ( "~ICCprofile beendet" )
  #endif
  DBG_PROG_ENDE
}
 
void
ICCprofile::clear (void)
{ DBG_PROG_START
  DBG_PROG_S( "Profil wird geleert" )

  if (data_ && size_) free(data_);
  data_ = NULL;
  size_ = 0;

  _filename = "";
  header.load(NULL);

  tags.clear();
  measurement.clear();

  DBG_NUM_S( "data_, tags und measurement gel�scht" )
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
{
  DBG_PROG_START // ICC Profil laden
 
  std::string file = _filename;
  changing_ = true;

  try {
    data_ = ladeDatei (file, &size_);
    DBG_MEM
  }
    catch (Ausnahme & a) {	// f�ngt alles von Ausnahme Abstammende
        DBG_NUM_V( _("Ausnahme aufgetreten: ") << a.what() );
        a.report();
        _filename = "";
    }
    catch (std::exception & e) { // f�ngt alles von exception Abstammende
        DBG_NUM_V( _("Std-Ausnahme aufgetreten: ") << e.what() );
        _filename = "";
    }
    catch (...) {		// f�ngt alles �briggebliebene
        DBG_NUM_V( _("Huch, unbekannte Ausnahme") );
        _filename = "";
    }

  DBG_MEM_V( (int*)data_ <<" "<< size_ )

  if (data_ && size_) {
    DBG_PROG_S( _("!!!! Profil wird wiederbenutzt !!!! ") )
    clear();
    // zweites mal Laden nach clear() ; k�nnte optimiert werden
    data_ = ladeDatei (file, &size_);
    _filename = file;

    Speicher s ((const char*)data_, size_);
    s = file;
    load(s);

  } else {
    DBG_PROG_ENDE
    return;
  }
#if 0
  // Test   > 132 byte
  if (size_ < 132) {
    WARN_S( _("Kein Profil")<<_(" Gr��e ")<<size_ )
    measurement.load( this, data_, size_ );
    DBG_PROG_ENDE
    return;
  }

  DBG_PROG
  //Kopf
  header.load ((void*)data_); DBG_PROG

  // Test acsp
  char magic[5];
  memcpy( magic, header.magicName(), 4); magic[4] = 0;
  if (strstr(magic, "acsp") == 0)
  {
    WARN_S( _("Kein Profil") )
    header.clear();

    // "targ"  Messdaten als Block hinzuf�gen
    int groesse = 8 + size_ + 1;
    char* tag_block = (char*) calloc (sizeof (char), groesse);
    icTag ic_tag;
    ICCtag tag;

    ic_tag.size = icValue ((icUInt32Number)groesse); DBG_MEM_V( groesse )
    ic_tag.offset = 0;

    if( _filename.size() &&
        (_filename.find( "wrl",  _filename.find_last_of(".") )
         != std::string::npos) )
      memcpy (&ic_tag.sig, "vrml", 4);
    else
      ic_tag.sig = icValue (icSigCharTargetTag);

    memcpy (&tag_block[0], "text", 4); DBG_NUM_S( tag_block )
    memcpy (&tag_block[8], data_, size_);

    tag.load( this, &ic_tag, tag_block );
    addTag( tag );

    free (tag_block);

    measurement.load( this, tag );
    DBG_PROG_ENDE
    return;
  }
   
  //Profilabschnitte
  // TagTabelle bei 132 abholen
  icTag *tagList = (icTag*)&((char*)data_)[132];
  //(icTag*) new char ( getTagCount() * sizeof (icTag));
  //memcpy (tagList , &((char*)data_)[132], sizeof (icTag) * getTagCount());
  DBG_PROG
  tags.resize(getTagCount()); DBG_PROG
  for (int i = 0 ; i < getTagCount() ; i++)
  { DBG_PROG
    tags[i].load( this, &tagList[i] ,
              &((char*)data_)[ icValue(tagList[i].offset) ]); DBG_PROG
    #ifdef DEBUG_ICCPROFILE
    cout << " sig: " << tags[i].getTagName() << " " << i << " "; DBG_PROG
    #endif

    DBG_PROG
    // bekannte Tags mit Messdaten
    if (tags[i].getTagName() == "targ"
     || tags[i].getTagName() == "DevD"
     || tags[i].getTagName() == "CIED") {
      DBG_PROG
      #ifdef DEBUG_ICCPROFILE
      DBG_NUM_S( "Messdaten gefunden " << tags[i].getTagName() )
      #endif
      measurement.load( this, tags[i] );
      DBG_PROG
    }
    DBG_PROG
  }
  DBG_PROG
  #ifdef DEBUG_ICCPROFILE
  DBG_NUM_S( "TagCount: " << getTagCount() << " / " << tags.size() )
  #endif
 
  DBG_NUM_V( _filename )
#endif
  changing_ = false;
  DBG_PROG_ENDE
}

void
ICCprofile::load (const Speicher & prof)
{
  DBG_PROG_START // ICC Profil laden
  std::string file = prof.name();
  changing_ = true;

  DBG_MEM_V( (int*)data_ <<" "<< size_ )

  if (prof.size()) {
    //WARN_S( _("!!!! Profil wird wiederbenutzt !!!! ") )
    clear();
    size_ = prof.size();
    data_ = (char*)calloc (sizeof (char), size_+1);
    const char* z = prof;
    memcpy(data_, z, size_);
    _filename = file;
    DBG_MEM_V( _filename )
    DBG_MEM_V( size_ )
    DBG_MEM_V( (int*)data_ )
  } else {
    DBG_PROG_ENDE
    return;
  }

  // Test   > 132 byte
  if (size_ < 132) {
    WARN_S( _("Kein Profil")<<_(" Gr��e ")<<size_ )
    measurement.load( this, data_, size_ );
    DBG_PROG_ENDE
    return;
  }

  DBG_PROG
  //Kopf
  header.load ((void*)data_); DBG_PROG

  // Test acsp
  char magic[5];
  memcpy( magic, header.magicName(), 4); magic[4] = 0;
  if (strstr(magic, "acsp") == 0)
  {
    WARN_S( _("Kein Profil") )
    header.clear();

    // "targ"  Messdaten als Block hinzuf�gen
    int groesse = 8 + size_ + 1;
    char* tag_block = (char*) calloc (sizeof (char), groesse);
    icTag ic_tag;
    ICCtag tag;

    ic_tag.size = icValue ((icUInt32Number)groesse); DBG_MEM_V( groesse )
    ic_tag.offset = 0;

    if( _filename.size() &&
        (_filename.find( "wrl",  _filename.find_last_of(".") )
         != std::string::npos) )
      memcpy (&ic_tag.sig, "vrml", 4);
    else
      ic_tag.sig = icValue (icSigCharTargetTag);

    memcpy (&tag_block[0], "text", 4); DBG_NUM_S( tag_block )
    memcpy (&tag_block[8], data_, size_);

    tag.load( this, &ic_tag, tag_block );
    addTag( tag );

    free (tag_block);

    measurement.load( this, tag );
    DBG_PROG_ENDE
    return;
  }
   
  //Profilabschnitte
  // TagTabelle bei 132 abholen
  icTag *tagList = (icTag*)&((char*)data_)[132];
  //(icTag*) new char ( getTagCount() * sizeof (icTag));
  //memcpy (tagList , &((char*)data_)[132], sizeof (icTag) * getTagCount());
  DBG_PROG
  tags.resize(getTagCount()); DBG_PROG
  for (int i = 0 ; i < getTagCount() ; i++)
  { DBG_PROG
    tags[i].load( this, &tagList[i] ,
              &((char*)data_)[ icValue(tagList[i].offset) ]); DBG_PROG
    #ifdef DEBUG_ICCPROFILE
    DBG_PROG_S( " sig: " << tags[i].getTagName() << " " << i )
    #endif

    DBG_PROG
    // bekannte Tags mit Messdaten
    if (tags[i].getTagName() == "targ"
     || tags[i].getTagName() == "DevD"
     || tags[i].getTagName() == "CIED") {
      DBG_PROG
      #ifdef DEBUG_ICCPROFILE
      DBG_NUM_S( "Messdaten gefunden " << tags[i].getTagName() )
      #endif
      measurement.load( this, tags[i] );
      DBG_PROG
    }
    DBG_PROG
  }
  DBG_PROG
  #ifdef DEBUG_ICCPROFILE
  DBG_NUM_S( "TagCount: " << getTagCount() << " / " << tags.size() )
  #endif
 
  DBG_NUM_V( _filename )

  changing_ = false;
  DBG_PROG_ENDE
}


const char*
ICCprofile::filename ()
{
  DBG_PROG
  return _filename.c_str();
}

void
ICCprofile::filename (const char* s)
{
  DBG_PROG _filename = s;
}

int
ICCprofile::size     ()
{
  DBG_PROG
  return header.size();
}
    //const char*         cmm      ()        {DBG_PROG return header.cmmName(); }
    //void                cmm      (const char* s) {DBG_PROG header.cmmName (s); }
    //int                 version  ()        {DBG_PROG return (int) header.version(); }
    //const char*         creator  ()        {DBG_PROG return header.creatorName(); }

std::string
ICCprofile::printHeader     ()
{
  DBG_PROG
  return header.print();
}

std::string
ICCprofile::printLongHeader ()
{
  DBG_PROG
  return header.print_long();
}

std::vector<std::string>
ICCprofile::getPCSNames     ()
{
  DBG_PROG
  return getChannelNames(header.pcs());
}

int
ICCprofile::getTagCount     ()
{
  DBG_PROG
  return icValue(((icProfile*)data_)-> count);
}

int
ICCprofile::tagCount        ()
{
  DBG_PROG
  return tags.size();
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
    DBG_NUM_S( (*it).getTagName() << " " << count )
  #endif
  }
  DBG_PROG_ENDE
  return StringList;
}

std::vector<std::string>
ICCprofile::getTagText                                  (int item)
{ DBG_PROG_START
  // Pr�fen
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
  // Pr�fen
  std::string leer = tags[item].getTypName() + " Typ - keine Textausgabe";
  std::vector<std::string> v;
  v.push_back( leer );

  // Pr�fen
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
  // Pr�fen
  std::vector<std::string> leer;
  if (tags[item].getTypName() != "desc") { DBG_PROG_ENDE return leer; }

  DBG_PROG_ENDE
  return tags.at(item).getDescription();
}

std::vector<double>
ICCprofile::getTagCIEXYZ                         (int item)
{ DBG_PROG_START
  // Pr�fen
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
  // Pr�fen
  std::vector<double> leer;
  if (tags[item].getTypName() != "curv")
  {
    #ifdef DEBUG_ICCPROFILE
    DBG_NUM_S( tags[item].getTypName() )
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
  // Pr�fen
  std::vector<std::vector<double> > leer;
  if (tags[item].getTypName() != "mft2"
   && tags[item].getTypName() != "mft1"
   && tags[item].getTypName() != "vcgt")
  {
    #ifdef DEBUG_ICCPROFILE
    DBG_NUM_S( "gibt nix f�r " << tags[item].getTypName() )
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
  // Pr�fen
  std::vector<std::vector<std::vector<std::vector<double> > > > leer;
  if (tags[item].getTypName() != "mft2"
   && tags[item].getTypName() != "mft1")
  {
    #ifdef DEBUG_ICCPROFILE
    DBG_NUM_S( "gibt nix f�r " << tags[item].getTypName() )
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
  // Pr�fen
  std::vector<double> leer;
  if (tags[item].getTypName() != "mft2"
   && tags[item].getTypName() != "mft1"
   && tags[item].getTypName() != "ncl2")
  {
    #ifdef DEBUG_ICCPROFILE
    DBG_NUM_S( tags[item].getTypName() )
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
      DBG_PROG_S( item << " = " << (*it).getTagName() << " gefunden" )
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
  if (!tags.size()) {
    DBG_PROG_ENDE
    return false;
  } DBG_PROG

  int item = 0;
  for (std::vector<ICCtag>::iterator it = tags.begin(); it != tags.end(); it++){
    if ( (*it).getTagName() == name
      && (*it).getSize()            ) {
      #ifdef DEBUG_ICCPROFILE
      DBG_NUM_S( (*it).getTagName() << " gefunden" )
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

bool
ICCprofile::valid ()
{
  const char* magic = header.magicName();
  if (magic[0] == 'a' &&
      magic[1] == 'c' &&
      magic[2] == 's' &&
      magic[3] == 'p')
    return true;
  else
    return false;
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
  if (data_ && size_) free(data_);//delete []data_;
  size_ = sizeof (icHeader) + sizeof (icUInt32Number); DBG_MEM_V(size_ <<" "<<sizeof (icProfile))
  data_ = (char*)calloc (sizeof (char) , size_); //new char (sizeof(icHeader) );
  writeTagTable ();
  writeTags ();
  header.size(size_); DBG_MEM_V (size_ )
  writeHeader ();

  std::ofstream f ( filename,  std::ios::out );
  f.write ( (char*)data_, size_ );
  f.close();
  DBG_PROG_ENDE
}

int
ICCprofile::getProfileSize  ()
{ DBG_PROG_START
  if (data_ && size_) free(data_);//delete []data_;
  size_ = sizeof (icHeader) + sizeof (icUInt32Number); DBG_MEM_V(size_ <<" "<<sizeof (icProfile))
  data_ = (char*)calloc (sizeof (char) , size_); //new char (sizeof(icHeader) );
  writeTagTable ();
  writeTags ();
  header.size(size_); DBG_PROG_V (size_ )
  writeHeader ();

  DBG_MEM_V( size_ )
  DBG_PROG_ENDE
  return size_;
}

char*
ICCprofile::saveProfileToMem  (int *size)
{ DBG_PROG_START
  if (data_ && size_) free(data_);//delete []data_;
  size_ = sizeof (icHeader) + sizeof (icUInt32Number); DBG_PROG_V(size_ <<" "<<sizeof (icProfile))
  data_ = (char*)calloc (sizeof (char) , size_); //new char (sizeof(icHeader) );
  writeTagTable ();
  writeTags ();
  header.size(size_); DBG_PROG_V (size_ )
  writeHeader ();

  char *block = (char*)calloc (sizeof (char) , size_);
  memcpy (block, data_, size_);
  if(size)
    *size = size_;
  DBG_MEM_V( size_ )
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
    icTagList* list = (icTagList*)&((char*)data_)[128];
    
    list->tags[i].sig = icValue((icTagSignature)tags[i].getSignature());
    list->tags[i].offset = icValue((icUInt32Number)size_); DBG_MEM_V (icValue(list->tags[i].offset))
    list->tags[i].size = icValue((icUInt32Number)size); DBG_MEM_V(size_)
    char* temp = (char*) calloc (sizeof(char), size_ + size + 
                                               (size%4 ? 4 - size%4 : 0));
    memcpy (temp, data_, size_); DBG_MEM_V( size_<<" "<< size<<" "<<size%4 )
    memcpy (&temp[size_], data, size);
    size_ = size_ + size + (size%4 ? 4 - size%4 : 0);
    free(data_);//delete [] data_;
    data_ = temp;
    list = (icTagList*)&temp[128];
    DBG_MEM_V (icValue(list->tags[i].offset))
    DBG_MEM_V (icValue(list->tags[i].sig))
    DBG_MEM_V (icValue(list->tags[i].size) << " " << (int)&data_[0])
  }
  DBG_MEM_V( size_ )
  DBG_PROG_ENDE
}

void
ICCprofile::writeTagTable (void)
{ DBG_PROG_START
  icProfile* p = (icProfile*) data_;
  p->count = icValue((icUInt32Number)tags.size());
  int size = sizeof (icTag) * tags.size();
  char* data = (char*) calloc (sizeof(char), size + size_);
  memcpy (data, data_, size_);
  size_ = size_ + size;
  free(data_);//delete [] data_;
  data_ = data;
  DBG_PROG_ENDE
}

void
ICCprofile::writeHeader (void)
{ DBG_PROG_START
  memcpy (data_, header.header_raw(), 128);
  DBG_PROG_ENDE
}



int
ICCprofile::checkProfileDevice (char* type, icProfileClassSignature deviceClass)
{
  DBG_PROG_START
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

bool
ICCprofile::hasMeasurement()
{
  DBG_PROG
  return (hasTagName("targ") || (hasTagName("CIED")&&hasTagName("DevD")));
}

std::string
ICCprofile::report (bool auss)
{
  DBG_PROG
  return measurement.getHtmlReport(auss);
}

ICCmeasurement &
ICCprofile::getMeasurement ()
{
  DBG_PROG
  if (hasMeasurement())
    measurement.init();
  return measurement;
}
std::string
ICCprofile::cgats()
{
  DBG_PROG
  return measurement.getCGATS();
}

std::string
ICCprofile::cgats_max()
{
  DBG_PROG
  return measurement.getMaxCGATS();
}

void
ICCprofile::setHeader (void* h)
{
  DBG_PROG
  header.header_raw(h);
}

void
ICCprofile::addTag (ICCtag tag)
{
  DBG_PROG tags.push_back(tag);
}

ICCtag &
ICCprofile::getTag (int item)
{
  return tags[item];
}

void
ICCprofile::removeTagByName (std::string name)
{
  DBG_PROG
  if (hasTagName(name))
    removeTag (getTagByName(name));
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
    #if 0
    memcpy (string, text, n);
    #else
    snprintf(&string[0], n, text);
    #endif
  string[1023] = '\000';

  #ifdef DEBUG
  DBG_MEM_V( n << " Buchstaben kopieren " <<  (int)text << " " << string)
  #endif
  DBG_MEM_ENDE
  return string;
}


