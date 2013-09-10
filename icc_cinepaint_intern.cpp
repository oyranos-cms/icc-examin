/* 
 * ICC Examin plug-in for cinepaint.
 *
 * Copyright (C) 2004-2013 Kai-Uwe Behrmann <ku.b@gmx.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

/* 
 * copies an assigned ICC profil to $TMP_DIR and calls the dialog
 *
 * internal linking of iccexamin in the icc_examin_cp plug-in
 *  2006(?)
 * add writing of image samples out to an profile - name: plug_in_icc_watch
 *  2005-02-28
 * bugfixes
 * query only with found iccexamin executable
 *  2005-04-28
 * reorder functions, doxygenise, more variable checks
 *  2005-10-13
 * colour == farbkanaele
 *  2005-10-21
 * added imagename to written profile name
 *  2005-10-30
 */


/***   includes   ***/

#include "icc_utils.h"
#include "config.h"
#include "icc_helfer.h"
#include "icc_examin.h"
#include "icc_kette.h"
#include "icc_examin_version.h"

#include <oyProfiles_s.h>

#ifdef DEBUG 
#define DBG_PLUG_V(x)  DBG_PROG_V(x) 
#define DBG_PLUG_S(x)  DBG_PROG_S(x)
#else
#define DBG_PLUG_V(x) 
#define DBG_PLUG_S(x)
#endif




/*** local macros ***/


/** \addtogroup plug_in_api Externe Plug-in API

 *  @{
 */

#ifdef __cplusplus
extern "C" {
#endif
struct Bvals {
  int ein_Argument;
} bvals;
#ifdef __cplusplus
} /* extern "C" */
#endif
/** @} */

/** alle Variablen -> icc_examin_cp */
namespace icc_examin_cp {

/** @brief Typ der ben&ouml;tigten &Auml;nderung */
typedef int ChanModE;
#define LAYOUT_NEU(b)          ((b) << 0)     //!< +- Ebene
#define GET_LAYOUT(b)          (((b)>> 0)&1)  //!< +- Ebene
#define GEOMETRY_NEU(b)        ((b) << 1)     //!< Versatz, Skalierung
#define GET_GEOMETRY(b)        (((b)>> 1)&1)  //!< Versatz, Skalierung
#define BITDEPTH_NEU(b)        ((b) << 2)     //!< Farbtiefe
#define GET_BITDEPTH(b)        (((b)>> 2)&1)  //!< Farbtiefe
#define CHANNELS_NEU(b)        ((b) << 3)     //!< alpha Kanal, Rgb<->SW
#define GET_CHANNELS(b)        (((b)>> 3)&1)  //!< alpha Kanal, Rgb<->SW
#define TRANSFORM_NEU(b)       ((b) << 4)     //!< &Uuml;bertragung
#define GET_TRANSFORM(b)       (((b)>> 4)&1)  //!< &Uuml;bertragung
#define PROFIL_NEU(b)          ((b) << 5)     //!< Farbprofil
#define GET_PROFIL(b)          (((b)>> 5)&1)  //!< Farbprofil


/*** struct definitions ***/

/** alle Layer relevanten Informationen */
struct Channel {
  gint32        display_ID;      //!< view
  gint32        ID;              //!< drawable_ID
  GimpDrawable *drawable;
  GDrawableType/*GimpDrawableType*/ drawable_type;
  GimpPixelRgn  srcRgn;          //!< start image
  //GimpPixelRgn  dstRgn;          //!< shadow image
//  guchar       *pixels,          //!< raw pixel buffer for colors
//               *pixel;           //!< pointer to actual position in pixels
  int           precision,       //!< precision gimp_drawable_precision (layer->ID);
                samplesperpixel, //!< channels per pixel
                alpha;           //!< existence
  int           width, height;   //!< total dimensions
  int           offx, offy;      //!< layer offsets
  gint          sel_x1, sel_y1, sel_x2, sel_y2; //!< selection mask
  gint          sel_w, sel_h;    //!< dimension of mask
  icUInt32Number intent;         //!< CinePaint rendering intent
  icUInt32Number intent_proof;   //!< CinePaint simulation intent
  int           flags;           //!< CinePaint CMM switches
  ChanModE      status;          //!< tasks to do later
  Channel() {
    display_ID = 0;
    ID = 0;
    drawable = 0;
    drawable_type = RGB_IMAGE;
    //srcRgn = 0;
    precision = samplesperpixel = alpha = 0;
    width = height = 0;
    offx = offy = 0;
    sel_x1 = sel_y1 = sel_x2 = sel_y2 = 0;
    sel_w = sel_h = 0;
    intent = 0;
    intent_proof = 0;
    flags = 0;
    status = 256;
  }
  ~Channel() {
    if(drawable) gimp_drawable_detach(drawable);
  }
};
typedef Channel channel;

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
struct Ncl2Farbe {
  char name[32];
  icUInt16Number pcsfarbe[3]; // PCS Lab or XYZ
  icUInt16Number geraetefarbe[16];
};

/** ncl2 profiletag */
struct Ncl2 {
  char vendor_flag[4];
  icUInt32Number anzahl;
  icUInt32Number koord;
  char vorname[32];
  char nachname[32];
  Ncl2Farbe *farben;
};



/**   global variables   */

oyProfile_s * hl = NULL;       //!< CIE*Lab profile
oyProfile_s * hp = NULL;       //!< image profile
oyProfile_s * hs = NULL;       //!< simulation profile
oyConversion_s * transf = NULL;//!< device link
oyPixel_t format;              //!< colour layout
int farb_kanaele;              //!< colour channels as in image profile
double *colour = 0;            //!< measured colours : 0.0 -> 1.0 ==farb_kanaele
double *outbuf = 0;            //!< colours converted to Lab
char* colour_profile = 0;      //!< measurement colours profile (named colours)
char *image_profile = NULL;    //!< image profile
char *proof_profile = NULL;    //!< simulation profile
ICClist<double>       pcsfarbe;       //!< -> CIE*Lab 
ICClist<double>       geraetefarbe;   //!< image colours
ICClist<std::string>  name;           //!< colour names
std::string an,                //!< image profile name
            bn,                //!< colour profile
            pn,                //!< proof profile
            mn;                //!< monitor profile
size_t tag_size;               //!< ncl2 tag size
int x_num;                     //!< number of measurement points in x/y
int y_num;
int l = 30;                    //!< raster points in one dimension
double x_diff;                 //!< raster distance
double y_diff;
int x_start;                   //!< start point of measurement raster
int y_start;
int min_x, min_y, max_x, max_y;//!< intensity Max and Min
gint32 nlayers = 0;            //!< layer count
int    n_points;               //!< measurement point count
gint32 image_ID;               //!< CinePaint image number

bool farben_sind_gleich = true;//!< test switch
int  intent_alt = -12;         //!< test for changed intent
int  intent_alt_proof = -12;   //!< test for changed proof intent
int  flags_alt = -12;          //!< test for bpc ...
static bool erstes_mal = true; //!< identify programm thread
static bool farbe_pruefen_laeuft = false; //!< veto switch
}

using namespace icc_examin_cp;


/*** declaration of local functions ***/


/** \addtogroup internal_plug_in_api Interne Plug-in Funktions
 *  @{ */
static int      doExamin  (gint32   image_ID, CMSProfileType typ);
static int      doWatch   (gint32   image_ID);
static int      dialog_   (gint32   image_ID);
int             holeLayerInfo( channel & layer);
void            aufraeumen(channel *layer);
int             setzeRaster(channel*layer);
int             reserviereSpeicher(channel & layer);
const char*     dateiName(const char* name);
/** @} */
/** \addtogroup colour_api Colour Information API

 *  @{ */
static void     minMax    (gint32   image_ID, int & min_x, int & min_y,
                                    int & max_x, int & max_y );
static std::string holeFarbPunkt ( channel* layers, int & x, int & y,
                                unsigned char *buf, int &n,
                                int &colour_x);
static void     getColour (channel* layers, int n,
                           const unsigned char* data, double *colour,
                           const int & x, const int & y );
/** @} */

/** \addtogroup profil_api ICC Profile API

 *  The terms ncl2 and named colour are considered synonym.

 *  @{ */
void            schreibeProfil (icUInt32Number intent);
void            setzeRenderingIntent (char *header, icUInt32Number intent);
size_t          berechneTagGroesse (int farben_n, int farb_kanaele);
int             bearbeiteEingebetteteProfile( channel *layers );
void            transformAnlegen( channel & layer );
gint            drawableColourLayoutToOy  ( channel    & layer,
                                            oyProfile_s *  p );
/** @} */
/** \addtogroup thread_api Thread API
 *  @{ */
void*           waechter (void* zeiger);
void            pthreatFehler (int fehler);
/** @} */
/** \addtogroup io_api File API
 *  @{ */
void            schreibeDatei(const void *data, gint groesse, std::string name);
/** @} */


/*** functions ***/

/** @brief not used */
static int
dialog_ (gint32 image_ID)
{
  return true;
}

/** @brief start ICC Exmain GUI */
static void
startWithArgs( int argc, char **argv )
{
# if HAVE_PTHREAD_H
  registerThreadId( pthread_self(), THREAD_HAUPT );
# ifdef CWDEBUG
  Debug(myproject::debug::init_thread());	// This is a custom function defined
  						// in example-project/debug.cc.
# endif
# endif

    if(getenv("ICCEXAMIN_DEBUG") && atoi(getenv("ICCEXAMIN_DEBUG"))>0)
      icc_debug = atoi(getenv("ICCEXAMIN_DEBUG"));
    else
      icc_debug = 0;

    DBG_PROG_START

    setI18N( argv[0] );

    ICCexamin hauptprogramm;

    icc_examin = &hauptprogramm;
    icc_examin->scheme("gtk+");

    hauptprogramm.start(argc, argv);
}

/** @brief watch colour profile in ICC Examin

 *  needs ICC Examin

 *  @param image_ID			CinePaint image number
 *  @param typ				CinePaint profile typ (image/simulation)
 */
static gint32
doExamin (gint32 image_ID, CMSProfileType typ)
{
  DBG_PROG_S( "image: " << image_ID )

  char   *mem_profile=NULL;
  gint  size;

  if(gimp_image_has_icc_profile(image_ID, typ)) {
      mem_profile = gimp_image_get_icc_profile_by_mem(image_ID, &size, typ);
  } else {
    g_message ("%s",_("No profil assigned to image."));
    return -1;
  }


  DBG_PROG_S( (int*)mem_profile << " " << size )

  if (size && mem_profile) {
    char *ptr = gimp_image_get_filename(image_ID);
    const char *dateiname = dateiName(ptr);
    
    std::stringstream profil_temp_name;
    if(getenv("TMPDIR"))
      profil_temp_name << getenv("TMPDIR") << "/icc_examin_temp_" << dateiname << "_" << typ << ".icc";
    else
      profil_temp_name << "/tmp/icc_examin_temp_" << dateiname << "_" << typ << ".icc";
    std::string tname = profil_temp_name.str();
    schreibeDatei(mem_profile, size, tname.c_str());
    const char *args_c[2];

    args_c[0] = argv[0];
    args_c[1] = tname.c_str();

    startWithArgs(2, (char**)args_c);

    remove( tname.c_str() );
  } else
    g_message ("%s",_("Profil not written."));

  return image_ID;
}


/** @brief write rendering intent in profile header

 *  @param header		of profile
 *  @param intent		of profile
 */
void
setzeRenderingIntent (char *header, icUInt32Number intent)
{
  icProfile *p = (icProfile *)header;
  p->header.renderingIntent = icValue( intent );
}


/** @brief preparation for ncl2 - compute size

 *  @param farben_n		count of colours
 *  @param farb_kanaele	number of colour channels
 */
size_t
berechneTagGroesse (int farben_n, int farb_kanaele)
{
  size_t groesse = 8 + 76 +
                   (38 + farb_kanaele * sizeof(icUInt16Number)) * farben_n;
  return groesse;
}

/** @brief write ncl2 colours in a block

 *  @param pcsfarbe		colours in PCS colour space
 *  @param geraetefarbe	        colours in device colour space
 *  @param farb_kanaele	        count of colour channels
 *  @param vorname		sur name for all colours
 *  @param name			individual colour name
 *  @param nachname		common family name for all colours
 */
char*
schreibeNcl2Tag              ( ICClist<double>       pcsfarbe,
                               ICClist<double>       geraetefarbe,
                               int                       farb_kanaele,
                               const char*               vorname,
                               ICClist<std::string>  name,
                               const char*               nachname)
{
  int    farben_n = pcsfarbe.size() / 3;
  size_t groesse  = berechneTagGroesse( farben_n, farb_kanaele );

  DBG_PROG_S( "farb_kanaele: " << farb_kanaele <<" farben_n: "<< farben_n )

  char* tag_block = (char*) new char [groesse];

  DBG_PROG_S( "tag_block: " << (int*)tag_block <<" groesse: "<< groesse )

  for(size_t i = 0; i < groesse; ++i)
    tag_block[i] = 0;

  // 0: count of colours
  // 1...n: CIE*Lab colour values
  // n = 3 * FarbAnzahl

  Ncl2 *ncl2 = (Ncl2*) &tag_block[8];

  ncl2->anzahl = icValue((icUInt32Number)farben_n);
  ncl2->koord  = icValue((icUInt32Number)farb_kanaele);
  if(vorname && strlen(vorname) < 32)
    sprintf(ncl2->vorname, "%s",vorname);
  if(nachname && strlen(nachname) < 32)
    sprintf(ncl2->nachname, "%s",nachname);

  DBG_PROG_S( farben_n <<" "<< pcsfarbe.size() )

  for (int i = 0; i < farben_n; ++i)
  {
    Ncl2Farbe *f = (Ncl2Farbe*) ((char*)ncl2 + 76 + // base size of Ncl2
                   (i * (38 +                 // base size of Ncl2Farbe
                         farb_kanaele         // number of device colours
                         * sizeof(icUInt16Number))));//Ncl2Farbe::geraetefarbe
    f->pcsfarbe[0] = icValue((icUInt16Number)(pcsfarbe[3*i+0]*65280.0));
    f->pcsfarbe[1] = icValue((icUInt16Number)(pcsfarbe[3*i+1]*65535.0));
    f->pcsfarbe[2] = icValue((icUInt16Number)(pcsfarbe[3*i+2]*65535.0));
    for(int j=0; j < farb_kanaele; ++j)
      f->geraetefarbe[j] = icValue((icUInt16Number)
                                   (geraetefarbe[farb_kanaele*i+j]*65535.0));
    // TODO mark pointer here
    if (name.size() && name[i].size() < 32)
      sprintf(f->name, "%s", name[i].c_str());

    #ifdef DEBUG_
    if( 10 < i && i < 20 )
    DBG_PLG_S(  icValue(f->pcsfarbe[0]) << "," << pcsfarbe[3*i+0] <<" "<<
            icValue(f->pcsfarbe[1]) << "," << pcsfarbe[3*i+1] <<" "<<
            f->pcsfarbe[2] << " " << pcsfarbe[3*i+2] <<" "<<
            f->geraetefarbe[0] << " " <<
            f->geraetefarbe[1] << " " <<
            f->geraetefarbe[2] );
    #endif

  }

  icTag ic_tag;

  ic_tag.size = icValue ((icUInt32Number)groesse);
  ic_tag.offset = 0;
  memcpy(&ic_tag.sig, "ncl2", 4);

  char sig[] = "ncl2";
  memcpy (&tag_block[0], &sig, 4);

  return tag_block;
}


/** @brief create device link

 *  @param layer		informations
 */
void
transformAnlegen( channel & layer )
{
    oyOptions_s * options = 0;
    char num[12];
    drawableColourLayoutToOy( layer, hp );

    DBG_PROG_S( transf )

    oyConversion_Release ( &transf );

    DBG_PROG_S( transf <<" "<< layer.intent )

    sprintf(num,"%d", layer.intent);
    oyOptions_SetFromText( &options, OY_DEFAULT_RENDERING_INTENT, num,
                           OY_CREATE_NEW );
    if(layer.intent_proof == 3)
      sprintf(num,"%d", 1);
    else
      sprintf(num,"%d", 0);
    oyOptions_SetFromText( &options, OY_DEFAULT_RENDERING_INTENT_PROOF,
                           num, OY_CREATE_NEW );
    if(layer.flags & 0x2000) /* BPC */
      oyOptions_SetFromText( &options, OY_DEFAULT_RENDERING_BPC,
                             "1", OY_CREATE_NEW );

    if(layer.flags & 0x1000) /* gamut warning */
      oyOptions_SetFromText( &options, OY_DEFAULT_RENDERING_GAMUT_WARNING,
                             "1", OY_CREATE_NEW );
    if(layer.flags & 0x4000) /* proofing */
    {
      oyProfiles_s * proofs = oyProfiles_New(0);
      oyProfile_s * proof = oyProfile_Copy( hs, 0 );
      oyOptions_SetFromText( &options, OY_DEFAULT_PROOF_SOFT,
                             "1", OY_CREATE_NEW );
      oyProfiles_MoveIn( proofs, &proof, -1 );
      oyOptions_MoveInStruct( &options,
                              OY_TOP_SHARED OY_SLASH OY_DOMAIN_STD OY_SLASH OY_TYPE_STD "/icc/profiles_simulation",
                              (oyStruct_s**) &proofs, OY_CREATE_NEW );

    }

    transf = oyConversion_CreateBasicPixelsFromBuffers(
                                           hp, colour, oyDOUBLE,
                                           hl, outbuf, oyDOUBLE,
                                           options, n_points );

    if(!transf)
      WARN_S( "no conversion context created: " <<
                  oyOptions_GetText(options, oyNAME_NAME));
    oyOptions_Release( &options );

    DBG_PLUG_S( transf <<" "<< hp <<" "<< hl <<" "<< hs <<" channels: "<<
           oyToChannels_m(format) << " depth "<< oyToDataType_m(format) <<" i"<<
           layer.intent <<" ip"<< layer.intent_proof <<" f"<< layer.flags );

    /*DBG_PLUG_S( out[0]<<" "<<out[1]<<" "<<out[2]<<"  "<<
           out2[0]<<" "<<out2[1]<<" "<<out2[2] )*/
}

/** @brief search for changing colours

 *  as there is no information about changing in image,
 *  the image is polled for changes, and a new ncl2 will be generated.

 *  @param zeiger		layer stack
 */
bool
vergleicheFarben(void* zeiger)
{
  farbe_pruefen_laeuft = true;
  channel* layer = 0;
  if(zeiger)
    layer = (channel*) zeiger;

  DBG_PLUG_S( "layer "<< (int*)layer )

  // colour memory - static is perhaps dangerous?
  static ICClist<double> vorherige_farben;

  DBG_PLUG_S( "zeiger " << (int*)zeiger )

  if(!layer) {
    farbe_pruefen_laeuft = false;
    return true;
  }

  pcsfarbe.clear();       // -> unchanged colours: CIE*Lab 
  geraetefarbe.clear();   // image colours
  name.clear();           // colour names

  // ask the image in raster distance
  guchar buf[128]; // point cache
  int colour_x = 0; // counter
  int x_punkt = 0 , y_punkt = 0;
  int n = 0;

  layer->status = 0;

  holeLayerInfo( *layer );

  // save of the embeded image profile -> a
  if (bearbeiteEingebetteteProfile( layer ))
    return -1;

  if( GET_LAYOUT(layer->status)) {
  }
  if( GET_GEOMETRY(layer->status)) {
    setzeRaster( layer );
	DBG_PROG_S("raster")
  }

  if( GET_GEOMETRY(layer->status) ||
      GET_PROFIL(layer->status) ||
      GET_CHANNELS(layer->status) ||
      GET_BITDEPTH(layer->status)) {
    reserviereSpeicher( *layer );
	DBG_PROG_S("reserved")
  }

  // create device link
  if( GET_TRANSFORM(layer->status) ||
      GET_PROFIL(layer->status) ||
      GET_CHANNELS(layer->status) ||
      GET_BITDEPTH(layer->status) ) {
    transformAnlegen( *layer );
  }
  DBG_PLUG_S( "nlayers: " << nlayers )
 
  name.resize(x_num*y_num+2);
 
  if(nlayers)
  {
    for( int x = 0; x < x_num; ++x )
      for( int y = 0; y < y_num; ++y )
      {
        x_punkt = (int)(x_start + x*x_diff);
        y_punkt = (int)(y_start + y*y_diff);

        name[colour_x] = holeFarbPunkt(layer, x_punkt, y_punkt,
                      buf, n, colour_x);
        ++colour_x;
      }
  }

    // Maximalwerte
  int x = MAX(MIN(min_x, layer->sel_w + layer->sel_x1), layer->sel_x1);
  int y = MAX(MIN(min_y, layer->sel_h + layer->sel_y1), layer->sel_y1);
  name[colour_x] = holeFarbPunkt(layer, x, y, buf, n, colour_x);
  ++colour_x;
  x = MAX(MIN(max_x, layer->sel_w + layer->sel_x1), layer->sel_x1);
  y = MAX(MIN(max_y, layer->sel_h + layer->sel_y1), layer->sel_y1);
  name[colour_x] = holeFarbPunkt(layer, x, y, buf, n, colour_x);

  {
    // Vergleich der vorherigen Auslese
    if( GET_TRANSFORM(layer->status) ||
        GET_PROFIL(layer->status) )
      farben_sind_gleich = false;
    else
      farben_sind_gleich = true;
    int gleichviele = true;
    if( (int)vorherige_farben.size() != n_points*farb_kanaele )
    {
      gleichviele = false;
      farben_sind_gleich = false;
      vorherige_farben.clear();
      DBG_PROG_S( "n_points: " << n_points <<
             " vorherige_farben.size(): " << vorherige_farben.size() )
    }

    { // aktuelle Farben merken
      for(int i = 0; i < n_points*farb_kanaele; ++i)
      {
        //DBG_PROG_S( i << " " << n_points*farb_kanaele )
        if(gleichviele)
          if(colour[i] != vorherige_farben[i])
            farben_sind_gleich = false;
        if(i < (int)vorherige_farben.size())
          vorherige_farben[i] = colour[i];
        else
          vorherige_farben.push_back(colour[i]);
      }
    }

    DBG_PLUG_S( colour_x << " " << n_points )
 
      // Wir koennen das weitere auslassen
    if(farben_sind_gleich &&
       !layer->status)
    {
      farbe_pruefen_laeuft = false;
      DBG_PLUG_S("colours are equal")
      return false;
    } else
      DBG_PLUG_S("colours are not equal");
  }

# ifdef DEBUG_
  DBG_PROG_S( farb_kanaele <<" "<< oyToChannels_m(format) <<" "<< oyToDataType_m(format) )
  cout <<
          oyToSwapColourChannels_m(format) <<" "<<
          oyToFlavor_m(format) <<" "<<
          oyToPlanar_m(format) <<" "<<
          oyToByteswap_m(format) <<" "<<
          oyToColourOffset_m(format) <<" "; DBG
# endif

  oyConversion_RunPixels( transf, NULL );

  // Berechnung Auswerten ...
  for(int i = 0; i < n_points; ++i)
  {
    pcsfarbe.push_back( outbuf[3*i+0]);
    pcsfarbe.push_back( outbuf[3*i+1]);
    pcsfarbe.push_back( outbuf[3*i+2]);

    //DBG_PROG_S( pcsfarbe[farb_kanaele*i+0] << "," << pcsfarbe[farb_kanaele*i+1] << "," << pcsfarbe[farb_kanaele*i+2] )

    for(int j = 0; j < farb_kanaele; ++j)
    {
      geraetefarbe.push_back( colour[farb_kanaele*i+j]/100.0 );

      //DBG_PROG_S( colour[farb_kanaele*i+j] )
    }
  }

  // ... und das ncl2 Profil vervollstaendigen
  memcpy(colour_profile, data, 320);

    // Profilegroesse
  char zahl[4];
  *((icUInt32Number*)zahl) = icValue(236 + (icUInt32Number)tag_size);
  memcpy(colour_profile, zahl, 4);
    // Abschnittsgroesse
  *((icUInt32Number*)zahl) = icValue((icUInt32Number)tag_size);
  memcpy(&colour_profile[164], zahl, 4);
    // Farbraum
  *((icUInt32Number*)zahl) = icValue((icUInt32Number)
                        oyProfile_GetSignature( hp, oySIGNATURE_COLOR_SPACE ));
  memcpy(&colour_profile[16], zahl, 4);

  DBG_PROG_S( (int*)image_profile << " " << tag_size )

  schreibeProfil( layer->intent );

  farbe_pruefen_laeuft = false;

  return false;
}

#include <unistd.h>
#include <pthread.h>

/** @brief Bild beobachten

 *  und gleichzeitig nach ICC Examin schauen

 *  @param zeiger		Ebenenstapel
 */
void*
waechter (void* zeiger)
{

  bool bin_erste = false;
  if(erstes_mal)
  {
    bin_erste = true;
    erstes_mal = false;
  }

  channel* layer = 0;
  if(zeiger)
    layer = (channel*) zeiger;

  DBG_PROG_S( (int*)layer )
  
  int fehler = false;

  DBG_PROG_S( "bin_erste: " << bin_erste )

  static bool freilauf = true;

  // observe image
  if(bin_erste)
  {
    int sl = 1000000;
    while(!fehler && freilauf)
    {
      double rz = icc_examin_ns::zeitSekunden();

      if(!fehler)
      fehler = vergleicheFarben( layer );

      rz = icc_examin_ns::zeitSekunden() - rz;
      if(farben_sind_gleich) {
        sl = max(sl,500000);
        usleep(sl*2);
      } else {
        sl = (int)(rz*1000000.0)*4;
        sl = max(sl,50000);
        usleep(sl);
        DBG_PROG_S( "rz: " << rz*1000000 << " sl " << sl )
      }
    }
    DBG_PROG_S( "bin_erste: " << bin_erste )
    sleep(10);
    DBG_PROG_S( "bin_erste: " << bin_erste )
  }

  // start ICC Examin
  if(!bin_erste)
  {
    const char *args_c[5];
    int argc_c = 4;

    args_c[0] = argv[0];
    args_c[1] = bn.c_str();
    args_c[2] = an.c_str();
    args_c[3] = pn.c_str();
    args_c[4] = mn.c_str();


    // get the main screens net color spec ICC profile as supported by CinePaint
#ifdef HAVE_X
    Display * display = XOpenDisplay(NULL);;
    int screen = DefaultScreen( display );
    Window root = RootWindow( display, screen );
    Atom atom = XInternAtom( display, "_ICC_DEVICE_PROFILE", False ),
         a;
    int actual_format_return;
    unsigned long nitems_return=0, bytes_after_return=0;
    unsigned char* prop_return=0;
    XGetWindowProperty( display, root, atom, 0, INT_MAX, 0, XA_CARDINAL, &a,
                     &actual_format_return, &nitems_return, &bytes_after_return,
                     &prop_return );
    if(nitems_return)
    {
      schreibeDatei( prop_return, nitems_return, mn.c_str() );
      ++argc_c;
    }
#endif

    startWithArgs(argc_c, (char**)args_c);

    DBG_PROG_S( "bin_erste: " << bin_erste )
    freilauf = false;
    while(farbe_pruefen_laeuft)
    {
      DBG_PROG_S( "bin_erste: " << bin_erste )
      icc_examin_ns::sleep( 0.1 );
      DBG_PROG_S( "bin_erste: " << bin_erste )
    }
    freilauf = true;
    DBG_PROG_S( "bin_erste: " << bin_erste )
  }

  if(freilauf)
  {
    freilauf = false;
    aufraeumen( layer );
    DBG_PROG_S( "bin_erste: " << bin_erste )
  } else
    DBG_PROG_S( "bin_erste: " << bin_erste );

  return layer;
}

#ifndef EAGAIN
#define EAGAIN 11
#endif
#ifndef PTHREAD_THREADS_MAX
#define PTHREAD_THREADS_MAX 16384
#endif

/** @brief Programmzweigfehler entschl&uuml;sseln */
void
pthreatFehler (int fehler)
{
  if( fehler == EAGAIN)
  {
    WARN_S( "observer thread not started Error: "  << fehler );
  } else
  if( fehler == 64/*PTHREAD_THREADS_MAX -- linux*/ )
  {
    WARN_S( "too many observer threads Error: " << fehler );
  } else
  if( fehler != 0 )
  {
    WARN_S( "unknown error at start of observer thread: " << fehler );
  }
}

/** @brief clean layers

 *  @param layer		stack
 */
void
aufraeumen(channel *layer)
{
  {
    while(farbe_pruefen_laeuft) {
      DBG_PROG_S( "farbe_pruefen_laeuft " << farbe_pruefen_laeuft )
      sleep(1);
    }
    // clean
    remove(an.c_str());
    remove(bn.c_str());
    remove(pn.c_str());
    remove(mn.c_str());
    if(colour_profile) delete [] colour_profile;
    if(image_profile) free( image_profile);
    if(proof_profile) free (proof_profile);
    image_profile = 0;
    proof_profile = 0;
    if(colour) delete [] colour;
    if(outbuf) delete [] outbuf;
    oyConversion_Release( &transf );
    oyProfile_Release( &hl);
    oyProfile_Release( &hp);
    if(hs) oyProfile_Release( &hs);
  }
}


/** @brief memory block -> file

 *  @param data		memory block
 *  @param groesse	size
 *  @param name		file name
 */
void
schreibeDatei(const void *data, gint groesse, std::string name)
{
  DBG_PROG_S( (int*)data <<": "<< groesse <<" "<< name )
  if(data && groesse && name.size())
  {
    std::ofstream f;
    f.clear();
    f.open ( name.c_str(),  std::ios::out );
    if(f.good())
    {
      f.write ( (char*)data, groesse );
        DBG_MEM_S( "Profile %s written " << name.c_str() )
    }
    f.close();
    DBG_PROG_S("Profile written")
  } else
    g_print ("Profile %s not written.", name.c_str());
}

/** @brief layer layout -> lcms colour layout
 
 *  following cariables are to handle:\n
    T_COLORSPACE(s) \n
    T_SWAPFIRST(s) \n
    T_FLAVOR(s) \n
    T_PLANAR(p) \n
    T_ENDIAN16(e) \n
    T_DOSWAP(e) \n
    T_EXTRA(e) \n
    oyToChannels_m(c) \n
    oyToDataType_m(b) - always 0, as we convert to float.\n


 *  @param layer	layers
 *  @param p		lcms colour profile
 */
gint
drawableColourLayoutToOy (  channel    & layer,
                            oyProfile_s *  p )
{
  //GDrawableType/*GimpDrawableType*/ drawable_type;
  gint success = GIMP_PDB_SUCCESS;
  oyDATATYPE_e data_type = oyUINT8;

  format = 0;

  DBG_PROG_S( farb_kanaele <<" "<< format ) 


    switch (layer.precision) {
      case 1:         // uint8
        data_type = oyUINT8;
        break;
      case 2:         // uint16
        data_type = oyUINT16;
        break;
      case 3:         // f32
        data_type = oyFLOAT;
        break;
      case 4:         // f16 OpenEXR
        data_type = oyFLOAT;
        break;
      default:
        g_print ("!!! Precision = %d not allowed!\n", layer.precision);
        return GIMP_PDB_CALLING_ERROR;
    }

    if(farb_kanaele < layer.samplesperpixel)
      ;//format |= EXTRA_SH(layer.samplesperpixel - farb_kanaele);

    format |= oyChannels_m(farb_kanaele) | oyDataType_m(data_type);

  return success;
}

/** @brief handle embeded profile

 *  a Mix from ichecking and processing of profile informations

 *  @param layer	layer
 */
int
bearbeiteEingebetteteProfile( channel *layer )
{
  // has the profile name changed?
  if (!gimp_image_has_icc_profile(image_ID, ICC_IMAGE_PROFILE)) {
    g_message ("%s",_("No profil assigned to image."));
    return 1;
  }

  layer->display_ID = gimp_display_active();
  layer->intent = gimp_display_get_cms_intent (layer->display_ID/*image_ID*/, ICC_IMAGE_PROFILE);
  layer->intent_proof = gimp_display_get_cms_intent (layer->display_ID/*image_ID*/, ICC_PROOF_PROFILE);
  layer->flags = gimp_display_get_cms_flags (layer->display_ID);
  DBG_PLUG_S( layer->intent <<" "<< intent_alt )

  char* profil_name = gimp_image_get_icc_profile_description(image_ID,
                                                             ICC_IMAGE_PROFILE);
  char* pprofil_name = (char*) calloc (sizeof(char*),1);
  //if( gimp_image_has_icc_profile( image_ID, ICC_PROOF_PROFILE ) &&
  //    gimp_display_is_colormanaged( layer->display_ID, ICC_PROOF_PROFILE ) )
  {
    pprofil_name = gimp_image_get_icc_profile_description(image_ID, 
                                                          ICC_PROOF_PROFILE);
  }
  static std::string old_profil_name;
  static std::string old_pprofil_name;

  if(profil_name)
  {
    ;//DBG_PROG_S( image_ID <<": "<< profil_name <<" "<< old_profil_name )
  }
  if(pprofil_name)
  {
    DBG_PLUG_S( image_ID <<": "<< pprofil_name <<" "<< old_pprofil_name )
  }

  // Test for changes in profile
  if( strcmp(old_profil_name.c_str(), profil_name) == 0/* &&
      (pprofil_name ?
       (strcmp(old_pprofil_name.c_str(), pprofil_name) == 0) : old_pprofil_name.size()) */)
  {
    if(profil_name) old_profil_name = profil_name; else old_profil_name = "";
    if(pprofil_name) old_pprofil_name = pprofil_name; else old_pprofil_name ="";
  } else
    layer->status |= PROFIL_NEU(1);

  //DBG_PROG_S( "hp = " << hp )
  // save of embeded profiles
  if(strcmp(old_profil_name.c_str(), profil_name) != 0 ||
     (int)layer->intent != intent_alt)
  {
    gint size=0;
    image_profile = gimp_image_get_icc_profile_by_mem( image_ID, &size,
                                                     ICC_IMAGE_PROFILE);
    setzeRenderingIntent ( image_profile, layer->intent );
    schreibeDatei( image_profile, size, an );
//sleep(10);
    // calculate -> CIE*Lab prepare
    if(hl)oyProfile_Release( &hl);
    if(hp)oyProfile_Release( &hp);
    hl   = oyProfile_FromStd( oyEDITING_LAB, 0 );
    hp   = oyProfile_FromMem( size, image_profile, 0,0 );
    farb_kanaele = oyProfile_GetChannelsCount( hp );
    if(farb_kanaele > layer->samplesperpixel) {
      g_message ("%s",_("Wrong profil assigned to image."));
      return 1;
    }
    layer->status |= PROFIL_NEU(1);
    //DBG_PROG_S( "hp = " << hp << " status:"<< layer->status )
  }

#define cmsFLAGS_SOFTPROOFING 0x4000

  // save of embeded profiles
  int        new_proofing = 
                 gimp_display_get_cms_flags (image_ID) & cmsFLAGS_SOFTPROOFING;
  static int old_proofing = 0;

  if( (pprofil_name ?
       (strcmp(old_pprofil_name.c_str(), pprofil_name) != 0 ||
        old_proofing != new_proofing ) : 0) ||
      (int)layer->intent_proof != intent_alt_proof )
  {
    gint psize=0;
    if ( gimp_image_has_icc_profile (image_ID, ICC_PROOF_PROFILE) &&
         gimp_display_get_cms_flags (image_ID) & cmsFLAGS_SOFTPROOFING)
    {
      proof_profile = gimp_image_get_icc_profile_by_mem ( image_ID, &psize,
                                                          ICC_PROOF_PROFILE);
      if(hs) oyProfile_Release( &hs);
      hs   = oyProfile_FromMem( psize, proof_profile, 0,0 );
      setzeRenderingIntent ( proof_profile, layer->intent_proof );
      schreibeDatei( proof_profile, psize, pn );
    } else {
      DBG_MEM_S( "wrote 1 byte\n" )
      schreibeDatei( "", 1, pn );
    }
    layer->status |= PROFIL_NEU(1);
  }

  old_proofing = new_proofing;

  //DBG_PROG_S( old_pprofil_name.size() <<" status:"<< layer->status )
  if(profil_name) old_profil_name = profil_name; else old_profil_name = "";
  if(pprofil_name) old_pprofil_name = pprofil_name; else old_pprofil_name ="";

  if(profil_name) free(profil_name);
  if(pprofil_name) free(pprofil_name);

  // check
  if((int)layer->intent != intent_alt) {
    intent_alt = layer->intent;
    layer->status |= TRANSFORM_NEU(1);
  }

  if((int)layer->intent_proof != intent_alt_proof) {
    intent_alt_proof = layer->intent_proof;
    layer->status |= TRANSFORM_NEU(1);
  }

  // adapt switch
  if((int)layer->flags != flags_alt) {
    flags_alt = layer->flags;
    layer->status |= TRANSFORM_NEU(1);
  }

  return 0;
}

/** @brief watch colours in 3D

 *  needs ICC Examin

 *  @param image_ID_		CinePaint image number
 */
static int
doWatch (gint32 image_ID_)
{
  image_ID = image_ID_;

  DBG_PROG_S( "Bild: " << image_ID )

  std::stringstream profil_temp_name;

  if(getenv("TMPDIR")) {
    profil_temp_name << getenv("TMPDIR") << "/cinepaint_" << time(0) ;
    DBG_PROG_S( getenv("TMPDIR") )
  } else {
    profil_temp_name << "/tmp/cinepaint_" << time(0) ;
  }

  DBG_PROG_S( profil_temp_name.str() )

  an = profil_temp_name.str(); an.append("_image.icc");
  bn = profil_temp_name.str(); bn.append("_pixel.icc");
  pn = profil_temp_name.str(); pn.append("_proof.icc");
  mn = profil_temp_name.str(); mn.append("_moni0.icc");

  channel *layer = 0;

  DBG_PROG_S( "image_ID: " << image_ID )

  layer = (channel*) new channel [1];
  nlayers = 1;
  layer->display_ID = gimp_display_active();

  DBG_PROG_S( "layer: " << (int*)layer ) 

  // determine Min/Max
  minMax( image_ID, min_x, min_y, max_x, max_y );

  // measure colours and repeat in certain intervals
  //
  // We have two threads.
  // The first one is starting a while condition for observation
  // of image. The main thread watches ICC Examin.

  pthread_t p_t;
  int fehler = false;
  if(layer)
  {
    fehler = vergleicheFarben( layer );
    if(!fehler)
    {
      fehler = pthread_create(&p_t, NULL, &waechter, (void *)layer);
      if(fehler) pthreatFehler (fehler);
    }
  }

  // start iccexamin and wait for its end 
  if(!fehler) {
    waechter(layer);
  }

  DBG_PROG_S( "end of " <<__func__ )

  return image_ID;
}

/** @brief takes a colour point
 *  @param layer	layer
 *  @param x_punkt	koordinate
 *  @param y_punkt	koordinate
 *  @param buf		generic memory for saving the colour
 *  @param n		layer number
 *  @param colour_x	tells the position to write in the colour in colour[]
 */
static std::string
holeFarbPunkt (channel* layer, int & x_punkt, int & y_punkt,
               unsigned char *buf, int & n, int &colour_x)
{
    char txt[80] = {0};

        #ifdef DEBUG_
        DBG_CINE_S( n <<": " << x_punkt <<","<< y_punkt << " " )
        #endif
        gimp_pixel_rgn_get_pixel( &layer->srcRgn, buf, x_punkt, y_punkt);

        getColour ( layer, n, buf, &colour[colour_x*farb_kanaele],
                    x_punkt, y_punkt);

        sprintf( txt, "%d,%d", x_punkt, y_punkt );
        #ifdef DEBUG_
        cout << colour_x <<": "<<
                colour[colour_x*farb_kanaele+0] <<" "<< 
                colour[colour_x*farb_kanaele+1] <<" "<<
                colour[colour_x*farb_kanaele+2] <<" ";
        if(farb_kanaele == 4)
        cout << colour[colour_x*farb_kanaele+3] <<" ";
        DBG
        #endif
    return std::string(txt);
}

/** @brief write ncl2

 *  @param intent		rendering intent
 */
void
schreibeProfil (icUInt32Number intent)
{
  char* tag = schreibeNcl2Tag (pcsfarbe, geraetefarbe, farb_kanaele,"",name,"");

  DBG_PROG_S( (int*)tag <<" "<< tag_size )

  memcpy (&colour_profile[236], tag, tag_size);
  if(tag)    delete [] tag;

  // set intent
  setzeRenderingIntent ( colour_profile, intent );

  // save the colour profile
  if(colour_profile && tag_size)
    schreibeDatei( colour_profile, 236 + tag_size , bn );
}


/** @brief actualise layer informations

 *  One layer is active at time as usual.

 *  holeLayerInfo takes the information in channel*.\n
 *  Later it can be decided whether all informationen are valid.

 *  @param layer		stack
 *  @return 			changed
 */
int
holeLayerInfo    (channel & layer)
{
  gint   var, var1, var2, var3, var4;
  GDrawableType/*GimpDrawableType*/ drawable_type;

  layer.display_ID = gimp_display_active();
  image_ID = gimp_display_get_image_id( layer.display_ID);
  gint32 drawable_ID = gimp_image_get_active_layer (image_ID);
  if(drawable_ID < 0)
    g_message("%s",_("No active drawable found."));

  // run
  {
      // ID
      if(layer.ID != drawable_ID) {
        layer.status |= LAYOUT_NEU(1)|GEOMETRY_NEU(1);
        layer.ID = drawable_ID;
        gimp_drawable_detach(layer.drawable);
        layer.drawable = gimp_drawable_get (layer.ID);
      }
      // drawable_type
      drawable_type = gimp_drawable_type (layer.ID);
      if(layer.drawable_type != drawable_type) {
        layer.status |= GEOMETRY_NEU(1);
        layer.drawable_type = drawable_type;
      }
      // dimension
      if(layer.width != (int)layer.drawable->width ||
         layer.height != (int)layer.drawable->height) {
        layer.status |= GEOMETRY_NEU(1);
        layer.width  = layer.drawable->width;
        layer.height = layer.drawable->height;
      }
      // selection or visible area
      gimp_drawable_mask_bounds(layer.drawable->id, &var1,&var2, &var3,&var4);
      if(layer.sel_x1 != var1 ||
         layer.sel_x2 != var3 ||
         layer.sel_y1 != var2 ||
         layer.sel_y2 != var4 ) {
        layer.status |= GEOMETRY_NEU(1);
        layer.sel_x1 = var1;
        layer.sel_x2 = var3;
        layer.sel_y1 = var2;
        layer.sel_y2 = var4;
        layer.sel_w = layer.sel_x2-layer.sel_x1;
        layer.sel_h = layer.sel_y2-layer.sel_y1;
      }
      // offset
      gimp_drawable_offsets( layer.ID, &var1, &var2);
      if(layer.offx != var1 ||
         layer.offy != var2) {
        layer.status |= GEOMETRY_NEU(1);
        layer.offx = var1;
        layer.offy = var2;
      }
      // channels
      var = gimp_drawable_num_channels(layer.ID);
      if(layer.samplesperpixel != var) {
        layer.status |= CHANNELS_NEU(1);
        layer.samplesperpixel = var;
      }
      // alpha
      var = gimp_drawable_has_alpha( layer.ID );
      if(layer.alpha != var) {
        layer.status |= CHANNELS_NEU(1);
        layer.alpha = var;
      }
      // precision
      var = gimp_drawable_precision (layer.ID);
      if(layer.precision != var) {
        layer.status |= BITDEPTH_NEU(1);
        layer.precision = var;
      }
      // pixel_rgn
      if(layer.status) {
        DBG_PROG_S("new region")
        gimp_drawable_detach(layer.drawable);
        layer.drawable = gimp_drawable_get (layer.ID);
        gimp_pixel_rgn_init (&(layer.srcRgn), layer.drawable,
                             layer.sel_x1, layer.sel_y1,
                             layer.sel_w,layer.sel_h, FALSE,FALSE);
      }
  }

  return 0;
}

/** @brief Raster parameter

 *  and look at ICC Examin

 *  @param layer		stack
 *  @todo consider selection
 */
int
setzeRaster( channel *layer )
{

  if(layer->sel_w > layer->sel_h)
  {
    x_num = l;
    y_num = (int)((double)x_num * (double)layer->sel_h/(double)layer->sel_w+.5);
  } else {
    y_num = l;
    x_num = (int)((double)y_num * (double)layer->sel_w/(double)layer->sel_h+.5);
  } DBG_PROG_S( x_num <<" "<< y_num )
    DBG_PROG_S( "w/h: " << (double)layer->sel_w/(double)layer->sel_h )
    DBG_PROG_S( "sel_w|sel_h "<<layer->sel_w<<"|"<<layer->sel_h ) 

  if(x_num > (layer->sel_w)) { x_num = layer->sel_w; }
  if(y_num > (layer->sel_h)) { y_num = layer->sel_h; }

    DBG_PROG_S( x_num <<" "<< y_num )

  x_diff = (double)layer->sel_w / (double)x_num;
  y_diff = (double)layer->sel_h / (double)y_num;
  x_start = (int)((double)x_num/layer->sel_w / 2) + layer->sel_x1;
  y_start = (int)((double)y_num/layer->sel_h / 2) + layer->sel_y1;

  DBG_PROG_S( "dist:  "<< x_diff <<","<< y_diff << 
              "start: "<< x_start <<","<< y_start << " "
              "num:   "<< x_num <<","<< y_num )
  return 0;
}

/** @brief reserve colour memory
 */
int
reserviereSpeicher( channel & layer )
{
  n_points = x_num * y_num + 2;

  DBG_MEM_S( n_points )

  outbuf = (double*) new double [n_points*3];
  if(!outbuf) return 1;

  // continue preparation
  tag_size  = berechneTagGroesse( n_points, farb_kanaele );
  if(colour_profile) delete [] colour_profile;
  // tag_size * 3 should be enough for changing channel numbers
  colour_profile = (char*) new char [320 + tag_size * 3];

  DBG_NUM_S( "320 + tag_size: " << 320 + tag_size )

  if(colour) delete [] colour;
  colour = (double*) new double [n_points * (farb_kanaele == 1 ? 4 : 4)];
  if(!colour) return 1;
  for(int i = 0; i < n_points*farb_kanaele; ++i)
    colour[i] = 0.0;

  layer.status |= TRANSFORM_NEU(1);
  return 0;
}

/** @brief search for extremes

    @todo too thick, should better use available functions
 */
static void
minMax(gint32 image_ID, int & min_x, int & min_y,
                      int & max_x, int & max_y )
{
  DBG_PROG_S( "image_ID: " << image_ID )

  gint32  *layers;
  gint32   nlayers;

  layers = gimp_image_get_layers (image_ID, &nlayers);

  float max = -100000.0, max_color[4], //max_x, max_y,
        min = 100000.0, min_color[4]; //, min_x, min_y;
  ShortsFloat u;

  for(int i = 0; i < 4; ++i) {
    max_color[i] = 0.0;
    min_color[i] = 0.0;
  }

  if(nlayers)
  {

    gint32 drawable_ID = gimp_image_get_active_layer (image_ID);

    DBG_PROG_S( "drawable: " << drawable_ID )

    int tile_height = gimp_tile_height ();


    GimpPixelRgn pixel_rgn;
    GimpDrawable *drawable = gimp_drawable_get (drawable_ID);
    GDrawableType drawable_type = gimp_drawable_type (drawable_ID);
    int channels = gimp_drawable_num_channels(drawable_ID);
    int colors = channels;
    if (channels == 2 || channels == 4) colors--;

    DBG_PROG_S( "colors: " << colors )

    int bpp = gimp_drawable_bpp(drawable_ID);
    unsigned char* data = (unsigned char*) calloc (sizeof (char), drawable->width * channels * bpp * tile_height);

    gimp_pixel_rgn_init (&pixel_rgn, drawable, 0, 0,
                         (gint32)drawable->width, (gint32)drawable->height,
                         FALSE, FALSE);

    gimp_tile_cache_size(drawable->width/tile_height+1);

    int yend = 0;
    int ystart, y,x, c, colori, pos;
    for (ystart = 0; ystart < (int)drawable->height; ystart = yend + 1) {
      yend = ystart + tile_height - 1;
      yend = MIN (yend, (int)drawable->height);

      DBG_PROG_S( ": " <<  ystart <<" - " << yend )

      gimp_pixel_rgn_get_rect (&pixel_rgn, data, 0, ystart, drawable->width, yend - ystart);

      DBG_PROG_S( "ystart: " << ystart )

      for (y = ystart; y < yend; y++) {
        for (x = 0; x < (int)drawable->width; x++) {
          for (c = 0; c < colors; c++) {
            pos = ( (y-ystart) * drawable->width + x) * channels;
            switch (drawable_type)
            {
              case RGB_IMAGE:
              case RGBA_IMAGE:
              case GRAY_IMAGE:
              case GRAYA_IMAGE:
                     if (max < data[pos + c]
                         ) {
                       for (colori = 0; colori < colors; colori++)
                         max_color[colori] = data[pos + colori];
                       max = data[pos + c];
                       max_x = x;
                       max_y = y;
                     }
                     if (min > data[pos + c]
                        ) {
                       for (colori = 0; colori < colors; colori++)
                         min_color[colori] = data[pos + colori];
                       min = data[pos + c];
                       min_x = x;
                       min_y = y;
                     }
                   break;
              case INDEXED_IMAGE: break;
              case INDEXEDA_IMAGE: break;
              case U16_RGB_IMAGE:
              case U16_RGBA_IMAGE:
              case U16_GRAY_IMAGE:
              case U16_GRAYA_IMAGE:
              case BFP_RGB_IMAGE:
              case BFP_RGBA_IMAGE:
              case BFP_GRAY_IMAGE:
              case BFP_GRAYA_IMAGE:
                     if (max < ((guint16*)data)[pos + c]
                      ) {
                       for (colori = 0; colori < colors; colori++)
                         max_color[colori] = ((guint16*)data)[pos + colori];
                       max = ((guint16*)data)[pos + c];
                       max_x = x;
                       max_y = y;
                     }
                     if (min > ((guint16*)data)[pos + c]
                      ) {
                       for (colori = 0; colori < colors; colori++)
                         min_color[colori] = ((guint16*)data)[pos + colori];
                       min = ((guint16*)data)[pos + c];
                       min_x = x;
                       min_y = y;
                     }
                   break;
              case U16_INDEXED_IMAGE: break;
              case U16_INDEXEDA_IMAGE: break;
              case FLOAT_RGB_IMAGE:
              case FLOAT_RGBA_IMAGE:
              case FLOAT_GRAY_IMAGE:
              case FLOAT_GRAYA_IMAGE:
                     if (max < ((float*)data)[pos + c]) {
                       for (colori = 0; colori < colors; colori++)
                         max_color[colori] = ((float*)data)[pos + colori];
                       max = ((float*)data)[pos + c];
                       max_x = x;
                       max_y = y;
                     }
                     if (min > ((float*)data)[pos + c]) {
                       for (colori = 0; colori < colors; colori++)
                         min_color[colori] = ((float*)data)[pos + colori];
                       min = ((float*)data)[pos + c];
                       min_x = x;
                       min_y = y;
                     }
                   break;
              case FLOAT16_RGB_IMAGE: break;
              case FLOAT16_RGBA_IMAGE: break;
              case FLOAT16_GRAY_IMAGE: break;
              case FLOAT16_GRAYA_IMAGE: break;
                     if (max < FLT(((guint16*)data)[pos + c], u)
                      ) {
                       for (colori = 0; colori < colors; colori++)
                         max_color[colori] = FLT(((guint16*)data)[pos + colori], u);
                       max = FLT(((guint16*)data)[pos + c], u);
                       max_x = x;
                       max_y = y;
                     }
                     if (min > FLT(((guint16*)data)[pos + c], u)
                      ) {
                       for (colori = 0; colori < colors; colori++)
                         min_color[colori] = FLT(((guint16*)data)[pos + colori], u);
                       min = FLT(((guint16*)data)[pos + c], u);
                       min_x = x;
                       min_y = y;
                     }
                   break;
              default: g_message ("%s",_("What kind of image is this?")); break;
            }
          }
        }
      }
    }

    free (data);
    gimp_drawable_flush(drawable);
    gimp_drawable_detach (drawable);

  }
  if(layers) free(layers);

  if(icc_debug > 1) 
  {
    printf("max(%d,%d) = ",
                (int)max_x,(int)max_y);
    for (int c=0; c < 4; c++)
         printf("%f ", max_color[c]);
    printf ("\n");

    printf("\nmin(%d,%d) = ",
                (int)min_x,(int)min_y); 
    for (int c=0; c < 4; c++)
         printf("%f ", min_color[c]);
    printf ("\n");
  }
}


/** @brief transformiert einen Farbspeicher in eine gemeinsame Farbtiefe
 *  @param layers		Ebenenstapel
 *  @param i			Ebenennummer
 *  @param [in] data 	generischer Speicher zu Ablegen der Farbe
 *  @param [out] colour Farbspeicher
 *  @param x			Koordinate
 *  @param y			Koordinate
 */
static void
getColour (channel* layers, int i,
           const unsigned char* data, double *colour,
           const int & x, const int & y )
{
  if (x > layers[i].sel_x2-1
   || x < layers[i].sel_x1
   || y > layers[i].sel_y2-1
   || y < layers[i].sel_y1)
  {
    colour[0] = 0.0;
    colour[1] = 0.0;
    colour[2] = 0.0;
    if(farb_kanaele == 4)
    colour[3] = 0.0;
    return;
  }

          for (int c = 0; c < farb_kanaele; ++c)
          {
            switch (layers[i].drawable_type)
            {
              case RGB_IMAGE:
              case RGBA_IMAGE:
              case GRAY_IMAGE:
              case GRAYA_IMAGE:
                   colour[c] = data[c] / 255.0;
                   break;
              case INDEXED_IMAGE: break;
              case INDEXEDA_IMAGE: break;
              case U16_RGB_IMAGE:
              case U16_RGBA_IMAGE:
              case U16_GRAY_IMAGE:
              case U16_GRAYA_IMAGE:
              case BFP_RGB_IMAGE:
              case BFP_RGBA_IMAGE:
              case BFP_GRAY_IMAGE:
              case BFP_GRAYA_IMAGE:
                   colour[c] = ((guint16*)data)[c] / 65535.0;
                   break;
              case U16_INDEXED_IMAGE: break;
              case U16_INDEXEDA_IMAGE: break;
              case FLOAT_RGB_IMAGE:
              case FLOAT_RGBA_IMAGE:
              case FLOAT_GRAY_IMAGE:
              case FLOAT_GRAYA_IMAGE:
                   colour[c] = ((float*)data)[c];
                   break;
              case FLOAT16_RGB_IMAGE:
              case FLOAT16_RGBA_IMAGE:
              case FLOAT16_GRAY_IMAGE:
              case FLOAT16_GRAYA_IMAGE:
                   ShortsFloat u;
                   colour[c] = (double) FLT( ((guint16*)data)[c],u);
                   break;
              default: cout << (_("What kind of image is this?\n")); break;
            }
          }
}



