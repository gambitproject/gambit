//
// FILE: behavfilter.h -- Behavior profile sorter/filter
//
// $Id$
//

#ifndef BEHAVFILTER_H
#define BEHAVFILTER_H

#include "wx/wx.h"

#include "garray.h"
#include "gslist.h"
#include "behavsol.h"

class BehavListFilter : public gListSorter<BehavSolution>,
			public gListFilter<BehavSolution> {
private:
  int m_sortBy;
  gArray<bool> m_filterCreator;
  gArray<bool> m_filterNash, m_filterPerfect, m_filterSequential;

protected:
  // Overload the comparison functions
  bool Passes(const BehavSolution &a) const;
  bool LessThan(const BehavSolution &a, const BehavSolution &b) const;

public:
  BehavListFilter(void);
  virtual ~BehavListFilter() { }

  int &SortBy(void);
  int SortBy(void) const;
  gArray<bool> &FilterCreator(void);
  const gArray<bool> &FilterCreator(void) const;
  gArray<bool> &FilterNash(void);
  const gArray<bool> &FilterNash(void) const;
  gArray<bool> &FilterPerfect(void);
  const gArray<bool> &FilterPerfect(void) const;
  gArray<bool> &FilterSequential(void);
  const gArray<bool> &FilterSequential(void) const;
};

#define     NUM_BCREATORS       16

#define     BSORT_BY_ID         1       // this order must correspond to that in GUI dialogs
#define     BSORT_BY_CREATOR    2
#define     BSORT_BY_NASH       3
#define     BSORT_BY_PERFECT    4
#define     BSORT_BY_SEQ        5
#define     BSORT_BY_GVALUE     6
#define     BSORT_BY_GLAMBDA    7
#define     BSORT_BY_LVALUE     8

class dialogBehavFilter : public wxDialog {
private:
  wxRadioBox *m_sortBy;
  wxListBox *m_filterCreator;
  wxListBox *m_filterNash, *m_filterPerfect, *m_filterSequential;

public:
  dialogBehavFilter(wxWindow *, const BehavListFilter &);
  virtual ~dialogBehavFilter() { }

  void Update(BehavListFilter &);
};

#endif  // BEHAVFILTER_H
