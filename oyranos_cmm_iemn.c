/** @file oyranos_cmm_iemn.c
 *
 *  a filter for Oyranos 
 *
 *  @par Copyright:
 *            2010-2012 (C) Kai-Uwe Behrmann
 *
 *  @brief    ICC Examin viewer filter for Oyranos
 *  @internal
 *  @author   Kai-Uwe Behrmann <ku.b@gmx.de>
 *  @par License:
 *            new BSD <http://www.opensource.org/licenses/bsd-license.php>
 *  @since    2010/05/17
 */


#include "alpha/oyranos_alpha.h"
#include "alpha/oyranos_cmm.h"
#include "oyranos_definitions.h"

#include <math.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>


/* --- internal definitions --- */

/** The CMM_NICK consists of four bytes, which appear as well in the library name. This is important for Oyranos to identify the required filter struct name. */
#define CMM_NICK "iemn"
#define OY_ICMN_FILTER_REGISTRATION OY_TOP_INTERNAL OY_SLASH OY_DOMAIN_INTERNAL OY_SLASH OY_TYPE_STD OY_SLASH "icc_examin"

int iemnCMMWarnFunc( int code, const oyPointer context, const char * format, ... );
/** The msg_iemn function pointer to use for messaging. */
oyMessage_f msg_iemn = iemnCMMWarnFunc;

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
int                iemnCMMInit       ( )
{
  int error = 0;
  return error;
}



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
int iemnCMMWarnFunc( int code, const oyPointer cp, const char * format,...)
{
  char* text = (char*)calloc(sizeof(char), 4096);
  va_list list;
  const char * type_name = "";
  int id = -1;
  oyStruct_s * context = (oyStruct_s*) cp;

  if(context && oyOBJECT_NONE < context->type_)
  {
    type_name = oyStructTypeToText( context->type_ );
    id = oyObject_GetId( context->oy_ );
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
  msg_iemn = message_func;
  return 0;
}


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
                                       oyStruct_s        * context )
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
      return _("Me");
    else if(type == oyNAME_NAME)
      return _("Kai-Uwe Behrmann");
    else
      return _("My project; www: http://www.oyranos.org; support/email: ku.b@gmx.de; sources: http://www.oyranos.org/wiki/index.php?title=Oyranos/Download");
  } else if(strcmp(select, "copyright")==0)
  {
         if(type == oyNAME_NICK)
      return _("newBSD");
    else if(type == oyNAME_NAME)
      return _("Copyright (c) 2010 Kai-Uwe Behrmann; newBSD");
    else
      return _("new BSD license: http://www.opensource.org/licenses/bsd-license.php");
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


/** @instance iemn_cmm_module
 *  @brief    iemn module infos
 *
 *  This structure is dlopened by Oyranos. Its name has to consist of the
 *  following elements:
 *  - the four byte CMM_NICK plus
 *  - "_cmm_module"
 *  This string must be included in the the filters filename.
 *
 *  @version ICC Examin: 0.47
 *  @date    2010/05/17
 *  @since   2010/05/17 (ICC Examin: 0.47)
 */
oyCMMInfo_s iemn_cmm_module = {

  oyOBJECT_CMM_INFO_S, /**< ::type; the object type */
  0,0,0,               /**< static objects omit these fields */
  CMM_NICK,            /**< ::cmm; the four char filter id */
  (char*)"0.1.10",     /**< ::backend_version */
  iemnGetText,         /**< ::getText; UI texts */
  (char**)iemn_texts,  /**< ::texts; list of arguments to getText */
  OYRANOS_VERSION,     /**< ::oy_compatibility; last supported Oyranos CMM API*/

  /** ::api; The first filter api structure. */
  (oyCMMapi_s*) & iemn_api4_iemn_filter,

  /** ::icon; zero terminated list of a icon pyramid */
  {oyOBJECT_ICON_S, 0,0,0, 0,0,0, (char*)"oyranos_logo.png"}
};


/* OY_ICMN_FILTER_REGISTRATION ----------------------------------------------*/



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
    msg_iemn( oyMSG_WARN, (oyStruct_s*)node,
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
        msg_iemn(oyMSG_WARN, (oyStruct_s *) 0, _DBG_FORMAT_ "\n " "Could not allocate enough memory.", _DBG_ARGS_);
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


/* OY_ICMN_FILTER_REGISTRATION ----------------------------------------------*/

