//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Declaration of utility classes for algorithms
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

#ifndef ALGUTILS_H
#define ALGUTILS_H

#include "base/gstream.h"
#include "base/gstatus.h"
#include "math/gnumber.h"

// 
// gFact is a class for storing information that may or may not 
// be needed, and which might be expensive to compute.  Hence: 
// 
// 1. Answer is not computed unless specifically requested, and  
// 2. Answer is cached in case it is requested again.  
// 

template <class T> class gFact {
private:
  mutable T *m_answer;

public:
  class Contradiction : public gException  {
  public:
    virtual ~Contradiction();
    gText Description(void) const;   
  };
  class Unknown : public gException  {
  public:
    virtual ~Unknown();
    gText Description(void) const;   
  };
  
  gFact(void);
  gFact(const gFact<T> &);
  virtual ~gFact();
  
  gFact &operator=(const gFact<T> &);
  
  const T &Answer(void) const;
  bool Checked(void) const { return m_answer; }
  void Invalidate(void) { if (m_answer)  { delete m_answer; m_answer = 0; } }
  void Set(const T &state);
};

#endif // ALGUTILS_H

