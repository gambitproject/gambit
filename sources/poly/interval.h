//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Interface to interval type
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

#ifndef INTERVAL_H
#define INTERVAL_H

//#include<stream.h>
//#include<String.h>
#include "math/rational.h"
#include "base/gstream.h"

/* This file provides the template class

   gInterval

which models the concept of a nonempty compact interval.
Since boundary points can be identified, open and half 
open (bounded) intervals can be effected, but less directly.
*/

template<class T> class gInterval {
private:
    const T lower_bd;
    const T upper_bd;

public:
    // constructors
    gInterval(const gInterval<T>&);
    gInterval(const T, const T);
    ~gInterval();

    // operators
    gInterval<T>& operator =  (const gInterval<T>& y);
    bool          operator == (const gInterval<T>& y) const;
    bool          operator != (const gInterval<T>& y) const;

    // information
    const T            LowerBound()                                 const;
    const T            UpperBound()                                 const;
    const bool         Contains(const T&)                           const;
    const bool         Contains(const gInterval<T>&)                const;
    const bool         LiesBelow(const T&)                          const;
    const bool         LiesAbove(const T&)                          const;
    const T            Length()                                     const;
    const T            Midpoint()                                   const;
    const gInterval<T> LeftHalf()                                   const;
    const gInterval<T> RightHalf()                                  const;
    const gInterval<T> SameCenterTwiceLength()                      const;
    const gInterval<T> SameCenterWithNewLength(const T&)            const;

};

template <class T> gOutput &operator<<(gOutput &, const gInterval<T> &);

#endif // INTERVAL_H
