//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Interface to grid solve method for computing LQRE
//

#ifndef NFGQREGRID_H
#define NFGQREGRID_H

#include "base/base.h"
#include "mixedsol.h"
#include "algutils.h"

class QreNfgGrid {
private:
  // Parameters
  double m_minLam, m_maxLam, m_delLam, m_delp1, m_delp2, m_tol1, m_tol2;
  int m_powLam;
  bool m_fullGraph;

  // Auxiliary functions
  gVector<double> UpdateFunc(const MixedProfile<double> &, int, double) const;
  bool CheckEqu(MixedProfile<double> &, double, int, double) const;

  void OutputHeader(const NFSupport &, gOutput &) const;
  void OutputResult(gOutput &, const MixedProfile<double> &,
		    double, double) const;

protected:
  // could use norms other then the simple one
  virtual double Distance(const gVector<double> &a,const gVector<double> &b) const;

public:
  // LIFECYCLE
  QreNfgGrid(void);

  // ACCESSING AND SETTING ALGORITHM PARAMETERS
  double GetMinLambda(void) const { return m_minLam; }
  void SetMinLambda(double p_minLam) { m_minLam = p_minLam; }

  double GetMaxLambda(void) const { return m_maxLam; }
  void SetMaxLambda(double p_maxLam) { m_maxLam = p_maxLam; }

  double GetDelLambda(void) const { return m_delLam; }
  void SetDelLambda(double p_delLam) { m_delLam = p_delLam; }

  double GetDelP1(void) const { return m_delp1; }
  void SetDelP1(double p_delp1) { m_delp1 = p_delp1; }

  double GetDelP2(void) const { return m_delp2; }
  void SetDelP2(double p_delp2) { m_delp2 = p_delp2; }

  double GetTol1(void) const { return m_tol1; }
  void SetTol1(double p_tol1) { m_tol1 = p_tol1; }

  double GetTol2(void) const { return m_tol2; }
  void SetTol2(double p_tol2) { m_tol2 = p_tol2; }

  int GetPowLambda(void) const { return m_powLam; }
  void SetPowLambda(int p_powLam) { m_powLam = p_powLam; }

  bool GetFullGraph(void) const { return m_fullGraph; }
  void SetFullGraph(bool p_fullGraph) { m_fullGraph = p_fullGraph; }

  // RUN THE ALGORITHM
  void Solve(const NFSupport &, gOutput &p_pxifile, gStatus &p_status,
	     gList<MixedSolution> &);
};

MixedProfile<double> LogitDynamics(const MixedProfile<double> &p_start,
				   double p_lambda, double p_tmax,
				   gOutput &);

#endif  // NFGQREGRID_H

