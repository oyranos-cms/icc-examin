/*
 * ICC Examin ist eine ICC Profil Betrachter
 * 
 * Copyright (C) 2004-2005  Kai-Uwe Behrmann 
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
 * Der CMS Sortierer.
 * 
 */

// Date:      25. 11. 2004

#include "icc_utils.h"
#include "icc_fenster.h"

using namespace icc_examin_ns;

void
oyranos_einstellungen()
{
  DBG_PROG_START
  bool hat_oy, hat_flu;
  #if HAVE_OY
  hat_oy = true;
  #else
  hat_oy = false;
  #endif
  #if HAVE_FLU
  hat_flu = true;
  #else
  hat_flu = false;
  #endif
  int fehl = system("oyranos_config_flu");
  if(fehl) {
    if(hat_oy && hat_flu)
      nachricht(_("Konnte\nOyranos Konfigurationsdialog\nnicht im aktuellen\nAusfuehrungspfad finden."));
    if(hat_oy && !hat_flu)
      nachricht(_("FLU ist nicht installiert\nund wird benoetigt von\nOyranos Konfigurationsdialog"));
    if(!hat_oy && !hat_flu)
      nachricht(_("Oyranos und FLU sind nicht installiert\nund wird benoetigt von\nOyranos mit seinem Konfigurationsdialog"));
    if(!hat_oy && hat_flu)
      nachricht(_("Oyranos ist nicht installiert\nund wird benoetigt von\nOyranos mit seinem Konfigurationsdialog"));
  }
  DBG_PROG_ENDE
}

