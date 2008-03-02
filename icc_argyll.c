/*
 * ICC Examin ist eine ICC Profil Betrachter
 * 
 * Copyright (C) 2007  Kai-Uwe Behrmann 
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
 * visualise colour gamut in WWW-3D format vrml.
 * 
 */

/* Date:      August 2007 */


#ifdef USE_ARGYLL

#define SURFACE_ONLY
#define GAMRES 10.0             /* Default surface resolution */

#define USE_CAM_CLIP_OPT

#define RGBRES 33       /* 33 */
#define CMYKRES 17      /* 17 */

#undef RANDOMPOINTS
#undef RANDCUBE

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <fcntl.h>
#include <string.h>
#include <math.h>
#include "numlib.h"
#include "icc.h" 
#include "xicc.h"
#include "gamut.h"

#endif /* USE_ARGYLL */

int
iccCreateArgyllVrml( const char* prof_name, int intent, double * volume )
{

#ifdef USE_ARGYLL
  /* taken from iccgamut.c : argyll-0.60 */

        int fa,nfa;                             /* argument we're looking at */
        char *xl, out_name[100];
        icmFile *fp;
        icc *icco;
        xicc *xicco;
        gamut *gam;
        int verb = 0;
        int rv = 0;
        int vrml = 1; /*0;*/
        int doaxes = 0 /*1*/;
        int docusps = 0;
        double gamres = GAMRES;         /* Surface resolution */
        int fl = 0;                                     /* luobj flags */
        icxInk ink;                                     /* Ink parameters */
        int tlimit = -1;                        /* Total ink limit as a % */
        int klimit = -1;                        /* Black ink limit as a % */
        icxViewCond vc;                         /* Viewing Condition for CIECAM */
        int vc_e = -1;                          /* Enumerated viewing condition */
        int vc_s = -1;                          /* Surround override */
        double vc_wXYZ[3] = {-1.0, -1.0, -1.0}; /* Adapted white override in XYZ */
        double vc_wxy[2] = {-1.0, -1.0};                /* Adapted white override in x,y */
        double vc_a = -1.0;                     /* Adapted luminance */
        double vc_b = -1.0;                     /* Background % overid */
        double vc_f = -1.0;                     /* Flare % overid */
        double vc_fXYZ[3] = {-1.0, -1.0, -1.0}; /* Flare color override in XYZ */
        double vc_fxy[2] = {-1.0, -1.0};                /* Flare color override in x,y */

        icxLuBase *luo;

        /* Lookup parameters */
        icmLookupFunc     func   = icmFwd;                              /* Default */
        /*icRenderingIntent intent = icAbsoluteColorimetric;*/     /* Default */
        icColorSpaceSignature pcsor = icSigLabData;     /* Default */
        icmLookupOrder    order  = icmLuOrdNorm;                /* Default */

        /* Open up the profile for reading */
        if ((fp = new_icmFileStd_name(prof_name,"r")) == NULL)
                error ("Can't open file '%s'",prof_name);

        if ((icco = new_icc()) == NULL)
                error ("Creation of ICC object failed");

        if ((rv = icco->read(icco,fp,0)) != 0)
                error ("%d, %s",rv,icco->err);

        if (verb) {
                icmFile *op;
                if ((op = new_icmFileStd_fp(stdout)) == NULL)
                        error ("Can't open stdout");
                icco->header->dump(icco->header, op, 1);
                op->del(op);
        }

        /* Wrap with an expanded icc */
        if ((xicco = new_xicc(icco)) == NULL)
                error ("Creation of xicc failed");

        /* Set the ink limits */
        icxDefaultLimits(icco, &ink.tlimit, tlimit/100.0, &ink.klimit, klimit/100.0);

        if (verb) {
                if (ink.tlimit >= 0.0)
                        printf("Total ink limit assumed is %3.0f%%\n",100.0 * ink.tlimit);
                if (ink.klimit >= 0.0)
                        printf("Black ink limit assumed is %3.0f%%\n",100.0 * ink.klimit);
        }

        /* Setup a safe ink generation (not used) */
        ink.k_rule = icxKluma5k;
        ink.c.Ksmth = ICXINKDEFSMTH;    /* Default smoothing */
        ink.c.Kstle = 0.0;              /* Min K at white end */
        ink.c.Kstpo = 0.0;              /* Start of transition is at white */
        ink.c.Kenle = 1.0;              /* Max K at black end */
        ink.c.Kenpo = 1.0;              /* End transition at black */
        ink.c.Kshap = 1.0;              /* Linear transition */

        /* Setup the default viewing conditions */
        if (xicc_enum_viewcond(xicco, &vc, -1, NULL, 0) == -2)
                error ("%d, %s",xicco->errc, xicco->err);

        if (vc_e != -1)
                if (xicc_enum_viewcond(xicco, &vc, vc_e, NULL, 0) == -2)
                        error ("%d, %s",xicco->errc, xicco->err);
        if (vc_s >= 0)
                vc.Ev = vc_s;
        if (vc_wXYZ[1] > 0.0) {
                /* Normalise it to current media white */
                vc.Wxyz[0] = vc_wXYZ[0]/vc_wXYZ[1] * vc.Wxyz[1];
                vc.Wxyz[2] = vc_wXYZ[2]/vc_wXYZ[1] * vc.Wxyz[1];
        }
        if (vc_wxy[0] >= 0.0) {
                double x = vc_wxy[0];
                double y = vc_wxy[1];   /* If Y == 1.0, then X+Y+Z = 1/y */
                double z = 1.0 - x - y;
                vc.Wxyz[0] = x/y * vc.Wxyz[1];
                vc.Wxyz[2] = z/y * vc.Wxyz[1];
        }

        if (vc_a >= 0.0)
                vc.La = vc_a;
        if (vc_b >= 0.0)
                vc.Yb = vc_b/100.0;
        if (vc_f >= 0.0)
                vc.Yf = vc_f/100.0;
        if (vc_fXYZ[1] > 0.0) {
                /* Normalise it to current media white */
                vc.Fxyz[0] = vc_fXYZ[0]/vc_fXYZ[1] * vc.Fxyz[1];
                vc.Fxyz[2] = vc_fXYZ[2]/vc_fXYZ[1] * vc.Fxyz[1];
        }
        if (vc_fxy[0] >= 0.0) {
                double x = vc_fxy[0];
                double y = vc_fxy[1];   /* If Y == 1.0, then X+Y+Z = 1/y */
                double z = 1.0 - x - y;
                vc.Fxyz[0] = x/y * vc.Fxyz[1];
                vc.Fxyz[2] = z/y * vc.Fxyz[1];
        }

#ifdef USE_CAM_CLIP_OPT
         fl |= ICX_CAM_CLIP;
#endif

#ifdef NEVER
        printf("~1 output space flags = 0x%x\n",fl);
        printf("~1 output space intent = %s\n",icx2str(icmRenderingIntent,intent));
        printf("~1 output space pcs = %s\n",icx2str(icmColorSpaceSignature,pcsor));
        printf("~1 output space viewing conditions =\n"); xicc_dump_viewcond(&vc);
        printf("~1 output space inking =\n"); xicc_dump_inking(&ink);
#endif

        /* Get a expanded color conversion object */
        if ((luo = xicco->get_luobj(xicco, fl, func, intent, pcsor, order, &vc, &ink)) == NULL)
                error ("%d, %s",xicco->errc, xicco->err);

        /* Creat a gamut surface */
        if ((gam = luo->get_gamut(luo, gamres)) == NULL)
                error ("%d, %s",xicco->errc, xicco->err);

        strcpy(out_name, prof_name);
        if ((xl = strrchr(out_name, '.')) == NULL)      /* Figure where extention is */
                xl = out_name + strlen(out_name);

        strcpy(xl,".gam");
        if (gam->write_gam(gam,out_name))
                error ("write gamut failed on '%s'",out_name);

        if (vrml) {
                strcpy(xl,".wrl");
                if (gam->write_vrml(gam,out_name, doaxes, docusps))
                        error ("write vrml failed on '%s'",out_name);
        }

        if (verb) {
                printf("Total volume of gamut is %f cubic colorspace units\n",gam->volume(gam));
        }
        *volume = gam->volume(gam);
        gam->del(gam);

        luo->del(luo);                  /* Done with lookup object */

        xicco->del(xicco);              /* Expansion wrapper */
        icco->del(icco);                        /* Icc */
        fp->del(fp);

#endif /* USE_ARGYLL */

  return 0;
}


