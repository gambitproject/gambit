//#
//# FILE: infoset.h -- Declaration of Infoset class
//#
//# $Id$
//#

#ifndef INFOSET_H
#define INFOSET_H

#include "basic.h"
#include "gstring.h"
#include "gblock.h"
#include "gvector.h"
#include "gnumber.h"
#include "noderep.h"

//
// <category lib=glib sect="Extensive Form">
//
// This class is used to name an action, and assign it a probability.
// It contains no information about the context of the action.
//
class Action   {
  private:
    gString name;
    gNumber prob;

  public:
	//
	// Creates an empty action, with probability -1
	//
    Action(void) : prob(-1.0)   { }
	//
	// Copy constructor
	//
    Action(const Action &b) : name(b.name), prob(b.prob)  { }
	//
	// Deallocate an action
	//
    ~Action()   { }

	//
	// Assignment operator
	//
    Action &operator=(const Action &b)
      { name = b.name;  prob = b.prob;  return *this; }

	// 
	// Public functions to access private data
	//+grp
    gString GetActionName(void) const   { return name; }
    void SetActionName(const gString &s)    { name = s; }
    
    gNumber GetActionProb(void) const   { return prob; }
    void SetActionProb(gNumber d)    { prob = d; }
	//-grp
};

//
// <category lib=glib sect="Extensive Form">
//
// This class creates an information set (infoset in most of my
// documentation).  The gBlock of Actions contains all the possible
// choices in the information set, and nodes contains all the nodes
// encompassed by the information set (ie all nodes in the infoset).
//
class Infoset   {
  private:
    gString         name;
    gBlock<Action *>  actions;
    gBlock<NodeRep *> nodes;

  public:
	//# CONSTRUCTORS AND DESTRUCTOR
	//
	// create a new infoset with acts actions
	//
    Infoset(int acts = 0);
	//
	// clean up after an infoset
	//
    ~Infoset();

	//# OPERATOR OVERLOADING
	//
	// retrieves the nod'th node.  Used to make statements less verbose.
	//
    NodeRep *operator()(int nod)
      { return GetMember(nod); }
    
	//# DATA ACCESS AND MANIPULATION
	//
        // return the infoset name
	//
    gString GetInfosetName(void) const    { return name; }
	//
        // set the infoset name
	//
    void SetInfosetName(const gString &s)   { name = s; }

	//# ACTION FUNCTIONS
	//
	// return the number of actions at the infoset
	//
    int NumActions(void) const   { return actions.Length(); }
	//
	// set an action's name
	//
    void SetActionName(int act, const gString &s)
      { actions[act]->SetActionName(s); }
	//
	// return an action's name
	//
    gString GetActionName(int act) const
      { return actions[act]->GetActionName(); }
	//
	// get an action's probability
	//
    gNumber GetActionProb(int act) const
      { return actions[act]->GetActionProb(); }
	//
	// get the vector of probabilities for the actions
	//
    gTuple<gNumber> GetActionProbs(void) const;
	//
	// set the probabilities of the actions
	//
    void SetActionProbs(const gTuple<gNumber> &probs);
	//
	// remove an action from the infoset
	//
    void RemoveAction(int act)
      { delete actions.Remove(act); }
	//
	// add an action to the infoset
	//
    void InsertAction(int act)
      { actions.Insert(new Action, act); }
	//
	// append an action to the infoset
	//
    void AppendAction(void)
      { actions.Append(new Action); }

	//# NODE FUNCTIONS
	//
	// return the number of nodes in the infoset
	//
    int NumNodes(void) const  { return nodes.Length(); }
	//
	// add a new node to the infoset
	//
    int CreateMember()
      { return nodes.Append(new NodeRep); }
	//
	// add a node knode to the infoset
	//
    int CreateMember(NodeRep *knode)
      { return nodes.Append(knode); }
	//
	// remove a node from the infoset
	//
    NodeRep *RemoveMember(int nod)
      { return nodes.Remove(nod); }
	//
	// read a node from the infoset
	//
    NodeRep *GetMember(int nod)
      { return nodes[nod]; }
};


#endif   //# INFOSET_H

