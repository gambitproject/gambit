//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Implementation of utility classes for algorithms
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

#include "algutils.h"

//------------------------------------------------------------------------
//                      class gFact<T>: Member functions
//------------------------------------------------------------------------

template <class T> gFact<T>::gFact(void) 
  : m_answer(0)
{ }

template <class T> gFact<T>::gFact(const gFact<T> &p_fact) 
  : m_answer(0)
{ 
  if (p_fact.Checked()) {
    Set(p_fact.Answer());
  }
}

template <class T> gFact<T>::~gFact() 
{ 
  if (m_answer)  {
    delete m_answer;
  }
}


template <class T> gFact<T> &gFact<T>::operator=(const gFact<T> &p_fact)
{ 
  if (this != &p_fact) {
    if (m_answer) {
      delete m_answer;
      m_answer = 0;
    }

    if (p_fact.Checked()) { 
      Set(p_fact.Answer());
    }
  } 
  return *this;
}

template <class T> const T &gFact<T>::Answer(void) const
{
  if (!m_answer) { 
    throw Unknown();
  }
  return *m_answer;
}

template <class T> void gFact<T>::Set(const T &p_state)
{
  if (m_answer && *m_answer != p_state) { 
    throw Contradiction();
  }
  if (!m_answer) { 
    m_answer = new T(p_state);
  }
}

template <class T> gFact<T>::Contradiction::~Contradiction()
{ }

template <class T> gText gFact<T>::Contradiction::Description(void) const 
{
  return "Contradictory information in gFact";
}

template <class T> gFact<T>::Unknown::~Unknown()
{ }

template <class T> gText gFact<T>::Unknown::Description(void) const 
{
  return "Unknown information in gFact";
}

template class gFact<gNumber>;
template class gFact<gTriState>;

#include "math/gpvector.h"
template class gFact<gPVector<gNumber> >;

