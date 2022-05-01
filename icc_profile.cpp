/*
 * ICC Examin ist eine ICC Profil Betrachter
 * 
 * Copyright (C) 2004-2008  Kai-Uwe Behrmann 
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
 * profile interpretation
 * 
 */

// Date:      04. 05. 2004

#if 0
# ifndef DEBUG
#  define DEBUG
# endif
# define DEBUG_ICCPROFILE
#endif

#include "icc_profile.h"
#include "icc_utils.h"

#include <cmath>

#if (!defined(WIN32) || (defined(WIN32) && defined(__MINGW32__)))
# define g_message printf
#else
# define g_message(text) 
#endif

/**
  *  @brief ICCprofile functions
  */

ICCprofile::ICCprofile (void)
 : measurement(this)
{ DBG_PROG_START
  data_ = NULL;
  size_ = 0;
  profile_ = NULL;
  DBG_PROG_ENDE
}

ICCprofile::ICCprofile ( const ICCprofile & p )
 : measurement(this)
{ DBG_PROG_START
  copy_(p);
  DBG_PROG_ENDE
}

ICCprofile::ICCprofile (const Speicher & s)
 : measurement(this)
{ DBG_PROG_START
  if (data_ && size_) free(data_);
  oyProfile_Release( &profile_ );
  data_ = NULL;
  size_ = 0;

  // delegate
  filename_ = s.name();
  load(s);
  DBG_PROG_ENDE
}

ICCprofile::~ICCprofile (void)
{ DBG_PROG_START
  this->clear();

# ifdef DEBUG_PROFILE
  DBG__PROG_S ( "~ICCprofile ended" )
# endif
  DBG_PROG_ENDE
}

ICCprofile &
ICCprofile::operator=  ( const ICCprofile & p )
{ DBG_PROG_START
  copy_(p);
  DBG_PROG_ENDE
  return *this;
}

ICCprofile &
ICCprofile::copy_ ( const ICCprofile & p )
{ DBG_PROG_START
  data_ = (char*) calloc (sizeof(char), p.size_);
  size_ = p.size_;
  memcpy(data_, p.data_, size_);
  profile_ = oyProfile_Copy( p.profile_, 0 );
  filename_ = p.filename_;
  changing_ = p.changing_;
  measurement = p.measurement;
  measurement.profile_ = this;
  tags = p.tags;
  header = p.header;
  data_type = p.data_type;
  
  DBG_PROG_ENDE
  return *this;
}

void
ICCprofile::clear (void)
{ DBG_PROG_START
  DBG_PROG_S( "Profil will be cleared" )

  if (data_ && size_) free(data_);
  data_ = NULL;
  size_ = 0;
  oyProfile_Release( &profile_ );

  filename_ = "";
  header.load(NULL);

  tags.clear();
  measurement.clear();

  DBG_NUM_S( "data_, tags and measurement freed" )
  DBG_PROG_ENDE
}


ICCprofile::ICCDataType
ICCprofile::load (const Speicher & prof)
{
  DBG_PROG_START // ICC Profil load
  std::string file = prof.name();
  changing_ = true;
  Speicher * prof_ = NULL;

  DBG_MEM_V( (int*)data_ <<" "<< size_ )

  this->clear();
  measurementReparent();

  if( file.size() )
    data_type = guessFileType( file.c_str() );

  if(data_type == ICCimageDATA)
  {
    oyImage_s * image = NULL;
    oyImage_FromFile(file.c_str(), icc_oyranos.icc_profile_flags, &image, NULL);
    oyProfile_s * p = oyImage_GetProfile( image );
    size_t size = 0;
    char * data = (char*)oyProfile_GetMem( p, &size, 0, malloc);
    prof_ = new Speicher();
    prof_->ladeUndFreePtr( &data, size );
  }

  // check minimum size for plausible data
  size_t prof_size = prof.size();
  if(prof_)
    prof_size = prof_->size();
  if (prof_size > 64) {
    //WARN_S( _("!!!! Profil wird wiederbenutzt !!!! ") )
    size_ = prof_size;
    data_ = (char*)calloc (sizeof (char), size_+1);
    const char* z = prof;
    if(prof_)
      z = (*prof_);
    memcpy(data_, z, size_);
    filename_ = file;
    if(data_type != ICCimageDATA)
      profile_ = oyProfile_FromFile( filename_.c_str(), icc_oyranos.oy_profile_from_flags, 0 );
    if(!profile_)
      profile_ = oyProfile_FromMem( size_, data_, 0, 0 );
    DBG_MEM_V( filename_ )
    DBG_MEM_V( size_ )
    DBG_MEM_V( (int*)data_ <<" "<< (int*)z )
  } else {
    DBG_PROG_ENDE
    changing_ = false;
    data_type = ICCnullDATA;
    return ICCnullDATA;
  }

  // test   > 132 byte, otherwise interprete as measurement
  if (size_ < 132) {
    WARN_S( _("no profile") <<" "<<_("Size")<<" "<<size_ )
    measurement.load( this, data_, size_ );
    DBG_PROG_ENDE
    changing_ = false;
    data_type = ICCmeasurementDATA;
    return ICCmeasurementDATA;
  }

  DBG_PROG
  // head
  header.load ((void*)data_); DBG_PROG

  // test acsp
  char magic[5];
  memcpy( magic, header.magicName(), 4); magic[4] = 0;
  DBG_MEM
  if (strstr(magic, "acsp") == 0)
  {
    WARN_S( _("no profile") )
    header.clear();

#   if 0
    DBG_PROG_ENDE
    changing_ = false;
    return ICCnullDATA;
#   endif

    // "targ"  add measurement as block
    int groesse = 8 + size_ + 1;
    char* tag_block = (char*) calloc (sizeof (char), groesse);
    icTag ic_tag;
    ICCtag tag;

    ic_tag.size = icValue ((icUInt32Number)groesse); DBG_MEM_V( groesse )
    ic_tag.offset = 0;

    if(data_type == ICCprofileDATA)
      data_type = ICCmeasurementDATA;

    if( data_type == ICCvrmlDATA )
      memcpy (&ic_tag.sig, "vrml", 4);
    else
      ic_tag.sig = icValue (icSigCharTargetTag);

    memcpy (&tag_block[0], "text", 4); DBG_NUM_S( tag_block )
    memcpy (&tag_block[8], data_, size_);

    tag.load( this, &ic_tag, tag_block );
    addTag( tag );

    free (tag_block);

    if(data_type == ICCmeasurementDATA)
      measurement.load( this, tag );

    DBG_PROG_ENDE
    changing_ = false;
    return data_type;
  }
   
  DBG_MEM
  // profile parts
  // pick TagTabelle at 132
  icTag *tagList = (icTag*)&((char*)data_)[132];

# if BYTE_ORDER == LITTLE_ENDIAN
    DBG_PROG_S("LITTLE_ENDIAN")
# else
    DBG_PROG_S("BIG_ENDIAN")
# endif
# ifdef _BIG_ENDIAN
    DBG_PROG_S( "BIG_ENDIAN" )
# endif
# ifdef _LITTLE_ENDIAN
    DBG_PROG_S( "LITTLE_ENDIAN" )
# endif
  int tag_count = getTagCount();
  DBG_MEM_V( tag_count <<" "<< tags.size() )

  if(tag_count || tags.size())
    tags.resize(tag_count);

  if(tag_count != (int)tags.size())
    WARN_S(_("wrong tag size in profile ") << tag_count <<" "<< tags.size() <<" :"<< file);

  // our guess
  data_type = ICCprofileDATA;

  DBG_MEM
  for (int i = 0 ; i < tag_count; i++)
  {
    DBG_MEM
    icTag *ic_tag = &tagList[i];
    int offset = icValue(tagList[i].offset);
    int tag_size = icValue(tagList[i].size);
    char *tag_block = &((char*)data_)[offset];

    if((offset+tag_size) > ((int)size_)) {
      WARN_S(_("tag list is corrupted for tag")<<" "<<i<<" - "<< _("not inside memory block") );

      tag_block = NULL;
      data_type = ICCcorruptedprofileDATA;
    }
    tags[i].load( this, ic_tag, tag_block );

#   ifdef DEBUG_ICCPROFILE
    DBG_PROG_S( " sig: " << tags[i].getTagName() << " " << i )
#   endif

    // known tags with measurements
    if (tagBelongsToMeasurement(i)
     && data_type == ICCprofileDATA) {
#     ifdef DEBUG_ICCPROFILE
      DBG_NUM_S( "measurements found " << tags[i].getTagName() )
#     endif
      measurement.load( this, tags[i] );
    }
  }
# ifdef DEBUG_ICCPROFILE
  DBG_NUM_S( "TagCount: " << tag_count << " / " << tags.size() )
# endif
 
  DBG_NUM_V( filename_ )

  if(prof_)
    delete prof_;

  changing_ = false;
  DBG_PROG_ENDE
  return data_type;
}


const char*
ICCprofile::filename ()
{
  DBG_PROG
  return filename_.c_str();
}

void
ICCprofile::filename (const char* s)
{
  DBG_PROG filename_ = s;
}

size_t
ICCprofile::size     ()
{
  DBG_PROG
  size_t size = header.size();

  // needed to load non profile vrml's
  if(!size && data_ && size_)
    size = size_;

  return size;
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

ICClist<std::string>
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

ICClist<std::string>
ICCprofile::printTagInfo         (int item)
{ DBG_PROG_START
  ICClist<std::string> liste;

  if(0 > item || item >= (int)tags.size())
    return liste;

  liste.push_back( tags.at(item).getTagName() );
  liste.push_back( tags.at(item).getTypName() );

  DBG_PROG_ENDE
  return liste;
}

ICClist<std::string>
ICCprofile::printTags            ()
{ DBG_PROG_START
  ICClist<std::string> StringList;
  std::string text;
  std::stringstream s;

  int size = tags.size();

  if (!size) { DBG_PROG_ENDE
    return StringList;
  } DBG_MEM

  int count = 0;
  for (size_t i = 0; i < tags.size(); i++)
  {
    s.str("");
    s << count; count++;           StringList.push_back(s.str()); s.str("");
    s.str(tags[i].getTagName());     StringList.push_back(s.str()); s.str("");
    s.str(tags[i].getTypName());     StringList.push_back(s.str()); s.str("");
    s << tags[i].getSize();          StringList.push_back(s.str()); s.str("");
    s.str(tags[i].getInfo()); StringList.push_back(s.str()); s.str("");
# ifdef DEBUG_ICCPROFILE
    DBG_NUM_S( tags[i].getTagName() << " " << count )
# endif
  }
  DBG_PROG_ENDE
  return StringList;
}

ICClist<std::string>
ICCprofile::getTagText                                  (int item)
{ DBG_PROG_START
  // check
  std::string name = tags[item].getTypName();
  std::string leer = name + _(" typ - no text output");
  ICClist<std::string> v;
  v.push_back( leer );
  if(item < 0 || item >= (int)tags.size())
  {
    WARN_S("tag not available " << item)
    return v;
  }
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

ICClist<std::string>
ICCprofile::getTagChannelNames                          (int item,
                                                         ICCtag::MftChain typ)
{ DBG_PROG_START
  // check
  std::string leer = tags[item].getTypName() + _(" typ - no text output");
  ICClist<std::string> v;
  v.push_back( leer );

  // check
  if (tags[item].getTypName() != "mft2"
   && tags[item].getTypName() != "mft1"
   && tags[item].getTypName() != "mAB "
   && tags[item].getTypName() != "mBA "
   && tags[item].getTypName() != "mpet")
  { DBG_PROG_ENDE
    return v;
  }

  DBG_PROG_ENDE
  return tags.at(item).getText(typ);
}

ICClist<std::string>
ICCprofile::getTagDescription                    (int item)
{ DBG_PROG_START
  // check
  ICClist<std::string> leer;
  if (tags[item].getTypName() != "desc") { DBG_PROG_ENDE return leer; }

  DBG_PROG_ENDE
  return tags.at(item).getText();
}

ICClist<double>
ICCprofile::getTagCIEXYZ                         (int item)
{ DBG_PROG_START
  // check
  ICClist<double> XYZ;

  if ( tags[item].getTypName() == "XYZ"
    || tags[item].getTypName() == "chrm")
    XYZ = tags.at(item).getCIEXYZ();

  DBG_PROG_ENDE
  return XYZ;
}

ICClist<double>
ICCprofile::getTagCurve                          (int item)
{ DBG_PROG_START
  // check
  ICClist<double> leer;
  if (tags[item].getTypName() != "curv")
  {
#   ifdef DEBUG_ICCPROFILE
    DBG_NUM_S( tags[item].getTypName() )
#   endif
    DBG_PROG_ENDE
    return leer;
  }

  DBG_PROG_ENDE
  return tags.at(item).getCurve();
}

ICClist<ICClist<double> >
ICCprofile::getTagCurves                         (int item,ICCtag::MftChain typ)
{ DBG_PROG_START
  // check
  ICClist<ICClist<double> > leer;
  if (tags[item].getTypName() != "mft2"
   && tags[item].getTypName() != "mft1"
   && tags[item].getTypName() != "vcgt")
  {
#   ifdef DEBUG_ICCPROFILE
    DBG_NUM_S( "nothing found for " << tags[item].getTypName() )
#   endif
    DBG_PROG_ENDE
    return leer;
  }

  DBG_PROG_ENDE
  return tags.at(item).getCurves(typ);
}

ICClist<ICClist<ICClist<ICClist<double> > > >
ICCprofile::getTagTable              ( int                 item,
                                       ICCtag::MftChain    typ,
                                       ICClist<int>        channels )
{ DBG_PROG_START
  // check
  ICClist<ICClist<ICClist<ICClist<double> > > > leer;
  if (tags[item].getTypName() != "mft2"
   && tags[item].getTypName() != "mft1")
  {
#   ifdef DEBUG_ICCPROFILE
    DBG_NUM_S( "nothing found: " << tags[item].getTypName() )
#   endif
    DBG_PROG_ENDE
    return leer;
  }

  DBG_PROG_ENDE
  return tags.at(item).getTable(typ, channels);
}

ICClist<double>
ICCprofile::getTagNumbers                        (int item,ICCtag::MftChain typ)
{ DBG_PROG_START
  // check
  ICClist<double> leer;
  if (tags[item].getTypName() != "mft2"
   && tags[item].getTypName() != "mft1"
   && tags[item].getTypName() != "ncl2")
  {
#   ifdef DEBUG_ICCPROFILE
    DBG_NUM_S( tags[item].getTypName() )
#   endif
    DBG_PROG_ENDE
    return leer;
  }
  DBG_PROG_ENDE
  return tags.at(item).getNumbers(typ);
}

oyStructList_s * ICCprofile::getTagNumbers       (int item)
{
  return tags.at(item).getNumbers();
}

int
ICCprofile::getTagIDByName            (std::string name)
{ DBG_PROG_START
  if (!tags.size()) { DBG_PROG_ENDE 
    return -1;
  } DBG_MEM

  int item = 0;
  for (size_t i = 0; i < tags.size(); i++)
  {
    if ( tags[i].getTagName() == name
      && tags[i].getSize()            ) {
#     ifdef DEBUG_ICCPROFILE
      DBG_PROG_S( item << " = " << tags[i].getTagName() << " found" )
#     endif
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

  DBG_PROG_V( tags.size() )

  if (!tags.size()) {
    DBG_PROG_ENDE
    return false;
  }

  int item = 0;
  DBG_PROG_V( tags.size() )
  for (size_t i = 0; i < tags.size(); i++)
  {
    DBG_PROG_V((intptr_t)(&tags[i]))
    if ( tags[i].getTagName() == name
      && tags[i].getSize()            ) {
#     ifdef DEBUG_ICCPROFILE
      DBG_NUM_S( tags[i].getTagName() << " found" )
#     endif
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
  {
    return true;
  /*} else if(data_type == ICCvrmlDATA || data_type == ICCmeasurementDATA &&
            data_ && size_) {
    return true;
  */} else
    return false;
}

int
ICCprofile::getColourChannelsCount(void)
{ DBG_PROG_START
  int channels = getColorSpaceChannels(colorSpace());

  DBG_PROG_ENDE
  return channels;
}

ICClist<double>
ICCprofile::getWhitePkt           (void)
{ DBG_PROG_START
  ICClist<double> XYZ;
  if (hasTagName ("wtpt"))
    XYZ = getTagCIEXYZ (getTagIDByName ("wtpt"));
 
  DBG_PROG_ENDE
  return XYZ;
}

char*
ICCprofile::getProfileDescription  ()
{
  char *text = NULL;
  DBG_PROG_START

  if(hasTagName("desc"))
  {
    int id = getTagIDByName("desc");
    ICClist<std::string> texte = getTagText( id );
    if(texte.size())
    {
      text = (char*) calloc( sizeof(char), strlen(texte[0].c_str()) * 4 );
      sprintf( text, "%s", texte[0].c_str() );
    }
  }

  DBG_PROG_ENDE
  return text;
}


size_t
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
ICCprofile::saveProfileToMem  (size_t *size)
{ DBG_PROG_START
  char *block = 0;

  // vrml part
  /*if(data_type == ICCvrmlDATA || data_type == ICCmeasurementDATA &&
     data_ && size_)
  {
    block = (char*)calloc (sizeof (char) , size_);
    memcpy (block, data_, size_);
    if(size) {
      *size = size_;
    }
    DBG_PROG_ENDE
    return block;
  }*/

  if (data_ && size_) free(data_);//delete []data_;
  size_ = sizeof (icHeader) + sizeof (icUInt32Number); DBG_PROG_V(size_ <<" "<<sizeof (icProfile))
  data_ = (char*)calloc (sizeof (char) , size_); //new char (sizeof(icHeader) );
  writeTagTable ();
  writeTags ();
  header.size(size_); DBG_PROG_V (size_ )
  writeHeader ();

  block = (char*)calloc (sizeof (char) , size_);
  memcpy (block, data_, size_);
  if(size) {
    *size = size_;
    DBG_MEM_V( *size )
  }
  DBG_MEM_V( size_ <<" "<< size )
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
    DBG_MEM_V (icValue(list->tags[i].size) << " " << (intptr_t)&data_[0])
  }
  DBG_MEM_V( size_ )
  DBG_PROG_ENDE
}

void
ICCprofile::writeTagTable (void)
{ DBG_PROG_START
  icProfile* p = (icProfile*) data_;
  p->count = icValue((icUInt32Number)tags.size());
  size_t size = sizeof (icTag) * tags.size();
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
  memcpy (data_, header.headerRaw(), 128);
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
            g_message ("%s - %s - %s \"%s %s",_("Colour Space"),
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
  return (hasTagName("targ") ||
          hasTagName("b015") ||
          hasTagName("TGL2") ||
          (hasTagName("CIED")&&hasTagName("DevD")));
}

bool
ICCprofile::tagBelongsToMeasurement( int tag )
{
  DBG_PROG
  if(0 > tag || tag >= (int)tags.size())
  {
    if(tag >= 0)
      WARN_S("wrong tag "<<tag<<"["<<tags.size()<<"]");
    return 0;
  }

  std::string name = tags[tag].getTagName();
  int s = tags[tag].getSize();
  return ( (name == "targ" ||
            name == "b015" ||
            name == "TGL2" ||
            (name == "CIED"&&hasTagName("DevD")) ||
            (hasTagName("CIED")&&name == "DevD"))
           && s );
}

std::string
ICCprofile::report                   ( bool                auss, 
                                       oyOptions_s       * opts )
{
  DBG_PROG
  measurement.options( opts );
  return measurement.getHtmlReport(auss);
}

ICCmeasurement &
ICCprofile::getMeasurement ()
{
  DBG_PROG
  if (hasMeasurement() && !measurement.validHalf())
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
ICCprofile::setHeader (const void* h)
{
  DBG_PROG
  header.headerRaw(h);
}

void
ICCprofile::addTag (ICCtag & tag)
{
  int schonmal = 0;

  for( int i = 0; i < (int)tags.size(); ++i )
  {
    if( tags[i].getSignature() == tag.getSignature() )
    {
      schonmal = 1;
      tags[i] = tag; 
      break;
    }
  }

  DBG_PROG
  if(!schonmal)
    tags.push_back(tag);
}

ICCtag &
ICCprofile::getTag (int item)
{
  static ICCtag tag;
  if(item >= 0)
    return tags[item];
  else
    return tag;
}

ICCtag &
ICCprofile::getTag (std::string name)
{
  static ICCtag tag;
  if( hasTagName( name ) )
    return getTag( getTagIDByName( name ) );
  else
    return tag;
}

int
ICCprofile::removeTag (std::string name)
{
  int error = 0;
  DBG_PROG
  if (hasTagName(name))
    removeTag (getTagIDByName(name));
  else
    error = 1;

  return error;
}

int
ICCprofile::removeTag (int item)
{
  int error = 0;
  DBG_PROG_START

  if (item >= (int)tags.size() ) { DBG_PROG_ENDE return 1; }

  ICClist <ICCtag> t(tags.size()-1); DBG_PROG
    DBG_PROG_V (tags.size())
  int i = 0,
      zahl = 0; DBG_PROG
  for (; i < (int)tags.size(); i++)
  {
    if (i != item)
    {   DBG_PROG
      t[zahl] = tags[i]; DBG_PROG_S("i: " << i << " -> zahl: " << zahl)
      zahl++; DBG_PROG
    }
  }

    DBG_PROG
# if 0
  tags.resize(t.size());
  for (int i = 0; i < (int)t.size(); i++)
    tags[i].copy (t[i]);
# else
  tags = t;
# endif
    DBG_MEM_V( i << " " << tags.size())

  DBG_PROG_ENDE
  return error;
}


