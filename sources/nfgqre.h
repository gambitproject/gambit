//
// FILE: nfgqre.h -- Interface to computation of QRE correspondence
//
// $id$
//

#ifndef NQRE_H
#define NQRE_H

#include "base/base.h"
#include "algutils.h"
#include "corresp.h"

#include "nfg.h"
#include "mixedsol.h"

typedef enum {
  qreOPTIMIZE = 0, qreHOMOTOPY = 1
} qreAlgorithmType;  

class NFQreParams : public FuncMinParams {
public:
  qreAlgorithmType m_method;
  int powLam;
  double minLam, maxLam, delLam;
  bool fullGraph;

  NFQreParams(void);
};


void Qre(const Nfg &, NFQreParams &, gOutput &,
	 const MixedProfile<gNumber> &,
	 Correspondence<double, MixedSolution> &, gStatus &,
	 long &nevals, long &nits);


void KQre(const Nfg &N, NFQreParams &params, gOutput &,
	    const MixedProfile<gNumber> &start,
	    gList<MixedSolution> &solutions, gStatus &,
	    long &nevals, long &nits);

#endif    // NQRE_H



