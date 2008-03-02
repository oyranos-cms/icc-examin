/*
 * ICC Examin ist eine ICC Profil Betrachter
 * 
 * Copyright (C) 2004  Kai-Uwe Behrmann 
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
 * Aufbereitung von ICC internen Informationen - werkzeugabhängig
 * 
 */

// Date:      04. 05. 2004

#if 0
# ifndef DEBUG
#  define DEBUG
# endif
# define DEBUG_ICCFUNKT
#endif

#include "icc_utils.h"
#include "icc_helfer_fltk.h"
#include "icc_helfer.h"
#include "icc_icc.h"
#include <Fl/Fl.H>
#if HAVE_X
#include <X11/xpm.h>
#include <X11/extensions/shape.h>
#include <FL/x.H>
#endif


#ifdef HAVE_FLTK
int*
getChannel_flColours (icColorSpaceSignature color)
{
  static int flFarben[16];
# define nFARBEN(n) for (int i = 0; i < n; i++) \
                       { \
                         flFarben[i] = 9 + i; \
                       }

  switch (color) {
    case icSigXYZData: flFarben[0] = FL_RED;
                       flFarben[1] = FL_GREEN;
                       flFarben[2] = FL_BLUE;
                       break;
    case icSigLabData: flFarben[0] = FL_WHITE;
                       flFarben[1] = FL_RED;
                       flFarben[2] = FL_BLUE;
                       break;
    case icSigLuvData: flFarben[0] = FL_WHITE;
                       flFarben[1] = FL_RED;
                       flFarben[2] = FL_BLUE;
                       break;
    case icSigYCbCrData:flFarben[0] = FL_WHITE;
                       flFarben[1] = FL_BLUE;
                       flFarben[2] = FL_RED;
                       break;
    case icSigYxyData: flFarben[0] = FL_WHITE;
                       flFarben[1] = FL_RED;
                       flFarben[2] = FL_GREEN;
                       break;
    case icSigRgbData: flFarben[0] = FL_RED;
                       flFarben[1] = FL_GREEN;
                       flFarben[2] = FL_BLUE;
                       break;
    case icSigGrayData:flFarben[0] = FL_LIGHT2;
                       break;
    case icSigHsvData: flFarben[0] = FL_RED;
                       flFarben[1] = FL_GREEN;
                       flFarben[2] = FL_WHITE;
                       break;
    case icSigHlsData: flFarben[0] = FL_RED;
                       flFarben[1] = FL_WHITE;
                       flFarben[2] = FL_GREEN;
                       break;
    case icSigCmykData:flFarben[0] = FL_CYAN;
                       flFarben[1] = FL_MAGENTA;
                       flFarben[2] = FL_YELLOW;
                       flFarben[3] = FL_BLACK;
                       break;
    case icSigCmyData: flFarben[0] = FL_CYAN;
                       flFarben[1] = FL_MAGENTA;
                       flFarben[2] = FL_YELLOW;
                       break;
    case icSig2colorData: nFARBEN(2) break;
    case icSig3colorData: nFARBEN(3) break;
    case icSig4colorData: nFARBEN(4) break;
    case icSig5colorData: nFARBEN(5) break;
    case icSig6colorData: nFARBEN(6) break;
    case icSig7colorData: nFARBEN(7) break;
    case icSig8colorData: nFARBEN(8) break;
    case icSig9colorData: nFARBEN(9) break;
    case icSig10colorData: nFARBEN(10) break;
    case icSig11colorData: nFARBEN(11) break;
    case icSig12colorData: nFARBEN(12) break;
    case icSig13colorData: nFARBEN(13) break;
    case icSig14colorData: nFARBEN(14) break;
    case icSig15colorData: nFARBEN(15) break;
    default: flFarben[0] = FL_LIGHT2; break;
  }
  return flFarben;
}

#undef nFARBEN

void
dbgFltkEvents(int event)
{
  DBG_MEM_S( "event: " << dbgFltkEvent(event) )
}

std::string
dbgFltkEvent(int event)
{
  std::string text;
  switch(event) {
  case FL_NO_EVENT: text = "FL_NO_EVENT"; break;
  case FL_PUSH: text = "FL_PUSH"; break;
  case FL_RELEASE: text = "FL_RELEASE"; break;
  case FL_ENTER: text = "FL_ENTER"; break;
  case FL_LEAVE: text = "FL_LEAVE"; break;
  case FL_DRAG: text = "FL_DRAG"; break;
  case FL_FOCUS: text = "FL_FOCUS"; break;
  case FL_UNFOCUS: text = "FL_UNFOCUS"; break;
  case FL_KEYDOWN: text = "FL_KEYDOWN"; break;
  case FL_KEYUP: text = "FL_KEYUP"; break;
  case FL_CLOSE: text = "FL_CLOSE"; break;
  case FL_MOVE: text = "FL_MOVE"; break;
  case FL_SHORTCUT: text = "FL_SHORTCUT"; break;
  case FL_DEACTIVATE: text = "FL_DEACTIVATE"; break;
  case FL_ACTIVATE: text = "FL_ACTIVATE"; break;
  case FL_HIDE: text = "FL_HIDE"; break;
  case FL_SHOW: text = "FL_SHOW"; break;
  case FL_PASTE: text = "FL_PASTE"; break;
  case FL_SELECTIONCLEAR: text = "FL_SELECTIONCLEAR"; break;
  case FL_MOUSEWHEEL: text = "FL_MOUSEWHEEL"; break;
  case FL_DND_ENTER: text = "FL_DND_ENTER"; break;
  case FL_DND_DRAG: text = "FL_DND_DRAG"; break;
  case FL_DND_LEAVE: text = "FL_DND_LEAVE"; break;
  case FL_DND_RELEASE: text = "FL_DND_RELEASE"; break;
  default: text = "event: "; event += event;
  }
  return text;
}

void
setzeIcon      ( Fl_Window *fenster, char   **xpm_daten )
{
# if HAVE_X && !APPLE
  fl_open_display();
  fenster->make_current();
  DBG_PROG_V( (int*) fl_display <<" "<< fl_window )
  Pixmap pm, mask;
  XpmCreatePixmapFromData(  fl_display,
                            DefaultRootWindow(fl_display),
                            xpm_daten,
                            &pm,
                            &mask,
                            NULL);
# if 0
  XShapeCombineMask(fl_display, fl_window,
                    ShapeBounding,0,0,
                    mask,ShapeSet);
# endif
  //fenster->icon((char*)p); // die FLTK Methode

  XWMHints *hinweis;
  hinweis = XGetWMHints( fl_display, fl_window );
  if (!hinweis)
    hinweis = XAllocWMHints();
  hinweis->flags |= IconPixmapHint;
  hinweis->icon_pixmap = pm;
  hinweis->flags |= IconMaskHint;
  hinweis->icon_mask = mask;
  XSetWMHints( fl_display, fl_window, hinweis );
  XFree( hinweis );
# endif
}


namespace icc_examin_ns {

  static int icc_thread_lock_zaehler_ = 0;
  void lock(const char *file, int line)
  {
    ++icc_thread_lock_zaehler_;
    DBG_THREAD_S( "locks: "<<icc_thread_lock_zaehler_ <<" Aufruf bei: "<<file<<":"<<line )
    Fl::lock();
    DBG_THREAD_S( "weiter" )
  }
  void unlock(void *widget, const char *file, int line)
  {
    --icc_thread_lock_zaehler_;
    DBG_THREAD_S( "locks: "<<icc_thread_lock_zaehler_ <<" Aufruf bei: "<<file<<":"<<line )
    Fl::unlock();
    Fl::awake(widget);
    DBG_THREAD_S( "weiter" )
    icc_thread_lock_zaehler_ = 0;
  }
}


void
fl_delayed_redraw(void *w)
{
  ((Fl_Widget*)w)->redraw();
  DBG
  Fl::remove_idle(fl_delayed_redraw, w);
}


#endif
