//#
//# FILE: infoset.h -- Declaration of Infoset class
//#
//# $Id$
//#

#ifndef INFOSET_H
#define INFOSET_H

#include "rational.h"
#include "gvector.h"

#include "efplayer.h"

class Node;

template <class T> class ChanceInfoset;

class Action   {
  friend class BaseEfg;
  friend class Efg<double>;
  friend class Efg<gRational>;
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
  friend class BaseEfg;
  friend class EFPlayer;
  friend class Efg<double>;
  friend class Efg<gRational>;

  protected:
    bool valid;
    BaseEfg *E;
    int number;
    gString name;
    EFPlayer *player;
    gBlock<Action *> actions;
    gBlock<Node *> members;
    
    Infoset(BaseEfg *e, int n, EFPlayer *p, int br)
      : valid(true), E(e), number(n), player(p), actions(br), flag(0) 
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
    int flag, whichbranch;

    bool IsValid(void) const             { return valid; }
    BaseEfg *BelongsTo(void) const   { return E; }

    bool IsChanceInfoset(void) const   { return (player->IsChance()); }

    EFPlayer *GetPlayer(void) const    { return player; }

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
  friend class BaseEfg;
  friend class Efg<double>;
  friend class Efg<gRational>;

  private:
    gBlock<T> probs;

    ChanceInfoset(BaseEfg *E, int n, EFPlayer *p, int br)
      : Infoset(E, n, p, br), probs(br)
      { for (int i = 1; i <= br; probs[i++] = (T) 1 / (T) br); }
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

