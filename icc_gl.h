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
 * Der 3D Betrachter.
 * 
 */

// Date:      12. 09. 2004


#ifndef ICC_GL_H
#define ICC_GL_H
#define _(text) text

extern int kanal;
extern bool duenn;

class GL_Ansicht : public Fl_Group {
  std::vector<std::vector<std::vector<std::vector<double> > > > tabelle;
  std::vector<std::string>texte;
  std::vector<std::string>pcsNamen;
  std::vector<double>punkte;
  std::vector<std::vector<double> >kurven;
  Fl_Group *GLFenster;
  void MenueErneuern();
  int  MenueKanalEintraege;
  void init();
  void myGLinit();
  void MenuInit();
public:
  bool GLfenster_zeigen,
       first;
  GL_Ansicht(int X,int Y,int W,int H);
  ~GL_Ansicht();
  void draw();
  void hinein_punkt(std::vector<double> vect, std::vector<std::string> txt);
  void hinein_kurven(std::vector<std::vector<double> >vect, std::vector<std::string> txt);
  void hinein_tabelle(std::vector<std::vector<std::vector<std::vector<double> > > >vect,
                               std::vector<std::string> txt,
                               std::vector<std::string> pcs);
  void ruhig_neuzeichnen(void);

  int  Punktform;                // Form der Gitterpunkte der Transformationstabelle
  void MakeDisplayLists();
  void zeigen();
  void verstecken();
  void stop() {if (!first) { agvSwitchMoveMode (AGV_STOP); } }

  char* kanalName() {return (char*)texte[kanal].c_str(); }
  char* kanalName(unsigned int i) { if (texte.size()>i) 
                                      return (char*)texte[i].c_str();  
                                    else  return _("Gibts nicht"); }
  unsigned int kanaele() {return texte.size(); }
};

void reshape(int w, int h);
void display();
void sichtbar(int v);
void menuuse(int v);
void handlemenu(int value);


#endif //ICC_GL_H
