//
// FILE: dlnfg.cc -- Normal form-related dialog implementations
//
// $Id$
//

#include "wx.h"
#include "wxmisc.h"

#include "nfg.h"
#include "nfstrat.h"

#include "dlnfgpayoff.h"
#include "dlnfgoutcome.h"
#include "dlnfgsave.h"
#include "dlnfgnewsupport.h"

//=========================================================================
//                 class dialogNfgPayoffs: Member functions 
//=========================================================================

int dialogNfgPayoffs::s_payoffsPerDialog = 8;

dialogNfgPayoffs::dialogNfgPayoffs(const Nfg &p_nfg, NFOutcome *p_outcome,
				   bool p_solutions, wxWindow *p_parent)
  : wxDialogBox(p_parent, "Change Payoffs", TRUE),
    m_outcome(p_outcome), m_nfg(p_nfg), m_pageNumber(0),
    m_payoffs(p_nfg.NumPlayers())
{
  for (int pl = 1; pl <= m_nfg.NumPlayers(); pl++)
    m_payoffs[pl] = m_nfg.Payoff(p_outcome, pl);

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

  m_outcomePayoffs = new wxNumberItem *[m_nfg.NumPlayers()];

  for (int pl = 1; pl <= gmin(m_nfg.NumPlayers(),
			      s_payoffsPerDialog); pl++) {
    m_outcomePayoffs[pl-1] = new wxNumberItem(this,
					      ToText(pl) + "  ",
					      m_payoffs[pl]);
    NewLine();
  }

  m_outcomePayoffs[0]->SetFocus();
#ifndef LINUX_WXXT
  m_outcomePayoffs[0]->SetSelection(0, strlen(m_outcomePayoffs[0]->GetValue()));
#endif

  NewLine();
  if (m_nfg.NumPlayers() > s_payoffsPerDialog) {
    m_backButton = new wxButton(this, (wxFunction) CallbackBack, "<< Back");
    m_backButton->SetClientData((char *) this);
    m_backButton->Enable(FALSE);
    m_nextButton = new wxButton(this, (wxFunction) CallbackNext, "Next >>");
    m_nextButton->SetClientData((char *) this);
  }

  m_okButton = new wxButton(this, (wxFunction) CallbackOK, "Ok");
  m_okButton->SetClientData((char *) this);
  m_okButton->SetDefault();
  m_cancelButton = new wxButton(this, (wxFunction) CallbackCancel,
				"Cancel");
  m_cancelButton->SetClientData((char *) this);

  Fit();
  Show(TRUE);
}

void dialogNfgPayoffs::OnOK(void)
{
  m_completed = wxOK;
  int entry = 0;
  for (int pl = m_pageNumber * s_payoffsPerDialog;
       pl < gmin((m_pageNumber + 1) * s_payoffsPerDialog,
		 m_nfg.NumPlayers()); pl++, entry++)
    m_payoffs[pl + 1] = m_outcomePayoffs[entry]->GetNumber();
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

void dialogNfgPayoffs::OnBack(void)
{
  int entry = 0;
  for (int pl = m_pageNumber * s_payoffsPerDialog;
       pl < gmin((m_pageNumber + 1) * s_payoffsPerDialog,
		 m_nfg.NumPlayers()); pl++, entry++)
    m_payoffs[pl + 1] = m_outcomePayoffs[entry]->GetNumber();

  m_pageNumber--;
  entry = 0;
  for (int pl = m_pageNumber * s_payoffsPerDialog;
       pl < (m_pageNumber + 1) * s_payoffsPerDialog; pl++, entry++) {
    m_outcomePayoffs[entry]->Show(FALSE);
    m_outcomePayoffs[entry]->SetNumber(m_payoffs[pl + 1]);
    m_outcomePayoffs[entry]->SetValue(ToText(m_payoffs[pl + 1]));
    m_outcomePayoffs[entry]->SetLabel(ToText(pl + 1) + "  ");
  }
  m_backButton->Show(FALSE);
  m_nextButton->Show(FALSE);
  m_okButton->Show(FALSE);
  m_cancelButton->Show(FALSE);
  
  // This gyration ensures the tabbing order remains the same
  m_cancelButton->Show(TRUE);
  m_okButton->Show(TRUE);
  m_nextButton->Show(TRUE);
  m_backButton->Show(TRUE);
  for (entry = s_payoffsPerDialog - 1; entry >= 0; entry--)
    m_outcomePayoffs[entry]->Show(TRUE);
  
  m_outcomePayoffs[0]->SetFocus();
  m_backButton->Enable(m_pageNumber > 0);
  m_nextButton->Enable(TRUE);
}

void dialogNfgPayoffs::OnNext(void)
{
  int entry = 0;
  for (int pl = m_pageNumber * s_payoffsPerDialog;
       pl < (m_pageNumber + 1) * s_payoffsPerDialog; pl++, entry++)
    m_payoffs[pl + 1] = m_outcomePayoffs[entry]->GetNumber();

  m_pageNumber++;
  entry = 0;
  for (int pl = m_pageNumber * s_payoffsPerDialog;
       pl < (m_pageNumber + 1) * s_payoffsPerDialog; pl++, entry++) {
    if (pl < m_nfg.NumPlayers()) {
      m_outcomePayoffs[entry]->SetNumber(m_payoffs[pl + 1]);
      m_outcomePayoffs[entry]->SetValue(ToText(m_payoffs[pl + 1]));
      m_outcomePayoffs[entry]->SetLabel(ToText(pl + 1) + "  ");
    }
    else
      m_outcomePayoffs[entry]->Show(FALSE);
  }

  m_outcomePayoffs[0]->SetFocus();
  m_backButton->Enable(TRUE);
  m_nextButton->Enable((m_pageNumber + 1) * s_payoffsPerDialog <=
		       m_nfg.NumPlayers());
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
//                   dialogNfgNewSupport: Member functions
//=========================================================================

dialogNfgNewSupport::dialogNfgNewSupport(const Nfg &p_nfg, wxWindow *p_parent)
  : wxDialogBox(p_parent, "Define support", TRUE), m_nfg(p_nfg)
{
  SetLabelPosition(wxVERTICAL);
  m_strategyLists = new wxListBox *[m_nfg.NumPlayers()];
  for (int pl = 1; pl <= m_nfg.NumPlayers(); pl++) {
    m_strategyLists[pl-1] = new wxListBox(this, 0, "Player " + ToText(pl),
					  wxMULTIPLE);
    for (int st = 1; st <= m_nfg.NumStrats(pl); st++) {
      m_strategyLists[pl-1]->Append(ToText(st) + ": " +
				    m_nfg.Strategies(pl)[st]->Name());
#ifndef LINUX_WXXT
      m_strategyLists[pl-1]->SetSelection(st - 1, TRUE);
#endif  // LINUX_WXXT
    }
  }

  wxButton *okButton = new wxButton(this, (wxFunction) CallbackOK, "Ok");
  okButton->SetClientData((char *) this);
  okButton->SetDefault();
  wxButton *cancelButton = new wxButton(this, (wxFunction) CallbackCancel,
					"Cancel");
  cancelButton->SetClientData((char *) this);

  Fit();
  Show(TRUE);
}

dialogNfgNewSupport::~dialogNfgNewSupport()
{
  delete [] m_strategyLists;
}

void dialogNfgNewSupport::OnOK(void)
{
  for (int pl = 1; pl <= m_nfg.NumPlayers(); pl++) {
    int *selections;
    if (m_strategyLists[pl-1]->GetSelections(&selections) == 0)
      return;
  }
  m_completed = wxOK;
  Show(FALSE);
}

void dialogNfgNewSupport::OnCancel(void)
{
  m_completed = wxCANCEL;
  Show(FALSE);
}

Bool dialogNfgNewSupport::OnClose(void)
{
  m_completed = wxCANCEL;
  Show(FALSE);
  return FALSE;
}

NFSupport *dialogNfgNewSupport::CreateSupport(void) const
{
  NFSupport *support = new NFSupport(m_nfg);
  for (int pl = 1; pl <= m_nfg.NumPlayers(); pl++) {
    for (int st = 1; st <= m_nfg.NumStrats(pl); st++) {
      if (!m_strategyLists[pl-1]->Selected(st-1))
	support->RemoveStrategy(m_nfg.Strategies(pl)[st]);
    }
  }

  return support;
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


