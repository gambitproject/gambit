//#
//# FILE: enum.h -- Interface to Enum solution module
//#
//# @(#)enum.h	1.6 2/7/95
//#

#ifndef ENUM_H
#define ENUM_H

#include "normal.h"
#include "rational.h"
#include "glist.h"
#include "gsignal.h"
#include "tableau.h"
#include "gtableau.h"

    // Ted -- I tried to put EnumTableau in .cc file, but gnu wouldn't
    //        let me.  
//template <class T> class EnumTableau;

//---------------------------------------------------------------------------
//                    EnumTableau<T>: class definition
//---------------------------------------------------------------------------

template <class T> class EnumTableau : public Tableau<T> {
  private:
    int n1, n2;
  public:
    EnumTableau(void);
    EnumTableau(const NormalForm<T> &);
    int MinRow(void) const;
    int MaxRow(void) const;
//    void AddSolution(gList< gPVector<T> > &);
    virtual ~EnumTableau();
};

class EnumParams     {
public:
  int plev, nequilib;
  gOutput *outfile, *errfile;
  gSignal &sig;
  
  EnumParams(void);
};

//
// The general-purpose interface to the Module routines
//

template <class T> class EnumModule  {
private:
  BFS_List List;
//  gList<gPVector<T> > solutions;
  EnumTableau<T> tab;
  EnumParams params;
  int rows,cols,level,players;
  long count,npivots;
  gRational time;
  
//  void AddSolution(const gPVector<T> &s);
  
public:
  EnumModule(const NormalForm<T> &N, const EnumParams &p); 
  
  int Enum(void);
  void SubSolve(int pr, int pcl, Basis<T> &B1, gTuple<int> &targ1);
  
  int NumPivots(void) const;
  gRational Time(void) const;
  
  EnumParams &Parameters(void);

      //  Ted -- I like 2nd form better, but does this ever get 
      //         de-allocated?  
//  void GetSolutions(gList<gPVector<T> > &solutions) const;
  gList<gPVector<T> > &GetSolutions(void) const;
};

//
// Convenience functions for "one-shot" evaluations
//
template <class T> int Enum(const NormalForm<T> &N, const EnumParams &p,
			    gList<gPVector<T> > &solutions,
			    int &npivots, gRational &time);

#endif    // ENUM_H




