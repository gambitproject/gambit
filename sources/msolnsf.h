// File: bsolnsf.h -- Behav Solution Sorter and Filter
//  $Id$
#ifndef MSOLNSF_H
#define MSOLNSF_H
#include "garray.h"
#include "gslist.h"
#include "mixedsol.h"
#include "gnumber.h"
// Sorting and filtering options
class MSolnSortFilterOptions
{
private:
	int sort_by;
	gArray<bool> filter_cr,filter_nash,filter_perfect,filter_proper;
	char *defaults_file;
public:
// these variables are mostly for GUI use.  they may not belong here
	static char *filter_cr_str[];
	static NfgAlgType filter_cr_id[];
	static char *filter_tri_str[];
	static TriState filter_tri_id[];
// Contructor
	MSolnSortFilterOptions(void);
	MSolnSortFilterOptions &operator=(const MSolnSortFilterOptions &o);
	virtual ~MSolnSortFilterOptions();
// Data access, must have a const version to avoid errors/warnings
	int &SortBy(void);
	int	SortBy(void) const;
	gArray<bool> &FilterCr(void);
	const gArray<bool> &FilterCr(void) const;
	gArray<bool> &FilterNash(void);
	const gArray<bool> &FilterNash(void) const;
	gArray<bool> &FilterPerfect(void);
	const gArray<bool> &FilterPerfect(void) const;
	gArray<bool> &FilterProper(void);
	const gArray<bool> &FilterProper(void) const;
};

// Implement sorting and filtering
class MSolnSorterFilter:public gListSorter<MixedSolution>,
												public gListFilter<MixedSolution>
{
private:
	MSolnSortFilterOptions &options;
protected:
	// Overload the comparison functions
	bool Passes(const MixedSolution &a) const;
	CompareResult Compare(const MixedSolution &a,const MixedSolution &b) const;
public:
	MSolnSorterFilter(gSortList<MixedSolution> &solns,MSolnSortFilterOptions &options_);
};

#define		MSOLN_SHOW_SECT	"Mixed-Soln-Show"
#define		NUM_MCREATORS		9

#define		MSORT_BY_ID				1		// this order must correspond to that in GUI dialogs
#define		MSORT_BY_CREATOR	2
#define		MSORT_BY_NASH			3
#define		MSORT_BY_PERFECT	4
#define		MSORT_BY_PROPER			5
#define		MSORT_BY_GVALUE		6
#define		MSORT_BY_GLAMBDA	7
#define		MSORT_BY_LVALUE		8

#endif
