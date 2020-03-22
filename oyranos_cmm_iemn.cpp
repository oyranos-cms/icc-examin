/** @file oyranos_cmm_iemn.c
 *
 *  a filter for Oyranos 
 *
 *  @par Copyright:
 *            2010-2019 (C) Kai-Uwe Behrmann
 *
 *  @brief    ICC Examin viewer filter for Oyranos Color Management System
 *  @internal
 *  @author   Kai-Uwe Behrmann <ku.b@gmx.de>
 *  @par License:
 *            new BSD <http://www.opensource.org/licenses/bsd-license.php>
 *  @since    2010/05/17
 */


#include <oyranos.h>
#include <oyranos_conversion.h>
#include <oyranos_definitions.h>
#include <oyranos_version.h>
#include <oyCMM_s.h>
#include <oyPointer_s.h>
#include <oyCMMapi4_s.h>
#include <oyCMMapi7_s.h>
#include <oyCMMapi10_s.h>
#include <oyConnectorImaging_s.h>

#include <ctype.h>
#include <math.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <oyjl/oyjl.h> /* oyjl_val */
#include <oyjl/oyjl_macros.h> /* OYJL_DEBUG_ARGS */

#include "icc_cgats_filter.h"

/* --- internal definitions --- */

#ifdef __cplusplus
extern "C" {
#endif

/** The CMM_NICK consists of four bytes, which appear as well in the library name. This is important for Oyranos to identify the required filter struct name. */
#define CMM_NICK "iemn"
#define OY_ICMN_FILTER_REGISTRATION OY_TOP_INTERNAL OY_SLASH OY_DOMAIN_INTERNAL OY_SLASH OY_TYPE_STD OY_SLASH "icc_examin"

int iemnCMMWarnFunc( int code, const void* context, const char * format, ... );
/** The iemn_msg function pointer to use for messaging. */
oyMessage_f iemn_msg = iemnCMMWarnFunc;

int    iemnInit( oyStruct_s        * module_info );
int    iemnReset( oyStruct_s       * module_info );
extern oyCMMapi4_s   iemn_api4_iemn_filter;
extern oyCMMapi7_s   iemn_api7_iemn_filter;

#ifndef USE_I18N
/** i18n prototype */
#define _(text) text
#endif
#define _DBG_FORMAT_ "%s:%d %s()"
#define _DBG_ARGS_ __FILE__,__LINE__,__func__

/* --- implementations --- */

/** Function iemnCMMInit
 *  @brief API requirement
 *
 *  @version ICC Examin: 0.47
 *  @date    2010/05/17
 *  @since   2010/05/17 (ICC Examin: 0.47)
 */
int                iemnCMMInit       ( oyStruct_s * OY_UNUSED )
{
  int error = 0;
  return error;
}
int                iemnCMMReset      ( oyStruct_s * OY_UNUSED )
{ int error = 0; return error; }



/** Function iemnCMMWarnFunc
 *  @brief message handling
 *
 *  A internal implementation of a message function. Oyranos can select a 
 *  different one. Do not call this directly.
 *
 *  @version ICC Examin: 0.47
 *  @date    2010/05/17
 *  @since   2010/05/17 (ICC Examin: 0.47)
 */
int iemnCMMWarnFunc( int code, const void* cp, const char * format,...)
{
  char* text = (char*)calloc(sizeof(char), 4096);
  va_list list;
  const char * type_name = "";
  int id = -1;
  oyStruct_s * context = (oyStruct_s*) cp;

  if(context && oyOBJECT_NONE < context->type_)
  {
    type_name = oyStructTypeToText( context->type_ );
    id = oyStruct_GetId( context );
  }

  va_start( list, format);
  vsprintf( text, format, list);
  va_end  ( list );

  switch(code)
  {
    case oyMSG_WARN:
         fprintf( stderr, "WARNING"); fprintf( stderr, ": " );
         break;
    case oyMSG_ERROR:
         fprintf( stderr, "!!! ERROR"); fprintf( stderr, ": " );
         break;
  }

  fprintf( stderr, "%s[%d] ", type_name, id );

  fprintf( stderr, "%s", text ); fprintf( stderr, "%s", "\n" );
  free( text );

  return 0;
}

/** Function iemnCMMMessageFuncSet
 *  @brief API requirement
 *
 *  A Oyranos user might want its own message function and omit our internal
 *  one.
 *
 *  @version ICC Examin: 0.47
 *  @date    2010/05/17
 *  @since   2010/05/17 (ICC Examin: 0.47)
 */
int            iemnCMMMessageFuncSet ( oyMessage_f         message_func )
{
  iemn_msg = message_func;
  return 0;
}

/** \addtogroup misc_modules
 *  @{ */
/** \addtogroup iemn_misc iemn Module
 *  @brief      ICC Examin
 *
 *  The modules provide ICC observation functionality.
 *
 *  @{ */

/* OY_IEMN_PARSE_CGATS -------------------------- */

typedef enum {
  Lab = 0x01,
  XYZ = 0x02,
  CMYK = 0x04,
  RGB = 0x08,
  xyY = 0x10,
  SPEC = 0x20
} colorEncoding;
#define NAME 0x100
#define ID   0x200

int channelsForCEncoding(colorEncoding space)
{
  switch(space)
  {
    case Lab:
    case XYZ:
    case RGB:
    case xyY:
      return 3;
    case CMYK:
      return 4;
    case SPEC:
      return -1;
  }
  return 0;
}
void orderForCEncoding(const char ** fieldNames, colorEncoding space, int * order)
{
  int n = 0, i;
  while(fieldNames[n]) ++n;
  for(i = 0; i < n; ++i)
  {
    const char * name = fieldNames[i];
    switch(space)
    {
      case Lab:
        if(strcmp(name,"LAB_L") == 0) order[0] = i;
        if(strcmp(name,"LAB_A") == 0) order[1] = i;
        if(strcmp(name,"LAB_B") == 0) order[2] = i;
        break;
      case XYZ:
        if(strcmp(name,"XYZ_X") == 0) order[0] = i;
        if(strcmp(name,"XYZ_Y") == 0) order[1] = i;
        if(strcmp(name,"XYZ_Z") == 0) order[2] = i;
        break;
      case CMYK:
        if(strcmp(name,"CMYK_C") == 0) order[0] = i;
        if(strcmp(name,"CMYK_M") == 0) order[1] = i;
        if(strcmp(name,"CMYK_Y") == 0) order[2] = i;
        if(strcmp(name,"CMYK_K") == 0) order[3] = i;
        break;
      case RGB:
        if(strcmp(name,"RGB_R") == 0) order[0] = i;
        if(strcmp(name,"RGB_G") == 0) order[1] = i;
        if(strcmp(name,"RGB_B") == 0) order[2] = i;
        break;
      case xyY:
        if(strcmp(name,"XYY_X") == 0) order[0] = i;
        if(strcmp(name,"XYY_Y") == 0) order[1] = i;
        if(strcmp(name,"XYY_CAPY") == 0) order[2] = i;
        break;
      case SPEC:
        order[0] = order[1] = order[2] = order[3] = -1;
        break;
    }
  }
}


int orderForSpectral(const char ** fieldNames, int startNM, double lambda, int endNM, int * startNM_ret, double * lambda_ret, int * endNM_ret, int ** order_ret, int * count_ret, const char * spec)
{
  char ** nameList = NULL;
  int nameList_n = 0;
  int n = 0, i,j, fn = 0;
  double startNMd = 0, lambdad = 0, endNMd = 0;
  int error = -1;
  int * order = NULL;
  int speclen = strlen(spec);

  /* simple constrained case */
  if(startNM && lambda && endNM)
  {
    n = (endNM-startNM)/lambda + 1;
    oyjlAllocHelper_m(nameList, char*, n+1, malloc, return 1)
    for(i = 0; i < n; ++i)
      oyjlStringAdd( &nameList[i], 0,0, "SPECTRAL_%d", startNM + i  * lambda );
    startNMd = startNM;
    lambdad = lambda;
    endNMd = endNM;
  } else
    /* parameterless case */
  {
    int firstSpec = -1;
    while(fieldNames[fn]) ++fn;
    for(i = 0; i < fn; ++i)
    {
      const char * name = fieldNames[i];
      speclen = 0;

      if( strstr( name, spec ) != NULL ||
          strstr( name, "SPEC" ) != NULL ||
          strstr( name, "NM_") != NULL ||
          isdigit(name[0]) )
      {
        char ns = name[speclen];
        while(ns && (ns < '0' || '9' < ns)) ns = name[++speclen];
        const char * spect = name + speclen;
        double d = 0;
        int err = oyjlStringToDouble( spect, &d );
        if(err > 0)
          iemn_msg( oyMSG_WARN, 0, OYJL_DBG_FORMAT " could not detect spectral wave length: %s/%s", OYJL_DBG_ARGS, spect, name );
        oyjlStringListAddStaticString( &nameList, &nameList_n, name, 0,0 );
        if(n == 0)
        {
          firstSpec = i;
          startNMd = d;
        } else if(n == 1)
          lambdad = d - startNMd;
        else
          if(endNMd && fabs(d - endNMd) > lambdad + 1)
        {
          iemn_msg( oyMSG_WARN, 0, OYJL_DBG_FORMAT "  unexpected local lambda: %g (%s,%s,%s)", OYJL_DBG_ARGS, d - endNMd, nameList[i-firstSpec-2], nameList[i-firstSpec-1], nameList[i-firstSpec] );
          error = 1;
          break;
        }

        endNMd = d;
        ++n;
      }
    }
  }

  if(startNMd && lambdad && endNMd)
  {
    oyjlAllocHelper_m(order, int, n+1, malloc, return 1);
    j = 0;
    for(i = 0; i < fn; ++i)
    {
      if( strcmp( nameList[j], fieldNames[i] ) == 0 )
      {
        order[j] = i;
        ++j;
      }
    }
    error = 0;
    if(startNM_ret) *startNM_ret = startNMd;
    if(lambda_ret) *lambda_ret = lambdad;
    if(endNM_ret) *endNM_ret = endNMd;
    if(order_ret) *order_ret = order;
    if(count_ret) *count_ret = j;
  }
  oyjlStringListRelease( &nameList, n, NULL );

  return error;
}

void writeSpace(colorEncoding space, const char ** SampleNames, CgatsFilter * cgats, int m, int n, int id_index, int name_index, oyjl_val root, double max_rgb)
{
  const char * json_cn = NULL;
  switch(space)
  {
    case Lab: json_cn = "lab"; break;
    case XYZ: json_cn = "xyz"; break;
    case RGB: json_cn = "rgb"; break;
    case CMYK:json_cn = "cmyk"; break;
    case xyY: json_cn = "xyY"; break;
    case SPEC: break;
  }

  int i,j,cchan,order[4];
  if(space & Lab || space & XYZ || space & RGB || space & CMYK || space & xyY)
  {
    cchan = channelsForCEncoding(space);
    orderForCEncoding( (const char**)SampleNames, space, (int*)&order);
    for(i = 0; i < n; ++i)
    {
      const char * val;
      if(name_index >= 0)
      {
        val = cgats->messungen[m].block[i][name_index].c_str();
        oyjlTreeSetStringF( root, OYJL_CREATE_NEW, val, "collection/[%d]/colors/[%d]/name", m, i);
      }
      if(id_index >= 0)
      {
        val = cgats->messungen[m].block[i][id_index].c_str();
        if(name_index < 0)
          oyjlTreeSetStringF( root, OYJL_CREATE_NEW, val, "collection/[%d]/colors/[%d]/name", m, i);
        else
          oyjlTreeSetStringF( root, OYJL_CREATE_NEW, val, "collection/[%d]/colors/[%d]/id", m, i);
      }
      for(j = 0; j < cchan; ++j)
      {
        int index = order[j];
        val = cgats->messungen[m].block[i][index].c_str();
        double d = 0.0;
        int double_error = oyjlStringToDouble(val,&d);
        switch(space)
        {
          case Lab: if(j == 0) d /= 100.; else d = d/255. + 0.5; break;
          case XYZ: d /= 100.; break;
          case RGB: d /= max_rgb; break;
          case CMYK:d /= 100.; break;
          case xyY: break;
          case SPEC: break;
        }
        if(double_error <= 0)
          oyjlTreeSetDoubleF( root, OYJL_CREATE_NEW, d, "collection/[%d]/colors/[%d]/%s/[0]/data/[%d]", m, i, json_cn, j);
      }
    }
  }
}

void writeSpectral(const char ** SampleNames, CgatsFilter * cgats, int m, int n, int id_index, int name_index, oyjl_val root)
{
  const char * json_cn = "spectral";

  int i,j,
      cchan, // number color channels for a selected color space
      *order = NULL;

  int startNM = 0,
      endNM = 0;
  double lambda = 0;
  const char * spec = cgats->spektral.c_str();
  {
    int error = orderForSpectral(SampleNames, 0, 0, 0, &startNM, &lambda, &endNM, &order, &cchan, spec);
    if(error)
      iemn_msg( oyMSG_WARN, 0, OYJL_DBG_FORMAT "  %scould not get spectral properties", OYJL_DBG_ARGS, error > 0 ? "ERROR: ":"" );
    else if(lambda)
    {
      oyjlTreeSetDoubleF( root, OYJL_CREATE_NEW, m+1, "collection/[0]/spectral/[0]/id" );
      oyjlTreeSetDoubleF( root, OYJL_CREATE_NEW, startNM, "collection/[0]/spectral/[0]/startNM" );
      oyjlTreeSetDoubleF( root, OYJL_CREATE_NEW, lambda, "collection/[0]/spectral/[0]/lambda" );
      oyjlTreeSetDoubleF( root, OYJL_CREATE_NEW, (int)((endNM-startNM+lambda)/lambda + 0.5), "collection/[0]/spectral/[0]/steps" );
      oyjlTreeSetDoubleF( root, OYJL_CREATE_NEW, endNM, "collection/[0]/spectral/[0]/endNM" );
    }
    int block_count = cgats->messungen[m].block.size();
    for(i = 0; i < n; ++i)
    {
      const char * val;
      int count = cgats->messungen[m].block[i].size();
      if(name_index >= 0 && name_index < count)
      {
        val = cgats->messungen[m].block[i][name_index].c_str();
        oyjlTreeSetStringF( root, OYJL_CREATE_NEW, val, "collection/[%d]/colors/[%d]/name", m, i);
      } else
      {
        iemn_msg( oyMSG_WARN, 0, OYJL_DBG_FORMAT "  Internal error: unecpected name_index: %d|%d", OYJL_DBG_ARGS, name_index, count );
        oyjlTreeSetDoubleF( root, OYJL_CREATE_NEW, i, "collection/[%d]/colors/[%d]/name", m, i);
      }
      if(id_index >= 0 && id_index < count)
      {
        val = cgats->messungen[m].block[i][name_index].c_str();
        oyjlTreeSetStringF( root, OYJL_CREATE_NEW, val, "collection/[%d]/colors/[%d]/id", m, i);
      }

      oyjlTreeSetDoubleF( root, OYJL_CREATE_NEW, m+1, "collection/[%d]/colors/[%d]/%s/[0]/id", m, i, json_cn );
      for(j = 0; j < cchan; ++j)
      {
        int index = order[j];
        const char * val = cgats->messungen[m].block[i][index].c_str();
        double d = 0.0;
        int double_error = oyjlStringToDouble(val,&d);
        if(double_error <= 0)
          oyjlTreeSetDoubleF( root, OYJL_CREATE_NEW, d, "collection/[%d]/colors/[%d]/%s/[0]/data/[%d]", m, i, json_cn, j);
      }
    }
  }
}


/** @brief   Parse a CGATS text
 *
 *  @version Oyranos: 0.9.7
 *  @since   2017/11/26 (Oyranos: 0.9.7)
 *  @date    2019/02/04
 */
oyPointer_s* iemnParseCGATS          ( const char        * cgatsT )
{
  int error = !cgatsT;
  double max_rgb = 255.0;
  oyPointer_s * ptr = NULL;
  oyjl_val root = oyjlTreeNew("");

  if(error) return ptr;

  oyjlTreeSetStringF( root, OYJL_CREATE_NEW, "ncc1", "type" );
  oyjlTreeSetStringF( root, OYJL_CREATE_NEW, "Named Color Collection v1", "comment" );

  if(strstr(cgatsT, "\nDESCRIPTOR \"Argyll Calibration Target chart"))
    max_rgb = 100.0;
  else
  if(strstr(cgatsT, "ORIGINATOR \"Monaco Systems, Inc\""))
    max_rgb = 1.0;

  /** @todo implement CGATS parsing with CgatsFilter */
  CgatsFilter * cgats = new CgatsFilter;
  int len = strlen(cgatsT);
  cgats->lade( cgatsT, len?len+1:0 );
  std::string text = cgats->lcms_gefiltert();

  int i;
  int m_n = cgats->messungen.size();
  int m = 0; // usualy the first data block
  int f_n = cgats->messungen[m].felder.size(); // should be always 1
  char ** SampleNames = NULL; // list from DATA_FORMAT
  int SNsize = 0;
  if(f_n != 1)
    iemn_msg( oyMSG_WARN, 0, OYJL_DBG_FORMAT "  Ambiguity: found more than one line in DATA_FORMAT section %d", OYJL_DBG_ARGS, f_n );
  for(m = 0; m < m_n; ++m)
  {
    int spaces = 0;
    int name_index = -1, id_index = -1;
    if(cgats->messungen[m].felder.size() == 0)
      break;
    int n = cgats->messungen[m].block_zeilen;
    int chan = cgats->messungen[m].felder[0].size();
    for (i = 0; i < (int)cgats->messungen[m].kommentare; ++i)
    {
      const char * name = cgats->messungen[m].kommentare[i].c_str();
      std::string line = name;
      ICClist<std::string> v = cgats->unterscheideZiffernWorte( line );
      if(icc_debug) iemn_msg( oyMSG_DBG, 0, OYJL_DBG_FORMAT "  comments: %s", OYJL_DBG_ARGS, name );
      const char * val = v.size() > 1 ? v[1].c_str() : NULL;
      double d = 0.0;
      int double_error = oyjlStringToDouble(val,&d);

#define SET_VAL(_CGATS_NAME_, _JSON_NAME_,_test_double_) \
      if(strstr(name, _CGATS_NAME_) != NULL) { (_test_double_ && (double_error <= 0)) ? oyjlTreeSetDoubleF( root, OYJL_CREATE_NEW, d, _JSON_NAME_) : oyjlTreeSetStringF( root, OYJL_CREATE_NEW, val, _JSON_NAME_ ); }

           SET_VAL("ORIGINATOR",        "creator",            0)
      else SET_VAL("DESCRIPTOR",        "description",        0)
      else SET_VAL("CREATED",           "date",               0)
      else SET_VAL("MANUFACTURER",      "manufacturer",       0)
      else SET_VAL("PROD_DATE",         "prod_date",          0)
      else SET_VAL("SERIAL",            "serial",             0)
      else SET_VAL("MATERIAL",          "material",           0)
      else SET_VAL("INSTRUMENTATION",   "collection/[0]/spectral/[0]/measurement/device/instrumentation",    0)
      else SET_VAL("TARGET_INSTRUMENT", "collection/[0]/spectral/[0]/measurement/device/instrumentation",    0) // Argyll CMS
      else SET_VAL("MEASUREMENT_SOURCE","collection/[0]/spectral/[0]/measurement/device/illumination", 0)
      else SET_VAL("PRINT_CONDITIONS",  "print_conditions",   0)
      else SET_VAL("DEVICE_CLASS",      "collection/[0]/spectral/[0]/measurement/device/class",       0) // Argyll CMS
      else SET_VAL("LUMINANCE_XYZ_CDM2","luminance",          0) // Argyll CMS
      else SET_VAL("SPECTRAL_START_NM", "collection/[0]/spectral/[0]/startNM",   1)
      else SET_VAL("SPECTRAL_NORM",     "collection/[0]/spectral/[0]/lambda",    1)
      else SET_VAL("SPECTRAL_END_NM",   "collection/[0]/spectral/[0]/endNM",     1)
#undef SET_VAL
    }
    if(SampleNames)
    { oyjlStringListRelease( &SampleNames, SNsize, 0 ); SNsize = 0; }
    for (i = 0; i < chan; ++i)
    {
      const char * name = cgats->messungen[m].felder[0][i].c_str();
      oyjlStringListAddStaticString( &SampleNames, &SNsize, name, 0,0 );
      if(icc_debug)
        iemn_msg( oyMSG_DBG, 0, OYJL_DBG_FORMAT "  FieldName: %s", OYJL_DBG_ARGS, name );
    }
    for (i = 0; i < chan; ++i)
    {
      if (strstr(SampleNames[i],"SAMPLE_ID") != 0)
      {
        id_index = i;
        if(!(spaces&ID)) iemn_msg( oyMSG_DBG,0, "ID" );
        spaces |= ID;
      }
      if (strstr(SampleNames[i],"SAMPLE_NAME") != 0)
      {
        name_index = i;
        if(!(spaces&NAME)) iemn_msg( oyMSG_DBG,0, "NAME" );
        spaces |= NAME;
      }
      if ((strstr (SampleNames[i], "LAB_L") != 0)
       || (strstr (SampleNames[i], "LAB_A") != 0)
       || (strstr (SampleNames[i], "LAB_B") != 0))
      {
        if(!(spaces&Lab)) iemn_msg( oyMSG_DBG,0, "Lab data " );
        spaces |= Lab;
      } else if ((strstr (SampleNames[i], "XYZ_X") != 0)
              || (strstr (SampleNames[i], "XYZ_Y") != 0)
              || (strstr (SampleNames[i], "XYZ_Z") != 0)) {
        if(!(spaces&XYZ)) iemn_msg( oyMSG_DBG,0, "XYZ data " );
        spaces |= XYZ;
      } else if ((strstr (SampleNames[i], "CMYK_C") != 0)
              || (strstr (SampleNames[i], "CMYK_M") != 0)
              || (strstr (SampleNames[i], "CMYK_Y") != 0)
              || (strstr (SampleNames[i], "CMYK_K") != 0))
      {
        if(!(spaces&CMYK)) iemn_msg( oyMSG_DBG,0, "CMYK data " );
        spaces |= CMYK;
      } else if ((strstr (SampleNames[i], "RGB_R") != 0)
              || (strstr (SampleNames[i], "RGB_G") != 0)
              || (strstr (SampleNames[i], "RGB_B") != 0))
      {
        if(!(spaces&RGB)) iemn_msg( oyMSG_DBG,0, "RGB data " );
        spaces |= RGB;
      } else if ((strstr (SampleNames[i], "XYY_X") != 0)
              || (strstr (SampleNames[i], "XYY_Y") != 0)
              || (strstr (SampleNames[i], "XYY_CAPY") != 0))
      {
        if(!(spaces&xyY)) iemn_msg( oyMSG_DBG,0, "xyY data " );
        spaces |= xyY;
      } else if (strstr (SampleNames[i], cgats->spektral.c_str()) != 0 ||
                 strstr (SampleNames[i], "SPEC") != 0 ||
                 strstr (SampleNames[i], "NM_") != 0 ||
                 isdigit(SampleNames[i][0]) )
      {
        if(!(spaces&SPEC)) iemn_msg( oyMSG_DBG,0, "Spectral data " );
        spaces |= SPEC;
      }
    }

    // write data
    if(spaces & Lab) writeSpace( Lab, (const char **) SampleNames, cgats, m, n, id_index, name_index, root, 0.0);
    if(spaces & XYZ) writeSpace( XYZ, (const char **) SampleNames, cgats, m, n, id_index, name_index, root, 0.0);
    if(spaces & RGB) writeSpace( RGB, (const char **) SampleNames, cgats, m, n, id_index, name_index, root, max_rgb);
    if(spaces & CMYK) writeSpace( CMYK, (const char **) SampleNames, cgats, m, n, id_index, name_index, root, 0.0);
    if(spaces & xyY) writeSpace( xyY, (const char **) SampleNames, cgats, m, n, id_index, name_index, root, 0.0);
    if(spaces & SPEC) writeSpectral( (const char **) SampleNames, cgats, m, n, id_index, name_index != -1 ? name_index : id_index, root);
  }

  ptr = oyPointer_New(0);
  oyPointer_Set( ptr, __FILE__,
                 "oyjl_val", root, 0, 0 );

  delete cgats;
  return ptr;
}
#define OY_IEMN_PARSE_CGATS OY_TOP_SHARED OY_SLASH OY_DOMAIN_INTERNAL OY_SLASH OY_TYPE_STD OY_SLASH \
  "parse_cgats.cgats._" CMM_NICK "._CPU"

/** @brief  iemnMOptions_Handle5()
 *  This function implements oyMOptions_Handle_f.
 *
 *  @param[in]     options             expects at least one options
 *                                     - "cgats": The option shall be a string.
 *  @param[in]     command             "//" OY_TYPE_STD "/parse_cgats"
 *  @param[out]    result              will contain a oyPointer_s in "data"
 *
 *  The Handler uses internally iemnParseCGATS().
 *
 *  @version Oyranos: 0.9.7
 *  @since   2017/11/26 (Oyranos: 0.9.7)
 *  @date    2019/02/04
 */
int          iemnMOptions_Handle5   ( oyOptions_s       * options,
                                       const char        * command,
                                       oyOptions_s      ** result )
{
  int error = 0;

  if(oyFilterRegistrationMatch(command,"can_handle", oyOBJECT_NONE))
  {
    if(oyFilterRegistrationMatch(command,"parse_cgats", oyOBJECT_NONE))
    {
      const char * cgats = oyOptions_FindString( options, "cgats", 0 );
      if(!cgats) error = 1;
      return error;
    }
    else
      return -1;
  }
  else if(oyFilterRegistrationMatch(command,"parse_cgats", oyOBJECT_NONE))
  {
    oyPointer_s * spec = NULL;
    const char * cgats = NULL;

    cgats = oyOptions_FindString( options, "cgats", 0 );

    spec = iemnParseCGATS( cgats );

    if(spec)
    {
      oyOption_s * o = oyOption_FromRegistration( OY_TOP_SHARED OY_SLASH OY_DOMAIN_INTERNAL OY_SLASH OY_TYPE_STD OY_SLASH "cgats.data._" CMM_NICK, 0 );
      error = oyOption_MoveInStruct( o, (oyStruct_s**) &spec );
      if(!*result)
        *result = oyOptions_New(0);
      oyOptions_MoveIn( *result, &o, -1 );
    } else
        iemn_msg( oyMSG_WARN, (oyStruct_s*)options, OYJL_DBG_FORMAT
                   "parsing creation failed",
                   OYJL_DBG_ARGS );
  }

  return 0;
}
/**
 *  This function implements oyCMMGetText_f.
 *
 *  @version ICC Examin: 0.57
 *  @since   2019/02/07 (ICC Examin: 0.57)
 *  @date    2019/02/07
 */
const char * iemnApi10UiGetText (
                                       const char        * select,
                                       oyNAME_e            type,
                                       oyStruct_s        * context )
{
  iemn_msg( oyMSG_DBG, context, "select: %s type: %d", select, (int)type );
         if(strcmp(select,"name")==0)
  {
         if(type == oyNAME_NICK)
      return "iemn.parse_cgats";
    else if(type == oyNAME_NAME)
      return _("Color book[iemn]");
    else
      return _("ICC Examin CGATS parser");
  } else if(strcmp(select, "can_handle")==0)
  {
         if(type == oyNAME_NICK)
      return "check";
    else if(type == oyNAME_NAME)
      return _("check");
    else
      return _("Check if this module can handle a certain command.");
  } else if(strcmp(select, "parse_cgats")==0)
  {
         if(type == oyNAME_NICK)
      return "parse_cgats";
    else if(type == oyNAME_NAME)
      return _("Parse CGATS text.");
    else
      return _("The ICC Examin \"parse_cgats\" command lets you parse CGATS files. The filter expects a oyOption_s object with name \"cgats\" containing a string value. The result will appear in \"data\" as a oyPointer_s containing a oyjl_val.");
  }
  else if(strcmp(select,"help")==0)
  {
         if(type == oyNAME_NICK)
      return "help";
    else if(type == oyNAME_NAME)
      return _("Parse CGATS in a robust way.");
    else
      return _("The filter is activated by the \"pares_cgats\" command. It expects a \"cgats\" option containing a string with CGATS color sample format. The result will contain a \"data\" object with a oyPointer_s object referencing a oyjl_val JSON data structure ready for the oyTreeToCGATS() API");
  }
  return 0;
}
const char *iemn_texts_parse_cgats[4] = {"can_handle","parse_cgats","help",0};

#if 0
/** l2cms_api10_cmm5
 *  @brief   Node for Parsing a CGATS text
 *
 *  littleCMS 2 oyCMMapi10_s implementation
 *
 *  For the front end API see oyOptions_Handle(). The backend options
 *  are described in l2cmsMOptions_Handle5().
 *
 *  @version Oyranos: 0.9.7
 *  @since   2017/06/05 (Oyranos: 0.9.7)
 *  @date    2019/02/04
 */
oyCMMapi10_s_    l2cms_api10_cmm5 = {

  oyOBJECT_CMM_API10_S,
  0,0,0,
  0,

  l2cmsCMMInit,
  l2cmsCMMMessageFuncSet,

  OY_LCM2_PARSE_CGATS,

  CMM_VERSION,
  CMM_API_VERSION,                  /**< int32_t module_api[3] */
  0,   /* id_; keep empty */
  0,   /* api5_; keep empty */
  0,   /* runtime_context */
 
  l2cmsInfoGetTextProfileC5,            /**< getText */
  (char**)l2cms_texts_parse_cgats,      /**<texts; list of arguments to getText*/
 
  l2cmsMOptions_Handle5                 /**< oyMOptions_Handle_f oyMOptions_Handle */
};
#endif

/* OY_IEMN_PARSE_CGATS -------------------------- */

/**  @} *//* iemn_misc */
/**  @} *//* misc_modules */

/**
 *  @brief   Node for Parsing a CGATS text
 *
 *  ICC Examin oyCMMapi10_s implementation
 *
 *  For the front end API see oyOptions_Handle(). The backend options
 *  are described in l2cmsMOptions_Handle5().
 *
 *  @version Oyranos: 0.9.7
 *  @since   2017/06/05 (Oyranos: 0.9.7)
 *  @date    2019/02/07
 */
int    iemnInit( oyStruct_s        * module_info )
{
  oyCMM_s * info = (oyCMM_s*) module_info;
  int32_t cmm_version[3] = {OYRANOS_VERSION_A,OYRANOS_VERSION_B,OYRANOS_VERSION_C},
          module_api[3]  = {0,9,7};
  oyCMMapi10_s * filter = oyCMMapi10_Create( iemnCMMInit, iemnCMMReset,
                                       iemnCMMMessageFuncSet,
                                       OY_IEMN_PARSE_CGATS,
                                       cmm_version,
                                       module_api,
                                       iemnApi10UiGetText,
                                       iemn_texts_parse_cgats,
                                       iemnMOptions_Handle5,
                                       NULL /* Oyranos object*/ );
  if( !info || module_info->type_ != oyOBJECT_CMM_INFO_S ||
      memcmp(info->cmm,CMM_NICK,4) != 0)
    iemn_msg( oyMSG_WARN, module_info, OYJL_DBG_FORMAT "wrong module info passed in", OYJL_DBG_ARGS );
  info->api = (oyCMMapi_s*)filter;
  icc_debug = oy_debug;
  return 0;
}

int    iemnReset( oyStruct_s        * module_info )
{
  oyCMM_s * info = (oyCMM_s*) module_info;
  if(info->api && info->api->release)
    info->api->release( (oyStruct_s**)&info->api );
  return 0;
}


/**
 *  This function implements oyCMMInfoGetText_f.
 *
 *  Implement at least "name", "manufacturer" and "copyright". If you like with
 *  internationalisation.
 *
 *  @version ICC Examin: 0.47
 *  @since   2010/05/17 (ICC Examin: 0.47)
 *  @date    2012/02/19
 */
const char * iemnGetText             ( const char        * select,
                                       oyNAME_e            type,
                                       oyStruct_s        * context OYJL_UNUSED )
{
         if(strcmp(select, "name")==0)
  {
         if(type == oyNAME_NICK)
      return _(CMM_NICK);
    else if(type == oyNAME_NAME)
      return _("ICC Examin output filter");
    else
      return _("ICC Examin output filter");
  } else if(strcmp(select, "manufacturer")==0)
  {
         if(type == oyNAME_NICK)
      return _("oy");
    else if(type == oyNAME_NAME)
      return _("Kai-Uwe Behrmann");
    else
      return _("Oyranos project; www: http://www.oyranos.org; support/email: ku.b@gmx.de; sources: http://www.oyranos.org/wiki/index.php?title=Oyranos/Download");
  } else if(strcmp(select, "copyright")==0)
  {
         if(type == oyNAME_NICK)
      return _("GPL2+");
    else if(type == oyNAME_NAME)
      return _("Copyright (c) 2010 Kai-Uwe Behrmann");
    else
      return _("GPL2+ license: http://www.opensource.org/licenses/gpl2-license.php");
  } else if(strcmp(select, "help")==0)
  {
         if(type == oyNAME_NICK)
      return _("help");
    else if(type == oyNAME_NAME)
      return _("The filter allows to visually observe a images colours.");
    else
      return _("The 3D view uses OpenGL to project the image colours into a 3D view representing the CIE*Lab colour space. The filter needs a running window system.");
  }
  return 0;
}
const char *iemn_texts[5] = {"name","copyright","manufacturer","help",0};
oyIcon_s iemn_icon = {oyOBJECT_ICON_S, 0,0,0, 0,0,0, "oyranos_logo.png"};


/** @instance iemn_cmm_module
 *  @brief    iemn module infos
 *
 *  This structure is dlopened by Oyranos. Its name has to consist of the
 *  following elements:
 *  - the four byte CMM_NICK plus
 *  - "_cmm_module"
 *  This string must be included in the the filters filename.
 *
 *  @version ICC Examin: 0.57
 *  @date    2019/02/05
 *  @since   2010/05/17 (ICC Examin: 0.47)
 */
oyCMM_s iemn_cmm_module = {

  oyOBJECT_CMM_INFO_S, /**< ::type; the object type */
  0,0,0,               /**< static objects omit these fields */
  CMM_NICK,            /**< ::cmm; the four char filter id */
  (char*)"0.9.7",     /**< ::backend_version */
  iemnGetText,         /**< ::getText; UI texts */
  (char**)iemn_texts,  /**< ::texts; list of arguments to getText */
  OYRANOS_VERSION,     /**< ::oy_compatibility; last supported Oyranos CMM API*/

  /** ::api; The first filter api structure. */
  NULL,

  /** ::icon; zero terminated list of a icon pyramid */
  &iemn_icon,
  iemnInit,
  iemnReset
};


/* OY_ICMN_FILTER_REGISTRATION ----------------------------------------------*/


#if 0
/** For very simple options we do not need event handling. Dummies suffice in
 *  this case. */
const char * iemnWidget_GetDummy     ( const char        * func_name,
                                       uint32_t          * result )
{return 0;}
oyWIDGET_EVENT_e iemnWidget_EventDummy
                                     ( const char        * wid,
                                       oyWIDGET_EVENT_e    type )
{return (oyWIDGET_EVENT_e)0;}


oyWIDGET_EVENT_e   iemnWidgetEvent   ( oyOptions_s       * options,
                                       oyWIDGET_EVENT_e    type,
                                       oyStruct_s        * event )
{return (oyWIDGET_EVENT_e)0;}


/** Function iemnFilterNode_MyContextToMem
 *  @brief   implement oyCMMFilter_ContextToMem_f()
 *
 *  Serialise into a Oyranos specific ICC profile containers "Info" tag.
 *  We do not have any binary context to include.
 *  Thus oyFilterNode_TextToInfo_() is fine.
 *
 *  @version ICC Examin: 0.47
 *  @date    2010/05/17
 *  @since   2010/05/17 (ICC Examin: 0.47)
 */
oyPointer  iemnFilterNode_MyContextToMem (
                                       oyFilterNode_s    * node,
                                       size_t            * size,
                                       oyAlloc_f           allocateFunc )
{
  return oyFilterNode_TextToInfo_( node, size, allocateFunc );
}

/** ncl2 profilbody */
char data[320] =
  {
    0,0,1,64,108,99,109,115,
    2,48,0,0,110,109,99,108,
    82,71,66,32,76,97,98,32,
    0,0,0,0,0,0,0,0,
    0,0,0,0,97,99,115,112,
    83,71,73,32,0,0,0,0,
    110,111,110,101,110,111,110,101,
    -64,48,11,8,-40,-41,-1,-65,
    0,0,0,0,0,0,-10,-42,
    0,1,0,0,0,0,-45,45,
    67,80,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,
    0,0,0,3,100,101,115,99,
    0,0,0,-88,0,0,0,33,
    99,112,114,116,0,0,0,-52,
    0,0,0,29,110,99,108,50,
    0,0,0,-20,0,0,0,84,
    116,101,120,116,0,0,0,0,
    67,105,110,101,80,97,105,110,
    116,32,99,111,108,111,117,114,
    32,115,97,109,112,108,101,115,
    0,0,0,0,116,101,120,116,
    0,0,0,0,110,111,116,32,
    99,111,112,121,114,105,103,104,
    116,101,100,32,100,97,116,97,
    0,0,0,0,110,99,108,50,
    0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,3,
    0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0
  };

/** ncl2 snipet - one colour */
typedef struct {
  char name[32];
  icUInt16Number pcsfarbe[3]; /* PCS Lab or XYZ */
  icUInt16Number geraetefarbe[16];
} Ncl2Farbe;

/** ncl2 profiletag */
typedef struct {
  char vendor_flag[4];
  icUInt32Number anzahl;
  icUInt32Number koord;
  char vorname[32];
  char nachname[32];
  Ncl2Farbe *farben;
} Ncl2;


oyOptions_s* iemnFilter_MyFilterValidateOptions
                                     ( oyFilterCore_s    * filter,
                                       oyOptions_s       * validate,
                                       int                 statical,
                                       uint32_t          * result )
{
  uint32_t error = !filter;

#if 0
  if(!error)
    error = !oyOptions_FindString( validate, "iemn_options", 0 );
#endif

  *result = error;

  return 0;
}

/** Function iemnFilterPlug_MyFilterRun
 *  @brief   implement oyCMMFilter_GetNext_f()
 *
 *  The primary filter entry for data processing.
 *
 *  @param         requestor_plug      the plug of the requesting node after 
 *                                     iemn filter in the graph
 *  @param         ticket              the job ticket
 *
 *  @version ICC Examin: 0.47
 *  @date    2010/05/17
 *  @since   2010/05/17 (ICC Examin: 0.47)
 */
int      iemnFilterPlug_MyFilterRun (
                                       oyFilterPlug_s    * requestor_plug,
                                       oyPixelAccess_s   * ticket )
{
  oyFilterSocket_s * socket;
  oyFilterPlug_s * plug;
  oyFilterNode_s * node;
  oyImage_s * image_input;
  uint8_t * u8_array;
  uint16_t * u16_array;
  int i;
  double my_max_value = 0.0;

  /** iemn filters socket */
  socket = requestor_plug->remote_socket_;
  /** iemn filter node */
  node = socket->node;
  /** iemn filters plug */
  plug = (oyFilterPlug_s *)node->plugs[0];

#if 0
  /** get the full image data in case iemn filter needs it */
  image_input = oyFilterPlug_ResolveImage( plug, socket, ticket );
#endif

  if( !ticket )
  {
    iemn_msg( oyMSG_WARN, (oyStruct_s*)node,
             "failed to get a job ticket");
    return 1;
  }

  /** Handle all supported data types. Here is the core of iemn filter. */
  if(ticket->array->t == oyUINT8)
  {
    u8_array = (uint8_t*)ticket->array->array2d[0];
    for(i = 0; i < ticket->array->width; ++i)
      if(u8_array[i] > my_max_value)
        my_max_value = u8_array[i];
  } else if(ticket->array->t == oyUINT16)
  {
    u16_array = (uint16_t*)ticket->array->array2d[0];
    for(i = 0; i < ticket->array->width; ++i)
      if(u16_array[i] > my_max_value)
        my_max_value = u16_array[i];
  }

  oyImage_Release( &image_input );

  return 0;
}


oyDATATYPE_e iemn_data_types[3] = {oyUINT8, oyUINT16, (oyDATATYPE_e)0};

/** My filters socket for delivering results */
oyConnectorImaging_s iemn_iemnFilter_connectorSocket = {
  oyOBJECT_CONNECTOR_IMAGING_S,0,0,0,
  oyCMMgetImageConnectorPlugText, /* getText */
  oy_image_connector_texts, /* texts */
  "//" OY_TYPE_STD "/image", /* connector_type */
  oyFilterSocket_MatchImagingPlug, /* filterSocket_MatchPlug */
  0, /* is_plug == oyFilterPlug_s */
  iemn_data_types,
  2, /* data_types_n; elements in data_types array */
  -1, /* max_colour_offset */
  1, /* min_channels_count; */
  3, /* max_channels_count; */
  1, /* min_colour_count; */
  3, /* max_colour_count; */
  0, /* can_planar; can read separated channels */
  1, /* can_interwoven; can read continuous channels */
  0, /* can_swap; can swap colour channels (BGR)*/
  0, /* can_swap_bytes; non host byte order */
  0, /* can_revert; revert 1 -> 0 and 0 -> 1 */
  1, /* can_premultiplied_alpha; */
  1, /* can_nonpremultiplied_alpha; */
  0, /* can_subpixel; understand subpixel order */
  0, /* oyCHANNELTYPE_e    * channel_types; */
  0, /* count in channel_types */
  1, /* id; relative to oyFilter_s, e.g. 1 */
  0  /* is_mandatory; mandatory flag */
};
/** My filter has just one socket connector. Tell about it. */
oyConnectorImaging_s * iemn_iemnFilter_connectorSockets[2] = 
             { &iemn_iemnFilter_connectorSocket, 0 };

/** My filters plug for obtaining data */
oyConnectorImaging_s iemn_iemnFilter_connectorPlug = {
  oyOBJECT_CONNECTOR_IMAGING_S,0,0,0,
  oyCMMgetImageConnectorPlugText, /* getText */
  oy_image_connector_texts, /* texts */
  "//" OY_TYPE_STD "/image", /* connector_type */
  oyFilterSocket_MatchImagingPlug, /* filterSocket_MatchPlug */
  1, /* is_plug == oyFilterPlug_s */
  iemn_data_types,
  2, /* data_types_n; elements in data_types array */
  -1, /* max_colour_offset */
  1, /* min_channels_count; */
  65535, /* max_channels_count; */
  1, /* min_colour_count; */
  65535, /* max_colour_count; */
  1, /* can_planar; can read separated channels */
  1, /* can_interwoven; can read continuous channels */
  0, /* can_swap; can swap colour channels (BGR)*/
  0, /* can_swap_bytes; non host byte order */
  0, /* can_revert; revert 1 -> 0 and 0 -> 1 */
  1, /* can_premultiplied_alpha; */
  1, /* can_nonpremultiplied_alpha; */
  0, /* can_subpixel; understand subpixel order */
  0, /* oyCHANNELTYPE_e    * channel_types; */
  0, /* count in channel_types */
  1, /* id; relative to oyFilter_s, e.g. 1 */
  0  /* is_mandatory; mandatory flag */
};
/** My filter has just one plug connector. Tell about it. */
oyConnectorImaging_s * iemn_iemnFilter_connectorPlugs[2] = 
             { &iemn_iemnFilter_connectorPlug, 0 };


/**
 *  This function implements oyCMMGetText_f.
 *
 *  @version ICC Examin: 0.47
 *  @since   2010/05/17 (ICC Examin: 0.47)
 *  @date    2010/05/17
 */
const char * iemnApi4UiGetText (
                                       const char        * select,
                                       oyNAME_e            type,
                                       oyStruct_s        * context )
{
  static char * category = 0;
  if(strcmp(select,"name"))
  {
         if(type == oyNAME_NICK)
      return "iemn_filter";
    else if(type == oyNAME_NAME)
      return _("Image[iemn_filter]");
    else
      return _("ICC Examin Object");
  }
  else if(strcmp(select,"help"))
  {
         if(type == oyNAME_NICK)
      return "help";
    else if(type == oyNAME_NAME)
      return _("Some help for My example filter.");
    else
      return _("More indepth help  for My example filter.");
  }
  else if(strcmp(select,"category"))
  {
    if(!category)
    {
      /* The following strings must match the categories for a menu entry. */
      const char * i18n[] = {_("Colour"),_("ICC Examin"),0};
      int len =  strlen(i18n[0]) + strlen(i18n[1]);
      category = (char*)malloc( len + 64 );
      if(category)
        /* Create a translation for iemn_api4_ui_iemn_filter::category. */
        sprintf( category,"%s/%s", i18n[0], i18n[1] );
      else
        iemn_msg(oyMSG_WARN, (oyStruct_s *) 0, _DBG_FORMAT_ "\n " "Could not allocate enough memory.", _DBG_ARGS_);
    }
         if(type == oyNAME_NICK)
      return "category";
    else if(type == oyNAME_NAME)
      return category;
    else
      return category;
  }
  return 0;
}
const char * iemn_api4_ui_texts[] = {"name", "category", "help", 0};
/** @instance iemn_api4_ui_iemn_filter
 *  @brief    iemn oyCMMapi4_s::ui implementation
 *
 *  The UI for iemn.
 *
 *  @version ICC Examin: 0.47
 *  @since   2010/05/17 (ICC Examin: 0.47)
 *  @date    2010/05/17
 */
oyCMMui_s iemn_api4_ui_iemn_filter = {
  oyOBJECT_CMM_DATA_TYPES_S,           /**< oyOBJECT_e       type; */
  0,0,0,                            /* unused oyStruct_s fields; keep to zero */

  {OYRANOS_VERSION_A,OYRANOS_VERSION_B,OYRANOS_VERSION_C}, /**< version[3] */
  {0,4,0},                            /**< int32_t module_api[3] */

  iemnFilter_MyFilterValidateOptions, /* oyCMMFilter_ValidateOptions_f */
  iemnWidgetEvent, /* oyWidgetEvent_f */

  "Filter/ICC Examin", /* UI category */
  0,   /* const char * options */
  0,   /* oyCMMuiGet_f oyCMMuiGet */

  iemnApi4UiGetText, /* oyCMMGetText_f   getText */
  iemn_api4_ui_texts /* const char    ** texts */
};

/** @instance iemn_api4
 *  @brief    iemn oyCMMapi4_s implementation
 *
 *  My filter api4 object to provide basic informations.
 *
 *  A context handler for preprocessing can be attached here. See for instance
 *  the 'lcms' filter.
 *
 *  @version ICC Examin: 0.47
 *  @date    2010/05/17
 *  @since   2010/05/17 (ICC Examin: 0.47)
 */
oyCMMapi4_s   iemn_api4_iemn_filter = {

  oyOBJECT_CMM_API4_S, /* oyStruct_s::type oyOBJECT_CMM_API4_S */
  0,0,0, /* unused oyStruct_s fileds; keep to zero */
  (oyCMMapi_s*) & iemn_api7_iemn_filter, /* oyCMMapi_s * next */
  
  iemnCMMInit, /* oyCMMInit_f */
  iemnCMMMessageFuncSet, /* oyCMMMessageFuncSet_f */

  /* registration */
  OY_ICMN_FILTER_REGISTRATION,

  {OYRANOS_VERSION_A,OYRANOS_VERSION_B,OYRANOS_VERSION_C},/**< version[3] */
  {0,4,0},                  /**< int32_t last_api_version[3] */
  0,   /* id_; keep empty */
  0,   /* api5_; keep empty */

  iemnFilterNode_MyContextToMem, /* oyCMMFilterNode_ContextToMem_f */
  0, /* oyCMMFilterNode_GetText_f        oyCMMFilterNode_GetText */
  {0}, /* char context_type[8] */

  &iemn_api4_ui_iemn_filter              /**< oyCMMui_s *ui */
};

/** @instance iemn_api7
 *  @brief    iemn oyCMMapi7_s implementation
 *
 *  My filter api7 object, to provide the processing routine and
 *  node connectors.
 *
 *  @version ICC Examin: 0.47
 *  @date    2010/05/17
 *  @since   2010/05/17 (ICC Examin: 0.47)
 */
oyCMMapi7_s   iemn_api7_iemn_filter = {

  oyOBJECT_CMM_API7_S, /* oyStruct_s::type oyOBJECT_CMM_API7_S */
  0,0,0, /* unused oyStruct_s fileds; keep to zero */
  (oyCMMapi_s*) 0, /* oyCMMapi_s * next */
  
  iemnCMMInit, /* oyCMMInit_f */
  iemnCMMMessageFuncSet, /* oyCMMMessageFuncSet_f */

  /* registration */
  OY_ICMN_FILTER_REGISTRATION,

  {OYRANOS_VERSION_A,OYRANOS_VERSION_B,OYRANOS_VERSION_C},/**< version[3] */
  {0,4,0},                  /**< int32_t module_api[3] */
  0,   /* id_; keep empty */
  0,   /* api5_; keep empty */

  iemnFilterPlug_MyFilterRun, /* oyCMMFilterPlug_Run_f */
  {0}, /* char data_type[8] */

  (oyConnector_s**) iemn_iemnFilter_connectorPlugs,   /* plugs */
  1,   /* plugs_n */
  0,   /* plugs_last_add */
  (oyConnector_s**) iemn_iemnFilter_connectorSockets,   /* sockets */
  1,   /* sockets_n */
  0    /* sockets_last_add */
};
#endif

/* OY_ICMN_FILTER_REGISTRATION ----------------------------------------------*/

char *       oyJsonPrint             ( oyjl_val            root );

#ifdef __cplusplus
}
#endif 

#ifdef USE_MAIN
int main( int argc OYJL_UNUSED, char ** argv)
{
  int size = 0;
  oy_debug = 1;
  oyjlDebugVariableSet(&oy_debug);
  char * cgatsT = oyjlReadFile( argv[1], &size );
  int error = !cgatsT;
  if(error) puts("need a CGATS file as sole argument\n");
  oyPointer_s*ptr = iemnParseCGATS( cgatsT );

  oyjl_val json = (oyjl_val) oyPointer_GetPointer(ptr);
  printf( "%s\n", oyJsonPrint(json) );

  return 0;
}
#endif
