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

extern int kanal;
extern bool duenn;

class GL_Ansicht : public Fl_Group {
  std::vector<std::vector<std::vector<std::vector<double> > > > tabelle;
  std::vector<std::string>nachFarbNamen;
  std::vector<std::string>vonFarbNamen;
  std::vector<std::string>farbNamen;
  std::vector<double> punkte;
  std::vector<std::vector<double> >kurven;
  Fl_Group *GLFenster;
  void MenueErneuern();
  int  MenueKanalEintraege;
  int  MenueSchnitt;
  void init();
  void myGLinit();
  void MenuInit();
public:
  int agv;
  bool GLfenster_zeigen,
       first;
  GL_Ansicht(int X,int Y,int W,int H);
  ~GL_Ansicht();
  void draw();
  void hinein_punkte(std::vector<double> vect, std::vector<std::string> txt);
  void hinein_kurven(std::vector<std::vector<double> >vect, std::vector<std::string> txt);
  void hinein_tabelle(std::vector<std::vector<std::vector<std::vector<double> > > >vect,
                               std::vector<std::string> vonFarben,
                               std::vector<std::string> nachFarben);

  int  Punktform;                // Form der Gitterpunkte der Transformationstabelle
  void MakeDisplayLists();
  void zeigen();
  void verstecken();
  void stop() {if (!first) { agvSwitchMoveMode (agviewer::AGV_STOP); } }

  char* kanalName() {return (char*)nachFarbNamen[kanal].c_str(); }
  char* kanalName(unsigned int i) { if (nachFarbNamen.size()>i) 
                                      return (char*)nachFarbNamen[i].c_str();  
                                    else  return _("Gibts nicht"); }
  unsigned int kanaele() {return nachFarbNamen.size(); }
};

void reshape(int w, int h);
void display();
void sichtbar(int v);
void menuuse(int v);
void handlemenu(int value);


#endif //ICC_GL_H
