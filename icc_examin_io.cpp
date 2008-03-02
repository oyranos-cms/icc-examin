/*
 * ICC Examin ist eine ICC Profil Betrachter
 * 
 * Copyright (C) 2004-2006  Kai-Uwe Behrmann 
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
 * file I/O of the central class
 * 
 */


#include "icc_examin.h"
#include "icc_examin_io.h"
#include "icc_betrachter.h"
#include "icc_waehler.h"

using namespace icc_examin_ns;

#if USE_THREADS
#include "threads.h"
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


#include "icc_vrml_parser.h"

ICCexaminIO::ICCexaminIO ()
{
  DBG_PROG_START
  lade_ = false;
  neu_laden_ = true;
  DBG_PROG_ENDE
}

int
ICCexaminIO::erneuern()
{
  if(erneuern_.size()) {
    DBG_PROG_START
    int i = *erneuern_.begin();
    std::set<int>::iterator it = erneuern_.begin();
    DBG_NUM_V( *it )
    erneuern_.erase(it);
    DBG_PROG_ENDE
    return i;
  } else {
    return -1;
  }
}

void
ICCexaminIO::erneuern(int pos)
{
  DBG_PROG_START
  erneuern_.insert( pos );
  DBG_NUM_V( *erneuern_.begin() <<" "<< erneuern_.size() )
  DBG_PROG_ENDE
}

void
ICCexaminIO::oeffnenThread_ (int pos)
{ DBG_PROG_START
  if(wandelThreadId( iccThreadSelf() ) != THREAD_LADEN)
    WARN_S("THREAD_LADEN???");

  if(erneuern_.size() < 0) {
    icc_examin->fortschritt( 1.1 , 1.0 );
    DBG_PROG_ENDE
    return;
  }

  // load
  icc_examin->fortschritt( .1 , 1.0 );
  if((int)speicher_vect_.size() < (pos+1))
    WARN_S( "memory not used" )

  dateiNachSpeicher( speicher_vect_[pos], profile.name(pos) );
  
  //for (unsigned int i = 0; i < speicher_vect_.size(); ++i)
  {
    int i = pos;
    DBG_PROG_V( speicher_vect_[i].size()<<" "<<speicher_vect_[i].name() )
    icc_examin->fortschritt( 1./3.+ (double)(i)/speicher_vect_.size()/3.0 , 1 );
    profile.einfuegen( speicher_vect_[pos], pos );
    icc_examin->fortschritt( 1./3.+ (double)(i+1)/speicher_vect_.size()/3.0, 1);
    DBG_THREAD
  }

  if (profile.size())
  {
      // UI handling
    icc_examin_ns::lock(__FILE__,__LINE__);
    //erneuerTagBrowserText_ ();
    if(icc_examin->icc_betrachter->DD_farbraum->visible() &&
       !icc_examin->icc_betrachter->inspekt_html->visible() )
      icc_examin->icc_betrachter->DD_farbraum->damage(FL_DAMAGE_ALL);
    icc_examin->icc_betrachter->menueintrag_gamut_speichern->activate();
    icc_examin->icc_betrachter->menueintrag_gamut_vrml_speichern->activate();

    icc_examin->fortschritt( 2./3.+ 1./6. , 1.0 );
    if(profile.profil() == profile[pos])
      icc_examin->icc_betrachter->measurement( profile.profil()->hasMeasurement() );
    icc_examin_ns::unlock(this, __FILE__,__LINE__);

    icc_examin->fortschritt( 2./3.+ 2./6. , 1.0 );
  }
  icc_examin->fortschritt( 1.1 , 1.0 );
  DBG_PROG_ENDE
}

void
ICCexaminIO::oeffnenThread_ ()
{ DBG_PROG_START
# ifdef WIN32
  int st = wandelThreadId( iccThreadSelf() );
# endif
  if( wandelThreadId( iccThreadSelf() ) != THREAD_LADEN)
    WARN_S("THREAD_LADEN???");

  if(!speicher_vect_.size()) {
    icc_examin->fortschritt( 1.1 , 1.0 );
    DBG_PROG_ENDE
    return;
  }

  // load
  icc_examin->clear();
  icc_examin->icc_betrachter->DD_farbraum->punkte_clear();
  profile.clear();
  icc_examin->fortschritt( -.1 , 1.0  );
  for (unsigned int i = 0; i < speicher_vect_.size(); ++i)
  {
    DBG_PROG_V( speicher_vect_[i].size()<<" "<<speicher_vect_[i].name() )
    icc_examin->fortschritt( 1./3.+ (double)(i)/speicher_vect_.size()/3.0 , 1.);
    profile.einfuegen( speicher_vect_[i], -1 );

    icc_examin->fortschritt( 1./3.+ (double)(i+1)/speicher_vect_.size()/3.0 ,1);
    DBG_THREAD
  }

  std::vector<std::string> dateinamen = profile;

  if (profile.size())
  {
      // UI handling
    icc_examin_ns::lock(__FILE__,__LINE__);
    icc_examin->erneuerTagBrowserText_ ();
    if(icc_examin->icc_betrachter->DD_farbraum->visible() &&
       !icc_examin->icc_betrachter->inspekt_html->visible() )
      icc_examin->icc_betrachter->DD_farbraum->damage(FL_DAMAGE_ALL);
    icc_examin->icc_betrachter->menueintrag_gamut_speichern->activate();
    icc_examin->icc_betrachter->menueintrag_gamut_vrml_speichern->activate();

    icc_examin->fortschritt( 2./3.+ 1./6. , 1.0 );

    icc_examin->icc_betrachter->measurement( profile.profil()->hasMeasurement() );

      // UI handling
    if(icc_examin->farbraumModus())
      icc_examin->gamutAnsichtZeigen();
    else if(!icc_examin->icc_betrachter->details->visible_r())
    {
      icc_examin->icc_betrachterNeuzeichnen( icc_examin->icc_betrachter->tag_viewer);
      icc_examin->waehleTag( icc_examin->tag_nr() );
    }

    icc_examin_ns::unlock(this, __FILE__,__LINE__);

      // sort
    if( dateinamen.size() &&
        (dateinamen[0].find( "wrl",  dateinamen[0].find_last_of(".") )
         != std::string::npos) )
    {
      std::string d (speicher_vect_[0], speicher_vect_[0].size());
      //DBG_NUM_V( d <<" "<< size )
      std::vector<ICCnetz> netze = extrahiereNetzAusVRML (d);
      if( netze.size() )
      { DBG_NUM_V( netze.size() )
        for(unsigned int i = 0; i < netze.size(); ++i ) {
          netze[i].undurchsicht = 0.6;
          netze[i].grau = false;
          netze[i].aktiv = true;
        }
        icc_examin_ns::lock(__FILE__,__LINE__);
        icc_examin->icc_betrachter->DD_farbraum->hineinNetze(netze);
        std::vector<std::string> texte;
        texte.push_back(_("CIE *L"));
        texte.push_back(_("CIE *a"));
        texte.push_back(_("CIE *b"));
        icc_examin->icc_betrachter->DD_farbraum->achsNamen(texte);
        icc_examin->icc_betrachter->DD_farbraum->punkte_clear();
        icc_examin->gamutAnsichtZeigen();
        icc_examin_ns::unlock(this, __FILE__,__LINE__);
      } else
        WARN_S(_("no net found in VRML file"))
    } else {
      DBG_PROG
      //farbraum();
    }

    icc_examin->fortschritt( 2./3.+ 2./6. , 1.0 );
    // ICCwaehler
      // refresh
    static std::vector<std::string> namen_neu, namen_alt;
    bool namensgleich = false;
    namen_neu = profile;
    DBG_PROG_V( namen_neu.size() <<" "<< namen_alt.size() )
    if(namen_alt.size() == namen_neu.size()) {
      namensgleich = true;
      DBG_NUM_S( "name was allready there" )
      for(int i = 0; i < (int)namen_neu.size(); ++i)
        if(namen_neu[i] != namen_alt[i])
          namensgleich = false;
    }
    namen_alt = namen_neu;
    DBG_NUM_V( "#################### " << namensgleich << " ##############")
    if(!namensgleich)
    {
      icc_examin_ns::lock(__FILE__,__LINE__);
      if(icc_examin->icc_waehler_)
        icc_examin->icc_waehler_->clear();
      int anzahl = profile.size();
      DBG_PROG_V( anzahl )
      double undurchsicht;
      bool grau;
      bool waehlbar;
      std::vector<int> aktiv = profile.aktiv();
      DBG_PROG_V( aktiv.size() )
      for(int i = 0; i < anzahl; ++i) {
        DBG_PROG_V( i )

        if( i >= (int)icc_examin->icc_betrachter->DD_farbraum->dreiecks_netze.size() ) {
          WARN_S( _("no net found. Is Argyll installed?") )
          break;
        }

        undurchsicht= icc_examin->icc_betrachter->DD_farbraum->dreiecks_netze[i].undurchsicht;
        DBG_PROG_V( undurchsicht )
        grau = icc_examin->icc_betrachter->DD_farbraum->dreiecks_netze[i].grau?true:false;
        waehlbar = profile[i]->size() > 128 ? true : false;

        if(icc_examin->icc_waehler_)
        {
          std::string name = profile.name(i);
          icc_examin->icc_waehler_->push_back( dateiName( name.c_str() ),
                                undurchsicht, grau , aktiv[i], waehlbar);
        }
      }
      icc_examin_ns::unlock(this, __FILE__,__LINE__);
    }

    // set window name
    if(0) {
      icc_examin->detaillabel = "ICC Examin: ";
      icc_examin->detaillabel.insert( icc_examin->detaillabel.size(), dateiName(dateinamen[0]) );
      icc_examin_ns::lock(__FILE__,__LINE__);
      icc_examin->icc_betrachter->details->label(icc_examin->detaillabel.c_str());
      icc_examin_ns::unlock(this, __FILE__,__LINE__);
      DBG_PROG_V( dateinamen[0] <<" "<< icc_examin->detaillabel )
    }

  }

  icc_examin->fortschritt( 1.1 , 1.0 );
  DBG_PROG_ENDE
}

/** @brief gives the file name without path back
 *  @param name                 long file name
 *  @return                     file name
 */
const char*
dateiName(const char* name)
{
  const char* dateiname=0;
        // extract file name
        if(name)
          dateiname = strrchr(name,ICC_DIR_SEPARATOR_C);

        if(!dateiname)
          dateiname = name;
        else
          ++dateiname;
  return dateiname;
}

const char*
dateiName(std::string name)
{
  const char * ptr = name.c_str();
  return dateiName( ptr );
}

void
ICCexaminIO::lade (std::vector<Speicher> & neu)
{
  if(!lade())
  {
    speicher_vect_ = neu;
    lade_ = true;
  } else {
    DBG_THREAD_S( "must wait" )
  }
}

// split in threads for a liquid UI
#if USE_THREADS
void*
#else
void
#endif
ICCexaminIO::oeffnenStatisch_ (void* ie)
{
  registerThreadId( iccThreadSelf(), THREAD_LADEN );

  DBG_PROG_START

  // detect run time errors
  {
    static int erster = true;
    if(!erster)
      WARN_S("programing error: thread must\n" <<
             "run only one time.")
    erster = false;
  }
  if(!ie) WARN_S( "no ICCexaminIO class available" )

  // connect to main class
  ICCexamin* examin = (ICCexamin*) ie;

  // start loop for this thread
  while(1) {
    if(icc_examin->status_) {
      if(examin->io_->lade_) {
        examin->io_->oeffnenThread_();
        examin->io_->lade_ = false;
        //examin->erneuern(-1);
      } else {
        int e = examin->erneuern();
        if(e >= 0 &&
           e < profile.size()) {
          examin->io_->oeffnenThread_( e );
          examin->io_->lade_ = false;
        } else {
          // short pause 
          icc_examin_ns::sleep(0.2); DBG_THREAD
        }
      }
    } else
      icc_examin_ns::sleep(0.2);
  }

  DBG_PROG_ENDE
# if USE_THREADS
  return 0;
# endif
}

void
oeffnen_cb(const char* filenames)
{ DBG_PROG_START
  std::vector<std::string> dateinamen;
  dateinamen.push_back( filenames );
  status( filenames )
  DBG_PROG_V( filenames )
  icc_examin->oeffnen( dateinamen );
  DBG_PROG_ENDE
}

void
ICCexaminIO::oeffnen (std::vector<std::string> dateinamen)
{ DBG_PROG_START
# if 1
  if(!dateinamen.size()) {
    WARN_S("no file name specified")
    icc_examin->fortschritt( 1.1 , 1.0 );
    DBG_PROG_ENDE
    return;
  }
# endif

  int x = icc_examin->icc_betrachter->vcgt->x() + icc_examin->icc_betrachter->vcgt->w()/2;
  int y = icc_examin->icc_betrachter->vcgt->y() + icc_examin->icc_betrachter->vcgt->h()/2;


  icc_examin->fortschritt( 0.0 , 1.0 );

  // loading
  std::vector<Speicher> ss;
  // resize uses copy, and creates a reference by this to the
  // same object ; instead we need a new object => push_back()
  int moni_dabei = 0;
  for (unsigned int i = 0; i < dateinamen.size(); ++i)
  {
    ss.push_back(Speicher());

    ss[i] = dateiNachSpeicher(dateinamen[i]);
    if(i == 0 && ss[i].size() == 0) {
      status( _("Stop loading ") << dateinamen[i] )
      icc_examin->fortschritt( 1.1 , 1.0 );
      return;
    }
    if (dateinamen[i] == icc_oyranos.moni_name( x,y ))
      moni_dabei = true;
  }
  DBG_PROG

  // the monitor profile is allways included for comparisions 
  if (!moni_dabei && ss.size())
    ss.push_back(icc_oyranos.moni( x,y ));

  lade (ss);

  DBG_PROG_ENDE
}

void
ICCexaminIO::oeffnen ()
{ DBG_PROG_START
  icc_examin->fortschritt(0.01 , 1.0);
  std::vector<std::string> dateinamen = profile;

  //Fl_File_Icon	*icon;	// New file icon
  DBG_PROG
    dateiwahl->callback(dateiwahl_cb);
    dateiwahl->preview(true);

    const char* ptr = NULL;
    if (dateinamen.size()) {
      ptr = dateinamen[0].c_str();
      dateiwahl->value(ptr);
      DBG_PROG_S( dateinamen[0])
    } 
      if(ptr) DBG_PROG_V( ptr );
    if (!ptr)
      ptr = getenv("PWD");

      if(ptr) DBG_PROG_V( ptr );
    if(( ptr &&
        (ptr[0] == '/') &&
        (strlen(ptr) == 1) ) ||
        !ptr )
    {
      ptr = getenv("HOME");
    }

    dateiwahl->show();

    if(ptr)
      dateiwahl->value(ptr);


    // protected: dateiwahl->window->clear_flag(64); //Fl_Window::FL_MODAL
    dateiwahl->type(MyFl_File_Chooser::MULTI);
    while (dateiwahl->visible())
      icc_examin_ns::wait( 0, true );
    //dateiwahl->type(MyFl_File_Chooser::SINGLE | MyFl_File_Chooser::CREATE);
    //dateiwahl->window->set_modal();

    DBG_NUM_V( dateiwahl->count() )
    if (dateiwahl->count() && dateiwahl->value()) {
      DBG_NUM_V( dateiwahl->value() )
      dateinamen.resize(dateiwahl->count());
      for (int i = 1; i <= dateiwahl->count(); i++)
        dateinamen[i-1] = dateiwahl->value(i);
    }
  DBG_PROG


  if (dateinamen.size() == 0) {
  }

  dateiwahl->callback(0);
  dateiwahl->preview(false);

  oeffnen( dateinamen );
  neu_laden_ = true;
  DBG_PROG_ENDE
}

bool
ICCexaminIO::berichtSpeichern (void)
{ DBG_PROG_START
  bool erfolgreich = true;
  std::string dateiname = profile.name();  DBG_PROG_V( dateiname )

  // substitute profile name
  std::string::size_type pos=0;
  if ((pos = dateiname.find_last_of(".", dateiname.size())) != std::string::npos)
  { DBG_PROG
    dateiname.replace (pos, 5, ".html"); DBG_NUM_S( ".html set" )
  } DBG_PROG_V( dateiname )

  // call FLTK file dialog
  DBG_PROG_V( dateiwahl->filter() )

  std::string muster = dateiwahl->filter(); DBG_PROG
  std::string datei;
  if (dateiwahl->value())
    datei = dateiwahl->value(); DBG_PROG
  std::string titel = dateiwahl->label(); DBG_PROG

  dateiwahl->filter(_("HTML Documents (*.htm*)")); DBG_PROG
# ifdef HAVE_FLU
  dateiwahl->cd(".");
# endif
  dateiwahl->label(_("Save Report")); DBG_PROG
  dateiwahl->value(dateiname.c_str()); DBG_PROG

  dateiwahl->show(); DBG_PROG
  dateiwahl->type(MyFl_File_Chooser::SINGLE | MyFl_File_Chooser::CREATE);
  while( dateiwahl->shown() )
    icc_examin_ns::wait( 0.05, true );

  DBG_PROG_V( dateiwahl->filter() )
  if (dateiwahl->value())
    dateiname = dateiwahl->value();
  else
    dateiname = "";
  DBG_PROG

  dateiwahl->filter(muster.c_str()); DBG_PROG
  dateiwahl->value(datei.c_str()); DBG_PROG
  dateiwahl->label(titel.c_str()); DBG_PROG
  DBG_PROG_V( dateiwahl->filter() )

  DBG_PROG_V( dateiname )

  if (dateiname == "" ||
      dateiname == profile.name())
  { DBG_PROG_V( dateiwahl->count() << dateiname )
    icc_examin->fortschritt (1.1 , 1.0);
    DBG_PROG_ENDE
    return false;
  }

  // generate report
  bool export_html = true;
  std::string bericht = profile.profil()->report(export_html);
  // save
  std::ofstream f ( dateiname.c_str(),  std::ios::out );
  f.write ( bericht.c_str(), (std::streamsize)bericht.size() );
  f.close();

  DBG_PROG_ENDE
  return erfolgreich;
}

bool
ICCexaminIO::gamutSpeichern (IccGamutFormat format)
{
    DBG_PROG_START
  bool erfolgreich = true;
  std::string dateiname = profile.name();  DBG_PROG_V( dateiname )

  // set profile name
  std::string::size_type pos=0;
  if ((pos = dateiname.find_last_of(".", dateiname.size())) != std::string::npos)
  {   DBG_PROG
    if(format == ICC_ABSTRACT) {
      dateiname.replace (pos, 5, "_proof.icc"); DBG_NUM_S( "_proof.icc gesetzt")
    } else if(format == ICC_VRML) {
      dateiname.replace (pos, 5, ".wrl"); DBG_NUM_S( ".wrl gesetzt")
    }
  } DBG_PROG_V( dateiname )

  // call FLTK file dialog
    DBG_PROG_V( dateiwahl->filter() )
  std::string muster = dateiwahl->filter(); DBG_PROG
  std::string datei;
  if (dateiwahl->value())
    datei = dateiwahl->value(); DBG_PROG
  std::string titel = dateiwahl->label(); DBG_PROG

# ifdef HAVE_FLU
  dateiwahl->cd(".");
# endif
  if(format == ICC_ABSTRACT) {
    dateiwahl->filter(_("ICC colour profiles (*.ic*)")); DBG_PROG
    dateiwahl->label(_("Save Gamut as Profile")); DBG_PROG
  } else if(format == ICC_VRML) {
    dateiwahl->filter(_("VRML Files (*.wrl)")); DBG_PROG
    dateiwahl->label(_("Save Gamut as VRML")); DBG_PROG
  }
  dateiwahl->value(dateiname.c_str()); DBG_PROG

  dateiwahl->show(); DBG_PROG
  dateiwahl->type(MyFl_File_Chooser::SINGLE | MyFl_File_Chooser::CREATE);
  while( dateiwahl->shown() )
    icc_examin_ns::wait( 0.05, true );

    DBG_PROG_V( dateiwahl->filter() )
  if (dateiwahl->value())
    dateiname = dateiwahl->value();
  else
    dateiname = "";

    DBG_PROG

  dateiwahl->filter(muster.c_str()); DBG_PROG
  dateiwahl->value(datei.c_str()); DBG_PROG
  dateiwahl->label(titel.c_str()); DBG_PROG
    DBG_PROG_V( dateiwahl->filter() )

    DBG_PROG_V( dateiname )

  if (dateiname == "" ||
      dateiname == profile.name())
  { DBG_PROG_V( dateiwahl->count() << dateiname )
    icc_examin->fortschritt (1.1 , 1.0);
      DBG_PROG_ENDE
    return false;
  }

  Speicher profil;
  size_t groesse = 0;
  char* daten = 0;
  daten = profile.profil()->saveProfileToMem( &groesse );
  profil.ladeNew( daten, groesse );
  if(format == ICC_ABSTRACT) {
    // generate Gamut profile
    Speicher speicher;
    icc_oyranos.gamutCheckAbstract( profil, speicher,
                                    icc_examin->intentGet(NULL),
                                    /*cmsFLAGS_GAMUTCHECK |*/ cmsFLAGS_SOFTPROOFING );

    // save
    saveMemToFile ( dateiname.c_str(), (const char*)speicher, speicher.size() );
    speicher.clear();
  } else if(format == ICC_VRML) {
    std::string vrml;
    vrml = icc_oyranos.vrmlVonProfil ( *profile.profil(),
                                       icc_examin->intentGet(NULL) );
    // save
    saveMemToFile ( dateiname.c_str(), vrml.c_str(), vrml.size() );
  }
  profil.clear();

    DBG_PROG_ENDE
  return erfolgreich;
}

