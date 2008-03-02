#include "icc_examin.h"
#include "icc_fenster.h"

namespace icc_examin_ns {

void
fortschritt(const double f)
{
  icc_examin->fortschritt(f);
}

void
info(const char* text)
{
  nachricht(text);
}

void
status_info(const char* text)
{
  icc_examin->statlabel = text;
  icc_examin->statusAktualisieren();
}

int
laeuft()
{
  return icc_examin->laeuft();
}

}
