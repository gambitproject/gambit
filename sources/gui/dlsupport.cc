//
// FILE: dlsupport.cc -- Dialogs for manipulation of supports
//
// $Id$
//

#include "wx/wxprec.h"
#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif  // WX_PRECOMP
#include "guishare/wxmisc.h"

#include "efstrat.h"
#include "base/base.h"
#include "gambit.h"
#include "nfgshow.h"
#include "dlelim.h"
#include "dlsupportselect.h"

//=========================================================================
//               class dialogElimMixed: Member functions
//=========================================================================

BEGIN_EVENT_TABLE(dialogElimMixed, guiAutoDialog)
  EVT_BUTTON(wxID_OK, dialogElimMixed::OnOK)
  EVT_LISTBOX(idMETHOD_BOX, dialogElimMixed::OnMethod)
END_EVENT_TABLE()

dialogElimMixed::dialogElimMixed(wxWindow *p_parent,
				 const gArray<gText> &p_players)
  : guiAutoDialog(p_parent, "Dominance Elimination Parameters")
{
  long all = 0, domType = 0, domMethod = 0, domPrecision = 0;
  wxConfig config("Gambit");
  config.Read("Solutions/ElimDom-Mixed-All", &all);
  config.Read("Solutions/ElimDom-Mixed-Type", &domType);
  config.Read("Solutions/ElimDom-Mixed-Method", &domMethod);
  config.Read("Solutions/ElimDom-Mixed-Precision", &domPrecision);

  m_allBox = new wxCheckBox(this, -1, "Eliminate iteratively");
  m_allBox->SetValue(all);
					
  wxString domTypeList[2] = { "Weak", "Strong" };
  m_domTypeBox = new wxRadioBox(this, -1, "Type",
				wxDefaultPosition, wxDefaultSize,
				2, domTypeList);
  if (domType == 0 || domType == 1)
    m_domTypeBox->SetSelection(domType);
  
  wxString domMethodList[2] = { "Pure", "Mixed" };
  m_domMethodBox = new wxRadioBox(this, -1, "Method",
				  wxDefaultPosition, wxDefaultSize,
				  2, domMethodList);
  if (domMethod == 0 || domMethod == 1)
    m_domMethodBox->SetSelection(domMethod);
  
  wxString domPrecisionList[2] = { "Float", "Rational" };
  m_domPrecisionBox = new wxRadioBox(this, -1, "Precision",
				     wxDefaultPosition, wxDefaultSize,
				     2, domPrecisionList);
  if (domPrecision == 0 || domPrecision == 1)
    m_domPrecisionBox->SetSelection(domPrecision);

  m_playerGrid = new wxGrid(this, -1, wxDefaultPosition, wxSize(100, 100));
  m_playerGrid->CreateGrid(p_players.Length(), 2);
  m_playerGrid->SetLabelSize(wxHORIZONTAL, 0);
  m_playerGrid->SetLabelSize(wxVERTICAL, 0);
  m_playerGrid->SetColumnWidth(0, 20);
  for (int pl = 0; pl < p_players.Length(); pl++) {
    m_playerGrid->SetCellValue(pl, 0, "1");
    m_playerGrid->SetCellRenderer(pl, 0, new wxGridCellBoolRenderer);
    m_playerGrid->SetCellEditor(pl, 0, new wxGridCellBoolEditor);

    m_playerGrid->SetCellValue(pl, 1, (char *) p_players[pl+1]);
    m_playerGrid->SetReadOnly(pl, 1);
  }

  wxBoxSizer *optionSizer = new wxBoxSizer(wxHORIZONTAL);
  optionSizer->Add(m_domTypeBox, 0, wxALL, 5);
  optionSizer->Add(m_domMethodBox, 0, wxALL, 5);
  optionSizer->Add(m_domPrecisionBox, 0, wxALL, 5);

  wxBoxSizer *topSizer = new wxBoxSizer(wxVERTICAL);
  topSizer->Add(optionSizer, 0, wxCENTER | wxALL, 5);
  topSizer->Add(m_playerGrid, 1, wxCENTER | wxALL, 5);
  topSizer->Add(m_allBox, 1, wxCENTER | wxALL, 5);
  topSizer->Add(m_buttonSizer, 0, wxCENTER | wxALL, 5);

  SetSizer(topSizer);
  topSizer->Fit(this);
  topSizer->SetSizeHints(this);
  Layout();

  wxCommandEvent event;
  OnMethod(event);
}

dialogElimMixed::~dialogElimMixed()
{ }

void dialogElimMixed::OnOK(wxCommandEvent &p_event)
{
  wxConfig config("Gambit");
  config.Write("Solutions/ElimDom-Mixed-All", (long) m_allBox->GetValue());
  config.Write("Solutions/ElimDom-Mixed-Type",
	       (long) m_domTypeBox->GetSelection());
  config.Write("Solutions/ElimDom-Mixed-Method",
	       (long) m_domMethodBox->GetSelection());
  config.Write("Solutions/ElimDom-Mixed-Precision",
	       (long) m_domPrecisionBox->GetSelection());
  p_event.Skip();
}

gArray<int> dialogElimMixed::Players(void) const
{
  gBlock<int> players;
  for (int i = 0; i < m_playerGrid->GetRows(); i++) {
    players.Append(i+1);
  }
  return players;
}

void dialogElimMixed::OnMethod(wxCommandEvent &)
{
  m_domPrecisionBox->Enable(m_domMethodBox->GetSelection() == 1);
}

//=========================================================================
//                  class dialogElimBehav: Member functions
//=========================================================================

BEGIN_EVENT_TABLE(dialogElimBehav, guiAutoDialog)
  EVT_BUTTON(wxID_OK, dialogElimBehav::OnOK)
END_EVENT_TABLE()

dialogElimBehav::dialogElimBehav(wxWindow *p_parent, 
				 const gArray<gText> &p_players)
  : guiAutoDialog(p_parent, "Dominance Elimination Parameters")
{
  long all = 0, domType = 0, domConditional = 0;
  wxConfig config("Gambit");
  config.Read("Solutions/ElimDom-Behav-All", &all);
  config.Read("Solutions/ElimDom-Behav-Type", &domType);
  config.Read("Solutions/ElimDom-Behav-Conditional", &domConditional);

  m_allBox = new wxCheckBox(this, -1, "Eliminate iteratively");
  m_allBox->SetValue(all);

  wxString domTypeList[2] = { "Weak", "Strong" };
  m_domTypeBox = new wxRadioBox(this, -1, "Type",
				wxDefaultPosition, wxDefaultSize,
				2, domTypeList);
  if (domType == 0 || domType == 1)
    m_domTypeBox->SetSelection(domType);
  
  wxString domConditionalList[2] = { "Conditional", "Unconditional" };
  m_domConditionalBox = new wxRadioBox(this, -1, "Conditional",
				       wxDefaultPosition, wxDefaultSize,
				       2, domConditionalList);
  if (domConditional == 0 || domConditional == 1)
    m_domConditionalBox->SetSelection(domConditional);

#ifdef __WXGTK__
  m_playerBox = new wxListBox(this, -1, wxDefaultPosition, wxDefaultSize,
			      0, 0, wxLB_EXTENDED);
#else
  m_playerBox = new wxListBox(this, -1, wxDefaultPosition, wxDefaultSize,
			      0, 0, wxLB_MULTIPLE);
#endif // __WXGTK__

  for (int pl = 1; pl <= p_players.Length(); pl++) {
    m_playerBox->Append((char *) (ToText(pl) + ": " + p_players[pl]));
    m_playerBox->SetSelection(pl - 1, true);
  }

  wxBoxSizer *optionSizer = new wxBoxSizer(wxHORIZONTAL);
  optionSizer->Add(m_domTypeBox, 0, wxALL, 5);
  optionSizer->Add(m_domConditionalBox, 0, wxALL, 5);

  wxBoxSizer *topSizer = new wxBoxSizer(wxVERTICAL);
  topSizer->Add(optionSizer, 0, wxCENTER | wxALL, 5);
  topSizer->Add(m_playerBox, 0, wxCENTER | wxALL, 5);
  topSizer->Add(m_allBox, 1, wxCENTER | wxALL, 5);
  topSizer->Add(m_buttonSizer, 0, wxCENTER | wxALL, 5);

  SetSizer(topSizer);
  topSizer->Fit(this);
  topSizer->SetSizeHints(this);
  Layout();
}

dialogElimBehav::~dialogElimBehav()
{ }

void dialogElimBehav::OnOK(wxCommandEvent &p_event)
{
  wxConfig config("Gambit");
  config.Write("Solutions/ElimDom-Behav-All", (long) m_allBox->GetValue());
  config.Write("Solutions/ElimDom-Behav-Type",
	       (long) m_domTypeBox->GetSelection());
  config.Write("Solutions/ElimDom-Behav-Conditional",
	       (long) m_domConditionalBox->GetSelection());
  p_event.Skip();
}

gArray<int> dialogElimBehav::Players(void) const
{
  gBlock<int> players;
  for (int i = 1; i <= m_playerBox->Number(); i++) {
    if (m_playerBox->Selected(i-1)) {
      players.Append(i);
    }
  }
  return players;
}

//=========================================================================
//                class dialogSupportSelect: Member functions
//=========================================================================

dialogSupportSelect::dialogSupportSelect(wxWindow *p_parent,
					 const gList<NFSupport *> &p_supports,
					 NFSupport *p_current,
					 const gText &p_caption)
  : guiAutoDialog(p_parent, p_caption)
{
  m_supportList = new wxListBox(this, -1);
  for (int s = 1; s <= p_supports.Length(); s++) {
    NFSupport *support = p_supports[s];
    gText item = ToText(s) + ": " + support->GetName();

    item += (" (" + ToText(support->NumStrats(1)) + ", " +
	     ToText(support->NumStrats(2)));
    if (support->Game().NumPlayers() > 2) {
      item += ", " + ToText(support->NumStrats(3));
      if (support->Game().NumPlayers() > 3) 
	item += ",...)";
      else
	item += ")";
    }
    else
      item += ")";

    m_supportList->Append((char *) item);
    if (p_supports[s] == p_current) {
      m_supportList->SetSelection(s - 1);
    }
  }

  wxBoxSizer *topSizer = new wxBoxSizer(wxVERTICAL);
  topSizer->Add(m_supportList, 1, wxCENTER | wxALL, 5);
  topSizer->Add(m_buttonSizer, 0, wxCENTER | wxALL, 5);

  SetSizer(topSizer);
  topSizer->Fit(this);
  topSizer->SetSizeHints(this);
  Layout();
}

dialogSupportSelect::dialogSupportSelect(wxWindow *p_parent, 
					 const gList<EFSupport *> &p_supports,
					 EFSupport *p_current,
					 const gText &p_caption)
  : guiAutoDialog(p_parent, p_caption)
{
  m_supportList = new wxListBox(this, -1);
  for (int s = 1; s <= p_supports.Length(); s++) {
    EFSupport *support = p_supports[s];
    gText item = ToText(s) + ": " + support->GetName();
    m_supportList->Append((char *) item);
    if (p_supports[s] == p_current) {
      m_supportList->SetSelection(s - 1);
    }
  }

  wxBoxSizer *topSizer = new wxBoxSizer(wxVERTICAL);
  topSizer->Add(m_supportList, 1, wxCENTER | wxALL, 5);
  topSizer->Add(m_buttonSizer, 0, wxCENTER | wxALL, 5);

  SetSizer(topSizer);
  topSizer->Fit(this);
  topSizer->SetSizeHints(this);
  Layout();
}
