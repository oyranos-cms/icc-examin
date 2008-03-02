/* 
 * ICC Examin plug-in for cinepaint.
 *
 * Copyright (C) 2004-2006 Kai-Uwe Behrmann <ku.b@gmx.de>
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
 * copies an assigned ICC profil to $TMP_DIR and call iccexamin
 *
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

#define PLUG_IN_NAME          "plug_in_icc_examin"
#define PLUG_IN_NAME2         "plug_in_icc_examin_proof"
#define PLUG_IN_NAME3         "plug_in_icc_watch"
#define PLUG_IN_NAME4         "plug_in_icc_watch2"
#define PLUG_IN_BRIEF         "shows ICC image profile of an image"
#define PLUG_IN_BRIEF2        "shows ICC proof profile of an image"
#define PLUG_IN_BRIEF3        "shows image colours compared to ICC profile"
#define PLUG_IN_DESCRIPTION   "Loads an assigned ICC profil from image to ICC Examin."
#define PLUG_IN_DESCRIPTION2  "Loads an assigned ICC proof profil from image to ICC Examin."
#define PLUG_IN_DESCRIPTION3  "Shows some colours of the image in a ICC Examin including profile gamut"
#define PLUG_IN_VERSION       version()
#define PLUG_IN_AUTHOR        "Kai-Uwe Behrmann <ku.b@gmx.de>"
#define PLUG_IN_COPYRIGHT     "2004-2007 Kai-Uwe Behrmann"

/***   includes   ***/

#include <string>
#include <sstream>
#include <iostream>
#include <fstream>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>

#include "icc_utils.h"
#include "config.h"
#include "icc_helfer.h"
#include "icc_examin.h"
#include "icc_kette.h"
#include "icc_examin_version.h"

#ifdef DEBUG_ 
#define DBG_PLUG_V(x)  DBG_V(x) 
#define DBG_PLUG_S(x)  DBG_S(x)
#else
#define DBG_PLUG_V(x) 
#define DBG_PLUG_S(x)
#endif

char* version() { static char t[80];
                  sprintf(t, "%s - %s", ICC_EXAMIN_V, ICC_EXAMIN_D); return t; }


using namespace std;


extern "C" {
#include <libgimp/gimp.h>
#include <libgimp/stdplugins-intl.h>
#include <lcms.h>
#include <icc34.h>
}


/*** local macros ***/


/** \addtogroup plug_in_api Externe Plug-in API

 *  @{
 */

#ifdef __cplusplus
extern "C" {
#endif

static void   query      (void);
static void   run        (char    *name,
			  int      nparams,
			  GParam  *param,
			  int     *nreturn_vals,
			  GParam **return_vals);

GPlugInInfo PLUG_IN_INFO =
{
  NULL,    /* init_proc */
  NULL,    /* quit_proc */
  query,   /* query_proc */
  run,     /* run_proc */
};
static int n_args_;
static int n_return_vals_;

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
  gint32        display_ID;      //!< die Ansicht
  gint32        ID;              //!< drawable_ID
  GimpDrawable *drawable;
  GDrawableType/*GimpDrawableType*/ drawable_type;
  GimpPixelRgn  srcRgn;          //!< Ausgangsbild
  //GimpPixelRgn  dstRgn;          //!< Schattenbild
//  guchar       *pixels,          //!< raw pixel buffer for colors
//               *pixel;           //!< pointer to actual position in pixels
  int           precision,       //!< Pr&auml;zission gimp_drawable_precision (layer->ID);
                samplesperpixel, //!< Kan&auml;le pro Pixel
                alpha;           //!< Existenz
  int           width, height;   //!< Gesamtbreite und -hoehe
  int           offx, offy;      //!< Ebenenversaetze
  gint          sel_x1, sel_y1, sel_x2, sel_y2; //!< Auswahlmaske
  gint          sel_w, sel_h;    //!< Dimension der Auswahlmaske
  icUInt32Number intent;         //!< CinePaint &Uuml;bertragungsart
  icUInt32Number intent_proof;   //!< CinePaint Simulations &Uuml;bertragungsart
  int           flags;           //!< CinePaint CMM Schalter
  ChanModE      status;          //!< Aufgaben f&uuml;r sp&auml;ter
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

/** ncl2 Profilrumpf */
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
    67,80,0,0,-32,51,12,8,
    10,0,0,0,40,-40,-1,-65,
    -12,-17,37,64,-12,-17,37,64,
    -64,108,1,64,8,-40,-1,-65,
    -97,-51,13,64,-32,51,12,8,
    -128,48,11,8,40,-40,-1,-65,
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

/** einzelner ncl2 Schnipsel - eine Farbe */
struct Ncl2Farbe {
  char name[32];
  icUInt16Number pcsfarbe[3]; // PCS Lab oder XYZ
  icUInt16Number geraetefarbe[16];
};

/** der ncl2 Profiltag */
struct Ncl2 {
  char vendor_flag[4];
  icUInt32Number anzahl;
  icUInt32Number koord;
  char vorname[32];
  char nachname[32];
  Ncl2Farbe *farben;
};



/**   global variables   */

cmsHPROFILE hl;                //!< lcms CIE*Lab Profil
cmsHPROFILE hp;                //!< lcms Bildprofil
cmsHPROFILE hs;                //!< lcms Simulationsprofil
cmsHTRANSFORM transf = 0;      //!< lcms &Uuml;bertragungstabelle
long format;                   //!< lcms Farblayout
int farb_kanaele;              //!< Farbkanaele wie im Bildprofil angegeben
double *colour = 0;            //!< gemessene Farben : 0.0 -> 1.0 ==farb_kanaele
double *outbuf = 0;            //!< nach Lab umgewandelte Farben
char* colour_profile = 0;      //!< Messfarbprofile (Schmuckfarben)
char *image_profile = NULL;    //!< Bildprofil
char *proof_profile = NULL;    //!< Simulationsprofil
std::vector<double>       pcsfarbe;       //!< -> ungerechnete Farben: CIE*Lab 
std::vector<double>       geraetefarbe;   //!< Bildfarben
std::vector<std::string>  name;           //!< Farbnamen
std::string an,                //!< Bildprofil
            bn,                //!< Farben
            pn,                //!< Proofprofil
            tn;                //!< Dateinamen und Befehlszeile
size_t tag_size;               //!< ncl2 Abschnittsgroesse
int x_num;                     //!< Anzahl Messpunkte in x/y Richtung
int y_num;
int l = 30;                    //!< Rasterpunkte in einer Dimension
double x_diff;                 //!< Raster Abstaende
double y_diff;
int x_start;                   //!< Startpunkte des Messrasters
int y_start;
int min_x, min_y, max_x, max_y;//!< Intensitaets Maxima und Minima
gint32 nlayers = 0;            //!< beteiligte Ebenen
int    n_points;               //!< Anzahl Messpunkte
gint32 image_ID;               //!< CinePaint Bildnummer

bool farben_sind_gleich = true;//!< Pr&uuml;fschalter
int  intent_alt = -12;         //!< Test auf ver&auml;nderte &Uuml;bertragung
int  intent_alt_proof = -12;   //!< Test auf ver&auml;nderte &Uuml;bertragung
int  flags_alt = -12;          //!< Test auf Schwarzpunktkompensation ...
static bool erstes_mal = true; //!< Programmzweig identifizieren
static bool farbe_pruefen_laeuft = false; //!< Vetoschalter
}

using namespace icc_examin_cp;


/*** declaration of local functions ***/


/** \addtogroup internal_plug_in_api Interne Plug-in Funktionen
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
/** \addtogroup colour_api Farbinformationen API

 *  @{ */
static void     minMax    (gint32   image_ID, int & min_x, int & min_y,
                                    int & max_x, int & max_y );
static void     holeFarbPunkt ( channel* layers, int & x, int & y,
                                unsigned char *buf, int &n,
                                int &colour_x);
static void     getColour (channel* layers, int n,
                           const unsigned char* data, double *colour,
                           const int & x, const int & y );
/** @} */

/** \addtogroup profil_api ICC Profil API

 *  Die Begriffe ncl2 and Einzelfarben werden hier synonym verwendet.

 *  @{ */
void            schreibeProfil (icUInt32Number intent);
void            setzeRenderingIntent (char *header, icUInt32Number intent);
size_t          berechneTagGroesse (int farben_n, int farb_kanaele);
int             bearbeiteEingebetteteProfile( channel *layers );
void            transformAnlegen( channel & layer );
gint            drawableColourLayoutToLcms( channel    & layer,
                                            cmsHPROFILE  p );
/** @} */
/** \addtogroup thread_api Thread API
 *  @{ */
void*           waechter (void* zeiger);
void            pthreatFehler (int fehler);
/** @} */
/** \addtogroup io_api Datei API
 *  @{ */
void            schreibeDatei(const void *data, gint groesse, std::string name);
/** @} */


/*** functions ***/

MAIN()

/** @brief plug-in Anmeldung */
static void
query ()
{
  static GParamDef args[] =
  {
    { PARAM_INT32, "run_mode", "Interactive, non-interactive" },
    { PARAM_IMAGE, "image", "Input Image" },
    { PARAM_DRAWABLE, "drawable", "Input Drawable" },
  };
  static GParamDef return_vals[] =
  {
    { PARAM_IMAGE, "image", "Output Image" },
  };
  n_args_ = sizeof (args) / sizeof (args[0]);
  n_return_vals_ = sizeof (return_vals) / sizeof (return_vals[0]);

  {
    gimp_install_procedure (PLUG_IN_NAME,
                            PLUG_IN_BRIEF,
                            PLUG_IN_DESCRIPTION,
                            PLUG_IN_AUTHOR,
                            PLUG_IN_COPYRIGHT,
                            PLUG_IN_VERSION,
                            "<Image>/Image/ICC Profile Information/Image Profile...",
                            "*",
                            PROC_PLUG_IN,
                            n_args_, n_return_vals_,
                            args, return_vals);

    gimp_install_procedure (PLUG_IN_NAME2,
                            PLUG_IN_BRIEF2,
                            PLUG_IN_DESCRIPTION2,
                            PLUG_IN_AUTHOR,
                            PLUG_IN_COPYRIGHT,
                            PLUG_IN_VERSION,
                            "<Image>/Image/ICC Profile Information/Proof Profile...",
                            "*",
                            PROC_PLUG_IN,
                            n_args_, n_return_vals_,
                            args, return_vals);

    gimp_install_procedure (PLUG_IN_NAME3,
                            PLUG_IN_BRIEF3,
                            PLUG_IN_DESCRIPTION3,
                            PLUG_IN_AUTHOR,
                            PLUG_IN_COPYRIGHT,
                            PLUG_IN_VERSION,
                            "<Image>/Image/Watch Colours 3D...",
                            "*",
                            PROC_PLUG_IN,
                            n_args_, n_return_vals_,
                            args, return_vals);

    gimp_install_procedure (PLUG_IN_NAME4,
                            PLUG_IN_BRIEF3,
                            PLUG_IN_DESCRIPTION3,
                            PLUG_IN_AUTHOR,
                            PLUG_IN_COPYRIGHT,
                            PLUG_IN_VERSION,
                            "<Image>/Image/Watch Colours 3D.2...",
                            "*",
                            PROC_PLUG_IN,
                            n_args_, n_return_vals_,
                            args, return_vals);
  }
  _("Image Profile...");
  _("Proof Profile...");
  _("Watch Colours 3D...");
  _("Watch Colours 3D.2...");
}

/** @brief plug-in Start

 *  @param name					Plug-in Name
 *  @param[in] nparams			Argumente Anzahl
 *  @param[in] param			Argumente
 *  @param[out] nreturn_vals	R&uuml;ckgabewerte Anzahl
 *  @param[out] return_vals		R&uuml;ckgabewerte
 */
static void
run (char    *name,
     int      nparams,
     GParam  *param,
     int     *nreturn_vals,
     GParam **return_vals)
{
  GParam values[12];
  gint32 image_ID;
  GRunModeType run_mode;
  GStatusType status = GIMP_PDB_SUCCESS;

  run_mode = (GRunModeType) param[0].data.d_int32;

  *nreturn_vals = n_return_vals_;
  *return_vals = values;
  values[0].type = PARAM_STATUS;
  values[0].data.d_status = STATUS_CALLING_ERROR;

  if (strcmp (name, PLUG_IN_NAME) == 0 ||
      strcmp (name, PLUG_IN_NAME2) == 0 )
  {
    CMSProfileType typ = ICC_IMAGE_PROFILE;
    if(strcmp (name, PLUG_IN_NAME2) == 0)
      typ = ICC_PROOF_PROFILE;
    switch (run_mode)
    {
    case RUN_INTERACTIVE:
      // gespeicherte Daten abholen
      gimp_get_data (PLUG_IN_NAME, &bvals);

      // einen Dialog oeffnen
      if (! dialog_ (param[1].data.d_image))
        return;
      image_ID = doExamin (param[1].data.d_image, typ);
      break;

    case RUN_NONINTERACTIVE:
      // das Ganze automatisch
      if (nparams != n_args_) 
        status = STATUS_CALLING_ERROR;
      if (status == STATUS_SUCCESS)
	  {
        bvals.ein_Argument = (int)param[2].data.d_drawable;
      }
      image_ID = doExamin (param[1].data.d_image, typ);

      *nreturn_vals = n_return_vals_ + 1;
      values[0].data.d_status = STATUS_SUCCESS;
      values[1].type = PARAM_IMAGE;
      values[1].data.d_image = image_ID;
      break;

    case RUN_WITH_LAST_VALS:
      // gespeicherte Daten abholen
      gimp_get_data (PLUG_IN_NAME, &bvals);
      break;

    default:
      break;
    }
  } else
  if (strcmp (name, PLUG_IN_NAME3) == 0 ||
      strcmp (name, PLUG_IN_NAME4) == 0)
  {
  if (strcmp (name, PLUG_IN_NAME4) == 0)
    l = 120;
  switch (run_mode)
    {
    case RUN_INTERACTIVE:
      // gespeicherte Daten abholen
      gimp_get_data (PLUG_IN_NAME3, &bvals);

      // einen Dialog oeffnen
      if (! dialog_ (param[1].data.d_image))
        return;
      image_ID = doWatch (param[1].data.d_image);
      break;

    case RUN_NONINTERACTIVE:
      // das Ganze automatisch
      if (nparams != n_args_) 
	status = STATUS_CALLING_ERROR;
      if (status == STATUS_SUCCESS)
	{
	  bvals.ein_Argument = (int)param[2].data.d_drawable;
	}
      image_ID = doWatch (param[1].data.d_image);

      *nreturn_vals = n_return_vals_ + 1;
      values[0].data.d_status = STATUS_SUCCESS;
      values[1].type = PARAM_IMAGE;
      values[1].data.d_image = image_ID;
      break;

    case RUN_WITH_LAST_VALS:
      // gespeicherte Daten abholen
      gimp_get_data (PLUG_IN_NAME, &bvals);
      break;

    default:
      break;
    }
  }
  values[0].data.d_status = status;
}

/** @brief ungenutzt */
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

    hauptprogramm.start(argc, argv);
}

/** @brief Farbprofil in ICC Examin ansehen

 *  ben&ouml;tigt ICC Examin

 *  @param image_ID			CinePaint Bildnummer
 *  @param typ				CinePaint Profiltyp (Bild/Simulation)
 */
static gint32
doExamin (gint32 image_ID, CMSProfileType typ)
{
  DBG_PROG_S( "Bild: " << image_ID )

  char   *mem_profile=NULL;
  gint  size;

  if(gimp_image_has_icc_profile(image_ID, typ)) {
      mem_profile = gimp_image_get_icc_profile_by_mem(image_ID, &size, typ);
  } else {
    g_message (_("No profil assigned to image."));
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
    std::string tn = "export PATH=$PATH:/opt/local/bin; iccexamin '";
    tn += tname;
    tn += "'";

#if 0
    system (tn.c_str());
#else
    const char *args_c[2];

    args_c[0] = argv[0];
    args_c[1] = tname.c_str();

    startWithArgs(2, (char**)args_c);
#endif

    remove( tname.c_str() );
  } else
    g_message (_("Profil not written."));

  return image_ID;
}


/** @brief &Uuml;bertragungsart in Profilkopf schreiben

 *  @param header		Profilkopf
 *  @param intent		&Uuml;bertragungsart
 */
void
setzeRenderingIntent (char *header, icUInt32Number intent)
{
  icProfile *p = (icProfile *)header;
  p->header.renderingIntent = icValue( intent );
}


/** @brief Vorbereitung f&uuml;r ncl2 - Gr&ouml;&szlig;e Berechnen

 *  @param farben_n		Anzahl der Farben
 *  @param farb_kanaele	Anzahl der Farbekan&auml;le
 */
size_t
berechneTagGroesse (int farben_n, int farb_kanaele)
{
  size_t groesse = 8 + 76 +
                   (38 + farb_kanaele * sizeof(icUInt16Number)) * farben_n;
  return groesse;
}

/** @brief Einzelfarben in einen ncl2 Block schreiben

 *  @param pcsfarbe		Farbe im PCS Farbraum
 *  @param geraetefarbe	Farbe im Ger&auml;te Farbraum
 *  @param farb_kanaele	Anzahl der Farbekan&auml;le
 *  @param vorname		f&uuml;r alle Farben g&uuml;ltiger Vorname
 *  @param name			Namen der einzelnen Farben
 *  @param nachname		f&uuml;r alle Farben g&uuml;ltiger Nachname
 */
char*
schreibeNcl2Tag              ( std::vector<double>       pcsfarbe,
                               std::vector<double>       geraetefarbe,
                               int                       farb_kanaele,
                               const char*               vorname,
                               std::vector<std::string>  name,
                               const char*               nachname)
{
  int    farben_n = pcsfarbe.size() / 3;
  size_t groesse  = berechneTagGroesse( farben_n, farb_kanaele );

  DBG_PROG_S( "farb_kanaele: " << farb_kanaele <<" farben_n: "<< farben_n )

  char* tag_block = (char*) new char [groesse];

  DBG_PROG_S( "tag_block: " << (int*)tag_block <<" groesse: "<< groesse )

  for(size_t i = 0; i < groesse; ++i)
    tag_block[i] = 0;

  // 0: Anzahl Farben
  // 1...n: CIE*Lab Farbwerte
  // n = 3 * FarbAnzahl

  Ncl2 *ncl2 = (Ncl2*) &tag_block[8];

  ncl2->anzahl = icValue((icUInt32Number)farben_n);
  ncl2->koord  = icValue((icUInt32Number)farb_kanaele);
  if(vorname && strlen(vorname) < 32)
    sprintf(ncl2->vorname, vorname);
  if(nachname && strlen(nachname) < 32)
    sprintf(ncl2->nachname, nachname);

  DBG_PROG_S( farben_n <<" "<< pcsfarbe.size() )

  for (int i = 0; i < farben_n; ++i)
  {
    Ncl2Farbe *f = (Ncl2Farbe*) ((char*)ncl2 + 76 + // Basisgroesse von Ncl2
                   (i * (38 +                 // Basisgroesse von Ncl2Farbe
                         farb_kanaele         // Anzahl Geraetefarben
                         * sizeof(icUInt16Number))));//Ncl2Farbe::geraetefarbe
    f->pcsfarbe[0] = icValue((icUInt16Number)(pcsfarbe[3*i+0]*65280.0));
    f->pcsfarbe[1] = icValue((icUInt16Number)(pcsfarbe[3*i+1]*65535.0));
    f->pcsfarbe[2] = icValue((icUInt16Number)(pcsfarbe[3*i+2]*65535.0));
    for(int j=0; j < farb_kanaele; ++j)
      f->geraetefarbe[j] = icValue((icUInt16Number)
                                   (geraetefarbe[farb_kanaele*i+j]*65535.0));
    // TODO Zeiger hier markieren
    if (name.size() && name[i].size() < 32)
      sprintf(f->name, name[i].c_str());

    #ifdef DEBUG_
    if( 10 < i && i < 20 )
    DBG_S(  icValue(f->pcsfarbe[0]) << "," << pcsfarbe[3*i+0] <<" "<<
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


/** @brief &Uuml;bertragungstabelle anlegen

 *  @param layer		Ebeneninformationen
 */
void
transformAnlegen( channel & layer )
{
    drawableColourLayoutToLcms( layer, hp );

    DBG_PROG_S( transf )
    double in[6] = {0.5,0.5,0.5,0.5,0.5,0.5}, out[3]={0,0,0},
           out2[3]={0,0,0};
    if(transf)
    {
      cmsDoTransform( transf, in, out, 1);
      cmsDeleteTransform (transf);
    }
    transf = 0;
    DBG_PROG_S( transf <<" "<< layer.intent )

    transf = cmsCreateProofingTransform (hp, format,
                                 hl, TYPE_Lab_DBL,
                                 hs,
                                 layer.intent, layer.intent_proof,
                                 layer.flags | cmsFLAGS_NOTPRECALC);
    DBG_S( transf <<" "<< hp <<" "<< hl <<" "<< hs <<" channels: "<<
           T_CHANNELS(format) << " depth "<< T_BYTES(format) <<" i"<<
           layer.intent <<" ip"<< layer.intent_proof <<" f"<< layer.flags );

    cmsDoTransform( transf, in, out2, 1);
    DBG_S( out[0]<<" "<<out[1]<<" "<<out[2]<<"  "<<
           out2[0]<<" "<<out2[1]<<" "<<out2[2] )
}

/** @brief sucht nach sich ver&auml;ndernden Farben

 *  da keine R&uuml;ckmeldung &uuml;ber ein sich veraenderndes Bild m&ouml;glich ist
 *  wird best&auml;ndig auf Ver&auml;nderungen gepr&uuml;ft, und ein
 *  neues ncl2 erzeugt.

 *  @param zeiger		Ebenenstapel
 */
bool
vergleicheFarben(void* zeiger)
{
  farbe_pruefen_laeuft = true;
  channel* layer = 0;
  if(zeiger)
    layer = (channel*) zeiger;

  DBG_PLUG_S( "layer "<< (int*)layer )

  // Farbgedaechtnis - static ist vielleicht gefaehrlich?
  static std::vector<double> vorherige_farben;

  DBG_PLUG_S( "zeiger " << (int*)zeiger )

  if(!layer) {
    farbe_pruefen_laeuft = false;
    return true;
  }

  pcsfarbe.clear();       // -> ungerechnete Farben: CIE*Lab 
  geraetefarbe.clear();   // Bildfarben
  name.clear();           // Farbnamen

  // Das Bild befragen im Gitterraster
  guchar buf[128]; // Punktspeicher
  int colour_x = 0; // Zaehler
  int x_punkt = 0 , y_punkt = 0;
  int n = 0;

  layer->status = 0;

  holeLayerInfo( *layer );

  // Speichern des eingebetteten Bildprofiles -> a
  if (bearbeiteEingebetteteProfile( layer ))
    return -1;

  if( GET_LAYOUT(layer->status)) {
  }
  if( GET_GEOMETRY(layer->status)) {
    setzeRaster( layer );
	DBG_PROG_S("gerastert")
  }

  // &Uuml;bertragungstabelle anlegen
  if( GET_TRANSFORM(layer->status) ||
      GET_PROFIL(layer->status) ||
      GET_CHANNELS(layer->status) ||
      GET_BITDEPTH(layer->status) ) {
    transformAnlegen( *layer );
  }
  if( GET_GEOMETRY(layer->status) ||
      GET_PROFIL(layer->status) ||
      GET_CHANNELS(layer->status) ||
      GET_BITDEPTH(layer->status)) {
    reserviereSpeicher( *layer );
	DBG_PROG_S("reserviert")
  }

  DBG_PLUG_S( "nlayers: " << nlayers )
  if(nlayers)
    for( int x = 0; x < x_num; ++x )
      for( int y = 0; y < y_num; ++y )
      {
        x_punkt = (int)(x_start + x*x_diff);
        y_punkt = (int)(y_start + y*y_diff);

        holeFarbPunkt(layer, x_punkt, y_punkt,
                      buf, n, colour_x);
        ++colour_x;
      }

    // Maximalwerte
  holeFarbPunkt(layer, min_x, min_y, buf, n, colour_x);
  ++colour_x;
  holeFarbPunkt(layer, max_x, max_y, buf, n, colour_x);

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
  DBG_PROG_S( farb_kanaele <<" "<< T_CHANNELS(format) <<" "<< T_BYTES(format) )
  cout <<
          T_COLORSPACE(format) <<" "<<
          T_SWAPFIRST(format) <<" "<<
          T_FLAVOR(format) <<" "<<
          T_PLANAR(format) <<" "<<
          T_ENDIAN16(format) <<" "<<
          T_DOSWAP(format) <<" "<<
          T_EXTRA(format) <<" "<<
          T_CHANNELS(format) <<" "<<
          T_BYTES(format) <<" "; DBG
# endif

  cmsDoTransform( transf, colour, outbuf, n_points);


  // Berechnung Auswerten ...
  for(int i = 0; i < n_points; ++i)
  {
    pcsfarbe.push_back( outbuf[3*i+0]/100.0);
    pcsfarbe.push_back((outbuf[3*i+1]+128.0)/255.0);
    pcsfarbe.push_back((outbuf[3*i+2]+128.0)/255.0);

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

  // Bild beobachten
  if(bin_erste)
  {
    int sl = 1000000;
    while(!fehler && freilauf)
    {
      double rz = (double)clock()/(double)CLOCKS_PER_SEC;

      if(!fehler)
      fehler = vergleicheFarben( layer );

      rz = (double)clock()/(double)CLOCKS_PER_SEC - rz;
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

  // ICC Examin starten
  if(!bin_erste)
  {
    tn = "iccexamin ";
    tn += bn;  // die Farben
    tn += " ";
    tn += an;  // das Bildprofil
    tn += " '";
    tn += pn;  // das Proofprofil
    tn += "'";

    DBG_PROG_S( tn )

#if 0
    system (tn.c_str());
#else
    const char *args_c[4];

    args_c[0] = argv[0];
    args_c[1] = bn.c_str();
    args_c[2] = an.c_str();
    args_c[3] = pn.c_str();

    startWithArgs(4, (char**)args_c);
#endif

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

/** @brief Ebenen aufr&auml;men

 *  @param layer		Ebenenstapel
 */
void
aufraeumen(channel *layer)
{
  {
    while(farbe_pruefen_laeuft) {
      DBG_PROG_S( "farbe_pruefen_laeuft " << farbe_pruefen_laeuft )
      sleep(1);
    }
    // Aufraeumen
    remove(an.c_str());
    remove(bn.c_str());
    remove(pn.c_str());
    if(colour_profile) delete [] colour_profile;
    if(image_profile) free( image_profile);
    if(proof_profile) free (proof_profile);
    image_profile = 0;
    proof_profile = 0;
    if(colour) delete [] colour;
    if(outbuf) delete [] outbuf;
    cmsDeleteTransform (transf);
    cmsCloseProfile (hl);
    cmsCloseProfile (hp);
    if(hs) cmsCloseProfile (hs);
  }
}


/** @brief Speicherblock -> Datei

 *  @param data		Speicherblock
 *  @param groesse	Gr&ouml;&szlig;e
 *  @param name		Dateiname
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
    DBG_PROG_S("Profil geschrieben")
  } else
    g_print ("Profile %s not written.", name.c_str());
}

/** @brief Ebenenlayout -> lcms Farblayout
 
 *  folgende Aspekte sind zu behandeln:\n
    T_COLORSPACE(s) \n
    T_SWAPFIRST(s) \n
    T_FLAVOR(s) \n
    T_PLANAR(p) \n
    T_ENDIAN16(e) \n
    T_DOSWAP(e) \n
    T_EXTRA(e) \n
    T_CHANNELS(c) \n
    T_BYTES(b) - immer 0, da nach float gewandelt wird.\n


 *  @param layer	Ebene
 *  @param p		lcms Farbprofil
 */
gint
drawableColourLayoutToLcms( channel    & layer,
                            cmsHPROFILE  p )
{
  //GDrawableType/*GimpDrawableType*/ drawable_type;
  gint success = GIMP_PDB_SUCCESS;
  int     bit, lcms_bytes,
          in_color_space = PT_ANY;

  format = 0;

  DBG_PROG_S( farb_kanaele <<" "<< format ) 


    switch (layer.precision) {
      case 1:         // uint8
        bit =  8;
        lcms_bytes = 1;
        break;
      case 2:         // uint16
        bit = 16; 
        lcms_bytes = 2;
        break;
      case 3:         // f32
        bit = 32; 
        lcms_bytes = 0;
        break;
      case 4:         // f16 OpenEXR
        bit = 16; 
        lcms_bytes = 0;
        break;
      default:
        g_print ("!!! Precision = %d not allowed!\n", layer.precision);
        return GIMP_PDB_CALLING_ERROR;
    }

    if(farb_kanaele < layer.samplesperpixel)
      ;//format |= EXTRA_SH(layer.samplesperpixel - farb_kanaele);

    format |=             (COLORSPACE_SH(in_color_space)|
                           CHANNELS_SH(farb_kanaele)|
                           BYTES_SH(0)); // lcms_bytes));

  DBG_PROG_S( farb_kanaele <<" "<< T_CHANNELS(format) <<" "<< T_EXTRA(format) <<" "<< T_BYTES(format) )


  return success;
}

/** @brief eingebettetes Profil behandeln

 *  ein Mix aus Pr&uuml;fen und Bearbeiten von Profilinformationen

 *  @param layer	Ebenen
 */
int
bearbeiteEingebetteteProfile( channel *layer )
{
  // hat sich der Profilname geaendert?
  if (!gimp_image_has_icc_profile(image_ID, ICC_IMAGE_PROFILE)) {
    g_message (_("No profil assigned to image."));
    return 1;
  }

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
    ;//DBG_PROG_S( image_ID <<": "<< profil_name <<" "<< old_profil_name )
  if(pprofil_name)
    DBG_PLUG_S( image_ID <<": "<< pprofil_name <<" "<< old_pprofil_name )

  // Test auf Veraenderung des Profiles
  if( strcmp(old_profil_name.c_str(), profil_name) == 0/* &&
      (pprofil_name ?
       (strcmp(old_pprofil_name.c_str(), pprofil_name) == 0) : old_pprofil_name.size()) */)
  {
    if(profil_name) old_profil_name = profil_name; else old_profil_name = "";
    if(pprofil_name) old_pprofil_name = pprofil_name; else old_pprofil_name ="";
  } else
    layer->status |= PROFIL_NEU(1);

  //DBG_PROG_S( "hp = " << hp )
  // Speichern des eingebetteten Profiles
  if(strcmp(old_profil_name.c_str(), profil_name) != 0 ||
     layer->intent != intent_alt)
  {
    gint size=0;
    image_profile = gimp_image_get_icc_profile_by_mem( image_ID, &size,
                                                     ICC_IMAGE_PROFILE);
    setzeRenderingIntent ( image_profile, layer->intent );
    schreibeDatei( image_profile, size, an );
//sleep(10);
    // Berechnung -> CIE*Lab vorbereiten
    if(hl)cmsCloseProfile (hl);
    if(hp)cmsCloseProfile (hp);
    hl   = cmsCreateLabProfile( cmsD50_xyY() );
    hp   = cmsOpenProfileFromMem( image_profile, size );
    farb_kanaele = _cmsChannelsOf( cmsGetColorSpace( hp ) );
    if(farb_kanaele > layer->samplesperpixel) {
      g_message (_("Wrong profil assigned to image."));
      return 1;
    }
    layer->status |= PROFIL_NEU(1);
    //DBG_PROG_S( "hp = " << hp << " status:"<< layer->status )
  }

  // Speichern des eingebetteten Proofprofiles
  int        new_proofing = 
                 gimp_display_get_cms_flags (image_ID) & cmsFLAGS_SOFTPROOFING;
  static int old_proofing = 0;

  if( (pprofil_name ?
       (strcmp(old_pprofil_name.c_str(), pprofil_name) != 0 ||
        old_proofing != new_proofing ) : 0) ||
      layer->intent_proof != intent_alt_proof )
  {
    gint psize=0;
    if ( gimp_image_has_icc_profile (image_ID, ICC_PROOF_PROFILE) &&
         gimp_display_get_cms_flags (image_ID) & cmsFLAGS_SOFTPROOFING)
    {
      proof_profile = gimp_image_get_icc_profile_by_mem ( image_ID, &psize,
                                                          ICC_PROOF_PROFILE);
      if(hs) cmsCloseProfile (hs);
      hs   = cmsOpenProfileFromMem( proof_profile, psize );
      setzeRenderingIntent ( proof_profile, layer->intent_proof );
      schreibeDatei( proof_profile, psize, pn );
    } else {
      DBG_MEM_S( "schreibe 1 byte\n" )
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

  // Intent anpassen
  if((int)layer->intent != intent_alt) {
    intent_alt = layer->intent;
    layer->status |= TRANSFORM_NEU(1);
  }

  // Intent anpassen
  if((int)layer->intent_proof != intent_alt_proof) {
    intent_alt_proof = layer->intent_proof;
    layer->status |= TRANSFORM_NEU(1);
  }

  // Schalter anpassen
  if((int)layer->flags != flags_alt) {
    flags_alt = layer->flags;
    layer->status |= TRANSFORM_NEU(1);
  }

  return 0;
}

/** @brief Farben in 3D ansehen

 *  ben&ouml;tigt ICC Examin

 *  @param image_ID_		CinePaint Bildnummer
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
  
  channel *layer = 0;

  DBG_PROG_S( "image_ID: " << image_ID )

  layer = (channel*) new channel [1];
  nlayers = 1;
  layer->display_ID = gimp_display_active();

  DBG_PROG_S( "layer: " << (int*)layer ) 

  // Min/Max bestimmen
  minMax( image_ID, min_x, min_y, max_x, max_y );

  // Farben Messen und in bestimmten Zeitabstaenden wiederholen
  //
  // Es gibt zwei threads.
  // Der erste Neben-thread started eine while Schleife zum Beobachten
  // des Bildes. Der Haupthread beobachtet ICC Examin.

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

  // starte iccexamin und warte auf seine Beendigung 
  if(!fehler) {
    waechter(layer);
  }

  DBG_PROG_S( "end of " <<__func__ )

  return image_ID;
}

/** @brief Holt einen Farbpunkt
 *  @param layer	Ebene
 *  @param x_punkt	Koordinate
 *  @param y_punkt	Koordinate
 *  @param buf		generischer Speicher zu Ablegen der Farbe
 *  @param n		Layernummer
 *  @param colour_x	gibt die Farbnummer an in welche die Farbe in colour[] soll
 */
static void
holeFarbPunkt (channel* layer, int & x_punkt, int & y_punkt,
               unsigned char *buf, int & n, int &colour_x)
{

        #ifdef DEBUG_
        DBG_CINE_S( n <<": " << x_punkt <<","<< y_punkt << " " )
        #endif
        gimp_pixel_rgn_get_pixel( &layer->srcRgn, buf, x_punkt, y_punkt);

        getColour ( layer, n, buf, &colour[colour_x*farb_kanaele],
                    x_punkt, y_punkt);

        colour[colour_x*farb_kanaele+0] *= 100.0;
        colour[colour_x*farb_kanaele+1] *= 100.0;
        colour[colour_x*farb_kanaele+2] *= 100.0;
        if(farb_kanaele == 4)
        colour[colour_x*farb_kanaele+3] *= 100.0;
        #ifdef DEBUG_
        cout << colour_x <<": "<<
                colour[colour_x*farb_kanaele+0] <<" "<< 
                colour[colour_x*farb_kanaele+1] <<" "<<
                colour[colour_x*farb_kanaele+2] <<" ";
        if(farb_kanaele == 4)
        cout << colour[colour_x*farb_kanaele+3] <<" ";
        DBG
        #endif
}

/** @brief Einzelfarbenprofil schreiben

 *  @param intent		&Uuml;bertragungsart
 */
void
schreibeProfil (icUInt32Number intent)
{
  char* tag = schreibeNcl2Tag (pcsfarbe, geraetefarbe, farb_kanaele,"",name,"");

  DBG_PROG_S( (int*)tag <<" "<< tag_size )

  memcpy (&colour_profile[236], tag, tag_size);
  if(tag)    delete [] tag;

  // Intent setzen
  setzeRenderingIntent ( colour_profile, intent );

  // Speichern des Farbprofiles
  if(colour_profile && tag_size)
    schreibeDatei( colour_profile, 236 + tag_size , bn );
}


/** @brief Ebeneninformationen aktualisieren

 *  Es wird wie sonst &uuml;blich an einer Ebene gearbeitet

 *  holeLayerInfo holt die Informationen in channel*.\n
 *  Sp&auml;ter kann entschieden werden ob alle Informationen g&uuml;ltig
 *  sind.

 *  @param layer		Ebenenstapel
 *  @return 			ge&auml;ndert
 */
int
holeLayerInfo    (channel & layer)
{
  gint   var, var1, var2, var3, var4;
  GDrawableType/*GimpDrawableType*/ drawable_type;

  gint32 drawable_ID = gimp_image_get_active_layer (image_ID);
  if(drawable_ID < 0)
    g_message(_("No active drawable found."));

  // Durchlauf
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
      // Dimension
      if(layer.width != (int)layer.drawable->width ||
         layer.height != (int)layer.drawable->height) {
        layer.status |= GEOMETRY_NEU(1);
        layer.width  = layer.drawable->width;
        layer.height = layer.drawable->height;
      }
      // Auswahl oder sichtbarer Bereich
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
      // Versatz
      gimp_drawable_offsets( layer.ID, &var1, &var2);
      if(layer.offx != var1 ||
         layer.offy != var2) {
        layer.status |= GEOMETRY_NEU(1);
        layer.offx = var1;
        layer.offy = var2;
      }
      // Kanaele
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

/** @brief Rasterparameter festlegen

 *  und gleichzeitig nach ICC Examin schauen

 *  @param layer		Ebenenstapel
 *  @todo Auswahl ber&uuml;cksichtigen
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

/** @brief reserviere Farbspeicher
 */
int
reserviereSpeicher( channel & layer )
{
  n_points = x_num * y_num + 2;

  DBG_MEM_S( n_points )

  outbuf = (double*) new double [n_points*3];
  if(!outbuf) return 1;

  // weiter Aufbereiten
  tag_size  = berechneTagGroesse( n_points, farb_kanaele );
  if(colour_profile) delete [] colour_profile;
  // tag_size * 3 sollte fuer sich aendernde Kanalzahlen ausreichen
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

/** @brief Extreme heraussuchen

    @todo zu dick, sollte vorhandene Funktionen besser nutzen
 */
static void
minMax(gint32 image_ID, int & min_x, int & min_y,
                      int & max_x, int & max_y )
{
  DBG_PROG_S( "Bild: " << image_ID )

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
              default: g_message (_("What kind of image is this?")); break;
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
    DBG printf("max(%d,%d) = ",
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



