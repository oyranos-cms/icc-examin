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

// Date:      März 2005

#ifndef ICC_WAEHLER_H
#define ICC_WAEHLER_H

#include <string>
#include <vector>
#include "icc_utils.h"
#include "icc_modell_beobachter.h"
#include "icc_kette.h"
#include "icc_examin.h"

#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Scroll.H>
#include <FL/Fl_Pack.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Output.H>
#include <FL/Fl_Value_Slider.H>
#include <FL/Fl_Light_Button.H>

class ICCwaehlerProfil : public Fl_Pack
{
  Fl_Button *aktiv_knopf_;
  void aktiv_knopf_cb_(int aktiv) {aktiv_knopf_->value(aktiv); }
  static void aktiv_knopf_cb_statisch_(Fl_Widget* w, void* data) {
                ICCwaehlerProfil* obj = dynamic_cast<ICCwaehlerProfil*>(w);
                if(obj) 
                  obj->aktiv_knopf_cb_( (int)data );
              }
  Fl_Pack   *gruppe_;
  void gruppe_cb_(int aktiv) {if(aktiv) gruppe_->activate(); else gruppe_->deactivate(); }
  static void gruppe_cb_statisch_(Fl_Widget* w, void* data) {
                ICCwaehlerProfil* obj = dynamic_cast<ICCwaehlerProfil*>(w);
                if(obj) {
                    obj->gruppe_cb_( (int)data );
                }
              }
  Fl_Output *name_;
  void name_cb_(char* name) {name_->value(name); }
  static void name_cb_statisch_(Fl_Widget* w, void* data) {
                ICCwaehlerProfil* obj = dynamic_cast<ICCwaehlerProfil*>(w);
                if(obj) {
                  if((int)data)
                    obj->name_cb_((char*) data);
                }
              }
  Fl_Value_Slider *transparenz_;
  void transparenz_cb_(double wert) {transparenz_->value(wert); }
  static void transparenz_cb_statisch_(Fl_Widget* w, void* data) {
                ICCwaehlerProfil* obj = dynamic_cast<ICCwaehlerProfil*>(w);
                if(obj && (int)data)
                  obj->transparenz_cb_( *(double*)data );
              }
  Fl_Light_Button *grau_;
  void grau_cb_(int aktiv) {grau_->value(aktiv); }
  static void grau_cb_statisch_(Fl_Widget* w, void* data) {
                ICCwaehlerProfil* obj = dynamic_cast<ICCwaehlerProfil*>(w);
                if(obj) 
                  obj->grau_cb_( (int)data );
              }
  public:
  ICCwaehlerProfil(const char* name, double transparenz, 
                            bool grau, bool aktiv)//int x_, int y_, int w_, int h_)
    : Fl_Pack( 0,0,470,25 )//x_,y_,w_,h_)
{
  DBG_PROG_START
  {
          type(1);
          { Fl_Button* o = aktiv_knopf_ = new Fl_Button(6, 6, 25, 25);
            o->type(1);
            o->selection_color((Fl_Color)103);
            o->value( aktiv );
            o->callback(aktiv_knopf_cb_statisch_);
          }
          { Fl_Pack* o = gruppe_ = new Fl_Pack(29, 6, 445, 25);
            o->type(1);
            o->callback(gruppe_cb_statisch_);
            if(aktiv) o->activate(); else o->deactivate();
            { Fl_Output* o = name_ = new Fl_Output(29, 6, 230, 25);
              o->align(FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
              o->value( name );
              this->resizable(o);
              o->callback(name_cb_statisch_);
            }
            { Fl_Value_Slider* o = transparenz_ = new Fl_Value_Slider(259, 6, 150, 25);
              o->type(1);
              o->value(transparenz);
              o->callback(transparenz_cb_statisch_);
            }
            { Fl_Light_Button* o = grau_ = new Fl_Light_Button(409, 6, 65, 25, "Grau");
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
    void  aktiv(bool wert) { aktiv_knopf_->value(wert);
             if(wert) gruppe_->activate(); else gruppe_->deactivate(); }
};

class ICCwaehler : public icc_examin_ns::Modell, icc_examin_ns::Beobachter,
                          Fl_Double_Window
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

    { Fl_Scroll* o = scroll_profile = new Fl_Scroll(5, 5, 471, 165);
      o->box(FL_THIN_DOWN_BOX);
      { Fl_Pack* o = hbox = new Fl_Pack(6, 6, 469, 161);
        o->end();
      }   
      o->end();
    }       
    this->end();
    this->resizable(scroll_profile);

  this->show();

  modellDazu( &profile ); // wird in nachricht ausgewertet
  DBG_PROG_ENDE
}
                 ~ICCwaehler () {; }
    void         clear ()       {; }
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
  DBG_PROG_ENDE
}
    void         aktiv  (int pos) {if(pos < size()) profile_[pos]->aktiv(true);
                                   else profile_[pos]->aktiv(false); }

    int          size      ()
{
  DBG_PROG_START
  int size = 0;
  for(int i = 0; i < 128; ++i)
    if(profile_[i]) ++size;
  return size;
  DBG_PROG_ENDE
}
      // virtual aus icc_examin_ns::Beobachter::
    void nachricht( icc_examin_ns::Modell* modell , int infos )
{
  clear();
  ICCkette* p = dynamic_cast<ICCkette*>(modell);

  if(p) {
    int anzahl = p->size();
    double transparenz;
    bool grau;
    std::vector<int> aktiv = profile.aktiv();
    for(int i = 0; i < anzahl; ++i) {
      const char* name = profile.name(i).c_str();
      transparenz = icc_examin->icc_betrachter->DD_histogram->dreiecks_netze[i].transparenz;
      grau = icc_examin->icc_betrachter->DD_histogram->dreiecks_netze[i].grau;
      push_back(name, transparenz, grau , aktiv[i]);
    }
  }
}

};

#endif //ICC_WAEHLER_H

