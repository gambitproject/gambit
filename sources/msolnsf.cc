//
// FILE: msolnsf.cc -- Implements sorting and filtering of mixed solutions
//
// $Id$
//

#include "msolnsf.h"

/****************************************************************************
                     MIXED SOLUTION SORTER FILTER OPTIONS
****************************************************************************/

// All possible algorithms that could create a behav solution and their
// corresponding ids.  Note that the indexing should start from 1, not 0.

wxString MSolnSortFilterOptions::filter_cr_str[NUM_MCREATORS+1] =
{
  "Error", "User",
  "EnumPure", "EnumMixed", "Lcp", "Lp", "Liap", "Simpdiv", "PolEnum",
  "Qre", "QreAll"
};


NfgAlgType MSolnSortFilterOptions::filter_cr_id[NUM_MCREATORS+1] =
{
  algorithmNfg_USER, algorithmNfg_USER,
  algorithmNfg_ENUMPURE, algorithmNfg_ENUMMIXED,
  algorithmNfg_LCP, algorithmNfg_LP, algorithmNfg_LIAP,
  algorithmNfg_SIMPDIV, algorithmNfg_POLENUM,
  algorithmNfg_QRE, algorithmNfg_QREALL
};

wxString MSolnSortFilterOptions::filter_tri_str[3+1] = { "E", "Yes", "No", "DK" };

gTriState MSolnSortFilterOptions::filter_tri_id[3+1] = { triUNKNOWN, triTRUE, triFALSE, triUNKNOWN };


// Constructor
MSolnSortFilterOptions::MSolnSortFilterOptions(void)
    : filter_cr(NUM_MCREATORS), filter_nash(3), filter_perfect(3), filter_proper(3)
{
    sort_by = MSORT_BY_ID;
    int i;

    for (i = 1; i <= NUM_MCREATORS; i++) 
        filter_cr[i] = true;

    for (i = 1; i <= 3; i++)
    {
        filter_nash[i]    = true;
        filter_perfect[i] = true;
        filter_proper[i]  = true;
    }
}


MSolnSortFilterOptions &MSolnSortFilterOptions::operator=(const MSolnSortFilterOptions &o)
{
    sort_by        = o.sort_by;
    filter_cr      = o.filter_cr;
    filter_nash    = o.filter_nash;
    filter_perfect = o.filter_perfect;
    filter_proper  = o.filter_proper;

    return (*this);
}


MSolnSortFilterOptions::~MSolnSortFilterOptions()
{ }


// Data Access
int &MSolnSortFilterOptions::SortBy(void)
{
    return sort_by;
}


int MSolnSortFilterOptions::SortBy(void) const
{
    return sort_by;
}


gArray<bool> &MSolnSortFilterOptions::FilterCr(void)
{
    return filter_cr;
}


const gArray<bool> &MSolnSortFilterOptions::FilterCr(void) const
{
    return filter_cr;
}


gArray<bool> &MSolnSortFilterOptions::FilterNash(void)
{
    return filter_nash;
}


const gArray<bool> &MSolnSortFilterOptions::FilterNash(void) const
{
    return filter_nash;
}


gArray<bool> &MSolnSortFilterOptions::FilterPerfect(void)
{
    return filter_perfect;
}


const gArray<bool> &MSolnSortFilterOptions::FilterPerfect(void) const
{
    return filter_perfect;
}


gArray<bool> &MSolnSortFilterOptions::FilterProper(void)
{
    return filter_proper;
}


const gArray<bool> &MSolnSortFilterOptions::FilterProper(void) const
{
    return filter_proper;
}


/****************************************************************************
                       BEHAV SOLUTION SORTER FILTER
****************************************************************************/

// Constructor

MSolnSorterFilter::MSolnSorterFilter(gSortList<MixedSolution> &solns, 
                                     MSolnSortFilterOptions &options_)
  : options(options_)
{
    Filter(solns);
    Sort(solns);
}


// Filtering function

bool MSolnSorterFilter::Passes(const MixedSolution &a) const
{
    int i;

    for (i = 1; i <= NUM_MCREATORS; i++)
    {
        if (options.filter_cr_id[i] == a.Creator() && !options.FilterCr()[i])
            return false;
    }

    for (i = 1; i <= 3; i++)
    {
        if (options.filter_tri_id[i] == a.IsNash() && !options.FilterNash()[i])
            return false;

        if (options.filter_tri_id[i] == a.IsPerfect() && !options.FilterPerfect()[i])
            return false;

        if (options.filter_tri_id[i] == a.IsProper() && !options.FilterProper()[i])
            return false;
    }

    return true;
}


// Sorting function

bool MSolnSorterFilter::LessThan(const MixedSolution &a,
                                 const MixedSolution &b) const
{
    switch (options.SortBy())
    {
    case MSORT_BY_ID:
        return (a.Id() < b.Id());

    case MSORT_BY_CREATOR:
        return (NameNfgAlgType(a.Creator()) < NameNfgAlgType(b.Creator()));

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
        assert(0 && "Unknown sort criterion");
    }

    return false;
}


#include "gnumber.h"
#include "gslist.imp"

template class gSortList<MixedSolution>;
template class gListSorter<MixedSolution>;
template class gListFilter<MixedSolution>;

