// generated by Fast Light User Interface Designer (fluid) version 1.0106

#include "fl_oyranos.h"
#include <string>
#include "icc_oyranos.h"

Fl_Group *order_gruppe=(Fl_Group *)0;

static void cb_L(Fl_Button*, void*) {
  oyranos_pfade_loeschen();
}

static void cb_Auffrischen(Fl_Button*, void*) {
  oyranos_pfade_einlesen();
}

static void cb_Neu(Fl_Button*, void*) {
  oyranos_pfade_waehlen();
}

TagTexts *fl_oyanos_pfade_browser=(TagTexts *)0;

Fl_Double_Window* voreinstellungen_intern() {
  Fl_Double_Window* w;
  { Fl_Double_Window* o = new Fl_Double_Window(375, 205, "Einstellungen");
    w = o;
    { Fl_Tabs* o = new Fl_Tabs(0, 0, 375, 205, "Einstellungen");
      o->align(FL_ALIGN_TOP|FL_ALIGN_INSIDE);
      { Fl_Group* o = new Fl_Group(0, 25, 375, 180, "Farbprofile");
        { Fl_Group* o = order_gruppe = new Fl_Group(5, 30, 365, 115);
          o->box(FL_ENGRAVED_FRAME);
          { Fl_Button* o = new Fl_Button(80, 105, 75, 30, "L\366schen");
            o->callback((Fl_Callback*)cb_L);
          }
          { Fl_Button* o = new Fl_Button(160, 105, 85, 30, "Auffrischen");
            o->callback((Fl_Callback*)cb_Auffrischen);
          }
          { Fl_Button* o = new Fl_Button(280, 105, 85, 30, "Neu");
            o->callback((Fl_Callback*)cb_Neu);
          }
          { TagTexts* o = fl_oyanos_pfade_browser = new TagTexts(80, 35, 285, 70, "Pfade:");
            o->box(FL_NO_BOX);
            o->color(FL_BACKGROUND2_COLOR);
            o->selection_color(FL_SELECTION_COLOR);
            o->labeltype(FL_NORMAL_LABEL);
            o->labelfont(0);
            o->labelsize(14);
            o->labelcolor(FL_BLACK);
            o->align(FL_ALIGN_LEFT);
            o->when(FL_WHEN_RELEASE_ALWAYS);
          }
          o->end();
        }
        o->end();
        Fl_Group::current()->resizable(o);
      }
      o->end();
      Fl_Group::current()->resizable(o);
    }
    o->show();
    o->end();
  }
  return w;
}

void oyranos_pfade_waehlen() {
  std::string text;
  text.append ("Hallo\nHallo",11);

    fl_oyanos_pfade_browser->add( text.c_str(), 0);


      fl_oyanos_pfade_browser->topline(0);
      fl_oyanos_pfade_browser->textfont(FL_COURIER);
      fl_oyanos_pfade_browser->textsize(14);
}
