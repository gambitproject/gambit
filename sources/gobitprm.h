//
// FILE: gobitprm.h -- Parameter selection for QreSolve
//
// $Id$
//

#ifndef GOBITPRM_H
#define GOBITPRM_H

#include "algdlgs.h"

#define DATA_TYPE_LOG 1

class EFQreParams;
class NFQreParams;

class QreParamsSettings : public virtual PxiParamsSettings {
protected:
  float minLam, maxLam, delLam, tol1, tolN;
  int maxits1,maxitsN;
  int  start_option; // 0-default,1-saved,2-query
  void SaveDefaults(void);

public:
  QreParamsSettings(const char *fn);
  ~QreParamsSettings();
  void GetParams(EFQreParams &);
  void GetParams(NFQreParams &);
  int StartOption(void) const { return start_option; }
};

class QreSolveParamsDialog : public PxiParamsDialog,
			       public QreParamsSettings {
public:
  QreSolveParamsDialog(wxWindow *parent,const gText filename);
  //	~QreSolveParamsDialog(void);
};

#endif  // GOBITPRM_H
