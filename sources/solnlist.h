//
// FILE: solnlist.h -- Declaration of SolutionList class
//
// $Id$
//

#ifndef SOLNLIST_H
#define SOLNLIST_H

#include "gslist.h"

template <class T> class SolutionList : public gSortList<T> {
private:
  unsigned int max_id;

public:
  SolutionList(void):gSortList<T>(), max_id(1) { }
  SolutionList(const gList<T> &l): gSortList<T>(l), max_id(1) { }

  virtual int Append(const T &a)  
    { (*this)[gSortList<T>::Append(a)].SetId(max_id++);
    return Length();
    }
};

#endif  // SOLNLIST_H
