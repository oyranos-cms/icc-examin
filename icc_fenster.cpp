/*
 * ICC Examin ist eine ICC Profil Betrachter
 * 
 * Copyright (C) 2004-2005  Kai-Uwe Behrmann 
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
 * Dateiwahl
 * 
 */

#include "icc_utils.h"
#include "icc_kette.h"
#include "icc_examin.h"
#include "icc_dateiwahl.h"
#include "icc_fenster.h"
#include <string>
#include <vector>

#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Return_Button.H>
#include <FL/Fl_Output.H>
#include <FL/Fl_Widget.H>
#include <FL/Fl_Scroll.H>


namespace icc_examin_ns {

#ifdef HAVE_FLTK
#ifdef HAVE_FLU

Flu_File_Chooser *dateiwahl;

void
dateiwahl_cb (const char *dateiname, int typ, void *arg)
{ DBG_PROG_START

  // kein Profile Dialog
  if (strstr( dateiwahl->pattern(), "*.ic*") == 0 &&
      // potentielle Messdaten
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
      std::vector<std::string> profilnamen;
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
dateiwahl_cb (MyFl_File_Chooser *f,void *data)
{ DBG_PROG_START

  const char *filename;

    MyFl_File_Chooser* fl = (MyFl_File_Chooser*)f;

    DBG_NUM_V( data )
    filename = fl->value();
  
    // kein Profile Dialog
    if (0 && strstr(fl->filter(), "Profile [*.{I,i}{C,c}]") == 0) {
      if (filename)
        DBG_PROG_V( filename )
      DBG_PROG_ENDE
      return;
    }

    if (filename && fl->count() && dateiwahl->preview()) {
      std::vector<std::string> profilnamen;
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
      DBG_PROG_V( profilnamen.size() << filename )
      icc_examin->oeffnen(profilnamen);
    }

  DBG_PROG_ENDE
}
#endif

MyFl_Double_Window* nachricht_ (std::string text); 

#if 1
MyFl_Double_Window*
nachricht (std::string text) {
  // Fuer Fl_Scroll wird keine vtable erzeugt:
  // icc_fenster.cpp:162: undefined reference to `icc_examin_ns::Fl_Scroll::Fl_Scroll[in-charge](int, int, int, int, char const*)'
  // Nun ist die eigentliche Funtion ausserhalb von icc_examin_ns::
  return nachricht_(text);
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

static Fl_Output *output_info=(Fl_Output *)0;

MyFl_Double_Window*
nachricht_ (std::string text) {
  MyFl_Double_Window* w;
  { MyFl_Double_Window* o = new MyFl_Double_Window(275, 326, _("Information:"));
    w = o;
    w->hotspot(o);
    { Fl_Return_Button* o = new Fl_Return_Button(60, 295, 160, 25, "Gut");
      o->shortcut(0xff0d);
      o->callback((Fl_Callback*)cb_Gut, (void*)(w));
      o->align(FL_ALIGN_CENTER|FL_ALIGN_INSIDE);
      w->hotspot(o);
    }
      { Fl_Output* o = output_info = new Fl_Output(0, 0, 275, 290);
        o->type(12);
        //o->color((Fl_Color)53);
        o->value(text.c_str());
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
MyFl_Double_Window *  MyFl_Double_Window::main_win = NULL;
int                   MyFl_Double_Window::n_ = 0;
int                   MyFl_Double_Window::ref_ = 0;
char               ** MyFl_Double_Window::icon = NULL;

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
  DBG_S( id_<<" "<<(user_hide?"u":" ")<<" "<<(visible()?"v":" ")<<" "<<(shown()?"s":" ") )
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

  Fl_Double_Window::xclass("iccexamin");
  Fl_Double_Window::show();
  user_hide = true;

# if HAVE_X
  if(/*this == main_win &&*/ icon)
    setzeIcon( main_win, icon );
# endif

  DBG_S( id_<<" "<<(user_hide?"u":" ")<<" "<<(visible()?"v":" ")<<" "<<(shown()?"s":" ") )
}

void MyFl_Double_Window::hide()
{
  DBG_S( id_<<" "<<(user_hide?"u":" ")<<" "<<(visible()?"v":" ")<<" "<<(shown()?"s":" ") )
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
    Fl_Double_Window::iconize();
  else
    Fl_Double_Window::hide();
  user_hide = true;
  DBG_S( id_<<" "<<(user_hide?"u":" ")<<" "<<(visible()?"v":" ")<<" "<<(shown()?"s":" ") )
}

void MyFl_Double_Window::hide(MyFl_Double_Window * by)
{
  DBG_S( id_<<" "<<(user_hide?"u":" ")<<" "<<(visible()?"v":" ")<<" "<<(shown()?"s":" ") )
  hide();
  user_hide = false;
  DBG_S( id_<<" "<<(user_hide?"u":" ")<<" "<<(visible()?"v":" ")<<" "<<(shown()?"s":" ") )
}

void MyFl_Double_Window::iconize()
{
  DBG_S( id_<<" "<<(user_hide?"u":" ")<<" "<<(visible()?"v":" ")<<" "<<(shown()?"s":" ") )
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
  DBG_S( id_<<" "<<(user_hide?"u":" ")<<" "<<(visible()?"v":" ")<<" "<<(shown()?"s":" ") )
}

void MyFl_Double_Window::iconize(MyFl_Double_Window * by)
{
  DBG_S( id_<<" "<<(user_hide?"u":" ")<<" "<<(visible()?"v":" ")<<" "<<(shown()?"s":" ") )
  if(this == main_win)
    Fl_Double_Window::iconize();
  else
    iconize();
  user_hide = false;
  DBG_S( id_<<" "<<(user_hide?"u":" ")<<" "<<(visible()?"v":" ")<<" "<<(shown()?"s":" ") )
}

int MyFl_Double_Window::handle( int e )
{
  int ergebnis = tastatur(e);

  if(!ergebnis)
    ;

  int fl_window_events = 1;
  switch(e)
  {
    case FL_HIDE:
         if(user_hide)
         {
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
         show();
         fl_window_events = 0;
         break;
  }
  DBG_S( id_<<" "<<(user_hide?"u":" ")<<" "<<(visible()?"v":" ")<<" "<<(shown()?"s":" ")<<" "<<dbgFltkEvent(e) )

  if(use_escape_hide)
  if(e == FL_SHORTCUT && Fl::event_key() == FL_Escape)
    hide();

  if(fl_window_events)
    return Fl_Double_Window::handle(e);
  else
    return ergebnis;
}


}
