//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Declaration of monomial class
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

#include "prepoly.h"


// This file provides the template class
//
//              gbtMonomial
//
// whose objects are monomials in several variables
// with coefficients of class T and nonnegative exponents. 
// This role of this class is to support the class gbtPolyMulti. 


template<class T> class gbtMonomial {
private:
    T        coef;
    gbtPolyExponent exps;

public:
    // constructors
    gbtMonomial(const gbtPolySpace*, const T&);
    gbtMonomial(const T&, const gbtPolyExponent&);
    gbtMonomial(const gbtMonomial<T>&);
    ~gbtMonomial();

    // operators
          gbtMonomial<T>& operator =  (const gbtMonomial<T>&);

          bool      operator == (const gbtMonomial<T>&) const;
          bool      operator != (const gbtMonomial<T>&) const;
          gbtMonomial<T>  operator *  (const gbtMonomial<T>&) const;
          gbtMonomial<T>  operator /  (const gbtMonomial<T>&) const;
          gbtMonomial<T>  operator +  (const gbtMonomial<T>&) const; // assert exps ==
          gbtMonomial<T>& operator += (const gbtMonomial<T>&);       // assert exps ==
          gbtMonomial<T>& operator *= (const T&);      
          gbtMonomial<T>  operator -  ()                const; 

    // information
    const T &       Coef()                      const;
    int             Dmnsn()                     const;
    int             TotalDegree()               const;
    bool            IsConstant()                const;
    bool            IsMultiaffine()             const;
    const gbtPolyExponent& ExpV()                      const;
    T               Evaluate(const gbtArray<T>&)  const;
    T               Evaluate(const gbtVector<T>&) const;
};

template <class T> gbtOutput &operator<<(gbtOutput &, const gbtMonomial<T> &);
