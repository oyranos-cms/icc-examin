// Author:    Kai-Uwe Behrmann
// Copyright: Kai-Uwe Behrmann
// Date:      Mai 2004

#include "icc_examin.h"
#include "icc_vrml.h"
#include <stdio.h>
#include <iostream>
#include <string.h>
#include <FL/Fl.H>


void dump_vrml_header (char *vrml);

int
erase_file (char *file)
{
  FILE *fp;

  fp = fopen (file, "r");
  if (fp) {
  fclose (fp);
  remove (file);
  return (1);
  }

  return (0);
}

#define lp {l+=0.1; load_progress->value(l); details->flush();}
int 
create_vrml              ( const char *profilA, char *profilB, char *vrml)
{
  char systemBefehl[1024];
  float l = load_progress->value();

  if (!vrml || (!profilA && !profilB))
  return (0);

  lp
  erase_file (vrml); lp
  dump_vrml_header (vrml); lp

  // gamut A
  if (profilA) {
  if (!erase_file ("/tmp/tmpA.icc")) remove ("/tmp/tmpA.icc");
  sprintf (systemBefehl, "ln -s \"%s\" /tmp/tmpA.icc", profilA);
  system (systemBefehl);
  system ("iccgamut -n -w -d 6.0 /tmp/tmpA.icc"); lp
  erase_file ("/tmp/tmpA.wrl");
  erase_file ("/tmp/tmpA.icc");
  system ("viewgam -n -c n /tmp/tmpA.gam /tmp/tmp.wrl");
  sprintf (systemBefehl ,"cat /tmp/tmp.wrl >> \"%s\"", vrml); lp
  system (systemBefehl);
  erase_file ("/tmp/tmp.wrl");
  }

  // gamut B transparent
  if (profilB) {
  if (!erase_file ("/tmp/tmpB.icc")) remove ("/tmp/tmpB.icc");
  sprintf (systemBefehl, "ln -s \"%s\" /tmp/tmpB.icc", profilB);
  system (systemBefehl);
  system ("iccgamut -n -w -d 6.0 /tmp/tmpB.icc"); lp
  erase_file ("/tmp/tmpB.wrl");
  erase_file ("/tmp/tmpB.icc");
  system ("viewgam -n -t 0.5 -c w /tmp/tmpB.gam /tmp/tmp.wrl"); lp
  sprintf (systemBefehl ,"cat /tmp/tmp.wrl >> \"%s\"", vrml);
  system (systemBefehl);
  erase_file ("/tmp/tmp.wrl");
  }

  // Unterschiede
  if (profilA && profilB) {
  system ("smthtest /tmp/tmpA.gam /tmp/tmpB.gam /tmp/tmp.wrl"); lp
  sprintf (systemBefehl ,"cat /tmp/tmp.wrl >> \"%s\"", vrml);
  system (systemBefehl);
  erase_file ("/tmp/tmp.wrl");
  erase_file ("/tmp/tmpA.gam");
  erase_file ("/tmp/tmpB.gam");
  }

  return (1);  
}

void
dump_vrml_header (char *vrml)
{
  FILE *fp;
  char vrml_text[] = "#VRML V2.0 utf8 \nTransform { children [ DirectionalLight { color 0.500000 0.500000 0.500000 } Viewpoint { position 0 0 255 } ] } Transform { children [ NavigationInfo { avatarSize [ 0.250000 1.600000 0.750000 ] type [ \"EXAMINE\" ] } Transform { children [ Shape { appearance 	  Appearance { material 	    Material { diffuseColor 0.495000 0 0 } } geometry 	  Box { size 100 2 2 } } ] scale 1 0.500000 0.500000 translation 50 0 -50 } Transform { children [ Shape { appearance 	  Appearance { material 	    Material { diffuseColor 0 0 0.504998 } } geometry 	  Box { size 2 100 2 } } ] scale 0.500000 1 0.500000 translation 0 -50 -50 } Transform { children [ Shape { appearance 	  Appearance { material 	    Material { diffuseColor 0 0.504998 0 } } geometry 	  Box { size 100 2 2 } } ] scale 1 0.500000 0.500000 translation -50 0 -50 } Transform { children [ Shape { appearance 	  Appearance { material 	    Material { diffuseColor 1 1 0 } } geometry 	  Box { size 2 100 2 } } ] scale 0.500000 1 0.500000 translation 0 50 -50 } Shape { appearance       Appearance { material 	Material { diffuseColor 0.699998 0.699998 0.699998 } } geometry       Box { size 1 1 100 } } ] } Group { children [ Transform { children [ Billboard { axisOfRotation 0 0 0 children [ Shape { appearance 	      Appearance { material 		Material { } } geometry 	      Text { string [ \"*L=100\" ] } } ] } ] scale 5 5 5 translation 0 0 52 } Transform { children [ Billboard { axisOfRotation 0 0 0 children [ Shape { appearance 	      Appearance { material 		Material { } } geometry 	      Text { string [ \"*L= 0\" ] } } ] bboxSize 1 1 1 } ] scale 5 5 5 translation 0 0 -57 } ] } Viewpoint { fieldOfView 0.790000 orientation 0.999990 1.788147e-3 -3.940986e-3 1.515311 position 0 -340 20 } Background { groundColor [ 0.500000 0.500000 0.500000 ] skyColor [ 0.500000 0.500000 0.500000 ] } ";

  int text_len = strlen (vrml_text);
  fp = fopen (vrml, "w+");
  if (fp) {
    for (int i = 0 ; i < text_len ; i++ )
      fputc ( vrml_text[i] , fp);

    fclose (fp);
  }
}


