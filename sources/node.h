//#
//# FILE: node.h -- Declaration of Node class
//#
//# $Id$
//#

#ifndef NODE_H
#define NODE_H

#include "rational.h"
#include "gblock.h"

class Node    {
  friend class BaseExtForm;
  friend class ExtForm<double>;
  friend class ExtForm<gRational>;
  
  protected:
    gString name;
    Infoset *infoset;
    Node *parent;
    Outcome *outcome;
    gBlock<Node *> children;

    Node(Node *p) : parent(p), infoset(0), outcome(0)   { }
    virtual ~Node()
      { for (int i = children.Length(); i; delete children[i--]); }

    virtual void Resize(int) = 0;

  public:
    int NumChildren(void) const    { return children.Length(); }
    Infoset *GetInfoset(void) const   { return infoset; }

    Node *GetChild(int i) const    { return children[i]; }
    Node *GetParent(void) const    { return parent; }
    Node *NextSibling(void) const  
      { if (!parent)   return 0;
	if (parent->children.Find(this) == parent->children.Length())
	  return 0;
	else
	  return parent->children[parent->children.Find(this) + 1];
      }
    const gString &GetName(void) const   { return name; }
    void SetName(const gString &s)       { name = s; }

    Outcome *GetOutcome(void) const   { return outcome; }
    void SetOutcome(Outcome *outc)    { outcome = outc; }
};

#endif   // NODE_H






