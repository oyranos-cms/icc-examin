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
 * Aufbereitung von X internen Informationen - werkzeugabhängig
 * 
 */

// Date:      11. 01. 2005

# define BOOL   LCMS_BOOL

#include "icc_utils.h"
#include "icc_icc.h"
#include "icc_helfer_x.h"

#undef BOOL

#include <X11/Xutil.h>
#include <X11/extensions/xf86vmode.h>
#ifdef HAVE_FLTK
//#include <FL/x.H>
#endif

std::vector<std::vector<double> >
leseGrafikKartenGamma        (std::string display_name,
                              std::vector<std::string> &texte )
{ DBG_PROG_START

  Display *display;
  XF86VidModeGamma gamma;
  XF86VidModeMonitor monitor;
  int screen = 0;  // TODO
  std::vector<std::vector<double> > kurven;

  texte.resize(4);
  texte[0] = _("Rot");
  texte[1] = _("Grün");
  texte[2] = _("Blau");
  texte[3] = "gamma_start_ende";

  if(display_name.size())
    display = XOpenDisplay(display_name.c_str());
  else
  {
    //#ifdef HAVE_FLTK
    //display = fl_display;
    //#else
    display = XOpenDisplay(0);
    //#endif
  }

  if (!display) {
    WARN_S( XDisplayName (display_name.c_str()) )
    DBG_PROG_ENDE
    return kurven;
  }
  DBG_PROG_V( XDisplayName (display_name.c_str()) )

  if (!XF86VidModeGetMonitor(display, screen, &monitor))
    WARN_S( _("Keine Monitor Information erhalten") )
  else {
    texte.push_back(_("Hersteller: "));
    texte[texte.size()-1].append(monitor.vendor);
    texte.push_back(_("Model:      "));
    texte[texte.size()-1].append(monitor.model);
  }
  DBG_PROG_V( monitor.vendor )
  DBG_PROG_V( monitor.model )
  if (!XF86VidModeGetGamma(display, screen, &gamma))
    WARN_S( _("Keine Gamma Information erhalten") )
  else {
    char t[24];
    if( gamma.red != 1.0 ) {
      texte.push_back("");
      texte.push_back(_("Gamma Rot:   "));
      sprintf(t, "%.2f", gamma.red);
      texte[texte.size()-1].append(t);
    }
    DBG_NUM_V( gamma.red )
    if( gamma.green != 1.0 ) {
      texte.push_back(_("Gamma Grün: "));
      sprintf(t, "%.2f", gamma.green);
      texte[texte.size()-1].append(t);
    }
    DBG_NUM_V( gamma.green )
    if( gamma.blue != 1.0 ) {
      texte.push_back(_("Gamma Blau:  "));
      sprintf(t, "%.2f", gamma.blue);
      texte[texte.size()-1].append(t);
    }
    DBG_NUM_V( gamma.blue )
  }

  int size;
  if (!XF86VidModeGetGammaRampSize(display, screen, &size))
    WARN_S( _("Kein Gammagradient Information erhalten") );

  DBG_PROG_V( size )
  if (size)
  {
    unsigned short *red   = new unsigned short [size],
                   *green = new unsigned short [size],
                   *blue  = new unsigned short [size];
    if (!XF86VidModeGetGammaRamp(display, screen, size, red, green, blue))
      WARN_S( _("Kein Gammagradient Information erhalten") )

    kurven.resize(3);
    for( int i = 0; i < 3; ++i) {
      kurven[i].resize(size);
    }
    for(int j = 0; j < size; ++j) {
        kurven[0][j] = red[j]  /65535.0;
        kurven[1][j] = green[j]/65535.0;
        kurven[2][j] = blue[j] /65535.0;
    }

    delete [] red;
    delete [] green;
    delete [] blue;
  } else DBG_NUM_S( "kein vcgt in X anzeigbar" );

  //XCloseDisplay(display);

  DBG_PROG_ENDE
  return kurven;
}


