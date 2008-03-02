// generated by Fast Light User Interface Designer (fluid) version 1.0106

#include <libintl.h>
#include "icc_betrachter.h"
#include "icc_draw.h"
#include "icc_kette.h"
#include "icc_oyranos_extern.h"
#include "icc_gl.h"
using namespace icc_examin_ns;

char* iccReadInfo(char* filename) {
  DBG_PROG_START
  char systemBefehl[1024];
  char *textfile = "/tmp/icc_temp.txt";


  sprintf (systemBefehl, "iccdump \"%s\" > %s",
                                    filename, textfile);
  system (systemBefehl);

  return textfile;
  DBG_PROG_ENDE
}

TagBrowser::TagBrowser(int X,int Y,int W,int H,const char* start_info) : Fl_Hold_Browser(X,Y,W,H,start_info), X(X), Y(Y), W(W), H(H) {
}

void TagBrowser::reopen_nicht() {
  DBG_PROG_START
  DBG_PROG_ENDE
}

void TagBrowser::selectItem(int item) {
  DBG_PROG_START
  //Auswahl aus tag_browser

  status("")

  item -= 6;
  DBG_PROG_S( item << ". Tag " )
  
  std::string text = icc_examin->waehleTag(item);
  if (text != "")
    selectedTagName = text;
  DBG_PROG_V( text );
  DBG_PROG_ENDE
}

int TagBrowser::visible() {
  return Fl_Widget::visible();
}

TagTexts::TagTexts(int X,int Y,int W,int H,const char* start_info) : Fl_Hold_Browser(X,Y,W,H,start_info), X(X), Y(Y), W(W), H(H) {
}

void TagTexts::hinein(std::string text) {
  DBG_PROG_START
  //Text aus tag_browser anzeigen

      this->clear();

      std::vector <std::string> texte = icc_parser::zeilenNachVector( text );
      for (unsigned int i = 0; i < texte.size(); i++)
        this->add( texte[i].c_str(), 0);


      this->topline(0);
      this->textfont(FL_COURIER);
      this->textsize(14);
  DBG_PROG_ENDE
}
#include <FL/fl_draw.H>

MftChoice::MftChoice(int X,int Y,int W,int H,const char* start_info) : Fl_Choice(X,Y,W,H,start_info), X(X), Y(Y), W(W), H(H) {
  gewaehlter_eintrag = 0;
}

void MftChoice::profilTag(int _tag, std::string text) {
  DBG_PROG_START
  icc_examin->icc_betrachter->tag_nummer = _tag;

// = profile.profil()->printTagInfo(icc_examin->icc_betrachter->tag_nummer);
    sprintf (&typ[0], profile.profil()->printTagInfo(icc_examin->icc_betrachter->tag_nummer)[1].c_str());

    DBG_PROG_V( profile.profil()->printTagInfo(icc_examin->icc_betrachter->tag_nummer)[1].c_str() )

    Info = icc_parser::zeilenNachVector (text);

    if ( strstr (typ,"mft2") != 0 )
    { DBG_PROG
      Fl_Menu_Item *mft_menue = (Fl_Menu_Item *)calloc (sizeof (Fl_Menu_Item), 6);

      mft_menue[0].text = Info[0].c_str();
      mft_menue[1].text = Info[4].c_str();
      mft_menue[2].text = Info[5].c_str();
      mft_menue[3].text = Info[6].c_str();
      mft_menue[4].text = Info[7].c_str();
      mft_menue[5].text = 0;
      icc_examin->icc_betrachter->mft_choice->menu(mft_menue);
    } else { DBG_PROG
      Fl_Menu_Item *mft_menue = (Fl_Menu_Item *)calloc (sizeof (Fl_Menu_Item), 6);

      mft_menue[0].text = Info[0].c_str();
      mft_menue[1].text = Info[4].c_str();
      mft_menue[2].text = _("lineare entrance curve with 256 steps");
      mft_menue[3].text = Info[5].c_str();
      mft_menue[4].text = _("lineare exit curve with 256 steps");
      mft_menue[5].text = 0;
      icc_examin->icc_betrachter->mft_choice->menu(mft_menue);
    }

    icc_examin->icc_betrachter->mft_choice->value( gewaehlter_eintrag );

  //auswahlCb();
  DBG_PROG_ENDE
}

void MftChoice::auswahlCb(void) {
  DBG_PROG_START
  //Auswahl aus mft_choice

  status("")

  Fl_Menu_* mw = (Fl_Menu_*)this;
  const Fl_Menu_Item* m = mw->mvalue();
  if (!m) {
    DBG_PROG_S("NULL \n")
  } else if (m->shortcut()) {
    DBG_PROG_S("%s - %s \n" << m->label() << fl_shortcut_label(m->shortcut()))
  } else {
    DBG_PROG_S("%s \n" << m->label())
  }

  icc_examin->waehleMft( mw->value() );

  DBG_PROG
  DBG_PROG_ENDE
}

My_Fl_Box::My_Fl_Box(int X,int Y,int W, int H, const char* title ) : Fl_Box(X,Y,W,H,title) {
}

int My_Fl_Box::handle( int event ) {
  //zeigFltkEvents( event);

  int ergebnis = tastatur(event);
  if(!ergebnis)
    ;//Fl_No_Box::handle(event);
  return ergebnis;
}

inline void ICCfltkBetrachter::cb_ja_i(Fl_Button*, void*) {
  ueber->hide();
}
void ICCfltkBetrachter::cb_ja(Fl_Button* o, void* v) {
  ((ICCfltkBetrachter*)(o->parent()->parent()->user_data()))->cb_ja_i(o,v);
}

inline void ICCfltkBetrachter::cb_vcgt_set_button_i(Fl_Button*, void*) {
  icc_examin->moniSetzen();
}
void ICCfltkBetrachter::cb_vcgt_set_button(Fl_Button* o, void* v) {
  ((ICCfltkBetrachter*)(o->parent()->parent()->user_data()))->cb_vcgt_set_button_i(o,v);
}

inline void ICCfltkBetrachter::cb_vcgt_reset_button_i(Fl_Button*, void*) {
  icc_examin->standardGamma();
}
void ICCfltkBetrachter::cb_vcgt_reset_button(Fl_Button* o, void* v) {
  ((ICCfltkBetrachter*)(o->parent()->parent()->user_data()))->cb_vcgt_reset_button_i(o,v);
}

inline void ICCfltkBetrachter::cb_vcgt_load_button_i(Fl_Button*, void*) {
  icc_examin->moniHolen();
}
void ICCfltkBetrachter::cb_vcgt_load_button(Fl_Button* o, void* v) {
  ((ICCfltkBetrachter*)(o->parent()->parent()->user_data()))->cb_vcgt_load_button_i(o,v);
}

inline void ICCfltkBetrachter::cb_vcgt_refresh_button_i(Fl_Button*, void*) {
  icc_examin->vcgtZeigen();
}
void ICCfltkBetrachter::cb_vcgt_refresh_button(Fl_Button* o, void* v) {
  ((ICCfltkBetrachter*)(o->parent()->parent()->user_data()))->cb_vcgt_refresh_button_i(o,v);
}

inline void ICCfltkBetrachter::cb_vcgt_close_button_i(Fl_Button*, void*) {
  vcgt->hide();
}
void ICCfltkBetrachter::cb_vcgt_close_button(Fl_Button* o, void* v) {
  ((ICCfltkBetrachter*)(o->parent()->parent()->user_data()))->cb_vcgt_close_button_i(o,v);
}

inline void ICCfltkBetrachter::cb_Open_i(Fl_Menu_*, void*) {
  icc_examin->oeffnen();
}
void ICCfltkBetrachter::cb_Open(Fl_Menu_* o, void* v) {
  ((ICCfltkBetrachter*)(o->parent()->parent()->user_data()))->cb_Open_i(o,v);
}

inline void ICCfltkBetrachter::cb_menueintrag_html_speichern_i(Fl_Menu_*, void*) {
  DBG_PROG_START

  icc_examin->berichtSpeichern ();

  DBG_PROG_ENDE;
}
void ICCfltkBetrachter::cb_menueintrag_html_speichern(Fl_Menu_* o, void* v) {
  ((ICCfltkBetrachter*)(o->parent()->parent()->user_data()))->cb_menueintrag_html_speichern_i(o,v);
}

inline void ICCfltkBetrachter::cb_menueintrag_gamut_speichern_i(Fl_Menu_*, void*) {
  DBG_PROG_START

  icc_examin->gamutSpeichern (icc_examin_ns::ICC_ABSTRACT);

  DBG_PROG_ENDE;
}
void ICCfltkBetrachter::cb_menueintrag_gamut_speichern(Fl_Menu_* o, void* v) {
  ((ICCfltkBetrachter*)(o->parent()->parent()->user_data()))->cb_menueintrag_gamut_speichern_i(o,v);
}

inline void ICCfltkBetrachter::cb_menueintrag_gamut_vrml_speichern_i(Fl_Menu_*, void*) {
  DBG_PROG_START

  icc_examin->gamutSpeichern (icc_examin_ns::ICC_VRML);

  DBG_PROG_ENDE;
}
void ICCfltkBetrachter::cb_menueintrag_gamut_vrml_speichern(Fl_Menu_* o, void* v) {
  ((ICCfltkBetrachter*)(o->parent()->parent()->user_data()))->cb_menueintrag_gamut_vrml_speichern_i(o,v);
}

inline void ICCfltkBetrachter::cb_Quit_i(Fl_Menu_*, void*) {
  quit();
}
void ICCfltkBetrachter::cb_Quit(Fl_Menu_* o, void* v) {
  ((ICCfltkBetrachter*)(o->parent()->parent()->user_data()))->cb_Quit_i(o,v);
}

inline void ICCfltkBetrachter::cb_Oyranos_i(Fl_Menu_*, void*) {
  oyranos_einstellungen();
}
void ICCfltkBetrachter::cb_Oyranos(Fl_Menu_* o, void* v) {
  ((ICCfltkBetrachter*)(o->parent()->parent()->user_data()))->cb_Oyranos_i(o,v);
}

inline void ICCfltkBetrachter::cb_menueintrag_gamutwarn_i(Fl_Menu_* o, void*) {
  Fl_Menu_* mw = (Fl_Menu_*)o;
  const Fl_Menu_Item* m = mw->mvalue();

  DBG_PROG_S (m->value())

  icc_examin->gamutwarn(m->value());
}
void ICCfltkBetrachter::cb_menueintrag_gamutwarn(Fl_Menu_* o, void* v) {
  ((ICCfltkBetrachter*)(o->parent()->parent()->user_data()))->cb_menueintrag_gamutwarn_i(o,v);
}

inline void ICCfltkBetrachter::cb_menueintrag_huelle_i(Fl_Menu_* o, void*) {
  DBG_PROG_START
  Fl_Menu_* mw = (Fl_Menu_*)o;
  const Fl_Menu_Item* m = mw->mvalue();

  DBG_PROG_S (m->value())
  if (m->value()) {
    widget_oben = WID_3D;
    ((Fl_Menu_Item*)menueintrag_3D)->set();
  } else {
    widget_oben = WID_0;
    ((Fl_Menu_Item*)menueintrag_3D)->clear();
  }
  DBG_PROG_V( widget_oben )
  DBG_PROG_V( menueintrag_3D->value() )
  DBG_PROG_V( menueintrag_huelle->value() )
  icc_examin->icc_betrachterNeuzeichnen(icc_examin->icc_betrachter->DD_farbraum);
  DBG_PROG_ENDE;
}
void ICCfltkBetrachter::cb_menueintrag_huelle(Fl_Menu_* o, void* v) {
  ((ICCfltkBetrachter*)(o->parent()->parent()->user_data()))->cb_menueintrag_huelle_i(o,v);
}

inline void ICCfltkBetrachter::cb_menueintrag_Voll_i(Fl_Menu_*, void*) {
  Fl_Window *w = (Fl_Window *)details;

  if (!fullscreen) {
    px = w->x();
    py = w->y();
    pw = w->w();
    ph = w->h();

    w->fullscreen();
    fullscreen = true;
  } else {
    w->fullscreen_off(px,py,pw,ph);
    fullscreen = false;
  };
}
void ICCfltkBetrachter::cb_menueintrag_Voll(Fl_Menu_* o, void* v) {
  ((ICCfltkBetrachter*)(o->parent()->parent()->user_data()))->cb_menueintrag_Voll_i(o,v);
}

inline void ICCfltkBetrachter::cb_menueintrag_3D_i(Fl_Menu_* o, void*) {
  DBG_PROG_START
  Fl_Menu_* mw = (Fl_Menu_*)o;
  const Fl_Menu_Item* m = mw->mvalue();

  DBG_PROG_S (m->value())
  if (m->value()) {
    widget_oben = WID_3D;
    ((Fl_Menu_Item*)menueintrag_huelle)->set();
  } else {
    widget_oben = WID_0;
    ((Fl_Menu_Item*)menueintrag_huelle)->clear();
  }
  DBG_PROG_V( widget_oben )
  DBG_PROG_V( menueintrag_3D->value() )
  DBG_PROG_V( menueintrag_huelle->value() )
  icc_examin->icc_betrachterNeuzeichnen(icc_examin->icc_betrachter->DD_farbraum);
  DBG_PROG_ENDE;
}
void ICCfltkBetrachter::cb_menueintrag_3D(Fl_Menu_* o, void* v) {
  ((ICCfltkBetrachter*)(o->parent()->parent()->user_data()))->cb_menueintrag_3D_i(o,v);
}

inline void ICCfltkBetrachter::cb_menueintrag_inspekt_i(Fl_Menu_* o, void*) {
  Fl_Menu_* mw = (Fl_Menu_*)o;
  const Fl_Menu_Item* m = mw->mvalue();

  DBG_PROG_S (m->value())

  if (m->value())
  { bool export_html = false;
    widget_oben = WID_INSPEKT;
    inspekt_html->value(profile.profil()->report(export_html).c_str());
    inspekt_html->topline(tag_text->inspekt_topline);
  } else {
    widget_oben = WID_0;
    tag_text->inspekt_topline = inspekt_html->topline();
  }
  icc_examin->icc_betrachterNeuzeichnen(inspekt_html);
}
void ICCfltkBetrachter::cb_menueintrag_inspekt(Fl_Menu_* o, void* v) {
  ((ICCfltkBetrachter*)(o->parent()->parent()->user_data()))->cb_menueintrag_inspekt_i(o,v);
}

inline void ICCfltkBetrachter::cb_menueintrag_zeigcgats_i(Fl_Menu_* o, void*) {
  Fl_Menu_* mw = (Fl_Menu_*)o;
  const Fl_Menu_Item* m = mw->mvalue();

  DBG_PROG_S (m->value())

  icc_examin->zeigCGATS();
}
void ICCfltkBetrachter::cb_menueintrag_zeigcgats(Fl_Menu_* o, void* v) {
  ((ICCfltkBetrachter*)(o->parent()->parent()->user_data()))->cb_menueintrag_zeigcgats_i(o,v);
}

inline void ICCfltkBetrachter::cb_menueintrag_vcgt_i(Fl_Menu_*, void*) {
  vcgt->show();
icc_examin->vcgtZeigen();
}
void ICCfltkBetrachter::cb_menueintrag_vcgt(Fl_Menu_* o, void* v) {
  ((ICCfltkBetrachter*)(o->parent()->parent()->user_data()))->cb_menueintrag_vcgt_i(o,v);
}

inline void ICCfltkBetrachter::cb_menueintrag_testkurven_i(Fl_Menu_*, void*) {
  vcgt->show();
icc_examin->testZeigen();
}
void ICCfltkBetrachter::cb_menueintrag_testkurven(Fl_Menu_* o, void* v) {
  ((ICCfltkBetrachter*)(o->parent()->parent()->user_data()))->cb_menueintrag_testkurven_i(o,v);
}

inline void ICCfltkBetrachter::cb_About_i(Fl_Menu_*, void*) {
  ueber->show();
ueber_html->value(getUeberHtml().c_str());
}
void ICCfltkBetrachter::cb_About(Fl_Menu_* o, void* v) {
  ((ICCfltkBetrachter*)(o->parent()->parent()->user_data()))->cb_About_i(o,v);
}

Fl_Menu_Item ICCfltkBetrachter::menu_menueleiste[] = {
 {_("File"), 0,  0, 0, 64, 0, 0, 14, 56},
 {_("Open"), 0x4006f,  (Fl_Callback*)ICCfltkBetrachter::cb_Open, 0, 0, 0, 0, 14, 56},
 {_("Save Report"), 0,  (Fl_Callback*)ICCfltkBetrachter::cb_menueintrag_html_speichern, 0, 129, 0, 0, 14, 56},
 {_("Save Gamut as Profile"), 0,  (Fl_Callback*)ICCfltkBetrachter::cb_menueintrag_gamut_speichern, 0, 1, 0, 0, 14, 56},
 {_("Save Gamut as VRML"), 0,  (Fl_Callback*)ICCfltkBetrachter::cb_menueintrag_gamut_vrml_speichern, 0, 129, 0, 0, 14, 56},
 {_("Quit"), 0x40071,  (Fl_Callback*)ICCfltkBetrachter::cb_Quit, 0, 0, 0, 0, 14, 56},
 {0,0,0,0,0,0,0,0,0},
 {_("Edit"), 0,  0, 0, 80, 0, 0, 14, 56},
 {_("Oyranos Settings"), 0,  (Fl_Callback*)ICCfltkBetrachter::cb_Oyranos, 0, 0, 0, 0, 14, 56},
 {0,0,0,0,0,0,0,0,0},
 {_("Options"), 0,  0, 0, 64, 0, 0, 14, 56},
 {_("Gamut Warning"), 0x40077,  (Fl_Callback*)ICCfltkBetrachter::cb_menueintrag_gamutwarn, 0, 2, 0, 0, 14, 56},
 {0,0,0,0,0,0,0,0,0},
 {_("Gamut"), 0x40068,  (Fl_Callback*)ICCfltkBetrachter::cb_menueintrag_huelle, 0, 2, 0, 0, 14, 56},
 {_("View"), 0,  0, 0, 192, 0, 0, 14, 56},
 {_("Whole Screen on/off"), 0x40076,  (Fl_Callback*)ICCfltkBetrachter::cb_menueintrag_Voll, 0, 0, 0, 0, 14, 56},
 {_("Gamut"), 0x40068,  (Fl_Callback*)ICCfltkBetrachter::cb_menueintrag_3D, 0, 130, 0, 0, 14, 56},
 {_("Report View"), 0x40062,  (Fl_Callback*)ICCfltkBetrachter::cb_menueintrag_inspekt, 0, 3, 0, 0, 14, 56},
 {_("CGATS View"), 0x40067,  (Fl_Callback*)ICCfltkBetrachter::cb_menueintrag_zeigcgats, 0, 129, 0, 0, 14, 56},
 {_("Videocard Gamma"), 0x40074,  (Fl_Callback*)ICCfltkBetrachter::cb_menueintrag_vcgt, 0, 0, 0, 0, 14, 56},
 {_("Test Curves"), 0x50074,  (Fl_Callback*)ICCfltkBetrachter::cb_menueintrag_testkurven, 0, 16, 0, 0, 14, 56},
 {_("3D"), 0,  0, 0, 80, 0, 0, 14, 56},
 {0,0,0,0,0,0,0,0,0},
 {0,0,0,0,0,0,0,0,0},
 {_("Help"), 0,  0, 0, 64, 0, 0, 14, 56},
 {_("About"), 0xffbe,  (Fl_Callback*)ICCfltkBetrachter::cb_About, 0, 0, 0, 0, 14, 56},
 {0,0,0,0,0,0,0,0,0},
 {0,0,0,0,0,0,0,0,0}
};
Fl_Menu_Item* ICCfltkBetrachter::menueintrag_html_speichern = ICCfltkBetrachter::menu_menueleiste + 2;
Fl_Menu_Item* ICCfltkBetrachter::menueintrag_gamut_speichern = ICCfltkBetrachter::menu_menueleiste + 3;
Fl_Menu_Item* ICCfltkBetrachter::menueintrag_gamut_vrml_speichern = ICCfltkBetrachter::menu_menueleiste + 4;
Fl_Menu_Item* ICCfltkBetrachter::menueintrag_gamutwarn = ICCfltkBetrachter::menu_menueleiste + 11;
Fl_Menu_Item* ICCfltkBetrachter::menueintrag_huelle = ICCfltkBetrachter::menu_menueleiste + 13;
Fl_Menu_Item* ICCfltkBetrachter::menueintrag_Voll = ICCfltkBetrachter::menu_menueleiste + 15;
Fl_Menu_Item* ICCfltkBetrachter::menueintrag_3D = ICCfltkBetrachter::menu_menueleiste + 16;
Fl_Menu_Item* ICCfltkBetrachter::menueintrag_inspekt = ICCfltkBetrachter::menu_menueleiste + 17;
Fl_Menu_Item* ICCfltkBetrachter::menueintrag_zeigcgats = ICCfltkBetrachter::menu_menueleiste + 18;
Fl_Menu_Item* ICCfltkBetrachter::menueintrag_vcgt = ICCfltkBetrachter::menu_menueleiste + 19;
Fl_Menu_Item* ICCfltkBetrachter::menueintrag_testkurven = ICCfltkBetrachter::menu_menueleiste + 20;
Fl_Menu_Item* ICCfltkBetrachter::untermenue_3D = ICCfltkBetrachter::menu_menueleiste + 21;
Fl_Menu_Item* ICCfltkBetrachter::menu_hilfe = ICCfltkBetrachter::menu_menueleiste + 23;

inline void ICCfltkBetrachter::cb_tag_browser_i(TagBrowser* o, void*) {
  o->selectItem( o->value() );
}
void ICCfltkBetrachter::cb_tag_browser(TagBrowser* o, void* v) {
  ((ICCfltkBetrachter*)(o->parent()->parent()->parent()->user_data()))->cb_tag_browser_i(o,v);
}

inline void ICCfltkBetrachter::cb_mft_choice_i(MftChoice* o, void*) {
  o->auswahlCb();
}
void ICCfltkBetrachter::cb_mft_choice(MftChoice* o, void* v) {
  ((ICCfltkBetrachter*)(o->parent()->parent()->parent()->parent()->parent()->user_data()))->cb_mft_choice_i(o,v);
}

Fl_Double_Window* ICCfltkBetrachter::init(int argc, char** argv) {
  Fl_Double_Window* w;
  DBG_PROG_START
  fullscreen = false;
  setTitleUrl = true;

  #ifdef HAVE_FLU
    Flu_File_Chooser::favoritesTxt = _("Favorites");
    Flu_File_Chooser::myComputerTxt = _("Home");
    Flu_File_Chooser::myDocumentsTxt = _("Documents");
    Flu_File_Chooser::desktopTxt = _("Desktop");

    Flu_File_Chooser::detailTxt[0] = _("Name");
    Flu_File_Chooser::detailTxt[3] = _("Type");
    Flu_File_Chooser::detailTxt[1] = _("Size");
    Flu_File_Chooser::detailTxt[2] = _("Date");
    Flu_File_Chooser::contextMenuTxt[0] = _("New Folder");
    Flu_File_Chooser::contextMenuTxt[1] = _("Rename");
    Flu_File_Chooser::contextMenuTxt[2] = _("Delete");
    Flu_File_Chooser::diskTypesTxt[0] = _("Floppy Disk");
    Flu_File_Chooser::diskTypesTxt[1] = _("Removable Disk");
    Flu_File_Chooser::diskTypesTxt[2] = _("Local Disk");
    Flu_File_Chooser::diskTypesTxt[3] = _("Compact Disk");
    Flu_File_Chooser::diskTypesTxt[4] = _("Network Disk");
    Flu_File_Chooser::diskTypesTxt[5] = _("RAM Disk");

    Flu_File_Chooser::filenameTxt = _("Filename");
    Flu_File_Chooser::okTxt = _("Yes");
    Flu_File_Chooser::cancelTxt = _("Cancel");
    Flu_File_Chooser::locationTxt = _("Folder");
    Flu_File_Chooser::showHiddenTxt = _("Show Hidden Files");
    Flu_File_Chooser::fileTypesTxt = _("File Types");
    Flu_File_Chooser::directoryTxt = _("Folder");
    Flu_File_Chooser::allFilesTxt = _("All Files (*)");
    Flu_File_Chooser::defaultFolderNameTxt = _("New Folder");

    Flu_File_Chooser::backTTxt = _("Go back one directory in the history");
    Flu_File_Chooser::forwardTTxt = _("Go forward one directory in the history");
    Flu_File_Chooser::upTTxt = _("Go to the parent directory");
    Flu_File_Chooser::reloadTTxt = _("Refresh this directory");
    Flu_File_Chooser::trashTTxt = _("Delete file(s)");
    Flu_File_Chooser::newDirTTxt = _("Create new directory");
    Flu_File_Chooser::addFavoriteTTxt = _("Add this directory to my favorites");
    Flu_File_Chooser::previewTTxt = _("Preview files");
    Flu_File_Chooser::listTTxt = _("List mode");
    Flu_File_Chooser::wideListTTxt = _("Wide list mode");
    Flu_File_Chooser::detailTTxt = _("Detail mode");

    Flu_File_Chooser::createFolderErrTxt = _("Could not create directory '%s'. You may not have permission to perform this operation.");
    Flu_File_Chooser::deleteFileErrTxt = _("An error ocurred while trying to delete '%s'.");
    Flu_File_Chooser::fileExistsErrTxt = _("File '%s' already exists!");
    Flu_File_Chooser::renameErrTxt = _("Unable to rename '%s' to '%s'");

    const char* ptr = NULL;
    if (profile.size())
      ptr = profile.name().c_str();
    dateiwahl = new Flu_File_Chooser(ptr, _("ICC colour profiles (*.ic*)"), Flu_File_Chooser::/*SINGLE*/MULTI, _("Which ICC profile?"));
    dateiwahl->add_context_handler(Flu_File_Chooser::ENTRY_FILE, "icc", _("Open Profile"), dateiwahl_cb, NULL);
    dateiwahl->add_context_handler(Flu_File_Chooser::ENTRY_FILE, "icm", _("Open Profile"), dateiwahl_cb, NULL);
    dateiwahl->add_context_handler(Flu_File_Chooser::ENTRY_FILE, "it8", _("Open Measurement"), dateiwahl_cb, NULL);
    dateiwahl->add_context_handler(Flu_File_Chooser::ENTRY_FILE, "txt", _("Open Measurement"), dateiwahl_cb, NULL);
    dateiwahl->add_context_handler(Flu_File_Chooser::ENTRY_FILE, "IT8", _("Open Measurement"), dateiwahl_cb, NULL);
    dateiwahl->add_context_handler(Flu_File_Chooser::ENTRY_FILE, "Q60", _("Open Measurement"), dateiwahl_cb, NULL);
    dateiwahl->add_context_handler(Flu_File_Chooser::ENTRY_FILE, "LAB", _("Open Measurement"), dateiwahl_cb, NULL);
    dateiwahl->add_context_handler(Flu_File_Chooser::ENTRY_FILE, "CMYK", _("Open Measurement"), dateiwahl_cb, NULL);
    dateiwahl->add_context_handler(Flu_File_Chooser::ENTRY_FILE, "DLY", _("Open Measurement"), dateiwahl_cb, NULL);
    dateiwahl->add_context_handler(Flu_File_Chooser::ENTRY_FILE, "nCIE", _("Open Measurement"), dateiwahl_cb, NULL);
  #else
    const char* ptr = NULL;
    if (profile.size())
      ptr = profile.name().c_str();
    dateiwahl = new Fl_File_Chooser(ptr, _("ICC colour profiles (*.{I,i}{C,c}{M,m,C,c})"), Fl_File_Chooser::SINGLE, _("Which ICC profile?"));
    dateiwahl->callback(dateiwahl_cb);
    dateiwahl->preview_label = _("Preview");
  #endif
  { Fl_Double_Window* o = ueber = new Fl_Double_Window(366, 241, _("About ICC Examin"));
    w = o;
    o->user_data((void*)(this));
    { Fl_Group* o = new Fl_Group(0, 0, 365, 240);
      { Fl_Help_View* o = ueber_html = new Fl_Help_View(0, 0, 365, 205);
        Fl_Group::current()->resizable(o);
      }
      { Fl_Button* o = ja = new Fl_Button(130, 210, 110, 25, _("Yes"));
        o->callback((Fl_Callback*)cb_ja);
      }
      o->end();
    }
    o->hide();
    o->set_non_modal();
    o->end();
    o->resizable(o);
  }
  { Fl_Double_Window* o = vcgt = new Fl_Double_Window(370, 390, _("Grafic Card Gamma Table"));
    w = o;
    o->user_data((void*)(this));
    { Fl_Group* o = new Fl_Group(0, 0, 370, 390);
      { TagDrawings* o = vcgt_viewer = new TagDrawings(0, 0, 370, 360);
        o->box(FL_NO_BOX);
        o->color(FL_BACKGROUND_COLOR);
        o->selection_color(FL_BACKGROUND_COLOR);
        o->labeltype(FL_NORMAL_LABEL);
        o->labelfont(0);
        o->labelsize(14);
        o->labelcolor(FL_BLACK);
        o->align(FL_ALIGN_CENTER);
        o->when(FL_WHEN_RELEASE);
        o->show();
      }
      { Fl_Button* o = vcgt_set_button = new Fl_Button(10, 360, 65, 25, _("Set"));
        o->tooltip(_("Set the current profile as monitor profile and upload vcgt tag to the video c\
ard"));
        o->callback((Fl_Callback*)cb_vcgt_set_button);
        w->hotspot(o);
      }
      { Fl_Button* o = vcgt_reset_button = new Fl_Button(75, 360, 75, 25, _("Reset"));
        o->tooltip(_("reset to standard gamma"));
        o->callback((Fl_Callback*)cb_vcgt_reset_button);
        w->hotspot(o);
      }
      { Fl_Button* o = vcgt_load_button = new Fl_Button(150, 360, 65, 25, _("Load"));
        o->tooltip(_("Load the current monitor profile"));
        o->callback((Fl_Callback*)cb_vcgt_load_button);
        w->hotspot(o);
      }
      { Fl_Button* o = vcgt_refresh_button = new Fl_Button(215, 360, 75, 25, _("Refresh"));
        o->tooltip(_("refresh gamma table from video card"));
        o->callback((Fl_Callback*)cb_vcgt_refresh_button);
        w->hotspot(o);
      }
      { Fl_Button* o = vcgt_close_button = new Fl_Button(290, 360, 75, 25, _("Close"));
        o->callback((Fl_Callback*)cb_vcgt_close_button);
      }
      o->end();
    }
    o->hide();
    o->end();
    o->resizable(o);
  }
  { Fl_Double_Window* o = details = new Fl_Double_Window(385, 520, _("ICC Examin"));
    w = o;
    o->box(FL_NO_BOX);
    o->color((Fl_Color)53);
    o->user_data((void*)(this));
    { Fl_Group* o = new Fl_Group(0, 0, 385, 520);
      { Fl_Menu_Bar* o = menueleiste = new Fl_Menu_Bar(0, 0, 385, 25);
        o->align(FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
        o->when(3);
        { Fl_Menu_Item* o = &menu_menueleiste[19];
          o->hide();
        }
        o->menu(menu_menueleiste);
      }
      { GL_Ansicht* o = DD_farbraum = new GL_Ansicht(0, 25, 385, 470);
        o->box(FL_NO_BOX);
        o->color(FL_BACKGROUND_COLOR);
        o->selection_color(FL_BACKGROUND_COLOR);
        o->labeltype(FL_NORMAL_LABEL);
        o->labelfont(0);
        o->labelsize(14);
        o->labelcolor(FL_BLACK);
        o->align(FL_ALIGN_CENTER);
        o->when(FL_WHEN_RELEASE);
        o->hide();
      }
      { Fl_Help_View* o = inspekt_html = new Fl_Help_View(0, 25, 385, 470, _("Inspect"));
        o->align(FL_ALIGN_CENTER|FL_ALIGN_INSIDE);
        o->hide();
      }
      { Fl_Tile* o = examin = new Fl_Tile(0, 25, 385, 470);
        { TagBrowser* o = tag_browser = new TagBrowser(0, 25, 385, 135, _("Tags"));
          o->tooltip(_("Choose one profile tag"));
          o->box(FL_NO_BOX);
          o->color(FL_BACKGROUND_COLOR);
          o->selection_color(FL_SELECTION_COLOR);
          o->labeltype(FL_NORMAL_LABEL);
          o->labelfont(0);
          o->labelsize(14);
          o->labelcolor(FL_BLACK);
          o->textcolor(32);
          o->callback((Fl_Callback*)cb_tag_browser);
          o->align(FL_ALIGN_TOP|FL_ALIGN_INSIDE);
          o->when(FL_WHEN_RELEASE_ALWAYS);
          int lines = tag_browser->size();
          DBG_PROG_V( lines )
        }
        { Fl_Group* o = ansichtsgruppe = new Fl_Group(0, 160, 385, 335);
          { Fl_Group* o = tabellengruppe = new Fl_Group(0, 160, 385, 335);
            { MftChoice* o = mft_choice = new MftChoice(0, 160, 385, 25, _("Chain selection"));
              o->tooltip(_("Choose a attribute"));
              o->box(FL_NO_BOX);
              o->down_box(FL_BORDER_BOX);
              o->color(FL_BACKGROUND_COLOR);
              o->selection_color(FL_SELECTION_COLOR);
              o->labeltype(FL_NORMAL_LABEL);
              o->labelfont(0);
              o->labelsize(14);
              o->labelcolor(FL_BLACK);
              o->callback((Fl_Callback*)cb_mft_choice);
              o->align(FL_ALIGN_LEFT);
              o->when(FL_WHEN_RELEASE);
              o->show();
            }
            { TagDrawings* o = mft_viewer = new TagDrawings(0, 185, 385, 310);
              o->box(FL_NO_BOX);
              o->color(FL_BACKGROUND_COLOR);
              o->selection_color(FL_BACKGROUND_COLOR);
              o->labeltype(FL_NORMAL_LABEL);
              o->labelfont(0);
              o->labelsize(14);
              o->labelcolor(FL_BLACK);
              o->align(FL_ALIGN_CENTER);
              o->when(FL_WHEN_RELEASE);
              o->show();
            }
            { TagTexts* o = mft_text = new TagTexts(0, 185, 385, 310, _("Chain -texts"));
              o->box(FL_NO_BOX);
              o->color((Fl_Color)48);
              o->selection_color(FL_SELECTION_COLOR);
              o->labeltype(FL_NORMAL_LABEL);
              o->labelfont(0);
              o->labelsize(14);
              o->labelcolor(FL_BLACK);
              o->textcolor(32);
              o->align(FL_ALIGN_BOTTOM|FL_ALIGN_INSIDE);
              o->when(FL_WHEN_RELEASE_ALWAYS);
              o->show();
            }
            { GL_Ansicht* o = mft_gl = new GL_Ansicht(0, 185, 385, 310);
              o->box(FL_NO_BOX);
              o->color(FL_BACKGROUND_COLOR);
              o->selection_color(FL_BACKGROUND_COLOR);
              o->labeltype(FL_NORMAL_LABEL);
              o->labelfont(0);
              o->labelsize(14);
              o->labelcolor(FL_BLACK);
              o->align(FL_ALIGN_BOTTOM|FL_ALIGN_INSIDE);
              o->when(FL_WHEN_RELEASE);
              o->hide();
            }
            o->show();
            o->end();
          }
          { TagDrawings* o = tag_viewer = new TagDrawings(0, 160, 385, 335);
            o->box(FL_NO_BOX);
            o->color(FL_BACKGROUND_COLOR);
            o->selection_color(FL_BACKGROUND_COLOR);
            o->labeltype(FL_NORMAL_LABEL);
            o->labelfont(0);
            o->labelsize(14);
            o->labelcolor(FL_BLACK);
            o->align(FL_ALIGN_CENTER);
            o->when(FL_WHEN_RELEASE);
            o->hide();
          }
          { TagTexts* o = tag_text = new TagTexts(0, 160, 385, 335, _("Texts"));
            o->box(FL_NO_BOX);
            o->color(FL_BACKGROUND_COLOR);
            o->selection_color(FL_SELECTION_COLOR);
            o->labeltype(FL_NORMAL_LABEL);
            o->labelfont(0);
            o->labelsize(14);
            o->labelcolor(FL_BLACK);
            o->textcolor(32);
            o->align(FL_ALIGN_BOTTOM|FL_ALIGN_INSIDE);
            o->when(FL_WHEN_RELEASE_ALWAYS);
            o->show();
          }
          o->end();
        }
        o->end();
        Fl_Group::current()->resizable(o);
      }
      { Fl_Group* o = new Fl_Group(0, 495, 385, 25);
        { Fl_Box* o = box_stat = new Fl_Box(0, 495, 385, 25);
          o->box(FL_THIN_DOWN_BOX);
          o->align(FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
        }
        { Fl_Progress* o = load_progress = new Fl_Progress(0, 495, 385, 25, _("Loading .."));
          o->hide();
          o->minimum(0.0);
          o->maximum(1.0);
        }
        o->end();
      }
      o->end();
    }
    { My_Fl_Box* o = no_box = new My_Fl_Box(0, 0, 385, 520);
      o->box(FL_NO_BOX);
      o->color(FL_BACKGROUND_COLOR);
      o->selection_color(FL_BACKGROUND_COLOR);
      o->labeltype(FL_NORMAL_LABEL);
      o->labelfont(0);
      o->labelsize(14);
      o->labelcolor(FL_BLACK);
      o->align(FL_ALIGN_CENTER);
      o->when(FL_WHEN_RELEASE);
      o->box(FL_NO_BOX);
    }
    o->end();
  }
  tag_text->inspekt_topline = 0;

  w->resizable(tag_text);
  //Fl::background(190,190,190);
  //Fl::background(255,255,255);
  //Fl::scheme(NULL);
  Fl_File_Icon::load_system_icons();
  Fl::get_system_colors();
  w->show(/*argc,argv*/);
  DBG_PROG
  DBG_PROG_ENDE
  return w;
}

void ICCfltkBetrachter::run() {
  DBG_PROG_START
  DBG_PROG_ENDE;  Fl::run();
}

std::vector<std::string> ICCfltkBetrachter::open(std::vector<std::string> dateinamen) {
  DBG_PROG_START
  #include "icc_vrml.h"

  //Fl_File_Icon	*icon;	// New file icon
  DBG_PROG

    const char* ptr = NULL;
    if (dateinamen.size()) {
      ptr = dateinamen[0].c_str();
      dateiwahl->value(ptr);
      DBG_PROG_S( dateinamen[0])
    } 
      if(ptr) DBG_PROG_V( ptr );
    if (!ptr)
      ptr = getenv("PWD");

      if(ptr) DBG_PROG_V( ptr )
    if(( ptr &&
        (ptr[0] == '/') &&
        (strlen(ptr) == 1) ) ||
        !ptr )
    {
      ptr = getenv("HOME");
    }
    if(ptr)
      dateiwahl->value(ptr);

    dateiwahl->show(); //filename=fl_file_chooser("W�hle ICC Profil?", "ICC Farbprofile (*.{I,i}{C,c}{M,m,C,c})", filenamen_alt[0].c_str());

    


    while (dateiwahl->visible())
      Fl::wait();

    DBG_NUM_V( dateiwahl->count() )
    if (dateiwahl->count() && dateiwahl->value()) {
      DBG_NUM_V( dateiwahl->value() )
      dateinamen.resize(dateiwahl->count());
      for (int i = 1; i <= dateiwahl->count(); i++)
        dateinamen[i-1] = dateiwahl->value(i);
    }
  DBG_PROG


  if (dateinamen.size() == 0) {
    DBG_PROG_ENDE
    return dateinamen;
  }

  DBG_PROG_ENDE
  return dateinamen;
}

void ICCfltkBetrachter::quit(void) {
  DBG_PROG_START
  /*Fl::remove_timeout((void (*)(void *))timeIT, (void *)viewer);
  delete viewer;
  delete browser;
  delete canvas;*/
  details->hide();
  icc_examin->quit();
  DBG_PROG_ENDE
}

void ICCfltkBetrachter::zeig_mich_(void* widget) {
  DBG_PROG_START
  // zeigt das ausgew�hlte Fenster (widget)

  mft_viewer->hide();
  mft_text->hide();
  tag_viewer->hide(); DBG_PROG
  tag_viewer->clear_visible(); DBG_PROG
  tag_text->hide();
  inspekt_html->hide();

  // stop
  if (widget != mft_gl) {
    mft_gl->hide();
  }
  if (widget != DD_farbraum ) {
    DD_farbraum->hide();
  }

  // start
  if( widget == inspekt_html )
  { DBG_PROG_S("inspekt_html behandeln")
    if( menueintrag_inspekt->value() )
      inspekt_html->show();
    else
      inspekt_html->hide();
  }

  if (widget != mft_gl &&
      widget != DD_farbraum )
  { 
    ((Fl_Widget*)widget)->show(); DBG_PROG
    if( !menueintrag_inspekt->value() &&
        !menueintrag_3D->value() )
      ;//selectItem(icc_examin->tag_nr()+6);
  } else if (widget == mft_gl) {
    DBG_PROG_S( "mft GL Fenster belassen." )
    mft_gl->show();
  } else if (widget == DD_farbraum) {
    DBG_PROG_S( "3D GL Fenster belassen." )
    DD_farbraum->show();
  }
  DBG_PROG_ENDE
}

void ICCfltkBetrachter::measurement(bool has_measurement) {
  if (has_measurement) {
    DBG_PROG_S(menueintrag_inspekt->value())
    if (menueintrag_inspekt->value()) {
      inspekt_html->value(profile.profil()->report(false).c_str());
      if (inspekt_html->size() -75 < tag_text->inspekt_topline)
        inspekt_html->topline (inspekt_html->size() - 75);
      else
        inspekt_html->topline (tag_text->inspekt_topline);
    }
    menueintrag_inspekt->activate();
    menueintrag_html_speichern->activate();
    menueintrag_zeigcgats->activate();
  } else {
    menueintrag_inspekt->deactivate();
    menueintrag_html_speichern->deactivate();
    menueintrag_zeigcgats->deactivate();
  }
}

void dHaendler(void* o) {
  DBG_PROG_START
  Fl::remove_timeout( (void(*)(void*))dHaendler, 0 );

  if (!Fl::has_timeout( (void(*)(void*))dHaendler, 0 )
   && ((TagDrawings*)o)->active()
   && ((TagDrawings*)o)->visible_r()
   && ((TagDrawings*)o)->wiederholen)
  {
    ((TagDrawings*)o)->ruhigNeuzeichnen();

    #ifdef DEBUG
    DBG_PROG_V( ((TagDrawings*)o)->wiederholen )
    #endif
  }
  DBG_PROG_ENDE
}
