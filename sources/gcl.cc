#include "rational.h"
#include "gstring.h"
#include "glist.h"
#include "gsm.h"
#include "gcompile.h"

main()
{
  char buffer[256];
  int command = 1;

  GCLCompiler C;
  
  while (!C.quit)   {
    strcpy(buffer, "");
    gout << "GCL" << command++ << ": ";
    fgets(buffer, 255, stdin);
    if (feof(stdin))   break;
    C.Parse(buffer);
  }
}

