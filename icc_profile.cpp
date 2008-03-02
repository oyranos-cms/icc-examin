// Author:    Kai-Uwe Behrmann
// Copyright: Kai-Uwe Behrmann <ku.b@gmx.de>
// Date:      04. 05. 2004

#include <iostream>
#include "icc_profile.h"


ICCprofile::ICCprofile (void)
{
  //header = new ICCheader();
  
}

ICCprofile::ICCprofile (const char *filename)
  : _filename (filename)
{
  // ICC Profil laden
  //profile_name  (char *filename);
}

ICCprofile::~ICCprofile (void)
{
  ;
}

