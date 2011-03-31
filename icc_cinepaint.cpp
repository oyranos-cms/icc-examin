/* 
 * ICC Examin plug-in for cinepaint.
 *
 * Copyright (C) 2004-2010 Kai-Uwe Behrmann <ku.b@gmx.de>
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
#define PLUG_IN_COPYRIGHT     "2004-2010 Kai-Uwe Behrmann"

/***   includes   ***/

#include <string>
#include <sstream>
#include <iostream>
#include <fstream>
#include <cstdio>
#include <cstdlib>
#include <cstring>


using namespace std;


extern "C" {
#include <libgimp/gimp.h>
#include <libgimp/stdplugins-intl.h>
#include <icc34.h>
}

#include "icc_cinepaint_intern.cpp"

char* version() { static char t[80];
                  sprintf(t, "%s - %s %s - %s %s", ICC_EXAMIN_V,
                  START_MONTH,START_YEAR,DEVEL_MONTH,DEVEL_YEAR); return t; }


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


#ifdef __cplusplus
} /* extern "C" */
#endif
/** @} */

/** all variables -> icc_examin_cp */

/*** declaration of local functions ***/


/*** functions ***/

MAIN()

/** @brief plug-in registration */
static void
query ()
{
  static GParamDef args[] =
  {
    { PARAM_INT32 },
    { PARAM_IMAGE },
    { PARAM_DRAWABLE },
  };
  args[0].type = PARAM_INT32; args[0].name = (char*)"run_mode";
  args[0].description = (char*)"Interactive, non-interactive";
  args[1].type = PARAM_IMAGE; args[1].name = (char*)"image";
  args[1].description = (char*)"Input Image";
  args[2].type = PARAM_DRAWABLE; args[2].name = (char*)"drawable";
  args[2].description = (char*)"Input Drawable";
  static GParamDef return_vals[] =
  {
    { PARAM_IMAGE },
  };
  return_vals[0].type = PARAM_IMAGE; return_vals[0].name = (char*)"image";
  return_vals[0].description = (char*)"Output Image";

  n_args_ = sizeof (args) / sizeof (args[0]);
  n_return_vals_ = sizeof (return_vals) / sizeof (return_vals[0]);

  {
    gimp_install_procedure ((char*)PLUG_IN_NAME,
                            (char*)PLUG_IN_BRIEF,
                            (char*)PLUG_IN_DESCRIPTION,
                            (char*)PLUG_IN_AUTHOR,
                            (char*)PLUG_IN_COPYRIGHT,
                            PLUG_IN_VERSION,
                            (char*)"<Image>/Image/ICC Profile Information/Image Profile...",
                            (char*)"*",
                            PROC_PLUG_IN,
                            n_args_, n_return_vals_,
                            args, return_vals);

    gimp_install_procedure ((char*)PLUG_IN_NAME2,
                            (char*)PLUG_IN_BRIEF2,
                            (char*)PLUG_IN_DESCRIPTION2,
                            (char*)PLUG_IN_AUTHOR,
                            (char*)PLUG_IN_COPYRIGHT,
                            PLUG_IN_VERSION,
                            (char*)"<Image>/Image/ICC Profile Information/Proof Profile...",
                            (char*)"*",
                            PROC_PLUG_IN,
                            n_args_, n_return_vals_,
                            args, return_vals);

    gimp_install_procedure ((char*)PLUG_IN_NAME3,
                            (char*)PLUG_IN_BRIEF3,
                            (char*)PLUG_IN_DESCRIPTION3,
                            (char*)PLUG_IN_AUTHOR,
                            (char*)PLUG_IN_COPYRIGHT,
                            PLUG_IN_VERSION,
                            (char*)"<Image>/Image/Watch Colours 3D...",
                            (char*)"*",
                            PROC_PLUG_IN,
                            n_args_, n_return_vals_,
                            args, return_vals);

    gimp_install_procedure ((char*)PLUG_IN_NAME4,
                            (char*)PLUG_IN_BRIEF3,
                            (char*)PLUG_IN_DESCRIPTION3,
                            (char*)PLUG_IN_AUTHOR,
                            (char*)PLUG_IN_COPYRIGHT,
                            PLUG_IN_VERSION,
                            (char*)"<Image>/Image/Watch Colours 3D.2...",
                            (char*)"*",
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
 *  @param[in] nparams			Argument number
 *  @param[in] param			Arguments
 *  @param[out] nreturn_vals		return argument number
 *  @param[out] return_vals		return arguments
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
      // get saved plug-in settings
      gimp_get_data ((char*)PLUG_IN_NAME, &bvals);

      // open a dialog
      if (! dialog_ (param[1].data.d_image))
        return;
      image_ID = doExamin (param[1].data.d_image, typ);
      break;

    case RUN_NONINTERACTIVE:
      // now automatic
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
      // get saved settings
      gimp_get_data ((char*)PLUG_IN_NAME, &bvals);
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
      // get saved plug-in settings
      gimp_get_data ((char*)PLUG_IN_NAME3, &bvals);

      // open a dialog
      if (! dialog_ (param[1].data.d_image))
        return;
      image_ID = doWatch (param[1].data.d_image);
      break;

    case RUN_NONINTERACTIVE:
      // now automatic
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
      // get saved plug-in settings
      gimp_get_data ((char*)PLUG_IN_NAME, &bvals);
      break;

    default:
      break;
    }
  }
  values[0].data.d_status = status;
}


