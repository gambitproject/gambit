//#
//# FILE: infoset.h -- Declaration of Infoset class
//#
//# $Id$
//#

#ifndef INFOSET_H
#define INFOSET_H

#include "rational.h"
#include "gvector.h"

#include "player.h"

class Node;

template <class T> class ChanceInfoset;

class Action   {
  friend class BaseExtForm;
  friend class ExtForm<double>;
  friend class ExtForm<gRational>;
  friend class Infoset;
  friend class ChanceInfoset<double>;
  friend class ChanceInfoset<gRational>;
  private:
    gString name;

    Action(const gString &s) : name(s)   { }
    ~Action()   { }

  public:
    const gString &GetName(void) const   { return name; }
    void SetName(const gString &s)       { name = s; }
};

class Infoset   {
  friend class BaseExtForm;
  friend class Player;
  friend class ExtForm<double>;
  friend class ExtForm<gRational>;

  protected:
    bool valid;
    BaseExtForm *E;
    int number;
    gString name;
    Player *player;
    gBlock<Action *> actions;
    gBlock<Node *> members;
    
    Infoset(BaseExtForm *e, int n, Player *p, int br)
      : valid(true), E(e), number(n), player(p), actions(br) 
      { while (br)  { actions[br] = new Action(ToString(br)); br--; } }
    virtual ~Infoset()  
      { for (int i = 1; i <= actions.Length(); i++)  delete actions[i]; }

    virtual void PrintActions(gOutput &f) const
      { f << "{ ";
	for (int i = 1; i <= actions.Length(); i++)
	  f << '"' << actions[i]->name << "\" ";
	f << "}";
      }

  public:
    bool IsValid(void) const             { return valid; }
    BaseExtForm *BelongsTo(void) const   { return E; }

    bool IsChanceInfoset(void) const   { return (player->IsChance()); }

    Player *GetPlayer(void) const    { return player; }

    void SetName(const gString &s)    { name = s; }
    const gString &GetName(void) const   { return name; }

    virtual void InsertAction(int where)
      { actions.Insert(new Action(""), where); }
    virtual void RemoveAction(int which)
      { delete actions.Remove(which); }

    void SetActionName(int i, const gString &s)
      { actions[i]->name = s; }
    const gString &GetActionName(int i) const  { return actions[i]->name; }

    Action *GetAction(const gString &s) const
      { for (int i = 1; i <= actions.Length(); i++)
	  if (actions[i]->name == s)   return actions[i];
	return 0;
      }

    const gArray<Action *> &GetActionList(void) const  { return actions; }  
    int NumActions(void) const   { return actions.Length(); }

    const gArray<Node *> &GetMemberList(void) const   { return members; }
    int NumMembers(void) const   { return members.Length(); }

// This function exists only to facilitate the current version of efg2nfg
// and is not guaranteed to exist in the future, so do not depend on it!
    int GetNumber(void) const    { return number; }
};

template <class T> class ChanceInfoset : public Infoset  {
  friend class BaseExtForm;
  friend class ExtForm<double>;
  friend class ExtForm<gRational>;

  private:
    gBlock<T> probs;

    ChanceInfoset(BaseExtForm *E, int n, Player *p, int br)
      : Infoset(E, n, p, br), probs(br)
      { probs[1] = (T) 1.0;
	for (int i = 2; i <= br; probs[i++] = (T) 0.0);
      }
    ~ChanceInfoset()    { }

    void PrintActions(gOutput &f) const
      { f << "{ ";
	for (int i = 1; i <= actions.Length(); i++)
	  f << '"' << actions[i]->GetName() << "\" " << probs[i] << ' ';
	f << "}";
      }
  public:
    void InsertAction(int where)
      { actions.Insert(new Action(""), where);
	probs.Insert((T) 0, where);
      }

    void RemoveAction(int which)
      { delete actions.Remove(which);
	probs.Remove(which);
      }

    void SetActionProb(int i, const T &value)  { probs[i] = value; }
    const T &GetActionProb(int i) const   { return probs[i]; }
    const gArray<T> &GetActionProbs(void) const  { return probs; }
};



#endif   //# INFOSET_H

