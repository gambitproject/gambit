// File: bsolnsf.cc -- Actual code to implement sorting and filtering of
// behav solutions
// $Id$

#include "bsolnsf.h"
#include "gsmincl.h"
/****************************************************************************
												BEHAV SOLUTION SORTER FILTER OPTIONS
****************************************************************************/

#define		BSORT_BY_NONE			1		// this order must correspond to that in GUI dialogs
#define		BSORT_BY_CREATOR	2
#define		BSORT_BY_NASH			3
#define		BSORT_BY_PERFECT	4
#define		BSORT_BY_SEQ			5
#define		BSORT_BY_GVALUE		6
#define		BSORT_BY_GLAMBDA	7
#define		BSORT_BY_LVALUE		8

// All possible algorithms that could create a behav solution and their corresponding ids
// Note that the indexing should start from 1, not 0
char *BSolnSortFilterOptions::filter_cr_str[NUM_BCREATORS+1] =
				{ "Error","Gobit","ELiap","NLiap","SeqForm","PureNash","Lemke",
					"GobitGrid","SimpDiv","Enum","ZSum","User" };
int BSolnSortFilterOptions::filter_cr_id[NUM_BCREATORS+1] =
				{-1,id_GOBIT,id_ELIAPSUB,id_NLIAPSUB,id_SEQFORMSUB,id_PURENASHSUB,
				id_LEMKESUB,id_GOBITGRID,
				id_SIMPDIVSUB,id_ENUMSUB,id_ZSUMSUB,id_USER};
char *BSolnSortFilterOptions::filter_tri_str[3+1] = {"E", "Yes", "No", "DK" };
int		BSolnSortFilterOptions::filter_tri_id[3+1] = {-1, T_YES,T_NO,T_DONTKNOW};
// Constructor
BSolnSortFilterOptions::BSolnSortFilterOptions(void):filter_cr(NUM_BCREATORS),
												filter_nash(3),filter_perfect(3),filter_seq(3)
{
sort_by=1;
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
template <class T>
BSolnSorterFilter<T>::BSolnSorterFilter(gSortList<BehavSolution<T> > &solns,BSolnSortFilterOptions &options_):
	gListSorter<BehavSolution<T> >(solns),gListFilter<BehavSolution<T> >(solns),
	options(options_)
{
Filter();
Sort();
}

// Filtering function
template <class T>
bool BSolnSorterFilter<T>::Passes(const BehavSolution<T> &a) const
{
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
template <class T>
CompareResult BSolnSorterFilter<T>::Compare(const BehavSolution<T> &a,const BehavSolution<T> &b) const
{
switch (options.SortBy())
{
case BSORT_BY_NONE:
			return Equal;
case BSORT_BY_CREATOR:
			if (NameID(a.Creator())>NameID(b.Creator())) return GreaterThan;
			if (NameID(a.Creator()<NameID(b.Creator())) return LessThan;
			return Equal;
case BSORT_BY_NASH:
			if (a.IsNash()==b.IsNash())
				return Equal;
			else
				if (a.IsNash()>b.IsNash())
					return GreaterThan;
				else
					return LessThan;
case BSORT_BY_PERFECT:
			if (a.IsSubgamePerfect()==b.IsSubgamePerfect())
				return Equal;
			else
				if (a.IsSubgamePerfect()>b.IsSubgamePerfect())
					return GreaterThan;
				else
					return LessThan;
case BSORT_BY_SEQ:
			if (a.IsSequential()==b.IsSequential())
				return Equal;
			else
				if (a.IsSequential()>b.IsSequential())
					return GreaterThan;
				else
					return LessThan;
case BSORT_BY_GVALUE:
			if (a.GobitValue()==b.GobitValue())
				return Equal;
			else
				if (a.GobitValue()>b.GobitValue())
					return GreaterThan;
				else
					return LessThan;
case BSORT_BY_GLAMBDA:
			if (a.GobitLambda()==b.GobitLambda())
				return Equal;
			else
				if (a.GobitLambda()>b.GobitLambda())
					return GreaterThan;
				else
					return LessThan;
case BSORT_BY_LVALUE:
			if (a.LiapValue()==b.LiapValue())
				return Equal;
			else
				if (a.LiapValue()>b.LiapValue())
					return GreaterThan;
				else
					return LessThan;
default: assert(0 && "Unknown sort criterion");
}
return Equal;
}

#ifdef __GNUG__
#define TEMPLATE template
#elif defined __BORLANDC__
#pragma option -Jgd
#define TEMPLATE
#endif   // __GNUG__, __BORLANDC__
#include "rational.h"
#include "gslist.imp"
TEMPLATE class gSortList<BehavSolution<double> >;
TEMPLATE class gSortList<BehavSolution<gRational> >;

TEMPLATE class BSolnSorterFilter<double>;
TEMPLATE class BSolnSorterFilter<gRational>;

