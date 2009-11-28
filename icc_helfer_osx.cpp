/*
 * ICC Examin ist eine ICC Profil Betrachter
 * 
 * Copyright (C) 2005  Kai-Uwe Behrmann 
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

#include "icc_utils.h"
#include "icc_icc.h"
#include "icc_helfer.h"
//#include "icc_examin.h"
#include "icc_helfer_x.h"

#if APPLE
# include <Carbon/Carbon.h>
# ifdef HAVE_FLTK
  //#include <FL/osx.H>
# endif
#endif

ICClist<ICClist<double> >
leseGrafikKartenGamma  (std::string display_name,
                        ICClist<std::string> &texte,
                              int x, int y )
{ DBG_PROG_START
  ICClist<ICClist<double> > kurven;
# if APPLE

  int pos = 0;
  CGDisplayErr err = kCGErrorSuccess;
  CGDisplayCount alloc_disps = 0;
  CGDirectDisplayID * active_displays = 0,
                    cg_direct_display_id = 0;
  CGDisplayCount n = 0;

  err = CGGetActiveDisplayList( alloc_disps, active_displays, &n );
  if(n <= 0 || err != kCGErrorSuccess)
  {
    WARN_S( _("open X Display failed") << display_name)
    return 0;
  }
  alloc_disps = n + 1;
  active_displays = (CGDirectDisplayID*) calloc( sizeof(CGDirectDisplayID), alloc_disps );
  if(active_displays)
    err = CGGetActiveDisplayList( alloc_disps, active_displays, &n);

  if(display_name.size())
    pos = atoi( display_name.c_str() );

  if(err)
  {
    WARN_S( "CGGetActiveDisplayList call with error " << err );
  } else
  {
    cg_direct_display_id = active_displays[pos];

    if(active_displays)
      free( active_displays ); active_displays = 0;
  }

  OSStatus              theErr;
  UInt32                size=0, count=0, kanaele=0;
  CMVideoCardGamma*     gamma = nil;

  theErr = CMGetGammaByAVID(cg_direct_display_id, nil, &size);
  require_noerr(theErr, bail);
  gamma = (CMVideoCardGamma*) NewPtrClear(size);
  require(gamma, bail);

  theErr = CMGetGammaByAVID(cg_direct_display_id, gamma, &size);
  require_noerr(theErr, bail);

  texte.resize(4);
  texte[0] = _("Red");
  texte[1] = _("Green");
  texte[2] = _("Blue");
  texte[3] = "gamma_start_ende";

  texte.push_back(_("Manufacturer: "));
  //texte[texte.size()-1].append(monitor.vendor);
  texte.push_back(_("Model:      "));
  //texte[texte.size()-1].append(monitor.model);

  if (gamma->tagType == cmVideoCardGammaTableType)
  {
    kanaele = gamma->u.table.channels;
    count   = gamma->u.table.entryCount; 
    size    = gamma->u.table.entrySize;
    DBG_PROG_V(kanaele<<" "<<count<<" "<<size)
  } else if (gamma->tagType == cmVideoCardGammaFormulaType) {
    char t[24];
    if( gamma->u.formula.redGamma != 1.0 ) {
      texte[0] =_("Gamma Red:   ");
      sprintf(t, "%.2f", gamma->u.formula.redGamma/256.0);
      texte[texte.size()-1].append(t);
    }
    DBG_NUM_V( gamma->u.formula.redGamma )
    if( gamma->u.formula.greenGamma != 1.0 ) {
      texte[1] = _("Gamma Green: ");
      sprintf(t, "%.2f", gamma->u.formula.greenGamma/256.0);
      texte[texte.size()-1].append(t);
    }
    DBG_NUM_V( gamma->u.formula.greenGamma )
    if( gamma->u.formula.blueGamma != 1.0 ) {
      texte[1] = _("Gamma Blue:  ");
      sprintf(t, "%.2f", gamma->u.formula.blueGamma/256.0);
      texte[texte.size()-1].append(t);
    }
    DBG_NUM_V( gamma->u.formula.blueGamma )
  }

  DBG_PROG_V( size )

  if (count)
  {

    unsigned char* data = (unsigned char*)gamma->u.table.data;
    kurven.resize(kanaele);
    for(unsigned int j = 0; j < kanaele; ++j) {
      kurven[j].resize(count);
      for( unsigned int i = 0; i < count; ++i) {
        kurven[j][i] = data[count*size*j + i*size]  / (float)pow(2, 8*size);
      }
    }

  } else DBG_NUM_S( "no vcgt in X displayable" )

  bail:

# endif
  DBG_PROG_ENDE
  return kurven;
}

namespace icc_examin_ns {

  std::string
  holeBundleResource(const char* dateiname, const char* ende)
  {
    std::string adresse, suchen = "%20", ersetzen = " ";
    // osX Resourcen
    CFBundleRef mainBundle;
    // Get the main bundle for the app
    mainBundle = CFBundleGetMainBundle();
    CFURLRef fontURL;
    // Look for a resource in the main bundle by name and type.
    if(mainBundle) {
      CFStringRef d, e;
      d = CFStringCreateWithCString(NULL, dateiname,kCFStringEncodingISOLatin1);
      e = CFStringCreateWithCString(NULL, ende,kCFStringEncodingISOLatin1);
      fontURL = CFBundleCopyResourceURL( mainBundle, d, e, NULL );
      CFRelease(d); CFRelease(e);
      CFStringRef cfstring;
      if(fontURL) {
        cfstring = CFURLCopyPath(fontURL);
        // copy to a C buffer
        CFIndex gr = 1024;
        char *text = (char*)malloc (CFStringGetLength(cfstring));
        text[0] = 0;
        CFStringGetCString( cfstring, text, gr, kCFStringEncodingISOLatin1 );
        adresse = text;
        if(adresse.size()) {
          icc_parser::suchenErsetzen(adresse, suchen, ersetzen, 0);
        }
        DBG_PROG_S( adresse )
        CFRelease(cfstring);
        if(text) free (text);
      } else { WARN_S( "For "<<dateiname<<"|"<<ende<<" no adress found" ) }
    }
    return adresse;
  }

}

