//
// FILE: algutils.h -- class definition for Algorithm specific classes
//

#ifndef ALGUTILS_H
#define ALGUTILS_H

#include "gstream.h"
#include "gstatus.h"
#include "gnumber.h"

class AlgParams   {
protected:
  gNumber m_accuracy;

public:
  int trace, stopAfter;
  gPrecision precision;

  gOutput *tracefile;
  gStatus &status;
  
  AlgParams(gStatus & = gstatus);
  virtual ~AlgParams();

  const gNumber &Accuracy(void) const { return m_accuracy; }
  virtual void SetAccuracy(const gNumber &);
};

class FuncMinParams : public AlgParams {
public:
  int maxits1, maxitsN;
  double tol1, tolN;
  
  FuncMinParams(gStatus & = gstatus);
  virtual ~FuncMinParams();

  void SetAccuracy(const gNumber &);
};

#endif // ALGUTILS_H

