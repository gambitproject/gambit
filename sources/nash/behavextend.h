//
// FILE: behavextend.h -- Algorithms for extending (incomplete) behavior
//                        profiles to Nash equilibria
//
// $Id$
//

#ifndef BEHAVEXTEND_H
#define BEHAVEXTEND_H

#include "behavsol.h"

class algExtendsToNash {
public:
  bool ExtendsToNash(const BehavSolution &p_solution,
		     const EFSupport &p_littleSupport,
		     const EFSupport &p_bigSupport,
		     gStatus &p_status);
};

class algExtendsToAgentNash {
public:
  bool ExtendsToAgentNash(const BehavSolution &p_solution,
			  const EFSupport &p_littleSupport,
			  const EFSupport &p_bigSupport,
			  gStatus &p_status);
};



#endif   // BEHAVEXTEND_H
