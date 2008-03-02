// Author:    Kai-Uwe Behrmann
// Copyright: Kai-Uwe Behrmann
// Date:      August 2004

#ifndef ICC_PROFILIERER_H
#define ICC_PROFILEIRER_H

#include <string>
#include <iostream>
#include <sstream>
#include <map>
#include <vector>
#include <fstream>

#include "icc_profile.h"

#define ICC_PROFILIERER_VERSION 0.01

// interne Funktionen

/**
  *   @brief Profilierer
  **/


class Profilierer {
  public:
                        Profilierer (); 
  private:
    ICCprofile          _profil; // ZIEL
    ICCmeasurement      _measurement;
    bool                check (ICCprofile& p);
  public:
    void                load  (ICCprofile& p);
    // erzeuge Matrixprofil
    const ICCprofile&   matrix();
  private:
    void                RGB_Tags (void);
    void                gemeinsamerHeader (ICCheader *header);
    std::string         print ();
    void                schreibKurveTag (icTagSignature name, double gamma);
    void                schreibKurveTag (icTagSignature name,
                                         std::vector<double> kurve);
    void                schreibXYZTag (icTagSignature name,
                                       double X, double Y, double Z);
    void                schreibTextTag (icTagSignature name, std::string text);
};


#endif //ICC_PROFILIERER_H
