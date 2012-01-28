/*
 * ICC Examin ist eine ICC Profil Betrachter
 * 
 * Copyright (C) 2004-2011  Kai-Uwe Behrmann 
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
 * file selector and other windows
 * 
 */

// Date:      Januar 2005

#ifndef ICC_FENSTER_H
#define ICC_FENSTER_H

#include <string>

#include "icc_utils.h"

//#if HAVE_FLTK
#include <FL/Fl_Hold_Browser.H> 
#include <FL/Fl_File_Icon.H> 
#include <FL/Fl_Shared_Image.H> 
#include <FL/Fl_PNM_Image.H>
#include <FL/Fl_Double_Window.H> 
#include <FL/fl_ask.H>

#include "Oy_Fl_Window_Base.h"

namespace icc_examin_ns {


  //! register all windows inside this class, to adjust behaviour
  class MyFl_Double_Window : public Fl_Double_Window, public Oy_Fl_Window_Base
  {

     static MyFl_Double_Window ** list_;
     static int n_;              //!< available entries in list_
     static int ref_;            //!< used entries in list_
     int    id_;                 //!< identification number
     int    desktop_;            //!< Dekstop number - _NET_CURRENT_DESKTOP 
     void   init_class_();       //!< initialisation if the static elements
     void   init_object_();      //!< initialisation of the dynamic objects
     char   titel_[256];         //!< window title

   public:

     MyFl_Double_Window(int X, int Y, int W, int H, const char *title = 0);
     MyFl_Double_Window(int W, int H, const char* title = 0);
     ~MyFl_Double_Window (); 

     static const char** icon;              //!< Icon (X11 - xpm)
     static const char * my_xclass;         //!< Fl_Window::xclass string
     static MyFl_Double_Window *  main_win; //!< the main window
     MyFl_Double_Window * only_with;        //!< observed window (WM_TRANSIENT_FOR ?)

     //! user generated hide : hide() -> true : hide(*) -> false
     int user_hide;              //!< hidden with hide(void)
     int use_escape_hide;        //!< allow to hide with Escape
     int is_toolbox;             //!< toolbox

     void hide (void);           //!< hide for user
     void hide (MyFl_Double_Window * by); //!< hide automatic
     void iconize (void);        //!< minimise for user
     void iconize (MyFl_Double_Window * by); //!< minimise automatic
     void show (void);           //!< show for all
     void show (int, char**);    //!< show for all
  private:
     enum {HIDE, SHOW, ICONIZE, HANDLE, FUNC_MAX};
     int in_middle_of_[FUNC_MAX];      //!< status variable
  public:
     void label (const char * t); //!< set window title
     const char* label() { return Fl_Double_Window::label(); }

     int  handle (int e);        //!< query events (keys, FL_HIDE...)
     static int  (*event_handler) (int e);//!< general application event handler
  };

  //! window with news
  MyFl_Double_Window* nachricht(std::string text);
  MyFl_Double_Window* log(std::string text, int level);
  void log_show(void);
  extern MyFl_Double_Window *log_window;
  extern int log_window_poped;

  void        setzeIcon    ( Fl_Window    *fenster,
                             const char  **xpm_daten );
}

//#endif

#endif //ICC_FENSTER_H

