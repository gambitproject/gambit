#include "gmisc.h"

// Find String In File -- uses a file pointer to find a string in the file.
// This is useful for quickly getting to the section of the file you need
int FindStringInFile(gInput &in,const char *s)
{
  char fsif_str[200];
  in.seekp(0L);	// go to the start of the file
  do {
      in>>fsif_str;
  } while (strcmp(fsif_str,s)!=0 && !in.eof() && in.IsValid());
  if (in.eof() || !in.IsValid()) return 0; else return 1;
}

