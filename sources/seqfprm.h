//
// FILE: seqfprm.h -- definition of params dialog for LCPSolve[EFG]
//
// $Id$
//

#ifndef SEQFPRM_H
#define SEQFPRM_H

#include "algdlgs.h"

class SeqFormParamsSettings: public virtual OutputParamsSettings {
protected:
  int plev, maxdepth, dup_strat;
  void SaveDefaults(void);

public:
  SeqFormParamsSettings(void);
  ~SeqFormParamsSettings();

  void GetParams(SeqFormParams &P);
  int DupStrat(void) const { return dup_strat; }
};

class SeqFormParamsDialog : public OutputParamsDialog,
			    public SeqFormParamsSettings {
public:
  SeqFormParamsDialog(wxWindow *p_parent = 0, bool p_subgames = false);
  //	~SeqFormParamsDialog(void);
};

#endif
