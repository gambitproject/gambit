//
// FILE: extform.cc -- Implementation of extensive form data type
//
// $Id$
//

#define EXTFORM_C
#include "extform.h"
#include "outcome.h"
#include "player.h"
#include "node.h"
#include "infoset.h"
#include "game.h"

//------------------------------------------------------------------------
//                EXTFORM: CONSTRUCTORS AND DESTRUCTOR
//------------------------------------------------------------------------

//************************************************************************
// ExtForm -- Construct the trivial extensive form
// Inputs:  None
// Outputs: '*this' is a properly initialized trivial extensive form
//************************************************************************

ExtForm::ExtForm(void)
{
  _title = new gString("Untitled");
  _outcomes = new gSet<Outcome>;
  _players = new gSet<Player>;
  _nodes = new gSet<Node>;
  _infosets = new gSet<Infoset>;

  _root = gHandle<Node>(new Node);
  *_nodes += _root;
  _root->SetName("ROOT");
  *_infosets += gHandle<Infoset>(new Infoset(_root, gHandle<Player>(0)));

  _iterator = 0;

  _trivial = 1;
  _modified = 0;
}

//************************************************************************
// ~ExtForm -- ExtForm destructor
// Inputs:  None
// Outputs: All substructures of ExtForm properly deallocated
//***********************************************************************

ExtForm::~ExtForm()
{
  delete _title;
  delete _outcomes;
  delete _players;
  delete _nodes;
  delete _infosets;

  if (_iterator)   _iterator->Invalidate();
}

//-----------------------------------------------------------------------
//                     EXTFORM: OPERATIONS ON NODES
//-----------------------------------------------------------------------

gHandle<Node> ExtForm::RootNode(void) const
{
  return _root;
}

//***********************************************************************
// AddNode -- Add children to a terminal node
// Inputs:  n -- a terminal node to which to add children
//          player -- player who has decision at node 'n'
//          child_count -- number of children to add
// Outputs: returns nonzero if successful
//***********************************************************************

int ExtForm::AddNode(gHandle<Node> &n, uint player, uint child_count)
{
  if (!_nodes->ElNumber(n))  return 0;
  if (!n->IsTerminal())  return 0;

  *_nodes += n->AddChildren(child_count);

      // generate infosets for the new children
  for (int i = 1; i <= child_count; i++)
    *_infosets += gHandle<Infoset>(new Infoset((*n)(i), gHandle<Player>(0)));

  AddPlayer(player);

  if (player)   // need to switch player...
    *_infosets += gHandle<Infoset>(new Infoset(n, (*_players)[player]));

  _trivial = 0;
  _modified = 1;

  return 1;
}


//**********************************************************************
// InsertNode -- Insert a node in the extensive form
// Inputs:  n -- Location at which to insert node
//          player -- Player who has decision at new node
//          child_count -- Number of children new node will have
// Outputs: Returns nonzero if successful
// Note:    Node 'n' becomes first child of the newly inserted node
//**********************************************************************

int ExtForm::InsertNode(gHandle<Node> &n, uint player, uint branch_count)
{
  if (!_nodes->ElNumber(n))  return 0;

  gHandle<Node> temp = new Node;

  MoveTree(n, temp);

  printf("tree moved\n");
/*
  AddPlayer(player);

  printf("player added\n");

  *_nodes += n->AddChildren(branch_count);

      // generate infosets for the new children
  for (int i = 1; i <= branch_count; i++)
    *_infosets += gHandle<Infoset>(new Infoset((*n)(i), gHandle<Player>(0)));

  if (player)
    *_infosets += gHandle<Infoset>(new Infoset(n, (*_players)[player]));
  else
    *_infosets += gHandle<Infoset>(new Infoset(n, gHandle<Player>(0)));

  printf("infoset created\n");

  MoveTree(temp, (*n)(1));
  */

  MoveTree(n, temp);
  printf("tree returned\n");

  _trivial = 0;
  _modified = 1;

  return 1;
}

//***********************************************************************
// LabelNode -- set label of a node
// Inputs:  n -- node to be labeled
//          label -- the new label
// Outputs: returns nonzero if successful
//***********************************************************************

int ExtForm::LabelNode(gHandle<Node> &n, gString &label)
{
  if (!_nodes->ElNumber(n))  return 0;
  n->SetName(label);

  _trivial = 0;
  _modified = 1;

  return 1;
}

//**********************************************************************
// SetOutcome -- set the outcome associated with a node
// Inputs:  n -- the node to which to associate the outcome
//          outcome_number -- the number of the outcome
// Outputs: returns nonzero if successful
//*********************************************************************

int ExtForm::SetOutcome(gHandle<Node> &n, uint outcome_number)
{
  if (!_nodes->ElNumber(n))  return 0;

  AddOutcome(outcome_number);
  n->SetOutcome((*_outcomes)[outcome_number]);

  _trivial = 0;
  _modified = 1;

  return 1;
}

int ExtForm::GetOutcome(gHandle<Node> &n)
{
  if (!_nodes->ElNumber(n))   return 0;
  return _outcomes->ElNumber(n->GetOutcome());
}

//*********************************************************************
// DeleteNode -- delete a node from the extensive form
// Inputs:  n -- the node to be removed
//          keep -- the branch of the node 'n' to be kept
// Outputs: returns nonzero if successful
// Note:    does not deal with possibility of 'n' being terminal
//*********************************************************************

int ExtForm::DeleteNode(gHandle<Node> &n, gHandle<Node> &keep)
{
  if (!_nodes->ElNumber(n) || !_nodes->ElNumber(keep))  return 0;
  if (!keep->IsChildOf(n))  return 0;

  gHandle<Node> temp = gHandle<Node>(new Node);
  MoveTree(temp, keep);

  if (_root == n)    _root = keep;
 
  DeleteTree(n);

  MoveTree(n, keep);
  
  _trivial = 0;
  _modified = 1;

  return 1;
}

int ExtForm::InfosetMember(gHandle<Node> &new_member, gHandle<Node> &to_iset)
{
  return 1;
}

int ExtForm::BreakInfoset(gHandle<Node> &breakpoint)
{
  return 1;
}

int ExtForm::JoinInfoset(gHandle<Node> &new_members, gHandle<Node> &to_iset)
{
  return 1;
}

int ExtForm::LabelBranch(gHandle<Node> &n, gString &label)
{
  return 1;
}

int ExtForm::InsertBranch(gHandle<Node> &n, uint branch_number)
{
  return 1;
}

int ExtForm::DeleteBranch(gHandle<Node> &n, gHandle<Node> &remove)
{
  return 1;
}

int ExtForm::DeleteTree(gHandle<Node> &n)
{
  return 1;
}

int ExtForm::CopyTree(gHandle<Node> &n, gHandle<Node> &to)
{
  return 1;
}

int ExtForm::LabelTree(const gString &label)
{
  return 1;
}

gString ExtForm::TreeLabel(void) const
{
  return "";
}

//------------------------------------------------------------------------
//                   EXTFORM: PRIVATE MEMBER FUNCTIONS
//------------------------------------------------------------------------

void ExtForm::MoveTree(gHandle<Node> &src, gHandle<Node> &dest)
{
  if (!dest->IsTerminal())  return;

  dest->AdoptChildren(src);
  dest->JoinInfoset(src->GetInfoset());
  src->LeaveInfoset();
}

void ExtForm::AddPlayer(uint player_number)
{
  while (_players->Length() < player_number)
    *_players += gHandle<Player>(new Player);

  for (uint i = 1; i <= _outcomes->Length(); i++)
    (*_outcomes)[i]->SetNumPlayers(_players->Length());
}

void ExtForm::AddOutcome(uint outcome_number)
{
  while (_outcomes->Length() < outcome_number)
    *_outcomes += gHandle<Outcome>(new Outcome(_players->Length()));
}

bool ExtForm::IsTrivial(void) const   { return _trivial; }
bool ExtForm::IsModified(void) const  { return _modified; }

void ExtForm::RegisterIterator(ExtFormIter *iter)
	 { if (_iterator == 0)   _iterator = iter; }
void ExtForm::UnregisterIterator(ExtFormIter *iter)
	 { if (_iterator == iter)   _iterator = 0; }

//------------------------------------------------------------------------
//                       EXTFORMITER IMPLEMENTATION
//------------------------------------------------------------------------

ExtFormIter::ExtFormIter(ExtForm *extform) : _extform(extform)
{
  _extform->RegisterIterator(this);
  _cursor = _extform->RootNode();
}

ExtFormIter::~ExtFormIter()
{
  if (_extform)
    _extform->UnregisterIterator(this);
}

void ExtFormIter::GoParent(void)
{
  if (_cursor->_parent.IsNonNull())   _cursor = _cursor->_parent;
}

void ExtFormIter::GoFirstChild(void)
{
  if (_cursor->_children->Length())
    _cursor = (*_cursor->_children)[1];
}

void ExtFormIter::GoPriorSibling(void)
{
  if (!_cursor->_parent.IsNonNull())   return;

  int child_number = _cursor->_parent->_children->ElNumber(_cursor);
  if (child_number > 1)
    _cursor = (*_cursor->_parent)(child_number - 1);
}

void ExtFormIter::GoNextSibling(void)
{
  if (!_cursor->_parent.IsNonNull())   return;

  int child_number = _cursor->_parent->_children->ElNumber(_cursor);
  if (child_number < _cursor->_parent->_children->Length())
    _cursor = (*_cursor->_parent)(child_number + 1);
}

gHandle<Node> ExtFormIter::Cursor(void) const   { return _cursor; }
void ExtFormIter::SetCursor(const gHandle<Node> &n)
	{ _cursor = n; }
void ExtFormIter::Invalidate(void)
{
  _cursor = gHandle<Node>(0);
  _extform = 0;
}


int ExtForm::NumNodes(void) const
{
  return _nodes->Length();
}


