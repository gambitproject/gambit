// File: bsolnsf.cc -- Actual code to implement sorting and filtering of
// behav solutions
// $Id$

#include "bsolnsf.h"
/****************************************************************************
												BEHAV SOLUTION SORTER FILTER OPTIONS
****************************************************************************/

// All possible algorithms that could create a behav solution and their corresponding ids
// Note that the indexing should start from 1, not 0
char *BSolnSortFilterOptions::filter_cr_str[NUM_BCREATORS+1] =
				{ "Error","Gobit","ELiap","NLiap","SeqForm","PureNash","Lemke",
					"SimpDiv","Enum","ZSum","User" };
EfgAlgType BSolnSortFilterOptions::filter_cr_id[NUM_BCREATORS+1] =
				{EfgAlg_USER,EfgAlg_GOBIT,EfgAlg_ELIAPSUB,EfgAlg_NLIAPSUB,EfgAlg_SEQFORMSUB,EfgAlg_PURENASHSUB,
				EfgAlg_LEMKESUB,
				EfgAlg_SIMPDIVSUB,EfgAlg_ENUMSUB,EfgAlg_ZSUMSUB,EfgAlg_USER};
char *BSolnSortFilterOptions::filter_tri_str[3+1] = {"E", "Yes", "No", "DK" };
TriState BSolnSortFilterOptions::filter_tri_id[3+1] = {T_DONTKNOW, T_YES,T_NO,T_DONTKNOW};
// Constructor
BSolnSortFilterOptions::BSolnSortFilterOptions(void):filter_cr(NUM_BCREATORS),
												filter_nash(3),filter_perfect(3),filter_seq(3)
{
sort_by=BSORT_BY_ID;
int i;
for (i=1;i<=NUM_BCREATORS;i++) filter_cr[i]=true;
for (i=1;i<=3;i++)
	{filter_nash[i]=true;filter_perfect[i]=true;filter_seq[i]=true;}
}

BSolnSortFilterOptions &BSolnSortFilterOptions::operator=(const BSolnSortFilterOptions &o)
{
sort_by=o.sort_by;filter_cr=o.filter_cr;filter_nash=o.filter_nash;
filter_perfect=o.filter_perfect;filter_seq=o.filter_seq;
return (*this);
}

BSolnSortFilterOptions::~BSolnSortFilterOptions()
{ }
// Data Access
int &BSolnSortFilterOptions::SortBy(void) {return sort_by;}
int	BSolnSortFilterOptions::SortBy(void) const {return sort_by;}
gArray<bool> &BSolnSortFilterOptions::FilterCr(void) {return filter_cr;}
const gArray<bool> &BSolnSortFilterOptions::FilterCr(void) const {return filter_cr;}
gArray<bool> &BSolnSortFilterOptions::FilterNash(void) {return filter_nash;}
const gArray<bool> &BSolnSortFilterOptions::FilterNash(void) const {return filter_nash;}
gArray<bool> &BSolnSortFilterOptions::FilterPerfect(void) {return filter_perfect;}
const gArray<bool> &BSolnSortFilterOptions::FilterPerfect(void) const {return filter_perfect;}
gArray<bool> &BSolnSortFilterOptions::FilterSeq(void) {return filter_seq;}
const gArray<bool> &BSolnSortFilterOptions::FilterSeq(void) const {return filter_seq;}


/****************************************************************************
												BEHAV SOLUTION SORTER FILTER
****************************************************************************/
// Constructor

BSolnSorterFilter::BSolnSorterFilter(gSortList<BehavSolution>  &solns,BSolnSortFilterOptions &options_):
	gListSorter<BehavSolution>(solns),gListFilter<BehavSolution> (solns),
	options(options_)
{
Filter();
Sort();
}

// Filtering function

bool BSolnSorterFilter::Passes(const BehavSolution &a_) const
{
// casting away CONST @@
BehavSolution &a=(BehavSolution &)a_;

int i;
for (i=1;i<=NUM_BCREATORS;i++)
	if (options.filter_cr_id[i]==a.Creator() && !options.FilterCr()[i])
		return false;
for (i=1;i<=3;i++)
{
	if (options.filter_tri_id[i]==a.IsNash() && !options.FilterNash()[i])
		return false;
	if (options.filter_tri_id[i]==a.IsSubgamePerfect() && !options.FilterPerfect()[i])
		return false;
	if (options.filter_tri_id[i]==a.IsSequential() && !options.FilterSeq()[i])
		return false;
}
return true;
}

// Sorting function

bool BSolnSorterFilter::LessThan(const BehavSolution &a,
                                 const BehavSolution &b) const
{
  switch (options.SortBy())   {
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
    return (a.GobitValue() < b.GobitValue());
  case BSORT_BY_GLAMBDA:
    return (a.GobitLambda() < b.GobitLambda());
  case BSORT_BY_LVALUE:
    return (a.LiapValue() < b.LiapValue());
  default: assert(0 && "Unknown sort criterion");
  }

  return false;
}

#include "rational.h"
#include "gslist.imp"
template class gSortList<BehavSolution>;
template class gListSorter<BehavSolution>;
template class gListFilter<BehavSolution>;

