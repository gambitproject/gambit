//
// FILE: efdom.cc -- Compute dominated strategies on extensive form
//
// $Id$
//

#include "efg.h"
#include "efgciter.h"
#include "rational.h"
#include "gstatus.h"
#include "gsm.h"

// The following computes whether action a dominates action b.
// If `conditional' is false, then the computation is with respect
// to whether a dominates b with respect to all possibilities for 
// the game allowed by the support.  The argument `strong' describes
// whether domination is supposed to be strong or weak.  In particular,
// if strong is true and conditional is false, true cannot be
// returned unless all plays (given the support) go through the
// infoset where a and b might be chosen.

bool Dominates(const EFSupport &S, 
	       const int pl, 
	       const int iset, 
	       const int a, const int b, 
	       const bool strong,
	       const bool conditional,
	       const gStatus &status)
{
  const Infoset *infoset = S.Game().GetInfosetByIndex(pl,iset);

  if (!conditional) {
    if (!S.MayReach(infoset))
      return false;
    if (S.AlwaysReaches(infoset) || !strong)
      return Dominates(S,pl,iset,a,b,strong,true,status);
    else
      return false;
  }

  const EFSupportWithActiveInfo SAct(S);
  Action *aAct = S.Actions(pl,iset)[a];
  Action *bAct = S.Actions(pl,iset)[b];

  gList<const Node *> nodelist = SAct.ReachableNodesInInfoset(infoset);  
  if (nodelist.Length() == 0)
    nodelist = infoset->ListOfMembers();  // This may not be a good idea;
                                          // I suggest checking for this 
                                          // prior to entry

  bool equal = true;
  for (int n = 1; n <= nodelist.Length(); n++) {

    gList<const Infoset *> L;
    L += S.ReachableInfosets(nodelist[n],aAct);
    L += S.ReachableInfosets(nodelist[n],bAct);
    L.RemoveRedundancies();

    EfgConditionalContIter A(S,L), B(S,L);
    A.Set(pl, iset, a);
    B.Set(pl, iset, b);
    
    do  {
      status.Get();
      gRational ap = A.Payoff(nodelist[n],pl);  
      gRational bp = B.Payoff(nodelist[n],pl);
      if (strong)
	{ if (ap <= bp)  return false; }
      else
	if (ap < bp)   return false;
	else if (ap > bp)  equal = false;
    } while (A.NextContingency() && B.NextContingency());
  }
    
  if (strong) return true;
  else return (!equal);
}

// Another window to the computation above.

bool Dominates(const EFSupport &S, 
	       const Action *a, const Action *b,
	       const bool strong,
	       const bool conditional,
	       const gStatus &status)
{
  const Infoset *infoset = a->BelongsTo();
  /*
  if (infoset != b->BelongsTo())
    throw gclRuntimeError("Dominates(..) needs actions in same infoset.\n");
  */
  const EFPlayer *player = infoset->GetPlayer();

  return Dominates(S,player->GetNumber(),infoset->GetNumber(),
		   a->GetNumber(),b->GetNumber(),
		   strong, conditional, status);
}

bool SomeListElementDominates(const EFSupport &S, 
			      const gList<Action *> &l,
			      const Action *a, 
			      const bool strong,
			      const bool conditional,
			      const gStatus &status)
{
  for (int i = 1; i <= l.Length(); i++)
    if (l[i] != a)
      if (Dominates(S,l[i],a,strong,conditional,status))
	return true;
  return false;
}

bool InfosetHasDominatedElement(const EFSupport &S, 
				const Infoset *i,
				const bool strong,
				const bool conditional,
				const gStatus &status)
{
  gList<Action *> actions = S.ListOfActions(i);
  for (int i = 1; i <= actions.Length(); i++)
    if (SomeListElementDominates(S,actions,actions[i],
				 strong,conditional,gstatus))
      return true;

  return false;
}

class ActionCursorForSupport {
protected:
  const EFSupport *support;
  int pl;
  int iset;
  int nodenum;

public:
  //Constructors and dtor
  ActionCursorForSupport(const EFSupport &S);
  ActionCursorForSupport(const ActionCursorForSupport &a);
  ~ActionCursorForSupport();

  // Operators
  ActionCursorForSupport &operator =(const ActionCursorForSupport &);
  bool                    operator==(const ActionCursorForSupport &) const;
  bool                    operator!=(const ActionCursorForSupport &) const;
};


//----------------------------------------------------
//                ActionCursorForSupport
// ---------------------------------------------------

ActionCursorForSupport::ActionCursorForSupport(const EFSupport &S)
  : support(&S), pl(0), iset(0), nodenum(0)
{}

ActionCursorForSupport::ActionCursorForSupport(
                  const ActionCursorForSupport &ac)
  : support(ac.support), pl(ac.pl), iset(ac.iset), nodenum(ac.nodenum)
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
    nodenum = rhs.nodenum;
  }
  return *this;
}

bool 
ActionCursorForSupport::operator==(const ActionCursorForSupport &rhs) const
{
  if (support != rhs.support ||
      pl      != rhs.pl      ||
      iset    != rhs.iset    ||
      nodenum != rhs.nodenum)
    return false;
  return true;
}

bool 
ActionCursorForSupport::operator!=(const ActionCursorForSupport &rhs) const
{
 return (!(*this==rhs));
}

// We now build a series of functions of increasing complexity.  The
// final one, which is our goal, is the undominated support function.
// We begin by simply enumerating all subsupports.

gList<const EFSupport> AllSubsupports(const EFSupport *S)
{
  gList<const EFSupport> answer;

  EFSupportWithActiveInfo SAct(*S);
  /*
  while (!SAct.IsFinalSubsupportOf(S)) {
    answer += (EFSupport)SAct;
    SAct.GoToNextSubsupportOf(S);
  }
  */
  answer += (EFSupport)SAct;

  return answer;
}


// Subsupports of a given support are _path equivalent_ if they
// agree on every infoset that can be reached under either, hence both,
// of them.  The next routine outputs one support for each equivalence
// class.  It is not for use in solution routines, but is instead a 
// prototype of the eventual path enumerator, which will also perform
// dominance elimination.

gList<const EFSupport> AllInequivalentSubsupports(const EFSupport *S)
{
  gList<const EFSupport> answer;

  // Under construction ...
  // Phases:
  // 1.  Enumerate everything
  // 2.  Add requirement that every infoset has an action
  EFSupportWithActiveInfo SAct(*S);
  answer += (EFSupport)SAct;

  return answer;
}


// The code below is old, and uses indexing mechanisms that I do not
// understand, which look error-prone. amm-8/98

bool ComputeDominated(const EFSupport &S, EFSupport &T,
		      const int pl, const int iset, const bool strong,
					gStatus &status)
{
  //DEBUG
  gout << "Got into ComputeDominated().\n";

  const gArray<Action *> &actions = S.Actions(pl, iset);

  gArray<int> set(actions.Length());
  int i;
  for (i = 1; i <= set.Length(); i++)
    set[i] = i;

  int min, dis;
  for (min = 0, dis = actions.Length() - 1; min <= dis; )  {
    status.Get();
    int pp;
    for (pp = 0;
	 pp < min && !Dominates(S, pl, iset, set[pp+1], set[dis+1], strong,false,status);
	 pp++);
    if (pp < min)
      dis--;
    else  {
      int foo = set[dis+1];
      set[dis+1] = set[min+1];
      set[min+1] = foo;

      for (int inc = min + 1; inc <= dis; )  {
	status.Get();
	if (Dominates(S, pl, iset, set[min+1], set[dis+1], strong, false,status))  {
		status << actions[set[dis+1]]->GetNumber() << " dominated by "
				<< actions[set[min+1]]->GetNumber() << '\n';
		dis--;
	}
	else if (Dominates(S, pl, iset, set[dis+1], set[min+1], strong,false,status))  {
		status << actions[set[min+1]]->GetNumber() << " dominated by "
			<< actions[set[dis+1]]->GetNumber() << '\n';
		foo = set[dis+1];
		set[dis+1] = set[min+1];
		set[min+1] = foo;
		dis--;
	}
	else  {
		foo = set[dis+1];
		set[dis+1] = set[inc+1];
		set[inc+1] = foo;
		inc++;
	}
			}
			min++;
		}
	}

	if (min + 1 <= actions.Length())   {

	  //DEBUG
	  gout << "Got in.\n";

	  status.Get();
	  for (i = min + 1; i <= actions.Length(); i++)
	    T.RemoveAction(actions[set[i]]);
	  //DEBUG
	  gout << "About to leave.\n";

	  return true;
	}
	else
	  return false;
}


EFSupport *ComputeDominated(EFSupport &S, 
			    bool strong,
			    const gArray<int> &players,
			    gOutput & /* tracefile */, 
			    gStatus &status)
{
  EFSupport *T = new EFSupport(S);
  bool any = false;
  unsigned long num_isets=0,cur_iset=0;
  // calc total # of isets
  int i,iset;
  for (i = 1; i <= players.Length(); i++)
    num_isets += S.Game().Players()[players[i]]->NumInfosets();

  for (i = 1; i <= players.Length(); i++)   {
    status.Get();
    int pl = players[i];
    status << "Dominated strategies for player " << pl << ":\n";
    for (iset = 1;
	 iset <= S.Game().Players()[pl]->NumInfosets();
	 iset++)   {
      status.Get();
      status << "Dominated strategies in infoset " << iset << ":\n";
      status.SetProgress((double)cur_iset/(double)num_isets);cur_iset++;
      if (ComputeDominated(S, *T, pl, iset, strong, status))
	any = true;
    }
  }

  if (!any)  {
    delete T;
    return 0;
  }

  return T;
}
