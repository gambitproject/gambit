//
// FILE: infoset.h -- Declaration of Infoset class
//
// $Id$
//

#ifndef INFOSET_H
#define INFOSET_H

#ifdef __GNUG__
#pragma interface
#endif   // __GNUG__

#include "rational.h"
#include "gvector.h"

#include "efplayer.h"

class Node;

template <class T> class ChanceInfoset;
template <class T> class Lexicon;

class Action   {
  friend class BaseEfg;
  friend class Efg<double>;
  friend class Efg<gRational>;
  friend class BehavProfile<double>;
  friend class BehavProfile<gRational>;
  friend class Infoset;
  friend class ChanceInfoset<double>;
  friend class ChanceInfoset<gRational>;
  private:
    int number;
    gString name;
    Infoset *owner;

    Action(int br, const gString &n, Infoset *s)
      : number(br), name(n), owner(s)   { }
    ~Action()   { }

  public:
    const gString &GetName(void) const   { return name; }
    void SetName(const gString &s)       { name = s; }

    int GetNumber(void) const        { return number; }
    Infoset *BelongsTo(void) const   { return owner; }
};

class Infoset   {
  friend class BaseEfg;
  friend class EFPlayer;
  friend class Efg<double>;
  friend class Efg<gRational>;
  friend class BehavProfile<double>;
  friend class BehavProfile<gRational>;
  friend class Lexicon<double>;
  friend class Lexicon<gRational>;

  protected:
    BaseEfg *E;
    int number;
    gString name;
    EFPlayer *player;
    gBlock<Action *> actions;
    gBlock<Node *> members;
    int flag, whichbranch;
    
    Infoset(BaseEfg *e, int n, EFPlayer *p, int br);
    virtual ~Infoset();  

    virtual void PrintActions(gOutput &f) const;

  public:
    BaseEfg *BelongsTo(void) const   { return E; }

    bool IsChanceInfoset(void) const   { return (player->IsChance()); }

    EFPlayer *GetPlayer(void) const    { return player; }

    void SetName(const gString &s)    { name = s; }
    const gString &GetName(void) const   { return name; }

    virtual Action *InsertAction(int where);
    virtual void RemoveAction(int which);

    void SetActionName(int i, const gString &s)
      { actions[i]->name = s; }
    const gString &GetActionName(int i) const  { return actions[i]->name; }

    const gArray<Action *> &Actions(void) const  { return actions; }  
    int NumActions(void) const   { return actions.Length(); }

    const gArray<Node *> &Members(void) const   { return members; }
    int NumMembers(void) const   { return members.Length(); }

    int GetNumber(void) const    { return number; }
};

template <class T> class ChanceInfoset : public Infoset  {
  friend class BaseEfg;
  friend class Efg<double>;
  friend class Efg<gRational>;
  friend class BehavProfile<double>;
  friend class BehavProfile<gRational>;

  private:
    gBlock<T> probs;

    ChanceInfoset(BaseEfg *E, int n, EFPlayer *p, int br);
    virtual ~ChanceInfoset()    { }

    void PrintActions(gOutput &f) const;

  public:
    Action *InsertAction(int where);
    void RemoveAction(int which);

    void SetActionProb(int i, const T &value)  { probs[i] = value; }
    const T &GetActionProb(int i) const   { return probs[i]; }
    const gArray<T> &GetActionProbs(void) const  { return probs; }
};



#endif   //# INFOSET_H

