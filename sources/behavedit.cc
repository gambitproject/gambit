//
// FILE: behavedit.cc -- Dialog for editing behavior profiles
//
// $Id$
//

#include "wx/wxprec.h"
#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif  // WX_PRECOMP
#include "behavsol.h"
#include "behavedit.h"

//-------------------------------------------------------------------------
//                class dialogBehavEditor: Member functions
//-------------------------------------------------------------------------

const int idINFOSET_TREE = 2001;
const int idPROB_GRID = 2002;

class ProbGrid : public wxGrid {
private:
  // Event handlers
  void OnKeyDown(wxKeyEvent &);

public:
  ProbGrid(wxWindow *p_parent)
    : wxGrid(p_parent, idPROB_GRID, wxPoint(5, 5), wxSize(200, 200)) { }
  virtual ~ProbGrid() { }

  DECLARE_EVENT_TABLE()
};

BEGIN_EVENT_TABLE(ProbGrid, wxGrid)
  EVT_KEY_DOWN(ProbGrid::OnKeyDown)
END_EVENT_TABLE()

void ProbGrid::OnKeyDown(wxKeyEvent &p_event)
{
  // This is here only to keep the program from crashing under MSW
  if (!IsCellEditControlEnabled()) {
    EnableCellEditControl();
    ShowCellEditControl();
  }
}


BEGIN_EVENT_TABLE(dialogBehavEditor, wxDialog)
  EVT_TREE_ITEM_COLLAPSING(idINFOSET_TREE, dialogBehavEditor::OnItemCollapsing)
  EVT_TREE_SEL_CHANGING(idINFOSET_TREE, dialogBehavEditor::OnSelChanging)
  EVT_TREE_SEL_CHANGED(idINFOSET_TREE, dialogBehavEditor::OnSelChanged)
  EVT_BUTTON(wxID_OK, dialogBehavEditor::OnOK)
END_EVENT_TABLE()

dialogBehavEditor::dialogBehavEditor(wxWindow *p_parent,
				     const BehavSolution &p_profile)
  : wxDialog(p_parent, -1, "Edit behavior profile"),
    m_profile(p_profile), m_map((Infoset *) 0)
{
  m_infosetTree = new wxTreeCtrl(this, idINFOSET_TREE,
				 wxPoint(5, 5), wxSize(200, 200));
  m_infosetTree->AddRoot((char *) p_profile.GetName());

  Infoset *firstInfoset = p_profile.GetGame().Infosets()[1];
  wxTreeItemId firstID;

  for (int pl = 1; pl <= p_profile.GetGame().NumPlayers(); pl++) {
    EFPlayer *player = p_profile.GetGame().Players()[pl];
    wxTreeItemId id = m_infosetTree->AppendItem(m_infosetTree->GetRootItem(),
						(char *) player->GetName());
    for (int iset = 1; iset <= player->NumInfosets(); iset++) {
      wxTreeItemId isetID = m_infosetTree->AppendItem(id,
					      (char *) player->Infosets()[iset]->GetName());
      m_map.Define(isetID, player->Infosets()[iset]);
      if (player->Infosets()[iset] == firstInfoset) {
	firstID = isetID;
      }
    }
    m_infosetTree->Expand(id);
  }
  m_infosetTree->Expand(m_infosetTree->GetRootItem());
  /*  long cookie;
  m_infosetTree->Expand(m_infosetTree->GetFirstChild(m_infosetTree->GetRootItem(), cookie));
  for (int pl = 1; m_profile.Game().Players()[pl] != firstInfoset->GetPlayer();
       pl++) {
    m_infosetTree->Expand(m_infosetTree->GetNextChild(m_infosetTree->GetRootItem(), cookie));
  }
  */
  // This seems to cause a crash...
  // m_infosetTree->SelectItem(firstID);

  m_probGrid = new ProbGrid(this);
  m_probGrid->CreateGrid(firstInfoset->NumActions(), 1);
  m_probGrid->SetLabelValue(wxHORIZONTAL, "Probability", 0);
  for (int act = 1; act <= firstInfoset->NumActions(); act++) {
    m_probGrid->SetLabelValue(wxVERTICAL,
			      (char *) firstInfoset->Actions()[act]->GetName(),
			      act - 1);
    m_probGrid->SetCellValue((char *) ToText(p_profile(firstInfoset->Actions()[act])),
			     act - 1, 0);
  }
  m_probGrid->UpdateDimensions();
  m_probGrid->Refresh();

  wxBoxSizer *editSizer = new wxBoxSizer(wxHORIZONTAL);
  editSizer->Add(m_infosetTree, 0, wxALL, 5);
  editSizer->Add(m_probGrid, 0, wxALL, 5);

  wxButton *okButton = new wxButton(this, wxID_OK, "OK");
  okButton->SetDefault();
  wxButton *cancelButton = new wxButton(this, wxID_CANCEL, "Cancel");
  wxButton *helpButton = new wxButton(this, wxID_HELP, "Help");

  wxBoxSizer *buttonSizer = new wxBoxSizer(wxHORIZONTAL);
  buttonSizer->Add(okButton, 0, wxALL, 5);
  buttonSizer->Add(cancelButton, 0, wxALL, 5);
  buttonSizer->Add(helpButton, 0, wxALL, 5);

  wxBoxSizer *topSizer = new wxBoxSizer(wxVERTICAL);
  topSizer->Add(editSizer, 0, wxEXPAND | wxALL, 5);
  topSizer->Add(buttonSizer, 0, wxCENTER | wxALL, 5);

  SetAutoLayout(true);
  SetSizer(topSizer);
  topSizer->Fit(this);
  topSizer->SetSizeHints(this);

  Layout();
}

dialogBehavEditor::~dialogBehavEditor()
{ }

void dialogBehavEditor::OnItemCollapsing(wxTreeEvent &p_event)
{
  if (p_event.GetItem() == m_infosetTree->GetRootItem()) {
    p_event.Veto();
  }
}

void dialogBehavEditor::OnSelChanging(wxTreeEvent &p_event)
{
  if (!m_map.IsDefined(p_event.GetItem())) {
    p_event.Veto();
  }
}

void dialogBehavEditor::OnSelChanged(wxTreeEvent &p_event)
{
  if (m_probGrid->IsCellEditControlEnabled()) {
    m_probGrid->SaveEditControlValue();
    m_probGrid->HideCellEditControl();
  }

  Infoset *oldInfoset = m_map.Lookup(p_event.GetOldItem());

  if (!oldInfoset) {
    oldInfoset = m_profile.GetGame().Infosets()[1];
  }

  for (int act = 1; act <= oldInfoset->NumActions(); act++) {
    m_profile.Set(oldInfoset->Actions()[act],
		  ToNumber(m_probGrid->GetCellValue(act - 1, 0).c_str()));
  }

  Infoset *infoset = m_map.Lookup(p_event.GetItem());

  if (oldInfoset->NumActions() > infoset->NumActions()) {
    m_probGrid->DeleteRows(0,
			   oldInfoset->NumActions() - infoset->NumActions());
  }
  else if (oldInfoset->NumActions() < infoset->NumActions()) {
    m_probGrid->InsertRows(0,
			   infoset->NumActions() - oldInfoset->NumActions());
  }

  if (infoset) {
    for (int act = 1; act <= infoset->NumActions(); act++) {
      m_probGrid->SetLabelValue(wxVERTICAL,
				(char *) infoset->Actions()[act]->GetName(),
				act - 1);
      m_probGrid->SetCellValue((char *) ToText(m_profile(infoset->Actions()[act])),
			       act - 1, 0);
    }
  }
}

void dialogBehavEditor::OnOK(wxCommandEvent &p_event)
{
  if (m_probGrid->IsCellEditControlEnabled()) {
    m_probGrid->SaveEditControlValue();
    m_probGrid->HideCellEditControl();
  }

  Infoset *infoset = m_map.Lookup(m_infosetTree->GetSelection());

  if (!infoset) {
    infoset = m_profile.GetGame().Infosets()[1];
  }

  for (int act = 1; act <= infoset->NumActions(); act++) {
    m_profile.Set(infoset->Actions()[act],
		  ToNumber(m_probGrid->GetCellValue(act - 1, 0).c_str()));
  }

  p_event.Skip();
}

#include "base/gmap.imp"

gOutput &operator<<(gOutput &p_output, wxTreeItemId)
{ return p_output; }

template class gBaseMap<wxTreeItemId, Infoset *>;
template class gOrdMap<wxTreeItemId, Infoset *>;
