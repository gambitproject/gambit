//
// FILE: seqform.h -- Interface to Sequence Form solution module
//
// $Id$
//

#ifndef SEQFORM_H
#define SEQFORM_H

#include "glist.h"
#include "gstatus.h"
#include "behavsol.h"

class SeqFormParams     {
  public:
    int trace, stopAfter, maxdepth;
    gPrecision precision;
    gOutput *tracefile;
    gStatus &status;
    
    SeqFormParams(gStatus &status_ = gstatus);
};

#include "subsolve.h"

class SeqFormBySubgame : public SubgameSolver  {
  private:
    int npivots;
    SeqFormParams params;
    gArray<gNumber> values;

    int SolveSubgame(const Efg &, const EFSupport &,
		     gList<BehavSolution> &);
    EfgAlgType AlgorithmID() const { return EfgAlg_SEQFORMSUB; }    

  public:
    SeqFormBySubgame(const EFSupport &, const SeqFormParams &, int max = 0);
    virtual ~SeqFormBySubgame();

    int NumPivots(void) const  { return npivots; }
};

int SeqForm(const EFSupport &, const SeqFormParams &,
	    gList<BehavSolution> &, int &npivots, double &time);

#endif    // SEQFORM_H





