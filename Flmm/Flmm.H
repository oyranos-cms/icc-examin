//
// "$Id:$"
//
// Main header file for the FLMM extension to FLTK.
//
// Copyright 2002-2004 by Matthias Melcher.
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Library General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Library General Public License for more details.
//
// You should have received a copy of the GNU Library General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
// USA.
//
// Please report all bugs and problems to "flmm@matthiasm.com".
//

#ifndef Flmm_H
#  define Flmm_H

#include <FL/Fl_Export.H>

#define FLMM_MAJOR_VERSION        1
#define FLMM_MINOR_VERSION        1
#define FLMM_PATCH_VERSION        6
#define FLMM_VERSION              ((double)FLMM_MAJOR_VERSION + \
                                   (double)FLMM_MINOR_VERSION * 0.01 + \
                                   (double)FLMM_PATCH_VERSION * 0.0001)

typedef unsigned char uchar;
typedef unsigned long ulong;

class FL_EXPORT Flmm {

private:
                  Flmm() {}; // Constructor is private.

public:
  static double   version();
  static void     set_plastic_scheme();
  static char     is_plastic_scheme();

private:
  static char     pPlasticScheme;

};

#endif // !Flmm_H

//
// End of "$Id:$".
//
