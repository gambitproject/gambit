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

template <class T> class Lexicon;

class Node    {
  friend class BaseEfg;
  friend class Efg<double>;
  friend class Efg<gRational>;
  friend class BehavProfile<double>;
  friend class BehavProfile<gRational>;
  friend class Lexicon<double>;
  friend class Lexicon<gRational>;
  
  protected:
    bool valid, mark;
    BaseEfg *E;
    gString name;
    Infoset *infoset;
    Node *parent;
    Outcome *outcome;
    gBlock<Node *> children;
    Node *whichbranch, *ptr, *gameroot;

    Node(BaseEfg *e, Node *p);
    virtual ~Node();

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
    Node *NextSibling(void) const;
    Node *PriorSibling(void) const;

    const gString &GetName(void) const   { return name; }
    void SetName(const gString &s)       { name = s; }

    Outcome *GetOutcome(void) const   { return outcome; }
    void SetOutcome(Outcome *outc)    { outcome = outc; }
    void DeleteOutcome(Outcome *outc);
};

#endif   // NODE_H






