//
// FILE: gsimplex.h -- gSimplex class
//
// $Id$
//

#ifndef GSIMPLEX_H
#define GSIMPLEX_H

#include "interval.h"
#include "rectangl.h"
#include "odometer.h"
#include "gvector.h"
#include "glpsolv3.h"
#include "gsmatrix.h"

/* This file provides the template class

   gSimplex

*/

template<class T> class gSimplex {
private:
    gList<gVector<T> > vertices;

public:
    // constructors
    gSimplex(const gSimplex<T>&);
    gSimplex(const gList<gVector<T> >&);
    ~gSimplex();

    // operators
    gSimplex<T>& operator =  (const gSimplex<T>& y);
    bool          operator == (const gSimplex<T>& y) const;
    bool          operator != (const gSimplex<T>& y) const;

    // information
    const int                AmbientDmnsn()                   const;
    const int                NumberOfVertices()               const;
    const gVector<T>         LowerBound()                     const;
    const gVector<T>         UpperBound()                     const;
    const T                  LowerBoundOfCoord(const int&)    const;
    const T                  UpperBoundOfCoord(const int&)    const;
    const T                  HeightInCoord(const int&)        const;
    const gSimplex<T>        SameCenterDoubleSideLengths()    const;
    const gSimplex<T>        Orthant(const gArray<int>&)      const;
    bool                     Contains(const gVector<T> &)     const;
    bool                     Contains(const gSimplex<T> &)    const;
    const T                  Volume()                         const;
    const gVector<T>         Center()                         const;
    const gRectangle<T>      BoundingRectangle()              const;
    const gList<gVector<T> > VertexList()                     const;
    const gVector<T>         Vertex(const int&)               const;
    const int                NumberOfCellsInSubdivision()     const;
    const gSimplex<T>        SubdivisionCell(const int&)      const;

    // output
friend gOutput& operator << (gOutput& output, const gSimplex<T>& x);
};


  //-------------
  // Conversion:
  //-------------

template <class T> gSimplex<gDouble> TogDouble(const gSimplex<T>&);

#endif // GSIMPLEX_H

