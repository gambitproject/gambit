//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Dialog for viewing and editing properties of a contingency
//

#ifndef DLEDITCONT_H
#define DLEDITCONT_H

class dialogEditContingency : public wxDialog {
private:
  wxChoice *m_outcome;

public:
  // Lifecycle
  dialogEditContingency(wxWindow *p_parent, const Nfg &, 
			const gArray<int> &);

  // Data access (only valid when ShowModal() returns with wxID_OK)
  int GetOutcome(void) const { return m_outcome->GetSelection(); }
};

#endif   // DLEDITCONT_H
