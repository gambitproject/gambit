//
// FILE: gsmpltpe.h -- gSimpletope class
//
// $Id$
//

#ifndef GSMPLTPE_H
#define GSMPLTPE_H

#include "interval.h"
#include "rectangl.h"
#include "odometer.h"
#include "gvector.h"
#include "glpsolv3.h"
#include "gsmatrix.h"
#include "gsimplex.h"

/* This file provides the template class

   gSimpletope

*/

template<class T> class gSimpletope {
private:
    gList<gSimplex<T> > simplices;
    gList<gVector<T> > vertices;  // eliminate after transforming everything

public:
    // constructors
    gSimpletope(const gSimpletope<T>&);
    gSimpletope(const gList<gVector<T> >&);
    gSimpletope(const gList<gSimplex<T> >&);
    ~gSimpletope();

    // operators
    gSimpletope<T>& operator =  (const gSimpletope<T>& y);
    bool          operator == (const gSimpletope<T>& y) const;
    bool          operator != (const gSimpletope<T>& y) const;

    // information
    const int                AmbientDmnsn()                   const;
    const int                NumberOfVertices()               const;
    const int                NumberOfSimplices()              const;
    const gVector<T>         LowerBound()                     const;
    const gVector<T>         UpperBound()                     const;
    const T                  LowerBoundOfCoord(const int&)    const;
    const T                  UpperBoundOfCoord(const int&)    const;
    const T                  HeightInCoord(const int&)        const;
    const gSimpletope<T>        SameCenterDoubleSideLengths()    const;
    const gSimpletope<T>        Orthant(const gArray<int>&)      const;
    bool                     Contains(const gVector<T> &)     const;
    bool                     Contains(const gSimpletope<T> &)    const;
    const T                  Volume()                         const;
    const gVector<T>         Center()                         const;
    const gRectangle<T>      BoundingRectangle()              const;
    const gList<gSimplex<T> > SimplexList()                     const;
    const gSimplex<T>         Simplex(const int&)               const;
    const int                NumberOfCellsInSubdivision()     const;
    const gSimpletope<T>        SubdivisionCell(const int&)      const;

    // output
friend gOutput& operator << (gOutput& output, const gSimpletope<T>& x);
};


  //-------------
  // Conversion:
  //-------------

template <class T> gSimpletope<gDouble> TogDouble(const gSimpletope<T>&);

#endif // GSMPLTPE_H

