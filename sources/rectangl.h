//
// FILE: rectangl.h -- gRectangle class
//
// $Id$ 
//

#include "interval.h"
#include "gvector.h"

/* This file provides the template class

   gRectangle

which models the concept of a nonempty compact interval.
Since boundary points can be identified, open and half 
open (bounded) intervals can be effected, but less directly.
*/

template<class T> class gRectangle {
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
    const int           Dmnsn()                          const;
    const gVector<T>    LowerBound()                     const;
    const gVector<T>    UpperBound()                     const;
    const gInterval<T>  CartesianFactor(const int&)      const;
    const gRectangle<T> SameCenterDoubleSideLengths()    const;
    const gVector<T>    SideLengths()                    const;
    bool                Contains(const gVector<T> &)     const;
    bool                Contains(const gRectangle<T> &)  const;
    const T             Volume()                         const;
    const gVector<T>    Center()                         const;

    // output
friend gOutput& operator << (gOutput& output, const gRectangle<T>& x);
};


  //-------------
  // Conversion:
  //-------------

template <class T> gRectangle<gDouble> TogDouble(const gRectangle<T>&);

