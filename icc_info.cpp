#include "icc_examin.h"
#include "icc_fenster.h"

namespace icc_examin_ns {

double
fortschritt()
{
  return icc_examin->fortschritt();
}

void
fortschritt(const double f)
{
  icc_examin->fortschritt(f,1.0);
}

void
fortschritt(const double f, const double anteil)
{
  icc_examin->fortschritt(f,anteil);
}

void
info(const char* text)
{
  nachricht(text);
}

void
status_info(const char* text, int fenster)
{
  icc_examin->statlabel[fenster] = text;
  icc_examin->statusAktualisieren();
}

int
laeuft()
{
  return icc_examin && icc_examin->laeuft();
}

}
