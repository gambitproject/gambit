//#
//# FILE: simpdiv.h -- Interface to Simpdiv solution module
//#
//# $Id$
//#

#ifndef SIMPDIV_H
#define SIMPDIV_H

#include "normal.h"
#include "glist.h"
#include "grarray.h"
#include "gsignal.h"

class SimpdivParams     {
  public:
    int plev, number, ndivs, leash;
    gOutput *output;
    gSignal &sig;
    
    SimpdivParams(void);
};

template <class T> class SimpdivModule  {
  private:
    const NormalForm<T> &rep;
    const SimpdivParams &params;

    long leash;
    int t, nplayers, ibar, nevals, nits;
    T pay,d,maxz,bestz,mingrid;
    gTuple<int> nstrats,ylabel;
    gVector<T> M;
    gRectArray<int> labels,pi;
    gPVector<int> U,TT;
    gPVector<T> ab,y,besty,v;

    T simplex(void);
    T getlabel(gPVector<T> &yy);
    void update(int j, int i);
    void getY(gPVector<T> &x,int k);
    void getnexty(gPVector<T> &x,int i);
    int get_c(int j, int h);
    int get_b(int j, int h);
  
  public:
    SimpdivModule(const NormalForm<T> &N, const SimpdivParams &);
    virtual ~SimpdivModule();

    int NumEvals(void) const  { return nevals; }
    int NumIters(void) const  { return nits; }
    double Time(void) const   { return 0.0; }

    int Simpdiv(gList<gPVector<T> > &);
};



#endif    // SIMPDIV_H



