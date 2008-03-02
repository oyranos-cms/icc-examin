// Author:    Kai-Uwe Behrmann
// Copyright: Kai-Uwe Behrmann
// Date:      Mai 2004

#ifndef ICC_DRAW_H
#define ICC_DRAW_H

void draw_kurve    (int X, int Y, int W, int H,
                    std::vector<std::string>          texte,
                    std::vector<std::vector<double> > kurven);

void draw_cie_shoe (int X, int Y, int W, int H,
                    std::vector<std::string> texte,
                    std::vector<double>      punkte,
                    int                      repeated);

#endif //ICC_DRAW_H

