/*
 * ICC Examin ist eine ICC Profil Betrachter
 * 
 * Copyright (C) 2005-2016  Kai-Uwe Behrmann 
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
 * preparation of osX internal informations - toolkitdependent
 * 
 */

// Date:      14. 01. 2005
#ifdef _
#undef _
#endif
#ifdef USE_GETTEXT
# include <libintl.h>
# define TEXTDOMAIN "icc-examin"
# define _(text) dgettext( TEXTDOMAIN, text )
#else
# define _(text) text
#endif

#include <string.h>
#if defined(__APPLE__)
# include <Carbon/Carbon.h>
#endif

char ** leseGrafikKartenGammaCG ( const char * display_name,
                                  double *** g,
                                  unsigned int * size_ )
{
  char ** texte = NULL;

# if defined(__APPLE__)
  int t_pos = 0;

  int pos = 0;
  CGDisplayErr err = kCGErrorSuccess;
  CGDisplayCount alloc_disps = 0;
  CGDirectDisplayID * active_displays = 0,
                    cg_direct_display_id = 0;
  CGDisplayCount n = 0;

  err = CGGetActiveDisplayList( alloc_disps, active_displays, &n );
  if(n <= 0 || err != kCGErrorSuccess)
  {
    return 0;
  }
  alloc_disps = n + 1;
  active_displays = (CGDirectDisplayID*) calloc( sizeof(CGDirectDisplayID), alloc_disps );
  if(active_displays)
    err = CGGetActiveDisplayList( alloc_disps, active_displays, &n);

  if(display_name)
    pos = atoi( display_name );

  if(!err)
  {
    cg_direct_display_id = active_displays[pos];

    if(active_displays)
      free( active_displays ); active_displays = 0;
  }

  OSStatus              theErr;
  UInt32                size=0, count=0;
  CGGammaValue *        gammas = NULL;

  size = CGDisplayGammaTableCapacity(cg_direct_display_id);
  gammas = (CGGammaValue*) calloc( sizeof(CGGammaValue), size*3 );
  theErr = CGGetDisplayTransferByTable(cg_direct_display_id, 0, &gammas[0], &gammas[size*1], &gammas[size*2], &count);
  require_noerr(theErr, bail);

  texte = calloc(sizeof(char*), 25);
  texte[t_pos++] = strdup(_("Red"));
  texte[t_pos++] = strdup(_("Green"));
  texte[t_pos++] = strdup(_("Blue"));
  texte[t_pos++] = strdup("gamma_start_ende");

  texte[t_pos++] = strdup(_("Manufacturer: "));
  //texte[texte.size()-1].append(monitor.vendor);
  texte[t_pos++] = strdup(_("Model:      "));
  //texte[texte.size()-1].append(monitor.model);

#if 0
  if (gamma->tagType == cmVideoCardGammaTableType)
  {
    count   = gamma->u.table.entryCount; 
    size    = gamma->u.table.entrySize;
  }
  else if (gamma->tagType == cmVideoCardGammaFormulaType) {
    char t[24];
    if( gamma->u.formula.redGamma != 1.0 ) {
      free(texte[0]); texte[0] = strdup(_("Gamma Red:   "));
      sprintf(t, "%.2f", gamma->u.formula.redGamma/256.0);
      texte[t_pos++] = strdup(t);
    }
    if( gamma->u.formula.greenGamma != 1.0 ) {
      free(texte[1]); texte[1] = strdup(_("Gamma Green: "));
      sprintf(t, "%.2f", gamma->u.formula.greenGamma/256.0);
      texte[t_pos++] = strdup(t);
    }
    if( gamma->u.formula.blueGamma != 1.0 ) {
      free(texte[2]); texte[2] = strdup(_("Gamma Blue:  "));
      sprintf(t, "%.2f", gamma->u.formula.blueGamma/256.0);
      texte[t_pos++] = strdup(t);
    }
  }
#endif

  if(size)
  {
    int i,x;
    for(i = 0; i < 3; ++i)
    {
      *g[i] = calloc( sizeof(double), size );

      for(x = 0; x < size; ++x)
        *g[i][x] = gammas[size*i+x];
    }
  }

  bail:

# endif
  return texte;
}


