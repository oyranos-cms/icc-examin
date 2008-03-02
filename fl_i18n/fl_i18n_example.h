// generated by Fast Light User Interface Designer (fluid) version 1.0107

#ifndef fl_i18n_example_h
#define fl_i18n_example_h
#include <FL/Fl.H>
#include <FL/fl_draw.H>
#include <FL/Fl_Box.H>

class MyBox : public Fl_Box {
public:
  void draw();
  MyBox( int x, int y, int w, int h, const char *label);
  MyBox( int x, int y, int w, int h );
};
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Button.H>
extern MyBox *b;
#endif
