/*
 * ICC Examin ist eine ICC Profil Betrachter
 * 
 * Copyright (C) 2004  Kai-Uwe Behrmann 
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
 * Der 3D Betrachter.
 * 
 */

// Date:      12. 09. 2004


#ifndef ICC_GL_H
#define ICC_GL_H
#include <vector>

#include "agviewer.h"

class GL_Ansicht : public Fl_Group {
  std::vector<std::vector<std::vector<std::vector<double> > > > tabelle_;
  std::vector<std::string>nach_farb_namen_;
  std::vector<std::string>von_farb_namen_;
  std::vector<std::string>farb_namen_;
  std::vector<std::vector<double> > dreiecks_netze_;
  std::vector<std::vector<float> >  dreiecks_farben_; // rgba 0.0 - 1.0 (n*4)
  std::vector<std::string>          netz_namen_;
  std::vector<double> punkte_;        //                (n*3)
  std::vector<float>  farben_;        // rgba 0.0 - 1.0 (n*4)
  std::vector<std::vector<double> >kurven_;
  Fl_Group *gl_fenster_;
  void menueErneuern();
  int  menue_kanal_eintraege_;
  int  menue_schnitt_;
  void init();
  void myGLinit();
  void menuInit();
  bool gl_fenster_zeigen_;
  int  agv_;
  bool first_;
public:
  GL_Ansicht(int X,int Y,int W,int H);
  ~GL_Ansicht();
  int  id()          {return agv_; }
  int  agv()         {return agv_; }
  void draw();
  void hineinPunkte (std::vector<double> vect,
                     std::vector<std::string> achsNamen);
  void hineinPunkte (std::vector<double> vect, 
                     std::vector<float> farben_,
                     std::vector<std::string> achsNamen);
  void hineinPunkte (std::vector<double> punktKoordinaten, 
                     std::vector<float>  punktFarben,
                     std::vector<std::string> farb_namen_,
                     std::vector<std::string> achsNamen);
  void hineinNetze  (std::vector<std::vector<double> >dreiecks_netze_, 
                     std::vector<std::vector<float> > dreiecks_farben_,
                     std::vector<std::string> netz_namen_,
                     std::vector<std::string> _achsNamen);

  void hineinKurven(std::vector<std::vector<double> >vect,
                     std::vector<std::string> txt);

  void hineinTabelle(std::vector<std::vector<std::vector<std::vector<double> > > >vect,
                               std::vector<std::string> vonFarben,
                               std::vector<std::string> nachFarben);

  int  kanal;                    // gewählter Kanal
  int  Punktform;                // Form der Gitterpunkte der Transformationstabelle
  double seitenverhaeltnis;
  double schnitttiefe;

  void makeDisplayLists();
  void zeigen();
  void verstecken();
  void stop() {if (!first_) { agvSwitchMoveMode (agviewer::AGV_STOP); } }

  char* kanalName() {return (char*)nach_farb_namen_[kanal].c_str(); }
  char* kanalName(unsigned int i) { if (nach_farb_namen_.size()>i) 
                                      return (char*)nach_farb_namen_[i].c_str();  
                                    else  return _("Gibts nicht"); }
  unsigned int kanaele() {return nach_farb_namen_.size(); }
};

void reshape(int w, int h);
void display();
void sichtbar(int v);
void menuuse(int v);
void handlemenu(int value);
int  dID(int display_list);



#endif //ICC_GL_H
