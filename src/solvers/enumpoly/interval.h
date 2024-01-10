//
// This file is part of Gambit
// Copyright (c) 1994-2024, The Gambit Project (http://www.gambit-project.org)
//
// FILE: src/tools/enumpoly/interval.h
// Interface to interval type
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

#include "gambit.h"
#include "core/rational.h"

/* This file provides the template class

   gInterval

which models the concept of a nonempty compact interval.
Since boundary points can be identified, open and half 
open (bounded) intervals can be effected, but less directly.
*/



template<class T> class gInterval {
private:
    T lower_bd;
    T upper_bd;
    
public:
    // constructors
    gInterval(const gInterval<T> &p_interval)
      : lower_bd(p_interval.lower_bd), upper_bd(p_interval.upper_bd) { }
    gInterval(const T &p_low, const T &p_high) : lower_bd(p_low), upper_bd(p_high) { }
    ~gInterval() = default;

    gInterval<T>& operator=(const gInterval<T> &y) = delete;

    // operators
    bool operator==(const gInterval<T> &p_rhs) const
    {
      return lower_bd == p_rhs.lower_bd && upper_bd == p_rhs.upper_bd;
    }
    bool operator!=(const gInterval<T> &p_rhs) const
    {
      return lower_bd != p_rhs.lower_bd || upper_bd != p_rhs.upper_bd;
    }

    // information
    const T &LowerBound() const { return lower_bd; }
    const T &UpperBound() const { return upper_bd; }
    bool Contains(const T &p_number) const { return lower_bd <= p_number && p_number <= upper_bd; }
    bool Contains(const gInterval<T> &p_interval) const
    { return (lower_bd <= p_interval.lower_bd && p_interval.upper_bd <= upper_bd); }
    bool LiesBelow(const T &p_number) const { return upper_bd <= p_number; }
    bool LiesAbove(const T &p_number) const { return p_number <= lower_bd; }
    T Length() const { return upper_bd - lower_bd; }
    T Midpoint() const { return (upper_bd + lower_bd)/(T)2; }
    gInterval<T> LeftHalf() const { return gInterval<T>(LowerBound(), Midpoint()); }
    gInterval<T> RightHalf() const { return gInterval<T>(Midpoint(), UpperBound()); }

    gInterval<T> SameCenterTwiceLength() const
    { return gInterval<T>((T)2 * LowerBound() - Midpoint(), (T)2 * UpperBound() - Midpoint()); }
    gInterval<T> SameCenterWithNewLength(const T &p_length) const
    { return gInterval<T>(Midpoint() - p_length / (T) 2, Midpoint() + p_length / (T) 2); }
};

#endif // INTERVAL_H
