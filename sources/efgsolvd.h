//
// FILE: efgsolvd.h -- the main dialog for running ExtensiveForm solution
//                     efg_algorithms. 
//
// $Id$
//

// You must add an entry here for each new efg_algorithm.
// Update: this dialog box now also includes the NormalForm solution
// efg_algorithms.  They are enabled by selecting the 'Use NF' box.  This is
// why "nfgsolvd.h" is included here

#ifndef EFGSOLVD_H
#define EFGSOLVD_H

#include "nfgsolvd.h"

#define PARAMS_SECTION	"Algorithm Params"		// section in .ini file

bool IsPerfectRecall(const Efg &, Infoset *&, Infoset *&);

class EfgSolveSettings {
protected:
  Bool use_nfg,normal,subgames,pick_solns,auto_inspect;
  int algorithm;
  char *defaults_file;
  int result;
  bool	solving;

  const Efg &ef;
  
  // PRIVATE MEMBER FUNCTIONS
  virtual void Warn(const char *p_warning);

public:
  // CONSTRUCTOR AND DESTRUCTOR
  EfgSolveSettings(const Efg &p_efg, bool p_solving = true);
  virtual ~EfgSolveSettings();

  // DATA ACCESS
  bool ViaNfg(void) const { return use_nfg; }
  EfgSolutionT GetEfgAlgorithm(void) const { return (EfgSolutionT) algorithm; }
  NfgSolutionT GetNfgAlgorithm(void) const { return (NfgSolutionT) algorithm; }

  bool MarkSubgames(void) const { return subgames; }
  bool AutoInspect(void) const { return auto_inspect; }
};

//
// NB: The numbering of the are important, as they are saved
// in the defaults file as integers and not text strings.
//
#define efgSTANDARD_NASH       0
#define efgSTANDARD_PERFECT    1
#define efgSTANDARD_SEQUENTIAL 2

#define efgSTANDARD_ONE        0
#define efgSTANDARD_TWO        1
#define efgSTANDARD_ALL        2

class EfgSolveStandardDialog : public EfgSolveSettings, public MyDialogBox {
private:
  int m_standardType, m_standardNum;
  gPrecision m_precision;

  char *m_standardTypeStr, *m_standardNumStr, *m_precisionStr;
  wxStringList *m_standardTypeList, *m_standardNumList, *m_precisionList;

  // PRIVATE MEMBER FUNCTIONS
  void StandardSettings(void);

public:
  EfgSolveStandardDialog(const Efg &p_efg, wxWindow *p_parent);
  virtual ~EfgSolveStandardDialog();
};


#endif   // EFGSOLVD_H
