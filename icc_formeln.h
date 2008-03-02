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
 * interne Typen und Formeln
 * 
 */

// Date:      September 2004

#ifndef ICC_FORMELN_H
#define ICC_FORMELN_H

#define ICC_EXAMIN_V 0.14
#define ICC_EXAMIN_D _("November 2004")

#include <string>
#include <iostream>
#include <sstream>
#include <map>
#include <vector>
#include <fstream>
#include "icc_utils.h"



// interne Typen
typedef struct {
    double X;
    double Y;
    double Z;
} XYZ;

typedef struct {
    double L;
    double a;
    double b;
} Lab;

typedef struct {
    double R;
    double G;
    double B;
} RGB;

typedef struct {
    double C;
    double M;
    double Y;
    double K;
} CMYK;

// Umwandlungen
void FarbeZuDouble (double* d_xyz, XYZ xyz);
void FarbeZuDouble (double* d_rgb, RGB rgb);
void FarbeZuDouble (double* d_lab, Lab lab);
void FarbeZuDouble (double* d_cmyk, CMYK cmyk);
void FarbeZuDouble (XYZ *xyz, double* d_xyz);
void FarbeZuDouble (RGB *rgb, double* d_rgb);
void FarbeZuDouble (Lab *lab, double* d_lab);
void FarbeZuDouble (CMYK *cmyk, double* d_cmyk);


// Farbfunktionen
double        dE2000 (Lab Lab1, Lab Lab2, double kL, double kC, double kH);


#endif //ICC_FORMELN_H
