//
// FILE: infoset.h -- Declaration of Infoset class
//
// $Id$
//

#ifndef INFOSET_H
#define INFOSET_H

#include "basic.h"
#include "gstring.h"
#include "gset.h"

#include "branch.h"

class Infoset   {
  private:
    gString name;
    gSet<Branch *> branches;

  public:
	// CONSTRUCTORS AND DESTRUCTOR
	// create a new infoset with brs branches
    Infoset(int brs = 0);
	// copy constructor
    Infoset(const Infoset &);
	// clean up after an infoset
    ~Infoset();

	// OPERATOR OVERLOADING
	// assignment operator
    Infoset &operator=(const Infoset &);
    
	// DATA ACCESS AND MANIPULATION
	// return the infoset name
    gString GetInfosetName(void) const    { return name; }
	// set the infoset name
    void SetInfosetName(const gString &s)   { name = s; }

	// return the number of branches in the infoset
    int NumBranches(void) const   { return branches.Length(); }

	// set a branch's name
    void SetBranchName(int br, const gString &s)
      { branches[br]->SetBranchName(s); }

	// return a branch's name
    gString GetBranchName(int br) const
      { return branches[br]->GetBranchName(); }

	// remove a branch from the infoset
    void RemoveBranch(int br)
      { delete branches.Remove(br); }

	// add a branch to the infoset
    void InsertBranch(int br)
      { branches.Insert(new Branch, br); }
};


#endif   // INFOSET_H

