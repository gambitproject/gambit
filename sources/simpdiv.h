//
// FILE: simpdiv.h -- Interface to Simpdiv solution module
//
// $Id$
//

#ifndef SIMPDIV_H
#define SIMPDIV_H

#include "base/base.h"
#include "nfg.h"
#include "algutils.h"
#include "mixedsol.h"

class SimpdivParams : public AlgParams {
public:
  int nRestarts, leashLength;
  
  SimpdivParams(void);
};

template <class T> class SimpdivModule  {
  private:
    const Nfg &N;
    const SimpdivParams &params;
    const NFSupport &support;

    MixedProfile<T> y;
    long leash;
    int t, nplayers, ibar, nevals, nits;
    T pay,d,maxz,bestz,mingrid;
    double time;
    gArray<int> nstrats,ylabel;
//    gVector<T> M;
    gRectArray<int> labels,pi;
    gPVector<int> U,TT;
    gPVector<T> ab,besty,v;

    gList<MixedSolution> solutions;

    T simplex(void);
    T getlabel(MixedProfile<T> &yy);
    void update(int j, int i);
    void getY(MixedProfile<T> &x,int k);
    void getnexty(MixedProfile<T> &x,int i);
    int get_c(int j, int h);
    int get_b(int j, int h);
  
  public:
    SimpdivModule(const NFSupport &N, const SimpdivParams &);
    virtual ~SimpdivModule();

    int NumEvals(void) const  { return nevals; }
    int NumIters(void) const  { return nits; }
    double Time(void) const   { return time; }

    int Simpdiv(gStatus &);
    const gList<MixedSolution> &GetSolutions(void) const  { return solutions; }
};

int Simpdiv(const NFSupport &, const SimpdivParams &,
	    gList<MixedSolution> &, gStatus &,
	    int &nevals, int &niters, double &time);


#endif    // SIMPDIV_H



