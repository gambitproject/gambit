//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Implementation of extensive form basis class
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

#include "efbasis.h"
#include "base/garray.imp"
#include "math/gvector.h"
#include "math/gpvector.h"
#include "math/gdpvect.imp"
#include "math/gmatrix.h"
#include "base/gnullstatus.h"
#include "efgutils.h"
#include "actiter.h"
#include "numerical/lpsolve.h"   

class EFNodeArrays   {
friend class gbtEfgNodeSet;
protected:
  gbtBlock<gbtEfgNode> nodes;
  
public:
  EFNodeArrays(const gbtArray<gbtEfgNode> &a);
  EFNodeArrays ( const EFNodeArrays &a);
  virtual ~EFNodeArrays();
  EFNodeArrays &operator=( const EFNodeArrays &a);
  bool operator==( const EFNodeArrays &a) const;
};

//----------------------------------------------------
// EFNodeArray: Constructors, Destructor, operators
// ---------------------------------------------------

EFNodeArrays::EFNodeArrays(const gbtArray<gbtEfgNode> &n)
  : nodes(n.Length())
{
  for (int i = 1; i <= nodes.Length(); i++)
    nodes[i] = n[i];
}

EFNodeArrays::EFNodeArrays(const EFNodeArrays &n)
  : nodes(n.nodes)
{ }

EFNodeArrays::~EFNodeArrays ()
{ }

EFNodeArrays &EFNodeArrays::operator=( const EFNodeArrays &n)
{
  nodes = n.nodes; 
  return *this;
}

#ifdef __BORLANDC__
bool operator==(const gbtArray<gbtEfgNode> &a, const gbtArray<gbtEfgNode> &b)
{
  if (a.First() != b.First() || a.Last() != b.Last())  {
    return false;
  }
  for (int i = a.First(); i <= a.Last(); i++) {
    if (a[i] != b[i])   return false;
  }
  return true;
}
#endif

bool EFNodeArrays::operator==(const EFNodeArrays &a) const
{
  return (nodes == a.nodes);
}

class gbtEfgNodeSet{

protected:
  gbtEfgPlayer efp;
  gbtArray < EFNodeArrays *> infosets;
public:
  
  //----------------------------------------
  // Constructors, Destructor, operators
  //----------------------------------------

//  gbtEfgNodeSet();
  gbtEfgNodeSet(const gbtEfgNodeSet &);
  gbtEfgNodeSet(const gbtEfgPlayer &);
  virtual ~gbtEfgNodeSet();

  gbtEfgNodeSet &operator=(const gbtEfgNodeSet &);
  bool operator==(const gbtEfgNodeSet &s) const;

  //--------------------
  // Member Functions
  //--------------------

  // Append a Node to an infoset;
  void AddNode(int iset, const gbtEfgNode &);

  // Insert a Node in a particular place in an infoset;
  void AddNode(int iset, const gbtEfgNode &, int index);


  // Remove a Node at int i, returns the removed Node pointer
  gbtEfgNode RemoveNode(int iset, int i);

  // Remove a Node from an infoset . 
  // Returns true if the Node was successfully removed, false otherwise.
  bool RemoveNode(int iset, const gbtEfgNode &);

  // Get a garray of the Nodes in an Infoset
  const gbtArray<gbtEfgNode> &NodeList(int iset) const
     { return infosets[iset]->nodes; }
  
  // Get a Node
  gbtEfgNode GetNode(int iset, int index);

  // returns the index of the Node if it is in the NodeSet
  int Find(const gbtEfgNode &) const;

  // Number of Nodes in a particular infoset
  int NumNodes(int iset) const;

  // return the player of the gbtEfgNodeSet
  gbtEfgPlayer GetPlayer(void) const;

  // checks for a valid gbtEfgNodeSet
  bool IsValid(void) const;

};

//--------------------------------------------------
// gbtEfgNodeSet: Constructors, Destructor, operators
//--------------------------------------------------

gbtEfgNodeSet::gbtEfgNodeSet(const gbtEfgPlayer &p)
  : efp(p), infosets(p.NumInfosets())
{
  for (int i = 1; i <= p.NumInfosets(); i++) {
    gbtArray<gbtEfgNode> members(p.GetInfoset(i).NumMembers());
    for (int j = 1; j <= members.Length(); j++) {
      members[j] = p.GetInfoset(i).GetMember(j);
    }
    infosets[i] = new EFNodeArrays(members);
  }
}

gbtEfgNodeSet::gbtEfgNodeSet( const gbtEfgNodeSet &s )
: infosets(s.infosets.Length())
{
  efp = s.efp;
  for (int i = 1; i <= s.infosets.Length(); i++){
    infosets[i] = new EFNodeArrays(*(s.infosets[i]));
  }
}

gbtEfgNodeSet::~gbtEfgNodeSet()
{ 
  for (int i = 1; i <= infosets.Length(); i++)
    delete infosets[i];
}

gbtEfgNodeSet &gbtEfgNodeSet::operator=(const gbtEfgNodeSet &s)
{
  if (this != &s && efp == s.efp) {
    for (int i = 1; i<= infosets.Length(); i++)  {
      delete infosets[i];
      infosets[i] = new EFNodeArrays(*(s.infosets[i]));
    }
  }    
  return *this;
}

bool gbtEfgNodeSet::operator==(const gbtEfgNodeSet &s) const
{
  if (infosets.Length() != s.infosets.Length() ||
      efp != s.efp)
    return false;
  
  int i;
  for (i = 1; i <= infosets.Length() && 
       *(infosets[i]) == *(s.infosets[i]);  i++);
  return (i > infosets.Length());
}

//------------------------------------------
// gbtEfgNodeSet: Member functions 
//------------------------------------------

// Append a Node to a particular infoset;
void gbtEfgNodeSet::AddNode(int iset, const gbtEfgNode &s)
{ 
  infosets[iset]->nodes.Append(s); 
}

// Insert a Node  to a particular infoset at a particular place;
void gbtEfgNodeSet::AddNode(int iset, const gbtEfgNode &s, int index) 
{ 
  infosets[iset]->nodes.Insert(s,index); 
}

// Remove a Node from infoset iset at int i, 
// returns the removed Infoset pointer
gbtEfgNode gbtEfgNodeSet::RemoveNode(int iset, int i) 
{ 
  return (infosets[iset]->nodes.Remove(i)); 
}

// Removes a Node from infoset iset . Returns true if the 
//Node was successfully removed, false otherwise.
bool gbtEfgNodeSet::RemoveNode(int iset, const gbtEfgNode &s)
{ 
  int t = infosets[iset]->nodes.Find(s); 
  if (t>0) infosets[iset]->nodes.Remove(t); 
  return (t>0); 
} 

// Get a Node
gbtEfgNode gbtEfgNodeSet::GetNode(int iset, int index)
{
  return (infosets[iset]->nodes)[index];
}

// Number of Nodes in a particular infoset
int gbtEfgNodeSet::NumNodes(int iset) const
{
  return (infosets[iset]->nodes.Length());
}

// Return the player of this gbtEfgNodeSet
gbtEfgPlayer gbtEfgNodeSet::GetPlayer(void) const
{
  return efp;
}

int gbtEfgNodeSet::Find(const gbtEfgNode &n) const
{
  return (infosets[n.GetInfoset().GetId()]->nodes.Find(n));
}

// checks for a valid gbtEfgNodeSet
bool gbtEfgNodeSet::IsValid(void) const
{
  if (infosets.Length() != efp.NumInfosets())   return false;

  for (int i = 1; i <= infosets.Length(); i++)
    if (infosets[i]->nodes.Length() == 0)   return false;

  return true;
}

//--------------------------------------------------
// gbtEfgBasis: Constructors, Destructors, Operators
//--------------------------------------------------

gbtEfgBasis::gbtEfgBasis(const gbtEfgGame &p_efg) 
  : gbtEfgSupport(p_efg), nodes(p_efg.NumPlayers())
{
  for (int i = 1; i <= nodes.Length(); i++) {
    nodes[i] = new gbtEfgNodeSet(p_efg.GetPlayer(i));
  }
}

gbtEfgBasis::gbtEfgBasis(const gbtEfgBasis &b)
  : gbtEfgSupport(b), nodes(b.nodes.Length())
{
  for (int i = 1; i <= nodes.Length(); i++)
    nodes[i] = new gbtEfgNodeSet(*(b.nodes[i]));
}

gbtEfgBasis::~gbtEfgBasis()
{
  for (int i = 1; i <= nodes.Length(); i++)
    delete nodes[i];
}

gbtEfgBasis &gbtEfgBasis::operator=(const gbtEfgBasis &b)
{
  gbtEfgSupport::operator=(b);
  return *this;
}

bool gbtEfgBasis::operator==(const gbtEfgBasis &b) const
{
  if( (*this).gbtEfgSupport::operator!=(b)) return false;

  if (nodes.Length() != b.nodes.Length()) return false;

  int i;
  for (i = 1; i <= nodes.Length() && *(nodes[i]) == *(b.nodes[i]); i++);
  return (i > nodes.Length());
}

bool gbtEfgBasis::operator!=(const gbtEfgBasis &b) const
{
  return !(*this == b);
}

//-----------------------------
// gbtEfgBasis: Member Functions 
//-----------------------------

int gbtEfgBasis::NumNodes(const gbtEfgInfoset &infoset) const
{
  return nodes[infoset.GetPlayer().GetId()]->NumNodes(infoset.GetId());
}

int gbtEfgBasis::NumNodes(int pl, int iset) const
{
  return nodes[pl]->NumNodes(iset);
}

const gbtArray<gbtEfgNode> &gbtEfgBasis::Nodes(int pl, int iset) const
{
  return nodes[pl]->NodeList(iset);
}

gbtEfgNode gbtEfgBasis::GetNode(const gbtEfgInfoset &infoset, int index) const
{
  return nodes[infoset.GetPlayer().GetId()]->GetNode(infoset.GetId(), index);
}

int gbtEfgBasis::Find(const gbtEfgNode &n) const
{
  if (n.GetInfoset().GetGame() != m_efg)   return 0;

  int pl = n.GetInfoset().GetPlayer().GetId();

  return nodes[pl]->Find(n);
}

bool gbtEfgBasis::IsValid(void) const
{
  if(!(*this).gbtEfgSupport::HasActiveActionsAtAllInfosets()) return false;
  if (nodes.Length() != m_efg.NumPlayers())   return false;
  for (int i = 1; i <= nodes.Length(); i++)
    if (!nodes[i]->IsValid())  return false;

  return true;
}

gbtPVector<int> gbtEfgBasis::NumNodes(void) const
{
  gbtArray<int> foo(m_efg.NumPlayers());
  int i;
  for (i = 1; i <= m_efg.NumPlayers(); i++)
    foo[i] = nodes[i]->GetPlayer().NumInfosets();

  gbtPVector<int> bar(foo);
  for (i = 1; i <= m_efg.NumPlayers(); i++)
    for (int j = 1; j <= nodes[i]->GetPlayer().NumInfosets(); j++)
      bar(i, j) = NumNodes(i,j);

  return bar;
}  

bool gbtEfgBasis::RemoveNode(const gbtEfgNode &n)
{
  gbtEfgInfoset infoset = n.GetInfoset();
  gbtEfgPlayer player = infoset.GetPlayer();

  return nodes[player.GetId()]->RemoveNode(infoset.GetId(), n);
}

bool gbtEfgBasis::IsReachable(gbtEfgNode n) const
{
  if (n == m_efg.GetRoot()) {
    return true;
  }

  while (n != m_efg.GetRoot()) {
    if (!n.GetParent().GetInfoset().IsChanceInfoset()) {
      if (!gbtEfgSupport::Contains(LastAction(m_efg, n))) {
	return false;
      }
    }
    n = n.GetParent();
  }
  return true;
}

void gbtEfgBasis::AddNode(const gbtEfgNode &n)
{
  gbtEfgInfoset infoset = n.GetInfoset();
  gbtEfgPlayer player = infoset.GetPlayer();

  nodes[player.GetId()]->AddNode(infoset.GetId(), n);
}

bool gbtEfgBasis::IsConsistent(void) const
{
  bigbasis = new gbtEfgBasis(m_efg);
  nodeIndex = new gbtDPVector<int>(bigbasis->NumNodes());
  actIndex = new gbtDPVector<int>(bigbasis->NumActions());
  MakeIndices();
  // gout << "\nactIndex:  " << (*actIndex);
  // gout << "\nnodeIndex: " << (*nodeIndex);
 
  int num_vars = num_act_vars + num_node_vars;

  // gout << "\nnum_eqs: " << num_eqs;
  // gout << " num_ineqs: " << num_ineqs;

  // gout << "\nnum_act_vars: " << num_act_vars;
  // gout << " num_node_vars: " << num_node_vars;
  // gout << " num_vars: " << num_vars;

  A = new gbtMatrix<double>(1,num_eqs+num_ineqs,1,num_vars);
  b = new gbtVector<double>(1,num_eqs+num_ineqs);
  c = new gbtVector<double>(1,num_vars);
  (*A) = 0.0; (*b) = 0.0; (*c)= 0.0;

  MakeAb();
  for (int i = 1; i <= num_act_vars; i++)
    (*c)[i]=-1.0;

  // gout << "\nA: \n" << (*A);
  // gout << "\nb: \n" << (*b);
  // gout << "\nc: \n" << (*c);

  gbtNullStatus status;
  LPSolve<double> lp((*A),(*b),(*c),num_eqs, status);

  // gout << "\noptimum: " << lp.OptimumVector();
  //  if(!lp.IsWellFormed()) gout << "\nLP not well formed";
  //  if(!lp.IsBounded()) gout << "\nLP not bounded";
  bool flag = lp.IsFeasible();
  for (int i = 1; i <= num_act_vars; i++)
    if(lp.OptimumVector()[i]<=0.0) flag = false;
  if(flag)
    GetConsistencySolution(lp.OptimumVector());

  delete A;
  delete b;
  delete c;

  delete bigbasis;
  delete nodeIndex;
  delete actIndex;

  return flag;
}

void gbtEfgBasis::MakeIndices(void) const
{
  int i,j;
  int ind = 1;

  for(i=1;i<=m_efg.NumPlayers();i++)
    for(j=1;j<=(m_efg.NumInfosets())[i];j++) {
      int k = 1;
      for (gbtActionIterator action(*bigbasis, i, j);
	   !action.End(); action++, k++)  {
	if (Contains(*action)) {
	  (*actIndex)(i,j,k)=0;
	}
	else {
	  (*actIndex)(i,j,k) = ind++;
	}
      }
    }
  num_act_vars=ind-1;
  for(i=1;i<=m_efg.NumPlayers();i++)
    for(j=1;j<=(m_efg.NumInfosets())[i];j++) 
      for(int k=1;k<=bigbasis->NumNodes(i,j);k++) {
	if(IsReachable(bigbasis->Nodes(i,j)[k]))
	  (*nodeIndex)(i,j,k)=0;
	else 
	  (*nodeIndex)(i,j,k) = ind++;
      }
  num_node_vars=ind-num_act_vars-1;
  MakeRowIndices();
}

void gbtEfgBasis::MakeRowIndices(void) const
{
  int i,j,k,kk;

  num_eqs = 0;
  num_ineqs = 0;
  for(i=1;i<=m_efg.NumPlayers();i++)
    for(j=1;j<=(m_efg.NumInfosets())[i];j++) {
      for(k=1;k<=bigbasis->NumActions(i,j);k++)
	if((*actIndex)(i,j,k))
	  num_ineqs++;
      for(k=1;k<=bigbasis->NumNodes(i,j);k++)
	if((*nodeIndex)(i,j,k))
	  num_eqs++;
      for(k=1;k<=bigbasis->NumNodes(i,j);k++)
	for(kk=k+1;kk<=bigbasis->NumNodes(i,j);kk++)
	  if(Find(bigbasis->Nodes(i,j)[k]))
	    if(Find(bigbasis->Nodes(i,j)[kk])) {
	      if((*nodeIndex)(i,j,k) || (*nodeIndex)(i,j,kk))
		num_eqs++;
	    }
	    else
	      num_ineqs++;
	  else
	    if(Find(bigbasis->Nodes(i,j)[kk]))
	      num_ineqs++;
    }
}

void gbtEfgBasis::MakeAb(void) const
{
  int i,j,k,kk;
  int eq = num_ineqs+1;
  int ineq = 1;

  for(i=1;i<=m_efg.NumPlayers();i++)
    for(j=1;j<=(m_efg.NumInfosets())[i];j++) {
      for (k=1;k<=bigbasis->NumActions(i,j);k++)
	if((*actIndex)(i,j,k))
	  AddEquation1(ineq++,bigbasis->GetAction(i,j,k));
      for(k=1;k<=bigbasis->NumNodes(i,j);k++)
	if((*nodeIndex)(i,j,k))
	  AddEquation2(eq++,bigbasis->Nodes(i,j)[k]);
      for(k=1;k<=bigbasis->NumNodes(i,j);k++)
	for(kk=k+1;kk<=bigbasis->NumNodes(i,j);kk++)
	  if(Find(bigbasis->Nodes(i,j)[k]))
	    if(Find(bigbasis->Nodes(i,j)[kk])) {
	      if((*nodeIndex)(i,j,k) || (*nodeIndex)(i,j,kk))
		AddEquation3(eq++,bigbasis->Nodes(i,j)[k],
			     bigbasis->Nodes(i,j)[kk]);
	    }
	    else
	      AddEquation4(ineq++,bigbasis->Nodes(i,j)[k],
			   bigbasis->Nodes(i,j)[kk]);
	  else
	    if(Find(bigbasis->Nodes(i,j)[kk]))
	      AddEquation4(ineq++,bigbasis->Nodes(i,j)[kk],
			   bigbasis->Nodes(i,j)[k]);
    }
}

int gbtEfgBasis::Col(const gbtEfgAction &p_action) const
{
  gbtEfgInfoset iset = p_action.GetInfoset();
  return (*actIndex)(iset.GetPlayer().GetId(), iset.GetId(),
		     (*bigbasis).gbtEfgSupport::GetIndex(p_action));
}

int gbtEfgBasis::Col(const gbtEfgNode &n) const
{
  gbtEfgInfoset iset = n.GetInfoset();
  return (*nodeIndex)(iset.GetPlayer().GetId(), iset.GetId(),
		      (*bigbasis).Find(n));
}

void gbtEfgBasis::AddEquation1(int row, const gbtEfgAction &p_action) const
{
  if(Col(p_action))
    (*A)(row,Col(p_action)) = -1.0;
  (*b)[row] = -1.0;
}

void gbtEfgBasis::AddEquation2(int row, gbtEfgNode n) const
{
  if(Col(n))
    (*A)(row,Col(n)) = 1.0;
  if(n!=m_efg.GetRoot()) {
    gbtEfgAction act = LastAction(m_efg,n);
    if(Col(act))
      (*A)(row,Col(act)) = -1.0;
    while(n.GetParent() != m_efg.GetRoot()) {
      n = n.GetParent();
      act = LastAction(m_efg,n);
      if(Col(act))
	(*A)(row,Col(act)) = -1.0;
    }
  }
}

void gbtEfgBasis::AddEquation3(int row, const gbtEfgNode &n1, 
			   const gbtEfgNode &n2) const
{
  if(Col(n1))
    (*A)(row,Col(n1)) = 1.0;
  if(Col(n2))
    (*A)(row,Col(n2)) = -1.0;
}

void gbtEfgBasis::AddEquation4(int row, const gbtEfgNode &n1,
			   const gbtEfgNode &n2) const
{
  if(Col(n1))
    (*A)(row,Col(n1)) = 1.0;
  if(Col(n2))
    (*A)(row,Col(n2)) = -1.0;
  (*b)[row] = -1.0;
}

void gbtEfgBasis::GetConsistencySolution(const gbtVector<double> &x) const
{
  gbtDPVector<int> nodes(bigbasis->NumNodes());
  gbtDPVector<int> acts(bigbasis->NumActions());
  nodes = 0;
  acts = 0;
  int i,j,k;
  for(i=1;i<=m_efg.NumPlayers();i++)
    for(j=1;j<=(m_efg.NumInfosets())[i];j++) {
      for(k=1;k<=bigbasis->NumActions(i,j);k++)
	if((*actIndex)(i,j,k))
	  acts(i,j,k) = (int)x[(*actIndex)(i,j,k)];
      for(k=1;k<=bigbasis->NumNodes(i,j);k++) 
	if((*nodeIndex)(i,j,k))
	  nodes(i,j,k) = (int)x[(*nodeIndex)(i,j,k)];
    }
  //  gout << "\nacts: " << acts;
  //  gout << "\nnodes: " << nodes;
}

void gbtEfgBasis::Dump(gbtOutput& s) const
{
  int numplayers;
  int i;
  int j;
  int k;

  s << "\nActions: ";
  (*this).gbtEfgSupport::Dump(s);
  s << "\nNodes:   ";
  s << "{ ";
  numplayers = m_efg.NumPlayers();
  for (i = 1; i <= numplayers; i++)  {
    gbtEfgPlayer player = nodes[i]->GetPlayer();
    s << '"' << player.GetLabel() << "\" { ";
    for (j = 1; j <= player.NumInfosets(); j++)  {
      gbtEfgInfoset infoset = player.GetInfoset(j);
      s << '"' << infoset.GetLabel() << "\" { ";
      for (k = 1; k <= NumNodes(i, j); k++)  {
	gbtEfgNode node = nodes[i]->NodeList(j)[k];
	s << '"' << node.GetLabel() << "\" ";
      }
      s << "} ";
    }
    s << "} ";
  }
  s << "} ";
}

gbtOutput& operator<<(gbtOutput&s, const gbtEfgBasis& e)
{
  e.Dump(s);
  return s;
}

template class gbtArray<gbtEfgNodeSet *>;
template class gbtArray<EFNodeArrays *>;
template class gbtDPVector<int>;
#ifndef __BCC55__
template gbtOutput & operator<< (gbtOutput&, const gbtDPVector<int>&);
#endif  // __BCC55__
