//
// FILE: mixedfilter.h -- Mixed profile sorter/filter
//
// $Id$
// 

#ifndef MIXEDFILTER_H
#define MIXEDFILTER_H

#include "wx/wx.h"

#include "base/base.h"
#include "game/mixedsol.h"

class MixedListFilter : public gListSorter<MixedSolution>,
                        public gListFilter<MixedSolution> {
private:
  int m_sortBy;
  gArray<bool> m_filterCreator;
  gArray<bool> m_filterNash, m_filterPerfect, m_filterProper;

protected:
  // Overload the comparison functions
  bool Passes(const MixedSolution &a) const;
  bool LessThan(const MixedSolution &a, const MixedSolution &b) const;

public:
  MixedListFilter(void);
  virtual ~MixedListFilter() { }

  int &SortBy(void);
  int SortBy(void) const;
  gArray<bool> &FilterCreator(void);
  const gArray<bool> &FilterCreator(void) const;
  gArray<bool> &FilterNash(void);
  const gArray<bool> &FilterNash(void) const;
  gArray<bool> &FilterPerfect(void);
  const gArray<bool> &FilterPerfect(void) const;
  gArray<bool> &FilterProper(void);
  const gArray<bool> &FilterProper(void) const;
};

#define NUM_MCREATORS 10

typedef enum {
  MSORT_BY_NAME = 1, MSORT_BY_CREATOR = 2,
  MSORT_BY_NASH = 3, MSORT_BY_PERFECT = 4,
  MSORT_BY_PROPER = 5, MSORT_BY_GVALUE = 6,
  MSORT_BY_GLAMBDA = 7, MSORT_BY_LVALUE = 8 } MixedSortType;

class dialogMixedFilter : public wxDialog {
private:
  wxRadioBox *m_sortBy;
  wxListBox *m_filterCreator;
  wxListBox *m_filterNash, *m_filterPerfect, *m_filterProper;

public:
  dialogMixedFilter(wxWindow *, const MixedListFilter &);
  virtual ~dialogMixedFilter() { }

  void Update(MixedListFilter &);
};

#endif  // MIXEDFILTER_H
