//
// FILE: efgensup.cc --Enumerate (Undominated) Subsupports of a Support
//
// $Id: efgensup.cc
//

#include "efg.h"
#include "efgciter.h"
#include "rational.h"
#include "gstatus.h"
#include "gsm.h"

class ActionCursorForSupport {
protected:
  const EFSupport *support;
  int pl;
  int iset;
  int act;

public:
  //Constructors and dtor
  ActionCursorForSupport(const EFSupport &S);
  ActionCursorForSupport(const ActionCursorForSupport &a);
  ~ActionCursorForSupport();

  // Operators
  ActionCursorForSupport &operator =(const ActionCursorForSupport &);
  bool                    operator==(const ActionCursorForSupport &) const;
  bool                    operator!=(const ActionCursorForSupport &) const;

  // Manipulation
  bool GoToNext();

  // Information
  Action *GetAction() const;
  Infoset *GetInfoset() const;
  bool IsLast() const;
};

// We build a series of functions of increasing complexity.  The
// final one, which is our goal, is the undominated support function.
// We begin by simply enumerating all subsupports.

void AllSubsupportsRECURSIVE(const EFSupport *s,
			     EFSupportWithActiveInfo *sact,
			     ActionCursorForSupport *c,
			     gList<const EFSupport> *list)
{ 
  (*list) += sact->UnderlyingSupport();

  ActionCursorForSupport place_holder(*c);

  do {
    if ( sact->NumActions(c->GetInfoset()) >= 2 ) {
      sact->RemoveAction(c->GetAction());
      AllSubsupportsRECURSIVE(s,sact,c,list);
      sact->AddAction(c->GetAction());
    }
  } while (c->GoToNext()) ;

  *c = place_holder;
}

gList<const EFSupport> AllSubsupports(const EFSupport &S)
{
  gList<const EFSupport> answer;
  EFSupportWithActiveInfo SAct(S);
  ActionCursorForSupport cursor(S);

  AllSubsupportsRECURSIVE(&S,&SAct,&cursor,&answer);

  return answer;
}


// Subsupports of a given support are _path equivalent_ if they
// agree on every infoset that can be reached under either, hence both,
// of them.  The next routine outputs one support for each equivalence
// class.  It is not for use in solution routines, but is instead a 
// prototype of the eventual path enumerator, which will also perform
// dominance elimination.

void AllInequivalentSubsupportsRECURSIVE(const EFSupport *s,
					 EFSupportWithActiveInfo *sact,
					 ActionCursorForSupport *c,
					 gList<const EFSupport> *list)
{ 
  (*list) += sact->UnderlyingSupport();

  //DEBUG
  gout << "The action under consideration is " 
       << c->GetAction()->GetName() << ".\n";

  ActionCursorForSupport place_holder(*c);

  do {
    if ( sact->NumActions(c->GetInfoset()) >= 2 ) {
      gList<Infoset *> deactivated_infosets;
      sact->RemoveActionReturningDeletedInfosets(c->GetAction(),
						 &deactivated_infosets);
      bool recurse = true;

      //DEBUG
      if (deactivated_infosets.Length() > 0)
	gout << "Currently the support is " << *sact << ".\n";

      for (int i = 1; i <= deactivated_infosets.Length(); i++) {


	if (c->GetInfoset()->GetNumber() >= 
	                              deactivated_infosets[i]->GetNumber()) {
	  if (!sact->AllActionsInSupportAtInfosetAreActive(*s,
		  			        deactivated_infosets[i])) {
	    recurse = false;
	  }
	}
      }

      if (recurse) AllInequivalentSubsupportsRECURSIVE(s,sact,c,list);
      sact->AddAction(c->GetAction());
    }
  } while (c->GoToNext()) ;

  *c = place_holder;
}

gList<const EFSupport> AllInequivalentSubsupports(const EFSupport &S)
{
  gList<const EFSupport> answer;
  EFSupportWithActiveInfo SAct(S);
  ActionCursorForSupport cursor(S);

  AllInequivalentSubsupportsRECURSIVE(&S,&SAct,&cursor,&answer);

  return answer;
}

//----------------------------------------------------
//                ActionCursorForSupport
// ---------------------------------------------------

ActionCursorForSupport::ActionCursorForSupport(const EFSupport &S)
  : support(&S), pl(1), iset(1), act(1)
{}

ActionCursorForSupport::ActionCursorForSupport(
                  const ActionCursorForSupport &ac)
  : support(ac.support), pl(ac.pl), iset(ac.iset), act(ac.act)
{}

ActionCursorForSupport::~ActionCursorForSupport()
{}

ActionCursorForSupport& 
ActionCursorForSupport::operator=(const ActionCursorForSupport &rhs)
{
  if (this != &rhs) {
    support = rhs.support;
    pl = rhs.pl;
    iset = rhs.iset;
    act = rhs.act;
  }
  return *this;
}

bool 
ActionCursorForSupport::operator==(const ActionCursorForSupport &rhs) const
{
  if (support != rhs.support ||
      pl      != rhs.pl      ||
      iset    != rhs.iset    ||
      act != rhs.act)
    return false;
  return true;
}

bool 
ActionCursorForSupport::operator!=(const ActionCursorForSupport &rhs) const
{
 return (!(*this==rhs));
}

bool
ActionCursorForSupport::GoToNext()
{
  if      (act != support->NumActions(pl,iset))
    { act++; return true; }
  else if (iset != support->Game().GetPlayer(pl)->NumInfosets())
    { iset++; act = 1; return true; }
  else if (pl != support->Game().NumPlayers())
    { pl++; iset = 1; act = 1; return true; }
  else return false;
}

Action *ActionCursorForSupport::GetAction() const
{
  return support->Actions(pl,iset)[act];
}


Infoset *ActionCursorForSupport::GetInfoset() const
{
  return support->Game().GetInfosetByIndex(pl,iset);
}

bool 
ActionCursorForSupport::IsLast() const
{
  if (pl == support->Game().NumPlayers())
    if (iset == support->Game().GetPlayer(pl)->NumInfosets())
      if (act == support->NumActions(pl,iset))
	return true;
  return false;
}

//////////////////////Testing////////////////////

void AndyTest(const EFSupport &S)
{
  //DEBUG
  // gout << "We got to go.\n";

  gList<const EFSupport> list = AllInequivalentSubsupports(S);
  for (int i = 1; i <= list.Length(); i++)
    gout << list[i] << "\n";
} 
