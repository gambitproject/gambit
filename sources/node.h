//
// FILE: node.h -- Declaration of Node class
//
// $Id$
//

#ifndef NODE_H
#define NODE_H

#include "rational.h"
#include "gblock.h"
#include "efg.h"

#ifdef __GNUG__
#pragma interface
#endif   // __GNUG__

class Lexicon;

class Node    {
  friend class Efg;
  friend class BehavProfile<double>;
  friend class BehavProfile<gRational>;
  friend class BehavProfile<gNumber>;
  friend class Lexicon;
  
  protected:
    bool mark;
    int number; // This is a unique identifier, not related to infoset memship
    Efg *E;
    gText name;
    Infoset *infoset;
    Node *parent;
    EFOutcome *outcome;
    gBlock<Node *> children;
    Node *whichbranch, *ptr, *gameroot;

    Node(Efg *e, Node *p);
    ~Node();

  public:
    Efg *Game(void) const   { return E; }

    int NumChildren(void) const    { return children.Length(); }
    int NumberInInfoset(void) const;
    const gArray<Node *> &Children(void) const { return children; }
    Infoset *GetInfoset(void) const   { return infoset; }
    bool IsTerminal(void) const { return (children.Length() == 0); }
    bool IsNonterminal(void) const { return !IsTerminal(); }
    EFPlayer *GetPlayer(void) const
      { if (!infoset)   return 0;
	else  return infoset->GetPlayer(); }
    const Action *GetAction() const; // Error if called on Game()->RootNode()
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

    EFOutcome *GetOutcome(void) const   { return outcome; }
    void SetOutcome(EFOutcome *outc)    { outcome = outc; }
    void DeleteOutcome(EFOutcome *outc);
};

#endif   // NODE_H






