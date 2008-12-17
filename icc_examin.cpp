/*
 * ICC Examin ist eine ICC Profil Betrachter
 * 
 * Copyright (C) 2004-2008  Kai-Uwe Behrmann 
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
 * the central class.
 * 
 */


#include "config.h"
#include "icc_utils.h"
#include "icc_betrachter.h"
#include "icc_draw.h"
#include "icc_examin.h"
#include "icc_examin_io.h"
#include "icc_gl.h"
#include "icc_helfer.h"
#include "icc_helfer_ui.h"
#include "icc_draw_fltk.h"
#include "icc_helfer_fltk.h"
#include "icc_fenster.h"
#include "icc_info.h"
#include "icc_kette.h"
#include "icc_waehler.h"
#include "fl_i18n/fl_i18n.H"
#include "Flmm/Flmm_Message.H"

#if APPLE
#include <Carbon/Carbon.h>
#endif

#include <limits.h>
#include <float.h>
#include <unistd.h>
#include <FL/x.H>

using namespace icc_examin_ns;

#if USE_THREADS
#include "threads.h"
#include "icc_threads.h"
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

ICCexamin * icc_examin = 0;

int level_Prog = 0;


int iccMessageFunc( int code, const oyStruct_s * context, const char * format, ... )
{
  char* text = 0, *pos = 0;
  va_list list;
  const char * type_name = "";
  int id = -1, i;
  int pid = 0;
  FILE * fp = 0;

  if(code == oyMSG_DBG && !oy_debug)
    return 0;


  if(context && oyOBJECT_NONE < context->type_)
  {
    type_name = oyStruct_TypeToText( context );
    id = oyObject_GetId( context->oy_ );
  }

  text = (char*)calloc(sizeof(char), 4096);
  text[0] = 0;

  if(format && strlen(format) > 6)
  {
    if(strncasecmp("Start:", format, 6 ) == 0)
      ++level_Prog;
    if(strncasecmp("  End:", format, 6 ) == 0)
      --level_Prog;
  }

# define MAX_LEVEL 20
  if(level_Prog < 0)
    level_Prog = 0;
  if(level_Prog > MAX_LEVEL)
    level_Prog = MAX_LEVEL;
  for (i = 0; i < level_Prog; i++)
    sprintf( &text[strlen(text)], " ");


  switch(code)
  {
    case oyMSG_WARN:
         sprintf( &text[strlen(text)], _("WARNING"));
         break;
    case oyMSG_ERROR:
         sprintf( &text[strlen(text)], _("!!! ERROR"));
         break;
    case oyMSG_DBG:
         break;
  }
  sprintf( &text[strlen(text)], dbgThreadId( iccThreadSelf()).c_str());

  snprintf( &text[strlen(text)], 4096 - strlen(text), " %03f %s[%d] ", 
                                                      DBG_UHR_, type_name,id );

  va_start( list, format);
  vsnprintf( &text[strlen(text)], 4096 - strlen(text), format, list);
  va_end  ( list );


  pos = &text[strlen(text)];
  *pos = '\n';
  pos++;
  *pos = 0;

  icc_examin_ns::log( text, code );
  /* for debugging it is better to see messages on the console rather than
     getting lost during a crash */
  cout << code <<" "<< text;

# define TMP_FILE "/tmp/icc_examin_gdb_temp.txt"
  pid = (int)getpid();
  fp = fopen( TMP_FILE, "w" );

  if(fp)
  {
    fprintf(fp, "attach %d\n", pid);
    fprintf(fp, "thread 1\nbacktrace\nthread 2\nbacktrace\nthread 3\nbacktrace\ndetach" );
    fclose(fp);
    if(code != oyMSG_DBG && code != ICC_MSG_DBG)
    {
      cout << "GDB output:" << endl;
      system("gdb -batch -x " TMP_FILE);
    }
  } else
    cout << "could not open " << TMP_FILE << endl;

  if(text) free( text );

  return 0;
}
/* just a wrapper */
int lcmsMessageFunc( int code, const char * txt )
{
  iccMessageFunc( code, 0, txt );
  return 0;
}


ICCexamin::ICCexamin ()
{ DBG_PROG_START
  icc_examin_ns::lock(__FILE__,__LINE__);

  // set a nice GUI surface
#ifdef __APPLE__
  Fl::scheme("plastic"); // gtk+
#endif

  _item = -1;
  _mft_item = -1;
  for(int i = 0; i < 4; ++i)
    statlabel[i] = "";
  status_ = false;
  intent_ = 3;
  intent_selection_ = 0;
  farbraum_modus_ = 0;
  bpc_ = 0;
  gamutwarn_ = 0;
  nativeGamut_ = 0;
  vcgt_cb_laeuft_b_ = 0;

  Fl_Preferences vor( Fl_Preferences::USER, "oyranos.org", "iccexamin");
  Fl_Preferences gl_gamut(vor, "gl_gamut");
  gl_gamut.get("gamutwarn", gamutwarn_, 0 );
  gl_gamut.get("native_gamut", nativeGamut_, 0 );
  DBG_PROG_V( gamutwarn_ )

  oyThreadLockingSet( iccStruct_LockCreate, iccLockRelease, iccLock, iccUnLock);
  oyMessageFuncSet( iccMessageFunc );

  cmsErrorAction( LCMS_ERRC_WARNING );
  cmsSetErrorHandler( lcmsMessageFunc );

  alle_gl_fenster = new icc_examin_ns::EinModell;
  icc_betrachter = new ICCfltkBetrachter;
  io_ = new ICCexaminIO;
  profile.init();
  collect_changing_points = 0;
  options_ = 0;

  DBG_PROG_ENDE
}

void
ICCexamin::quit ()
{ DBG_PROG_START
  status_ = false;
  icc_examin_ns::sleep(0.5);

  delete icc_betrachter->DD;
  delete icc_betrachter->details;
  delete icc_betrachter;
  delete alle_gl_fenster;
  delete io_;

  Fl_Preferences vor( Fl_Preferences::USER, "oyranos.org", "iccexamin");
  Fl_Preferences gl_gamut(vor, "gl_gamut");
  gl_gamut.set("gamutwarn", gamutwarn_ );
  gl_gamut.set("native_gamut", nativeGamut_ );
  gl_gamut.flush();
  DBG_PROG_V( gamutwarn_ )
  oyOptions_Release( &options_ );

  DBG_PROG_ENDE
  exit(0);
}

void
ICCexamin::clear ()
{ DBG_PROG_START

  icc_betrachter->DD_farbraum->namedColoursRelease();
  icc_betrachter->DD_farbraum->frei(false);
  profile.clear();
  icc_betrachter->DD_farbraum->frei(true);
  icc_betrachter->DD_farbraum->dreiecks_netze.frei(false);
  icc_betrachter->DD_farbraum->dreiecks_netze.clear();
  icc_betrachter->DD_farbraum->dreiecks_netze.frei(true);

  icc_betrachter->DD_farbraum->clearNet ();

  DBG_PROG_ENDE
}


void
resize_fuer_menubar(Fl_Widget* w)
{
# if APPLE
  w->resize( w->x(), w->y()-25, w->w(), w->h()+25 );
# endif
}

# if HAVE_X
# include "icc_examin.xpm" // icc_examin_xpm
# endif

void oeffnen_cb(const char* filenames);

void
ICCexamin::start (int argc, char** argv)
{ DBG_PROG_START

  kurven.resize(MAX_VIEWER);
  punkte.resize(MAX_VIEWER);
  texte.resize(MAX_VIEWER);
  kurve_umkehren.resize(MAX_VIEWER);

  fl_translate_menue( icc_betrachter->menu_menueleiste );
  fl_translate_menue( icc_betrachter->menu_DD_menueleiste );

  icc_betrachter->init( argc, argv );

  icc_betrachter->mft_gl->init(1);
  icc_betrachter->DD_farbraum->init(2);
  {
    icc_waehler_ = new ICCwaehler(485, 116, _("Gamut selector"));
    if(!icc_waehler_) WARN_S( "icc_waehler_ not reservable" )
    // gehoert zum DD_farbraum fenster
    icc_waehler_->only_with = dynamic_cast<icc_examin_ns::MyFl_Double_Window*>(icc_betrachter->DD_farbraum->window());
  }

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

  // Oberflaechenpflege
# if HAVE_X || APPLE
  icc_betrachter->menueintrag_vcgt->show();
#   if APPLE
    icc_betrachter->vcgt_set_button->deactivate();
    icc_betrachter->vcgt_reset_button->deactivate();
#   endif
# if HAVE_X
  MyFl_Double_Window::icon = icc_examin_xpm;
  setzeIcon( icc_betrachter->details, icc_examin_xpm );
# endif

  DBG_PROG_S( "Show vcgt" )
# else
  DBG_PROG_S( "Show vcgt not" )
# endif

#if !defined(WIN32)
  FILE *out = icc_popen_m("oyranos-config", "r");
  if(out)
  {  
    char name[64] = {0};
    size_t r = fscanf( out, "%12s", name ); r=r;
    if( strcmp(name, "oyranos") == 0 )
      icc_betrachter->menu_einstellungen->show();
    icc_pclose_m(out);
  } 
#endif

# if APPLE
  // osX Resourcen
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

  // osX DnD behandeln
  fl_open_callback( oeffnen_cb );

# endif // APPLE

  if(!icc_debug)
  {
    icc_betrachter->menueintrag_testkurven->hide();
    icc_betrachter->menueintrag_lang->hide();
  }

  DBG_PROG

  modellDazu( /*ICCkette*/&profile ); // wird in nachricht ausgewertet
  modellDazu( /*GL_Ansicht*/icc_betrachter->DD_farbraum);

  Fl::add_handler(tastatur);

  // Behandle Kommandozeilenargumente
      if (argc>1) {
        ICClist<std::string>profilnamen;
        for (int i = 1; i < argc; i++) {
          DBG_PROG_S( i <<" "<< argv[i] )
          // keine process serial number in osX
          if(std::string(argv[i]).find("-psn_") == std::string::npos)
          {
            if(i == 1)
              status( argv[1] << " " << _("loaded") )
            profilnamen.push_back( argv[i] );
          }
        }
        oeffnen (profilnamen);
      } else {
        status(_("Ready"))
      }


  // zur Benutzung freigeben
  status_ = 1;
  frei(true);

  // receive events
# if 0
  Fl_Widget* w = dynamic_cast<Fl_Widget*>(icc_betrachter->details);
  if (w) {
      Fl::pushed(w);
      DBG_PROG_S( "pushed("<< w <<") "<< Fl::pushed() )
  }
# endif

# if USE_THREADS
  int fehler = fl_create_thread( getThreadId(THREAD_LADEN), &ICCexaminIO::oeffnenStatisch_, (void *)this );
# if HAVE_PTHREAD_H
# ifdef CWDEBUG
  Debug(myproject::debug::init_thread());
# endif
# endif
  if( fehler == EAGAIN)
  {
    WARN_S( "observer thread not started. Error: "  << fehler );
  } else
# if !APPLE && !WIN32 && PTHREAD_THREADS_MAX
  if( fehler == (int)PTHREAD_THREADS_MAX )
  {
    WARN_S( "Too many observer threads. Error: " << fehler );
  } else
# endif
  if( fehler != 0 )
  {
    WARN_S( "unknown Error at start of a observer thread. Fehler: " << fehler );
  }
# else
  Fl::add_timeout( 0.01, /*(void(*)(void*))*/ICCexaminIO::oeffnenStatisch_ ,(void*)this);
# endif

  if(!dateiwahl)
  {
    my_fl_translate_file_chooser();

    const char* ptr = NULL;
    if (profile.size())
      ptr = profile.name().c_str();
    dateiwahl = new MyFl_File_Chooser(ptr, _("ICC colour profiles (*.{I,i}{C,c}{M,m,C,c})	Measurement (*.{txt,it8,IT8,RGB,CMYK,ti*,cgats,CIE,cie,nCIE,oRPT,DLY,LAB,Q60})	Argyll Gamuts (*.{wrl,vrml}"), MyFl_File_Chooser::MULTI, _("Which ICC profile?"));
    dateiwahl->callback(dateiwahl_cb);
    dateiwahl->preview(true);
    icc_examin_ns::MyFl_Double_Window *w = dateiwahl->window;
    w->use_escape_hide = true;
  }

  icc_betrachter->run( (int)argc, (char**)argv );

  DBG_PROG_ENDE
}

void
ICCexamin::zeig3D ()
{ DBG_PROG_START

  MyFl_Double_Window *w = icc_betrachter->DD;
  GL_Ansicht *wid = icc_betrachter->DD_farbraum;

  if(!icc_waehler_->visible())
    icc_waehler_->show();

  w->show();
  wid->show();
  wid->set_visible(); // needed for osX Tiger (FLTK bug?)
  wid->damage(FL_DAMAGE_ALL);
  wid->invalidate();
  wid->redraw();
  w->show(); // needed for osX

  //icc_waehler_->position( w->x(), w->y() - icc_waehler_->h() );

  setzeFensterTitel();

  DBG_PROG_ENDE
}

void
ICCexamin::zeigPrueftabelle ()
{ DBG_PROG_START

  MyFl_Double_Window *details = icc_betrachter->details;
  Fl_Help_View *inspekt_html = icc_betrachter->inspekt_html;

  Fl_Widget *wid = inspekt_html;

  if(wid->window() == details)
  {
    int lx = details->x(),
        ly = details->y()+10,
        lw = details->w(),
        lh = details->h();

    MyFl_Double_Window *w =
      new MyFl_Double_Window( lx+lw, ly, lw, lh, "" );
      w->user_data((void*)(0));
        Fl_Group *g = new Fl_Group(0,0,lw,lh);
        g->end();
        wid->resize(0,0, lw,lh);
        g->add( wid );
      w->end();
      w->resizable(w);
      //w->resizable(g);
      w->show();
      w->position(w->x(), ly-10);
  }

  wid->window()->show();
  wid->show();

  setzMesswerte();

  DBG_PROG_ENDE
}

void
ICCexamin::zeigCGATS()
{
  DBG_PROG_START
  icc_examin_ns::lock(__FILE__,__LINE__);
  // represent CGATS in window
  icc_examin_ns::nachricht(profile.profil()->cgats_max());
  icc_examin_ns::unlock(0, __FILE__,__LINE__);
  DBG_PROG_ENDE
}

void
ICCexamin::zeigMftTabellen ()
{ DBG_PROG_START

  const char* title = profile.profil()->filename();
  char* t = (char*) malloc(strlen(title)+20);
  int   item = tag_nr();
  ICClist<std::string> tag_info =
       profile.profil()->printTagInfo(item);

  sprintf(t, "%d:%s - %s", item + 1,
          tag_info[0].c_str(),
          title);

  int lx = icc_betrachter->details->x(),
      ly = icc_betrachter->details->y()+10,
      lw = icc_betrachter->mft_gl->w(),
      lh = icc_betrachter->mft_gl->h();

#if defined(__APPLE__) && 0
  int X,Y,bt,bx,by;
  Fl_X::fake_X_wm(icc_betrachter->details, &X, &Y, &bt, &bx, &by);
#endif

  ICClist<std::string> out_names =
      profile.profil()->getTagChannelNames (icc_betrachter->tag_nummer,
                                            ICCtag::TABLE_OUT);
  MyFl_Double_Window *w = NULL;
  for(int i = 0; i < (int)out_names.size(); ++i)
  {
    if(!w)
    w = new MyFl_Double_Window( lx+icc_betrachter->details->w(), ly, lw, lh, t);
    else
    w = new MyFl_Double_Window( w->x()+lw, ly, lw, lh, t);

      w->user_data((void*)(0));
      Fl_Group *g = new Fl_Group(0,0,lw,lh);
        GL_Ansicht *gl = 
          new GL_Ansicht (*icc_betrachter->mft_gl); //(0,0,lw,lh);

        GL_Ansicht::getAgv(gl, icc_betrachter->mft_gl);
        gl->init( icc_betrachter->mft_gl->id() );
        gl->copy( *icc_betrachter->mft_gl );
        gl->kanal = i;
      g->end();
    w->end();
    w->resizable(w);
    g->show();
    w->show();
    w->position( w->x(), ly-10 );
    DBG_PROG_V( icc_betrachter->details->y()<<" "<<w->y() )
    gl->show();
    gl->invalidate();
    gl->damage(FL_DAMAGE_ALL);
  }

  waehleMft(0);

  DBG_PROG_ENDE
}

/** virtual from icc_examin_ns::Beobachter:: */
void
ICCexamin::nachricht( Modell* modell , int info )
{
  DBG_PROG_START

  if(!frei()) {
    WARN_S("icc_examin is not free")
    //DBG_PROG_ENDE
    //return;
  }

  DBG_THREAD_V( info )
  // identify Modell
  ICCkette* k = dynamic_cast<ICCkette*>(modell);
  if(k && (k->size() > info))
  {
    DBG_PROG_S( "news from ICCkette" )
    DBG_PROG_S( "refresh of profile Nr.: " << info )
    if(info>=0)
    {
      DBG_PROG_V( (int*)(*k)[info] )
      if ((*k)[info])
      if((*k)[info]->changing()) {
        DBG_PROG_S( "just changing: " << info )
        //icc_examin_ns::sleep( 0.1 );
      }
      DBG_PROG_S( "load: " << info )

      { DBG_PROG
          // handle user interface - actualise
        fortschritt(0.6 , 1.0);
        if(profile[info]->tagCount() <= _item)
          _item = (-1);
        DBG_PROG_V( _item )
 
        if((*k)[info]->size() > 128)
          icc_examin->waehlbar( info, true );
        else {
          icc_examin->waehlbar( info, false );
          icc_betrachter->DD_farbraum->dreiecks_netze.frei(false);
          if(icc_betrachter->DD_farbraum->dreiecks_netze.size() > (size_t)info)
            icc_betrachter->DD_farbraum->dreiecks_netze[info].aktiv = false;
          icc_betrachter->DD_farbraum->dreiecks_netze.frei(true);
        }
      }

      if(profile.profil() == profile[info])
      {
        icc_examin_ns::lock(__FILE__,__LINE__);

        if(icc_betrachter->inspekt_html->visible_r())
          setzMesswerte();

        fortschritt(0.7 , 1.0);
        if(icc_betrachter->examin->visible())
          waehleTag(_item);

        fortschritt(0.9 , 1.0);
        if(icc_betrachter->DD_farbraum->visible())
          icc_betrachter->DD_farbraum->damage(FL_DAMAGE_ALL);

        setzeFensterTitel();

        icc_examin_ns::unlock(icc_betrachter->DD_farbraum, __FILE__,__LINE__);
      }
    }
    fortschritt(1.0 , 1.0);
    fortschritt(1.1 , 1.0);
    Beobachter::nachricht(modell, info);
  }


  GL_Ansicht* gl = dynamic_cast<GL_Ansicht*>(modell);
  if(gl && info == GL_MOUSE_HIT3D)
  {
    // find a CGATS/ncl2 tag_text / inspect_html line from a 3D(Lab) mouse hit
    // it's the inverse from selectTextsLine(int * line)
    int item = icc_examin->tag_nr();
    profile.frei(false);
    if(!profile.profil())
    {
      profile.frei(true);
      return;
    }
    ICClist<std::string> TagInfo = profile.profil()->printTagInfo(item),
                             names;
    ICClist<double> chan_dv;
    ICClist<double> lab_dv;
    double min = DBL_MAX, len;
    double cielab1[3], cielab2[3];
    double chan[32];
    int min_pos = -1,
        n;
#if 0
    if( icc_betrachter->inspekt_html->visible() ||
        (profile.profil()->tagBelongsToMeasurement(item) &&
         icc_betrachter->tag_browser->value() > 5)
      )
#endif
    if(profile.profil()->hasMeasurement())
    {
      ICCmeasurement & m = profile.profil()->getMeasurement();
      oyNamedColour_GetColourStd ( gl->mouse_3D_hit, oyEDITING_LAB, cielab1,
                                   oyDOUBLE, 0 );

      DBG_PROG_S( cielab1[0] <<" "<< cielab1[1] <<" "<< cielab1[2] )

      n = m.getPatchCount();

      ICClist<Lab_s> lab_v;
      lab_v = m.getMessLab();
      if(!lab_v.size())
        lab_v = m.getProfileLab();

      n =  lab_v.size();
      for(int i = 0; i < n; ++i)
      {
        LabToCIELab( lab_v[i], cielab2 );
        len = fabs( dE( cielab1, cielab2 ) );
        
        if(len < min)
        {
          min = len;
          min_pos = i;
        }
      }

      if(min < 5)
      {
        std::string name = m.getFieldName(min_pos);
        ICClist<int> pl;

        if(icc_betrachter->inspekt_html->visible())
          icc_betrachter->inspekt_html->topline( name.c_str() );

        if(profile.profil()->tagBelongsToMeasurement(item))
        {
          pl = m.getPatchLines ( TagInfo[0].c_str() );
          if (pl.size())
            icc_betrachter->tag_text->select(pl[min_pos]+1);
        }
        DBG_PROG_V(min <<" "<< min_pos)

        oyNamedColour_s * colour = 0;

        if(pl.size() && (signed)pl.size() > min_pos)
          colour = m.getPatchLine( pl[min_pos], TagInfo[0].c_str() );
        else
          colour = m.getMessColour( min_pos );

        icc_betrachter->DD_farbraum->emphasizePoint( colour );
        oyNamedColour_Release( &colour );
      }

    } else if( profile.profil()->hasTagName("ncl2") ) {

      farbenLese( profile.aktuell(), lab_dv, chan_dv, names );
      n = names.size();
      int mult = 0;
      int n_ = 0;

      if(n)
        mult = lab_dv.size()/3/names.size();
      n_ = n*3*mult;

      //double lab[3];
      oyNamedColour_GetColourStd ( gl->mouse_3D_hit, oyEDITING_LAB, cielab1,
                                   oyDOUBLE, 0);
      //LabToCIELab( lab, cielab1, 1 );

      for(int i = 0; i < n_; i+=3*mult)
      {
        LabToCIELab( &lab_dv[i], cielab2, 1 );
        len = fabs( dE( cielab1, cielab2 ) );
        if(len < min)
        {
          min = len;
          min_pos = i/3/mult;
        }
      }

      if(min < 5)
      {
        if(icc_betrachter->inspekt_html->visible())
        {
          icc_betrachter->inspekt_html->topline( names[min_pos].c_str() );
        }
        if(TagInfo.size())
        if(TagInfo[0] == "ncl2")
          icc_betrachter->tag_text->select(min_pos+6);

        DBG_PROG_V(min <<" "<< min_pos)
        lab_dv[0] = lab_dv[min_pos*3*mult+0];
        lab_dv[1] = lab_dv[min_pos*3*mult+1];
        lab_dv[2] = lab_dv[min_pos*3*mult+2];
        lab_dv.resize(3);
        LabToCIELab( &lab_dv[0], cielab1, 1 );
        double XYZ[3];
        oyLab2XYZ( cielab1, XYZ );
        memset(chan, 0, sizeof(double)*32);

        oyProfile_s * prof = oyProfile_FromFile
                                      ( profile.profil()->filename(), 0,NULL );
        if(!prof)
          prof = oyProfile_FromStd( oyASSUMED_WEB, NULL );
        int channels_n = oyProfile_GetChannelsCount( prof );

        for(int k = 0; k < channels_n; ++k)
          chan[k] = chan_dv[min_pos*channels_n*mult+k];

        const char * name = names[min_pos].c_str();

        oyNamedColour_s * colour = 
          oyNamedColour_CreateWithName( name, NULL, NULL,
                                        chan, XYZ, NULL,0, prof, 0 );
        oyProfile_Release( &prof );
        if(!names[min_pos].size())
          DBG_PROG_S( "no name found" );
        icc_betrachter->DD_farbraum->emphasizePoint( colour );
        oyNamedColour_Release( &colour );
      }
    }
    profile.frei(true);
  }

  DBG_PROG_ENDE
}

void
ICCexamin::setzeFensterTitel()
{
  DBG_PROG_START
  char* t = (char*) malloc(256);
  const char* title = NULL;
  unsigned int t_len = 256;

#ifdef WIN32
  t_len = 30;
#endif

  if(profile.profil())
    title = dateiName(profile.profil()->filename());


  Fl_Window * window = icc_betrachter->inspekt_html->window();
  if(window != icc_betrachter->details &&
     window->shown() )
  {
    if(title && profile.profil()->hasMeasurement())
      snprintf(t, 256, "ICC Examin: %s - %s", title,
              _("Compare Measurement <-> Profile Colours"));
    else
      snprintf(t, 256, "ICC Examin: %s - %s", _("none"),
              _("Compare Measurement <-> Profile Colours"));

    icc_examin_ns::lock(__FILE__,__LINE__);
	// TODO: ???
#ifndef WIN32
    window->label(t);
#endif
    icc_examin_ns::unlock(window, __FILE__,__LINE__);
  }

  window = icc_betrachter->DD_farbraum->window();
  if(window != icc_betrachter->details &&
     window->shown() )
  {
    if(title)
	{
      snprintf(t, t_len, "ICC Examin: %s - %s", title,
              _("Gamut View"));
	  if(strlen(title) > t_len)
	    sprintf( &t[t_len], "..." );
    } else
      snprintf(t, t_len, "ICC Examin: - %s", _("Gamut View"));

    icc_examin_ns::lock(__FILE__,__LINE__);
	// TODO: ???
#ifndef WIN32
    window->label(t);
#endif
    icc_examin_ns::unlock(window, __FILE__,__LINE__);
  }

  window = icc_betrachter->details;
  if(window->shown() )
  {
    if(title)
	{
      snprintf(t, t_len, "ICC Examin: %s", title);
	  if(strlen(title) > t_len)
	    sprintf( &t[t_len], "..." );
	} else
      snprintf(t, t_len, "ICC Examin: -");

    icc_examin_ns::lock(__FILE__,__LINE__);
	// TODO: ???
#ifndef WIN32
    window->label(t);
#endif
    icc_examin_ns::unlock(window, __FILE__,__LINE__);
  }

  if(t) free(t);
  DBG_PROG_ENDE
}

void ICCexamin::optionsRefresh_( void )
{
  oyOptions_Release( &options_ );
    
# if OYRANOS_VERSION >= 109
  options_ = oyOptions_ForFilter( "//colour", 0, 0, 0 );
  char t[4];
  /* should always be a single digit */
  sprintf( t, "%d", intentGet(NULL));
  oyOptions_SetFromText( options_, "rendering_intent", t );
  sprintf( t, "%d", bpc() );
  oyOptions_SetFromText( options_, "rendering_bpc", t );
  sprintf( t, "%d", gamutwarn() );
  oyOptions_SetFromText( options_, "rendering_gamut_warning", t );
# endif
}

void
ICCexamin::setzMesswerte()
{
  DBG_PROG_START
  bool export_html = false;

  if( icc_betrachter->inspekt_html->window()->shown() )
  {
    if(profile.profil()->hasMeasurement())
    {
      icc_examin_ns::lock(__FILE__,__LINE__);
      int topline = icc_betrachter->tag_text->inspekt_topline = icc_betrachter->inspekt_html->topline();
      oyOptions_s * opts = options();
      icc_betrachter->inspekt_html->value(profile.profil()->report( export_html,
                                                                opts ).c_str());
      oyOptions_Release( &opts );
      icc_betrachter->inspekt_html->topline( topline );
      icc_examin_ns::unlock(icc_betrachter->inspekt_html, __FILE__,__LINE__);

    } else
      icc_betrachter->inspekt_html->value(_("not available"));

    setzeFensterTitel();
  }

  DBG_PROG_ENDE
}


void
ICCexamin::testZeigen ()
{ DBG_PROG_START

# if HAVE_X || APPLE
  ICClist<ICClist<std::pair<double,double> > > kurven2;
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
  ICClist<std::string> txt;
  txt.resize(8);
  txt[0] = "a image";
  txt[1] = "paint";
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
beobachte_vcgt(void *ICCexamina)
{
  ICCexamin *ie = (ICCexamin*) ICCexamina;

  ie->vcgtZeigen();

  if (ie->icc_betrachter->vcgt->visible())
    Fl::add_timeout( 0.33, (void(*)(void*))beobachte_vcgt ,(void*)ie);
  else
    ie->vcgtStoppen();
}  

void
ICCexamin::vcgtStoppen ()
{ DBG_PROG_START
  vcgt_cb_laeuft_b_ = false;
  icc_betrachter->vcgt->hide();
  DBG_PROG_ENDE
}

void
ICCexamin::vcgtZeigen ()
{ DBG_PROG_START
  kurve_umkehren[VCGT_VIEWER] = true;

  if (!vcgt_cb_laeuft_b_) {
    vcgt_cb_laeuft_b_ = true;
    beobachte_vcgt( (void*)this );
  }

  frei(false);
# if HAVE_X || APPLE
  std::string display_name = "";
  int x = icc_betrachter->vcgt->x() + icc_betrachter->vcgt->w()/2;
  int y = icc_betrachter->vcgt->y() + icc_betrachter->vcgt->h()/2;
  kurven[VCGT_VIEWER] = leseGrafikKartenGamma (display_name,texte[VCGT_VIEWER], x,y);
  if (kurven[VCGT_VIEWER].size()) {
    icc_betrachter->vcgt_viewer->hide();
    icc_betrachter->vcgt_viewer->show();
    icc_betrachter->vcgt_viewer->hineinKurven( kurven[VCGT_VIEWER],
                                               texte [VCGT_VIEWER] );
    icc_betrachter->vcgt_viewer->kurve_umkehren = true;
  } else {
    ICClist<ICClist<double> > leer;
    icc_betrachter->vcgt_viewer->hide();
    icc_betrachter->vcgt_viewer->show();
    leer.resize(3);
    icc_betrachter->vcgt_viewer->hineinKurven( leer,
                                               texte [VCGT_VIEWER] );
    icc_betrachter->vcgt_viewer->kurve_umkehren = true;
  }
# endif

  frei(true);
  // TODO: osX
  DBG_PROG_ENDE
}

bool
ICCexamin::berichtSpeichern (void)
{ return io_->berichtSpeichern();
}
bool
ICCexamin::gamutSpeichern (icc_examin_ns::IccGamutFormat format)
{ return io_->gamutSpeichern(format);
}
void
ICCexamin::oeffnen ()
{ io_->oeffnen();
}
void
ICCexamin::oeffnen (ICClist<std::string> dateinamen)
{ io_->oeffnen( dateinamen );
}
bool
ICCexamin::lade ()
{ return io_->lade();
}
void
ICCexamin::lade (ICClist<Speicher> & neu)
{ io_->lade(neu);
}
int
ICCexamin::erneuern()
{
  frei(false);
  int i = io_->erneuern();
  frei(true);
  return i;
}
void
ICCexamin::erneuern(int pos)
{
  frei(false);
  io_->erneuern(pos);
  frei(true);
}

/** Auffrischen des Programmes (Neuladen) */
void
ICCexamin::auffrischen(int schalter)
{
  if(schalter & OYRANOS)
    icc_oyranos.clear();
  if(schalter & PROGRAMM)
  {
    ICClist<std::string> profilnamen = profile;
    oeffnen( profilnamen );
  }
}
void
ICCexamin::oyranos_einstellungen()
{
  int r = system("oyranos-config-fltk"); r = r;
  auffrischen( PROGRAMM | OYRANOS );
}

void
ICCexamin::moniHolen ()
{ DBG_PROG_START
  //frei(false);
  fortschritt( 0.01 , 1.0 );
  int x = icc_betrachter->vcgt->x() + icc_betrachter->vcgt->w()/2;
  int y = icc_betrachter->vcgt->y() + icc_betrachter->vcgt->h()/2;

  static ICClist<Speicher> ss;
  ss.clear();
  ss.push_back(icc_oyranos.moni(x,y));
  size_t size = ss[0].size();
  const char *moni_profil = ss[0];
  if(!moni_profil || !size) {
    //frei(true);
    DBG_PROG_ENDE
    return;
  }

  int erfolg = false;
  while(!erfolg) {
    if(!lade()) {
      lade(ss);
      erfolg = true;
    } else {
      // kurze Pause 
      DBG_THREAD_S( "musst wait" )
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

  fortschritt( 1.1 , 1.0 );
  DBG_PROG_ENDE
}

std::string
ICCexamin::moniName ()
{ DBG_PROG_START
  //frei(false);
  int x = icc_betrachter->vcgt->x() + icc_betrachter->vcgt->w()/2;
  int y = icc_betrachter->vcgt->y() + icc_betrachter->vcgt->h()/2;

  DBG_PROG_ENDE
  return icc_oyranos.moni_name( x,y );
}

void
ICCexamin::moniSetzen ()
{ DBG_PROG_START
  frei(false);
  int x = icc_betrachter->vcgt->x() + icc_betrachter->vcgt->w()/2;
  int y = icc_betrachter->vcgt->y() + icc_betrachter->vcgt->h()/2;

  if( profile.size() && profile.profil()->filename() &&
      strlen( profile.profil()->filename() ) ) { DBG_PROG
    icc_oyranos.setzeMonitorProfil( profile.profil()->filename(), x,y );
    frei(true);
    vcgtZeigen();
    frei(false);
  }
  frei(true);
  DBG_PROG_ENDE
}

void
ICCexamin::standardGamma ()
{ DBG_PROG_START
  frei(false);
# if HAVE_X
  int x = icc_betrachter->vcgt->x() + icc_betrachter->vcgt->w()/2;
  int y = icc_betrachter->vcgt->y() + icc_betrachter->vcgt->h()/2;

  frei(true);
  vcgtZeigen();
  frei(false);
  icc_oyranos.setzeMonitorProfil( 0, x,y );
# endif

  // TODO: osX
  frei(true);
  DBG_PROG_ENDE
}

void
ICCexamin::gamutAnsichtZeigen ()
{
      icc_examin_ns::lock(__FILE__,__LINE__);
      icc_betrachter->menueintrag_3D->set();
      icc_betrachter->menueintrag_huelle->set();
      icc_betrachter->widget_oben = ICCfltkBetrachter::WID_3D;
      farbraum_angezeigt_ = true;
      icc_betrachterNeuzeichnen(icc_betrachter->DD_farbraum);

#ifndef WIN32
      if(icc_betrachter->DD_farbraum->window() != icc_betrachter->details)
        icc_betrachter->details->iconize(icc_betrachter->details);
#endif

      icc_examin_ns::unlock(icc_betrachter->details, __FILE__,__LINE__);
      DBG_PROG_S("icc_betrachterNeuzeichnen DD_farbraum")
}

void
ICCexamin::intent( int intent_neu )
{
  int intent_alt = intent_;
  if(intent_neu < 0)
  {
    if(farbraumModus())
      intent_ = profile.profil()->intent();
    else
      intent_ = 3;

    intent_selection_ = 0;

  } else {
    intent_ = intent_neu;
    intent_selection_ = 1;
  }

  if(intent_alt != intent_neu) {
    auffrischen( PROGRAMM );
  }
}

void
ICCexamin::bpc( int bpc_neu )
{
  if(bpc_ != bpc_neu)
    auffrischen( PROGRAMM );
  bpc_ = bpc_neu;
}

void
ICCexamin::nativeGamut( int nativeGamut_neu )
{
  if(nativeGamut_ != nativeGamut_neu)
    auffrischen( PROGRAMM );
  nativeGamut_ = nativeGamut_neu;
}

void
ICCexamin::erneuerTagBrowserText_ (void)
{
  DBG_PROG_START
  //open and preparing the first selected item

  if(!icc_betrachter->details->visible())
    return;

  TagBrowser *b = icc_betrachter->tag_browser;

  std::stringstream s;
  std::string text;
  ICClist<std::string> tag_list = profile.profil()->printTags();
  DBG_PROG_V( tag_list.size() <<" "<< (int*) b )

# define add_s(stream) s << stream; b->add (s.str().c_str()); s.str("");
# define add_          s << " ";

  b->clear();
  const char *file_type_name = _("Filename (ICC data type)");
  DBG_PROG_V( profile.profil()->data_type )
  if(profile.profil()->data_type != ICCprofile::ICCprofileDATA)
    file_type_name = _("Filename (other data type)");
  if(profile.profil()->data_type == ICCprofile::ICCcorruptedprofileDATA)
    file_type_name = _("Filename (corrupted ICC data type)");
  add_s ("@f" << file_type_name << ":" )
  add_s ("@b    " << profile.profil()->filename() )
  add_s ("")
  if (tag_list.size() == 0) {
    add_s (_("found no content for") <<" \"" << profile.profil()->filename() << "\"")
    return;
  } else if ((int)tag_list.size() != profile.profil()->tagCount()*5 ) {
    add_s (_("Internal error") )
  }
  // this string is sensible to formatting in the tag browser GUI, please keep a mono spaced formatting
  add_s ("@B26@t" << _("No. Tag   Type   Size Description") )
  if(profile.profil()->data_type == ICCprofile::ICCprofileDATA ||
     profile.profil()->data_type == ICCprofile::ICCcorruptedprofileDATA) {
    add_s ("@t" << profile.profil()->printHeader() )
  } else {
    add_s("")
  }

  for(int i = 0; i < (int)tag_list.size(); ++i)
    ;//DBG_PROG_V( i <<" "<< tag_list[i] )
  int anzahl = 0;
  for (int j = 0; j < (int)tag_list.size(); ++j)
  {
    s << "@t";
    // Number
    int Nr = atoi( tag_list[j].c_str() ) + 1;
    std::stringstream t; t << Nr;
    for (int i = (int)t.str().size(); i < 3; i++) {s << " ";} s << Nr; j++; ++anzahl; s << " ";
    // Name/title
    s << tag_list[j]; for (int i = (int)(tag_list[j++]).size(); i < 6; i++) {s << " ";} ++anzahl;
    // Typ
    s << tag_list[j]; for (int i = (int)(tag_list[j++]).size(); i < 5; i++) {s << " ";} ++anzahl;
    // Size
    for (int i = (int)tag_list[j].size(); i < 6; i++) {s << " ";} s << tag_list[j++]; s << " "; ++anzahl;
    // description
    add_s (tag_list[j])
  }
  DBG_PROG_V( anzahl )
  if (b->value())
    b->selectItem (b->value()); // show
  else
    if(profile.profil()->data_type == ICCprofile::ICCprofileDATA ||
       profile.profil()->data_type == ICCprofile::ICCcorruptedprofileDATA) {
      b->selectItem (1);
    } else {
      b->selectItem (6);
    }

  if (profile.profil()->hasTagName (b->selectedTagName)) {
    int item = profile.profil()->getTagIDByName (b->selectedTagName) + 6;
    b->selectItem (item);
    b->value(item);
  }

  status ( dateiName( profile.profil()->filename() ) << " " << _("loaded")  )
  DBG_PROG_ENDE
}

void
ICCexamin::icc_betrachterNeuzeichnen (void* z)
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
  DBG_PROG_V( dynamic_cast<Fl_Widget*>(icc_betrachter->mft_gl_group)->visible() )
  DBG_PROG_V( dynamic_cast<Fl_Widget*>(icc_betrachter->mft_viewer)->visible() )

# if 0
  enum {ZEIGEN, VERSTECKEN, NACHRICHT, KEINEn};
# define widZEIG(zeigen,widget,dbg) { \
     if        (zeigen ==                  VERSTECKEN && widget->visible()) { \
         widget->                          hide(); \
         if(dbg==NACHRICHT) DBG_PROG_S( "verstecke " << #widget ); \
  \
     } else if (zeigen ==                  ZEIGEN     && !widget->visible()) { \
         widget->                          show(); \
         if(dbg==NACHRICHT) DBG_PROG_S( "zeige     " << #widget ); \
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

  // inhaltliches Zurueckschalten auf tiefere Ebene
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
      icc_betrachter->box_stat->color( fl_rgb_color( VG ) );
      icc_betrachter->box_stat->labelcolor(FL_BLACK);
      icc_betrachter->box_stat->redraw();
    }
    if(icc_waehler_->visible())
#   ifdef APPLE
      icc_waehler_->hide();
#   else
      icc_waehler_->iconize();
#   endif
  }

  if(icc_betrachter->inspekt_html->window() == icc_betrachter->details)
  {
    if(oben == INSPEKT_ZEIGEN)
      widZEIG(ZEIGEN, icc_betrachter->inspekt_html ,NACHRICHT)
    else
      widZEIG(VERSTECKEN, icc_betrachter->inspekt_html ,NACHRICHT)
  }

  if(oben == TAG_ZEIGEN) {
    widZEIG(ZEIGEN, icc_betrachter->examin ,NACHRICHT)
    widZEIG(ZEIGEN, icc_betrachter->tag_browser ,NACHRICHT)
    widZEIG(ZEIGEN, icc_betrachter->ansichtsgruppe ,NACHRICHT)
  } else {
    widZEIG(VERSTECKEN, icc_betrachter->examin ,NACHRICHT)
    widZEIG(VERSTECKEN, icc_betrachter->tag_browser ,NACHRICHT)
    widZEIG(VERSTECKEN, icc_betrachter->ansichtsgruppe ,NACHRICHT)
  }
# else

  if(wid == icc_betrachter->DD_farbraum) { 
    if(!icc_betrachter->DD_farbraum->visible_r()) {
      zeig3D();
    }
  }
  if(!icc_betrachter->DD_farbraum->visible_r()) {
      if(icc_waehler_ && icc_waehler_->visible())
#   ifdef APPLE
        icc_waehler_->hide();
#   else
        icc_waehler_->iconize();
#   endif
  }

# endif

  // clean up - here?
  if (wid == icc_betrachter->tag_viewer ||
      wid == icc_betrachter->mft_viewer) {
    wid->clear_visible(); DBG_PROG_V( item << _item )
  }

  // table companion
  if (wid == icc_betrachter->mft_text ||
      wid == icc_betrachter->mft_gl_group ||
      wid == icc_betrachter->mft_viewer)
  { icc_betrachter->mft_choice->show(); DBG_PROG_S( "mft_choice zeigen" ) 
  } else
    icc_betrachter->mft_choice->hide();

# define SichtbarkeitsWechsel(widget, oberst) \
  { \
    Fl_Widget *w = dynamic_cast<Fl_Widget*> (icc_betrachter->widget); \
    if (w != wid && w->visible()) \
    { DBG_PROG_S( #widget << " verstecken" ) \
      w->hide(); \
      Fl_Group *g = dynamic_cast<Fl_Group*> (w); \
      if( g ) \
        for(int i = 0; i < g->children(); ++i) \
          g->child(i)->hide(); \
      if(w->visible()) { \
        WARN_S( #widget << " ist noch sichbar" ); \
        w->hide(); \
      } \
    } else if(w == wid) \
    { DBG_PROG_S( #widget << " zeigen" ) \
      /* hide all other branches */ \
      for(int i = 0; i < wids_n; ++i) { \
        if(i != oberst) \
          wids[i]->hide(); \
      /* only all of our higher hierarchy branches are visible */ \
      if(!wids[oberst]->visible()) \
        wids[oberst]->show(); \
      } \
      if (!w->visible_r()) \
      { \
        w->show(); \
        Fl_Group *g = dynamic_cast<Fl_Group*> (w); \
        if( g ) \
          for(int i = 0; i < g->children(); ++i) \
            g->child(i)->show(); \
        item = _item; \
      } \
      if (!w->visible_r()) \
        w->window()->show(); \
    } \
  }

  int wids_n = 2;
  Fl_Widget *wids[2] = {icc_betrachter->tabellengruppe,
                        icc_betrachter->twoD_pack};

  SichtbarkeitsWechsel(mft_viewer, 0)
  SichtbarkeitsWechsel(mft_gl_group, 0)
  SichtbarkeitsWechsel(mft_text, 0)
  SichtbarkeitsWechsel(tag_viewer, 1)
  SichtbarkeitsWechsel(tag_text, 1)

# if 0
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
  DBG_PROG_V( dynamic_cast<Fl_Widget*>(icc_betrachter->mft_gl_group)->visible() )
  DBG_PROG_V( dynamic_cast<Fl_Widget*>(icc_betrachter->mft_viewer)->visible() )

  // Inhalte Erneuern
  if(waehle_tag) {
    //icc_examin_ns::unlock(this, __FILE__,__LINE__);
    waehleTag(_item);
    //icc_examin_ns::lock(__FILE__,__LINE__);
  }
# endif

  DBG_PROG_ENDE
}

void
ICCexamin::waehlbar( int pos, int wert )
{ DBG_MEM_START
  icc_waehler_->waehlbar(pos,wert);
  DBG_MEM_ENDE
}


/**  0...1 for the visible actual value,
    -1...0 for invisible progress bar */
double
ICCexamin::fortschritt()
{
  if(icc_betrachter->load_progress->visible())
    return icc_betrachter->load_progress-> value();
  else
    return icc_betrachter->load_progress-> value() * -1;
}

//! Progress bar: f<0-Start f=Wert f>1-Ende  a>=1 komplett a=0.1 10%
void
ICCexamin::fortschritt(double f, double anteil)
{ DBG_PROG_START
#ifndef __APPLE__ /* FLTK has problems to events from different threads on osX*/
  int thread = wandelThreadId(iccThreadSelf());
  if(thread != THREAD_HAUPT)
    icc_examin_ns::lock(__FILE__,__LINE__);

    if(0.0 < f && f <= 1.0) {
      if(!icc_betrachter->load_progress->visible() &&
         anteil > 0.0 )
        icc_betrachter->load_progress-> show();
      if(fabs(anteil) >= 1.0)
        icc_betrachter->load_progress-> value( (float)f );
      else
        icc_betrachter->load_progress-> value( (float)(1.0 -
                    icc_betrachter->load_progress->value() / fabs(anteil) * f) );
      DBG_PROG_V( f )
    } else if (1.0 < f &&
               anteil > 0.0) {
      icc_betrachter->load_progress-> hide();
      DBG_PROG_V( f )
    } else if(anteil > 0.0) {
      icc_betrachter->load_progress-> show();
      DBG_PROG_V( f )
    }
    icc_betrachter->load_progress-> damage(FL_DAMAGE_ALL);
  
  if(thread != THREAD_HAUPT)
    icc_examin_ns::unlock(icc_betrachter->load_progress, __FILE__,__LINE__);
#endif
  DBG_PROG_ENDE
}

void 
ICCexamin::gamutwarn (int warn)
{ DBG_PROG_START
  gamutwarn_ = warn;
  erneuern( profile );
  DBG_PROG_ENDE
}

void
ICCexamin::statusFarbe(double & L, double & a, double & b)
{ DBG_PROG_START
  double lab[3] = {L, a, b},
         *rgb = 0;
  DBG_PROG_V( lab[0]<<" "<<lab[1]<<" "<<lab[2] )
  rgb = icc_oyranos. wandelLabNachBildschirmFarben(
                                icc_betrachter->DD_box_stat->window()->x() + icc_betrachter->DD_box_stat->window()->w()/2,
                                icc_betrachter->DD_box_stat->window()->y() + icc_betrachter->DD_box_stat->window()->h()/2,
                                lab, 1,
                                icc_examin->intentGet(NULL),
                                (icc_examin->gamutwarn()?cmsFLAGS_GAMUTCHECK:0)|
                                (icc_examin->bpc()?cmsFLAGS_BLACKPOINTCOMPENSATION:0));
  Fl_Color colour = fl_rgb_color( (int)(rgb[0]*255),
                                  (int)(rgb[1]*255), (int)(rgb[2]*255) );

  int thread = wandelThreadId(iccThreadSelf());
  if(thread != THREAD_HAUPT)
    icc_examin_ns::lock(__FILE__,__LINE__);

    if (L < .5)
      icc_betrachter->DD_box_stat->labelcolor( fl_rgb_color( VG ) );
    else
      icc_betrachter->DD_box_stat->labelcolor(FL_BLACK);
    icc_betrachter->DD_box_stat->color(colour);
    icc_betrachter->DD_box_stat->damage(FL_DAMAGE_ALL);
    //Fl::add_timeout(0.2, fl_delayed_redraw, icc_betrachter->DD_box_stat);
    Fl::add_idle(fl_delayed_redraw, icc_betrachter->DD_box_stat);
    Fl::awake((void*)0);

  if(thread != THREAD_HAUPT)
    icc_examin_ns::unlock(icc_betrachter->DD_box_stat, __FILE__,__LINE__);

  DBG_PROG_ENDE
}

void
ICCexamin::statusAktualisieren()
{ DBG_PROG_START
  icc_betrachter->box_stat->label(statlabel[0].c_str());
  icc_betrachter->DD_box_stat->label(statlabel[1].c_str());
  DBG_PROG_ENDE
}

void
ICCexamin::scheme(const char *name)
{ DBG_PROG_START
  // set a nice GUI surface
  Fl::scheme(name);
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
      } else
      if(Fl::event_key() == 'o'
       && Fl::event_state() == FL_COMMAND) {
        DBG_NUM_S("FL_COMMAND+O")
        icc_examin->oeffnen();
        gefunden = 1;
      }
      if(Fl::event_key() == FL_F + 1) {
        DBG_NUM_S("F1")
        ICCfltkBetrachter *b = icc_examin->icc_betrachter;
        b->ueber->hotspot(b->ueber_html);
        b->ueber->show();
        initHilfe();

        /* set visible */
        Fl_Tabs *tb = dynamic_cast<Fl_Tabs*>(b->hilfe_html->parent());
        if(tb)
          tb->value(b->ueber_html);
        gefunden = 1;
      }
    break;
  case FL_DND_ENTER:
    DBG_PROG_S( "FL_DND_ENTER" )
    fortschritt(0.01 , 1.0);
    return 1;
    break;
  case FL_DND_DRAG:
    DBG_PROG_S( "FL_DND_DRAG dnd_text_ops(" <<Fl::dnd_text_ops() <<")" )
    return 1;
    break;
  case FL_DND_LEAVE:
    DBG_PROG_S( "FL_DND_LEAVE" )
    fortschritt(1.1 , 1.0);
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
      std::string adresse, suchen = "%20", ersetzen = " ";
      int pos;
#     if APPLE_
      if(dnd_kommt &&
         Fl::event_length())
      {
        DBG_PROG_S( Fl::event_text() );
        char *temp = (char*)malloc(Fl::event_length()+1),
             *text;
        sprintf(temp, Fl::event_text());
        ICClist<std::string>profilnamen;
        while((text = strrchr(temp,'\n')) != 0)
        {
          profilnamen.push_back(text+1);
          pos = profilnamen.size()-1;
          if(profilnamen[pos].size())
            icc_parser::suchenErsetzen(profilnamen[pos], suchen, ersetzen, 0);
          text[0] = 0;
        }
        profilnamen.push_back(temp);
        pos = profilnamen.size()-1;
        if(profilnamen[pos].size())
          icc_parser::suchenErsetzen(profilnamen[pos], suchen, ersetzen, 0);
        icc_examin->oeffnen(profilnamen);
        free(temp);
      }
      dnd_kommt = false;
#     else
      if(dnd_kommt &&
         Fl::event_length())
      {
        {
          int len = Fl::event_length();
          DBG_PROG_V( len )
          char *temp = (char*)malloc(MAX_PATH*64/*Fl::event_length()+1*/),
               *text;
          memcpy(temp, Fl::event_text(), Fl::event_length());
          temp[len]=0;
          // sprintf makes problems
          //sprintf(temp, Fl::event_text());
          DBG_PROG_V( Fl::event_text() )
          DBG_PROG_V( temp )
          ICClist<std::string>profilnamen;
          while((text = strrchr(temp,'\n')) != 0)
          { DBG_PROG_V( (int*)text<<" "<<text+1 )
            if(strlen(text+1))
              profilnamen.push_back(text+1);
            text[0] = 0;
          }
          profilnamen.push_back(temp);
          // corrections
          for(unsigned int i = 0; i < profilnamen.size(); ++i) {
            const char *filter_a = "file:";
            DBG_PROG_V( profilnamen[i] )
            if(strstr(profilnamen[i].c_str(), filter_a)) {
              int len_neu = (int)(len-strlen(filter_a));
              char *txt = (char*)malloc(profilnamen[i].size()+1);
              memcpy(txt, &(profilnamen[i].c_str())[strlen(filter_a)],
                     len_neu);
              txt[len_neu]=0;
              // remove some bits
              char *zeiger = strchr(txt, '\r');
              if(zeiger)
                zeiger[0] = 0;
              profilnamen[i] = txt;
              free(txt);
            }
            DBG_PROG_V( profilnamen[i] )
            // filter empty sign
            pos = i;
            if(profilnamen[pos].size())
              icc_parser::suchenErsetzen(profilnamen[pos], suchen, ersetzen, 0);
            DBG_PROG_S( i <<" "<< profilnamen[i] );
          }
          icc_examin->oeffnen(profilnamen);
          free(temp);
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
  case FL_CLOSE:
    DBG_PROG_S( "FL_CLOSE " )
    break;
  default: 
    {
      //if(Fl::event_length())
        //DBG_MEM_S( dbgFltkEvent(e) )
        DBG_MEM_S( Fl::event_length() << " bei: "<<Fl::event_x()<<","<<Fl::event_y() );
    }
    break;
  }
  
  if(icc_examin && icc_examin->icc_betrachter)
    icc_examin->icc_betrachter->DD_farbraum->tastatur(e);
  //DBG_PROG_ENDE
  return gefunden;
}


