//
// FILE: efgview.cc -- Extensive form viewing class
//
// $Id$
//

#include "wx/wxprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#include "wx/mdi.h"
#endif

#include "wx/splitter.h"
#include "wx/proplist.h"

#include "efg.h"
#include "behavsol.h"

#include "guiapp.h"
#include "efgview.h"
#include "nfgview.h"
#include "efgtree.h"
#include "efginfopanel.h"
#include "efgsolutions.h"

class guiEfgOutcomePropertyView : public wxPropertyListView {
private:
  EFOutcome *m_outcome;
  guiEfgView *m_parent;

  void OnPropertyChanged(wxProperty *);

public:
  guiEfgOutcomePropertyView(guiEfgView *p_parent, EFOutcome *p_outcome)
    : wxPropertyListView(0, wxPROP_PULLDOWN | wxPROP_SHOWVALUES |
			 wxPROP_DYNAMIC_VALUE_FIELD),
      m_outcome(p_outcome), m_parent(p_parent)
    { }
  virtual ~guiEfgOutcomePropertyView() { }
};

void guiEfgOutcomePropertyView::OnPropertyChanged(wxProperty *p_property)
{
  gText propertyName = p_property->GetName().c_str();

  if (propertyName == "Label") {
    m_outcome->SetName(p_property->GetValue().StringValue());
  }
  else {
    gRational playerNumber;
    FromText(propertyName, playerNumber);
    m_outcome->BelongsTo()->SetPayoff(m_outcome, (int) playerNumber,
				      p_property->GetValue().RealValue());
  }
  m_parent->OnOutcomeChanged(m_outcome);
}
      
guiEfgOutcomeProperties::guiEfgOutcomeProperties(guiEfgView *p_parent,
						 EFOutcome *p_outcome)
  : wxPropertyListPanel(new guiEfgOutcomePropertyView(p_parent, p_outcome),
			p_parent),
    m_parent(p_parent), m_outcome(p_outcome)
{
  wxPropertySheet *sheet = new wxPropertySheet;
  sheet->AddProperty(new wxProperty("Label", (char *) m_outcome->GetName(),
				    "string"));

  FullEfg *efg = m_outcome->BelongsTo();
  for (int pl = 1; pl <= efg->NumPlayers(); pl++) {
    sheet->AddProperty(new wxProperty((char *) (ToText(pl) + ": " + efg->Players()[pl]->GetName()), (double) efg->Payoff(m_outcome, pl), "real"));
  }
  GetView()->ShowView(sheet, this);

  wxPropertyValidatorRegistry *registry = new wxPropertyValidatorRegistry;
  registry->RegisterValidator("string", new wxStringListValidator);
  registry->RegisterValidator("real", new wxRealListValidator);
  GetView()->AddRegistry(registry);
}

class guiEfgNodePropertyView : public wxPropertyListView {
private:
  Node *m_node;
  guiEfgView *m_parent;

  void OnPropertyChanged(wxProperty *);

public:
  guiEfgNodePropertyView(guiEfgView *p_parent, Node *p_node)
    : wxPropertyListView(0, wxPROP_PULLDOWN | wxPROP_SHOWVALUES |
			 wxPROP_DYNAMIC_VALUE_FIELD),
      m_node(p_node), m_parent(p_parent)
    { }
  virtual ~guiEfgNodePropertyView() { }
};

void guiEfgNodePropertyView::OnPropertyChanged(wxProperty *p_property)
{
  gText propertyName = p_property->GetName().c_str();

  if (propertyName == "Label") {
    m_node->SetName(p_property->GetValue().StringValue());
  }

  m_parent->OnNodeChanged(m_node);
}
      
guiEfgNodeProperties::guiEfgNodeProperties(guiEfgView *p_parent,
					   Node *p_node)
  : wxPropertyListPanel(new guiEfgNodePropertyView(p_parent, p_node),
			p_parent),
    m_parent(p_parent), m_node(p_node)
{
  wxPropertySheet *sheet = new wxPropertySheet;
  sheet->AddProperty(new wxProperty("Label", (char *) m_node->GetName(),
				    "string"));
  GetView()->ShowView(sheet, this);

  wxPropertyValidatorRegistry *registry = new wxPropertyValidatorRegistry;
  registry->RegisterValidator("string", new wxStringListValidator);
  registry->RegisterValidator("real", new wxRealListValidator);
  GetView()->AddRegistry(registry);
}

BEGIN_EVENT_TABLE(guiEfgView, wxWindow)
  EVT_SIZE(OnSize)
END_EVENT_TABLE()

guiEfgView::guiEfgView(guiEfgFrame *p_parent, FullEfg *p_efg,
		       wxWindow *p_solutionSplitter)
  : wxPanel(p_solutionSplitter, -1),
    m_parent(p_parent), m_efg(p_efg), m_copyNode(0), m_copyOutcome(0)
{
  m_tree = new guiEfgTree(this, *p_efg);
  m_tree->Show(true);
  m_infoPanel = new guiEfgInfoPanel(this, *p_efg);
  m_infoPanel->Show(true);

  m_outcomeProps = 0;
  m_nodeProps = 0;

  Show(true);
}

void guiEfgView::OnSize(wxSizeEvent &p_event)
{
  Arrange();
}

void guiEfgView::OnCopy(void)
{
  if (m_tree->SelectedNode()) {
    m_copyNode = m_tree->SelectedNode();
    m_copyOutcome = 0;
  }
  else if (m_tree->SelectedOutcome()) {
    m_copyNode = 0;
    m_copyOutcome = m_tree->SelectedOutcome();
  }
}

void guiEfgView::OnPaste(void)
{
  if (m_copyNode) {
    Node *currentNode = m_tree->SelectedNode();

    if (!currentNode) {
      return;
    }
    
    if (currentNode->NumChildren() > 0) {
      return;
    }
    
    m_efg->CopyTree(m_copyNode, currentNode);
    m_tree->OnTreeChanged();
  }
  else if (m_copyOutcome) {
    Node *currentNode = m_tree->SelectedNode();

    if (!currentNode) {
      return;
    }

    currentNode->SetOutcome(m_copyOutcome);
    m_tree->OnTreeChanged();
  }
}

void guiEfgView::OnDelete(void)
{
}

void guiEfgView::OnProperties(void)
{
}

void guiEfgView::OnOutcomeChanged(EFOutcome *p_outcome)
{
  m_tree->OnOutcomeChanged(p_outcome);
}

void guiEfgView::OnNodeChanged(Node *p_node)
{
  m_tree->OnNodeChanged(p_node);
}

void guiEfgView::Arrange(void)
{
  int width, height;
  GetClientSize(&width, &height);
  if (m_outcomeProps) {
    m_outcomeProps->SetSize(0, 0, 200, height);
  }
  else if (m_nodeProps) {
    m_nodeProps->SetSize(0, 0, 200, height);
  }
  else {
    m_infoPanel->SetSize(0, 0, 200, height);
  }
  m_tree->SetSize(200, 0, width - 200, height);
}

void guiEfgView::OnSelectedOutcome(EFOutcome *p_outcome, bool p_selected)
{
  if (p_selected) {
    m_outcomeProps = new guiEfgOutcomeProperties(this, p_outcome);
    m_outcomeProps->Show(true);
  }
  else {
    delete m_outcomeProps;
    m_outcomeProps = 0;
  }
  m_infoPanel->Show(!p_selected);
  Arrange();
}

void guiEfgView::OnSelectedNode(Node *p_node, bool p_selected)
{
  if (p_selected) {
    m_nodeProps = new guiEfgNodeProperties(this, p_node);
    m_nodeProps->Show(true);
  }
  else {
    delete m_nodeProps;
    m_nodeProps = 0;
  }
  m_infoPanel->Show(!p_selected);
  Arrange();
}

void guiEfgView::ShowWindows(bool p_show)
{
  m_tree->Show(p_show);
  m_infoPanel->Show(p_show);
}

double guiEfgView::GetZoom(void) const
{
  return m_tree->GetZoom();
}

void guiEfgView::SetZoom(double p_zoom)
{
  m_tree->SetZoom(p_zoom);
}

