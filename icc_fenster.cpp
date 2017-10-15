/*
 * ICC Examin ist eine ICC Profil Betrachter
 * 
 * Copyright (C) 2004-2016  Kai-Uwe Behrmann 
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
 * file selection
 * 
 */

//#include "icc_utils.h"
//#include "icc_kette.h"
#include "icc_betrachter.h"
#include "icc_examin.h"
#include "icc_gl.h"
#include "icc_dateiwahl.h"
#include "icc_fenster.h"
#include "icc_helfer_fltk.h"

#include <string>

#include <FL/Fl.H>
#if HAVE_X && !defined(__APPLE__)
#include <FL/x.H>
#include <X11/Xutil.h>
#endif
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Return_Button.H>
#include <FL/Fl_Text_Display.H>
#include <FL/Fl_Output.H>
#include <FL/Fl_Widget.H>
#include <FL/Fl_Scroll.H>
#include <FL/Enumerations.H>
#include <FL/Fl_JPEG_Image.H>
#include <FL/Fl_PNG_Image.H>
#include <FL/Fl_Shared_Image.H>

namespace icc_examin_ns {

#ifdef HAVE_FLTK
MyFl_File_Chooser  *dateiwahl_ = 0;

Fl_Image* iccImageCheck( const char* fname, uchar *header ICC_UNUSED, int len ICC_UNUSED )
{
  const char    *home;          // Home directory
  char          preview[1024],  // Preview filename
                command[1024];  // Command
  int stat;

  const char *endung = strrchr(fname,'.');
  if( !endung ) return NULL;
  ICCprofile::ICCDataType file_type = guessFileType( fname );

  if(profile.size())
    file_type = guessFileType( profile.profil()->filename() );

  if(strcmp(endung+1,"jpg") == 0) return NULL;

  home = getenv("HOME");
  sprintf(preview, "%s/.preview.ppm", home ? home : "");
  remove(preview);

  if (file_type == ICCprofile::ICCimageDATA)
  {
    oyProfile_s * p;
    if(dateiwahl_)
    {
      icc_examin_ns::MyFl_Double_Window *w = dateiwahl()->window;
      p = icc_oyranos.oyMoni(w->x()+w->w()/2, w->y()+w->h()/2, 0);
    } else
      p = icc_oyranos.oyMoni(0,0,0);
    oyImage_s * image = NULL;
    oyImage_FromFile(fname, icc_oyranos.icc_profile_flags, &image, NULL);
    oyConversion_s * cc = oyConversion_CreateFromImage (
                                image, 0,
                                p, oyUINT8, oyOPTIONATTRIBUTE_ADVANCED, 0 );
    oyConversion_RunPixels( cc, 0 );
    oyImage_Release( &image );
    image = oyConversion_GetImage( cc, OY_OUTPUT );
    oyImage_WritePPM( image, preview, fname );
    WARN_S("wrote file:" << fname <<" to "<<preview);
    return new Fl_PNM_Image( preview );
  } else if(file_type == ICCprofile::ICCprofileDATA)
  {
    std::string t = "oyranos-profile-graph -w 512 -b";
    if(!icc_examin->icc_betrachter->DD_farbraum->spectral_line)
      t += " -s";
    t += " -o ";
    t += preview;
    if(profile.size())
    {
      for(int i = 0; i < profile.size(); ++i)
      {
        t += " '";
        t += profile.name(i);
        t += "'";
      }
    } else
    {
        t += " '";
        t += fname;
        t += "'";
    }
    DBG_NUM_V( t.c_str() )
    stat = system (t.c_str());
    return new Fl_PNG_Image( preview );

  } else
  {
    sprintf (command, "dcraw -i '%s'\n", fname);
    DBG_NUM_V( command )

    stat = system (command);
    if(stat) {
      if (file_type != ICCprofile::ICCimageDATA && stat) {
        if (stat > 0x200)
          WARN_S (_("The \"rawphoto\" plugin won't work because "
        "there is no \"dcraw\" executable in your path."));
      }
      return NULL;
    }

    sprintf(command,
          "dcraw -e -v -c"
          " \'%s\' > \'%s\' ", fname, preview);

    if (system(command)) return NULL;
  }

  return new Fl_JPEG_Image( preview );
}


MyFl_File_Chooser  * dateiwahl()
{
  if(!dateiwahl_)
  {
    my_fl_translate_file_chooser();

    const char* ptr = NULL;
    if (profile.size())
      ptr = profile.name().c_str();
    dateiwahl_ = new MyFl_File_Chooser(ptr, _("All Formats (*.{ICC,ICM,txt,it8,IT8,RGB,CMYK,ti*,cgats,CIE,cie,nCIE,oRPT,DLY,LAB,Q60,wrl,vrml,wrl.gz,vrml.gz,png,ppm,pnm,pgm,pfm})	ICC colour profiles (*.{I,i}{C,c}{M,m,C,c})	Measurement (*.{txt,it8,IT8,RGB,CMYK,ti*,cgats,CIE,cie,nCIE,oRPT,DLY,LAB,Q60})	Argyll Gamuts (*.{wrl,vrml,wrl.gz,vrml.gz}	Images (*.png,ppm,pnm,pgm,pfm)"), MyFl_File_Chooser::MULTI, _("Which ICC profile?"));
    dateiwahl_->callback(dateiwahl_cb);
    dateiwahl_->preview(true);


    icc_examin_ns::MyFl_Double_Window *w = dateiwahl_->window;
    w->use_escape_hide = true;
  }
  return dateiwahl_;
}

void
dateiwahl_cb (MyFl_File_Chooser *f, void *data, int finish ICC_UNUSED)
{ DBG_PROG_START

  const char *filename;

    MyFl_File_Chooser* fl = (MyFl_File_Chooser*)f;

    DBG_NUM_V( data )
    filename = fl->value();

    static ICCThreadList<std::string> file_vect;

    file_vect.frei(false);

    if (filename && fl->count() && dateiwahl()->preview()) {
      ICClist<std::string> profilnamen;
      profilnamen.resize(fl->count());
      for (int i = 0; i < fl->count(); i++) {
        if(strchr(fl->value(i), '/') == 0) {
          profilnamen[i] = fl->directory();
          profilnamen[i].append("/");
          profilnamen[i].append(fl->value(i));
        } else 
          profilnamen[i] = fl->value(i);
        DBG_PROG_V( i <<":"<< profilnamen[i] )
      }

      // add new files
      int file_in_list;
      for(int i = 0; i < (int)profilnamen.size(); ++i)
      {
        file_in_list = 0;
        for( int j = 0; j < (int)file_vect.size(); ++j )
          if(profilnamen[i] == file_vect[j])
          {
            file_in_list = 1;
            break;
          }

        if(!file_in_list)
          file_vect.push_back( profilnamen[i] );
      }

      // remove unseen files
      int run = 1;

      while(run)
      {
        run = 0;
        for(int i = 0; i < (int)file_vect.size(); ++i)
        {
          file_in_list = 0;
          for(int j = 0; j < (int)profilnamen.size(); ++j)
            if(profilnamen[j] == file_vect[i])
            {
              file_in_list = 1;
              break;
            }

          if(!file_in_list)
          {
            file_vect.erase( &file_vect[i] );
            run = 1;
            break;
          }
        }
      }

      if(profilnamen.size() != file_vect.size())
        WARN_S("Something went wrong here.");

      DBG_PROG_V( file_vect.size() << filename )
      icc_examin->oeffnen( file_vect );
    }

  file_vect.frei(true);

  DBG_PROG_ENDE
}

MyFl_Double_Window* nachricht_ (std::string text); 
MyFl_Double_Window* log_ (std::string text, int code); 

#if 1
MyFl_Double_Window*
nachricht (std::string text) {
  // for Fl_Scroll no vtable created:
  // icc_fenster.cpp:162: undefined reference to `icc_examin_ns::Fl_Scroll::Fl_Scroll[in-charge](int, int, int, int, char const*)'
  // Now the funtion is outside of icc_examin_ns::
  return nachricht_(text);
}

struct log_cb_struct {
  std::string text;
  int code;
};

void log_cb(void * data)
{
  struct log_cb_struct * a = (struct log_cb_struct *)data;
  log_(a->text, a->code);
  delete a;
}

iccThreadMutex_m icc_log_mutex_;

MyFl_Double_Window*
log (std::string text, int code)
{
  /* let FLTK call the main thread */
  struct log_cb_struct * a = new struct log_cb_struct;
  a->text = text;
  a->code = code;
#if ((FL_MAJOR_VERSION * 10000 + FL_MINOR_VERSION * 100 + FL_PATCH_VERSION) > 10108)
  Fl::awake( log_cb, a );
#else
  Fl::awake( );
#endif
  return 0;
}
void log_show(void)
{
  if(!log_window)
    log_("", 0);
  log_window->show();
}

#else
MyFl_Double_Window*
nachricht(std::string text)
{ DBG_PROG_START
  fl_message_icon()->resize(1,1,1,1);
  fl_message_icon()->hide();
  DBG_PROG
  fl_message(text.c_str());
  DBG_PROG_ENDE
}
#endif

#endif

static void cb_Gut(Fl_Return_Button*, void* v) {
  ((MyFl_Double_Window*)v)->hide();
}

static Fl_Text_Display *display_info=(Fl_Text_Display*)0;
static Fl_Text_Display *display_log=(Fl_Text_Display*)0;
static Fl_Text_Buffer *buffer_log=(Fl_Text_Buffer*)0;
MyFl_Double_Window *log_window=(MyFl_Double_Window*)0;
int log_window_poped = 0;
static ICCThreadList<int> icc_log_lock_;

int iccCLIMessage( const char * text, int code ICC_UNUSED )
{
  cout << text << endl;
  return 0;
}

MyFl_Double_Window*
log_ (std::string text, int code)
{
  int log_window_new = 0;

  int dbg_id = wandelThreadId ( iccThreadSelf() );

  /* do not go over expensive log window in case we want explicitely debug */
  if(icc_debug ||
     (dbg_id != THREAD_HAUPT &&!log_window))
    iccCLIMessage( text.c_str(), code );

  /* avoid to create a FLTK window from outside the main thread */
  if(dbg_id != THREAD_HAUPT && !log_window)
    return 0;

  icc_log_lock_.frei(false);

  if(icc_examin && icc_examin->laeuft() && !log_window)
  {
    MyFl_Double_Window* w = log_window = new MyFl_Double_Window(SCALE(600), SCALE(226), _("Log:"));
    w->hotspot(w);
    { Fl_Return_Button* o = new Fl_Return_Button(SCALE(220), SCALE(195), SCALE(160), SCALE(25), _("Yes"));
      o->shortcut(0xff0d);
      o->callback((Fl_Callback*)cb_Gut, (void*)(w));
      o->align(FL_ALIGN_CENTER|FL_ALIGN_INSIDE);
      w->hotspot(o);
    }
      { Fl_Text_Display* o = display_log = new Fl_Text_Display(0, 0, SCALE(600), SCALE(190));
        //o->type(12);
        //o->color((Fl_Color)53);
        o->textfont( FL_COURIER );
        o->box( FL_FLAT_BOX );
        o->color(FL_BACKGROUND_COLOR);
        Fl_Group::current()->resizable(o);
      }
    //w->show();
    w->end();
    w->use_escape_hide = true;

    log_window_new = 1;
  }

  if(!buffer_log)
    buffer_log = new Fl_Text_Buffer(0);

  if(display_log && buffer_log && !display_log->buffer())
    display_log->buffer(buffer_log);

  if(buffer_log)
  {
    int l = buffer_log->length();
    int ip = -1;

    if(display_log)
      ip = display_log->insert_position();

    buffer_log->append( text.c_str() );

    if((display_log && l == ip) || log_window_new)
    {
      ip = buffer_log->length();
      display_log->insert_position( ip );
      display_log->show_insert_position();
    }

    if(code == ICC_MSG_ERROR &&
       log_window &&
       icc_examin &&
       icc_examin->icc_betrachter &&
       !log_window_poped)
    {
      log_window_poped = 1;
      while(!icc_examin->frei())
        icc_examin_ns::sleep(0.01);
      log_window->show();
    }
  }

  icc_log_lock_.frei(true);
  return log_window;
}

MyFl_Double_Window*
nachricht_ (std::string text) {
  MyFl_Double_Window* w;
  { MyFl_Double_Window* o = new MyFl_Double_Window(SCALE(275), SCALE(326), _("Information:"));
    w = o;
    w->hotspot(o);
    { Fl_Return_Button* o = new Fl_Return_Button(SCALE(60), SCALE(295), SCALE(160), SCALE(25), _("Yes"));
      o->shortcut(0xff0d);
      o->callback((Fl_Callback*)cb_Gut, (void*)(w));
      o->align(FL_ALIGN_CENTER|FL_ALIGN_INSIDE);
      w->hotspot(o);
    }
      { Fl_Text_Display* o = display_info = new Fl_Text_Display(0, 0, SCALE(275), SCALE(290));
        //o->type(12);
        //o->color((Fl_Color)53);
        Fl_Text_Buffer * buffer = new Fl_Text_Buffer(0);
        buffer->append( text.c_str() );
        o->buffer( buffer );
        o->textfont( FL_COURIER );
        o->box( FL_FLAT_BOX );
        o->color(FL_BACKGROUND_COLOR);
        Fl_Group::current()->resizable(o);
      }
    o->show();
    o->end();
    o->use_escape_hide = true;
  }
  //output_info->value(text.c_str());
  return w;
}

}




