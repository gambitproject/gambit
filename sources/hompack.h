//
// FILE: hompack.h -- Hompack code, translated by David Bustos, 6/25/99
//
// $Id$
//

#ifndef HOMPACK_H
#define HOMPACK_H

// Includes //////////////////////////////////////////////////////////////
#include "base/base.h"
#include "gvector.h"
#include "gmatrix.h"
#include "stdio.h"
#include "homotopy.h" // needed for HomQreParams

// User Functions ////////////////////////////////////////////////////////
template <class T> class gHompack {
private: 
  const Nfg &nfg;
  const NFSupport &supp;
  const MixedProfile<gNumber> &start;
  HomQreParams params;
  gList<MixedSolution> solutions;

  const int limitd, Newton_iter_limit;
  const double curvature_limit;

  /*
    // Why doesn't this work ?? 
  static gVector<T> *Ytan_p;    // YP = tangent vector to zero curve at Y
  static gVector<T> *Y_old_p;   // Yold = previous point on the zero curve
  static gVector<T> *Ytan_old_p; // YPold = previous tangent vector
  */
public: 
  gHompack(const MixedProfile<gNumber> &s, const HomQreParams &p);
  virtual ~gHompack(void);

  void F(const gVector<T> &X, gVector<T> &V );
  void Fjac(const gVector<T> &X, gVector<T> &V, int K );
  void rho(const gVector<T> &A, const T lambda,
	   const gVector<T> &X, gVector<T> &V);
  void rhojac(const gVector<T> &A, const T lambda,
	      const gVector<T> &X, gVector<T> &V, int K );
  void fixpnf(int N, gVector<T> &Y, int &flag, T rel_arc_err,
	      T abs_arc_err, T rel_ans_err, T abs_ans_err,
	      bool trace, gVector<T> &A, gArray<T> &stepsize_params,
	      int &jeval_num, T &arclength,T max_lambda,
	      bool poly_switch);
  void stepnf(int N, int &jeval_num, int &flag,
	      bool &start, bool &crash,
	      T &old_step, T &step_size,
	      T &rel_err, T &abs_err, T &arclength,
	      gVector<T> &Y, gVector<T> &Ytan,
	      gVector<T> &Y_old, gVector<T> &Ytan_old,
	      const gVector<T> &A,
	      const gArray<T> &stepsize_params
	      );
  void rootnf(int N, 
	      int &jevalcount, int &flag,
	      T relerr, T abserr,
	      gVector<T> &Y,
	      gVector<T> &Ytan,
	      gVector<T> &Y_old,
	      gVector<T> &Ytan_old,
	      const gVector<T> &A
	      );
  void root(T &t, T &F_of_t, T &b, T &c,
	    T relerr, T abserr, int &flag
	    );
  void show_probs(char *msg,const gVector<T> &Y);
  bool out_of_bounds(gVector<T> &W,int range);
  void build_prediction(gVector<T> &W,int n, gVector<T> &Y_old,
			gVector<T> &Ytan_old,gVector<T> &Y,
			gVector<T> &Ytan,T old_step,T old_step_plus_step_size
			);
  void tangnf(T &rholen, 
	      const gVector<T> &Y,
	      gVector<T> &Ytan,
	      const gVector<T> &Ytan_old,
	      const gVector<T> &A,
	      gVector<T> &Newton_step,
	      int &jevalcount, int N, int &flag
	      );

  T qofs(T f0, T fp0, T f1, T fp1, T dels, T s);
  void cleanup(void);
  const gList<MixedSolution> &GetSolutions(void) const;
  T QreValue(MixedProfile<T> &sol, T lambda);
};

#endif // HOMPACK_H
