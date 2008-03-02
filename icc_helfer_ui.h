/*
 * ICC Examin ist eine ICC Profil Betrachter
 * 
 * Copyright (C) 2004  Kai-Uwe Behrmann 
 *
 * Autor: Kai-Uwe Behrmann <ku.b@gmx.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * -----------------------------------------------------------------------------
 *
 * ICC Helfer - werkzeugabhaengig
 * 
 */

// Date:      Mai 2004

#ifndef ICC_HELFER_UI_H
#define ICC_HELFER_UI_H

#include "icc_icc.h"
#if HAVE_FLTK
# include "icc_helfer_fltk.h"
#endif
#if HAVE_X || APPLE
# include "icc_helfer_x.h"
#endif

#endif //ICC_HELFER_UI_H
