#include "icc_formeln.h"
#include "icc_utils.h"
#include "ciede2000testdata.h"

int level_PROG;


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

  DBG_PROG_ENDE
  return 0;
}
