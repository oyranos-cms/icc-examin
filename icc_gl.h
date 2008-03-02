// Author:    Kai-Uwe Behrmann
// Copyright: Kai-Uwe Behrmann <ku.b@gmx.de>
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
