//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Dialog for viewing and editing properties of a node
//

#include "wx/wxprec.h"
#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif  // WX_PRECOMP
#include "game/efg.h"
#include "dleditnode.h"

//======================================================================
//                      class dialogEditNode
//======================================================================

dialogEditNode::dialogEditNode(wxWindow *p_parent, Node *p_node)
  : wxDialog(p_parent, -1, "Node properties"), m_node(p_node)
{
  SetAutoLayout(true);

  wxBoxSizer *topSizer = new wxBoxSizer(wxVERTICAL);

  wxBoxSizer *labelSizer = new wxBoxSizer(wxHORIZONTAL);
  labelSizer->Add(new wxStaticText(this, wxID_STATIC, "Node label"),
		  0, wxALL | wxCENTER, 5);
  m_nodeName = new wxTextCtrl(this, -1, (const char *) m_node->GetName());
  labelSizer->Add(m_nodeName, 1, wxALL | wxCENTER | wxEXPAND, 5);
  topSizer->Add(labelSizer, 0, wxALL | wxEXPAND, 5);

  wxBoxSizer *infosetSizer = new wxBoxSizer(wxHORIZONTAL);
  infosetSizer->Add(new wxStaticText(this, wxID_STATIC, "Information set"),
		    0, wxALL | wxCENTER, 5);
  m_infoset = new wxChoice(this, -1);
  if (p_node->NumChildren() > 0) {
    m_infoset->Append("New information set");
    gBlock<Infoset *> infosets = p_node->Game()->Infosets();
    int selection = 0;
    for (int iset = 1; iset <= infosets.Length(); iset++) {
      if (!infosets[iset]->IsChanceInfoset() &&
	  infosets[iset]->NumActions() == p_node->NumChildren()) {
	m_infosetList.Append(infosets[iset]);
	m_infoset->Append(wxString::Format("Player %d, Infoset %d",
			  infosets[iset]->GetPlayer()->GetNumber(),
			  infosets[iset]->GetNumber()));
	if (infosets[iset] == p_node->GetInfoset()) {
	  selection = m_infosetList.Length();
	}
      }
    }
    m_infoset->SetSelection(selection);
  }
  else {
    m_infoset->Append("(none)");
    m_infoset->SetSelection(0);
    m_infoset->Enable(false);
  }
  infosetSizer->Add(m_infoset, 1, wxALL | wxEXPAND, 5);
  topSizer->Add(infosetSizer, 0, wxALL | wxEXPAND, 5);

  wxBoxSizer *subgameSizer = new wxBoxSizer(wxVERTICAL);
  if (!p_node->GetParent()) {
    subgameSizer->Add(new wxStaticText(this, wxID_STATIC,
				       "This is the root node of the tree"),
		      0, wxALL | wxCENTER, 5);
  }
  else if (p_node->Game()->IsLegalSubgame(p_node)) {
    subgameSizer->Add(new wxStaticText(this, wxID_STATIC,
				       "This is the root of a proper subgame"),
		      0, wxALL | wxCENTER, 5);
    m_markedSubgame = new wxCheckBox(this, -1, "Subgame is marked");
    m_markedSubgame->SetValue(p_node->GetSubgameRoot() == p_node);
    subgameSizer->Add(m_markedSubgame, 0, wxALL | wxCENTER, 0);
  }
  topSizer->Add(subgameSizer, 0, wxALL | wxCENTER, 5);

  wxBoxSizer *outcomeSizer = new wxBoxSizer(wxHORIZONTAL);
  outcomeSizer->Add(new wxStaticText(this, wxID_STATIC, "Outcome"),
		    0, wxALL | wxCENTER, 5);
  m_outcome = new wxChoice(this, -1);
  m_outcome->Append("(null)");
  m_outcome->SetSelection(0);
  const efgGame &efg = *p_node->Game();
  for (int outc = 1; outc <= efg.NumOutcomes(); outc++) {
    efgOutcome *outcome = efg.GetOutcome(outc);
    gText item = ToText(outc) + ": " + efg.GetOutcomeName(outcome);
    if (item == "") {
      item = "Outcome" + ToText(outc);
    }

    item += (" (" + ToText(efg.Payoff(outcome, efg.Players()[1])) + ", " +
	     ToText(efg.Payoff(outcome, efg.Players()[2])));
    if (efg.NumPlayers() > 2) {
      item += ", " + ToText(efg.Payoff(outcome, efg.Players()[3]));
      if (efg.NumPlayers() > 3) {
	item += ",...)";
      }
      else {
	item += ")";
      }
    }
    else {
      item += ")";
    }

    m_outcome->Append((const char *) item);
    if (efg.GetOutcome(m_node) == outcome) {
      m_outcome->SetSelection(outc);
    }
  }
  outcomeSizer->Add(m_outcome, 1, wxALL | wxEXPAND, 5);
  topSizer->Add(outcomeSizer, 0, wxALL | wxEXPAND, 5);

  wxBoxSizer *buttonSizer = new wxBoxSizer(wxHORIZONTAL);
  wxButton *okButton = new wxButton(this, wxID_OK, "OK");
  okButton->SetDefault();
  buttonSizer->Add(okButton, 0, wxALL, 5);
  buttonSizer->Add(new wxButton(this, wxID_CANCEL, "Cancel"), 0, wxALL, 5);
  //  buttonSizer->Add(new wxButton(this, wxID_HELP, "Help"), 0, wxALL, 5);
  topSizer->Add(buttonSizer, 0, wxALL | wxCENTER, 5);

  SetSizer(topSizer);
  topSizer->Fit(this);
  topSizer->SetSizeHints(this);

  Layout();
}

Infoset *dialogEditNode::GetInfoset(void) const
{
  if (m_infoset->GetSelection() == 0) {
    return 0;
  }
  else {
    return m_infosetList[m_infoset->GetSelection()];
  }
}
