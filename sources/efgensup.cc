//
// FILE: efgensup.cc --Enumerate (Undominated) Subsupports of a Support
//
// $Id: efgensup.cc
//

#include "efgensup.h"

// We build a series of functions of increasing complexity.  The
// final one, which is our goal, is the undominated support function.
// We begin by simply enumerating all subsupports.

void AllSubsupportsRECURSIVE(const EFSupport *s,
			     EFSupportWithActiveInfo *sact,
			     ActionCursorForSupport *c,
			     gList<const EFSupport> *list)
{ 
  (*list) += sact->UnderlyingSupport();

  ActionCursorForSupport c_copy(*c);

  do {
    if ( sact->ActionIsActive((Action *)c_copy.GetAction()) ) {
      sact->RemoveAction(c_copy.GetAction());
      AllSubsupportsRECURSIVE(s,sact,&c_copy,list);
      sact->AddAction(c_copy.GetAction());
    }
  } while (c_copy.GoToNext()) ;
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
  if (sact->HasActiveActionsAtActiveInfosetsAndNoOthers())
    (*list) += sact->UnderlyingSupport();

  ActionCursorForSupport c_copy(*c);

  do {
    if ( sact->ActionIsActive((Action *)c_copy.GetAction()) ) {

      gList<Infoset *> deactivated_infosets;
      sact->RemoveActionReturningDeletedInfosets(c_copy.GetAction(),
						 &deactivated_infosets); 

      if (!c_copy.DeletionsViolateActiveCommitments(sact,
						    &deactivated_infosets))
	AllInequivalentSubsupportsRECURSIVE(s,sact,&c_copy,list);
      sact->AddAction(c_copy.GetAction());
    }
  } while (c_copy.GoToNext()) ;
}

gList<const EFSupport> AllInequivalentSubsupports(const EFSupport &S)
{
  gList<const EFSupport> answer;
  EFSupportWithActiveInfo SAct(S);
  ActionCursorForSupport cursor(S);

  AllInequivalentSubsupportsRECURSIVE(&S,&SAct,&cursor,&answer);

  return answer;
}

void AllUndominatedSubsupportsRECURSIVE(const EFSupport *s,
					 EFSupportWithActiveInfo *sact,
					 ActionCursorForSupport *c,
					const bool strong,
					const bool conditional,
					 gList<const EFSupport> *list,
					 const gStatus &status)
{ 
  bool abort = false;
  bool no_deletions = true;
  bool check_domination = false;
  if (sact->HasActiveActionsAtActiveInfosets()) 
    check_domination = true;
  gList<Action *> deletion_list;
  ActionCursorForSupport scanner(*s);

  // First we collect all the actions that can be deleted.
  do {
    Action *this_action = (Action *)scanner.GetAction();
    bool delete_this_action = false;

    if ( sact->ActionIsActive(this_action) ) 
      if ( !sact->InfosetIsActive(this_action->BelongsTo()) ) 
	delete_this_action = true;  
      else 
	if (check_domination) 
	  if (sact->IsDominated(this_action,strong,conditional) ) 
	    delete_this_action = true;
	
    if (delete_this_action) {
      no_deletions = false;
      if (c->IsSubsequentTo(this_action)) 
	abort = true;
      else 
	deletion_list += this_action;
    }
  } while (!abort && scanner.GoToNext());

  // Now we delete them, recurse, then restore
  if (!abort && !no_deletions) {
    gList<Action *> actual_deletions;
    for (int i = 1; !abort && i <= deletion_list.Length(); i++) {
      actual_deletions += deletion_list[i];
      gList<Infoset *> deactivated_infosets;
      
      sact->RemoveActionReturningDeletedInfosets(deletion_list[i],
						   &deactivated_infosets); 
	
      if (c->DeletionsViolateActiveCommitments(sact,&deactivated_infosets))
	abort = true;
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
      sact->AddAction(actual_deletions[i]);
  }

  // If there are no deletions, we ask if it is consistent, then recurse.
  if (!abort && no_deletions) {
    if (sact->HasActiveActionsAtActiveInfosetsAndNoOthers())
      (*list) += sact->UnderlyingSupport();
    
    ActionCursorForSupport c_copy(*c);
    
    do {
      if ( sact->ActionIsActive((Action *)c_copy.GetAction()) ) {
	
	gList<Infoset *> deactivated_infosets;
	sact->RemoveActionReturningDeletedInfosets(c_copy.GetAction(),
						   &deactivated_infosets); 
	
	if (!c_copy.DeletionsViolateActiveCommitments(sact,
						      &deactivated_infosets))
	  AllUndominatedSubsupportsRECURSIVE(s,
					     sact,
					     &c_copy,
					     strong,
					     conditional,
					     list,
					     status);
	sact->AddAction(c_copy.GetAction());
	
      }
    } while (c_copy.GoToNext()) ;
  }
}
  
gList<const EFSupport> AllUndominatedSubsupports(const EFSupport &S,
						 const bool strong,
						 const bool conditional,
						 const gStatus &status)
{
  gList<const EFSupport> answer;
  EFSupportWithActiveInfo sact(S);
  ActionCursorForSupport cursor(S);

  AllUndominatedSubsupportsRECURSIVE(&S,
				     &sact,
				     &cursor,
				     strong,
				     conditional,
				     &answer,
				     status);

  return answer;
}

void PossibleNashSubsupportsRECURSIVE(const EFSupport *s,
					    EFSupportWithActiveInfo *sact,
				            ActionCursorForSupport *c,
				            gList<const EFSupport> *list,
				      const gStatus &status)
{ 
  bool abort = false;
  bool no_deletions = true;
  bool check_domination = false;
  if (sact->HasActiveActionsAtActiveInfosets()) 
    check_domination = true;
  gList<Action *> deletion_list;
  ActionCursorForSupport scanner(*s);

  do {
    Action *this_action = (Action *)scanner.GetAction();
    bool delete_this_action = false;

    if ( sact->ActionIsActive(this_action) ) 
      if ( !sact->InfosetIsActive(this_action->BelongsTo()) )
	delete_this_action = true;  
      else
	if (check_domination) 
	  if (sact->IsDominated(this_action,true,true) ||
	      sact->IsDominated(this_action,true,false) ) 
	    delete_this_action = true;
    if (delete_this_action) {
      no_deletions = false;
      if (c->IsSubsequentTo(this_action)) 
	abort = true;
      else 
	deletion_list += this_action;
    }
  } while (!abort && scanner.GoToNext());
  
  if (!abort) {
    gList<Action *> actual_deletions;
    for (int i = 1; !abort && i <= deletion_list.Length(); i++) {
      actual_deletions += deletion_list[i];
      gList<Infoset *> deactivated_infosets;
      sact->RemoveActionReturningDeletedInfosets(deletion_list[i],
						   &deactivated_infosets); 
      if (c->DeletionsViolateActiveCommitments(sact,&deactivated_infosets))
	abort = true;
    }

    if (!abort && deletion_list.Length() > 0)
      PossibleNashSubsupportsRECURSIVE(s,sact,c,list,status);
    
    for (int i = 1; i <= actual_deletions.Length(); i++)
      sact->AddAction(actual_deletions[i]);
  }

  if (!abort && no_deletions) {

    if (sact->HasActiveActionsAtActiveInfosetsAndNoOthers())
      (*list) += sact->UnderlyingSupport();
    
    ActionCursorForSupport c_copy(*c);
    do {
      if ( sact->ActionIsActive((Action *)c_copy.GetAction()) ) {
	gList<Infoset *> deactivated_infosets;
	sact->RemoveActionReturningDeletedInfosets(c_copy.GetAction(),
						   &deactivated_infosets); 
	if (!c_copy.DeletionsViolateActiveCommitments(sact,
						      &deactivated_infosets))
	  PossibleNashSubsupportsRECURSIVE(s,sact,&c_copy,list,status);
	sact->AddAction(c_copy.GetAction());
      }
    } while (c_copy.GoToNext()) ;
  }
}
  
gList<const EFSupport> PossibleNashSubsupports(const EFSupport &S,
					       const gStatus &status)
{
  gList<const EFSupport> answer;
  EFSupportWithActiveInfo sact(S);
  ActionCursorForSupport cursor(S);

  PossibleNashSubsupportsRECURSIVE(&S,&sact,&cursor,&answer,status);

  // At this point answer has all consistent subsupports without
  // any strong dominations.  We now edit the list, removing all
  // subsupports that exhibit weak dominations, and we also eliminate
  // subsupports exhibiting domination by currently inactive actions.

  for (int i = answer.Length(); i >= 1; i--) {
    EFSupportWithActiveInfo current(answer[i]);
    ActionCursorForSupport crsr(S);
    bool remove = false;
    do {
      const Action *act = crsr.GetAction();
      if (current.ActionIsActive((Action *)act)) 
	for (int j = 1; j <= act->BelongsTo()->NumActions(); j++) {
	  Action *other_act = act->BelongsTo()->GetAction(j);
	  if (other_act != act)
	    if (current.ActionIsActive(other_act)) {
	      if (current.Dominates(other_act,act,false,true) ||
		  current.Dominates(other_act,act,false,false)) 
		remove = true;
	    }
	    else { 
	      current.AddAction(other_act);
	      if (current.HasActiveActionsAtActiveInfosetsAndNoOthers())
		if (current.Dominates(other_act,act,false,true) ||
		    current.Dominates(other_act,act,false,false)) {
		  remove = true;
		}
	      current.RemoveAction(other_act);
	    }
      }
    } while (crsr.GoToNext() && !remove);
    if (remove)
      answer.Remove(i);
  }
    
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

const Action *ActionCursorForSupport::GetAction() const
{
  return support->Actions(pl,iset)[act];
}

int ActionCursorForSupport::ActionIndex() const
{
  return act;
}


const Infoset *ActionCursorForSupport::GetInfoset() const
{
  return support->Game().GetInfosetByIndex(pl,iset);
}

int ActionCursorForSupport::InfosetIndex() const
{
  return iset;
}

const EFPlayer *ActionCursorForSupport::GetPlayer() const
{
  return support->Game().GetPlayer(pl);
}

int ActionCursorForSupport::PlayerIndex() const
{
  return pl;
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

bool 
ActionCursorForSupport::IsSubsequentTo(const Action *a) const
{
  /*
  //DEBUG
  gout << "In IsSubsequentTo with pl = " << pl 
       << ", and a's player = " << a ->BelongsTo()->GetPlayer()->GetNumber()
       << ".\n";
  */

  if (pl > a->BelongsTo()->GetPlayer()->GetNumber())
    return true; 
  else if (pl < a->BelongsTo()->GetPlayer()->GetNumber())
    return false;
  else
    if (iset > a->BelongsTo()->GetNumber())
      return true; 
    else if (iset < a->BelongsTo()->GetNumber())
      return false;
    else 
      if (act > a->GetNumber())
	return true;
      else
	return false;
}


bool ActionCursorForSupport::
DeletionsViolateActiveCommitments(const EFSupportWithActiveInfo *S,
				   const gList<Infoset *> *infosetlist)
{
  for (int i = 1; i <= infosetlist->Length(); i++) {
    const Infoset *infoset = (*infosetlist)[i];
    if (infoset->GetPlayer()->GetNumber() < PlayerIndex() ||
	( infoset->GetPlayer()->GetNumber() == PlayerIndex() &&
	  infoset->GetNumber() < InfosetIndex()) )
      if (S->NumActions(infoset) > 0)
	return true;
    if (infoset->GetPlayer()->GetNumber() == PlayerIndex() &&
	infoset->GetNumber() == InfosetIndex() )
      for (int act = 1; act < ActionIndex(); act++)
	if ( S->ActionIsActive(infoset->GetPlayer()->GetNumber(),
			       infoset->GetNumber(),
			       act) )
	  return true;
  }
  return false;
}

//////////////////////Testing////////////////////

void AndyTest(const EFSupport &S, gStatus &status)
{
  //  bool strong = true;
  //  bool conditional = true;
  gList<const EFSupport> list = PossibleNashSubsupports(S,
							  //strong, 
							  //conditional, 
							  status);
  for (int i = 1; i <= list.Length(); i++)
    gout << list[i] << "\n";
} 
