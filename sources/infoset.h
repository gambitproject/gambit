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

#include "gvector.h"

#include "efplayer.h"

class Node;

class Lexicon;

class Action   {
  friend class Efg;
  friend class BehavProfile<double>;
  friend class BehavProfile<gRational>;
  friend class Infoset;
  private:
    int number;
    gText name;
    Infoset *owner;

    Action(int br, const gText &n, Infoset *s)
      : number(br), name(n), owner(s)   { }
    ~Action()   { }

  public:
    const gText &GetName(void) const   { return name; }
    void SetName(const gText &s)       { name = s; }

    int GetNumber(void) const        { return number; }
    Infoset *BelongsTo(void) const   { return owner; }
    bool Precedes(const Node *) const;
};

class Infoset   {
  friend class Efg;
  friend class EFPlayer;
  friend class BehavProfile<double>;
  friend class BehavProfile<gRational>;
  friend class BehavProfile<gNumber>;
  friend class Lexicon;

  protected:
    Efg *E;
    int number;
    gText name;
    EFPlayer *player;
    gBlock<Action *> actions;
    gBlock<Node *> members;
    int flag, whichbranch;
    
    Infoset(Efg *e, int n, EFPlayer *p, int br);
    virtual ~Infoset();  

    virtual void PrintActions(gOutput &f) const;

  public:
    Efg *Game(void) const   { return E; }

    bool IsChanceInfoset(void) const   { return (player->IsChance()); }

    EFPlayer *GetPlayer(void) const    { return player; }

    void SetName(const gText &s)    { name = s; }
    const gText &GetName(void) const   { return name; }

    virtual Action *InsertAction(int where);
    virtual void RemoveAction(int which);

    void SetActionName(int i, const gText &s)
      { actions[i]->name = s; }
    const gText &GetActionName(int i) const  { return actions[i]->name; }

    const gArray<Action *> &Actions(void) const { return actions; }
    Action *GetAction(const int act) const { return actions[act]; }
    const gList<Action *> ListOfActions(void) const;
    int NumActions(void) const   { return actions.Length(); }

    const gArray<Node *> &Members(void) const   { return members; }
    const Node *GetMember(const int m) const { return members[m]; }
    const gList<const Node *> ListOfMembers(void) const;
    int NumMembers(void) const   { return members.Length(); }

    int GetNumber(void) const    { return number; }

    bool Precedes(const Node *) const;
};

class ChanceInfoset : public Infoset  {
  friend class Efg;
  friend class BehavProfile<double>;
  friend class BehavProfile<gRational>;
  friend class BehavProfile<gNumber>;
  friend class PureBehavProfile<double>;
  friend class PureBehavProfile<gRational>;
  friend class PureBehavProfile<gNumber>;

  private:
    gBlock<gNumber> probs;
    gBlock<double> double_probs;

    ChanceInfoset(Efg *E, int n, EFPlayer *p, int br);
    virtual ~ChanceInfoset()    { }

    void PrintActions(gOutput &f) const;

  public:
    Action *InsertAction(int where);
    void RemoveAction(int which);

    void SetActionProb(int i, const gNumber &value)  { probs[i] = value; }
    const gNumber &GetActionProb(int i) const   { return probs[i]; }
    const gArray<gNumber> &GetActionProbs(void) const  { return probs; }
};

#endif   //# INFOSET_H
