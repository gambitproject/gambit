//
// FILE: dlqre.h -- Parameter selection for QreSolve
//
// $Id$
//

#ifndef DLQRE_H
#define DLQRE_H

#include "algdlgs.h"

class dialogQre : public dialogPxi {
private:
  wxNumberItem *m_minLam, *m_maxLam, *m_delLam;
  wxIntegerItem *m_accuracy;
  wxRadioBox *m_startOption;

  const char *HelpString(void) const  { return "QRE"; }
  
  void AlgorithmFields(void);

  // Event handlers
  void OnOK(wxCommandEvent &);

public:
  dialogQre(wxWindow *parent, const gText &filename, bool p_vianfg = false);
  virtual ~dialogQre();

  double MinLam(void) const
    { return m_minLam->GetNumber(); }
  double MaxLam(void) const
    { return m_maxLam->GetNumber(); }
  double DelLam(void) const
    { return m_delLam->GetNumber(); }
  int Accuracy(void) const
    { return m_accuracy->GetInteger(); }

  int StartOption(void) const
    { return m_startOption->GetSelection(); }

  DECLARE_EVENT_TABLE()
};

#endif  // DLQRE_H
