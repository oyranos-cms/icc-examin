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

#include "icc_utils.h"
#include "icc_icc.h"
#include "icc_helfer_x.h"

#include <X11/Xutil.h>
#include <X11/extensions/xf86vmode.h>


std::vector<double>
getXgamma        (std::string display_name)
{ DBG_PROG_START

  Display *display;
  XF86VidModeGamma gamma;
  XF86VidModeMonitor monitor;
  int screen = 0;  // TODO

  if(display_name.size())
    display = XOpenDisplay(display_name.c_str());
  else
    display = XOpenDisplay(0);

  if (!display)
    WARN_S( XDisplayName (display_name.c_str()) )

  XF86VidModeGetMonitor(display, screen, &monitor);
  DBG_PROG_V( monitor.vendor )
  DBG_PROG_V( monitor.model )
  XF86VidModeGetGamma(display, screen, &gamma);
  int size;
  XF86VidModeGetGammaRampSize(display, screen, &size);
  DBG_PROG_V( size )
  unsigned short *red   = new unsigned short [size],
                 *green = new unsigned short [size],
                 *blue  = new unsigned short [size];
  XF86VidModeGetGammaRamp(display, screen, size, red, green, blue);
  delete [] red;
  delete [] green;
  delete [] blue;

  DBG_PROG_ENDE
}


