//
// This file is part of Gambit
// Copyright (c) 1994-2024, The Gambit Project (http://www.gambit-project.org)
//
// FILE: src/tools/enumpoly/rectangl.h
// Declaration of rectangle class
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

#include "interval.h"
#include "core/vector.h"

/* This file provides the template class

   gRectangle

which models the concept of a nonempty compact interval.
Since boundary points can be identified, open and half 
open (bounded) intervals can be effected, but less directly.
*/

template <class T> class gRectangle {
private:
    Gambit::List<gInterval<T> > sides;

public:
    // constructors
    gRectangle(const gRectangle<T>&);
    explicit gRectangle(const Gambit::List<gInterval<T> >&);
    gRectangle(const Gambit::Vector<T> &, const Gambit::Vector<T> &);
    ~gRectangle();

    // operators
    gRectangle<T>& operator =  (const gRectangle<T>& y);
    bool          operator == (const gRectangle<T>& y) const;
    bool          operator != (const gRectangle<T>& y) const;

    // information
    const int                Dmnsn()                            const;
    Gambit::Vector<T>         LowerBound()                       const;
    Gambit::Vector<T>         UpperBound()                       const;
    const T                  LowerBoundOfCoord(const int&)      const;
    const T                  UpperBoundOfCoord(const int&)      const;
    const T                  HeightInCoord(const int&)          const;
    const gInterval<T>       CartesianFactor(const int&)        const;
    const gRectangle<T>      SameCenterDoubleSideLengths()      const;
    const gRectangle<T>      CubeContainingCrcmscrbngSphere()   const;
    const gRectangle<T>      Orthant(const Gambit::Array<int>&)        const;
    const Gambit::Vector<T>         SideLengths()                      const;
    const T                  MaximalSideLength()                const;
    bool                     Contains(const Gambit::Vector<T> &)       const;
    bool                     Contains(const gRectangle<T> &)    const;
    const T                  Volume()                           const;
    const Gambit::Vector<T>         Center()                           const;
    const gRectangle<T>      BoundingRectangle()                const;
    const Gambit::List<Gambit::Vector<T> > VertexList()                       const;
    const int                NumberOfCellsInSubdivision()       const;
    const gRectangle<T>      SubdivisionCell(const int&)        const;
    const T                  DiameterSquared()                  const;

};

  //-------------
  // Conversion:
  //-------------

template <class T> gRectangle<double> TogDouble(const gRectangle<T>&);

#endif // RECTANGL_H
