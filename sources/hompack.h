// hompack.h by David Bustos, 6/25/99

#ifndef HOMPACK_H
#define HOMPACK_H

// Includes //////////////////////////////////////////////////////////////
#include "garray.h"
#include "gvector.h"
#include "gmatrix.h"
#include "stdio.h"

// User Functions ////////////////////////////////////////////////////////
void F( const gVector<double> &X, gVector<double> &V );

void Fjac( const gVector<double> &X, gVector<double> &V, int K );

void rho( const gVector<double> &A,const gMatrix<double> &row_payoffs,
          const gMatrix<double> &col_payoffs,const double lambda,
          const gVector<double> &X, gVector<double> &V);

void rhojac( const gVector<double> &A,const gMatrix<double> &row_payoffs,
             const gMatrix<double> &col_payoffs, const double lambda,
             const gVector<double> &X, gVector<double> &V, int K );


// Prototypes ////////////////////////////////////////////////////////////
void fixpnf( FILE *fp,const gMatrix<double> &row_payoffs,
             const gMatrix<double> &col_payoffs,
				 int N, gVector<double> &Y, int &flag, double rel_arc_err,
             double abs_arc_err, double rel_ans_err, double abs_ans_err,
             bool trace, gVector<double> &A, gArray<double> &stepsize_params,
             int &jeval_num, double &arclength,double max_lambda,
             bool poly_switch);

void stepnf( int N,
             const gMatrix<double> &row_payoffs,
             const gMatrix<double> &col_payoffs,
             int &jeval_num,
             int &flag,
             bool &start,       bool &crash,
             double &old_step,  double &step_size,
             double &rel_err,   double &abs_err,
             double &arclength,
             gVector<double> &Y,        gVector<double> &Ytan,
             gVector<double> &Y_old,    gVector<double> &Ytan_old,
             const gVector<double> &A,
             const gArray<double> &stepsize_params
           );

void rootnf( int N,const gMatrix<double> &row_payoffs,
             const gMatrix<double> &col_payoffs,
             int &jevalcount, int &flag,
             double relerr,     double abserr,
             const gVector<double> &Y,
             gVector<double> &Ytan,
             gVector<double> &Y_old,
             gVector<double> &Ytan_old,
             const gVector<double> &A
           );

void root( double &t, double &F_of_t,
           double &b, double &c,
           double relerr, double abserr,
           int &flag
         );
void show_probs(char *msg,const gVector<double> &Y,int n_rows,int n_cols);
bool out_of_bounds(gVector<double> &W,int range);
void build_prediction(gVector<double> &W,int n, gVector<double> &Y_old,
						gVector<double> &Ytan_old,gVector<double> &Y,
                  gVector<double> &Ytan,double old_step,double old_step_plus_step_size
                 );
void tangnf( double &rholen,
				 const gMatrix<double> &row_payoffs,
             const gMatrix<double> &col_payoffs,
             const gVector<double> &Y,
             gVector<double> &Ytan,
             const gVector<double> &Ytan_old,
             const gVector<double> &A,
             gVector<double> &Newton_step,
             int &jevalcount, int N, int &flag
           );

#endif
