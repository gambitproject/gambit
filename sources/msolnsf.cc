// File: msolnsf.cc -- Actual code to implement sorting and filtering of
// mixed solutions
//  $Id$

#include "msolnsf.h"
/****************************************************************************
												MIXED SOLUTION SORTER FILTER OPTIONS
****************************************************************************/

// All possible algorithms that could create a behav solution and their corresponding ids
// Note that the indexing should start from 1, not 0
char *MSolnSortFilterOptions::filter_cr_str[NUM_MCREATORS+1] =
				{ "Error","PureNash","Lemke","Liap","GobitGrid","Gobit",
					"Simpdiv","Enum","ZSum","User" };
NfgAlgType MSolnSortFilterOptions::filter_cr_id[NUM_MCREATORS+1] =
				{NfgAlg_USER,NfgAlg_PURENASH,NfgAlg_LEMKE,NfgAlg_LIAP,NfgAlg_GOBITGRID,NfgAlg_GOBIT,
				NfgAlg_SIMPDIV,NfgAlg_ENUM,NfgAlg_ZSUM,NfgAlg_USER};
char *MSolnSortFilterOptions::filter_tri_str[3+1] = {"E", "Yes", "No", "DK" };
TriState	MSolnSortFilterOptions::filter_tri_id[3+1] = {T_DONTKNOW, T_YES,T_NO,T_DONTKNOW};
// Constructor
MSolnSortFilterOptions::MSolnSortFilterOptions(void):filter_cr(NUM_MCREATORS),
												filter_nash(3),filter_perfect(3),filter_proper(3)
{
sort_by=MSORT_BY_ID;
int i;
for (i=1;i<=NUM_MCREATORS;i++) filter_cr[i]=true;
for (i=1;i<=3;i++)
	{filter_nash[i]=true;filter_perfect[i]=true;filter_proper[i]=true;}
}

MSolnSortFilterOptions &MSolnSortFilterOptions::operator=(const MSolnSortFilterOptions &o)
{
sort_by=o.sort_by;filter_cr=o.filter_cr;filter_nash=o.filter_nash;
filter_perfect=o.filter_perfect;filter_proper=o.filter_proper;
return (*this);
}

MSolnSortFilterOptions::~MSolnSortFilterOptions()
{ }
// Data Access
int &MSolnSortFilterOptions::SortBy(void) {return sort_by;}
int	MSolnSortFilterOptions::SortBy(void) const {return sort_by;}
gArray<bool> &MSolnSortFilterOptions::FilterCr(void) {return filter_cr;}
const gArray<bool> &MSolnSortFilterOptions::FilterCr(void) const {return filter_cr;}
gArray<bool> &MSolnSortFilterOptions::FilterNash(void) {return filter_nash;}
const gArray<bool> &MSolnSortFilterOptions::FilterNash(void) const {return filter_nash;}
gArray<bool> &MSolnSortFilterOptions::FilterPerfect(void) {return filter_perfect;}
const gArray<bool> &MSolnSortFilterOptions::FilterPerfect(void) const {return filter_perfect;}
gArray<bool> &MSolnSortFilterOptions::FilterProper(void) {return filter_proper;}
const gArray<bool> &MSolnSortFilterOptions::FilterProper(void) const {return filter_proper;}


/****************************************************************************
												BEHAV SOLUTION SORTER FILTER
****************************************************************************/
// Constructor

MSolnSorterFilter::MSolnSorterFilter(gSortList<MixedSolution > &solns,MSolnSortFilterOptions &options_):
	gListSorter<MixedSolution >(solns),gListFilter<MixedSolution >(solns),
	options(options_)
{
Filter();
Sort();
}

// Filtering function

bool MSolnSorterFilter::Passes(const MixedSolution &a) const
{
int i;
for (i=1;i<=NUM_MCREATORS;i++)
	if (options.filter_cr_id[i]==a.Creator() && !options.FilterCr()[i])
		return false;
for (i=1;i<=3;i++)
{
	if (options.filter_tri_id[i]==a.IsNash() && !options.FilterNash()[i])
		return false;
	if (options.filter_tri_id[i]==a.IsPerfect() && !options.FilterPerfect()[i])
		return false;
	if (options.filter_tri_id[i]==a.IsProper() && !options.FilterProper()[i])
		return false;
}
return true;
}

// Sorting function

CompareResult MSolnSorterFilter::Compare(const MixedSolution &a,const MixedSolution &b) const
{
switch (options.SortBy())
{
case MSORT_BY_ID:
			if (a.Id()>b.Id()) return GreaterThan;
      if (a.Id()<b.Id()) return LessThan;
			return Equal;
case MSORT_BY_CREATOR:
			if (NameNfgAlgType(a.Creator())>NameNfgAlgType(b.Creator())) return GreaterThan;
			if (NameNfgAlgType(a.Creator())<NameNfgAlgType(b.Creator())) return LessThan;
			return Equal;
case MSORT_BY_NASH:
			if (a.IsNash()==b.IsNash())
				return Equal;
			else
				if (a.IsNash()>b.IsNash())
					return GreaterThan;
				else
					return LessThan;
case MSORT_BY_PERFECT:
			if (a.IsPerfect()==b.IsPerfect())
				return Equal;
			else
				if (a.IsPerfect()>b.IsPerfect())
					return GreaterThan;
				else
					return LessThan;
case MSORT_BY_PROPER:
			if (a.IsProper()==b.IsProper())
				return Equal;
			else
				if (a.IsProper()>b.IsProper())
					return GreaterThan;
				else
					return LessThan;
case MSORT_BY_GVALUE:
			if (a.GobitValue()==b.GobitValue())
				return Equal;
			else
				if (a.GobitValue()>b.GobitValue())
					return GreaterThan;
				else
					return LessThan;
case MSORT_BY_GLAMBDA:
			if (a.GobitLambda()==b.GobitLambda())
				return Equal;
			else
				if (a.GobitLambda()>b.GobitLambda())
					return GreaterThan;
				else
					return LessThan;
case MSORT_BY_LVALUE:
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

#include "gnumber.h"
#include "gslist.imp"
template class gSortList<MixedSolution>;
template class gListSorter<MixedSolution>;
template class gListFilter<MixedSolution>;

