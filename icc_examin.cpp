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


#include "icc_kette.h"
#include "icc_examin.h"
#include "icc_betrachter.h"
#include "icc_draw.h"
#include "icc_gl.h"
#include "icc_helfer_ui.h"
#include "icc_waehler.h"

using namespace icc_examin_ns;


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
  icc_betrachter = new ICCfltkBetrachter;
  _item = -1;
  _mft_item = -1;
  farbraum_modus_ = false;
  statlabel = "";
  status_ = false;
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
ICCexamin::start (int argc, char** argv)
{ DBG_PROG_START

  kurven.resize(MAX_VIEWER);
  punkte.resize(MAX_VIEWER);
  texte.resize(MAX_VIEWER);
  kurve_umkehren.resize(MAX_VIEWER);

  icc_betrachter->init( argc, argv );

  icc_betrachter->mft_gl->init(1);
  icc_betrachter->DD_farbraum->init(2);
  icc_waehler_ = new  ICCwaehler(485, 186, _("Gamut selector"));
  if(!icc_waehler_) WARN_S( _("icc_waehler_ nicht reservierbar") )
  icc_waehler_->hide();

  // Die TagViewers registrieren und ihre Variablen initialisieren
  icc_betrachter->tag_viewer->id = TAG_VIEWER;
  icc_betrachter->mft_viewer->id = MFT_VIEWER;
  icc_betrachter->vcgt_viewer->id = VCGT_VIEWER;

  DBG_PROG

  #if HAVE_X// || HAVE_OSX
  icc_betrachter->menueintrag_vcgt->show();
  DBG_PROG_S( "Zeige vcgt" )
  #else
  DBG_PROG_S( "Zeige vcgt nicht" )
  #endif
  if(!icc_debug)
    icc_betrachter->menueintrag_testkurven->hide();

  status("");
  DBG_PROG

  modellDazu( /*ICCkette*/&profile ); // wird in nachricht ausgewertet

      if (argc>1) {
        statlabel = argv[1];
        statlabel.append (" ");
        statlabel.append (_("loaded"));
        status(statlabel.c_str());
        std::vector<std::string>profilnamen;
        profilnamen.resize(argc-1);
        for (int i = 1; i < argc; i++) {
          DBG_PROG_S( i <<" "<< argv[i] )
          profilnamen[i-1] = argv[i];
        }
        oeffnen (profilnamen);
      } else {
        status(_("Ready"));
      }

  Fl::add_handler(tastatur);

  // zur Benutzung freigeben
  status_ = 1;
  frei_ = true;

  icc_betrachter->run();

  DBG_PROG_ENDE
}


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
  // CGATS in Fenster praesentieren
  icc_examin_ns::nachricht(profile.profil()->cgats_max());
  DBG_PROG_ENDE
}

// virtual aus icc_examin_ns::Beobachter::
void
ICCexamin::nachricht( Modell* modell , int info )
{
  DBG_PROG_START
  if(!frei()) {
    //DBG_PROG_ENDE
    //return;
  }
  Fl::lock();

  frei_ = false;
  DBG_PROG_V( info )
  // Modell identifizieren
  ICCkette* k = dynamic_cast<ICCkette*>(modell);
  if(k && k->size() > info)
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
          static int intent_alt;
          // ncl2 ?
          DBG_PROG_V( profile.aktuell() );
          int intent_neu = profile.profil()->intent();
          DBG_PROG_V( intent_neu <<" "<< intent_alt )
          if(intent_alt != intent_neu) {
            farbraum ();
            intent_alt = intent_neu;
          } else
            farbraum (info);

          if (info < (int)icc_betrachter->DD_farbraum->dreiecks_netze.size())
            icc_betrachter->DD_farbraum->dreiecks_netze[info].aktiv = true;

        } else if (info < (int)icc_betrachter->DD_farbraum->dreiecks_netze.size()) {
          icc_betrachter->DD_farbraum->dreiecks_netze[info].aktiv = false;
        }
          // Oberflaechenpflege - Aktualisieren
        if(icc_betrachter->DD_farbraum->visible())
        {
          icc_betrachter->DD_farbraum->flush();
        }
        if(icc_betrachter->menueintrag_inspekt->active() &&
           profile[info]->hasMeasurement() )
          setzMesswerte();
        else if(icc_betrachter->examin->visible())
          waehleTag(_item);
      }
    }
  }
  Beobachter::nachricht(modell, info);
  Fl::unlock();
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

  #if HAVE_X || HAVE_OSX
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
  #endif

  // TODO: osX
  DBG_PROG_ENDE
}

void
ICCexamin::vcgtZeigen ()
{ DBG_PROG_START
  frei_ = false;
  kurve_umkehren[VCGT_VIEWER] = true;

  #if HAVE_X// || HAVE_OSX
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
  #endif

  frei_ = true;
  // TODO: osX
  DBG_PROG_ENDE
}

void
ICCexamin::moniSetzen ()
{ DBG_PROG_START
  frei_ = false;
  if( profile.size() ) { DBG_PROG
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

  #if HAVE_X
  system("xgamma -gamma 1.0");
  vcgtZeigen();
  #endif

  // TODO: osX
  frei_ = true;
  DBG_PROG_ENDE
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
  #define widZEIG(zeigen,widget,dbg) { \
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
      icc_waehler_->hide();
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

  #define SichtbarkeitsWechsel(widget) \
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

  if(0.0 < f && f <= 1.0)
    icc_betrachter->load_progress-> value(f);
  else if (1.0 < f)
    icc_betrachter->load_progress-> hide();
  else
    icc_betrachter->load_progress-> show();

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
  if( e == FL_SHORTCUT )
  {
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
  }
  icc_examin->icc_betrachter->DD_farbraum->tastatur(e);
  //DBG_PROG_ENDE
  return gefunden;
}


