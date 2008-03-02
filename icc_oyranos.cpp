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
 * Der CMS Sortierer.
 * 
 */

// Date:      25. 11. 2004


#include "icc_oyranos.h"

#include <kdb.h>

void
oyranos_pfade_loeschen()
{
}

void
oyranos_pfade_auffrischen()
{
}

void
oyranos_pfade_einlesen()
{
}

void
oyranos_pfad_dazu (char* pfad)
{
  KeySet myConfig;
  ksInit(&myConfig);
  kdbOpen();

  int rc = kdbGetChildKeys("user/sw/oyanos/paths", &myConfig, KDB_O_RECURSIVE);
	
  /* Close the Key database */
  kdbClose();

  Key *current;
	
  for (current=myConfig.start; current; current=current->next) {
    char keyName[200];
    char value[300];
		
    keyGetFullName(current,keyName,sizeof(keyName));
    keyGetString(current,value,sizeof(value));
		
    printf("Key %s was %s. ", keyName, value);
		
    /* Add "- modified" to the end of the string */
    //strcat(value,"- modified");
    //sprintf (value, pfad);
		
    /* change the key value */
    keySetString(current,value);
		
    /* reget it, just as an example */
    keyGetString(current,value,sizeof(value));
		
    printf("Now is %s\n", value);
  }

  kdbClose();
}


