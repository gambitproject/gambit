//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Implementation of (agent) QRE computation for extensive forms
//

#include <math.h>
#include "base/gnullstatus.h"
#include "math/gmath.h"
#include "math/gsmatrix.h"
#include "numerical/gfunc.h"
#include "efgqre.h"

static void WritePXIHeader(gOutput &pxifile, const Efg::Game &E)
{
  int pl, iset, nisets = 0;

  pxifile << "Dimensionality:\n";
  for (pl = 1; pl <= E.NumPlayers(); pl++)
    nisets += E.Players()[pl]->NumInfosets();
  pxifile << nisets;
  for (pl = 1; pl <= E.NumPlayers(); pl++)
    for (iset = 1; iset <= E.Players()[pl]->NumInfosets(); iset++)
      pxifile << " " << E.Players()[pl]->Infosets()[iset]->NumActions();
  pxifile << "\n";

  pxifile << "Settings:\n" << 0.0;
  pxifile << "\n" << 1000.0 << "\n" << 1.05;
  pxifile << "\n" << 0 << "\n" << 1 << "\n" << 1 << "\n";

  int numcols = E.ProfileLength() + 2;
  pxifile << "DataFormat:";
  pxifile << "\n" << numcols;
  for (int i = 1; i <= numcols; i++)
    pxifile << ' ' << i;
  pxifile << "\nData:\n";
}

extern void Project(gVector<double> &, const gArray<int> &);


//=========================================================================
//             QRE Correspondence Computation via Homotopy
//=========================================================================

static void QreJacobian(const Efg::Game &p_efg,
			const BehavProfile<double> &p_profile,
			const double &p_lambda, gMatrix<double> &p_matrix)
{
  p_matrix = (double) 0;

  int rowno = 0;  // indexes the row number in the Jacobian matrix
  for (int pl1 = 1; pl1 <= p_efg.NumPlayers(); pl1++) {
    EFPlayer *player1 = p_efg.Players()[pl1];

    for (int iset1 = 1; iset1 <= player1->NumInfosets(); iset1++) {
      Infoset *infoset1 = player1->Infosets()[iset1];
      for (int act1 = 1; act1 <= infoset1->NumActions(); act1++) {
	rowno++;

	int colno = 0;
	for (int pl2 = 1; pl2 <= p_efg.NumPlayers(); pl2++) {
	  EFPlayer *player2 = p_efg.Players()[pl2];
	  for (int iset2 = 1; iset2 <= player2->NumInfosets(); iset2++) {
	    Infoset *infoset2 = player2->Infosets()[iset2];

	    for (int act2 = 1; act2 <= infoset2->NumActions(); act2++) {
	      colno++;
	      if (infoset1 == infoset2) {
		if (act1 == act2) {
		  p_matrix(rowno, colno) = 1.0;
		}
		else {
		  p_matrix(rowno, colno) = 0.0;
		}
	      }
	      else {   // infoset1 != infoset2
		for (int k = 1; k <= infoset1->NumActions(); k++) {
		  p_matrix(rowno, colno) += (p_profile.DiffActionValue(infoset1->Actions()[k], infoset2->Actions()[act2]) - p_profile.DiffActionValue(infoset1->Actions()[act1], infoset2->Actions()[act2])) * p_profile.GetActionProb(infoset1->Actions()[k]);
		}
		p_matrix(rowno, colno) *= p_lambda * p_profile.GetActionProb(infoset1->Actions()[act1]);
	      }
	    }
	  }
	}

	// Now for the column wrt lambda
	for (int k = 1; k <= infoset1->NumActions(); k++) {
	  p_matrix(rowno, p_matrix.NumColumns()) += (p_profile.GetActionValue(infoset1->Actions()[k]) - p_profile.GetActionValue(infoset1->Actions()[act1])) * p_profile.GetActionProb(infoset1->Actions()[k]);
	}
	p_matrix(rowno, p_matrix.NumColumns()) *= p_profile.GetActionProb(infoset1->Actions()[act1]);
      }
    }
  }
}

static void QreComputeStep(const Efg::Game &p_efg, 
			   const BehavProfile<double> &p_profile,
			   const gMatrix<double> &p_matrix,
			   gDPVector<double> &p_delta, double &p_lambdainc,
			   double p_initialsign, double p_stepsize)
{
  double sign = p_initialsign;
  int rowno = 0; 

  gSquareMatrix<double> M(p_matrix.NumRows());

  for (int row = 1; row <= M.NumRows(); row++) {
    for (int col = 1; col <= M.NumColumns(); col++) {
      M(row, col) = p_matrix(row, col + 1);
    }
  }

  for (int i = 1; i <= p_delta.Length(); i++) {
    rowno++;
    p_delta[i] = sign * M.Determinant();   
    sign *= -1.0;

    for (int row = 1; row <= M.NumRows(); row++) {
      M(row, rowno) = p_matrix(row, rowno);
      if (rowno < M.NumColumns()) {
	M(row, rowno + 1) = p_matrix(row, rowno + 2);
      }
    }
  }   

  p_lambdainc = sign * M.Determinant();

  double norm = 0.0;
  for (int i = 1; i <= p_delta.Length(); i++) {
    norm += p_delta[i] * p_delta[i];
  }
  norm += p_lambdainc * p_lambdainc; 

  for (int i = 1; i <= p_delta.Length(); i++) {
    p_delta[i] /= sqrt(norm / p_stepsize);
  }

  p_lambdainc /= sqrt(norm / p_stepsize);
}

efgQre::efgQre(void)
  : m_maxLam(30.0), m_stepSize(0.0001), m_fullGraph(false)
{ }

gList<BehavSolution> efgQre::Solve(const EFSupport &p_support,
				   gStatus &p_status)
{
  gList<BehavSolution> solutions;

  const Efg::Game &efg = p_support.GetGame();
  gMatrix<double> H(efg.ProfileLength(), efg.ProfileLength() + 1);
  BehavProfile<double> profile(efg);
  double lambda = 0.0;
  int numSteps = 0;

  WritePXIHeader(gnull, efg);

  // Pick the direction to follow the path so that lambda starts out
  // increasing
  double initialsign = (efg.ProfileLength() % 2 == 0) ? 1.0 : -1.0;

  try {
    while (lambda <= m_maxLam) {
      // Use a first-order Runge-Kutta style method
      gDPVector<double> delta1(profile.GetDPVector());
      gDPVector<double> delta2(profile.GetDPVector());
      double lambdainc1, lambdainc2;

      QreJacobian(efg, profile, lambda, H);
      QreComputeStep(efg, profile, H,
		     delta1, lambdainc1, initialsign, m_stepSize);

      BehavProfile<double> profile2(profile);
      delta1 *= 0.5;
      profile2 += delta1;
      QreJacobian(efg, profile2, lambda + lambdainc1 * 0.5, H);
      QreComputeStep(efg, profile, H,
		     delta2, lambdainc2, initialsign, m_stepSize);

      profile += delta1;
      delta2 *= 0.5;
      profile += delta2; 
      lambda += 0.5 * (lambdainc1 + lambdainc2);

      // Write out the QreValue as 0 in the PXI file; not generally
      // going to be the case, but QreValue is suspect for large lambda 
      // p_pxifile << "\n" << lambda << " " << 0.0 << " ";
      // for (int i = 1; i <= profile.Length(); i++) {
      //   p_pxifile << profile[i] << " ";
      // }
 
      if (m_fullGraph) { 
	solutions.Append(BehavSolution(profile, algorithmEfg_QRE_EFG));
	solutions[solutions.Length()].SetQre(lambda, 0);
	//	solutions[solutions.Length()].SetEpsilon(params.Accuracy());
      }

      if (numSteps++ % 50 == 0) {
	p_status.Get();
	p_status.SetProgress(lambda / m_maxLam,
			     gText("Current lambda: ") + ToText(lambda));
      }
    }
  }
  catch (...) {
    return solutions;
  }

  if (!m_fullGraph) {
    solutions.Append(BehavSolution(profile, algorithmEfg_QRE_EFG));
    solutions[solutions.Length()].SetQre(lambda, 0);
    // This doesn't really apply, at least currently...
    //    solutions[solutions.Length()].SetEpsilon(params.Accuracy());
  }
  return solutions;
}

