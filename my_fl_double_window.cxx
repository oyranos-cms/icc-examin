/*
 * ICC Examin is a ICC colour profile viewer
 * 
 * Copyright (C) 2004-2012  Kai-Uwe Behrmann 
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
 * file selection
 * 
 */

#include "icc_fenster.h"
#include "icc_helfer_fltk.h"
#include "icc_utils.h"

#ifdef NO_ICC
#undef DBG_PROG_S
#define DBG_PROG_S(t)
#undef WARN_S
#define WARN_S(t)
#else
std::string dbgFltkEvent   ( int           event);
#endif

#include <string>

#include <FL/Fl.H>
#if defined(HAVE_X) && !defined(__APPLE__)
#include <FL/x.H>
#include <X11/Xutil.h>
#include <X11/xpm.h>
#include <X11/extensions/shape.h>
#endif
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Return_Button.H>
#include <FL/Fl_Text_Display.H>
#include <FL/Fl_Output.H>
#include <FL/Fl_Widget.H>
#include <FL/Fl_Scroll.H>

namespace icc_examin_ns {

#define IN_MIDDLE_OF_(n) { DBG_NUM_S("Break signal loop "<<n); /*return;*/ }

MyFl_Double_Window ** MyFl_Double_Window::list_ = NULL;
MyFl_Double_Window  * MyFl_Double_Window::main_win = NULL;
int                   MyFl_Double_Window::n_ = 0;
int                   MyFl_Double_Window::ref_ = 0;
const char         ** MyFl_Double_Window::icon = NULL;
const char          * MyFl_Double_Window::my_xclass = NULL;
int                (* MyFl_Double_Window::event_handler) (int e) = 0;

MyFl_Double_Window::MyFl_Double_Window(int W, int H, const char* title)
  : Fl_Double_Window(W,H,title)
{
  init_object_();
}

MyFl_Double_Window::MyFl_Double_Window(int X, int Y, int W, int H, const char *title)
  : Fl_Double_Window(X,Y,W,H,title)
{
  init_object_();
}

void
MyFl_Double_Window::init_object_ ()
{
  desktop_ = 0;
  memset(titel_, 0, 256);

  if(n_ <= 0)
    init_class_();

  id_ = ref_;
  ++ref_;

  if (n_ < ref_)
  {
    MyFl_Double_Window ** ptr_ = (MyFl_Double_Window**) calloc( sizeof(MyFl_Double_Window*), 2 * n_ );
    for(int i = 0; i < n_; ++i)
      ptr_[i] = list_[i];
    free(list_);
    list_ = ptr_;
  }

  list_[id_] = this;


  if(main_win)
    only_with = main_win;
  else
    only_with = NULL;

  user_hide = true;
  use_escape_hide = false;
  is_toolbox = false;
  for (int i = 0; i < FUNC_MAX; ++i)
    in_middle_of_[i] = 0;
}

MyFl_Double_Window::~MyFl_Double_Window()
{
  if(ref_-- == 1)
    free(list_);
}

void
MyFl_Double_Window::init_class_ ()
{
  if(!list_ || !n_)
  {
    n_ = 24;
    list_ = (MyFl_Double_Window**) calloc( sizeof(MyFl_Double_Window*), n_ );
  }
}

void MyFl_Double_Window::show()
{
  this->show( 0, NULL );
}

void MyFl_Double_Window::show(int argc, char** argv)
{
  if(in_middle_of_[SHOW])
    IN_MIDDLE_OF_(in_middle_of_[SHOW])
  in_middle_of_[SHOW] += 1;

  DBG_PROG_S( id_<<" "<<(user_hide?"u":" ")<<" "<<(visible()?"v":" ")<<" "<<(shown()?"s":" ") )
  for( int i = 0; i < ref_; ++i )
  {
    MyFl_Double_Window *w = list_[i];
    if(!w->only_with)
      w->only_with = main_win;
    if( w != this &&
        w -> only_with == this &&
        !w -> user_hide )
      w -> show();
  }

  if(this->my_xclass)
    Fl_Double_Window::xclass(my_xclass);

  if(!this->visible() || !this->shown())
  {
    if(argc)
      Fl_Double_Window::show((int)argc, (char**)argv);
    else
      Fl_Double_Window::show();

#if !defined(WIN32) && !defined(__APPLE__)
    Fl_Window::show();
#endif
  }

  if(/*this == main_win &&*/ icon &&
     this->visible() && this->shown())
    setzeIcon( this, icon );

#if defined(HAVE_X) && !defined(__APPLE__)
  if( is_toolbox )
  {
#if 0
    // seems not to be correct under KDE
    Atom type = XInternAtom (fl_display, "_NET_WM_WINDOW_TYPE", 0);
    Atom value = XInternAtom (fl_display, "_NET_WM_WINDOW_TYPE_UTILITY", 0);
    XChangeProperty (fl_display, fl_xid(this), type, XA_ATOM, 32,
                PropModeAppend, (unsigned char*) &value, 1);
#endif
    for( int i = 0; i < ref_; ++i )
    {
      MyFl_Double_Window *w = list_[i];
      if(!w->only_with)
        w->only_with = main_win;
      if( w != this && w->shown() )
      {
        if( w -> is_toolbox && w -> only_with == this )
          XSetTransientForHint(fl_display, fl_xid(this), fl_xid(w));
        if( this -> is_toolbox && this -> only_with == w  )
          XSetTransientForHint(fl_display, fl_xid(w), fl_xid(this));
      }        
    }
  }
# endif

  user_hide = true;

  in_middle_of_[SHOW] = 0;

  DBG_PROG_S( id_<<" "<<(user_hide?"u":" ")<<" "<<(visible()?"v":" ")<<" "<<
              (shown()?"s":" ") )
}

void MyFl_Double_Window::hide()
{
  if(in_middle_of_[HIDE])
    IN_MIDDLE_OF_(in_middle_of_[HIDE])
  in_middle_of_[HIDE] += 1;

  DBG_PROG_S( id_<<" "<<(user_hide?"u":" ")<<" "<<(visible()?"v":" ")<<" "<<
              (shown()?"s":" ") )
  if(!only_with)
    only_with = main_win;

  for( int i = 0; i < ref_; ++i )
  {
    MyFl_Double_Window *w = list_[i];
    if( w != this &&
        w -> only_with == this &&
        w -> visible() )
      w -> hide(this);
  }

  if(this == main_win)
    exit(0);
  else
    Fl_Double_Window::hide();
  user_hide = true;

  in_middle_of_[HIDE] = 0;

  DBG_PROG_S( id_<<" "<<(user_hide?"u":" ")<<" "<<(visible()?"v":" ")<<" "<<
              (shown()?"s":" ") )
}

void MyFl_Double_Window::hide(MyFl_Double_Window * by)
{
  DBG_PROG_S( id_<<" "<<(user_hide?"u":" ")<<" "<<(visible()?"v":" ")<<" "<<
              (shown()?"s":" ") )
  hide();
  user_hide = false;
  DBG_PROG_S( id_<<" "<<(user_hide?"u":" ")<<" "<<(visible()?"v":" ")<<" "<<
              (shown()?"s":" ") )
}

void MyFl_Double_Window::iconize()
{
  if(in_middle_of_[ICONIZE])
    IN_MIDDLE_OF_(in_middle_of_[ICONIZE])
  in_middle_of_[ICONIZE] += 1;

  DBG_PROG_S( id_<<" "<<(user_hide?"u":" ")<<" "<<(visible()?"v":" ")<<" "<<
              (shown()?"s":" ") )
  if(!only_with)
    only_with = main_win;

  for( int i = 0; i < ref_; ++i )
  {
    MyFl_Double_Window *w = list_[i];
    if( w != this &&
        w -> only_with == this &&
        w -> visible() )
      w -> iconize(this);
  }

  Fl_Double_Window::iconize();
  user_hide = true;

  in_middle_of_[ICONIZE] = 0;

  DBG_PROG_S( id_<<" "<<(user_hide?"u":" ")<<" "<<(visible()?"v":" ")<<" "<<
              (shown()?"s":" ") )
}

void MyFl_Double_Window::iconize(MyFl_Double_Window * by)
{
  DBG_PROG_S( id_<<" "<<(user_hide?"u":" ")<<" "<<(visible()?"v":" ")<<" "<<
              (shown()?"s":" ") )
  if(this == main_win)
    Fl_Double_Window::iconize();
  else
    iconize();
  user_hide = false;
  DBG_PROG_S( id_<<" "<<(user_hide?"u":" ")<<" "<<(visible()?"v":" ")<<
              " "<<(shown()?"s":" ") )
}

int MyFl_Double_Window::handle( int e )
{
  if(in_middle_of_[HANDLE])
    IN_MIDDLE_OF_(in_middle_of_[HANDLE])
  in_middle_of_[HANDLE] += 1;

  // update window position
  Oy_Fl_Window_Base::handle(this, e);

  int ergebnis = 0;
  if(event_handler)
    ergebnis = event_handler(e);

  int zeigen = (e == FL_HIDE || e == FL_SHOW);

  int x_,y_,w_,h_;
  x_=y_=w_=h_=0;
  Fl::screen_xywh(x_,y_,w_,h_);

  int net_desktop = -1;
#if defined(HAVE_X) && !defined(__APPLE__)
  if(fl_display && zeigen)
  {
    Atom atom = XInternAtom (fl_display, "_NET_CURRENT_DESKTOP", 0), a;
    Window w = RootWindow( fl_display, DefaultScreen(fl_display));
    int actual_format_return;
    unsigned long nitems_return=0, bytes_after_return=0;
    unsigned char* prop_return=0;
    if(atom)
      XGetWindowProperty(fl_display, w, atom, 0, 32, 0, AnyPropertyType, &a,
                     &actual_format_return, &nitems_return, &bytes_after_return,
                     &prop_return );

    if(prop_return)
      net_desktop = prop_return[0];
  }
#endif

  int fl_window_events = 1;
  switch(e)
  {
    case FL_HIDE:
         if(user_hide)
         {
           if(net_desktop >= 0 && net_desktop != desktop_)
             break;
           if(this->shown())
             iconize();
           else
             hide();
         } else {
           if(this->shown())
             iconize(this);
           else
             hide(this);
         }
         fl_window_events = 0;
         break;
    case FL_SHOW:
         if(net_desktop >= 0 && net_desktop != desktop_)
           break;
         show();
         fl_window_events = 0;
         break;
  }
  //DBG_PROG_S( id_<<" "<<(user_hide?"u":" ")<<" "<<(visible()?"v":" ")<<" "<<
  //            (shown()?"s":" ")<<" "<<dbgFltkEvent(e) )

  if(use_escape_hide)
  if(e == FL_SHORTCUT && Fl::event_key() == FL_Escape)
    hide();

  if(zeigen && net_desktop >= 0 && this->shown() && this->visible())
    desktop_ = net_desktop;

  in_middle_of_[HANDLE] = 0;

  if(fl_window_events && !ergebnis)
    ergebnis = Fl_Double_Window::handle(e);

  return ergebnis;
}

void MyFl_Double_Window::label( const char *titel )
{
  if(titel)
    snprintf(titel_,256, "%s", titel);
  if(strlen(titel_))
    Fl_Double_Window::label(titel_);
}


void setzeIcon      ( Fl_Window *fenster, const char   **xpm_daten )
{
# if HAVE_X && !defined(__APPLE__)
  fl_open_display();
  if(!fenster) {
    WARN_S("no window provided")
    return;
  }
  fenster->make_current();
  DBG_PROG_V( (int*) fl_display <<" "<< fl_window )
  Pixmap pm, mask;
# if HAVE_Xpm
  XpmCreatePixmapFromData(  fl_display,
                            DefaultRootWindow(fl_display),
                            const_cast<char**> (xpm_daten),
                            &pm,
                            &mask,
                            NULL);
# endif
# if 0
  XShapeCombineMask(fl_display, fl_window,
                    ShapeBounding,0,0,
                    mask,ShapeSet);
# endif
  //fenster->icon((char*)p); // die FLTK Methode

  XWMHints *hints;
  hints = XGetWMHints( fl_display, fl_window );
  if (!hints)
  {
    hints = XAllocWMHints();
    WARN_S("XGetWMHints() failed")
  }
  hints->flags = IconPixmapHint;
  hints->icon_pixmap = pm;
  hints->flags |= IconMaskHint;
  hints->icon_mask = mask;
  XSetWMHints( fl_display, fl_window, hints );
  XFree( hints );
# endif
}
} // namespace icc_examin_ns
