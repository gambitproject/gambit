//#
//# FILE: extform.cc -- Implementation of extensive form data type
//#
//# $Id$
//#

#include "extform.h"
#include "node.h"
#include "outcome.h"
#include "gmapiter.h"

//#--------------------------------------------------------------------------
//#                ExtForm: Constructors and Destructors 
//#--------------------------------------------------------------------------

//
// Construct the trivial extensive form game
//
ExtForm::ExtForm(void) : players(0, 0), nodes((NodeSet *) 0),
                         outcomes((Outcome *) 0)
{
  players[0] = "CHANCE";
  nodes.Define(1, new NodeSet(1, 0, 0));
}


//
// Construct the extensive form game stored in the given file
//
ExtForm::ExtForm(gInput &f) : players(0, 0),
                              nodes((NodeSet *) 0), outcomes((Outcome *) 0)
{
  ReadEfgFile(f);
}

//
// Clean up an extensive form game, deallocating all memory
//
ExtForm::~ExtForm()
{
  gSparseSetIter<NodeSet *> nodeiter(nodes);
  for (nodeiter.GoFirst(); !nodeiter.PastEnd();
       delete nodes.Remove(nodeiter.GetKey()));

  gSparseSetIter<Outcome *> outciter(outcomes);
  for (outciter.GoFirst(); !outciter.PastEnd();
       delete outcomes.Remove(outciter.GetKey()));
}

//#--------------------------------------------------------------------------
//#                  ExtForm: Private auxiliary functions
//#--------------------------------------------------------------------------

void ExtForm::AddPlayer(int p)
{
  gSparseSetIter<NodeSet *> iter(nodes);
  for (iter.GoFirst(); !iter.PastEnd(); iter++)
    iter.GetValue()->AddPlayer(p);

  while (players.Last() < p)   players.Expand(1);
}

int ExtForm::CreateInfoset(int p, int game, int kids)
{
  return nodes(game)->CreateInfoset(p, kids);
}

//
// Delete a terminal node, and return the new identification of its parent
//
Node ExtForm::DeleteTerminalNode(const Node &n)
{
  Node parent;

  nodes(n.GetGame())->DeleteNode(n, parent);
  return parent;
}

//
// Delete the subtree rooted at n, and return the new identification of
// n's parent
//
Node ExtForm::DeleteSubtree(Node n)
{
  while (nodes(n.GetGame())->NumChildren(n))
    n = DeleteSubtree(nodes(n.GetGame())->GetChildNumber(n, 1));

  return DeleteTerminalNode(n);
}

//#--------------------------------------------------------------------------
//#                ExtForm: Title access and manipulation
//#--------------------------------------------------------------------------

void ExtForm::SetTitle(const gString &s)
{
  title = s;
}

const gString &ExtForm::GetTitle(void) const
{
  return title;
}

//#--------------------------------------------------------------------------
//#              ExtForm: Data access -- general information
//#--------------------------------------------------------------------------

//
// Returns the total number of nodes in all subgames
//
int ExtForm::NumNodes(void) const
{
  int total = 0;
  gSparseSetIter<NodeSet *> iter(nodes);
  for (iter.GoFirst(); !iter.PastEnd(); iter++)
    total += iter.GetValue()->NumNodes();
  return total;
}

//
// Returns the total number of nodes in a subgame
//
int ExtForm::NumNodes(int game) const
{
  if (nodes.IsDefined(game))
    return nodes(game)->NumNodes();
  else
    return 0;
}

//
// Returns the number of nodes at which player 'pl' has the decision in
// subgame number 'game'
//
int ExtForm::NumNodes(int game, int pl) const
{
  if (nodes.IsDefined(game))
    return nodes(game)->NumNodes(pl);
  else
    return 0;
}

//
// Returns the number of nodes in the information set identified by
// 'game', 'pl', and 'iset'
//
int ExtForm::NumNodes(int game, int pl, int iset) const
{
  if (nodes.IsDefined(game))
    return nodes(game)->NumNodes(pl, iset);
  else
    return 0;
}

//
// Returns the number of (personal) players defined in the game
//
int ExtForm::NumPlayers(void) const
{
  return players.Last();
}

//
// Returns the number of information sets controlled by player 'pl' in
// subgame 'game'
//
int ExtForm::NumInfosets(int game, int pl) const
{
  if (nodes.IsDefined(game))
    return nodes(game)->NumInfosets(pl);
  else
    return 0;
}

//
// Returns the number of outcomes defined
//
int ExtForm::NumOutcomes(void) const
{
  return outcomes.Length();
}


//#--------------------------------------------------------------------------
//#                  ExtForm: Subgame-related Operations
//#--------------------------------------------------------------------------

//
// Create a new subgame with the lowest undefined subgame number, and return
// the number of the new subgame
//
int ExtForm::CreateSubgame(void)
{
  int game = nodes.FirstVacancy();
  nodes.Define(game, new NodeSet(game, players.Last(), 0));
  return game;
}

//
// Create a new subgame with the given number
//
int ExtForm::CreateSubgame(int game, int from_file /* = 0 */)
{
  if (nodes.IsDefined(game) || game <= 0)   return 0;
  nodes.Define(game, new NodeSet(game, players.Last(), from_file));
  return game;
}

//
// Delete a subgame
//
void ExtForm::RemoveSubgame(int game)
{
  if (!nodes.IsDefined(game))   return;

  delete nodes.Remove(game);

  gSparseSetIter<NodeSet *> iter(nodes);
  for (iter.GoFirst(); !iter.PastEnd(); iter++)
    iter.GetValue()->ExpungeGame(game);

  if (game == 1)
    nodes.Define(1, new NodeSet(1, players.Last()));
}

//
// Return the label of a subgame
//
gString ExtForm::GetSubgameLabel(int game) const
{
  if (nodes.IsDefined(game))
    return nodes(game)->GetName();
  else
    return "";
}

//
// Set the label of a subgame
//
void ExtForm::LabelSubgame(int game, const gString &name)
{
  if (nodes.IsDefined(game))
    nodes(game)->SetName(name);
}

//
// Returns nonzero when the given subgame number is defined
//
int ExtForm::IsSubgameDefined(int game) const
{
  return nodes.IsDefined(game);
}


//#--------------------------------------------------------------------------
//#                    ExtForm: Node-related Operations
//#--------------------------------------------------------------------------

//
// Append a decision node at a terminal node n, belonging to player 'player',
// and having 'child_count' children.  The node is made the only member of
// a newly-created information set, and the identity of the node is returned.
//
Node ExtForm::AddNode(const Node &n, int player, int child_count)
{
  int game = n.GetGame();

  if (!nodes.IsDefined(game) ||
      !nodes(game)->IsMember(n))  return Node(1, -1, 1, 1);
  if (!nodes(game)->IsTerminal(n))   return n;

  AddPlayer(player);

  int iset_no = CreateInfoset(player, game, child_count);
  Node ret(game, player, iset_no, 0);
  nodes(game)->MoveNode(n, ret);
  for (int i = 1; i <= child_count; i++)  
    nodes(game)->CreateNode(-1, CreateInfoset(-1, game, 0), ret);
  return ret;
}

//
// Set the label of a node
//
void ExtForm::LabelNode(const Node &n, const gString &s)
{
  int game = n.GetGame();

  if (nodes.IsDefined(game) && nodes(game)->IsMember(n))
    nodes(game)->SetNodeName(n, s);
}

//
// Insert a node in the tree at the location of the given node.  The new
// node belongs to player 'pl, has 'children' children, and is placed in
// a newly-created information set.  The existing node becomes the first
// (eldest) child of the newly-created node.  The identity of the newly-
// created node is returned.
//
Node ExtForm::InsertNode(const Node &n, int pl, int children)
{
  int game = n.GetGame();

  if (!nodes.IsDefined(game) ||
      !nodes(game)->IsMember(n))    return Node(1, -1, 1, 1);

  AddPlayer(pl);

  int iset = CreateInfoset(pl, game, children);

  Node new_node = nodes(game)->InsertNode(pl, iset, n);

  for (int i = 1; i < children; i++)
    nodes(game)->CreateNode(-1, CreateInfoset(-1, game, 0), new_node);

  return n;
}

//
// Delete a node in the tree.  The 'keep'th node is kept and replaces
// the deleted node in the tree; the subtrees rooted by the other children
// are deleted.
//
Node ExtForm::DeleteNode(const Node &n, int keep)
{
  int game = n.GetGame();

  if (!nodes.IsDefined(game) ||
      !nodes(game)->IsMember(n))    return Node(1, -1, 1, 1);
  if (nodes(game)->IsTerminal(n))    return n;

  Node foo(n), ret;
  
  for (int i = 1; i < keep; i++)
    foo = DeleteTerminalNode(DeleteTree(nodes(game)->GetChildNumber(foo, 1)));
  while (nodes(game)->NumChildren(foo) > 1) 
    foo = DeleteTerminalNode(DeleteTree(nodes(game)->GetChildNumber(foo, 2)));
  nodes(game)->DeleteInternalNode(foo, ret);
  return ret;
}

//#--------------------------------------------------------------------------
//#                  ExtForm: Infoset-related Operations
//#--------------------------------------------------------------------------

//
// Moves 'new_node' from its current information set to the information set
// to which 'to_iset' belongs.  If this empties the information set, the
// information set is destroyed.  Returns the new identification of 'new_node'
//
Node ExtForm::JoinInfoset(const Node &new_node, const Node &to_iset)
{
  if (new_node.GetGame() != to_iset.GetGame())    return new_node;

  int game = new_node.GetGame();

  if (!nodes.IsDefined(game) ||
      !nodes(game)->IsMember(new_node) || !nodes(game)->IsMember(to_iset))
    return Node(1, -1, 1, 1);
  
  if (nodes(game)->NumChildren(new_node) != nodes(game)->NumChildren(to_iset))
    return new_node;

  if (new_node[1] == to_iset[1] && new_node[2] == to_iset[2])
    return new_node;

  Node ret(to_iset);
  
  if (nodes(game)->MoveNode(new_node, ret))
	//# take into account possible infoset renumbering...
    if (new_node[1] == ret[1] && new_node[2] < ret[2]) ret[2]--;
  return ret;
}

//
// Removes the node from its current information set and places it in a
// newly-created singleton information set owned by the same player.  Returns
// the new identification of 'n'.
//
Node ExtForm::LeaveInfoset(const Node &n)
{
  int game = n.GetGame();

  if (!nodes.IsDefined(game) ||
      !nodes(game)->IsMember(n))   return Node(1, -1, 1, 1);

  if (nodes(game)->NumNodes(n[1], n[2]) == 1)   return n;

  Node ret(game, n[1], CreateInfoset(n[1], game, nodes(game)->NumChildren(n)),
	   1);

      //# we already know this won't empty the infoset...
  nodes(game)->MoveNode(n, ret);

  return ret;
}

//
// Merges the nodes of the information set to which 'from' belongs into
// the information set to which 'into' belongs.  The newly-emptied information
// set is destroyed.  The new identification of 'from' is returned.
//
Node ExtForm::MergeInfoset(const Node &from, const Node &into)
{
  if (from.GetGame() != into.GetGame())   return from;

  int game = from.GetGame();

  if (!nodes.IsDefined(game) ||
      !nodes(game)->IsMember(from) || !nodes(game)->IsMember(into))
    return Node(1, -1, 1, 1);

  if (nodes(game)->NumChildren(from) != nodes(game)->NumChildren(into))
    return from;

  if (from[1] == into[1] && from[2] == into[2])
    return from;

  return nodes(game)->MoveNodes(from, into);
}

//
// Set the label of the information set to which node 'n' belongs
//
void ExtForm::LabelInfoset(const Node &n, const gString &label)
{
  int game = n.GetGame();

  if (nodes.IsDefined(game) && nodes(game)->IsMember(n))
    nodes(game)->SetInfosetName(n.GetPlayer(), n.GetInfoset(), label);
}

//
// Set the label of the information set identified by subgame, player, and
// information set number
//
void ExtForm::LabelInfoset(int game, int pl, int iset, const gString &label)
{
  nodes(game)->SetInfosetName(pl, iset, label);
}

//
// Return the label associated with the information set to which the node
// belongs.
//
gString ExtForm::GetInfosetLabel(const Node &n) const
{
  int game = n.GetGame();

  if (nodes.IsDefined(game) && nodes(game)->IsMember(n))
    return nodes(game)->GetInfosetName(n.GetPlayer(), n.GetInfoset());
  else
    return "";
}

//#--------------------------------------------------------------------------
//#                  ExtForm: Action-related Operations
//#--------------------------------------------------------------------------

//
// Append a new action to all members of the information set specified
// by the game, player, and information set number
//
void ExtForm::AppendAction(int game, int pl, int iset)
{
  if (nodes.IsDefined(game))
    nodes(game)->AppendAction(pl, iset);
}

//
// Insert a new action for all members of the information set of which
// 'n' is a member.
//
void ExtForm::InsertAction(const Node &n, int where, int number)
{
  int game = n.GetGame();

  if (!nodes.IsDefined(game) || !nodes(game)->IsMember(n))   return;
  
  for (int i = 0; i < number; i++)
    nodes(game)->InsertAction(n.GetPlayer(), n.GetInfoset(), where + i);

      //# we have to remember to insert the branch in all members of the iset
  for (i = 0; i < number; i++)
    for (int j = 1; j <= nodes(game)->NumNodes(n[1], n[2]); j++)   
      nodes(game)->InsertChild(-1, CreateInfoset(-1, game, 0), 
			       Node(game, n[1], n[2], j), where + i);
}


//
// Deletes an action from all members of the information set of which
// 'n' is a member.  The subtrees rooted by each of the corresponding
// children are destroyed.  The new identification of 'n' is returned.
//
Node ExtForm::DeleteAction(const Node &n, int which)
{
  int game = n.GetGame();

  if (!nodes.IsDefined(game) || !nodes(game)->IsMember(n))
    return Node(1, -1, 1, 1);

  nodes(game)->RemoveAction(n.GetGame(), n.GetInfoset(), which);

  Node ret(n);

      //# remember to remove branch from all members of infoset
  for (int i = 1; i <= nodes(game)->NumNodes(ret[1], ret[2]); i++)  
    ret = DeleteTerminalNode(
	     DeleteTree(nodes(game)->GetChildNumber(Node(game,ret[1],ret[2],i),
					     which)));

  return ret;
}

//
// Sets the label of the 'act'th action at the information set of which
// 'n' is a member.
//
void ExtForm::LabelAction(const Node &n, int act, const gString &label)
{
  int game = n.GetGame();

  if (nodes.IsDefined(game) && nodes(game)->IsMember(n))
    nodes(game)->SetActionName(n.GetPlayer(), n.GetInfoset(), act, label);
}

//
// Sets the label of the 'act'th action at the information set specified
// by the game and player number
//
void ExtForm::LabelAction(int game, int pl, int iset, int act,
			  const gString &label)
{
  if (nodes.IsDefined(game) && pl >= 0 && pl <= players.Last() &&
      iset > 0 && iset <= nodes(game)->NumInfosets(pl))
    nodes(game)->SetActionName(pl, iset, act, label);
}

//
// Returns the vector of probabilities for each branch at a chance node
//
gTuple<gNumber> ExtForm::GetActionProbs(const Node &n) const
{
  int game = n.GetGame();

  if (!nodes.IsDefined(game) ||
      !nodes(game)->IsMember(n) || n.GetPlayer() != 0)
    return gTuple<gNumber>(1, 0);
  return nodes(game)->GetActionProbs(n.GetPlayer(), n.GetInfoset());
}

//
// Returns the probability for a branch at a chance node
//
gNumber ExtForm::GetActionProb(const Node &n, int act) const
{
  int game = n.GetGame();

  if (!nodes.IsDefined(game) ||
      !nodes(game)->IsMember(n) || n.GetPlayer() != 0)
    return -1.0;
  return nodes(game)->GetActionProb(n.GetPlayer(), n.GetInfoset(), act);
}

//
// Set the vector of probabilities for each branch at a chance node
//
void ExtForm::SetActionProbs(const Node &n, const gTuple<gNumber> &probs)
{
  int game = n.GetGame();
  
  if (!nodes.IsDefined(game) ||
      !nodes(game)->IsMember(n) || n.GetPlayer() != 0)   return;
  nodes(game)->SetActionProbs(n.GetPlayer(), n.GetInfoset(), probs);
}

//
// Set the vector of probabilities for each branch for a chance node
//
void ExtForm::SetActionProbs(int game, int iset, const gTuple<gNumber> &probs)
{
  nodes(game)->SetActionProbs(0, iset, probs);
}


//#--------------------------------------------------------------------------
//#                  ExtForm: Subtree-related Operations
//#--------------------------------------------------------------------------

//
// Move the subtree rooted at 'src' to the terminal node 'dest', and return
// the new identification of 'src'.
//
Node ExtForm::MoveTree(Node src, Node dest)
{
  if (src.GetGame() != dest.GetGame())    return src;

  int game = src.GetGame();

  if (!nodes.IsDefined(game) ||
      !nodes(game)->IsMember(src) || !nodes(game)->IsMember(dest)) 
    return Node(1, -1, 1, 1);
  if (!nodes(game)->IsTerminal(dest))   return src;

  Node ret(src);
  nodes(game)->AdoptChildren(src, dest);
  nodes(game)->MoveNode(src, dest, 1);
  nodes(game)->MoveNode(Node(game, dest.GetPlayer(), dest.GetInfoset(), 1),
			ret);
 
  return ret;
}

//
// Copy the subtree rooted art 'src' to the terminal node 'dest', and return
// the new identification of 'src'.
//
Node ExtForm::CopyTree(Node src, Node dest)
{
  if (src.GetGame() != dest.GetGame())    return src;
  
  int game = src.GetGame();

  if (!nodes.IsDefined(game) ||
      !nodes(game)->IsMember(src) || !nodes(game)->IsMember(dest))
    return Node(1, -1, 1, 1);
  if (!nodes(game)->IsTerminal(dest) || nodes(game)->IsTerminal(src))
    return src;

  if (nodes(game)->IsPredecessor(src, dest))   return src;

  Node foo(game, src.GetPlayer(), src.GetInfoset(), 0);

  nodes(game)->MoveNode(dest, foo);

  for (int i = 1; i <= nodes(game)->NumChildren(src); i++)   {
    nodes(game)->CreateNode(-1, CreateInfoset(-1, game, 0), foo);
    CopyTree(nodes(game)->GetChildNumber(src, i),
	     nodes(game)->GetChildNumber(foo, i));
  }

  return src;
}

//
// Delete the subtree rooted at 'n', leaving 'n' as a terminal node.  Returns
// the new identification of 'n'.
//
Node ExtForm::DeleteTree(const Node &n)
{
  int game = n.GetGame();

  if (!nodes.IsDefined(game) || !nodes(game)->IsMember(n))
    return Node(1, -1, 1, 1);
  if (nodes(game)->IsTerminal(n))    return n;

  Node foo(n);

  while (nodes(game)->NumChildren(foo))
    foo = DeleteSubtree(nodes(game)->GetChildNumber(foo, 1));

  Node ret(game, -1, CreateInfoset(-1, game, 0), 1);

  nodes(game)->MoveNode(foo, ret);

  return ret;
}

//#--------------------------------------------------------------------------
//#                  ExtForm: Outcome-related Operations
//#--------------------------------------------------------------------------

//
// Create a new outcome with the lowest unused outcome number, and return
// the number of the newly-created outcome
//
int ExtForm::CreateOutcome(void)
{
  int outc = outcomes.FirstVacancy();
  outcomes.Define(outc, new Outcome());
  return outc;
}

//
// Create a new outcome with the given number
//
int ExtForm::CreateOutcome(int outc)
{
  if (outcomes.IsDefined(outc) || outc <= 0)    return 0;
  outcomes.Define(outc, new Outcome());
  return outc;
}

//
// Delete an outcome
//
void ExtForm::RemoveOutcome(int outc)
{
  if (!outcomes.IsDefined(outc))   return;
  delete outcomes.Remove(outc);
  
  gSparseSetIter<NodeSet *> iter(nodes);
  for (iter.GoFirst(); !iter.PastEnd(); iter++)
    iter.GetValue()->ExpungeOutcome(outc);
}

//
// Return the label of an outcome
//
gString ExtForm::GetOutcomeLabel(int outc) const
{
  if (outcomes.IsDefined(outc))
    return outcomes(outc)->GetOutcomeName();
  else
    return "";
}

//
// Set the label of an outcome
//
void ExtForm::LabelOutcome(int outc, const gString &name)
{
  if (outcomes.IsDefined(outc))
    outcomes(outc)->SetOutcomeName(name);
}

//
// Returns nonzero if the given outcome number is defined
//
int ExtForm::IsOutcomeDefined(int outc) const
{
  return outcomes.IsDefined(outc);
}

//
// Returns the payoff vector associated with an outcome
//
gTuple<gNumber> ExtForm::GetOutcomeValues(int outc) const
{
  if (outcomes.IsDefined(outc))
    return outcomes(outc)->GetOutcomeVector(players.Last());
  else
    return gTuple<gNumber>();
}

//
// Set the payoff vector associated with an outcome
//
void ExtForm::SetOutcomeValues(int outc, const gTuple<gNumber> &vals)
{
  if (outcomes.IsDefined(outc))
    outcomes(outc)->SetOutcome(vals);
}

//
// Set the payoff associated with an outcome for one player
//
void ExtForm::SetOutcomeValue(int outc, int pl, gNumber value)
{
  if (outcomes.IsDefined(outc) && pl <= players.Last())
    outcomes(outc)->SetOutcome(pl, value);
}


//#--------------------------------------------------------------------------
//#                    ExtForm: Input/Output Operations
//#--------------------------------------------------------------------------

extern gInput *efg_input_stream;

void ExtForm::ReadEfgFile(gInput &f)
{
  if (!f.IsValid())   return;
  
  if (nodes.Length() > 0)   {
    gSparseSetIter<NodeSet *> nodeiter(nodes);
    for (nodeiter.GoFirst(); !nodeiter.PastEnd();
	 delete nodes.Remove(nodeiter.GetKey()));
  }
  
  if (outcomes.Length() > 0)  {
    gSparseSetIter<Outcome *> outciter(outcomes);
    for (outciter.GoFirst(); !outciter.PastEnd();
	 delete outcomes.Remove(outciter.GetKey()));
  }

  players = gTuple<gString>(0, 0);

  efg_input_stream = &f;

  if (EfgYaccer())   {
    gSparseSetIter<NodeSet *> nodeiter(nodes);
    for (nodeiter.GoFirst(); !nodeiter.PastEnd();
	 delete nodes.Remove(nodeiter.GetKey()));

    gSparseSetIter<Outcome *> outciter(outcomes);
    for (outciter.GoFirst(); !outciter.PastEnd();
	 delete outcomes.Remove(outciter.GetKey()));
    
    title = "";

    nodes.Define(1, new NodeSet(1, 0, 0));
  }
}

void ExtForm::WriteEfgFile(gOutput &f) const
{
  f << "{ \"" << title << "\"\n\n{ ";
  for (int i = 0; i <= players.Last(); i++)
    f << '"' << players[i] << '"' << ' ';
  f << "}\n\n";
  
  f << "{ ";

  gSparseSetIter<Outcome *> out_iter(outcomes);

  int flag = 0;
  for (out_iter.GoFirst(); !out_iter.PastEnd(); out_iter++)   {
    if (flag)
      f << "\n  ";
    else
      flag = 1;
    f << "{ " << out_iter.GetKey() << ' ' 
      << out_iter.GetValue()->GetOutcomeVector(NumPlayers())
      << " \"" << out_iter.GetValue()->GetOutcomeName() << "\" }";
  }
  f << " }\n\n";

  gSparseSetIter<NodeSet *> game_iter(nodes);
  for (game_iter.GoFirst(); !game_iter.PastEnd(); game_iter++)   {
    NodeSet *game = game_iter.GetValue();

    f << "{ " << game_iter.GetKey() << " \"" << game->GetName() << '"' << '\n';
    f << "  {";
    for (i = 0; i <= game->NumPlayers(); i++)   {
      f << ((i == 0) ? " {" : "    {");
      for (int j = 1; j <= game->NumInfosets(i); j++)  {
	f << ((j == 1) ? " " : "\n      ");
	f << "{ \"" << game->GetInfosetName(i, j) << "\" {";
	for (int k = 1; k <= game->NumActions(i, j); k++)
	  f << " \"" << game->GetActionName(i, j, k) << '"';
	f << " }";
	
	if (i == 0)   { // chance player, print branch probs
	  f << " { ";
	  for (k = 1; k <= game->NumActions(i, j); k++)
	    f << game->GetActionProb(0, j, k) << ' ';
	  f << '}';
	}
	
	f << " }";
      }
      f << " }\n";
    }
    f << "  }\n\n";  

    game->WriteToFile(f);
  
    f << "}\n";
  }
  
  f << "\n}\n";
}

//#--------------------------------------------------------------------------
//#                    ExtForm: Data access -- Nodes
//#--------------------------------------------------------------------------

//
// Returns the root node of subgame number 'game'
//
Node ExtForm::RootNode(int game /* = 1 */) const
{
  if (nodes.IsDefined(game))
    return nodes(game)->RootNode();
  else
    return Node();
}

//
// Returns the parent node of the given node
//
Node ExtForm::GetParent(const Node &n) const
{
  int game = n.GetGame();

  if (nodes.IsDefined(game) && nodes(game)->IsMember(n))
    return nodes(game)->GetParent(n);
  else
    return Node();
}

//
// Returns the number of children of a given node
//
int ExtForm::NumChildren(const Node &n) const
{
  int game = n.GetGame();

  if (nodes.IsDefined(game) && nodes(game)->IsMember(n))
    return nodes(game)->NumChildren(n);
  else
    return 0;
}

//
// Returns the 'number'th child of a given node
//
Node ExtForm::GetChildNumber(const Node &n, int number) const
{
  int game = n.GetGame();

  if (nodes.IsDefined(game) && nodes(game)->IsMember(n))
    return nodes(game)->GetChildNumber(n, number);
  else
    return Node();
}

//
// Returns all the children of a given node
//
gBlock<Node> ExtForm::GetChildren(const Node &n) const
{
  int game = n.GetGame();

  if (nodes.IsDefined(game) && nodes(game)->IsMember(n))
    return nodes(game)->GetChildren(n);
  else
    return gBlock<Node>();
}

//
// Returns the prior sibling of a given node
//
Node ExtForm::GetPriorSibling(const Node &n) const
{
  int game = n.GetGame();

  if (nodes.IsDefined(game) && nodes(game)->IsMember(n))
    return nodes(game)->GetPriorSibling(n);
  else
    return Node();
}

//
// Returns the next sibling of a given node
//
Node ExtForm::GetNextSibling(const Node &n) const
{
  int game = n.GetGame();

  if (nodes.IsDefined(game) && nodes(game)->IsMember(n))
    return nodes(game)->GetNextSibling(n);
  else
    return Node();
}

//
// Returns true (nonzero) when a continuation subgame has been attached to
// the node
//
int ExtForm::HasSuccessorGame(const Node &n) const
{
  int game = n.GetGame();

  if (nodes.IsDefined(game) && nodes(game)->IsMember(n))
    return (nodes(game)->GetNextGame(n) != 0);
  else
    return 0;
}

//
// Returns the root node of the successor game attached to the node
//
Node ExtForm::GetSuccessorGameRoot(const Node &n) const
{
  int game = n.GetGame();

  if (nodes.IsDefined(game) && nodes(game)->IsMember(n) &&
      nodes(game)->GetNextGame(n) != 0)
    return nodes(nodes(game)->GetNextGame(n))->RootNode();
  else
    return Node();
}

//
// Returns nonzero if 'n' is a successor node of 'from'
//
int ExtForm::IsSuccessor(const Node &n, const Node &from) const
{
  if (n.GetGame() != from.GetGame())   return 0;

  int game = n.GetGame();

  return (nodes.IsDefined(game) && nodes(game)->IsMember(n) &&
	  nodes(game)->IsMember(from) && nodes(game)->IsPredecessor(from, n));
}

//
// Returns nonzero if 'n' is a predecessor node of 'of'
//
int ExtForm::IsPredecessor(const Node &n, const Node &of) const
{
  if (n.GetGame() != of.GetGame())   return 0;

  int game = n.GetGame();

  return (nodes.IsDefined(game) && nodes(game)->IsMember(n) &&
	  nodes(game)->IsMember(of) && nodes(game)->IsPredecessor(n, of));
}

//
// Returns the label of a node
//
gString ExtForm::GetNodeLabel(const Node &n) const
{
  int game = n.GetGame();

  if (nodes.IsDefined(game) && nodes(game)->IsMember(n))
    return nodes(game)->GetNodeName(n);
  else
    return "";
}

//
// Returns the number of the outcome associated with a node, if any
//
int ExtForm::GetOutcome(const Node &n) const
{
  int game = n.GetGame();

  if (nodes.IsDefined(game) && nodes(game)->IsMember(n))
    return nodes(game)->GetOutcome(n);
  else
    return 0;
}

//
// Returns the number of the continuation subgame associated with a node,
// if any
//
int ExtForm::GetNextGame(const Node &n) const
{
  int game = n.GetGame();

  if (nodes.IsDefined(game) && nodes(game)->IsMember(n))
    return nodes(game)->GetNextGame(n);
  else
    return 0;
}

