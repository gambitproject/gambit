//
// FILE: algutils.cc -- Implementation of Algorithm specific classes
//

#include "algutils.h"

AlgParams::AlgParams(gStatus &s)
  : m_accuracy(0), trace(0), stopAfter(0), precision(precDOUBLE), 
    tracefile(&gnull), status(s)
{ }

AlgParams::~AlgParams()
{ }

void AlgParams::SetAccuracy(const gNumber &p_accuracy)
{ m_accuracy = p_accuracy; }


FuncMinParams::FuncMinParams(gStatus &s)
  : AlgParams(s), maxits1(100), maxitsN(20), tol1(2.0e-10), tolN(1.0e-10) 
{ }

FuncMinParams::~FuncMinParams()
{ }

void FuncMinParams::SetAccuracy(const gNumber &p_accuracy)
{ 
  m_accuracy = p_accuracy;
  maxits1 = maxitsN = 200;
  tol1 = tolN = p_accuracy * p_accuracy;
}

