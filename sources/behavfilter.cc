//
// FILE: behavfilter.cc -- Implements sorting/filtering of behavior profiles
//
// $Id$
//

#include "behavfilter.h"

static EfgAlgType filter_cr_id[NUM_BCREATORS] =
{
  algorithmEfg_USER,
  algorithmEfg_ENUMPURE_EFG, algorithmEfg_ENUMPURE_NFG,
  algorithmEfg_ENUMMIXED_NFG,
  algorithmEfg_LCP_EFG, algorithmEfg_LCP_NFG,
  algorithmEfg_LP_EFG, algorithmEfg_LP_NFG,
  algorithmEfg_LIAP_EFG, algorithmEfg_LIAP_NFG,
  algorithmEfg_SIMPDIV_NFG,
  algorithmEfg_POLENUM_EFG, algorithmEfg_POLENUM_NFG,
  algorithmEfg_QRE_EFG, algorithmEfg_QRE_NFG,
  algorithmEfg_QREALL_NFG
};

static gTriState filter_tri_id[3] = 
{ triTRUE, triFALSE, triUNKNOWN };

//-------------------------------------------------------------------------
//                 class BehavListFilter: Member functions
//-------------------------------------------------------------------------

BehavListFilter::BehavListFilter(void)
  : m_filterCreator(NUM_BCREATORS), m_filterNash(3), 
    m_filterPerfect(3), m_filterSequential(3)
{
  m_sortBy = BSORT_NONE;

  for (int i = 1; i <= NUM_BCREATORS; i++) {
    m_filterCreator[i] = true;
  }

  for (int i = 1; i <= 3; i++) {
    m_filterNash[i] = true;
    m_filterPerfect[i] = true;
    m_filterSequential[i] = true;
  }
}
int &BehavListFilter::SortBy(void)
{ return m_sortBy; }

int BehavListFilter::SortBy(void) const
{ return m_sortBy; }

gArray<bool> &BehavListFilter::FilterCreator(void)
{ return m_filterCreator; }

const gArray<bool> &BehavListFilter::FilterCreator(void) const
{ return m_filterCreator; }

gArray<bool> &BehavListFilter::FilterNash(void)
{ return m_filterNash; }

const gArray<bool> &BehavListFilter::FilterNash(void) const
{ return m_filterNash; }

gArray<bool> &BehavListFilter::FilterPerfect(void)
{ return m_filterPerfect; }

const gArray<bool> &BehavListFilter::FilterPerfect(void) const
{ return m_filterPerfect; }

gArray<bool> &BehavListFilter::FilterSequential(void)
{ return m_filterSequential; }

const gArray<bool> &BehavListFilter::FilterSequential(void) const
{ return m_filterSequential; }

bool BehavListFilter::Passes(const BehavSolution &p_solution) const
{
  for (int i = 1; i <= NUM_BCREATORS; i++) {
    if (filter_cr_id[i-1] == p_solution.Creator() && !m_filterCreator[i]) {
      return false;
    }
  }

  for (int i = 1; i <= 3; i++) {
    if (filter_tri_id[i-1] == p_solution.IsNash() && !m_filterNash[i]) {
      return false;
    }

    if (filter_tri_id[i-1] == p_solution.IsSubgamePerfect() &&
	!m_filterPerfect[i]) {
      return false;
    }

    if (filter_tri_id[i-1] == p_solution.IsSequential() && 
	!m_filterSequential[i]) {
      return false;
    }
  }

  return true;
}

bool BehavListFilter::LessThan(const BehavSolution &a,
			       const BehavSolution &b) const
{
  switch (SortBy()) {
  case BSORT_BY_NAME:
    return (a.GetName() < b.GetName());

  case BSORT_BY_CREATOR:
    return (ToText(a.Creator()) < ToText(b.Creator()));

  case BSORT_BY_NASH:
    return (a.IsNash() < b.IsNash());

  case BSORT_BY_PERFECT:
    return (a.IsSubgamePerfect() < b.IsSubgamePerfect());

  case BSORT_BY_SEQ:
    return (a.IsSequential() < b.IsSequential());

  case BSORT_BY_GVALUE:
    return (a.QreValue() < b.QreValue());

  case BSORT_BY_GLAMBDA:
    return (a.QreLambda() < b.QreLambda());

  case BSORT_BY_LVALUE:
    return (a.LiapValue() < b.LiapValue());

  default: 
    return false;
  }

  return false;
}


//-------------------------------------------------------------------------
//                 class BehavFilterDialog: Member functions
//-------------------------------------------------------------------------

dialogBehavFilter::dialogBehavFilter(wxWindow *p_parent,
				     const BehavListFilter &p_filter)
  : wxDialog(p_parent, -1, "Sorting and Filtering Options",
	     wxDefaultPosition, wxDefaultSize)
{
  wxBoxSizer *filterCreatorSizer = new wxBoxSizer(wxVERTICAL);
  filterCreatorSizer->Add(new wxStaticText(this, -1, "Creator"), 0, wxALL, 5);
  
  wxString creatorChoices[] = {
    "User Defined",
    "EnumPure[EFG]", "EnumPure[NFG]", "EnumMixed[NFG]",
    "Lcp[EFG]", "Lcp[NFG]", "Lp[EFG]", "Lp[NFG]",
    "Liap[EFG]", "Liap[NFG]", "Simpdiv[NFG]",
    "PolEnum[EFG]", "PolEnum[NFG]",
    "Qre[EFG]", "Qre[NFG]", "QreAll[NFG]"
  };

  m_filterCreator = new wxListBox(this, -1, 
				  wxDefaultPosition, wxDefaultSize,
				  NUM_BCREATORS, creatorChoices,
				  wxLB_MULTIPLE);
  filterCreatorSizer->Add(m_filterCreator, 0, wxALL, 5);

  wxString tristateChoices[3] = { "Yes", "No", "DK" };

  wxBoxSizer *filterNashSizer = new wxBoxSizer(wxVERTICAL);
  filterNashSizer->Add(new wxStaticText(this, -1, "Nash"), 0, wxALL, 5);
  m_filterNash = new wxListBox(this, -1,
			       wxDefaultPosition, wxDefaultSize,
			       3, tristateChoices, wxLB_MULTIPLE);
  filterNashSizer->Add(m_filterNash, 0, wxALL, 5);

  wxBoxSizer *filterPerfectSizer = new wxBoxSizer(wxVERTICAL);
  filterPerfectSizer->Add(new wxStaticText(this, -1, "Perfect"), 0, wxALL, 5);
  m_filterPerfect = new wxListBox(this, -1,
				  wxDefaultPosition, wxDefaultSize,
				  3, tristateChoices, wxLB_MULTIPLE);
  filterPerfectSizer->Add(m_filterPerfect, 0, wxALL, 5);

  wxBoxSizer *filterSequentialSizer = new wxBoxSizer(wxVERTICAL);
  filterSequentialSizer->Add(new wxStaticText(this, -1, "Sequential"),
			     0, wxALL, 5);
  m_filterSequential = new wxListBox(this, -1,
				     wxDefaultPosition, wxDefaultSize,
				     3, tristateChoices, wxLB_MULTIPLE);
  filterSequentialSizer->Add(m_filterSequential, 0, wxALL, 5);

  wxStaticBoxSizer *filterSizer =
    new wxStaticBoxSizer(new wxStaticBox(this, -1, "Filter By"), wxHORIZONTAL);
  filterSizer->Add(filterCreatorSizer, 0, wxALL, 5);
  filterSizer->Add(filterNashSizer, 0, wxALL, 5);
  filterSizer->Add(filterPerfectSizer, 0, wxALL, 5);
  filterSizer->Add(filterSequentialSizer, 0, wxALL, 5);

  for (int i = 0; i < NUM_BCREATORS; i++) {
    m_filterCreator->SetSelection(i, p_filter.FilterCreator()[i+1]);
  }

  for (int i = 0; i < 3; i++) {
    m_filterNash->SetSelection(i, p_filter.FilterNash()[i+1]);
    m_filterPerfect->SetSelection(i, p_filter.FilterPerfect()[i+1]);
    m_filterSequential->SetSelection(i, p_filter.FilterSequential()[i+1]);
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
  topSizer->Add(filterSizer, 0, wxCENTER, 5);
  topSizer->Add(buttonSizer, 0, wxCENTER, 5);

  SetAutoLayout(true);
  SetSizer(topSizer);
  topSizer->Fit(this);
  topSizer->SetSizeHints(this);
}

void dialogBehavFilter::Update(BehavListFilter &p_filter)
{
  for (int i = 1; i <= NUM_BCREATORS; i++) {
    p_filter.FilterCreator()[i] = m_filterCreator->Selected(i-1);
  }

  for (int i = 1; i <= 3; i++) {
    p_filter.FilterNash()[i] = m_filterNash->Selected(i-1);
    p_filter.FilterPerfect()[i] = m_filterPerfect->Selected(i-1);
    p_filter.FilterSequential()[i] = m_filterSequential->Selected(i-1);
  }
}

#include "base/gslist.imp"

template class gSortList<BehavSolution>;
template class gListSorter<BehavSolution>;
template class gListFilter<BehavSolution>;

