/*
 * ICC Examin ist eine ICC Profil Betrachter
 * 
 * Copyright (C) 2005-2007  Kai-Uwe Behrmann 
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

// Date:      11. 01. 2005

#define BOOL   LCMS_BOOL

#include "icc_utils.h"
#include "icc_icc.h"
#include "icc_helfer_x.h"
#include "icc_helfer.h"
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
#if defined( HAVE_XRANDR )
  # include <X11/extensions/Xrandr.h>
#endif
#ifdef HAVE_FLTK
#include <FL/x.H>
#endif

ICClist<ICClist<double> >
leseGrafikKartenGamma        (std::string display_name,
                              ICClist<std::string> &texte,
                              int x, int y )
{ DBG_PROG_START

  ICClist<ICClist<double> > kurven;

#ifdef HAVE_XF86VMODE
  Display *display = 0;

  texte.resize(4);
  texte[0] = _("Red");
  texte[1] = _("Green");
  texte[2] = _("Blue");
  texte[3] = "gamma_start_ende";

# ifdef DEBUG
  const char *disp_name = display_name.c_str();
# endif
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
  int n_xin_info = 0;
  int screens = 0;
  int screen = 0;
  int effective_screen = 0;
# ifdef HAVE_XIN
  XineramaScreenInfo* xin_info = 0;
# endif
  if( ScreenCount( display ) > 1 )
  {
    int scr_nr = -1;
    doLocked_m( char *display_name = getenv("DISPLAY");, NULL)
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
      xin_info = XineramaQueryScreens( display, &n_xin_info );
      for (int i = 0; i < n_xin_info; ++i) {
        DBG_PROG_S( "Fenster[" << xin_info[i].screen_number <<"]: "<<
                    xin_info[i].x_org <<"+"<<
                    xin_info[i].y_org <<","<< xin_info[i].width <<"x"<<
                    xin_info[i].height );
        if( x >= xin_info[i].x_org && x < xin_info[i].x_org + xin_info[i].width &&
            y >= xin_info[i].y_org && y < xin_info[i].y_org + xin_info[i].height )
        { char nr[8]; snprintf( nr, 8, "%d", xin_info[i].screen_number );
          texte.push_back(_("XineramaScreen:"));
          texte[texte.size()-1].append( " " );
          texte[texte.size()-1].append( nr );
          screen = xin_info[i].screen_number;
        }
        //int vp_x, vp_y;
        //XF86VidModeGetViewPort( display, i, &vp_x, &vp_y );
        //DBG_PROG_V( i <<": "<< vp_x <<" "<< vp_y )
        ++screens;
      }
    }
# endif
  if (ScreenCount( display ) > 1 || screen == 0)
    effective_screen = screen;
    
  DBG_PROG_V( ScreenCount( display ) )

  static int xrr_version = -1;
  int gamma_size = 0;
  int crtc = 0;
# if defined(HAVE_XRANDR)
  int major_versionp = 0;
  int minor_versionp = 0;
  int i, n = 0;
  Window root = RootWindow(display, DefaultScreen( display )); 

  if(xrr_version == -1)
  {
    XRRQueryVersion( display, &major_versionp, &minor_versionp );
    xrr_version = major_versionp*100 + minor_versionp;
  }

  if(xrr_version >= 102)
  {                           
    static XRRScreenResources * res = 0;
    if(!res)
      res = XRRGetScreenResources( display, root );

    if(res && res->ncrtc == n_xin_info)
    {
      int ncrtc = 0;
      n = res->noutput;
      for( i = 0; i < n; ++i )
      {
        RROutput output = res->outputs[i];
        XRROutputInfo * output_info = XRRGetOutputInfo( display, res,
                                                        output);
        if(output_info->crtc)
        {
          XRRCrtcInfo * info = XRRGetCrtcInfo( display, res, output_info->crtc );

          ncrtc++;

          if( x >= (int)info->x && x < (int)(info->x + info->width) &&
              y >= (int)info->y && y < (int)(info->y + info->height) )
          {
            crtc = output_info->crtc;
            gamma_size = XRRGetCrtcGammaSize( display, output_info->crtc );
            DBG_NUM_S("[" <<ncrtc-1<<"] (" <<screen<<") crtc: "
                       <<output_info->crtc<<" name: "<<output_info->name<<" "<<
                       info->width<<"x"<<info->height<<"+"<<info->x<<"+"<<info->y);
          }
          XRRFreeCrtcInfo( info );
        }

        XRRFreeOutputInfo( output_info ); output_info = 0;
      }
    }
    //XRRFreeScreenResources(res); res = 0;
  } 
    
#endif /* HAVE_XRANDR */

  XF86VidModeGamma gamma;
  XF86VidModeMonitor monitor;
  //int screen = DefaultScreen( display );
  int num = 0;
  char **infos = 0;
    if( (infos = icc_oyranos.moniInfo( x,y, &num)) != 0  && num ) {
      for( int i = 0; i < num; ++i ) {
        texte.push_back( infos[i*2 + 0] );
        texte[texte.size()-1]. append( infos[i*2 + 1] );
      }
    } else
    if (XF86VidModeGetMonitor(display, effective_screen, &monitor)) {
      texte.push_back(_("Manufacturer:"));
      texte[texte.size()-1].append(monitor.vendor?monitor.vendor:"");
      texte.push_back(_("Model:       "));
      texte[texte.size()-1].append(monitor.model?monitor.model:"");
      DBG_PROG_V( monitor.vendor )
      DBG_PROG_V( monitor.model )
    } else {
      WARN_S( "no monitor information obtained" )
    }
  DBG_PROG_V( DisplayWidth(display, screen) <<" "<< DisplayWidthMM(display, screen) )

  if(xrr_version >= 102 && gamma_size)
  {
    char t[24];
    texte.push_back("");
    texte.push_back(_("Gamma:   "));
    sprintf(t, "%d", gamma_size);
    texte[texte.size()-1].append(t);
  } else
  if (!XF86VidModeGetGamma(display, effective_screen, &gamma))
  { DBG_PROG_S( "no gamma information obtained" );
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

  int size = 0;
  if (!XF86VidModeGetGammaRampSize(display, effective_screen, &size))
    DBG_PROG_S( "no gammagradient information obtained" );

  DBG_PROG_V( size )
# if defined(HAVE_XRANDR)
  if(xrr_version >= 102 && gamma_size)
  {
    size = gamma_size;
    unsigned short *red   = new unsigned short [size],
                   *green = new unsigned short [size],
                   *blue  = new unsigned short [size];

    XRRCrtcGamma * gamma = XRRGetCrtcGamma(display, crtc);

    if(!gamma)
      WARN_S( _("no gammagradient information obtained") )

    kurven.resize(3);
    for( int i = 0; i < 3; ++i) {
      kurven[i].resize(size);
    }
    for(int j = 0; j < size; ++j) {
        kurven[0][j] = gamma->red[j]  /65535.0;
        kurven[1][j] = gamma->green[j]/65535.0;
        kurven[2][j] = gamma->blue[j] /65535.0;
    }

    XRRFreeGamma (gamma);
    delete [] red;
    delete [] green;
    delete [] blue;
  } else
#endif
  if (size)
  {
    unsigned short *red   = new unsigned short [size],
                   *green = new unsigned short [size],
                   *blue  = new unsigned short [size];
    if (!XF86VidModeGetGammaRamp(display, screen, size, red, green, blue))
      WARN_S( _("no gammagradient information obtained") )

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
  } else DBG_NUM_S( "no vcgt in X displayable" );

# ifndef HAVE_FLTK
  if (display) XCloseDisplay(display);
  else WARN_S( "no X Display active" )
# endif

#endif
  DBG_PROG_ENDE
  return kurven;
}


