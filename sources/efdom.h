//
// FILE: efdom.h -- Compute dominated strategies on extensive form
//
// $Id: efdom.h
//

#ifndef EFDOM_H
#define EFDOM_H

#include "efg.h"
#include "gstatus.h"

class efgDominanceException : public gException {
private:
  gText m_description;

public:
  efgDominanceException(const gText &);
  virtual ~efgDominanceException();

  gText Description(void) const;
};

#endif    // EFDOM_H
