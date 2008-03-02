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
    ICCprofile          matrix();

    std::string         print ();
};


#endif //ICC_PROFILIERER_H
