//
// FILE: node.h -- Declaration of Node class
//
// $Id$
//

#ifndef NODE_H
#define NODE_H

#include "math/rational.h"
#include "efg.h"

#ifdef __GNUG__
#pragma interface
#endif   // __GNUG__

class Lexicon;

template <class T> class BehavAssessment;

class Node    {
  friend class FullEfg;
  friend class BehavProfile<double>;
  friend class BehavProfile<gRational>;
  friend class BehavProfile<gNumber>;
  friend class BehavAssessment<double>;
  friend class BehavAssessment<gRational>;
  friend class BehavAssessment<gNumber>;
  friend class Lexicon;
  
  protected:
    bool mark;
    int number; // This is a unique identifier, not related to infoset memship
    FullEfg *E;
    gText name;
    Infoset *infoset;
    Node *parent;
    efgOutcome *outcome;
    gBlock<Node *> children;
    Node *whichbranch, *ptr, *gameroot;

    Node(FullEfg *e, Node *p);
    ~Node();

    void DeleteOutcome(efgOutcome *outc);

  public:
    FullEfg *Game(void) const   { return E; }

    int NumChildren(void) const    { return children.Length(); }
    int GetNumber(void) const { return number; }
    int NumberInInfoset(void) const;
    const gArray<Node *> &Children(void) const { return children; }
    Infoset *GetInfoset(void) const   { return infoset; }
    bool IsTerminal(void) const { return (children.Length() == 0); }
    bool IsNonterminal(void) const { return !IsTerminal(); }
    EFPlayer *GetPlayer(void) const
      { if (!infoset)   return 0;
	else  return infoset->GetPlayer(); }
    Action *GetAction(void) const; // returns null if root node
    Node *GetChild(int i) const    { return children[i]; }
    Node *GetChild(const Action& a) const    
      { return children[a.GetNumber()]; }
    Node *GetChild(const Action* a) const    
    { return children[a->GetNumber()]; }
    Node *GetParent(void) const    { return parent; }
    Node *GetSubgameRoot(void) const  { return gameroot; }
    Node *NextSibling(void) const;
    Node *PriorSibling(void) const;

    const gText &GetName(void) const   { return name; }
    void SetName(const gText &s)       { name = s; }

};

#endif   // NODE_H






