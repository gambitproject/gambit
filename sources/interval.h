//#
//# FILE: interval.h -- gInterval class
//#
//# @(#)interval.h	1.0  2/7/96
//#

#ifndef INTERVAL_H
#define INTERVAL_H

//#include<stream.h>
//#include<String.h>
#include "rational.h"
#include "gambitio.h"

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
    const T LowerBound()        const;
    const T UpperBound()        const;
    bool    Contains(const T&)  const;
    bool    Contains(const gInterval<T>&)  const;
    bool    LiesBelow(const T&) const;
    bool    LiesAbove(const T&) const;
    const T Length()            const;
    const T Midpoint()          const;
    const gInterval<T> LeftHalf() const;
    const gInterval<T> RightHalf() const;
    const gInterval<T> SameCenterTwiceLength() const;

    // output
  friend gOutput& operator <<(gOutput& output, const gInterval<T>& x);
};

#endif INTERVAL_H
