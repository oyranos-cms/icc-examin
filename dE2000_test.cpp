#include "icc_formeln.h"
#include "icc_utils.h"
#include "ciede2000testdata.h"


int
main ()
{
  DBG_PROG_START

  Lab quelle, ziel; 
  double de00;

  for (int i = 0 ; i < 34; i++) {
    FarbeZuDouble (&quelle, &cietest[i][0]);
    FarbeZuDouble (&ziel, &cietest[i][3]);
    de00 =  dE2000(quelle, ziel, 1.0,1.0,1.0);
    DBG_NUM_S( i << ": " << de00 << " - " << cietest[i][6] << " = " << de00 - cietest[i][6] )
  }

  quelle.L =  54.44073903;
  quelle.a = -34.82222743;
  quelle.b =  0.18986800;
  ziel.L =  54.92340686;
  ziel.a = -33.45703125;
  ziel.b =  0.00000000;
    de00 =  dE2000(quelle, ziel, 2.0,1.0,1.0);
    DBG_NUM_S( i << ": " << de00 << " - " << "Test = " << de00 )
  DBG_PROG_ENDE
  return 0;
}
