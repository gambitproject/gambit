// File: bsolnsf.h -- Behav Solution Sorter and Filter
// $Id$
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
	gArray<bool> filter_cr,filter_nash,filter_perfect,filter_seq;
	char *defaults_file;
public:
// these variables are mostly for GUI use.  they may not belong here
	static char *filter_cr_str[];
	static int	filter_cr_id[];
	static char *filter_tri_str[];
	static int  filter_tri_id[];
// Contructor
	BSolnSortFilterOptions(void);
	BSolnSortFilterOptions &operator=(const BSolnSortFilterOptions &o);
	virtual ~BSolnSortFilterOptions();
// Data access, must have a const version to avoid errors/warnings
	int &SortBy(void);
	int	SortBy(void) const;
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
template <class T>
class BSolnSorterFilter:public gListSorter<BehavSolution<T> >,
												public gListFilter<BehavSolution<T> >
{
private:
	BSolnSortFilterOptions &options;
protected:
	// Overload the comparison functions
	bool Passes(const BehavSolution<T> &a) const;
	CompareResult Compare(const BehavSolution<T> &a,const BehavSolution<T> &b) const;
public:
	BSolnSorterFilter(gSortList<BehavSolution<T> > &solns,BSolnSortFilterOptions &options_);
};

#define		BSOLN_SHOW_SECT	"Behav-Soln-Show"
#define		NUM_BCREATORS		6

#endif
