//
// FILE: algutils.h -- class definition for Algorithm specific classes
//

#ifndef ALGUTILS_H
#define ALGUTILS_H

#include "gstream.h"
#include "gstatus.h"
#include "gnumber.h"

class AlgParams   {
public:
  int trace;
  gNumber accuracy;
  gOutput *tracefile;
  gStatus &status;
  
  AlgParams(gStatus & = gstatus);
  virtual ~AlgParams();
};

#endif // ALGUTILS_H

