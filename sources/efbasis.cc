//
// FILE: efbasis.cc -- Implementation of EFBasis class
//
// $Id$ 
//

#include "efbasis.h"
#include "garray.imp"
#include "gvector.h"
#include "gpvector.h"
#include "gdpvect.imp"
#include "gmatrix.h"
#include "efgutils.h"
#include "lpsolve.h"

class EFNodeArrays   {
friend class EFNodeSet;
protected:
  gBlock<Node *> nodes;
  
public:
  EFNodeArrays ( const gArray <Node *> &a);
  EFNodeArrays ( const EFNodeArrays &a);
  virtual ~EFNodeArrays();
  EFNodeArrays &operator=( const EFNodeArrays &a);
  bool operator==( const EFNodeArrays &a) const;
};

//----------------------------------------------------
// EFNodeArray: Constructors, Destructor, operators
// ---------------------------------------------------

EFNodeArrays::EFNodeArrays(const gArray<Node *> &n)
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
bool operator==(const gArray<Node *> &a, const gArray<Node *> &b)
{
  if (a.mindex != b.mindex || a.maxdex != b.maxdex)   return false;
  for (int i = a.mindex; i <= a.maxdex; i++)
    if (a[i] != b[i])   return false;
  return true;
}
#endif

bool EFNodeArrays::operator==(const EFNodeArrays &a) const
{
  return (nodes == a.nodes);
}

class EFNodeSet{

protected:
  EFPlayer *efp;
  gArray < EFNodeArrays *> infosets;
public:
  
  //----------------------------------------
  // Constructors, Destructor, operators
  //----------------------------------------

//  EFNodeSet();
  EFNodeSet(const EFNodeSet &);
  EFNodeSet(EFPlayer &);
  virtual ~EFNodeSet();

  EFNodeSet &operator=(const EFNodeSet &);
  bool operator==(const EFNodeSet &s) const;

  //--------------------
  // Member Functions
  //--------------------

  // Append a Node to an infoset;
  void AddNode(int iset, Node *);

  // Insert a Node in a particular place in an infoset;
  void AddNode(int iset, Node *, int index);


  // Remove a Node at int i, returns the removed Node pointer
  Node *RemoveNode(int iset, int i);

  // Remove a Node from an infoset . 
  // Returns true if the Node was successfully removed, false otherwise.
  bool RemoveNode(int iset, Node *);

  // Get a garray of the Nodes in an Infoset
  const gArray<Node *> &NodeList(int iset) const
     { return infosets[iset]->nodes; }
  
  // Get a Node
  Node *GetNode(int iset, int index);

  // returns the index of the Node if it is in the NodeSet
  int Find(Node *) const;

  // Number of Nodes in a particular infoset
  int NumNodes(int iset) const;

  // return the EFPlayer of the EFNodeSet
  EFPlayer &GetPlayer(void) const;

  // checks for a valid EFNodeSet
  bool IsValid(void) const;

};

//--------------------------------------------------
// EFNodeSet: Constructors, Destructor, operators
//--------------------------------------------------

EFNodeSet::EFNodeSet(EFPlayer &p)
  : infosets(p.NumInfosets())
{
  efp = &p;
  for (int i = 1; i <= p.NumInfosets(); i++){
    infosets[i] = new EFNodeArrays(p.Infosets()[i]->Members());
  }
}

EFNodeSet::EFNodeSet( const EFNodeSet &s )
: infosets(s.infosets.Length())
{
  efp = s.efp;
  for (int i = 1; i <= s.infosets.Length(); i++){
    infosets[i] = new EFNodeArrays(*(s.infosets[i]));
  }
}

EFNodeSet::~EFNodeSet()
{ 
  for (int i = 1; i <= infosets.Length(); i++)
    delete infosets[i];
}

EFNodeSet &EFNodeSet::operator=(const EFNodeSet &s)
{
  if (this != &s && efp == s.efp) {
    for (int i = 1; i<= infosets.Length(); i++)  {
      delete infosets[i];
      infosets[i] = new EFNodeArrays(*(s.infosets[i]));
    }
  }    
  return *this;
}

bool EFNodeSet::operator==(const EFNodeSet &s) const
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
// EFNodeSet: Member functions 
//------------------------------------------

// Append a Node to a particular infoset;
void EFNodeSet::AddNode(int iset, Node *s) 
{ 
  infosets[iset]->nodes.Append(s); 
}

// Insert a Node  to a particular infoset at a particular place;
void EFNodeSet::AddNode(int iset, Node *s, int index) 
{ 
  infosets[iset]->nodes.Insert(s,index); 
}

// Remove a Node from infoset iset at int i, 
// returns the removed Infoset pointer
Node* EFNodeSet::RemoveNode(int iset, int i) 
{ 
  return (infosets[iset]->nodes.Remove(i)); 
}

// Removes a Node from infoset iset . Returns true if the 
//Node was successfully removed, false otherwise.
bool EFNodeSet::RemoveNode(int  iset, Node *s ) 
{ 
  int t = infosets[iset]->nodes.Find(s); 
  if (t>0) infosets[iset]->nodes.Remove(t); 
  return (t>0); 
} 

// Get a Node
Node *EFNodeSet::GetNode(int iset, int index)
{
  return (infosets[iset]->nodes)[index];
}

// Number of Nodes in a particular infoset
int EFNodeSet::NumNodes(int iset) const
{
  return (infosets[iset]->nodes.Length());
}

// Return the EFPlayer of this EFNodeSet
EFPlayer &EFNodeSet::GetPlayer(void) const
{
  return (*efp);
}

int EFNodeSet::Find(Node *n) const
{
  return (infosets[n->GetInfoset()->GetNumber()]->nodes.Find(n));
}

// checks for a valid EFNodeSet
bool EFNodeSet::IsValid(void) const
{
  if (infosets.Length() != efp->NumInfosets())   return false;

  for (int i = 1; i <= infosets.Length(); i++)
    if (infosets[i]->nodes.Length() == 0)   return false;

  return true;
}

//--------------------------------------------------
// EFBasis: Constructors, Destructors, Operators
//--------------------------------------------------

EFBasis::EFBasis(const Efg &E) : EFSupport(E), nodes(E.NumPlayers())
{
  for (int i = 1; i <= nodes.Length(); i++)
    nodes[i] = new EFNodeSet(*(E.Players()[i]));
}

EFBasis::EFBasis(const EFBasis &b)
  : EFSupport(b), nodes(b.nodes.Length())
{
  for (int i = 1; i <= nodes.Length(); i++)
    nodes[i] = new EFNodeSet(*(b.nodes[i]));
}

EFBasis::~EFBasis()
{
  for (int i = 1; i <= nodes.Length(); i++)
    delete nodes[i];
}

EFBasis &EFBasis::operator=(const EFBasis &b)
{
  EFSupport::operator=(b);
  return *this;
}

bool EFBasis::operator==(const EFBasis &b) const
{
  if( (*this).EFSupport::operator!=(b)) return false;

  if (nodes.Length() != b.nodes.Length()) return false;

  int i;
  for (i = 1; i <= nodes.Length() && *(nodes[i]) == *(b.nodes[i]); i++);
  return (i > nodes.Length());
}

bool EFBasis::operator!=(const EFBasis &b) const
{
  return !(*this == b);
}

//-----------------------------
// EFBasis: Member Functions 
//-----------------------------

int EFBasis::NumNodes(int pl, int iset) const
{
  return nodes[pl]->NumNodes(iset);
}

const gArray<Node *> &EFBasis::Nodes(int pl, int iset) const
{
  return nodes[pl]->NodeList(iset);
}

int EFBasis::Find(Node *n) const
{
  if (n->GetInfoset()->Game() != befg)   return 0;

  int pl = n->GetInfoset()->GetPlayer()->GetNumber();

  return nodes[pl]->Find(n);
}

bool EFBasis::IsValid(void) const
{
  if(!(*this).EFSupport::IsValid()) return false;
  if (nodes.Length() != befg->NumPlayers())   return false;
  for (int i = 1; i <= nodes.Length(); i++)
    if (!nodes[i]->IsValid())  return false;

  return true;
}

gPVector<int> EFBasis::NumNodes(void) const
{
  gArray<int> foo(befg->NumPlayers());
  int i;
  for (i = 1; i <= befg->NumPlayers(); i++)
    foo[i] = nodes[i]->GetPlayer().NumInfosets();

  gPVector<int> bar(foo);
  for (i = 1; i <= befg->NumPlayers(); i++)
    for (int j = 1; j <= nodes[i]->GetPlayer().NumInfosets(); j++)
      bar(i, j) = NumNodes(i,j);

  return bar;
}  

bool EFBasis::RemoveNode(Node *n)
{
  Infoset *infoset = n->GetInfoset();
  EFPlayer *player = infoset->GetPlayer();

  return nodes[player->GetNumber()]->RemoveNode(infoset->GetNumber(), n);
}

bool EFBasis::IsReachable(Node *n) const
{
  if(n == befg->RootNode()) return true;

  while (n != befg->RootNode()) {
    if(!n->GetParent()->GetInfoset()->IsChanceInfoset())
      if(!EFSupport::Find(LastAction(n))) return false;
    n = n->GetParent();
  }
return true;
}

void EFBasis::AddNode(Node *n)
{
  Infoset *infoset = n->GetInfoset();
  EFPlayer *player = infoset->GetPlayer();

  nodes[player->GetNumber()]->AddNode(infoset->GetNumber(), n);
}

bool EFBasis::IsConsistent()
{
  bigbasis = new EFBasis(*befg);
  nodeIndex = new gDPVector<int>(bigbasis->NumNodes());
  actIndex = new gDPVector<int>(bigbasis->NumActions());
  MakeIndices();
  // gout << "\nactIndex:  " << (*actIndex);
  // gout << "\nnodeIndex: " << (*nodeIndex);
 
  int num_vars = num_act_vars + num_node_vars;

  // gout << "\nnum_eqs: " << num_eqs;
  // gout << " num_ineqs: " << num_ineqs;

  // gout << "\nnum_act_vars: " << num_act_vars;
  // gout << " num_node_vars: " << num_node_vars;
  // gout << " num_vars: " << num_vars;

  A = new gMatrix<double>(1,num_eqs+num_ineqs,1,num_vars);
  b = new gVector<double>(1,num_eqs+num_ineqs);
  c = new gVector<double>(1,num_vars);
  (*A) = 0.0; (*b) = 0.0; (*c)= 0.0;

  MakeAb();
  for(int i=1;i<=num_act_vars;i++)
    (*c)[i]=-1.0;

  // gout << "\nA: \n" << (*A);
  // gout << "\nb: \n" << (*b);
  // gout << "\nc: \n" << (*c);

  LPSolve<double> lp((*A),(*b),(*c),num_eqs);

  // gout << "\noptimum: " << lp.OptimumVector();
  if(!lp.IsWellFormed()) gout << "\nLP not well formed";
  if(!lp.IsBounded()) gout << "\nLP not bounded";
  bool flag = lp.IsFeasible();
  for(int i=1;i<=num_act_vars;i++)
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

void EFBasis::MakeIndices()
{
  int i,j,k;
  int ind = 1;

  for(i=1;i<=befg->NumPlayers();i++)
    for(j=1;j<=(befg->NumInfosets())[i];j++) 
      for(k=1;k<=bigbasis->NumActions(i,j);k++)
	if(EFSupport::Find(bigbasis->Actions(i,j)[k]))
	  (*actIndex)(i,j,k)=0;
	else 
	  (*actIndex)(i,j,k) = ind++;
  num_act_vars=ind-1;
  for(i=1;i<=befg->NumPlayers();i++)
    for(j=1;j<=(befg->NumInfosets())[i];j++) 
      for(k=1;k<=bigbasis->NumNodes(i,j);k++) {
	if(IsReachable(bigbasis->Nodes(i,j)[k]))
	  (*nodeIndex)(i,j,k)=0;
	else 
	  (*nodeIndex)(i,j,k) = ind++;
      }
  num_node_vars=ind-num_act_vars-1;
  MakeRowIndices();
}

void EFBasis::MakeRowIndices()
{
  int i,j,k,kk;

  num_eqs = 0;
  num_ineqs = 0;
  for(i=1;i<=befg->NumPlayers();i++)
    for(j=1;j<=(befg->NumInfosets())[i];j++) {
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

void EFBasis::MakeAb()
{
  int i,j,k,kk;
  int eq = num_ineqs+1;
  int ineq = 1;

  for(i=1;i<=befg->NumPlayers();i++)
    for(j=1;j<=(befg->NumInfosets())[i];j++) {
      for(k=1;k<=bigbasis->NumActions(i,j);k++)
	if((*actIndex)(i,j,k))
	  AddEquation1(ineq++,bigbasis->Actions(i,j)[k]);
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

int EFBasis::Col(Action *a) const
{
  Infoset *iset = a->BelongsTo();
  return (*actIndex)(iset->GetPlayer()->GetNumber(),iset->GetNumber(),
		  (*bigbasis).EFSupport::Find(a));
}

int EFBasis::Col(Node *n) const
{
  Infoset *iset = n->GetInfoset();
  return (*nodeIndex)(iset->GetPlayer()->GetNumber(),iset->GetNumber(),
		  (*bigbasis).Find(n));
}

void EFBasis::AddEquation1(int row,Action *a) const
{
  if(Col(a))
    (*A)(row,Col(a)) = -1.0;
  (*b)[row] = -1.0;
}

void EFBasis::AddEquation2(int row,Node *n) const
{
  if(Col(n))
    (*A)(row,Col(n)) = 1.0;
  if(n!=befg->RootNode()) {
    Action *act = LastAction(n);
    if(Col(act))
      (*A)(row,Col(act)) = -1.0;
    while(n->GetParent() != befg->RootNode()) {
      n = n->GetParent();
      act = LastAction(n);
      if(Col(act))
	(*A)(row,Col(act)) = -1.0;
    }
  }
}

void EFBasis::AddEquation3(int row,Node *n1, Node *n2) const
{
  if(Col(n1))
    (*A)(row,Col(n1)) = 1.0;
  if(Col(n2))
    (*A)(row,Col(n2)) = -1.0;
}

void EFBasis::AddEquation4(int row,Node *n1, Node *n2) const
{
  if(Col(n1))
    (*A)(row,Col(n1)) = 1.0;
  if(Col(n2))
    (*A)(row,Col(n2)) = -1.0;
  (*b)[row] = -1.0;
}

void EFBasis::GetConsistencySolution(const gVector<double> &x)
{
  gDPVector<int> nodes(bigbasis->NumNodes());
  gDPVector<int> acts(bigbasis->NumActions());
  nodes = 0;
  acts = 0;
  int i,j,k;
  for(i=1;i<=befg->NumPlayers();i++)
    for(j=1;j<=(befg->NumInfosets())[i];j++) {
      for(k=1;k<=bigbasis->NumActions(i,j);k++)
	if((*actIndex)(i,j,k))
	  acts(i,j,k) = (int)x[(*actIndex)(i,j,k)];
      for(k=1;k<=bigbasis->NumNodes(i,j);k++) 
	if((*nodeIndex)(i,j,k))
	  nodes(i,j,k) = (int)x[(*nodeIndex)(i,j,k)];
    }
  gout << "\nacts: " << acts;
  gout << "\nnodes: " << nodes;
}

void EFBasis::Dump(gOutput& s) const
{
  int numplayers;
  int i;
  int j;
  int k;

  s << "\nActions: ";
  (*this).EFSupport::Dump(s);
  s << "\nNodes:   ";
  s << "{ ";
  numplayers = befg->NumPlayers();
  for (i = 1; i <= numplayers; i++)  {
    EFPlayer& player = nodes[i]->GetPlayer();
    s << '"' << player.GetName() << "\" { ";
    for (j = 1; j <= player.NumInfosets(); j++)  {
      Infoset* infoset = player.Infosets()[j];
      s << '"' << infoset->GetName() << "\" { ";
      for (k = 1; k <= NumNodes(i, j); k++)  {
	Node* Node = nodes[i]->NodeList(j)[k];
	s << '"' << Node->GetName() << "\" ";
      }
      s << "} ";
    }
    s << "} ";
  }
  s << "} ";
}

gOutput& operator<<(gOutput&s, const EFBasis& e)
{
  e.Dump(s);
  return s;
}

template class gArray<EFNodeSet *>;
template class gArray<EFNodeArrays *>;
template class gDPVector<int>;
template gOutput & operator<< (gOutput&, const gDPVector<int>&);
