/*
 * ICC Examin ist eine ICC Profil Betrachter
 * 
 * Copyright (C) 2004-2008  Kai-Uwe Behrmann 
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

//#include "icc_utils.h"
//#include "icc_kette.h"
#include "icc_examin.h"
#include "icc_dateiwahl.h"
#include "icc_fenster.h"

#include <string>

#include <FL/Fl.H>
#if HAVE_X
#include <FL/x.H>
#include <X11/Xutil.h>
#endif
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Return_Button.H>
#include <FL/Fl_Text_Display.H>
#include <FL/Fl_Output.H>
#include <FL/Fl_Widget.H>
#include <FL/Fl_Scroll.H>


namespace icc_examin_ns {

#define IN_MIDDLE_OF_(n) { DBG_NUM_S("Break signal loop "<<n); /*return;*/ }

#ifdef HAVE_FLTK
#ifdef HAVE_FLU

Flu_File_Chooser *dateiwahl;

void
dateiwahl_cb (const char *dateiname, int typ, void *arg)
{ DBG_PROG_START

  // no profile dialog
  if (strstr( dateiwahl->pattern(), "*.ic*") == 0 &&
      // potential measurements
      strstr( dateiwahl->pattern(), "*.txt") == 0 &&
      strstr( dateiwahl->pattern(), "*.TXT") == 0 &&
      strstr( dateiwahl->pattern(), "*.it8") == 0 &&
      strstr( dateiwahl->pattern(), "*.IT8") == 0 &&
      strstr( dateiwahl->pattern(), "*.CMYK") == 0 &&
      strstr( dateiwahl->pattern(), "*.DLY") == 0 &&
      strstr( dateiwahl->pattern(), "*.nCIE") == 0 &&
      strstr( dateiwahl->pattern(), "*.oRPT") == 0 &&
      strstr( dateiwahl->pattern(), "*.LAB") == 0 &&
      strstr( dateiwahl->pattern(), "*.Q60") == 0 &&
      strstr( dateiwahl->pattern(), "*.IC*") == 0 )
  {
    DBG_PROG_ENDE
    //return;
  }

    if (dateiname)
    {
      ICClist<std::string> profilnamen;
      profilnamen.resize(1);
      //profilnamen[0] = dateiname;

      DBG_NUM_V( profile )
      profilnamen[0] = dateiwahl->get_current_directory();
      profilnamen[0].append( dateiname );
      DBG_NUM_V( profilnamen[0] )
      icc_examin->oeffnen( profilnamen );
    }

  DBG_PROG_ENDE
}
#else
MyFl_File_Chooser  *dateiwahl;

void
dateiwahl_cb (MyFl_File_Chooser *f, void *data, int finish)
{ DBG_PROG_START

  const char *filename;

    MyFl_File_Chooser* fl = (MyFl_File_Chooser*)f;

    DBG_NUM_V( data )
    filename = fl->value();

    static ICClist<std::string> file_vect;

    if (filename && fl->count() && dateiwahl->preview()) {
      ICClist<std::string> profilnamen;
      profilnamen.resize(fl->count());
      for (int i = 0; i < fl->count(); i++) {
        if(strchr(fl->value(i), '/') == 0) {
          profilnamen[i] = fl->directory();
          profilnamen[i].append("/");
          profilnamen[i].append(fl->value(i));
        } else 
          profilnamen[i] = fl->value(i);
        DBG_PROG_V( i <<":"<< profilnamen[i] )
      }

      // add new files
      int file_in_list;
      for(int i = 0; i < (int)profilnamen.size(); ++i)
      {
        file_in_list = 0;
        for( int j = 0; j < (int)file_vect.size(); ++j )
          if(profilnamen[i] == file_vect[j])
          {
            file_in_list = 1;
            break;
          }

        if(!file_in_list)
          file_vect.push_back( profilnamen[i] );
      }

      // remove unseen files
      int run = 1;

      while(run)
      {
        run = 0;
        for(int i = 0; i < (int)file_vect.size(); ++i)
        {
          file_in_list = 0;
          for(int j = 0; j < (int)profilnamen.size(); ++j)
            if(profilnamen[j] == file_vect[i])
            {
              file_in_list = 1;
              break;
            }

          if(!file_in_list)
          {
            file_vect.erase( &file_vect[i] );
            run = 1;
            break;
          }
        }
      }

      if(profilnamen.size() != file_vect.size())
        WARN_S("Something went wrong here.");

      DBG_PROG_V( file_vect.size() << filename )
      icc_examin->oeffnen( file_vect );
    }

  DBG_PROG_ENDE
}
#endif

MyFl_Double_Window* nachricht_ (std::string text); 
MyFl_Double_Window* log_ (std::string text, int code); 

#if 1
MyFl_Double_Window*
nachricht (std::string text) {
  // for Fl_Scroll no vtable created:
  // icc_fenster.cpp:162: undefined reference to `icc_examin_ns::Fl_Scroll::Fl_Scroll[in-charge](int, int, int, int, char const*)'
  // Now the funtion is outside of icc_examin_ns::
  return nachricht_(text);
}
MyFl_Double_Window*
log (std::string text, int code) {
  // for Fl_Scroll no vtable created:
  // icc_fenster.cpp:162: undefined reference to `icc_examin_ns::Fl_Scroll::Fl_Scroll[in-charge](int, int, int, int, char const*)'
  // Now the funtion is outside of icc_examin_ns::
  return log_(text, code);
}

#else
MyFl_Double_Window*
nachricht(std::string text)
{ DBG_PROG_START
  fl_message_icon()->resize(1,1,1,1);
  fl_message_icon()->hide();
  DBG_PROG
  fl_message(text.c_str());
  DBG_PROG_ENDE
}
#endif

#endif

static void cb_Gut(Fl_Return_Button*, void* v) {
  ((MyFl_Double_Window*)v)->hide();
}

static Fl_Text_Display *display_info=(Fl_Text_Display*)0;
static Fl_Text_Display *display_log=(Fl_Text_Display*)0;
static Fl_Text_Buffer *buffer_log=(Fl_Text_Buffer*)0;
MyFl_Double_Window *log_window=(MyFl_Double_Window*)0;
int log_window_poped = 0;
static ICCThreadList<int> icc_log_lock_;

int iccCLIMessage( const char * text, int code )
{
  cout << text << endl;
  return 0;
}

MyFl_Double_Window*
log_ (std::string text, int code)
{
  int log_window_new = 0;

  /* do not go over expensive log window in case we want explicitely debug */
  if(icc_debug)
  {
    iccCLIMessage( text.c_str(), code );
    return 0;
  }

  icc_log_lock_.frei(false);

  if(icc_examin && icc_examin->laeuft() && !log_window)
  {
    MyFl_Double_Window* w = log_window = new MyFl_Double_Window(600, 226, _("Log:"));
    w->hotspot(w);
    { Fl_Return_Button* o = new Fl_Return_Button(220, 195, 160, 25, _("Yes"));
      o->shortcut(0xff0d);
      o->callback((Fl_Callback*)cb_Gut, (void*)(w));
      o->align(FL_ALIGN_CENTER|FL_ALIGN_INSIDE);
      w->hotspot(o);
    }
      { Fl_Text_Display* o = display_log = new Fl_Text_Display(0, 0, 600, 190);
        //o->type(12);
        //o->color((Fl_Color)53);
        o->textfont( FL_COURIER );
        o->box( FL_FLAT_BOX );
        o->color(FL_BACKGROUND_COLOR);
        Fl_Group::current()->resizable(o);
      }
    //w->show();
    w->end();
    w->use_escape_hide = true;

    log_window_new = 1;
  }

  if(!buffer_log)
    buffer_log = new Fl_Text_Buffer(0);

  if(display_log && buffer_log && !display_log->buffer())
    display_log->buffer(buffer_log);

  if(buffer_log)
  {
    int l = buffer_log->length();
    int ip = -1;

    if(display_log)
      ip = display_log->insert_position();

    buffer_log->append( text.c_str() );

    if(display_log && l == ip || log_window_new)
    {
      ip = buffer_log->length();
      display_log->insert_position( ip );
      display_log->show_insert_position();
    }

    if(code == ICC_MSG_ERROR &&
       log_window &&
       icc_examin &&
       icc_examin->icc_betrachter &&
       !log_window_poped)
    {
      log_window_poped = 1;
      while(!icc_examin->frei())
        icc_examin_ns::sleep(0.01);
      log_window->show();
    }
  }

  icc_log_lock_.frei(true);
  return log_window;
}

MyFl_Double_Window*
nachricht_ (std::string text) {
  MyFl_Double_Window* w;
  { MyFl_Double_Window* o = new MyFl_Double_Window(275, 326, _("Information:"));
    w = o;
    w->hotspot(o);
    { Fl_Return_Button* o = new Fl_Return_Button(60, 295, 160, 25, _("Yes"));
      o->shortcut(0xff0d);
      o->callback((Fl_Callback*)cb_Gut, (void*)(w));
      o->align(FL_ALIGN_CENTER|FL_ALIGN_INSIDE);
      w->hotspot(o);
    }
      { Fl_Text_Display* o = display_info = new Fl_Text_Display(0, 0, 275, 290);
        //o->type(12);
        //o->color((Fl_Color)53);
        Fl_Text_Buffer * buffer = new Fl_Text_Buffer(0);
        buffer->append( text.c_str() );
        o->buffer( buffer );
        o->textfont( FL_COURIER );
        o->box( FL_FLAT_BOX );
        o->color(FL_BACKGROUND_COLOR);
        Fl_Group::current()->resizable(o);
      }
    o->show();
    o->end();
    o->use_escape_hide = true;
  }
  //output_info->value(text.c_str());
  return w;
}

}



namespace icc_examin_ns {

MyFl_Double_Window ** MyFl_Double_Window::list_ = NULL;
MyFl_Double_Window  * MyFl_Double_Window::main_win = NULL;
int                   MyFl_Double_Window::n_ = 0;
int                   MyFl_Double_Window::ref_ = 0;
const char         ** MyFl_Double_Window::icon = NULL;
const char          * MyFl_Double_Window::my_xclass = NULL;

MyFl_Double_Window::MyFl_Double_Window(int W, int H, const char* title)
  : Fl_Double_Window(W,H,title)
{
  desktop_ = 0;
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

  if(/*this == main_win &&*/ icon)
    setzeIcon( this, icon );

# if HAVE_X
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
    icc_examin->quit();
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

  int ergebnis = tastatur(e);
  int zeigen = (e == FL_HIDE || e == FL_SHOW);


  int x_,y_,w_,h_,wx,wy;
  x_=y_=w_=h_=wx=wy=0;
#if ((FL_MAJOR_VERSION * 100 + FL_MINOR_VERSION) > 106)
  Fl::screen_xywh(x_,y_,w_,h_);
#endif
  wx = this->x();
  wy = this->y();

  /*if(!ergebnis)
    ;*/

  int net_desktop = -1;
#if HAVE_X
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
  DBG_PROG_S( id_<<" "<<(user_hide?"u":" ")<<" "<<(visible()?"v":" ")<<" "<<
              (shown()?"s":" ")<<" "<<dbgFltkEvent(e) )

  if(use_escape_hide)
  if(e == FL_SHORTCUT && Fl::event_key() == FL_Escape)
    hide();

  if(zeigen && net_desktop >= 0 && this->shown() && this->visible())
    desktop_ = net_desktop;

  in_middle_of_[HANDLE] = 0;

  if(fl_window_events)
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


} // namespace icc_examin_ns
