//
// FILE: dialogalgorithm.h -- Base class for algorithm parameter selection
//
// $Id$
//

#ifndef DIALOGALGORITHM_H
#define DIALOGALGORITHM_H

#include "gtext.h"
#include "gnumber.h"
#include "dialogauto.h"

class dialogAlgorithm : public guiAutoDialog {
private:
  void OnDepth(void);
  void OnAll(void);
  void OnTrace(void);

protected:
  bool m_usesNfg, m_subgames;
  int m_traceDest, m_traceLevel;
  gText m_traceFile;
  wxStaticBox *m_dominanceGroup, *m_subgamesGroup, *m_algorithmGroup;
  wxRadioBox *m_depthChoice, *m_typeChoice, *m_methodChoice;
  wxCheckBox *m_markSubgames, *m_selectSolutions;

  wxTextCtrl *m_stopAfter;
  wxCheckBox *m_findAll;
  wxRadioBox *m_precision;

  void DominanceFields(bool p_mixed);
  void SubgameFields(void);

  void StopAfterField(void);
  void PrecisionField(void);

  virtual void AlgorithmFields(void) { }
  const char *HelpTopic(void) const  { return "Solutions of Games"; }
  void MakeCommonFields(bool p_dominance, bool p_subgames, bool p_vianfg);

public:
  dialogAlgorithm(wxWindow *, const gText &, bool);
  virtual ~dialogAlgorithm();

  bool MarkSubgames(void) const 
    { return (m_markSubgames && m_markSubgames->GetValue()); }

  bool Eliminate(void) const
    { return (m_depthChoice->GetSelection() > 0); }
  bool EliminateAll(void) const 
    { return (m_depthChoice->GetSelection() == 2); }
  bool EliminateWeak(void) const
    { return (m_typeChoice->GetSelection() == 0); }
  bool EliminateMixed(void) const
    { return (m_methodChoice && m_methodChoice->GetSelection() == 1); }

  gOutput *TraceFile(void) const;
  int TraceLevel(void) const { return m_traceLevel; }

  virtual gPrecision Precision(void) const { return precDOUBLE; }

  DECLARE_EVENT_TABLE()
};


#endif   // DIALOGALGORITHM_H

