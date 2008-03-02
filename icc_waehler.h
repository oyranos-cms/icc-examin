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
 * Die Wähler von Profilen Klasse
 * 
 */

// Date:      Maerz 2005

#ifndef ICC_WAEHLER_H
#define ICC_WAEHLER_H

#include <string>
#include <vector>
#include "icc_utils.h"

#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Scroll.H>
#include <FL/Fl_Pack.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Output.H>
#include <FL/Fl_Value_Slider.H>
#include <FL/Fl_Light_Button.H>

// Einstellungen direkt in icc_examin->icc_betrachter->DD_farbraum

class ICCwaehlerProfil : public Fl_Pack
{
  Fl_Button *aktiv_knopf_;
  void aktiv_knopf_cb_() {
                if(aktiv_knopf_->value()) {DBG_PROG_START
                  aktivieren(true);
                  icc_examin->icc_betrachter->DD_farbraum->dreiecks_netze[parent()->find(this)].aktiv = true;
                } else {
                  aktivieren(false);
                  icc_examin->icc_betrachter->DD_farbraum->dreiecks_netze[parent()->find(this)].aktiv = false;
                }
                redraw();
                icc_examin->icc_betrachter->DD_farbraum->draw();
                icc_examin->icc_betrachter->DD_farbraum->flush();
                DBG_PROG_ENDE
              }
  static void aktiv_knopf_cb_statisch_(Fl_Widget* w, void* data) {DBG_PROG_START
                ICCwaehlerProfil* obj = dynamic_cast<ICCwaehlerProfil*>(w->parent());
                if(obj) 
                  obj->aktiv_knopf_cb_();
                else WARN_S( _("kein ICCwaehlerProfil??") )
                if(!w) WARN_S( _("kein Fl_Widget??") )
                DBG_PROG_ENDE
              }
  Fl_Pack   *gruppe_;
  Fl_Output *name_;
  Fl_Value_Slider *transparenz_;
  void transparenz_cb_() {
                icc_examin->icc_betrachter->DD_farbraum->dreiecks_netze[parent()->find(this)].transparenz = transparenz_->value();
                icc_examin->icc_betrachter->DD_farbraum->draw();
                icc_examin->icc_betrachter->DD_farbraum->flush();
              }
  static void transparenz_cb_statisch_(Fl_Widget* w, void* data) {
                ICCwaehlerProfil* obj = dynamic_cast<ICCwaehlerProfil*>(w->parent()->parent());
                if(obj)
                  obj->transparenz_cb_();
              }
  Fl_Light_Button *grau_;
  void grau_cb_() {
                icc_examin->icc_betrachter->DD_farbraum->dreiecks_netze[parent()->find(this)].grau = grau_->value();
                icc_examin->icc_betrachter->DD_farbraum->draw();
                icc_examin->icc_betrachter->DD_farbraum->flush();
              }
  static void grau_cb_statisch_(Fl_Widget* w, void* data) {
                ICCwaehlerProfil* obj = dynamic_cast<ICCwaehlerProfil*>(w->parent()->parent());
                if(obj) 
                  obj->grau_cb_();
              }
  public:
  ICCwaehlerProfil(const char* name, double transparenz, 
                   bool grau, bool aktiv)
    : Fl_Pack( 0,0,470,25 )
{
  DBG_PROG_START
  DBG_PROG_V( name )
  DBG_PROG_V( transparenz )
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
            { Fl_Value_Slider* o = transparenz_ = new Fl_Value_Slider(259, 6, 150, 25);
              o->type(1);
              o->value(transparenz);
              o->callback(transparenz_cb_statisch_);
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
    void  aktivieren(bool wert)
{
  aktiv_knopf_->value(wert);
  if(wert) { gruppe_->activate();
  } else {   gruppe_->deactivate();
  }
}
};

class ICCwaehler : public Fl_Double_Window
{
    Fl_Scroll *scroll_profile;
    Fl_Pack   *hbox;
  public:
    ICCwaehler  (int x_,int y_,const char* name)
  : Fl_Double_Window(x_, y_, name)
{
  DBG_PROG_START
  for(int i = 0; i < 128; ++i)
    profile_[i] = 0;

  { Fl_Scroll* o = scroll_profile = new Fl_Scroll(5, 5, w()-14, h()-21);
      o->box(FL_THIN_DOWN_BOX);
      { Fl_Pack* o = hbox = new Fl_Pack(6, 6, w()-16, h()-25);
        o->end();
      }   
      o->end();
  }       
  Fl_Double_Window::end();
  Fl_Double_Window::resizable(scroll_profile);

  //Fl_Double_Window::show();
  Fl_Double_Window::iconize();

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
    void         push_back (const char* name, double transparenz, 
                            bool grau, bool aktiv)
{
  DBG_PROG_START
  int pos = size();
  hbox->begin();
  profile_[pos] = new ICCwaehlerProfil( name, transparenz, grau, aktiv );
  hbox->end();
  redraw();
  DBG_PROG_ENDE
}
    void         aktiv  (int pos) {if(pos < size())
                                        profile_[pos]->aktivieren(true);
                                   else profile_[pos]->aktivieren(false); }

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

};

#endif //ICC_WAEHLER_H

