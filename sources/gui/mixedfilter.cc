//
// FILE: mixedfilter.cc -- Implements sorting and filtering of mixed profiles
//
// $Id$
//

#include "wx/wxprec.h"
#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif  // WX_PRECOMP
#include "mixedfilter.h"

NfgAlgType filter_cr_id[NUM_MCREATORS] =
{
  algorithmNfg_USER,
  algorithmNfg_ENUMPURE, algorithmNfg_ENUMMIXED,
  algorithmNfg_LCP, algorithmNfg_LP, algorithmNfg_LIAP,
  algorithmNfg_SIMPDIV, algorithmNfg_POLENUM,
  algorithmNfg_QRE, algorithmNfg_QREALL
};

gTriState filter_tri_id[3] = { triTRUE, triFALSE, triUNKNOWN };


//-------------------------------------------------------------------------
//                 class MixedListFilter: Member functions
//-------------------------------------------------------------------------

MixedListFilter::MixedListFilter(void)
  : m_filterCreator(NUM_MCREATORS), m_filterNash(3), m_filterPerfect(3),
    m_filterProper(3)
{
  m_sortBy = MSORT_BY_NAME;

  for (int i = 1; i <= NUM_MCREATORS; i++) { 
    m_filterCreator[i] = true;
  }

  for (int i = 1; i <= 3; i++) {
    m_filterNash[i] = true;
    m_filterPerfect[i] = true;
    m_filterProper[i] = true;
  }
}

int &MixedListFilter::SortBy(void)
{ return m_sortBy; }

int MixedListFilter::SortBy(void) const
{ return m_sortBy; }

gArray<bool> &MixedListFilter::FilterCreator(void)
{ return m_filterCreator; }

const gArray<bool> &MixedListFilter::FilterCreator(void) const
{ return m_filterCreator; }

gArray<bool> &MixedListFilter::FilterNash(void)
{ return m_filterNash; }

const gArray<bool> &MixedListFilter::FilterNash(void) const
{ return m_filterNash; }

gArray<bool> &MixedListFilter::FilterPerfect(void)
{ return m_filterPerfect; }

const gArray<bool> &MixedListFilter::FilterPerfect(void) const
{ return m_filterPerfect; }

gArray<bool> &MixedListFilter::FilterProper(void)
{ return m_filterProper; }

const gArray<bool> &MixedListFilter::FilterProper(void) const
{ return m_filterProper; }

bool MixedListFilter::Passes(const MixedSolution &p_solution) const
{
  for (int i = 1; i <= NUM_MCREATORS; i++) {
    if (filter_cr_id[i] == p_solution.Creator() && !FilterCreator()[i])
      return false;
  }

  for (int i = 1; i <= 3; i++)  {
    if (filter_tri_id[i] == p_solution.IsNash() && !FilterNash()[i]) {
      return false;
    }
    
    if (filter_tri_id[i] == p_solution.IsPerfect() && !FilterPerfect()[i]) {
      return false;
    }

    if (filter_tri_id[i] == p_solution.IsProper() && !FilterProper()[i]) {
      return false;
    }
  }

  return true;
}

bool MixedListFilter::LessThan(const MixedSolution &a,
			       const MixedSolution &b) const
{
  switch (SortBy()) {
  case MSORT_BY_NAME:
    return (a.GetName() < b.GetName());

  case MSORT_BY_CREATOR:
    return (ToText(a.Creator()) < ToText(b.Creator()));

  case MSORT_BY_NASH:
    return (a.IsNash() < b.IsNash());

  case MSORT_BY_PERFECT:
    return (a.IsPerfect() < b.IsPerfect());

  case MSORT_BY_PROPER:
    return (a.IsProper() < b.IsProper());

  case MSORT_BY_GVALUE:
    return (a.QreValue() < b.QreValue());

  case MSORT_BY_GLAMBDA:
    return (a.QreLambda() < b.QreLambda());

  case MSORT_BY_LVALUE:
    return (a.LiapValue() < b.LiapValue());

  default: 
    return false;
  }

  return false;
}

//-------------------------------------------------------------------------
//                 class dialogMixedFilter: Member functions
//-------------------------------------------------------------------------

dialogMixedFilter::dialogMixedFilter(wxWindow *p_parent,
				     const MixedListFilter &p_filter)
  : wxDialog(p_parent, -1, "Sorting and Filtering Options",
	     wxDefaultPosition, wxDefaultSize)
{
  wxString sortByChoices[] =
  { "Name", "Creator", "Nash", "Perfect", "Properl",
    "QreValue", "QreLambda", "LiapValue" };

  m_sortBy = new wxRadioBox(this, -1, "Sort By",
			    wxDefaultPosition, wxDefaultSize,
			    8, sortByChoices, 2);

  wxBoxSizer *filterCreatorSizer = new wxBoxSizer(wxVERTICAL);
  filterCreatorSizer->Add(new wxStaticText(this, -1, "Creator"), 0, wxALL, 5);
  
  wxString creatorChoices[] = {
    "User Defined",
    "EnumPure", "EnumMixed", "Lcp", "Lp", "Liap", "Simpdiv", "PolEnum",
    "Qre", "QreAll"
  };

  m_filterCreator = new wxListBox(this, -1, 
				  wxDefaultPosition, wxDefaultSize,
				  NUM_MCREATORS, creatorChoices,
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

  wxBoxSizer *filterProperSizer = new wxBoxSizer(wxVERTICAL);
  filterProperSizer->Add(new wxStaticText(this, -1, "Proper"),
			     0, wxALL, 5);
  m_filterProper = new wxListBox(this, -1,
				     wxDefaultPosition, wxDefaultSize,
				     3, tristateChoices, wxLB_MULTIPLE);
  filterProperSizer->Add(m_filterProper, 0, wxALL, 5);

  wxStaticBoxSizer *filterSizer =
    new wxStaticBoxSizer(new wxStaticBox(this, -1, "Filter By"), wxHORIZONTAL);
  filterSizer->Add(filterCreatorSizer, 0, wxALL, 5);
  filterSizer->Add(filterNashSizer, 0, wxALL, 5);
  filterSizer->Add(filterPerfectSizer, 0, wxALL, 5);
  filterSizer->Add(filterProperSizer, 0, wxALL, 5);

  m_sortBy->SetSelection(p_filter.SortBy()-1);

  for (int i = 0; i < NUM_MCREATORS; i++) {
    m_filterCreator->SetSelection(i, p_filter.FilterCreator()[i+1]);
  }

  for (int i = 0; i < 3; i++) {
    m_filterNash->SetSelection(i, p_filter.FilterNash()[i+1]);
    m_filterPerfect->SetSelection(i, p_filter.FilterPerfect()[i+1]);
    m_filterProper->SetSelection(i, p_filter.FilterProper()[i+1]);
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
  topSizer->Add(m_sortBy, 0, wxCENTER, 5);
  topSizer->Add(filterSizer, 0, wxCENTER, 5);
  topSizer->Add(buttonSizer, 0, wxCENTER, 5);

  SetAutoLayout(true);
  SetSizer(topSizer);
  topSizer->Fit(this);
  topSizer->SetSizeHints(this);
}

void dialogMixedFilter::Update(MixedListFilter &p_filter)
{
  p_filter.SortBy() = m_sortBy->GetSelection()+1;

  for (int i = 1; i <= NUM_MCREATORS; i++) {
    p_filter.FilterCreator()[i] = m_filterCreator->Selected(i-1);
  }

  for (int i = 1; i <= 3; i++) {
    p_filter.FilterNash()[i] = m_filterNash->Selected(i-1);
    p_filter.FilterPerfect()[i] = m_filterPerfect->Selected(i-1);
    p_filter.FilterProper()[i] = m_filterProper->Selected(i-1);
  }
}

#include "base/gslist.imp"

template class gSortList<MixedSolution>;
template class gListSorter<MixedSolution>;
template class gListFilter<MixedSolution>;

