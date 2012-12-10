/////////////////////////////////////////////////////////////////////////////
// Name:        lm_lsqr.h
// Purpose:     Levenberg-Marquart nonlinear least squares 2-D curve fitting
// Author:      John Labenski, mostly others (see below)
// Modified by:
// Created:     6/5/2002
// Copyright:   (c) John Labenski, mostly others (see below)
// Licence:     Public domain
/////////////////////////////////////////////////////////////////////////////

/*
 * Solves or minimizes the sum of squares of m nonlinear
 * functions of n variables.
 *
 * From public domain Fortran version
 * of Argonne National Laboratories MINPACK
 *
 * argonne national laboratory. minpack project. march 1980.
 * burton s. garbow, kenneth e. hillstrom, jorge j. more
 *
 * C translation by Steve Moshier http://www.moshier.net/
 *
 * C++ "translation" for use w/ wxWindows by John Labenski
 */

#ifndef _LM_LEASTSQUARE_H_
#define _LM_LEASTSQUARE_H_

#include "wx/plotctrl/plotdefs.h"
class WXDLLIMPEXP_PLOTCTRL wxPlotData;
class WXDLLIMPEXP_PLOTCTRL wxPlotFunction;

// When SetLM_LeastSquareProgressHandler is called with a non NULL handler it will be
//   called when fitting a curve every SetLM_LeastSquareProgressHandlerTicks
//      text is the function name
//      current is the current iteration, max is max allowed iterations
//          note: current may exceed max by a few iterations in some cases

// Usage: create a function like this
//   void LM_LeastSquareProgressHandler(const wxString &text, int current, int max)
//     { [ do stuff... for example update a progress dialog ]
//       wxString str = text + wxString::Format(wxT("\nIteration # %d of %d"), current, max);
//       int percent = wxMin(int(100.0*current/max), 99); // iterations may overflow!
//       return s_progressDialog->Update(percent, str); }
//
//   then call SetLM_LeastSquareProgressHandler( LM_LeastSquareProgressHandler );

extern "C" {
typedef bool (*LM_LeastSquareProgressHandler_)(const wxString &WXUNUSED(text),
                                               int WXUNUSED(current),
                                               int WXUNUSED(max));
extern void SetLM_LeastSquareProgressHandler( LM_LeastSquareProgressHandler_ handler );
extern void SetLM_LeastSquareProgressHandlerTicks( int iterations );
}

//=============================================================================
// LM_LeastSquare - Levenberg-Marquart nonlinear least squares 2-D curve fitting
//
// Fit the plot function to the plot data
//
// Notes :
//   the plot function must have fewer or equal vars than the plotdata has points
//   the plotfunction MUST! have the LAST variable as 'x'
//   you can set the starting values by filling 'initial_vals', size = (plotFunc.GetNumberVars - 1)
//   if initial_vars = NULL then they are all 0.1
//
// Sample usage :
//  wxString message;
//  // Create some plotData, in this case from a known function
//  wxPlotData data(wxPlotFunction("2.5*x*x-3*x+5+3.3*log(x)+13*exp(15*x/(x+4))", "x",dummy), 0, 1E-4, 10000);
//  // Create the plotFunc we want to fit to the data, note: x is last var
//  wxPlotFunction func("a*x*x+b*x+c+d*log(x)+e*exp(f*x/(x+g))", "a,b,c,d,e,f,g,x", message);
//  LM_LeastSquare lmLeastSquare;
//  if (lmLeastSquare.Create(data, func)) {
//      lmLeastSquare.Fit(NULL);
//      or Fit(init, init_count) where double init[init_count] = { a, b, c, ... }
//      for (int k=0; k<lmLeastSquare.GetNumberVariables(); k++) // print a,b,c,...
//           wxPrintf(wxT("%s=%g; "), func.GetVariableName(k).c_str(), lmLeastSquare.GetVariable(k));
//
//=============================================================================

class WXDLLIMPEXP_PLOTCTRL LM_LeastSquare
{
public:
    LM_LeastSquare();
    virtual ~LM_LeastSquare() { Destroy(); }

    // Initialize everything, returns sucess, on failure GetResultMessage()
    //   you may call Create and then Fit on a single instance on this
    //   as many times as you like.
    bool Create(const wxPlotData &plotData, const wxPlotFunction &plotFunc);
    // Has this been sucessfully created and is ready to be Fit()
    bool Ok() const { return m_ok; }

    // After creation fit the plotFunc's vars to the plotData, returns # iterations
    //   initial_vals are initial guesses for the variables which may be NULL
    //   specify the number of initial variables with init_count
    int Fit(const double *initial_vals = NULL, int init_count = 0);
    // returns true if this is currently fitting
    bool IsFitting() const { return m_fitting; }
    // abort a currently running fit, this may take a cycle or two so check
    //   IsFitting to determine when it is done.
    void AbortFitting() { m_abort_fitting = true; }
    // was the last fit aborted, reset when Create or Fit is called again
    bool GetAbortFitting() const { return m_abort_fitting; }

    // If you don't cal Fit(some_vars, count) then the variables are all
    // initialized with this value, default = 0.1
    double GetInitValue() const { return m_init_value; }
    void SetInitValue(double init_val) { m_init_value = init_val; }

    // Get the number of evaluations performed to find best fit
    int GetNumberIterations() const { return m_nfev; }
    // Get the euclidean norm of errors between data and function points
    double GetEuclideanNorm() const { return m_fnorm; }
    // Get the number of variables, i.e. (plotFunc.GetNumberVars() - 1, x is excluded)
    int GetNumberVariables() const { return m_n; }
    // Get the evaluated variables, size is (plotFunc.GetNumberVars() - 1, x is excluded)
    double *GetVariables() const { return m_x; }
    // Get a single evaluated variable, 0 to GetNumberVariables()-1
    double GetVariable(int n);
    // Get an informational message about the results
    wxString GetResultMessage() const;

protected:
    void ReInit();   // only after a call to destroy - reset the vars
    void Destroy();

    wxPlotData *m_plotData;
    wxPlotFunction *m_plotFunc;
    double m_init_value;
    wxString m_resultMsg;
    bool m_ok;
    bool m_fitting;
    bool m_abort_fitting;

    // this is the function to calculate the difference
    virtual void fcn(int m, int n, double x[], double fvec[], int *iflag);

    void lmdif( int m, int n, double x[], double fvec[], double ftol,
                double xtol, double gtol, int maxfev, double epsfcn,
                double diag[], int mode, double factor, int nprint, int *info,
                int *nfev, double fjac[], int ldfjac, int ipvt[], double qtf[],
                double wa1[], double wa2[], double wa3[], double wa4[]);

    // implementation - you probably don't want to mess with these!

    void lmpar(int n, double r[], int ldr, int ipvt[],
               double diag[], double qtb[], double delta, double *par,
               double x[], double sdiag[], double wa1[], double wa2[]);

    void qrfac(int m, int n, double a[], int lda, int pivot, int ipvt[],
               int lipvt, double rdiag[], double acnorm[], double wa[]);

    void qrsolv(int n, double r[], int ldr, int ipvt[], double diag[],
                double qtb[], double x[], double sdiag[], double wa[]);

    double enorm(int n, double x[]);

    void fdjac2(int m,int n, double x[], double fvec[], double fjac[],
                int ldfjac, int *iflag, double epsfcn, double wa[]);

    int    m_n;       // # of variables of plotFunc
    int    m_m;       // # of functions = points in plotData
    int    m_info;    // index of info message strings
    double m_fnorm;   // euclidean norm of errors
    double m_eps;     // resolution of arithmetic
    double m_dwarf;   // smallest nonzero number
    int    m_nfev;    // # iterations completed
    unsigned long m_nan; // # if times function evaluation had a NaN
    double m_ftol;    // relative error in the sum of the squares, if less done
    double m_xtol;    // relative error between two iterations, if less done
    double m_gtol;    // cosine of the angle between fvec and any column of the jacobian, if less done
    double m_epsfcn;  // step length for the forward-difference approximation
    double m_factor;  // initial step bound
    double *m_vars;   // variables + 1, where last is var 'x' for wxPlotFunction
    double *m_x;      // variables (size m_n)
    double *m_fvec;   // output of evaluated functions (size m_m)
    double *m_diag;   // multiplicative scale factors for the variables, see m_mode
    int    m_mode;    // =1 the vars scaled internally. if 2, scaling specified by m_diag.
    double *m_fjac;   // output m by n array
    int    m_ldfjac;  // the leading dimension of the array fjac >= m_m
    double *m_qtf;    // output array the first n elements of the vector (q transpose)*fvec
    int    *m_ipvt;   // integer output array of length n
    int    m_maxfev;  // maximum number of iterations to try

private:
    void Init();
};

#endif // _LM_LEASTSQUARE_H_
