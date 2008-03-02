/*
 * ICC Examin ist eine ICC Profil Betrachter
 * 
 * Copyright (C) 2004-2007  Kai-Uwe Behrmann 
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
 * Der CMS Sortierer.
 * 
 */

/* Date:      25. 11. 2004 */

#include "icc_oyranos_extern.h"

void* myCAllocFunc(size_t size)
{
  return calloc( sizeof(char), size );
}

void myCDeAllocFunc(void * buf)
{
  if(buf)
    free(buf);
}

/** @brief number of channels in a colour space
 *
 *  since: (ICC Examin: version 0.45)
 */
int
oyColourSpaceGetChannelCountFromSig ( icColorSpaceSignature color )
{
  int n;

  switch (color) {
    case icSigXYZData: n = 3; break;
    case icSigLabData: n = 3; break;
    case icSigLuvData: n = 3; break;
    case icSigYCbCrData: n = 3; break;
    case icSigYxyData: n = 3; break;
    case icSigRgbData: n = 3; break;
    case icSigGrayData: n = 1; break;
    case icSigHsvData: n = 3; break;
    case icSigHlsData: n = 3; break;
    case icSigCmykData: n = 4; break;
    case icSigCmyData: n = 3; break;
    case icSig2colorData: n = 2; break;
    case icSig3colorData: n = 3; break;
    case icSig4colorData: n = 4; break;
    case icSig5colorData:
    case icSigMCH5Data:
          n = 5; break;
    case icSig6colorData:
    case icSigMCH6Data:
         n = 6; break;
    case icSig7colorData:
    case icSigMCH7Data:
         n = 7; break;
    case icSig8colorData:
    case icSigMCH8Data:
         n = 8; break;
    case icSig9colorData:
    case icSigMCH9Data:
         n = 9; break;
    case icSig10colorData:
    case icSigMCHAData:
         n = 10; break;
    case icSig11colorData:
    case icSigMCHBData:
         n = 11; break;
    case icSig12colorData:
    case icSigMCHCData:
         n = 12; break;
    case icSig13colorData:
    case icSigMCHDData:
         n = 13; break;
    case icSig14colorData:
    case icSigMCHEData:
         n = 14; break;
    case icSig15colorData:
    case icSigMCHFData:
         n = 15; break;
    default: n = 0; break;
  }
  return n;
}

/** @brief number of channels in a colour space
 *
 *  since: (ICC Examin: version 0.45)
 */
int
oyColourSpaceGetChannelCount( oyProfile_s * ref )
{
  icColorSpaceSignature sig = oyProfileGetSig ( ref );
  return oyColourSpaceGetChannelCountFromSig( sig );
}

/** @brief name of a colour space
 *
 *  since: (ICC Examin: version 0.45)
 */
const char *
oyColourSpaceGetNameFromSig( icColorSpaceSignature sig )
{
  const char * text;

  switch (sig) {
    case icSigXYZData: text =_("XYZ"); break;
    case icSigLabData: text =_("Lab"); break;
    case icSigLuvData: text =_("Luv"); break;
    case icSigYCbCrData: text =_("YCbCr"); break;
    case icSigYxyData: text =_("Yxy"); break;
    case icSigRgbData: text =_("Rgb"); break;
    case icSigGrayData: text =_("Gray"); break;
    case icSigHsvData: text =_("Hsv"); break;
    case icSigHlsData: text =_("Hls"); break;
    case icSigCmykData: text =_("Cmyk"); break;
    case icSigCmyData: text =_("Cmy"); break;
    case icSig2colorData: text =_("2color"); break;
    case icSig3colorData: text =_("3color"); break;
    case icSig4colorData: text =_("4color"); break;
    case icSigMCH5Data:
    case icSig5colorData: text =_("5color"); break;
    case icSigMCH6Data:
    case icSig6colorData: text =_("6color"); break;
    case icSigMCH7Data:
    case icSig7colorData: text =_("7color"); break;
    case icSigMCH8Data:
    case icSig8colorData: text =_("8color"); break;
    case icSigMCH9Data:
    case icSig9colorData: text =_("9color"); break;
    case icSigMCHAData:
    case icSig10colorData: text =_("10color"); break;
    case icSigMCHBData:
    case icSig11colorData: text =_("11color"); break;
    case icSigMCHCData:
    case icSig12colorData: text =_("12color"); break;
    case icSigMCHDData:
    case icSig13colorData: text =_("13color"); break;
    case icSigMCHEData:
    case icSig14colorData: text =_("14color"); break;
    case icSigMCHFData:
    case icSig15colorData: text =_("15color"); break;
    default: { icUInt32Number i = oyValueCSpaceSig(sig);
               char t[8];
               memcpy (t,(char*)&i, 4);
               t[4] = '?';
               t[5] = 0;
               text = &t[0];
               break;
             }
  }
  return text;
}

/** @brief name of a colour space
 *
 *  since: (ICC Examin: version 0.45)
 */
const char *
oyColourSpaceGetName( oyProfile_s * ref )
{
  icColorSpaceSignature sig = oyProfileGetSig ( ref );
  return oyColourSpaceGetNameFromSig( sig );
}

/** @brief copy pure colours

    handle colour only, without from set to -1 default
 *
 *  since: (ICC Examin: version 0.45)
 */
void
oyCopyColour ( const double * from,
               double       * to,
               int            n,
               oyProfile_s  * ref,
               int            channels_n )
{
  int i, j;
  icColorSpaceSignature sig = oyProfileGetSig ( ref );
  int c = oyColourSpaceGetChannelCountFromSig( sig );

  if(!n || !to)
    return;

  if(from)
    memcpy( to, from, sizeof(double) * n * c );

  else
  {
    if(!channels_n && c)
      channels_n = c;
    else if(channels_n && !c)
      c = channels_n;

    if(channels_n)
    switch(sig)
    {
      case icSigLabData:
      case icSigLuvData:
      case icSigHsvData:
      case icSigHlsData:
      case icSigYCbCrData:
           for( i = 0; i < n; ++i )
           {
             to[i*channels_n+0] = -1;
             to[i*channels_n+1] = 0;
             to[i*channels_n+2] = 0;
             for( j = c; j < channels_n; ++j )
               if(j==c)
                 to[i*channels_n+j] = 1; /* set alpha */
               else
                 to[i*channels_n+j] = 0;
           }
           break;
      case icSigRgbData:
      default:
           for( i = 0; i < n; ++i )
           {
             for( j = 0; j < channels_n; ++j )
               if(j < c)
                 to[i*channels_n+j] = -1;
               else if(j==c)
                 to[i*channels_n+j] = 1; /* set alpha */
               else
                 to[i*channels_n+j] = 0;
           }
           break;
    }
  }
}

/* Has to go to oyranos_texts.x */
char*
oyStringCopy_       ( const char    * text,
                     oyAllocFunc_t   allocateFunc )
{
  char * text_copy = NULL;

  if(text)
    if( strlen(text) )
    {
      text_copy = allocateFunc(strlen(text) + 1);
      sprintf( text_copy, "%s", text );
    }
  return text_copy;
}

/** @brief manage complex oyNamedColour_s inside Oyranos
 *
 *  since: (ICC Examin: version 0.45)
 *
 *  @param[in]  chan        pointer to channel data with a number of elements specified in sig or channels_n
 *  @param[in]  sig         ICC colour space signature http://www.color.org
 *  @param[in]  channels_n  elements of channels including alpha ...
 *  @param[in]  names_chan  needed if sig provides no glue about the channel colour or you use more than one extra (non colour) channel. The first deteted extra channel is by default considered alpha.
 *  @param[in]  name        of the colour
 *  @param[in]  name_long   description
 *  @param[in]  nick_name   very short, only a few letters
 *  @param[in]  blob        CGATS or other reference data
 *  @param[in]  blob_len    length of the data blob
 *  @param[in]  ref_file    possibly a ICC profile, allowed abreviations are 'sRGB' 'XYZ' 'Lab'(D50)
 *  @param[in]  allocateFunc   user controled memory allocation
 *  @param[in]  deallocateFunc user controled memory deallocation
 * 
 */
oyNamedColour_s*
oyNamedColourCreate( const double* chan,
                     int           channels_n,
                     const char  * name,
                     const char  * name_long,
                     const char  * nick_name,
                     const char  * blob,
                     int           blob_len,
                     oyProfile_s * ref,
                     oyAllocFunc_t   allocateFunc,
                     oyDeAllocFunc_t deallocateFunc )
{
  oyNamedColour_s * colour = (oyNamedColour_s*)allocateFunc(sizeof(oyNamedColour_s));

  if(!colour)
  {
    WARN_S("MEM Error.")
    return NULL;
  }

  memset( colour, 0, sizeof(oyNamedColour_s) );

  colour->type = oyOBJECT_TYPE_NAMED_COLOUR_S;
  if(deallocateFunc)
    colour->deallocateFunc = deallocateFunc;
  else
    colour->deallocateFunc = myCDeAllocFunc;

  if(allocateFunc)
    colour->allocateFunc = allocateFunc;
  else
    colour->allocateFunc = myCAllocFunc;

  if(channels_n)
    colour->channels_n = channels_n;
  else if(ref)
    colour->channels_n = oyColourSpaceGetChannelCount( ref ) + 1;
  else
    colour->channels_n = 0;

  if(colour->channels_n)
    colour->channels = colour->allocateFunc( colour->channels_n );
  oyCopyColour( chan, &colour->channels[0], 1, ref, colour->channels_n );

  colour->name      = oyStringCopy_( name, colour->allocateFunc );
  colour->name_long = oyStringCopy_( name_long, colour->allocateFunc );
  colour->nick_name = oyStringCopy_( nick_name, colour->allocateFunc );

  colour->profile   = oyProfileCopy( ref, colour->allocateFunc );

  if(blob && blob_len)
  {
    colour->blob = colour->allocateFunc( blob_len );
    memcpy( colour->blob, blob, blob_len );
  }

  return colour;
}

/** @brief set channel names
 *
 *  The function should be used to specify extra channels or unusual colour
 *  layouts like CMYKRB. The number of elements in names_chan should fit to the
 *  channels count or to the colour space signature.
 *
 *  You can let single entries empty if they are understandable by the
 *  colour space signature. Oyranos will set them for you on request.
 *
 *  @param[in] colour   address of a Oyranos named colour structure
 *  @param[in] names    pointer to channel names 
 *
 *  since: (ICC Examin: version 0.45)
 */
void
oyNamedColourSetChannelNames          ( oyNamedColour_s  * colour,
                                        const char      ** names_chan )
{
  oyNamedColour_s * c = colour;
  if(names_chan && c->channels_n)
  {
    int i = 0;
    c->names_chan = c->allocateFunc( (c->channels_n + 1 ) * sizeof(char*) );
    c->names_chan[ c->channels_n ] = NULL;
    for( ; i < c->channels_n; ++i )
    {
      int len = 0;
      if(names_chan[i])
        len = strlen(names_chan[i]);
      if(len)
      {
        c->names_chan[i] = c->allocateFunc((len + 1 ) * sizeof(char));
        sprintf( c->names_chan[i], "%s", names_chan[i] );
      }
    }
  }
}

/** @brief copy
 *
 *  
 *
 *  @param[in] colour    address of Oyranos colour struct
 *  @return              copy
 *
 *  since: (ICC Examin: version 0.45)
 */
oyNamedColour_s*
oyNamedColourCopy  ( const oyNamedColour_s * colour,
                     oyAllocFunc_t   allocateFunc,
                     oyDeAllocFunc_t deallocateFunc )
{
  oyNamedColour_s * c = NULL;

  if(!colour)
    return c;

  c = oyNamedColourCreate( colour->channels,
                     colour->channels_n,
                     colour->name,
                     colour->name_long,
                     colour->nick_name,
                     colour->blob,
                     colour->blob_len,
                     colour->profile,
                     allocateFunc   ? allocateFunc   : colour->allocateFunc,
                     deallocateFunc ? deallocateFunc : colour->deallocateFunc );

  oyNamedColourSetChannelNames( c, (const char**)colour->names_chan );

  return c;
}


/*  @return                 pointer to D50 Lab doubles with L 0.0...1.0 a/b -1.27...1.27 */

int
oyNamedColourSetSpace ( oyNamedColour_s * colour,
                        oyProfile_s     * profile )
{
  WARN_S("not implemented");
  return 1;
}

int
oyNamedColourConvert  ( oyNamedColour_s * colour,
                        oyProfile_s     * profile,
                        oyPointer       * buf,
                        oyDATATYPE_e      buf_type )
{                        
  oyImage_s * in  = NULL,
            * out = NULL;
  oyColourConversion_s * conv = NULL;
  int ret = 0;

  in    = oyImageCreate( 1,1, 
                         (oyPointer*) colour->channels ,oyDOUBLE,
                         colour->profile,
                         0,0 );
  out   = oyImageCreate( 1,1, 
                         buf ,buf_type,
                         profile,
                         0,0 );

  conv = oyColourConversionCreate( 0,0,0, in,out );
  ret = oyColourConversionRun( conv );
  oyColourConversionRelease( conv );
  return ret;
}

/** @brief get associated colour name
 *
 *
 *  @param[in]    Oyranos colour struct pointer
 *
 *  since: (ICC Examin: version 0.45)
 */
const char *
oyNamedColourGetName( oyNamedColour_s * colour )
{
  double lab[3];
  oyProfile_s * profile;

  if(!colour)
    return NULL;

  if(colour->name)
    return colour->name;

  profile = oyProfileCreate ( icSigLabData, NULL, 0,NULL );
  oyProfileRelease ( profile );
  profile = oyProfileCreate ( 0, "Lab", 0,NULL );
  oyNamedColourConvert  ( colour, profile, (oyPointer*)lab, oyDOUBLE );
  oyProfileRelease ( profile );

  colour->name = (char*) colour->allocateFunc(80);
  snprintf( colour->name, 80, "%s%sLab: %.02f %.02f %.02f",
            colour->nick_name ? colour->nick_name : "",
            colour->nick_name ? " - " : "",
            lab[0], lab[1], lab[2]
             );
    
  return colour->name;
}

/** @brief get associated colour nick name
 *
 *
 *  @param[in]    Oyranos colour struct pointer
 *
 *  since: (ICC Examin: version 0.45)
 */
const char *
oyNamedColourGetNick( oyNamedColour_s * colour )
{
  if(!colour)
    return NULL;

  return colour->nick_name;
}

/** @brief get associated colour description
 *
 *
 *  @param[in]    Oyranos colour struct pointer
 *
 *  since: (ICC Examin: version 0.45)
 */
const char *
oyNamedColourGetDescription( oyNamedColour_s * colour )
{
  int c, i;
  char* tmp;
  
  if(!colour)
    return NULL;

  if(colour->name_long)
    return colour->name_long;

  c = oyColourSpaceGetChannelCount( colour->profile );
  tmp = oyStringCopy_( oyNamedColourGetName( colour ), colour->allocateFunc );
  colour->name_long = (char*) colour->allocateFunc(80);
  sprintf(colour->name_long, "%s", tmp);
  if(colour->profile)
  {
    int len = strlen(colour->name_long);
    snprintf(&colour->name_long[len], 80-len, " %s:",
             oyColourSpaceGetName( colour->profile ) );
    for(i=0; i < c && c < 32; ++i)
    {
      len = strlen(colour->name_long);
      snprintf(&colour->name_long[len], 80-len, " %.02f", colour->channels[i]);
    }
    len = strlen(colour->name_long);
    /*snprintf(&colour->name_long[len], 80-len, "");*/
  }

  return colour->name_long;
}

/** @brief release correctly
 *
 *  set pointer to zero
 *  should walk to a general oyFree(void* oy_struct) function
 *
 *  @param[in]    address of Oyranos colour struct pointer
 *
 *  since: (ICC Examin: version 0.45)
 */
void
oyNamedColourRelease( oyNamedColour_s ** colour )
{
  oyNamedColour_s * c;
  int i;

  /* TODO */
  if(!colour || !*colour)
    return;

  c = *colour;
  if( c->type != oyOBJECT_TYPE_NAMED_COLOUR_S)
    return;

  c->type = oyOBJECT_TYPE_NONE;

  for(i = 0; i < 32; ++i)
    if(c->names_chan[i])
      c->deallocateFunc( c->names_chan[i] );
  if(c->name)
    c->deallocateFunc( c->name );
  if(c->name_long)
    c->deallocateFunc( c->name_long );
  if(c->nick_name)
    c->deallocateFunc( c->nick_name );
  if(c->blob) /* c->bloblen */
    c->deallocateFunc( c->blob );

  oyProfileRelease( c->profile );

  c->deallocateFunc( c );

  *colour = NULL;
}



/** @brief MSB<->LSB */
icUInt16Number
oyValueUInt16 (icUInt16Number val)
{
#if BYTE_ORDER == LITTLE_ENDIAN
# define BYTES 2
# define KORB  4
  unsigned char        *temp  = (unsigned char*) &val;
  static unsigned char  korb[KORB];
  int i;
  for (i = 0; i < KORB ; i++ )
    korb[i] = (int) 0;  /* empty */

  {
  int klein = 0,
      gross = BYTES - 1;
  for (; klein < BYTES ; klein++ ) {
    korb[klein] = temp[gross--];
#   ifdef DEBUG_ICCFUNKT
    cout << klein << " "; DBG_PROG
#   endif
  }
  }

  {
  unsigned int *erg = (unsigned int*) &korb[0];

# ifdef DEBUG_ICCFUNKT
# if 0
  cout << *erg << " Groesse nach Wandlung " << (int)korb[0] << " "
       << (int)korb[1] << " " << (int)korb[2] << " " <<(int)korb[3]
       << " "; DBG_PROG
# else
  cout << *erg << " size after conversion " << (int)temp[0] << " " << (int)temp[1]
       << " "; DBG_PROG
# endif
# endif
# undef BYTES
# undef KORB
  return (long)*erg;
  }
#else
  return (long)val;
#endif
}

icUInt32Number
oyValueUInt32 (icUInt32Number val)
{
#if BYTE_ORDER == LITTLE_ENDIAN
  unsigned char        *temp = (unsigned char*) &val;

  static unsigned char  uint32[4];

  uint32[0] = temp[3];
  uint32[1] = temp[2];
  uint32[2] = temp[1];
  uint32[3] = temp[0];

  {
  unsigned int *erg = (unsigned int*) &uint32[0];

# ifdef DEBUG_ICCFUNKT
  cout << *erg << " size after conversion " << (int)temp[0] << " "
       << (int)temp[1] << " " << (int)temp[2] << " " <<(int)temp[3]
       << " "; DBG_PROG
# endif

  return (icUInt32Number) *erg;
  }
#else
# ifdef DEBUG_ICCFUNKT
  cout << "BIG_ENDIAN" << " "; DBG_PROG
# endif
  return (icUInt32Number)val;
#endif
}

unsigned long
oyValueUInt64 (icUInt64Number val)
{
#if BYTE_ORDER == LITTLE_ENDIAN
  unsigned char        *temp  = (unsigned char*) &val;

  static unsigned char  uint64[8];
  int little = 0,
      big    = 8;

  for (; little < 8 ; little++ ) {
    uint64[little] = temp[big--];
  }

  {
  unsigned long *erg = (unsigned long*) &uint64[0];

# ifdef DEBUG_ICCFUNKT
  cout << *erg << " size after conversion " << (int)temp[0] << " "
       << (int)temp[1] << " " << (int)temp[2] << " " <<(int)temp[3]
       << " "; DBG_PROG
# endif
  return (long)*erg;
  }
#else
  return (long)val;
#endif
}

#define icValue_to_icUInt32Number(funkname, typ) \
typ \
funkname (typ val) \
{ \
  icUInt32Number i = (typ) val; \
  return (typ) oyValueUInt32 (i); \
}

icValue_to_icUInt32Number( oyValueCSpaceSig, icColorSpaceSignature )
icValue_to_icUInt32Number( oyValuePlatSig, icPlatformSignature )
icValue_to_icUInt32Number( oyValueProfCSig, icProfileClassSignature )
icValue_to_icUInt32Number( oyValueTagSig, icTagSignature )


