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
bool BSolnSorterFilter<T>::Passes(const BehavSolution<T> &a_) const
{
// casting away CONST @@
BehavSolution<T> &a=(BehavSolution<T> &)a_;

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
CompareResult BSolnSorterFilter<T>::Compare(const BehavSolution<T> &a_,const BehavSolution<T> &b_) const
{
// casting away CONST @@
BehavSolution<T> &a=(BehavSolution<T> &)a_;
BehavSolution<T> &b=(BehavSolution<T> &)b_;

switch (options.SortBy())
{
case BSORT_BY_ID:
			if (a.Id()>b.Id()) return GreaterThan;
      if (a.Id()<b.Id()) return LessThan;
			return Equal;
case BSORT_BY_CREATOR:
			if (NameEfgAlgType(a.Creator())>NameEfgAlgType(b.Creator())) return GreaterThan;
			if (NameEfgAlgType(a.Creator())<NameEfgAlgType(b.Creator())) return LessThan;
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
TEMPLATE class gListSorter<BehavSolution<gRational> >;
TEMPLATE class gListSorter<BehavSolution<double> >;
TEMPLATE class  gListFilter<BehavSolution<gRational> >;
TEMPLATE class  gListFilter<BehavSolution<double> >; 
TEMPLATE class BSolnSorterFilter<double>;
TEMPLATE class BSolnSorterFilter<gRational>;


