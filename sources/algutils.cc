//
// FILE: algutils.cc -- Implementation of Algorithm specific classes
//

#include "algutils.h"

AlgParams::AlgParams(gStatus &s)
  : trace(0), stopAfter(0), accuracy(0), precision(precDOUBLE), 
    tracefile(&gnull), status(s)
{ }

AlgParams::~AlgParams()
{ }

FuncMinParams::FuncMinParams(gStatus &s)
  : AlgParams(), maxits1(100), maxitsN(20), tol1(2.0e-10), tolN(1.0e-10) 
{ }

FuncMinParams::~FuncMinParams()
{ }

void FuncMinParams::SetFuncMinParams(gNumber acc)
{ 
  accuracy = acc;
  maxits1 = maxitsN = 200;
  tol1 = tolN = acc *acc;
}

