//
// FILE: bsolnsf.h -- Behavior solution sorter and filter
//
// $Id$
//

#ifndef BSOLNSF_H
#define BSOLNSF_H

#include "garray.h"
#include "gslist.h"
#include "behavsol.h"

// Sorting and filtering options
class BSolnSortFilterOptions
{
private:
    int sort_by;
    gArray<bool> filter_cr, filter_nash, filter_perfect, filter_seq;
    char *defaults_file;

public:
    // these variables are mostly for GUI use.  they may not belong here
    static char *filter_cr_str[];
    static EfgAlgType   filter_cr_id[];
    static char *filter_tri_str[];
    static gTriState  filter_tri_id[];

    // Contructor
    BSolnSortFilterOptions(void);
    BSolnSortFilterOptions &operator=(const BSolnSortFilterOptions &o);
    virtual ~BSolnSortFilterOptions();

    // Data access, must have a const version to avoid errors/warnings
    int &SortBy(void);
    int SortBy(void) const;
    gArray<bool> &FilterCr(void);
    const gArray<bool> &FilterCr(void) const;
    gArray<bool> &FilterNash(void);
    const gArray<bool> &FilterNash(void) const;
    gArray<bool> &FilterPerfect(void);
    const gArray<bool> &FilterPerfect(void) const;
    gArray<bool> &FilterSeq(void);
    const gArray<bool> &FilterSeq(void) const;
};

// Implement sorting and filtering
class BSolnSorterFilter : public gListSorter<BehavSolution>,
                          public gListFilter<BehavSolution>
{
private:
    BSolnSortFilterOptions &options;

protected:
    // Overload the comparison functions
    bool Passes(const BehavSolution &a) const;
    bool LessThan(const BehavSolution &a, const BehavSolution &b) const;

public:
    BSolnSorterFilter(gSortList<BehavSolution> &solns,
                      BSolnSortFilterOptions &options_);
};

#define     BSOLN_SHOW_SECT "Behav-Soln-Show"
#define     NUM_BCREATORS       16

#define     BSORT_BY_ID         1       // this order must correspond to that in GUI dialogs
#define     BSORT_BY_CREATOR    2
#define     BSORT_BY_NASH       3
#define     BSORT_BY_PERFECT    4
#define     BSORT_BY_SEQ        5
#define     BSORT_BY_GVALUE     6
#define     BSORT_BY_GLAMBDA    7
#define     BSORT_BY_LVALUE     8

#endif
