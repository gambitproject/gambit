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
#include "wx/notebook.h"
#include "efg.h"
#include "dlnodeproperties.h"

//======================================================================
//                          class panelNode 
//======================================================================

class panelNode : public wxPanel {
private:
  Node *m_node;
  wxTextCtrl *m_nodeName;
  wxChoice *m_outcome;

public:
  panelNode(wxWindow *, Node *);

  wxString GetNodeName(void) const { return m_nodeName->GetValue(); }
  int GetOutcome(void) const { return m_outcome->GetSelection(); }
};

panelNode::panelNode(wxWindow *p_parent, Node *p_node)
  : wxPanel(p_parent, -1), m_node(p_node)
{
  SetAutoLayout(true);

  wxBoxSizer *topSizer = new wxBoxSizer(wxVERTICAL);

  wxBoxSizer *labelSizer = new wxBoxSizer(wxHORIZONTAL);
  labelSizer->Add(new wxStaticText(this, wxID_STATIC, "Node label"),
		  0, wxALL | wxCENTER, 5);
  m_nodeName = new wxTextCtrl(this, -1, (const char *) m_node->GetName());
  labelSizer->Add(m_nodeName, 1, wxALL | wxCENTER | wxEXPAND, 5);
  topSizer->Add(labelSizer, 0, wxALL | wxEXPAND, 5);

  wxBoxSizer *outcomeSizer = new wxBoxSizer(wxHORIZONTAL);
  outcomeSizer->Add(new wxStaticText(this, wxID_STATIC, "Outcome"),
		    0, wxALL | wxCENTER, 5);
  m_outcome = new wxChoice(this, -1);
  m_outcome->Append("(null)");
  m_outcome->SetSelection(0);
  const FullEfg &efg = *p_node->Game();
  for (int outc = 1; outc <= efg.NumOutcomes(); outc++) {
    Efg::Outcome outcome = efg.GetOutcome(outc);
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

  SetSizer(topSizer);
  topSizer->Fit(this);
  topSizer->SetSizeHints(this);
  Layout();
}

//======================================================================
//                          class panelMove 
//======================================================================

const int idLISTBOX_ACTIONS = 1000;

class panelMove : public wxPanel {
private:
  Node *m_node;
  wxTextCtrl *m_infosetName, *m_actionName;
  wxListBox *m_actionList;
  gBlock<gText> m_actionNames;
  int m_lastSelection;

  void OnActionChanged(wxCommandEvent &);

public:
  panelMove(wxWindow *, Node *);

  wxString GetInfosetName(void) const { return m_infosetName->GetValue(); }
  gText GetActionName(int p_act) const { return m_actionNames[p_act]; }

  DECLARE_EVENT_TABLE()
};

BEGIN_EVENT_TABLE(panelMove, wxPanel)
  EVT_LISTBOX(idLISTBOX_ACTIONS, panelMove::OnActionChanged)
END_EVENT_TABLE()

panelMove::panelMove(wxWindow *p_parent, Node *p_node)
  : wxPanel(p_parent, -1), m_node(p_node)
{
  SetAutoLayout(true);

  wxBoxSizer *topSizer = new wxBoxSizer(wxVERTICAL);

  wxBoxSizer *labelSizer = new wxBoxSizer(wxHORIZONTAL);
  labelSizer->Add(new wxStaticText(this, wxID_STATIC, "Information set label"),
		  0, wxALL | wxCENTER, 5);
  m_infosetName = new wxTextCtrl(this, -1,
				  (const char *) m_node->GetInfoset()->GetName());
  labelSizer->Add(m_infosetName, 1, wxALL | wxCENTER | wxEXPAND, 5);
  topSizer->Add(labelSizer, 0, wxALL | wxEXPAND, 5);

  wxStaticBoxSizer *actionBoxSizer =
    new wxStaticBoxSizer(new wxStaticBox(this, -1, "Actions"), wxHORIZONTAL);
 
  m_actionList = new wxListBox(this, idLISTBOX_ACTIONS);
  for (int act = 1; act <= p_node->GetInfoset()->NumActions(); act++) {
    m_actionList->Append((const char *)
			 (ToText(act) + ": " + 
			  p_node->GetInfoset()->GetAction(act)->GetName()));
    m_actionNames.Append(p_node->GetInfoset()->GetAction(act)->GetName());
  }
  actionBoxSizer->Add(m_actionList, 0, wxALL, 5);

  wxBoxSizer *editSizer = new wxBoxSizer(wxVERTICAL);
  editSizer->Add(new wxStaticText(this, wxID_STATIC, "Action name"),
		 0, wxALL, 5);
  m_actionName = new wxTextCtrl(this, -1, "");
  editSizer->Add(m_actionName, 0, wxALL | wxEXPAND, 5);
  m_actionList->SetSelection(0);
  m_lastSelection = 0;
  m_actionName->SetValue((const char *) m_actionNames[1]);

  actionBoxSizer->Add(editSizer, 0, wxALL | wxEXPAND, 5);
  topSizer->Add(actionBoxSizer, 0, wxALL | wxEXPAND, 5);

  SetSizer(topSizer);
  topSizer->Fit(this);
  topSizer->SetSizeHints(this);
  Layout();
}

void panelMove::OnActionChanged(wxCommandEvent &)
{
  m_actionNames[m_lastSelection+1] = m_actionName->GetValue().c_str();
  m_actionList->SetString(m_lastSelection,
			  (const char *) ((ToText(m_lastSelection+1) + ": " +
					   m_actionName->GetValue().c_str())));
  m_actionName->SetValue((const char *)
			 m_actionNames[m_actionList->GetSelection() + 1]);
  m_lastSelection = m_actionList->GetSelection();
}

//======================================================================
//                      class dialogNodeProperties
//======================================================================

dialogNodeProperties::dialogNodeProperties(wxWindow *p_parent, Node *p_node)
  : wxDialog(p_parent, -1, "Node properties"), m_node(p_node)
{
  SetAutoLayout(true);

  m_notebook = new wxNotebook(this, -1);
  wxNotebookSizer *notebookSizer = new wxNotebookSizer(m_notebook);
  m_notebook->AddPage(new panelNode(m_notebook, p_node), "Node");
  if (m_node->NumChildren() > 0) {
    m_notebook->AddPage(new panelMove(m_notebook, p_node), "Move");
  }  

  wxButton *okButton = new wxButton(this, wxID_OK, "OK");
  okButton->SetDefault();
  wxButton *cancelButton = new wxButton(this, wxID_CANCEL, "Cancel");
  wxButton *helpButton = new wxButton(this, wxID_HELP, "Help");

  wxBoxSizer *buttonSizer = new wxBoxSizer(wxHORIZONTAL);
  buttonSizer->Add(okButton, 0, wxALL, 5);
  buttonSizer->Add(cancelButton, 0, wxALL, 5);
  buttonSizer->Add(helpButton, 0, wxALL, 5);

  wxBoxSizer *topSizer = new wxBoxSizer(wxVERTICAL);
  topSizer->Add(notebookSizer, 1, wxALL | wxEXPAND, 5);
  topSizer->Add(buttonSizer, 0, wxALL | wxCENTER, 5);

  SetSizer(topSizer);
  topSizer->Fit(this);
  topSizer->SetSizeHints(this);

  Layout();
}

wxString dialogNodeProperties::GetNodeName(void) const
{
  return ((panelNode *) m_notebook->GetPage(0))->GetNodeName();
}

int dialogNodeProperties::GetOutcome(void) const
{
  return ((panelNode *) m_notebook->GetPage(0))->GetOutcome();
}

wxString dialogNodeProperties::GetInfosetName(void) const
{
  return ((panelMove *) m_notebook->GetPage(1))->GetInfosetName();
}

gText dialogNodeProperties::GetActionName(int p_act) const
{
  return ((panelMove *) m_notebook->GetPage(1))->GetActionName(p_act);
}
