//
// FILE: corresp.h -- Class for representing a correspondence
//
// $Id$
//

#ifndef CORRESP_H
#define CORRESP_H

#include "base/glist.h"

template <class T, class S> class Correspondence {
private:
  // For the moment, only one component
  gList<T> m_parameter;
  gList<S> m_data;

public:
  // CONSTRUCTOR, DESTRUCTOR, CONSTRUCTIVE OPERATORS
  Correspondence(void);
  Correspondence(const Correspondence<T,S> &);
  ~Correspondence();

  Correspondence<T,S> &operator=(const Correspondence<T,S> &);

  // DATA ACCESS
  int NumComponents(void) const;
  T MinParameter(int p_comp) const;
  T MaxParameter(int p_comp) const;
  int NumPoints(int p_comp) const;

  const S &GetPoint(int p_comp, int p_index) const;
  const T &GetParameter(int p_comp, int p_index) const;

  // EDITING COMPONENTS
  void Append(int p_comp, const T &p_parameter, const S &p_data); 
};


#endif  // CORRESP_H
