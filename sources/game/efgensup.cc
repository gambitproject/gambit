//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Enumerate undominated subsupports
//
// This file is part of Gambit
// Copyright (c) 2002, The Gambit Project
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
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
  status.Get();

  bool abort = false;
  bool add_support = true;

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
	  if ( sact->IsDominated(this_action,true,true) ||
	       sact->IsDominated(this_action,true,false) ) {
	    add_support = false;
	    if ( c->InfosetGuaranteedActiveByPriorCommitments(sact,
						   this_action->BelongsTo()) )
	      delete_this_action = true;
	  }
    if (delete_this_action) {
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
        
    for (int i = 1; i <= actual_deletions.Length(); i++) {
      sact->AddAction(actual_deletions[i]);
    }
  }

  if (!abort && deletion_list.Length() == 0) {

    if (add_support && sact->HasActiveActionsAtActiveInfosetsAndNoOthers())
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

gList<const EFSupport> SortSupportsBySize(gList<const EFSupport> &list) 
{
  gArray<int> sizes(list.Length());
  for (int i = 1; i <= list.Length(); i++)
    sizes[i] = list[i].NumDegreesOfFreedom();

  gArray<int> listproxy(list.Length());
  for (int i = 1; i <= list.Length(); i++)
    listproxy[i] = i;

  int maxsize(0);
  for (int i = 1; i <= list.Length(); i++)
    if (sizes[i] > maxsize)
      maxsize = sizes[i];

  int cursor(1);

  for (int j = 0; j < maxsize; j++) {
    int scanner(list.Length());
    while (cursor < scanner)
      if (sizes[scanner] != j)
	scanner--;
      else {
	while (sizes[cursor] == j)
	  cursor++;
	if (cursor < scanner) {
	  int tempindex = listproxy[cursor];
	  listproxy[cursor] = listproxy[scanner];
	  listproxy[scanner] = tempindex;
	  int tempsize = sizes[cursor];
	  sizes[cursor] = sizes[scanner];
	  sizes[scanner] = tempsize;
	  cursor++;
	}
      }
  }

  gList<const EFSupport> answer;
  for (int i = 1; i <= list.Length(); i++)
    answer += list[listproxy[i]];

  return answer;
}
  
gList<const EFSupport> PossibleNashSubsupports(const EFSupport &S,
					       gStatus &status)
{
  gList<const EFSupport> answer;
  EFSupportWithActiveInfo sact(S);
  ActionCursorForSupport cursor(S);

  status.SetProgress(0);
  PossibleNashSubsupportsRECURSIVE(&S,&sact,&cursor,&answer,status);
  status.SetProgress(.5);

  // At this point answer has all consistent subsupports without
  // any strong dominations.  We now edit the list, removing all
  // subsupports that exhibit weak dominations, and we also eliminate
  // subsupports exhibiting domination by currently inactive actions.

  for (int i = answer.Length(); i >= 1; i--) {
    status.SetProgress((2.0-((double)i/(double)answer.Length()))/2.0);
    status.Get();
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

  return SortSupportsBySize(answer);
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
  if (act != support->NumActions(pl,iset))
    { act++; return true; }
  
  int temppl(pl);
  int tempiset(iset);
  tempiset ++; 

  while (temppl <= support->GetGame().NumPlayers()) {
    while (tempiset <= support->GetGame().Players()[temppl]->NumInfosets()) {
      if (support->NumActions(temppl,tempiset) > 0) {
	pl = temppl;
	iset = tempiset;
	act = 1;
	return true;
      }
      else
	tempiset++;
    }
    tempiset = 1;
    temppl++;
  }
  return false;
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
  return support->GetGame().Players()[pl]->Infosets()[iset];
}

int ActionCursorForSupport::InfosetIndex() const
{
  return iset;
}

const EFPlayer *ActionCursorForSupport::GetPlayer() const
{
  return support->GetGame().Players()[pl];
}

int ActionCursorForSupport::PlayerIndex() const
{
  return pl;
}

bool 
ActionCursorForSupport::IsLast() const
{
  if (pl == support->GetGame().NumPlayers())
    if (iset == support->GetGame().Players()[pl]->NumInfosets())
      if (act == support->NumActions(pl,iset))
	return true;
  return false;
}

bool 
ActionCursorForSupport::IsSubsequentTo(const Action *a) const
{
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


bool ActionCursorForSupport::
InfosetGuaranteedActiveByPriorCommitments(const EFSupportWithActiveInfo *S,
					  const Infoset *infoset)
{
  gList<const Node *> members = infoset->ListOfMembers();
  for (int i = 1; i <= members.Length(); i++) {
    const Node* current = members[i];
    if ( current == S->GetGame().RootNode() )
      return true;
    else
      while ( S->ActionIsActive((Action *)current->GetAction()) &&
	      IsSubsequentTo(current->GetAction()) ) {
	current = current->GetParent();
	if ( current == S->GetGame().RootNode() )
	  return true;
      }
  }
  return false;
}

