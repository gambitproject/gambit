//
// FILE: nfgsolvd.h -- the main dialog for running NormalForm solution
//                     algorithms.
//
// $Id$
//

#include "nfg.h"

// You must add an entry here for each new algorithm.
// Update: this now takes the number of players so that it can turn
// of the algorithms that will not work with this number of players.

#ifndef NFGSOLVD_H
#define NFGSOLVD_H

#define SD_CANCEL           -1
#define SD_PARAMS            1
#define SD_SAVE              2

#define SOLN_SECT           "Soln-Defaults"
#define PARAMS_SECTION      "Algorithm Params"      // section in .ini file

#define STANDARD_NASH        0
#define STANDARD_PERFECT     1
#define STANDARD_SEQUENTIAL  2
#define STANDARD_ONE         0
#define STANDARD_TWO         1
#define STANDARD_ALL         2

#define DOM_WEAK             0
#define DOM_STRONG           1


// solution module constants.  Do not change the order.  Add new ones
// just before NFG_NUM_SOLUTIONS.

typedef enum {
  NFG_NO_SOLUTION = -1, NFG_ENUMPURE_SOLUTION, NFG_ENUMMIXED_SOLUTION,
  NFG_LP_SOLUTION, NFG_LCP_SOLUTION, NFG_QRE_SOLUTION, 
  NFG_QREALL_SOLUTION, NFG_LIAP_SOLUTION, NFG_SIMPDIV_SOLUTION, 
  NFG_NUM_SOLUTIONS
} NfgSolutionT;


class NfgSolveSettings {
protected:
  int algorithm;
  int result;
  Bool extensive, auto_inspect;
  int standard_type, standard_num;
  bool solving;

protected:
  char *defaults_file;
  const Nfg &nf;

  virtual void Warn(const char *warning);

public:
  NfgSolveSettings(const Nfg &p_nfg, bool p_solving = true);
  virtual ~NfgSolveSettings();

  NfgSolutionT GetAlgorithm(void) { return (NfgSolutionT)algorithm; }
  bool GetExtensive(void) const { return extensive; }
  bool AutoInspect(void) const { return auto_inspect;  }
  int StandardType(void) const { return standard_type; }
  int StandardNum(void) const { return standard_num;  }
};

class NfgSolveStandardDialog : public NfgSolveSettings, public MyDialogBox {
private:
  gPrecision m_precision;

  char *m_standardTypeStr, *m_standardNumStr, *m_precisionStr;
  wxStringList *m_standardTypeList, *m_standardNumList, *m_precisionList;

  // call this to convert standard settings to actual solution parameters
  void StandardSettings(void);

public:
  NfgSolveStandardDialog(const Nfg &p_nfg, wxWindow *p_parent);
  virtual ~NfgSolveStandardDialog();
};

#endif // NFGSOLVD_H
