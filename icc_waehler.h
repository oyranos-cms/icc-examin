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
 * Die Wähler von Profilen Klasse
 * 
 */

// Date:      Maerz 2005

#ifndef ICC_WAEHLER_H
#define ICC_WAEHLER_H

#include <string>
#include <vector>
#include "icc_utils.h"
#include "icc_fenster.h"

#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Scroll.H>
#include <FL/Fl_Pack.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Output.H>
#include <FL/Fl_Value_Slider.H>
#include <FL/Fl_Light_Button.H>

// Einstellungen direkt in icc_examin->icc_betrachter->DD_farbraum und 
// in ICCkette::profile syncronisieren

class ICCwaehlerProfil : public Fl_Pack
{
  int pos_;
  Fl_Button *aktiv_knopf_;
 void aktiv_knopf_cb_()
 {
  bool a = aktiv();
  aktivieren( a );
  icc_examin->icc_betrachter->DD_farbraum->invalidate();
  icc_examin->icc_betrachter->DD_farbraum->flush();
 }
 static void aktiv_knopf_cb_statisch_(Fl_Widget* w, void* data) {DBG_PROG_START
                ICCwaehlerProfil* obj = dynamic_cast<ICCwaehlerProfil*>(w->parent());
                if(obj) 
                  obj->aktiv_knopf_cb_();
                else WARN_S( "not a ICCwaehlerProfil class??" )
                if(!w) WARN_S( "not a Fl_Widget class??" )
                DBG_PROG_ENDE
              }
  Fl_Pack   *gruppe_;
  Fl_Output *name_;
  Fl_Value_Slider *undurchsicht_;
 void undurchsicht_cb_()
 {
  double u = undurchsicht();
  undurchsicht( u );
  icc_examin->icc_betrachter->DD_farbraum->invalidate();
  icc_examin->icc_betrachter->DD_farbraum->flush();
 }
  static void undurchsicht_cb_statisch_(Fl_Widget* w, void* data) {
                ICCwaehlerProfil* obj = dynamic_cast<ICCwaehlerProfil*>(w->parent()->parent());
                if(obj)
                  obj->undurchsicht_cb_();
              }
  Fl_Light_Button *grau_;
 void grau_cb_()
 {
  bool g = grau();
  grau( g );
  icc_examin->icc_betrachter->DD_farbraum->invalidate();
  icc_examin->icc_betrachter->DD_farbraum->flush();
 }
  static void grau_cb_statisch_(Fl_Widget* w, void* data) {
                ICCwaehlerProfil* obj = dynamic_cast<ICCwaehlerProfil*>(w->parent()->parent());
                if(obj) 
                  obj->grau_cb_();
              }
public:
 ICCwaehlerProfil(const char* name, double undurchsicht, 
                   bool grau, bool aktiv, int pos)
    : Fl_Pack( 0,0,470,25 ), pos_(pos)
 {
  DBG_PROG_START
  DBG_PROG_V( name )
  DBG_PROG_V( undurchsicht )
  DBG_PROG_V( grau )
  DBG_PROG_V( aktiv )
  {
          begin(); DBG_PROG
          type(1);
          { Fl_Button* o = aktiv_knopf_ = new Fl_Button(6, 6, 25, 25);
            o->type(1);
            //o->selection_color(FL_WHITE);
            o->value( aktiv );
            o->callback(aktiv_knopf_cb_statisch_);
          }
          { Fl_Pack* o = gruppe_ = new Fl_Pack(29, 6, 445, 25);
            o->type(1);
            if(aktiv) o->activate(); else o->deactivate();
            { Fl_Output* o = name_ = new Fl_Output(29, 6, 230, 25);
              o->align(FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
              o->value( name );
              this->resizable(o);
            }
            { Fl_Value_Slider* o = undurchsicht_ = new Fl_Value_Slider(259, 6, 150, 25);
              o->type(1);
              o->value(undurchsicht);
              o->callback(undurchsicht_cb_statisch_);
            }
            { Fl_Light_Button* o = grau_ = new Fl_Light_Button(409, 6, 65, 25, _("Gray"));
              o->selection_color(FL_DARK3);
              o->value(grau);
              o->callback(grau_cb_statisch_);
            }
            o->end();
          }
          end();
  } 
  
  DBG_PROG_ENDE
 }

 void  waehlbar(bool wert)
 {
  if(wert) {
    gruppe_->activate();
    aktiv_knopf_->activate();
  } else {
    gruppe_->deactivate();
    aktiv_knopf_->deactivate();
  }
  redraw();
 }

 bool  waehlbar(void)
 {
  return (gruppe_->active() && aktiv_knopf_->active());
 }

 void  aktivieren(bool wert)
 {
  DBG_PROG_START

  aktiv_knopf_->value(wert);

  //icc_examin->icc_betrachter->DD_farbraum->dreiecks_netze.frei(false);
  icc_examin->icc_betrachter->DD_farbraum->dreiecks_netze[pos_].aktiv = wert;
  //icc_examin->icc_betrachter->DD_farbraum->dreiecks_netze.frei(true);

  redraw();

  DBG_PROG_ENDE
 }
 bool aktiv()
 {
  return aktiv_knopf_->value();
 }

 void  undurchsicht(double wert)
 {
  undurchsicht_->value(wert);
  //icc_examin->icc_betrachter->DD_farbraum->dreiecks_netze.frei(false);
  icc_examin->icc_betrachter->DD_farbraum->dreiecks_netze[pos_].undurchsicht = wert;
  //icc_examin->icc_betrachter->DD_farbraum->dreiecks_netze.frei(true);
 }
 double undurchsicht()
 {
  return undurchsicht_->value();
 }

 void  grau(bool wert)
 {
  grau_->value(wert);
  //icc_examin->icc_betrachter->DD_farbraum->dreiecks_netze.frei(false);
  icc_examin->icc_betrachter->DD_farbraum->dreiecks_netze[pos_].grau = wert;
  //icc_examin->icc_betrachter->DD_farbraum->dreiecks_netze.frei(true);
 }
 bool  grau()
 {
  return grau_->value();
 }
};

// ----------------------------------------------------------------------------

class ICCwaehler : public icc_examin_ns::MyFl_Double_Window
{
    Fl_Scroll *scroll_profile;
    Fl_Pack/*Fl_Scroll2Pack*/   *hbox;
public:
    ICCwaehler  (int w_,int h_,const char* name)
  : icc_examin_ns::MyFl_Double_Window(w_, h_, name)
 {
  DBG_PROG_START
  for(int i = 0; i < 128; ++i)
    profile_[i] = 0;

  {
    Fl_Scroll* o = scroll_profile = new Fl_Scroll(5, 5, w()-14, h()-10);
      o->box(FL_THIN_DOWN_BOX);
      o->type(Fl_Scroll::VERTICAL);
      { Fl_Pack* o = hbox = new Fl_Pack(6, 6, w()-16, h()-12);
        o->box(FL_THIN_DOWN_FRAME);
        o->end();
      }   
    o->end();
    scroll_profile->resizable( hbox );
  }       
  icc_examin_ns::MyFl_Double_Window::end();
  icc_examin_ns::MyFl_Double_Window::resizable(scroll_profile);
  icc_examin_ns::MyFl_Double_Window::is_toolbox = 1;

  //icc_examin_ns::MyFl_Double_Window::show();
  //icc_examin_ns::MyFl_Double_Window::iconize();

  DBG_PROG_ENDE
 }
                 ~ICCwaehler () {DBG_PROG_S( "::~ICCwaehler()" ); }
 void         clear ()
 {
  DBG_PROG_START
  hbox->clear();
  for(int i = 0; i < 128; ++i)
    profile_[i] = 0;
  DBG_PROG_V( children() )
  DBG_PROG_ENDE
 }

private:
    int                      aktuelles_profil_;
    ICCwaehlerProfil* profile_[128];

public:
 void         push_back (const char* name, double undurchsicht_, 
                         bool grau_, bool aktiv_, int waehlbar_)
 {
  DBG_PROG_START
  int pos = size();

  if(!name || !strlen(name))
  {
    name = new char [12];
    sprintf((char*)name, "%d.", pos);
  }

  hbox->begin();
    profile_[pos] = new ICCwaehlerProfil( name, 
                                        undurchsicht_, grau_, aktiv_, pos );
  hbox->end();
  waehlbar( pos, waehlbar_ );
  undurchsicht( pos, undurchsicht_ );
  grau( pos, grau_ );
  aktiv( pos, aktiv_ );
  redraw();
  //scroll_profile->firstsizes_y( scroll_profile->y()+2, hbox->h() );
  DBG_PROG_ENDE
 }

 void draw () { DBG_PROG_S(hbox->x()<<","<<hbox->y()<<" "<<hbox->w()<<"x"<<hbox->h())
                icc_examin_ns::MyFl_Double_Window::draw(); };

 void         expose ( int pos )
 {
  if(0 <= pos && pos < size())
  {
    undurchsicht( pos, profile_[pos]->undurchsicht() );
    grau( pos, profile_[pos]->grau() );
    waehlbar( pos, profile_[pos]->waehlbar() );
    aktiv( pos, profile_[pos]->aktiv() );
    icc_examin->icc_betrachter->DD_farbraum->invalidate();
  }
 }

 void         aktiv  ( int pos, int wert )
 {
  if(0 <= pos && pos < size())
    profile_[pos]->aktivieren( wert?true:false );
 }

 int          size      ()
 {
  DBG_PROG_START
  int size_ = 0;
  for(int i = 0; i < 128; ++i)
    if(profile_[i]) ++size_;
    else break;
  DBG_PROG_V( size_ )
  DBG_PROG_ENDE
  return size_;
 }

 void         waehlbar  ( int pos, int wert )
 { DBG_PROG_START
   if(0 <= pos && pos < size())
	   profile_[pos]->waehlbar( wert?true:false );
   DBG_PROG_ENDE
 }

 void         undurchsicht ( int pos, double wert )
 { DBG_PROG_START
   if(0 <= pos && pos < size())
	   profile_[pos]->undurchsicht( wert );
   DBG_PROG_ENDE
 }

 void         grau ( int pos, int wert )
 { DBG_PROG_START
   if(0 <= pos && pos < size())
	   profile_[pos]->grau( wert?true:false );
   DBG_PROG_ENDE
 }
};

#endif //ICC_WAEHLER_H

