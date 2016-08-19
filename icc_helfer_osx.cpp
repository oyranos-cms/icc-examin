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

#if defined(__APPLE__)
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
# if defined(__APPLE__)


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

