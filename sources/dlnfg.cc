//
// FILE: dlnfg.cc -- Normal form-related dialog implementations
//
//
//

#include "wx.h"
#include "wxmisc.h"

#include "nfg.h"

#include "dlnfgpayoff.h"
#include "dlnfgoutcome.h"
#include "dlnfgsave.h"

//=========================================================================
//                 class dialogNfgPayoffs: Member functions 
//=========================================================================

dialogNfgPayoffs::dialogNfgPayoffs(const Nfg &p_nfg, NFOutcome *p_outcome,
				   bool p_solutions, wxWindow *p_parent)
  : wxDialogBox(p_parent, "Change Payoffs", TRUE),
    m_outcome(p_outcome), m_nfg(p_nfg),
    m_payoffs(p_nfg.NumPlayers())
{
  (void) new wxMessage(this, "Change payoffs for outcome:");
  NewLine();

  m_outcomeName = new wxText(this, 0, "Outcome");
  if (p_outcome)
    m_outcomeName->SetValue(p_outcome->GetName());
  else
    m_outcomeName->SetValue("Outcome" + ToText(p_nfg.NumOutcomes() + 1));
  NewLine();

  if (p_solutions) {
    (void) new wxMessage(this, "Pressing OK will delete computed solutions");
    NewLine();
  }

  m_outcomePayoffs = new wxText *[m_nfg.NumPlayers()];

  const int ENTRIES_PER_ROW = 3;

  for (int pl = 1; pl <= m_nfg.NumPlayers(); pl++) {
    m_outcomePayoffs[pl - 1] = new wxText(this, 0, "");
    m_outcomePayoffs[pl - 1]->SetValue(ToText(m_nfg.Payoff(p_outcome, pl)));
    if (pl % ENTRIES_PER_ROW == 0)
      NewLine();
  }

  m_outcomePayoffs[0]->SetFocus();
#ifndef LINUX_WXXT
  m_outcomePayoffs[0]->SetSelection(0, strlen(m_outcomePayoffs[0]->GetValue()));
#endif

  NewLine();
  wxButton *okButton = new wxButton(this, (wxFunction) CallbackOK, "Ok");
  okButton->SetClientData((char *) this);
  okButton->SetDefault();
  wxButton *cancelButton = new wxButton(this, (wxFunction) CallbackCancel,
					"Cancel");
  cancelButton->SetClientData((char *) this);

  Fit();
  Show(TRUE);
}

void dialogNfgPayoffs::OnOK(void)
{
  for (int pl = 1; pl <= m_nfg.NumPlayers(); pl++)
    FromText(m_outcomePayoffs[pl - 1]->GetValue(), m_payoffs[pl]);
  m_name = m_outcomeName->GetValue();

  m_completed = wxOK;
  Show(FALSE);
}

void dialogNfgPayoffs::OnCancel(void)
{
  m_completed = wxCANCEL;
  Show(FALSE);
}

Bool dialogNfgPayoffs::OnClose(void)
{
  m_completed = wxCANCEL;
  Show(FALSE);
  return FALSE;
}

//=========================================================================
//              class dialogNfgOutcomeSelect: Member functions 
//=========================================================================

dialogNfgOutcomeSelect::dialogNfgOutcomeSelect(Nfg &p_nfg, wxWindow *p_parent)
  : wxDialogBox(p_parent, "Select Outcome", TRUE), m_nfg(p_nfg)
{
  m_outcomeList = new wxListBox(this, 0, "Outcome");
  
  for (int outc = 1; outc <= m_nfg.NumOutcomes(); outc++) {
    NFOutcome *outcome = m_nfg.Outcomes()[outc];
    gText item = outcome->GetName();
    if (item == "")
      item = "Outcome" + ToText(outc);

    item += (" (" + ToText(m_nfg.Payoff(outcome, 1)) + ", " +
	     ToText(m_nfg.Payoff(outcome, 2)));
    if (m_nfg.NumPlayers() > 2) {
      item += ", " + ToText(m_nfg.Payoff(outcome, 3));
      if (m_nfg.NumPlayers() > 3) 
	item += ",...)";
      else
	item += ")";
    }
    else
      item += ")";

    m_outcomeList->Append(item);
  }

  m_outcomeList->SetSelection(0);

  NewLine();
  wxButton *okButton = new wxButton(this, (wxFunction) CallbackOK, "Ok");
  okButton->SetClientData((char *) this);
  okButton->SetDefault();
  wxButton *cancelButton = new wxButton(this, (wxFunction) CallbackCancel,
					"Cancel");
  cancelButton->SetClientData((char *) this);
  
  Fit();
  Show(TRUE);
}

void dialogNfgOutcomeSelect::OnOK(void)
{
  m_outcomeSelected = m_outcomeList->GetSelection();
  m_completed = wxOK;
  Show(FALSE);
}

void dialogNfgOutcomeSelect::OnCancel(void)
{
  m_completed = wxCANCEL;
  Show(FALSE);
}

Bool dialogNfgOutcomeSelect::OnClose(void)
{
  m_completed = wxCANCEL;
  Show(FALSE);
  return FALSE;
}

NFOutcome *dialogNfgOutcomeSelect::GetOutcome(void)
{
  return m_nfg.Outcomes()[m_outcomeSelected + 1];
}

//=========================================================================
//                     dialogNfgSave: Member functions
//=========================================================================

dialogNfgSave::dialogNfgSave(const gText &p_name,
			     const gText &p_label, int p_decimals,
			     wxWindow *p_parent)
  : wxDialogBox(p_parent, "Save File", TRUE)
{
  m_fileName = new wxText(this, 0, "Path:");
  m_fileName->SetValue(p_name);

  wxButton *browseButton = new wxButton(this, (wxFunction) CallbackBrowse,
					"Browse...");
  browseButton->SetClientData((char *) this);
  NewLine();

  m_treeLabel = new wxText(this, 0, "Description:", p_label, -1, -1, 300);
  m_treeLabel->SetValue(p_label);
  NewLine();

  m_numDecimals = new wxSlider(this, 0, "Decimal places:",
			       p_decimals, 0, 25, 100);
  NewLine();

  wxButton *okButton = new wxButton(this, (wxFunction) CallbackOK, "Ok");
  okButton->SetClientData((char *) this);
  okButton->SetDefault();
  wxButton *cancelButton = new wxButton(this, (wxFunction) CallbackCancel,
					"Cancel");
  cancelButton->SetClientData((char *) this);

  Fit();
  Show(TRUE);
}

void dialogNfgSave::OnOK(void)
{
  m_completed = wxOK;
  Show(FALSE);
}

void dialogNfgSave::OnCancel(void)
{
  m_completed = wxCANCEL;
  Show(FALSE);
}

Bool dialogNfgSave::OnClose(void)
{
  m_completed = wxCANCEL;
  Show(FALSE);
  return FALSE;
}

void dialogNfgSave::OnBrowse(void)
{
  char *file = wxFileSelector("Save data file", 
			      gPathOnly(m_fileName->GetValue()),
			      gFileNameFromPath(m_fileName->GetValue()),
			      ".nfg", "*.nfg");

  if (file) {
    m_fileName->SetValue(file);
  }
}


