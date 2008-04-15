//
// "$Id:$"
//
// Main source file for the FLMM extension to FLTK.
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

#include <Flmm.H>

#ifdef DEBUG
#  include <stdio.h>
#endif // DEBUG

/** \mainpage Flmm FLTK MatthiasM extensions
 *
 * Home page, download and documentation at http://fltk.matthiasm.com/ 
 *
 * \section About
 * Flmm is a collection of new and improved widgets for the FLTK toolkit.
 * Some widgets make life easier or just look better (Flmm_Tabs), some
 * have greatly improve functionality (Flmm_Message). However, all 
 * widgets and their API are kept in the same style as FLTK. Sometimes,
 * they are simply drop-in replacements for the original Widget.
 *
 * \section Platforms
 * Currenly supported platforms are MSWindows (Windows 2000 and higher),
 * Linux (tested under Ubuntu), and Mac OS X (tested on 10.4 PPC).
 *
 * \section License
 * Copyright 2002-2006 by Matthias Melcher.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA.
 *
 * Permission is granted to reproduce this manual or any portion for 
 * any purpose, provided this copyright and permission notice are preserved.
 *
 * Please report all bugs and problems to "flos@matthiasm.com".
 *
 */

/** \class Flmm
 * The Flmm class returns information about the Flmm extension to FLTK.
 *
 * This class is used to return the version number of FLMM.
 */

/**
 * Return the version number of Flmm.
 *
 * The version number is encoded as a double float value where the ones
 * give the major version, the 100th are the minor version, and the 
 * 10000th are the patch version.
 * \return version number in the format v.mmpp (v1.1.6 would be 
 *         returned as 1.0106)
 */
double Flmm::version() {
  return FLMM_VERSION;
}

/**
 * If this flag is set, all dialogs are created using a plastic scheme.
 */
void Flmm::set_plastic_scheme()
{
  pPlasticScheme = 1;
}

/**
 * Is Flmm operating in plastic scheme?
 *
 * \return 0 if default scheme, 1 if plastic scheme.
 */
char Flmm::is_plastic_scheme()
{
  return pPlasticScheme;
}


char Flmm::pPlasticScheme = 0;

//
// End of "$Id:$".
//
