#include <cstdlib>
#include <core/logging.h>

//------------------------------------------------------------------------

FILE* logFile;

//------------------------------------------------------------------------

static void AtExit()
{
  fclose(logFile);
}

//------------------------------------------------------------------------

void Logging::Initialise()
{
  logFile = fopen("stderr.txt", "wb");
  atexit(AtExit);
}
