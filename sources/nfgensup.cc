//
// FILE: nfgensup.cc --Enumerate (Undominated) Subsupports of a Support
//
// $Id: nfgensup.cc
//

#include "nfgensup.h"
#include "nfdom.h"

// Instantiations
//template class gList<const NFSupport>;
//template class gList<NFSupport const>;

// We build a series of functions of increasing complexity.  The
// final one, which is our goal, is the undominated support function.
// We begin by simply enumerating all subsupports.

void AllSubsupportsRECURSIVE(const NFSupport *s,
			           NFSupport *sact,
			           StrategyCursorForSupport *c,
			           gList<const NFSupport> *list)
{ 
  (*list) += *sact;

  StrategyCursorForSupport c_copy(*c);

  do {
    Strategy *str_ptr = (Strategy *)c_copy.GetStrategy();
    if ( sact->StrategyIsActive(str_ptr) ) {
      sact->RemoveStrategy(str_ptr);
      AllSubsupportsRECURSIVE(s,sact,&c_copy,list);
      sact->AddStrategy(str_ptr);
    }
  } while (c_copy.GoToNext()) ;
}

gList<const NFSupport> AllSubsupports(const NFSupport &S)
{
  gList<const NFSupport> answer;
  NFSupport SAct(S);
  StrategyCursorForSupport cursor(S);

  AllSubsupportsRECURSIVE(&S,&SAct,&cursor,&answer);

  return answer;
}

// In the next two routines we only output subsupports that
// have at least one active strategy for each agent.

void AllValidSubsupportsRECURSIVE(const NFSupport *s,
                                         NFSupport *sact,
					 StrategyCursorForSupport *c,
					 gList<const NFSupport> *list)
{ 
  (*list) += *sact;

  StrategyCursorForSupport c_copy(*c);

  do {
    if ( sact->NumStrats(c_copy.PlayerIndex()) > 1 ) {
      Strategy *str_ptr = (Strategy *)c_copy.GetStrategy();
      sact->RemoveStrategy(str_ptr); 
      AllValidSubsupportsRECURSIVE(s,sact,&c_copy,list);
      sact->AddStrategy(str_ptr);
    }
  } while (c_copy.GoToNext()) ;
}

gList<const NFSupport> AllValidSubsupports(const NFSupport &S)
{
  gList<const NFSupport> answer;
  NFSupport SAct(S);
  StrategyCursorForSupport cursor(S);

  AllValidSubsupportsRECURSIVE(&S,&SAct,&cursor,&answer);

  return answer;
}

void AllUndominatedSubsupportsRECURSIVE(const NFSupport *s,
					      NFSupport *sact,
					      StrategyCursorForSupport *c,
					const bool strong,
					const bool conditional,
					      gList<const NFSupport> *list,
					const gStatus &status)
{ 
  bool abort = false;
  bool no_deletions = true;


  gList<Strategy *> deletion_list;
  StrategyCursorForSupport scanner(*s);

  // First we collect all the strategies that can be deleted.
  do {
    Strategy *this_strategy = (Strategy *)scanner.GetStrategy();
    bool delete_this_strategy = false;
    if ( sact->StrategyIsActive(this_strategy) ) 
      if (sact->IsDominated(this_strategy,strong) ) 
	delete_this_strategy = true;
	
    if (delete_this_strategy) {
      no_deletions = false;
      if (c->IsSubsequentTo(this_strategy)) 
	abort = true;
      else 
	deletion_list += this_strategy;
    }
  } while (!abort && scanner.GoToNext());

  // Now we delete them, recurse, then restore
  if (!abort && !no_deletions) {
    gList<Strategy *> actual_deletions;
    for (int i = 1; !abort && i <= deletion_list.Length(); i++) {
      actual_deletions += deletion_list[i];

      sact->RemoveStrategy(deletion_list[i]); 
    }

    if (!abort)
      AllUndominatedSubsupportsRECURSIVE(s,
					 sact,
					 c,
					 strong,
					 conditional,
					 list,
					 status);
    
    for (int i = 1; i <= actual_deletions.Length(); i++)
      sact->AddStrategy(actual_deletions[i]);
  }

  // If there are no deletions, we ask if it is consistent, then recurse.
  if (!abort && no_deletions) {
    (*list) += *sact;
    
    StrategyCursorForSupport c_copy(*c);
    
    do {
      if ( sact->StrategyIsActive((Strategy *)c_copy.GetStrategy()) &&
	   sact->NumStrats(c_copy.PlayerIndex()) > 1 ) {

	Strategy *str_ptr = (Strategy *)c_copy.GetStrategy();
	sact->RemoveStrategy(str_ptr); 
	AllUndominatedSubsupportsRECURSIVE(s,
					   sact,
					   &c_copy,
					   strong,
					   conditional,
					   list,
					   status);
	sact->AddStrategy(str_ptr);
	
      }
    } while (c_copy.GoToNext()) ;
  }
}
  
gList<const NFSupport> AllUndominatedSubsupports(const NFSupport &S,
						 const bool strong,
						 const bool conditional,
						 const gStatus &status)
{
  gList<const NFSupport> answer;
  NFSupport sact(S);
  StrategyCursorForSupport cursor(S);

  AllUndominatedSubsupportsRECURSIVE(&S,
				     &sact,
				     &cursor,
				     strong,
				     conditional,
				     &answer,
				     status);

  return answer;
}


void PossibleNashSubsupportsRECURSIVE(const NFSupport *s,
					    NFSupport *sact,
				            StrategyCursorForSupport *c,
				            gList<const NFSupport> *list,
				      const gStatus &status)
{ 
  bool abort = false;
  bool no_deletions = true;

  gList<Strategy *> deletion_list;
  StrategyCursorForSupport scanner(*s);

  do {
    Strategy *this_strategy = (Strategy *)scanner.GetStrategy();
    bool delete_this_strategy = false;
    if ( sact->StrategyIsActive(this_strategy) ) 
      if (sact->IsDominated(this_strategy,true) ) {
	delete_this_strategy = true;
      }
    if (delete_this_strategy) {
      no_deletions = false;
      if (c->IsSubsequentTo(this_strategy)) 
	abort = true;
      else 
	deletion_list += this_strategy;
    }
  } while (!abort && scanner.GoToNext());
  
  if (!abort) {
    gList<Strategy *> actual_deletions;
    for (int i = 1; !abort && i <= deletion_list.Length(); i++) {
      actual_deletions += deletion_list[i];
      sact->RemoveStrategy(deletion_list[i]); 
    } 

    if (!abort && deletion_list.Length() > 0)
      PossibleNashSubsupportsRECURSIVE(s,sact,c,list,status);
    
    for (int i = 1; i <= actual_deletions.Length(); i++)
      sact->AddStrategy(actual_deletions[i]);
  }
  if (!abort && no_deletions) {
    (*list) += *sact;
    
    StrategyCursorForSupport c_copy(*c);
    do {
      Strategy *str_ptr = (Strategy *)c_copy.GetStrategy();
      if ( sact->StrategyIsActive(str_ptr) &&
	   sact->NumStrats(str_ptr->Player()) > 1 ) {
	sact->RemoveStrategy(str_ptr); 
	PossibleNashSubsupportsRECURSIVE(s,sact,&c_copy,list,status);
	sact->AddStrategy(str_ptr);
      }
    } while (c_copy.GoToNext()) ;
  }
}
  
gList<const NFSupport> PossibleNashSubsupports(const NFSupport &S,
					       const gStatus &status)
{
  gList<const NFSupport> answer;
  NFSupport sact(S);
  StrategyCursorForSupport cursor(S);

  PossibleNashSubsupportsRECURSIVE(&S,&sact,&cursor,&answer,status);

  // At this point answer has all consistent subsupports without
  // any strong dominations.  We now edit the list, removing all
  // subsupports that exhibit weak dominations, and we also eliminate
  // subsupports exhibiting domination by currently inactive strategies.

  for (int i = answer.Length(); i >= 1; i--) {
    NFSupport current(answer[i]);
    StrategyCursorForSupport crsr(S);
    bool remove = false;
    do {
      Strategy *strat = (Strategy *)crsr.GetStrategy();
      if (current.StrategyIsActive(strat)) 
	for (int j = 1; j <= strat->Player()->NumStrats(); j++) {
	  Strategy *other_strat = strat->Player()->GetStrategy(j);
	  if (other_strat != strat)
	    if (current.StrategyIsActive(other_strat)) {
	      if (current.Dominates(other_strat,strat,false)) 
		remove = true;
	    }
	    else { 
	      current.AddStrategy(other_strat);
	      if (current.Dominates(other_strat,strat,false)) {
		remove = true;
	      }
	      current.RemoveStrategy(other_strat);
	    }
      }
    } while (crsr.GoToNext() && !remove);
    if (remove)
      answer.Remove(i);
  }
    
  return answer;
}


//----------------------------------------------------
//                StrategyCursorForSupport
// ---------------------------------------------------

StrategyCursorForSupport::StrategyCursorForSupport(const NFSupport &S)
  : support(&S), pl(1), strat(1)
{}

StrategyCursorForSupport::StrategyCursorForSupport(
                  const StrategyCursorForSupport &sc)
  : support(sc.support), pl(sc.pl), strat(sc.strat)
{}

StrategyCursorForSupport::~StrategyCursorForSupport()
{}

StrategyCursorForSupport& 
StrategyCursorForSupport::operator=(const StrategyCursorForSupport &rhs)
{
  if (this != &rhs) {
    support = rhs.support;
    pl = rhs.pl;
    strat = rhs.strat;
  }
  return *this;
}

bool 
StrategyCursorForSupport::operator==(const StrategyCursorForSupport &rhs) const
{
  if (support != rhs.support ||
      pl      != rhs.pl      ||
      strat   != rhs.strat)
    return false;
  return true;
}

bool 
StrategyCursorForSupport::operator!=(const StrategyCursorForSupport &rhs) const
{
 return (!(*this==rhs));
}

bool
StrategyCursorForSupport::GoToNext()
{
  if (strat != support->NumStrats(pl))
    { strat++; return true; }
  else if (pl    != support->Game().NumPlayers())
    { pl++; strat = 1; return true; }
  else return false;
}

const Strategy *StrategyCursorForSupport::GetStrategy() const
{
  return support->Strategies(pl)[strat];
}

int StrategyCursorForSupport::StrategyIndex() const
{
  return strat;
}

const NFPlayer *StrategyCursorForSupport::GetPlayer() const
{
  return support->Game().GetPlayer(pl);
}

int StrategyCursorForSupport::PlayerIndex() const
{
  return pl;
}

bool 
StrategyCursorForSupport::IsLast() const
{
  if (pl == support->Game().NumPlayers())
    if (strat == support->NumStrats(pl))
      return true;
  return false;
}

bool 
StrategyCursorForSupport::IsSubsequentTo(const Strategy *s) const
{
  if (pl > s->Player()->GetNumber())
    return true; 
  else if (pl < s->Player()->GetNumber())
    return false;
  else
    if (strat > s->Number())
      return true;
    else
      return false;
}
