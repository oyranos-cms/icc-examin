// Author:    Kai-Uwe Behrmann
// Copyright: Kai-Uwe Behrmann
// Date:      August 2004

#ifndef ICC_PROFILER_H
#define ICC_PROFILER_H

#include <string>
#include <iostream>
#include <sstream>
#include <map>
#include <vector>
#include <fstream>


using namespace std;

// interne Funktionen

const char* cp_nchar (char* text, int n);

/**
  *   @brief Profilierer
  **/


class Parser {
  public:
                        Parser (); 
  private:

  public:
    void                load  (void*);

    std::string         print ();
};


#endif //ICC_PROFILER_H
