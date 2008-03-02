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
 * Die zentrale Klasse.
 * 
 */


#include "icc_betrachter.h"
#include "icc_draw.h"
#include "icc_examin.h"
#include "icc_gl.h"
#include "icc_helfer.h"
#include "icc_helfer_ui.h"
#include "icc_helfer_fltk.h"
#include "icc_fenster.h"
#include "icc_info.h"
#include "icc_kette.h"
#include "icc_waehler.h"

#if APPLE
#include <Carbon/Carbon.h>
#endif

using namespace icc_examin_ns;

#if USE_THREADS
#include "threads.h"
#else
#endif

//#define DEBUG_EXAMIN
#ifdef DEBUG_EXAMIN
#define DBG_EXAMIN_START DBG_PROG_START
#define DBG_EXAMIN_ENDE DBG_PROG_ENDE
#define DBG_EXAMIN_V( texte ) DBG_NUM_V( texte )
#define DBG_EXAMIN_S( texte ) DBG_NUM_S( texte )
#else
#define DBG_EXAMIN_START
#define DBG_EXAMIN_ENDE
#define DBG_EXAMIN_V( texte )
#define DBG_EXAMIN_S( texte )
#endif


ICCexamin::ICCexamin ()
{ DBG_PROG_START
  icc_examin_ns::lock(__FILE__,__LINE__);
  icc_betrachter = new ICCfltkBetrachter;
  profile.init();

  lade_ = false;
  neu_laden_ = true;
  _item = -1;
  _mft_item = -1;
  farbraum_modus_ = false;
  statlabel = "";
  status_ = false;
  intent_ = 3;
  gamutwarn_ = 0;
  DBG_PROG_ENDE
}

void
ICCexamin::quit ()
{ DBG_PROG_START
  delete icc_betrachter;
  profile.clear();
  DBG_PROG_ENDE
  exit(0);
}

void
resize_fuer_menubar(Fl_Widget* w)
{
# if APPLE
  w->resize( w->x(), w->y()-25, w->w(), w->h()+25 );
# endif
}

void
ICCexamin::start (int argc, char** argv)
{ DBG_PROG_START

  kurven.resize(MAX_VIEWER);
  punkte.resize(MAX_VIEWER);
  texte.resize(MAX_VIEWER);
  kurve_umkehren.resize(MAX_VIEWER);

  menue_translate( icc_betrachter->menu_menueleiste );

# if USE_THREADS
  static Fl_Thread fl_t;
  DBG_THREAD_V( fl_t )
  int fehler = fl_create_thread( fl_t, &oeffnenStatisch_, (void *)this );
# if HAVE_PTHREAD_H
  icc_thread_liste[THREAD_LADEN] = fl_t;
# endif
  if( fehler == EAGAIN)
  {
    WARN_S( _("Waechter Thread nicht gestartet Fehler: ")  << fehler );
  } else
# if !APPLE && !WIN32
  if( fehler == PTHREAD_THREADS_MAX )
  {
    WARN_S( _("zu viele Waechter Threads Fehler: ") << fehler );
  } else
# endif
  if( fehler != 0 )
  {
    WARN_S( _("unbekannter Fehler beim Start eines Waechter Threads Fehler: ") << fehler );
  }
# else
  Fl::add_timeout( 0.01, /*(void(*)(void*))*/oeffnenStatisch_ ,(void*)this);
# endif

  icc_betrachter->init( argc, argv );

  icc_betrachter->mft_gl->init(1);
  icc_betrachter->DD_farbraum->init(2);
  icc_waehler_ = new  ICCwaehler(485, 110, _("Gamut selector"));
  icc_waehler_->resize(icc_waehler_->x(), icc_waehler_->y(),
                       icc_waehler_->w()+20, icc_waehler_->h());
  if(!icc_waehler_) WARN_S( _("icc_waehler_ nicht reservierbar") )
  icc_waehler_->hide();

  // Die TagViewers registrieren und ihre Variablen initialisieren
  icc_betrachter->tag_viewer->id = TAG_VIEWER;
  icc_betrachter->mft_viewer->id = MFT_VIEWER;
  icc_betrachter->vcgt_viewer->id = VCGT_VIEWER;

  // Fuer eine Fl_Sys_Menu_Bar
# if 0
  resize_fuer_menubar( icc_betrachter->DD_farbraum );
  resize_fuer_menubar( icc_betrachter->examin );
  resize_fuer_menubar( icc_betrachter->inspekt_html );
# endif
  DBG_PROG

# if HAVE_X || APPLE
  icc_betrachter->menueintrag_vcgt->show();
  DBG_PROG_S( "Zeige vcgt" )
# else
  DBG_PROG_S( "Zeige vcgt nicht" )
# endif

# if APPLE
  // osX Rsourcen
  IBNibRef nibRef;
  OSStatus err;
  err = CreateNibReference(CFSTR("main"), &nibRef);
  require_noerr( err, CantGetNibRef );
  // Once the nib reference is created, set the menu bar. "MainMenu" is the name of the menu bar
  // object. This name is set in InterfaceBuilder when the nib is created.
  err = SetMenuBarFromNib(nibRef, CFSTR("MenuBar"));
  require_noerr( err, CantSetMenuBar );
  // We don't need the nib reference anymore.
  DisposeNibReference(nibRef);
  CantSetMenuBar:
  CantGetNibRef:
# endif // APPLE

  if(!icc_debug)
    icc_betrachter->menueintrag_testkurven->hide();

  DBG_PROG

  modellDazu( /*ICCkette*/&profile ); // wird in nachricht ausgewertet

  Fl::add_handler(tastatur);

      if (argc>1) {
        status( argv[1] << " " << _("loaded") )
        std::vector<std::string>profilnamen;
        profilnamen.resize(argc-1);
        for (int i = 1; i < argc; i++) {
          DBG_PROG_S( i <<" "<< argv[i] )
          profilnamen[i-1] = argv[i];
        }
        oeffnen (profilnamen);
      } else {
        status(_("Ready"))
      }

  // zur Benutzung freigeben
  status_ = 1;
  frei_ = true;

  // receive events
# if 0
  Fl_Widget* w = dynamic_cast<Fl_Widget*>(icc_betrachter->details);
  if (w) {
      Fl::pushed(w);
      DBG_PROG_S( "pushed("<< w <<") "<< Fl::pushed() )
  }
# endif

  icc_betrachter->run();

  DBG_PROG_ENDE
}

// TODO: beseitige Hack
static int frei_tuen = 0;
#define frei_ frei_tuen

void
ICCexamin::zeigPrueftabelle ()
{ DBG_PROG_START
  neuzeichnen(icc_betrachter->inspekt_html);
  DBG_PROG_ENDE
}

void
ICCexamin::zeigCGATS()
{
  DBG_PROG_START
  icc_examin_ns::lock(__FILE__,__LINE__);
  // CGATS in Fenster praesentieren
  icc_examin_ns::nachricht(profile.profil()->cgats_max());
  icc_examin_ns::unlock(this, __FILE__,__LINE__);
  DBG_PROG_ENDE
}

// virtual aus icc_examin_ns::Beobachter::
void
ICCexamin::nachricht( Modell* modell , int info )
{
  DBG_PROG_START
  if(!frei()) {
    WARN_S("icc_examin ist nicht frei")
    //DBG_PROG_ENDE
    //return;
  }

  frei_ = false;
  DBG_PROG_V( info )
  // Modell identifizieren
  ICCkette* k = dynamic_cast<ICCkette*>(modell);
  if(k && (k->size() > info))
  {
    DBG_PROG_S( _("Nachricht von ICCkette") )
    DBG_PROG_S( _("Auffrischen von Profil Nr.: ") << info )
    if(info>=0)
    {
      ICCprofile *p = (*k)[info];
      DBG_PROG_V( (int*)p )
      if (p && !p->changing())
      { DBG_PROG
        if(k->aktiv(info)) // momentan nicht genutzt
        {
          // ncl2 ?
          DBG_PROG_V( profile.aktuell() );
          int intent_neu = profile.profil()->intent();
          DBG_PROG_V( intent_neu <<" "<< intent_ )
          if(intent_ != intent_neu ||
             neu_laden_) {
            farbraum ();
            intent_ = intent_neu;
            neu_laden_ = false;
          } else
            farbraum (info);
          icc_examin->fortschrittThreaded(0.5);

          if (info < (int)icc_betrachter->DD_farbraum->dreiecks_netze.size())
            icc_betrachter->DD_farbraum->dreiecks_netze[info].aktiv = true;

        } else if (info < (int)icc_betrachter->DD_farbraum->dreiecks_netze.size()) {
          icc_betrachter->DD_farbraum->dreiecks_netze[info].aktiv = false;
        }
          // Oberflaechenpflege - Aktualisieren
        icc_examin->fortschrittThreaded(0.6);
        if(profile[info]->tagCount() <= _item)
          _item = (-1);
        DBG_PROG_V( _item )
 
        if(icc_betrachter->DD_farbraum->visible())
        {
          icc_betrachter->DD_farbraum->damage(FL_DAMAGE_ALL);
        }
        icc_examin->fortschrittThreaded(0.7);
        if(icc_betrachter->menueintrag_inspekt->active() &&
           profile[info]->hasMeasurement() )
          setzMesswerte();
        else if(icc_betrachter->examin->visible())
          waehleTag(_item);
        icc_examin->fortschrittThreaded(0.9);
      }
    }
  }

  Beobachter::nachricht(modell, info);
  icc_examin->fortschrittThreaded(1.0);
  icc_examin->fortschrittThreaded(1.1);
  frei_ = true;
  DBG_PROG_ENDE
}

void
ICCexamin::setzMesswerte()
{
  DBG_PROG_START
  bool export_html = false;
  if(icc_betrachter->menueintrag_inspekt->active()) {
    icc_betrachter->inspekt_html->value(profile.profil()->report(export_html).c_str());
    icc_betrachter->inspekt_html->topline(icc_betrachter->tag_text->inspekt_topline);
  }
  DBG_PROG_ENDE
}


void
ICCexamin::testZeigen ()
{ DBG_PROG_START

# if HAVE_X || APPLE
  std::vector<std::vector<std::pair<double,double> > > kurven2;
  kurven2.resize(8);
  kurven2[0].resize(4);
  kurven2[1].resize(3);
  kurven2[2].resize(56);
  kurven2[3].resize(56);
  kurven2[4].resize(56);
  kurven2[5].resize(56);
  kurven2[6].resize(56);
  kurven2[7].resize(56);
  for(unsigned int i = 0; i < kurven2.size(); ++i)
    for(unsigned int j = 0; j < kurven2[i].size(); ++j) {
      kurven2[i][j].first = sin(i) * 3.2 - 0.5* (cos(j*2)+0.1);
      kurven2[i][j].second = i * -0.2 + 0.05 * (sin(j/10.0)+2.7);
    }
  std::vector<std::string> txt;
  txt.resize(8);
  txt[0] = "ein Bild";
  txt[1] = "Gemälde";
  txt[2] = "fast HDR";
  txt[3] = "2 fast HDR";
  txt[4] = "3 fast HDR";
  txt[5] = "4 fast HDR";
  txt[6] = "5 fast HDR";
  txt[7] = "6 fast HDR";
  icc_betrachter->vcgt_viewer->hide();
  icc_betrachter->vcgt_viewer->show();
  icc_betrachter->vcgt_viewer->hineinDaten ( kurven2, txt );
  icc_betrachter->vcgt_viewer->kurve_umkehren = true;
# endif

  // TODO: osX
  DBG_PROG_ENDE
}

void
ICCexamin::vcgtZeigen ()
{ DBG_PROG_START
  frei_ = false;
  kurve_umkehren[VCGT_VIEWER] = true;

# if HAVE_X || APPLE
  std::string display_name = "";
  kurven[VCGT_VIEWER] = leseGrafikKartenGamma (display_name,texte[VCGT_VIEWER]);
  if (kurven[VCGT_VIEWER].size()) {
    icc_betrachter->vcgt_viewer->hide();
    icc_betrachter->vcgt_viewer->show();
    icc_betrachter->vcgt_viewer->hineinKurven( kurven[VCGT_VIEWER],
                                               texte [VCGT_VIEWER] );
    icc_betrachter->vcgt_viewer->kurve_umkehren = true;
  } else {
    WARN_S(_("Keine Kurve gefunden")) //TODO kleines Fenster
    icc_betrachter->vcgt_viewer->hide();
  }
# endif

  frei_ = true;
  // TODO: osX
  DBG_PROG_ENDE
}

void
ICCexamin::moniHolen ()
{ DBG_PROG_START
  //frei_ = false;
  fortschritt( 0.01 );

  static std::vector<Speicher> ss;
  ss.clear();
  ss.push_back(icc_oyranos.moni());
  size_t size = ss[0].size();
  const char *moni_profil = ss[0];
  if(!moni_profil || !size) {
    frei_ = true;
    DBG_PROG_ENDE
    return;
  }

  int erfolg = false;
  while(!erfolg) {
    if(kannLaden()) {
      lade(ss);
      erfolg = true;
    } else {
      // kurze Pause 
      DBG_THREAD_S( "muss warten" )
      icc_examin_ns::sleep(0.05);
    }
  }

  // TODO: X notification event
# if 0
  saveMemToFile("/tmp/vcgt_temp.icc", moni_profil, size);
  system ("xcalib /tmp/vcgt_temp.icc");
  remove ("/tmp/vcgt_temp.icc");
# endif
  vcgtZeigen();

  fortschritt( 1.1 );
  //frei_ = true;
  DBG_PROG_ENDE
}

void
ICCexamin::moniSetzen ()
{ DBG_PROG_START
  frei_ = false;
  if( profile.size() && profile.profil()->filename() &&
      strlen( profile.profil()->filename() ) ) { DBG_PROG
    icc_oyranos.setzeMonitorProfil( profile.profil()->filename() );
    vcgtZeigen();
  }
  frei_ = true;
  DBG_PROG_ENDE
}

void
ICCexamin::standardGamma ()
{ DBG_PROG_START
  frei_ = false;

# if HAVE_X
  system("xgamma -gamma 1.0");
  vcgtZeigen();
  icc_oyranos.setzeMonitorProfil( 0 );
# endif

  // TODO: osX
  frei_ = true;
  DBG_PROG_ENDE
}

void
ICCexamin::gamutAnsichtZeigen ()
{
      icc_betrachter->menueintrag_3D->set();
      icc_betrachter->menueintrag_huelle->set();
      icc_betrachter->widget_oben = ICCfltkBetrachter::WID_3D;
      farbraum_angezeigt_ = true;
      neuzeichnen(icc_betrachter->DD_farbraum);
      DBG_PROG_S("neuzeichnen DD_farbraum")
}

void
ICCexamin::neuzeichnen (void* z)
{ DBG_PROG_START
  Fl_Widget *wid = (Fl_Widget*)z;
  static int item;

  DBG_PROG_V( dynamic_cast<Fl_Widget*>(icc_betrachter->DD_farbraum)->visible())
  DBG_PROG_V( dynamic_cast<Fl_Widget*>(icc_betrachter->inspekt_html)->visible())
  DBG_PROG_V( dynamic_cast<Fl_Widget*>(icc_betrachter->examin)->visible() )
  DBG_PROG_V( dynamic_cast<Fl_Widget*>(icc_betrachter->tag_browser)->visible() )
  DBG_PROG_V( dynamic_cast<Fl_Widget*>(icc_betrachter->tag_text)->visible() )
  DBG_PROG_V( dynamic_cast<Fl_Widget*>(icc_betrachter->tag_viewer)->visible() )
  DBG_PROG_V( dynamic_cast<Fl_Widget*>(icc_betrachter->mft_choice)->visible() )
  DBG_PROG_V( dynamic_cast<Fl_Widget*>(icc_betrachter->mft_text)->visible() )
  DBG_PROG_V( dynamic_cast<Fl_Widget*>(icc_betrachter->mft_gl)->visible() )
  DBG_PROG_V( dynamic_cast<Fl_Widget*>(icc_betrachter->mft_viewer)->visible() )

  enum {ZEIGEN, VERSTECKEN, NACHRICHT, KEINEn};
# define widZEIG(zeigen,widget,dbg) { \
     if        (zeigen ==                  VERSTECKEN && widget->visible()) { \
         widget->                          hide(); \
         if(dbg==NACHRICHT) DBG_PROG_S( _("verstecke ") << #widget ); \
  \
     } else if (zeigen ==                  ZEIGEN     && !widget->visible()) { \
         widget->                          show(); \
         if(dbg==NACHRICHT) DBG_PROG_S( _("zeige     ") << #widget ); \
     } \
  }

  // oberstes Widget testen
  DBG_PROG_V( icc_betrachter->widget_oben )
  enum { DD_ZEIGEN, INSPEKT_ZEIGEN, TAG_ZEIGEN };
  int oben;
  if (icc_betrachter->menueintrag_3D->value() &&
      (   icc_betrachter->widget_oben == ICCfltkBetrachter::WID_3D
       || icc_betrachter->widget_oben == ICCfltkBetrachter::WID_0) )
    oben = DD_ZEIGEN;
  else if (icc_betrachter->menueintrag_inspekt->value() &&
      (   icc_betrachter->widget_oben == ICCfltkBetrachter::WID_INSPEKT
       || icc_betrachter->widget_oben == ICCfltkBetrachter::WID_0) )
    oben = INSPEKT_ZEIGEN;
  else
    oben = TAG_ZEIGEN;
  DBG_PROG_V( oben )

  // inhaltliches Zurückschalten auf tiefere Ebene
  bool waehle_tag = false;
  if( oben == TAG_ZEIGEN &&
      (icc_betrachter->DD_farbraum->visible()
    || icc_betrachter->inspekt_html->visible()) )
    waehle_tag = true;

  // die oberste Ebene zeigen/verstecken
  if(oben == DD_ZEIGEN) {
    if(!icc_betrachter->DD_farbraum->visible()) {
      DBG_PROG_S( "3D Histogramm zeigen" )
      icc_betrachter->DD_farbraum->show();
      widZEIG(ZEIGEN, icc_waehler_ ,NACHRICHT)
      widZEIG(VERSTECKEN, icc_betrachter->examin ,NACHRICHT)
    }
  } else {
    if(icc_betrachter->DD_farbraum->visible()) {
      DBG_PROG_S( "3D hist verstecken" )
      icc_betrachter->DD_farbraum->hide();
    }
    if(icc_waehler_->visible())
#   ifdef __APPLE__
      icc_waehler_->hide();
#   else
      icc_waehler_->iconize();
#   endif
  }

  if(oben == INSPEKT_ZEIGEN)
    widZEIG(ZEIGEN, icc_betrachter->inspekt_html ,NACHRICHT)
  else
    widZEIG(VERSTECKEN, icc_betrachter->inspekt_html ,NACHRICHT)

  if(oben == TAG_ZEIGEN) {
    widZEIG(ZEIGEN, icc_betrachter->examin ,NACHRICHT)
    widZEIG(ZEIGEN, icc_betrachter->tag_browser ,NACHRICHT)
    widZEIG(ZEIGEN, icc_betrachter->ansichtsgruppe ,NACHRICHT)
  } else {
    widZEIG(VERSTECKEN, icc_betrachter->examin ,NACHRICHT)
    widZEIG(VERSTECKEN, icc_betrachter->tag_browser ,NACHRICHT)
    widZEIG(VERSTECKEN, icc_betrachter->ansichtsgruppe ,NACHRICHT)
  }

  // Inhalte Erneuern
  if(waehle_tag)
    waehleTag(_item);

  // Bereinigen - hier?
  if (wid == icc_betrachter->tag_viewer ||
      wid == icc_betrachter->mft_viewer) {
    wid->clear_visible(); DBG_PROG_V( item << _item )
  }

  // Tabellenkompanion
  if (wid == icc_betrachter->mft_text ||
      wid == icc_betrachter->mft_gl ||
      wid == icc_betrachter->mft_viewer)
  { icc_betrachter->mft_choice->show(); DBG_PROG_S( "mft_choice zeigen" ) 
  } else
    icc_betrachter->mft_choice->hide();

# define SichtbarkeitsWechsel(widget) \
  { Fl_Widget *w = dynamic_cast<Fl_Widget*> (icc_betrachter->widget); \
    if (w != wid && w->visible()) { DBG_PROG_S( #widget << " verstecken" ) \
      w->hide(); \
    } else if(w == wid && !w->visible()) { DBG_PROG_S( #widget << " zeigen" ) \
      w->show(); \
      item = _item; \
    } \
  }

  SichtbarkeitsWechsel(mft_viewer)
  SichtbarkeitsWechsel(mft_gl)
  SichtbarkeitsWechsel(mft_text)
  SichtbarkeitsWechsel(tag_viewer)
  SichtbarkeitsWechsel(tag_text)

  // wenigstens ein Widget zeigen
  if(oben == TAG_ZEIGEN &&
     !icc_betrachter->mft_choice ->visible() &&
     !icc_betrachter->tag_viewer ->visible() )
    icc_betrachter->tag_text->show();

  DBG_PROG_V( dynamic_cast<Fl_Widget*>(icc_betrachter->DD_farbraum)->visible())
  DBG_PROG_V( dynamic_cast<Fl_Widget*>(icc_betrachter->inspekt_html)->visible())
  DBG_PROG_V( dynamic_cast<Fl_Widget*>(icc_betrachter->examin)->visible() )
  DBG_PROG_V( dynamic_cast<Fl_Widget*>(icc_betrachter->tag_browser)->visible() )
  DBG_PROG_V( dynamic_cast<Fl_Widget*>(icc_betrachter->ansichtsgruppe)->visible() )
  DBG_PROG_V( dynamic_cast<Fl_Widget*>(icc_betrachter->tag_text)->visible() )
  DBG_PROG_V( dynamic_cast<Fl_Widget*>(icc_betrachter->tag_viewer)->visible() )
  DBG_PROG_V( dynamic_cast<Fl_Widget*>(icc_betrachter->mft_choice)->visible() )
  DBG_PROG_V( dynamic_cast<Fl_Widget*>(icc_betrachter->mft_text)->visible() )
  DBG_PROG_V( dynamic_cast<Fl_Widget*>(icc_betrachter->mft_gl)->visible() )
  DBG_PROG_V( dynamic_cast<Fl_Widget*>(icc_betrachter->mft_viewer)->visible() )

  DBG_PROG_ENDE
}

void
ICCexamin::fortschritt(double f)
{ DBG_PROG_START
  if(0.0 < f && f <= 1.0) {
    if(!icc_betrachter->load_progress->visible())
      icc_betrachter->load_progress-> show();
    icc_betrachter->load_progress-> value( f );
    DBG_PROG_V( f )
  } else if (1.0 < f) {
    icc_betrachter->load_progress-> hide();
    DBG_PROG_V( f )
  } else {
    icc_betrachter->load_progress-> show();
    DBG_PROG_V( f )
  }
  DBG_PROG_ENDE
}

void
ICCexamin::fortschrittThreaded(double f)
{ DBG_PROG_START
  frei_ = false;
  icc_examin_ns::lock(__FILE__,__LINE__);
  if(0.0 < f && f <= 1.0) {
    if(!icc_betrachter->load_progress->visible())
      icc_betrachter->load_progress-> show();
    icc_betrachter->load_progress-> value( f );
    DBG_PROG_V( f )
  } else if (1.0 < f) {
    icc_betrachter->load_progress-> hide();
    DBG_PROG_V( f )
  } else {
    icc_betrachter->load_progress-> show();
    DBG_PROG_V( f )
  }
  icc_examin_ns::unlock(this, __FILE__,__LINE__);
  frei_ = true;
  DBG_PROG_ENDE
}

void
ICCexamin::statusAktualisieren()
{ DBG_PROG_START
  icc_betrachter->box_stat->label(statlabel.c_str());
  DBG_PROG_ENDE
}

int
tastatur(int e)
{ //DBG_PROG_START
  int gefunden = 0;
  static int dnd_kommt = false;

  switch (e)
  {
  case FL_SHORTCUT:
      if(Fl::event_key() == FL_Escape) {
        gefunden = 1;
        DBG_NUM_S("FL_Escape")
      } else
      if(Fl::event_key() == 'q'
       && Fl::event_state() == FL_CTRL) {
        DBG_NUM_S("FL_CTRL+Q")
        icc_examin->quit();
        gefunden = 1;
      }
    break;
  case FL_DND_ENTER:
    DBG_PROG_S( "FL_DND_ENTER" )
    fortschritt(0.01);
    return 1;
    break;
  case FL_DND_DRAG:
    DBG_PROG_S( "FL_DND_DRAG dnd_text_ops(" <<Fl::dnd_text_ops() <<")" )
    return 1;
    break;
  case FL_DND_LEAVE:
    DBG_PROG_S( "FL_DND_LEAVE" )
    fortschritt(1.1);
    return 1;
    break;
  case FL_DND_RELEASE:
    {
    DBG_PROG_S( "FL_DND_RELEASE " << Fl::event_length())
    icc_examin->icc_betrachter->details->take_focus();
    dnd_kommt = true;
    return 1;
    }
    break;
  case FL_PASTE:
    {
    DBG_PROG_S( "FL_PASTE " << Fl::event_length() )
#     if APPLE_
      if(dnd_kommt &&
         Fl::event_length())
      {
        DBG_PROG_S( Fl::event_text() );
        char *temp = (char*)alloca(Fl::event_length()+1),
             *text;
        sprintf(temp, Fl::event_text());
        std::vector<std::string>profilnamen;
        while((text = strrchr(temp,'\n')) != 0)
        {
          profilnamen.push_back(text+1);
          text[0] = 0;
        }
        profilnamen.push_back(temp);
        icc_examin->oeffnen(profilnamen);
      }
      dnd_kommt = false;
#     else
      if(dnd_kommt &&
         Fl::event_length())
      {
        {
          DBG_PROG_S( Fl::event_text() );
          char *temp = (char*)alloca(Fl::event_length()+1),
               *text;
          sprintf(temp, Fl::event_text());
          std::vector<std::string>profilnamen;
          while((text = strrchr(temp,'\n')) != 0)
          {
            if(strlen(text+1))
              profilnamen.push_back(text+1);
            text[0] = 0;
          }
          profilnamen.push_back(temp);
          // Korrekturen
          for(unsigned int i = 0; i < profilnamen.size(); ++i) {
            const char *filter_a = "file:";
            if(strstr(profilnamen[i].c_str(), filter_a)) {
              char *txt = (char*)alloca(profilnamen[i].size()+1);
              sprintf(txt, &(profilnamen[i].c_str())[strlen(filter_a)]);
              // Wagenruecklauf beseitigen
              char *zeiger = strchr(txt, '\r');
              if(zeiger)
                zeiger[0] = 0;
              profilnamen[i] = txt;
            }
            DBG_PROG_S( i <<" "<< profilnamen[i] );
          }
          icc_examin->oeffnen(profilnamen);
          dnd_kommt = false;
        }
      }
#     endif
    }
    break;
  case FL_RELEASE:
    DBG_PROG_S( "FL_RELEASE " << Fl::event_length() )
    break;
  case FL_DRAG:
    DBG_PROG_S( "FL_DRAG "<< Fl::event_length() )
    break;
  default: 
    {
      //if(Fl::event_length())
        dbgFltkEvents(e);
        DBG_MEM_S( Fl::event_length() << " bei: "<<Fl::event_x()<<","<<Fl::event_y() );
    }
    break;
  }
  
  icc_examin->icc_betrachter->DD_farbraum->tastatur(e);
  //DBG_PROG_ENDE
  return gefunden;
}


