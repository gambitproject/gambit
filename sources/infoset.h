//
// FILE: infoset.h -- Declaration of Infoset class
//
// $Id$
//

#ifndef INFOSET_H
#define INFOSET_H

#include "basic.h"
#include "gstring.h"
#include "gset.h"
#include "gvector.h"
#include "gnumber.h"

class Action   {
  private:
    gString name;
    gNumber prob;

  public:
    Action(void) : prob(-1.0)   { }
    Action(const Action &b) : name(b.name), prob(b.prob)  { }
    ~Action()   { }

    Action &operator=(const Action &b)
      { name = b.name;  prob = b.prob;  return *this; }

    gString GetActionName(void) const   { return name; }
    void SetActionName(const gString &s)    { name = s; }
    
    gNumber GetActionProb(void) const   { return prob; }
    void SetActionProb(gNumber d)    { prob = d; }
};


class Infoset   {
  private:
    gString name;
    gSet<Action *> actions;

  public:
	// CONSTRUCTORS AND DESTRUCTOR
	// create a new infoset with acts actions
    Infoset(int acts = 0);
	// copy constructor
    Infoset(const Infoset &);
	// clean up after an infoset
    ~Infoset();

	// OPERATOR OVERLOADING
	// assignment operator
    Infoset &operator=(const Infoset &);
    
	// DATA ACCESS AND MANIPULATION
	// return the infoset name
    gString GetInfosetName(void) const    { return name; }
	// set the infoset name
    void SetInfosetName(const gString &s)   { name = s; }

	// return the number of actions at the infoset
    int NumActions(void) const   { return actions.Length(); }

	// set an action's name
    void SetActionName(int act, const gString &s)
      { actions[act]->SetActionName(s); }

	// return an action's name
    gString GetActionName(int act) const
      { return actions[act]->GetActionName(); }

	// get an action's probability
    gNumber GetActionProb(int act) const
      { return actions[act]->GetActionProb(); }

	// get the vector of probabilities for the actions
    gVector<gNumber> GetActionProbs(void) const;

	// set the probabilities of the actions
    void SetActionProbs(const gVector<gNumber> &probs);

	// remove an action from the infoset
    void RemoveAction(int act)
      { delete actions.Remove(act); }

	// add an action to the infoset
    void InsertAction(int act)
      { actions.Insert(new Action, act); }

	// append an action to the infoset
    void AppendAction(void)
      { actions.Append(new Action); }
};


#endif   // INFOSET_H

