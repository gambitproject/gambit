//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Declaration of rectangle class
//
// This file is part of Gambit
// Copyright (c) 2002, The Gambit Project
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
//

#ifndef RECTANGL_H
#define RECTANGL_H

#include "base/odometer.h"
#include "interval.h"
#include "math/gvector.h"

/* This file provides the template class

   gRectangle

which models the concept of a nonempty compact interval.
Since boundary points can be identified, open and half 
open (bounded) intervals can be effected, but less directly.
*/

template <class T> class gRectangle {
private:
    gList<gInterval<T> > sides;

public:
    // constructors
    gRectangle(const gRectangle<T>&);
    gRectangle(const gList<gInterval<T> >&);
    gRectangle(const gVector<T>, const gVector<T>);
    ~gRectangle();

    // operators
    gRectangle<T>& operator =  (const gRectangle<T>& y);
    bool          operator == (const gRectangle<T>& y) const;
    bool          operator != (const gRectangle<T>& y) const;

    // information
    const int                Dmnsn()                            const;
    gVector<T>         LowerBound()                       const;
    gVector<T>         UpperBound()                       const;
    const T                  LowerBoundOfCoord(const int&)      const;
    const T                  UpperBoundOfCoord(const int&)      const;
    const T                  HeightInCoord(const int&)          const;
    const gInterval<T>       CartesianFactor(const int&)        const;
    const gRectangle<T>      SameCenterDoubleSideLengths()      const;
    const gRectangle<T>      CubeContainingCrcmscrbngSphere()   const;
    const gRectangle<T>      Orthant(const gArray<int>&)        const;
    const gVector<T>         SideLengths()                      const;
    const T                  MaximalSideLength()                const;
    bool                     Contains(const gVector<T> &)       const;
    bool                     Contains(const gRectangle<T> &)    const;
    const T                  Volume()                           const;
    const gVector<T>         Center()                           const;
    const gRectangle<T>      BoundingRectangle()                const;
    const gList<gVector<T> > VertexList()                       const;
    const int                NumberOfCellsInSubdivision()       const;
    const gRectangle<T>      SubdivisionCell(const int&)        const;
    const T                  DiameterSquared()                  const;

    // output
    void Output(gOutput &) const;
};

template <class T> gOutput &operator<<(gOutput &, const gRectangle<T> &);

  //-------------
  // Conversion:
  //-------------

template <class T> gRectangle<gDouble> TogDouble(const gRectangle<T>&);

#endif // RECTANGL_H
