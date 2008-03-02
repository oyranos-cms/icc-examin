// Author:    Kai-Uwe Behrmann
// Copyright: Kai-Uwe Behrmann <ku.b@gmx.de>
// Date:      12. 09. 2004


#ifndef ICC_GL_H
#define ICC_GL_H
#define _(text) text

class GL_Ansicht : public Fl_Group {
  std::vector<std::vector<std::vector<std::vector<double> > > > tabelle;
  std::vector<std::string>texte;
  std::vector<double>punkte;
  std::vector<std::vector<double> >kurven;
  Fl_Group *GLFenster;
public:
  bool GLfenster_zeigen,
       first;
  GL_Ansicht(int X,int Y,int W,int H) ;
  void init();
  void myGLinit();
  void MakeDisplayLists();
  void MenuInit();
  void draw();
  void hinein_punkt(std::vector<double> vect, std::vector<std::string> txt);
  void hinein_kurven(std::vector<std::vector<double> >vect, std::vector<std::string> txt);
  void hinein_tabelle(std::vector<std::vector<std::vector<std::vector<double> > > >vect, std::vector<std::string> txt);
  void ruhig_neuzeichnen(void);
  void zeigen();
  void verstecken();
};

void reshape(int w, int h);
void display();
void sichtbar(int v);
void menuuse(int v);
void handlemenu(int value);

#endif //ICC_GL_H
