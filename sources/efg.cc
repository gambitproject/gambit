//
// FILE: extform.cc -- Implementation of extensive form data type
//
// $Id$
//

#include "extform.h"

//------------------------------------------------------------------------
//                EXTFORM: CONSTRUCTORS AND DESTRUCTOR
//------------------------------------------------------------------------

//************************************************************************
// ExtForm -- Construct the trivial extensive form
// Inputs:  None
// Outputs: '*this' is a properly initialized trivial extensive form
//************************************************************************

ExtForm::ExtForm(void) : _title("Untitled")
{
      // note that _root and _chance are constructed implicitly!!!

  _nodes += _root;
  _root.SetName("ROOT");

  Infoset root_iset(_dummy, 0);

  _infosets += root_iset;
  _root.JoinInfoset(root_iset);

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
  if (_iterator)   _iterator->Invalidate();
}

//-----------------------------------------------------------------------
//                     EXTFORM: OPERATIONS ON NODES
//-----------------------------------------------------------------------

Node ExtForm::RootNode(void)
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

int ExtForm::AddNode(Node &n, uint player, uint child_count)
{
      // make sure is a node in the tree
  if (!_nodes.IsMember(n))    return 0;

      // make sure is a terminal node
  if (n.NumChildren() != 0)   return 0;

  _nodes += n.AddChildren(child_count);

  AddPlayer(player);
  n.SetPlayer(_players[player]);
  
  for (uint i = 1; i <= child_count; i++)   {
    Infoset is(_dummy, 0);
    _infosets += is;
    n(i).JoinInfoset(is);
  }

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

int ExtForm::InsertNode(Node &n, uint player, uint branch_count)
{
  return 1;
}

//***********************************************************************
// LabelNode -- set label of a node
// Inputs:  n -- node to be labeled
//          label -- the new label
// Outputs: returns nonzero if successful
//***********************************************************************

int ExtForm::LabelNode(Node &n, gString &label)
{
      // check if node is in the tree
  if (!_nodes.IsMember(n))   return 0;

  n.SetName(label);

  return 1;
}

//**********************************************************************
// SetOutcome -- set the outcome associated with a node
// Inputs:  n -- the node to which to associate the outcome
//          outcome_number -- the number of the outcome
// Outputs: returns nonzero if successful
//*********************************************************************

int ExtForm::SetOutcome(Node &n, uint outcome_number)
{
  if (!_nodes.IsMember(n))   return 0;

  AddOutcome(outcome_number);
  n.SetOutcome(_outcomes[outcome_number]);

  return 1;
}

int ExtForm::GetOutcome(Node &n) const
{
  if (!_nodes.IsMember(n))   return 0;

  return _outcomes.ElNumber(n.GetOutcome());
}

//*********************************************************************
// DeleteNode -- delete a node from the extensive form
// Inputs:  n -- the node to be removed
//          keep -- the branch of the node 'n' to be kept
// Outputs: returns nonzero if successful
// Note:    does not deal with possibility of 'n' being terminal
//*********************************************************************

int ExtForm::DeleteNode(Node &n, Node &keep)
{
  return 1;
}

int ExtForm::JoinInfoset(Node &new_member, Node &to_iset)
{
  if (!_nodes.IsMember(new_member) || !_nodes.IsMember(to_iset))   return 0;

  new_member.JoinInfoset(to_iset.GetInfoset());

  return 1;
}

int ExtForm::LeaveInfoset(Node &n)
{
  if (!_nodes.IsMember(n))   return 0;

  Infoset new_infoset(n.GetInfoset().GetPlayer(), n.NumChildren());

  _infosets += new_infoset;

  n.JoinInfoset(new_infoset);

  return 1;
}

int ExtForm::LabelBranch(Node &n, gString &label)
{
  return 1;
}

int ExtForm::InsertBranch(Node &n, uint branch_number)
{
  return 1;
}

int ExtForm::DeleteBranch(Node &n, Node &remove)
{
  return 1;
}

int ExtForm::DeleteTree(Node &n)
{
  return 1;
}

int ExtForm::CopyTree(Node &n, Node &to)
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

void ExtForm::MoveTree(Node &src, Node &dest)
{
}

void ExtForm::AddPlayer(uint player_number)
{
}

void ExtForm::AddOutcome(uint outcome_number)
{
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
  _cursor = _cursor.Parent();
}

void ExtFormIter::GoFirstChild(void)
{
  if (!_cursor.IsTerminal())
    _cursor = _cursor(1);
}

void ExtFormIter::GoPriorSibling(void)
{
  if (_cursor.Parent() == _cursor)   return;

  int child_number = _cursor.Parent().ChildNumber(_cursor);
  if (child_number > 1)
    _cursor = _cursor.Parent()(child_number - 1);
}

void ExtFormIter::GoNextSibling(void)
{
  if (_cursor.Parent() == _cursor)   return;

  int child_number = _cursor.Parent().ChildNumber(_cursor);
  if (child_number < _cursor.Parent().NumChildren())
    _cursor = _cursor.Parent()(child_number + 1);
}

Node ExtFormIter::Cursor(void)    { return _cursor; }
void ExtFormIter::SetCursor(const Node &n)
	{ _cursor = n; }

void ExtFormIter::Invalidate(void)
{
  _cursor = Node(); 
  _extform = 0;
}


