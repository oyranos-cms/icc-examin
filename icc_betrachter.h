// generated by Fast Light User Interface Designer (fluid) version 1.0104

#ifndef icc_betrachter_h
#define icc_betrachter_h
#include <FL/Fl.H>
#define DEBUG
#include <cassert>
#include <iostream>
//#include <sstream>
#include <stdio.h>
#include <string>
#include <vector>
#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Menu_Bar.H>
#include <FL/Fl_Box.H>
#ifdef HAVE_FLU
#include <FLU/Flu_File_Chooser.h> 
#endif
#include <FL/Fl_File_Chooser.H> 
#include <FL/Fl_Hold_Browser.H> 
#include <FL/Fl_File_Icon.H> 
#include <FL/Fl_Shared_Image.H> 
#include <FL/Fl_PNM_Image.H>
class TagDrawings;
class TagBrowser;
class TagTexts;
class MftChoice;
class GL_Ansicht;
#include "icc_examin.h"
#include "icc_profile.h"
#include "icc_utils.h"
#include "icc_ueber.h"
char* icc_read_info(char* filename);

class TagBrowser : public Fl_Hold_Browser {
  int X; int Y; int W; int H; char* start_info; std::string selectedTagName;
public:
  TagBrowser(int X,int Y,int W,int H,char* start_info) ;
  void reopen();
  void selectItem(int item);
};

class TagTexts : public Fl_Hold_Browser {
  int X; int Y; int W; int H; char* start_info;
public:
  int inspekt_topline;
  TagTexts(int X,int Y,int W,int H,char* start_info) ;
  void hinein(std::string text);
};

class TagDrawings : public Fl_Widget {
  int X; int Y; int W; int H;
public:
  int wiederholen;
  TagDrawings(int X,int Y,int W,int H) ;
  void draw();
  void hineinPunkt(std::vector<double> vect, std::vector<std::string> txt);
  void hineinKurven(std::vector<std::vector<double> >vect, std::vector<std::string> txt);
  void ruhigNeuzeichnen(void);
};

class MftChoice : public Fl_Choice {
  int X; int Y; int W; int H; char* start_info; char typ[5];
public:
  std::vector<std::string> Info; int gewaehlter_eintrag;
  MftChoice(int X,int Y,int W,int H,char* start_info) ;
  void profilTag(int _tag, std::string text);
  void auswahlCb(void);
};
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Help_View.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Menu_Bar.H>
#include <FL/Fl_Tile.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Progress.H>

class ICCfltkBetrachter {
public:
  bool setTitleUrl;

 int px,py,pw,ph;
 int fullscreen;
  
 int tag_nummer;
  Fl_Double_Window* init();
  Fl_Double_Window *ueber;
  Fl_Help_View *ueber_html;
  Fl_Button *ja;
private:
  inline void cb_ja_i(Fl_Button*, void*);
  static void cb_ja(Fl_Button*, void*);
public:
  Fl_Double_Window *details;
  static Fl_Menu_Item menu_[];
private:
  inline void cb_ffnen_i(Fl_Menu_*, void*);
  static void cb_ffnen(Fl_Menu_*, void*);
public:
  static Fl_Menu_Item *menueintrag_html_speichern;
private:
  inline void cb_menueintrag_html_speichern_i(Fl_Menu_*, void*);
  static void cb_menueintrag_html_speichern(Fl_Menu_*, void*);
  inline void cb_Beenden_i(Fl_Menu_*, void*);
  static void cb_Beenden(Fl_Menu_*, void*);
  inline void cb_Voreinstellungen_i(Fl_Menu_*, void*);
  static void cb_Voreinstellungen(Fl_Menu_*, void*);
public:
  static Fl_Menu_Item *menueintrag_Voll;
private:
  inline void cb_menueintrag_Voll_i(Fl_Menu_*, void*);
  static void cb_menueintrag_Voll(Fl_Menu_*, void*);
public:
  static Fl_Menu_Item *menueintrag_inspekt;
private:
  inline void cb_menueintrag_inspekt_i(Fl_Menu_*, void*);
  static void cb_menueintrag_inspekt(Fl_Menu_*, void*);
public:
  static Fl_Menu_Item *menueintrag_3D;
private:
  inline void cb_menueintrag_3D_i(Fl_Menu_*, void*);
  static void cb_menueintrag_3D(Fl_Menu_*, void*);
public:
  static Fl_Menu_Item *menu_hilfe;
private:
  inline void cb_ber_i(Fl_Menu_*, void*);
  static void cb_ber(Fl_Menu_*, void*);
public:
  Fl_Tile *examin;
  TagBrowser *tag_browser;
private:
  inline void cb_tag_browser_i(TagBrowser*, void*);
  static void cb_tag_browser(TagBrowser*, void*);
public:
  Fl_Group *ansichtsgruppe;
  Fl_Group *tabellengruppe;
  MftChoice *mft_choice;
private:
  inline void cb_mft_choice_i(MftChoice*, void*);
  static void cb_mft_choice(MftChoice*, void*);
public:
  TagDrawings *mft_viewer;
  TagTexts *mft_text;
  GL_Ansicht *mft_gl;
  TagDrawings *tag_viewer;
  TagTexts *tag_text;
  Fl_Help_View *inspekt_html;
  GL_Ansicht *DD_histogram;
  Fl_Box *box_stat;
  Fl_Progress *load_progress;
  void run();
  std::vector<std::string> open(std::vector<std::string> dateinamen);
  void quit(void);
  void zeig_mich_(void* widget);
  void measurement(bool has_measurement);
};
std::vector<std::string> zeilenNachVector(std::string text);
void dHaendler(void* o);
#endif
