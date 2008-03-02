#include "icc_examin.h"
#include "icc_fenster.h"

namespace icc_examin_ns {

void
fortschritt(const double f)
{
  icc_examin->fortschrittThreaded(f);
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
  return icc_examin->laeuft();
}

}
