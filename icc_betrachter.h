// generated by Fast Light User Interface Designer (fluid) version 1.0106

#ifndef icc_betrachter_h
#define icc_betrachter_h
#include <FL/Fl.H>
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
#include "icc_fenster.h" 
#include <FL/Fl_Hold_Browser.H>
class TagDrawings;
class TagBrowser;
class TagTexts;
class MftChoice;
class GL_Ansicht;
#include "icc_examin.h"
#include "icc_profile.h"
#include "icc_utils.h"
#include "icc_ueber.h"
#include "icc_helfer.h"
#include "icc_helfer_fltk.h"
char* iccReadInfo(char* filename);

class TagBrowser : public Fl_Hold_Browser {
public:
  int X; int Y; int W; int H; char* start_info; std::string selectedTagName;
  TagBrowser(int X,int Y,int W,int H,char* start_info) ;
  void reopen_nicht();
  void selectItem(int item);
  int visible();
};

class TagTexts : public Fl_Hold_Browser {
  int X; int Y; int W; int H; char* start_info;
public:
  int inspekt_topline;
  TagTexts(int X,int Y,int W,int H,char* start_info) ;
  void hinein(std::string text);
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
  
 enum{ WID_0, WID_3D, WID_INSPEKT }; int widget_oben;
  Fl_Double_Window* init(int argc, char** argv);
  Fl_Double_Window *ueber;
  Fl_Help_View *ueber_html;
  Fl_Button *ja;
private:
  inline void cb_ja_i(Fl_Button*, void*);
  static void cb_ja(Fl_Button*, void*);
public:
  Fl_Double_Window *vcgt;
  TagDrawings *vcgt_viewer;
private:
  inline void cb_Close_i(Fl_Button*, void*);
  static void cb_Close(Fl_Button*, void*);
  inline void cb_Refresh_i(Fl_Button*, void*);
  static void cb_Refresh(Fl_Button*, void*);
  inline void cb_Set_i(Fl_Button*, void*);
  static void cb_Set(Fl_Button*, void*);
  inline void cb_Reset_i(Fl_Button*, void*);
  static void cb_Reset(Fl_Button*, void*);
  inline void cb_Load_i(Fl_Button*, void*);
  static void cb_Load(Fl_Button*, void*);
public:
  Fl_Double_Window *details;
  Fl_Menu_Bar *menueleiste;
  static Fl_Menu_Item menu_menueleiste[];
private:
  inline void cb_Open_i(Fl_Menu_*, void*);
  static void cb_Open(Fl_Menu_*, void*);
public:
  static Fl_Menu_Item *menueintrag_html_speichern;
private:
  inline void cb_menueintrag_html_speichern_i(Fl_Menu_*, void*);
  static void cb_menueintrag_html_speichern(Fl_Menu_*, void*);
public:
  static Fl_Menu_Item *menueintrag_gamut_speichern;
private:
  inline void cb_menueintrag_gamut_speichern_i(Fl_Menu_*, void*);
  static void cb_menueintrag_gamut_speichern(Fl_Menu_*, void*);
  inline void cb_Quit_i(Fl_Menu_*, void*);
  static void cb_Quit(Fl_Menu_*, void*);
  inline void cb_Oyranos_i(Fl_Menu_*, void*);
  static void cb_Oyranos(Fl_Menu_*, void*);
public:
  static Fl_Menu_Item *menueintrag_huelle;
private:
  inline void cb_menueintrag_huelle_i(Fl_Menu_*, void*);
  static void cb_menueintrag_huelle(Fl_Menu_*, void*);
public:
  static Fl_Menu_Item *menueintrag_Voll;
private:
  inline void cb_menueintrag_Voll_i(Fl_Menu_*, void*);
  static void cb_menueintrag_Voll(Fl_Menu_*, void*);
public:
  static Fl_Menu_Item *menueintrag_gamutwarn;
private:
  inline void cb_menueintrag_gamutwarn_i(Fl_Menu_*, void*);
  static void cb_menueintrag_gamutwarn(Fl_Menu_*, void*);
public:
  static Fl_Menu_Item *menueintrag_3D;
private:
  inline void cb_menueintrag_3D_i(Fl_Menu_*, void*);
  static void cb_menueintrag_3D(Fl_Menu_*, void*);
public:
  static Fl_Menu_Item *menueintrag_inspekt;
private:
  inline void cb_menueintrag_inspekt_i(Fl_Menu_*, void*);
  static void cb_menueintrag_inspekt(Fl_Menu_*, void*);
public:
  static Fl_Menu_Item *menueintrag_zeigcgats;
private:
  inline void cb_menueintrag_zeigcgats_i(Fl_Menu_*, void*);
  static void cb_menueintrag_zeigcgats(Fl_Menu_*, void*);
public:
  static Fl_Menu_Item *menueintrag_vcgt;
private:
  inline void cb_menueintrag_vcgt_i(Fl_Menu_*, void*);
  static void cb_menueintrag_vcgt(Fl_Menu_*, void*);
public:
  static Fl_Menu_Item *menueintrag_testkurven;
private:
  inline void cb_menueintrag_testkurven_i(Fl_Menu_*, void*);
  static void cb_menueintrag_testkurven(Fl_Menu_*, void*);
public:
  static Fl_Menu_Item *untermenue_3D;
  static Fl_Menu_Item *menu_hilfe;
private:
  inline void cb_About_i(Fl_Menu_*, void*);
  static void cb_About(Fl_Menu_*, void*);
public:
  GL_Ansicht *DD_farbraum;
  Fl_Help_View *inspekt_html;
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
  Fl_Box *box_stat;
  Fl_Progress *load_progress;
  void run();
  std::vector<std::string> open(std::vector<std::string> dateinamen);
  void quit(void);
  void zeig_mich_(void* widget);
  void measurement(bool has_measurement);
};
void dHaendler(void* o);
#endif
