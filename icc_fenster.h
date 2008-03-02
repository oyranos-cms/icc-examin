/*
 * ICC Examin ist eine ICC Profil Betrachter
 * 
 * Copyright (C) 2004-2006  Kai-Uwe Behrmann 
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
 * Dateiwahl und andere Fenster
 * 
 */

// Date:      Januar 2005

#ifndef ICC_FENSTER_H
#define ICC_FENSTER_H

#include "icc_utils.h"

//#if HAVE_FLTK
#include <FL/Fl_Hold_Browser.H> 
#include <FL/Fl_File_Icon.H> 
#include <FL/Fl_Shared_Image.H> 
#include <FL/Fl_PNM_Image.H>
#include <FL/Fl_Double_Window.H> 
#include <FL/fl_ask.H>



namespace icc_examin_ns {


  //! register all windows inside this class, to adjust behaviour
  class MyFl_Double_Window : public Fl_Double_Window
  {

     static MyFl_Double_Window ** list_;
     static int n_;              //!< verfuegbare Plaetze in list_
     static int ref_;            //!< genutzte Plaetze in list_
     int    id_;                 //!< Identifikationsnummer
     void   init_class_();       //!< Initialisierung der statischen Elemente
     void   init_object_();      //!< Initialisierung des dynamischen Objektes
     char   titel_[256];         //!< Fenstertitel

   public:

     MyFl_Double_Window(int X, int Y, int W, int H, const char *title = 0);
     MyFl_Double_Window(int W, int H, const char* title = 0);
     ~MyFl_Double_Window (); 

     static char ** icon;                   //!< Icon (X11 - xpm)
     static const char * my_xclass;         //!< Fl_Window::xclass string
     static MyFl_Double_Window *  main_win; //!< das Programmhauptfenster
     MyFl_Double_Window * only_with;        //!< beobachtetes Fenster (WM_TRANSIENT_FOR ?)

     //! Benutzer ausgeloestes Verstecken : hide() -> true : hide(*) -> false
     int user_hide;              //!< mit hide(void) versteckt
     int use_escape_hide;        //!< erlaubt mit Escape zu verkleinern
     int is_toolbox;             //!< Werkzeugkasten

     void hide (void);           //!< Verstecken fuer Benutzer 
     void hide (MyFl_Double_Window * by); //!< Verstecken automatisch
     void iconize (void);        //!< Verkleinern fuer benutzer
     void iconize (MyFl_Double_Window * by); //!< Verkleinern automatisch
     void show (void);           //!< Zeigen fuer alle
     void label (const char * t); //!< Fenstertitel setzen
     const char* label() { return Fl_Double_Window::label(); }

     int  handle (int e);        //!< Ereignisse abfangen (Tastatur, FL_HIDE...)
  };

  //! Fenster mit Nachrichten
  MyFl_Double_Window* nachricht(std::string text);

}

//#endif

#endif //ICC_FENSTER_H

