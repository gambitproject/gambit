//#
//# FILE: node.h -- Declaration of Node class
//#
//# $Id$
//#

#ifndef NODE_H
#define NODE_H

#include "rational.h"
#include "gblock.h"
#include "efg.h"

class Node    {
  friend class BaseEfg;
  friend class Efg<double>;
  friend class Efg<gRational>;
  friend class BehavProfile<double>;
  friend class BehavProfile<gRational>;
  friend class Lexicon;
  
  protected:
    bool valid, mark;
    BaseEfg *E;
    gString name;
    Infoset *infoset;
    Node *parent;
    Outcome *outcome;
    gBlock<Node *> children;
    Node *whichbranch, *ptr, *gameroot;

    Node(BaseEfg *e, Node *p)
      : valid(true), mark(false), E(e), infoset(0), parent(p), outcome(0), gameroot(0)   { }
    virtual ~Node()
      { for (int i = children.Length(); i; delete children[i--]); }

    virtual void Resize(int) = 0;

  public:

    bool IsValid(void) const     { return valid; }
    BaseEfg *BelongsTo(void) const   { return E; }

    int NumChildren(void) const    { return children.Length(); }
    Infoset *GetInfoset(void) const   { return infoset; }
    EFPlayer *GetPlayer(void) const
      { if (!infoset)   return 0;
	else  return infoset->GetPlayer(); }

    Node *GetChild(int i) const    { return children[i]; }
    Node *GetParent(void) const    { return parent; }
    Node *GetSubgameRoot(void) const  { return gameroot; }
    Node *NextSibling(void) const  
      { if (!parent)   return 0;
	if (parent->children.Find((Node * const) this) == parent->children.Length())
	  return 0;
	else
	  return parent->children[parent->children.Find((Node * const)this) + 1];
      }
    Node *PriorSibling(void) const
      { if (!parent)   return 0;
	if (parent->children.Find((Node * const)this) == 1)
	  return 0;
	else
	  return parent->children[parent->children.Find((Node * const)this) - 1];
      }
    const gString &GetName(void) const   { return name; }
    void SetName(const gString &s)       { name = s; }

    Outcome *GetOutcome(void) const   { return outcome; }
    void SetOutcome(Outcome *outc)    { outcome = outc; }
    void DeleteOutcome(Outcome *outc)
      { if (outc == outcome)   outcome = 0;
	for (int i = 1; i <= children.Length(); i++)
	  children[i]->DeleteOutcome(outc);
      }
};

#endif   // NODE_H






