//
// FILE: bsolnsf.cc -- Implements sorting and filtering of behavior solutions
//
// $Id$
//

#include "bsolnsf.h"

/****************************************************************************
 *                  BEHAV SOLUTION SORTER FILTER OPTIONS
 ****************************************************************************/

// All possible algorithms that could create a behav solution and their
// corresponding ids Note that the indexing should start from 1, not 0
wxString BSolnSortFilterOptions::filter_cr_str[NUM_BCREATORS+1] =
{
  "Error", "User",
  "EnumPure[EFG]", "EnumPure[NFG]", "EnumMixed[NFG]",
  "Lcp[EFG]", "Lcp[NFG]", "Lp[EFG]", "Lp[NFG]",
  "Liap[EFG]", "Liap[NFG]", "Simpdiv[NFG]",
  "PolEnum[EFG]", "PolEnum[NFG]",
  "Qre[EFG]", "Qre[NFG]", "QreAll[NFG]"
};

EfgAlgType BSolnSortFilterOptions::filter_cr_id[NUM_BCREATORS+1] =
{
  algorithmEfg_USER, algorithmEfg_USER,
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

wxString BSolnSortFilterOptions::filter_tri_str[3+1] = 
{ "E", "Yes", "No", "DK" };

gTriState BSolnSortFilterOptions::filter_tri_id[3+1] = 
{ triUNKNOWN, triTRUE, triFALSE, triUNKNOWN };

// Constructor
BSolnSortFilterOptions::BSolnSortFilterOptions(void)
    : filter_cr(NUM_BCREATORS), filter_nash(3), 
      filter_perfect(3), filter_seq(3)
{
    sort_by = BSORT_BY_ID;
    int i;

    for (i = 1; i <= NUM_BCREATORS; i++) 
        filter_cr[i] = true;

    for (i = 1; i <= 3; i++)
    {
        filter_nash[i]    = true;
        filter_perfect[i] = true;
        filter_seq[i]     = true;
    }
}


BSolnSortFilterOptions &BSolnSortFilterOptions::operator=(const BSolnSortFilterOptions &o)
{
    sort_by        = o.sort_by;
    filter_cr      = o.filter_cr;
    filter_nash    = o.filter_nash;
    filter_perfect = o.filter_perfect;
    filter_seq     = o.filter_seq;

    return (*this);
}


BSolnSortFilterOptions::~BSolnSortFilterOptions()
{ }

// Data Access
int &BSolnSortFilterOptions::SortBy(void)
{
    return sort_by;
}

int BSolnSortFilterOptions::SortBy(void) const
{
    return sort_by;
}

gArray<bool> &BSolnSortFilterOptions::FilterCr(void)
{
    return filter_cr;
}

const gArray<bool> &BSolnSortFilterOptions::FilterCr(void) const
{
    return filter_cr;
}

gArray<bool> &BSolnSortFilterOptions::FilterNash(void)
{
    return filter_nash;
}

const gArray<bool> &BSolnSortFilterOptions::FilterNash(void) const
{
    return filter_nash;
}

gArray<bool> &BSolnSortFilterOptions::FilterPerfect(void)
{
    return filter_perfect;
}

const gArray<bool> &BSolnSortFilterOptions::FilterPerfect(void) const
{
    return filter_perfect;
}

gArray<bool> &BSolnSortFilterOptions::FilterSeq(void)
{
    return filter_seq;
}

const gArray<bool> &BSolnSortFilterOptions::FilterSeq(void) const
{
    return filter_seq;
}


/****************************************************************************
 *                       BEHAV SOLUTION SORTER FILTER
 ****************************************************************************/

// Constructor

BSolnSorterFilter::BSolnSorterFilter(gSortList<BehavSolution> &solns, 
                                     BSolnSortFilterOptions &options_)
    : gListSorter<BehavSolution>(solns), 
      gListFilter<BehavSolution> (solns),
      options(options_)
{
    Filter();
    Sort();
}


// Filtering function

bool BSolnSorterFilter::Passes(const BehavSolution &a_) const
{
    // casting away CONST @@
    BehavSolution &a = (BehavSolution &)a_;

    int i;

    for (i = 1; i <= NUM_BCREATORS; i++)
    {
        if (options.filter_cr_id[i] == a.Creator() && !options.FilterCr()[i])
            return false;
    }

    for (i = 1; i <= 3; i++)
    {
        if (options.filter_tri_id[i] == a.IsNash() && 
            !options.FilterNash()[i])
        {
            return false;
        }

        if (options.filter_tri_id[i] == a.IsSubgamePerfect() && 
            !options.FilterPerfect()[i])
        {
            return false;
        }

        if (options.filter_tri_id[i] == a.IsSequential() && 
            !options.FilterSeq()[i])
        {
            return false;
        }
    }

    return true;
}


// Sorting function

bool BSolnSorterFilter::LessThan(const BehavSolution &a,
                                 const BehavSolution &b) const
{
    switch (options.SortBy())
    {
    case BSORT_BY_ID:
        return (a.Id() < b.Id());

    case BSORT_BY_CREATOR:
        return (NameEfgAlgType(a.Creator()) < NameEfgAlgType(b.Creator()));

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
        assert(0 && "Unknown sort criterion");
    }

    return false;
}

#include "rational.h"
#include "gslist.imp"

template class gSortList<BehavSolution>;
template class gListSorter<BehavSolution>;
template class gListFilter<BehavSolution>;

