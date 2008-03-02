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

#define BOOL   LCMS_BOOL

#include "icc_utils.h"
#include "icc_icc.h"
#include "icc_helfer_x.h"
#include "icc_oyranos.h"

#undef BOOL

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#ifdef HAVE_XF86VMODE
#include <X11/extensions/xf86vmode.h>
#endif
#ifdef HAVE_XIN
  #include <X11/extensions/Xinerama.h>
#endif
#ifdef HAVE_FLTK
#include <FL/x.H>
#endif

std::vector<std::vector<double> >
leseGrafikKartenGamma        (std::string display_name,
                              std::vector<std::string> &texte,
                              int x, int y )
{ DBG_PROG_START

  std::vector<std::vector<double> > kurven;

#ifdef HAVE_XF86VMODE
  Display *display = 0;

  texte.resize(4);
  texte[0] = _("Red");
  texte[1] = _("Green");
  texte[2] = _("Blue");
  texte[3] = "gamma_start_ende";

  const char *disp_name = display_name.c_str();
  DBG_PROG_V( disp_name )

  if(display_name.size())
    display = XOpenDisplay(display_name.c_str());
  else
  {
#   ifdef HAVE_FLTK
    display = fl_display;
#   else
    display = XOpenDisplay(0);
#   endif
  }

  if (!display) {
    WARN_S( _("open X Display failed") )
    DBG_PROG_ENDE
    return kurven;
  }
  DBG_PROG_V( XDisplayName (display_name.c_str()) )

  // Wo befindet sich das Fenster?
  int n_fenster = 0;
  int screens = 0;
  int screen = 0;
# ifdef HAVE_XIN
  XineramaScreenInfo* fenster = 0;
# endif
  if( ScreenCount( display ) > 1 )
  {
    int scr_nr = -1;
    char *display_name = getenv("DISPLAY");
    char *ptr = NULL;

    if(display_name &&
       (ptr = strchr(display_name,':')) != 0)
      if( (ptr = strchr(ptr, '.')) != 0 )
        ++ptr;
    if(ptr)
    {
      Screen *scr = XScreenOfDisplay( display, atoi(ptr) );
      scr_nr = XScreenNumberOfScreen( scr );
      screen = scr_nr;
    }

    for (int i = 0; i < ScreenCount( display ); ++i)
    {
      Screen *scr = XScreenOfDisplay( display, i );
      scr_nr = XScreenNumberOfScreen( scr );
      DBG_PROG_V( scr_nr )
      if( scr_nr != i )
        WARN_S( "scr_nr != i" << scr_nr <<"/"<< i )
      else if( scr_nr == screen )
      {
        char nr[8]; snprintf( nr, 8, "%d", scr_nr );
        texte.push_back(_("Screen:"));
        texte[texte.size()-1].append( " ." );
        texte[texte.size()-1].append( nr );
        //screen = scr_nr;
      }
      XF86VidModeGetViewPort( display, scr_nr, &x, &y );
      DBG_PROG_V( x <<" "<< y )
      ++screens;
    }

  }
# ifdef HAVE_XIN
  else
    if( XineramaIsActive( display ) )
    {
      fenster = XineramaQueryScreens( display, &n_fenster );
      for (int i = 0; i < n_fenster; ++i) {
        DBG_PROG_S( "Fenster[" << fenster[i].screen_number <<"]: "<<
                    fenster[i].x_org <<"+"<<
                    fenster[i].y_org <<","<< fenster[i].width <<"x"<<
                    fenster[i].height );
        if( x >= fenster[i].x_org && x < fenster[i].x_org + fenster[i].width &&
            y >= fenster[i].y_org && y < fenster[i].y_org + fenster[i].height )
        { char nr[8]; snprintf( nr, 8, "%d", fenster[i].screen_number );
          texte.push_back(_("XineramaScreen:"));
          texte[texte.size()-1].append( " " );
          texte[texte.size()-1].append( nr );
          screen = fenster[i].screen_number;
        }
        //int vp_x, vp_y;
        //XF86VidModeGetViewPort( display, i, &vp_x, &vp_y );
        //DBG_PROG_V( i <<": "<< vp_x <<" "<< vp_y )
        ++screens;
      }
    }
# endif
    
  DBG_PROG_V( ScreenCount( display ) )

  XF86VidModeGamma gamma;
  XF86VidModeMonitor monitor;
  //int screen = DefaultScreen( display );
  char **infos = 0;
  int num = 0;
    if( (infos = icc_oyranos.moniInfo( x,y, &num)) != 0  && num ) {
      for( int i = 0; i < num; ++i ) {
        texte.push_back( infos[i*2 + 0] );
        texte[texte.size()-1]. append( infos[i*2 + 1] );
      }
    } else
    if (XF86VidModeGetMonitor(display, screen, &monitor)) {
      texte.push_back(_("Manufacturer:"));
      texte[texte.size()-1].append(monitor.vendor);
      texte.push_back(_("Model:       "));
      texte[texte.size()-1].append(monitor.model);
      DBG_PROG_V( monitor.vendor )
      DBG_PROG_V( monitor.model )
    } else {
      WARN_S( _("Keine Monitor Information erhalten") )
    }
  DBG_PROG_V( DisplayWidth(display, screen) <<" "<< DisplayWidthMM(display, screen) )

  if (!XF86VidModeGetGamma(display, screen, &gamma))
  { DBG_PROG_S( _("Keine Gamma Information erhalten") );
  } else {
    char t[24];
    if( gamma.red != 1.0 ) {
      texte.push_back("");
      texte.push_back(_("Gamma Red:   "));
      sprintf(t, "%.2f", gamma.red);
      texte[texte.size()-1].append(t);
    }
    DBG_NUM_V( gamma.red )
    if( gamma.green != 1.0 ) {
      texte.push_back(_("Gamma Green:"));
      sprintf(t, "%.2f", gamma.green);
      texte[texte.size()-1].append(t);
    }
    DBG_NUM_V( gamma.green )
    if( gamma.blue != 1.0 ) {
      texte.push_back(_("Gamma Blue:  "));
      sprintf(t, "%.2f", gamma.blue);
      texte[texte.size()-1].append(t);
    }
    DBG_NUM_V( gamma.blue )
  }

  int size;
  if (!XF86VidModeGetGammaRampSize(display, screen, &size))
    DBG_PROG_S( _("Kein Gammagradient Information erhalten") );

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

# ifndef HAVE_FLTK
  if (display) XCloseDisplay(display);
  else WARN_S( "no X Display active" )
# endif

#endif
  DBG_PROG_ENDE
  return kurven;
}


