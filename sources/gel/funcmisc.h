//
// FILE: funcmisc.h -- Miscellaneous internal structures for function matching
//
// $Id$ %G
//

#ifndef FUNCMISC_H
#define FUNCMISC_H

#include "gnumber.h"
#include "gtext.h"
#include "glist.h"
#include "garray.h"
#include "exprtree.h"

typedef gelExpr *gelAdapter(const gArray<gelExpr *> &); 

class gelParamInfo  {
private:
  gText m_Name;
  gelType m_Type;
  gText m_Default;
  bool m_Optional;
  bool m_ByReference;
  
public:
  gelParamInfo(const gText &fn);
  gelType Type(void) const   { return m_Type; }
};

class gelSignature   
{
private:
  gText m_Name;
  gelType m_Type;
  gList<gelParamInfo *> m_Parameters;

public:
  gelSignature(const gText &gelSignature);
  ~gelSignature();

  bool Matches(const gText &,
	       const gArray<gelExpr *> &) const;
  bool operator==(const gelSignature &) const  { return false; } 
  bool operator!=(const gelSignature &) const  { return false; }
  
  const gText &Name(void) const   { return m_Name; }
};


#endif   // FUNCMISC_H
