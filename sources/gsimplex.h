//
// FILE: gsimplex.h -- gSimplex class
//
// @(#)gsimplex.h	1.4 02/14/98 
//

#ifndef GSIMPLEX_H
#define GSIMPLEX_H

#include "interval.h"
#include "odometer.h"
#include "gvector.h"

/* This file provides the template class

   gSimplex

which models the concept of a nonempty compact interval.
Since boundary points can be identified, open and half 
open (bounded) intervals can be effected, but less directly.
*/

template<class T> class gSimplex {
private:
    gList<gInterval<T> > sides;

public:
    // constructors
    gSimplex(const gSimplex<T>&);
    gSimplex(const gList<gInterval<T> >&);
    gSimplex(const gVector<T>, const gVector<T>);
    ~gSimplex();

    // operators
    gSimplex<T>& operator =  (const gSimplex<T>& y);
    bool          operator == (const gSimplex<T>& y) const;
    bool          operator != (const gSimplex<T>& y) const;

    // information
    const int                Dmnsn()                          const;
    const gVector<T>         LowerBound()                     const;
    const gVector<T>         UpperBound()                     const;
    const T                  LowerBoundOfCoord(const int&)    const;
    const T                  UpperBoundOfCoord(const int&)    const;
    const T                  HeightInCoord(const int&)        const;
    const gInterval<T>       CartesianFactor(const int&)      const;
    const gSimplex<T>      SameCenterDoubleSideLengths()    const;
    const gSimplex<T>      Orthant(const gArray<int>&)      const;
    const gVector<T>         SideLengths()                    const;
    bool                     Contains(const gVector<T> &)     const;
    bool                     Contains(const gSimplex<T> &)  const;
    const T                  Volume()                         const;
    const gVector<T>         Center()                         const;
    const gSimplex<T>      BoundingSimplex()              const;
    const gList<gVector<T> > VertexList()                     const;
    const int                NumberOfCellsInSubdivision()     const;
    const gSimplex<T>      SubdivisionCell(const int&)      const;

    // output
friend gOutput& operator << (gOutput& output, const gSimplex<T>& x);
};


  //-------------
  // Conversion:
  //-------------

template <class T> gSimplex<gDouble> TogDouble(const gSimplex<T>&);

#endif // GSIMPLEX_H

