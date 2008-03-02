// Author:    Kai-Uwe Behrmann
// Copyright: Kai-Uwe Behrmann <ku.b@gmx.de>
// Date:      12. 09. 2004


#ifndef ICC_GL_H
#define ICC_GL_H
#define _(text) text

class GL_Ansicht : public Fl_Widget {
  int X; int Y; int W; int H; std::vector<std::string>texte; std::vector<double>punkte; std::vector<std::vector<double> >kurven;
public:
  int wiederholen; bool first;
  GL_Ansicht(int X,int Y,int W,int H) ;
  void init();
  void myGLinit();
  void MakeDisplayLists();
  void MenuInit();
  void display();
  void draw();
  void hinein_punkt(std::vector<double> vect, std::vector<std::string> txt);
  void hinein_kurven(std::vector<std::vector<double> >vect, std::vector<std::string> txt);
  void ruhig_neuzeichnen(void);
};

void reshape(int w, int h);

#endif //ICC_GL_H
