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
 * Aufbereitung von ICC internen Informationen - werkzeugabhaengig
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
  if(!fenster) {
    WARN_S("no window provided")
    return;
  }
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


  /** Programmstraenge - threads

      moegliche Szenarien: \n
      A: \n
         - Straenge A und B greifen nacheinander zu \n
         - A erhaelt das Recht des Ersteren, B wartet \n
         - B kann nachdem A freigegeben hat auch mal

      B: \n
         - A und B und C wollen \n
         - Strang A darf, B wartet und C auch \n
         - nachdem A nicht mehr mag, koennen B und C streiten sich
   */

#include "icc_examin.h"
#include "icc_betrachter.h"
namespace icc_examin_ns {

  // Pruefvariablen
  static int icc_thread_lock_zaehler_ = 0;
  Fl_Thread icc_thread_lock_besitzer_ = 0;

  int  awake(void)
  {
    Fl::awake(0);
    return 0;
  }
  int  leerWait(void) { return 0; }
  int  (*waitFunc)(void) = Fl::check;//awake;

  void lock(const char *file, int line)
  {
# if 0
    // Fuer Ungezaehlte, Ausserhaeusige und Besitzlose
    if(icc_thread_lock_zaehler_ == 0 ||
       (pthread_self() != icc_thread_lock_besitzer_ ||
        icc_thread_lock_besitzer_ == 0))
    { // Fehlersuche
      if(icc_thread_lock_besitzer_) {
        dbgThreadId(icc_thread_lock_besitzer_); DBG_THREAD_S( " Besitzer/Aufrufer locks: "<<icc_thread_lock_zaehler_ <<" Aufruf bei: "<<file<<":"<<line )
      } else {
        DBG_THREAD_S( "locks: "<<icc_thread_lock_zaehler_ <<" Aufruf bei: "<<file<<":"<<line )
      }
      // ... ins Haus gehen
      Fl::lock();
      // ... in Besitz nehmen
      icc_thread_lock_besitzer_ = pthread_self();
    } DBG_THREAD
# else
      DBG_THREAD_S( "locks: "<<icc_thread_lock_zaehler_ <<" anhalten bei: "<<file<<":"<<line )
      Fl::lock(); DBG_THREAD
      // ... in Besitz nehmen
      icc_thread_lock_besitzer_ = pthread_self();
# endif
    // ... Pruefnummer im Haus
    ++icc_thread_lock_zaehler_;
    //Fl::awake();
    DBG_THREAD_S( "locks: "<<icc_thread_lock_zaehler_ <<" angehalten bei: "<<file<<":"<<line )
  }

  void unlock(void *widget, const char *file, int line)
  {
# if 0
    --icc_thread_lock_zaehler_;
    DBG_THREAD_S( "locks: "<<icc_thread_lock_zaehler_ <<" Aufruf bei: "<<file<<":"<<line )
    // Fehlersuche
    if(pthread_self() != icc_thread_lock_besitzer_ &&
       icc_thread_lock_besitzer_ != 0) {
      dbgThreadId(icc_thread_lock_besitzer_);DBG_THREAD_S( "locks: " << icc_thread_lock_zaehler_ << " Besitzer/Aufrufer" )
    }
    // drausen nichts unternehmen, da die offene Tuer "gut" ist 
    if(icc_thread_lock_zaehler_ >= 0)
      Fl::unlock();
    // zuruecksetzen, da das Schloss nun offen ist
# else
      Fl::unlock();
# endif

    // hinausgehen und Tuere offen lassen
    if(widget) { DBG_THREAD
      Fl::awake(widget); DBG_THREAD
      // Ereignisse entlocken
      //Fl::wait(0); DBG_THREAD
    }

    icc_thread_lock_besitzer_ = 0;
    icc_thread_lock_zaehler_ = 0;

    // Oberflaeche erneuern
    DBG_THREAD_S( ": " << icc_thread_lock_zaehler_ << " weiter" )
  }
}


void
fl_delayed_redraw(void *w)
{
  ((Fl_Widget*)w)->redraw();
  DBG_PROG
  Fl::remove_idle(fl_delayed_redraw, w);
}


#include "my_file_chooser.h"
void
my_fl_translate_file_chooser( )
{
#ifdef USE_GETTEXT
  DBG_PROG_START
    MyFl_File_Chooser::add_favorites_label = _("Add to Favorites");
    MyFl_File_Chooser::all_files_label = _("All Files (*)");
    MyFl_File_Chooser::custom_filter_label = _("Custom Filter");
    MyFl_File_Chooser::existing_file_label = _("Please choose an existing file!");
    MyFl_File_Chooser::favorites_label = _("Favorites");
    MyFl_File_Chooser::filename_label = _("Filename");
    MyFl_File_Chooser::manage_favorites_label = _("Manage Favorites");
#   ifdef WIN32
    MyFl_File_Chooser::filesystems_label = _("My Computer");
#   else
    MyFl_File_Chooser::filesystems_label = _("File Systems");
#   endif
    MyFl_File_Chooser::new_directory_label = _("New Directory?");
    MyFl_File_Chooser::preview_label = _("Preview");
#   if (FL_MAJOR_VERSION == 1 && FL_MINOR_VERSION >= 1 && FL_PATCH_VERSION >= 7)
    MyFl_File_Chooser::save_label = _("Save"); // since 1.1.7?
#   endif
    MyFl_File_Chooser::show_label = _("Show:");
  DBG_PROG_ENDE
#endif
}


#endif
