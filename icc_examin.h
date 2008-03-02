// generated by Fast Light User Interface Designer (fluid) version 1.0104

#ifndef icc_examin_h
#define icc_examin_h
#include <FL/Fl.H>
#define DEBUG
#include <stdio.h>
#include <iostream>
//#include <sstream>
#include <cassert>
#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Menu_Bar.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_File_Chooser.H> 
#include <FLU/Flu_File_Chooser.h> 
#include <FL/Fl_Hold_Browser.H> 
#include <FL/Fl_File_Icon.H> 
#include <FL/Fl_Shared_Image.H> 
#include <FL/Fl_PNM_Image.H>
class TagDrawings;
class TagBrowser;
class TagTexts;
class MftChoice;
class GL_Ansicht;
#define _(text) text
#define status(_texte) {std::stringstream s; s << _texte; box_stat->label(s.str().c_str());}
#include "icc_profile.h"
#include "icc_utils.h"
#include "icc_profilierer.h"
#include "icc_ueber.h"
extern ICCprofile profile;
#include <FL/Fl_Double_Window.H>
extern Fl_Double_Window *ueber;
#include <FL/Fl_Group.H>
#include <FL/Fl_Help_View.H>
extern Fl_Help_View *ueber_html;
#include <FL/Fl_Button.H>
extern Fl_Button *ja;
extern Fl_Double_Window *details;
#include <FL/Fl_Menu_Bar.H>
extern Fl_Group *inspekt;
extern Fl_Help_View *inspekt_html;
#include <FL/Fl_Tile.H>
extern Fl_Tile *examin;
extern TagBrowser *tag_browser;
extern Fl_Group *ansichtsgruppe;
extern Fl_Group *tabellengruppe;
extern MftChoice *mft_choice;
extern TagDrawings *mft_viewer;
extern TagTexts *mft_text;
extern GL_Ansicht *mft_gl;
extern Fl_Group *tag_3D;
extern TagDrawings *tag_viewer;
extern TagTexts *tag_text;
#include <FL/Fl_Box.H>
extern Fl_Box *box_stat;
#include <FL/Fl_Progress.H>
extern Fl_Progress *load_progress;
extern Fl_Menu_Item menu_[];
#define menueintrag_html_speichern (menu_+2)
#define menueintrag_Voll (menu_+6)
#define menueintrag_inspekt (menu_+9)
#define menu_hilfe (menu_+10)
std::string open(int interaktiv);
void quit(void);
char* icc_read_info(char* filename);

class TagBrowser : public Fl_Hold_Browser {
  int X; int Y; int W; int H; char* start_info; std::string selectedTagName;
public:
  TagBrowser(int X,int Y,int W,int H,char* start_info) ;
  void reopen();
  void select_item(int item);
};

class TagTexts : public Fl_Hold_Browser {
  int X; int Y; int W; int H; char* start_info;
public:
  TagTexts(int X,int Y,int W,int H,char* start_info) ;
  void hinein(std::string text);
};

class TagDrawings : public Fl_Widget {
  int X; int Y; int W; int H; std::vector<std::string>texte; std::vector<double>punkte; std::vector<std::vector<double> >kurven;
public:
  int wiederholen;
  TagDrawings(int X,int Y,int W,int H) ;
  void draw();
  void hinein_punkt(std::vector<double> vect, std::vector<std::string> txt);
  void hinein_kurven(std::vector<std::vector<double> >vect, std::vector<std::string> txt);
  void ruhig_neuzeichnen(void);
};

class MftChoice : public Fl_Choice {
  int X; int Y; int W; int H; char* start_info; char typ[5]; std::vector<std::string> Info; int gewaehlter_eintrag;
public:
  MftChoice(int X,int Y,int W,int H,char* start_info) ;
  void profil_tag(int _tag);
  void auswahl_cb(void);
};
void d_haendler(void* o);
void zeig_mich(void* widget);
std::vector<std::string> zeilenNachVector(std::string text);
#endif
