//
// FILE: efgsolvd.h -- Selecting algorithm dialogs
//
// $Id$
//

#ifndef EFGSOLVD_H
#define EFGSOLVD_H

#include "nfgsolvd.h"

#define PARAMS_SECTION	"Algorithm Params"		// section in .ini file

bool IsPerfectRecall(const Efg &, Infoset *&, Infoset *&);

class EfgSolveSettings {
protected:
  Bool use_nfg,normal,subgames,pick_solns;
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
};

//
// NB: The numbering of the are important, as they are saved
// in the defaults file as integers and not text strings.
//
typedef enum {
  efgSTANDARD_NASH = 0, efgSTANDARD_PERFECT = 1, efgSTANDARD_SEQUENTIAL = 2
} guiStandardType;

typedef enum {
  efgSTANDARD_ONE = 0, efgSTANDARD_TWO = 1, efgSTANDARD_ALL 
} guiStandardNum;

class dialogEfgSolveStandard : public wxDialogBox {
private:
  int m_completed;
  wxRadioBox *m_standardType, *m_standardNum, *m_precision;
  wxText *m_description;
  const Efg &m_efg;

  static void CallbackOK(wxButton &p_object, wxEvent &)
    { ((dialogEfgSolveStandard *) p_object.GetClientData())->OnOK(); }
  static void CallbackCancel(wxButton &p_object, wxEvent &)
    { ((dialogEfgSolveStandard *) p_object.GetClientData())->OnCancel(); }
  static void CallbackChanged(wxRadioBox &p_object, wxEvent &)
    { ((dialogEfgSolveStandard *) p_object.GetClientData())->OnChanged(); }

  void OnOK(void);
  void OnCancel(void);
  Bool OnClose(void);
  
  void OnChanged(void);

public:
  dialogEfgSolveStandard(const Efg &p_efg, wxWindow *p_parent);
  virtual ~dialogEfgSolveStandard();

  int Completed(void) const { return m_completed; }
  guiStandardType Type(void) const;
  guiStandardNum Number(void) const;
  gPrecision Precision(void) const
   { return ((m_precision->GetSelection() == 0) ? precDOUBLE : precRATIONAL); }
};

#endif   // EFGSOLVD_H
