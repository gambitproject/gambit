//#
//# FILE: infoset.h -- Declaration of Infoset class
//#
//# $Id$
//#

#ifndef INFOSET_H
#define INFOSET_H

#include "glist.h"
#include "rational.h"
#include "gvector.h"

#include "player.h"

#include "gconvert.h"

class Node;

class Action   {
  friend class BaseExtForm;
  friend class Infoset;

  private:
    gString name;

    Action(const gString &s) : name(s)   { }
    ~Action()   { }

  public:
    const gString &GetName(void) const   { return name; }
};

class Infoset   {
  friend class BaseExtForm;
  friend class Player;
  friend class ExtForm<double>;
  friend class ExtForm<gRational>;

  protected:
    int number;
    gString name;
    Player *player;
    gBlock<Action *> actions;
    gList<Node *> members;
    
    Infoset(int n, Player *p, int br) : number(n), player(p), actions(br) 
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
    bool IsChanceInfoset(void) const   { return (player->IsChance()); }

    Player *GetPlayer(void) const    { return player; }

    void SetName(const gString &s)    { name = s; }
    const gString &GetName(void) const   { return name; }

    void SetActionName(int i, const gString &s)
      { actions[i]->name = s; }
    const gString &GetActionName(int i) const  { return actions[i]->name; }

    Action *GetAction(const gString &s) const
      { for (int i = 1; i <= actions.Length(); i++)
	  if (actions[i]->name == s)   return actions[i];
	return 0;
      }

    int NumActions(void) const   { return actions.Length(); }

// This function exists only to facilitate the current version of efg2nfg
// and is not guaranteed to exist in the future, so do not depend on it!
    int GetNumber(void) const    { return number; }
};

template <class T> class ChanceInfoset : public Infoset  {
  friend class BaseExtForm;
  friend class ExtForm<double>;
  friend class ExtForm<gRational>;

  private:
    gVector<T> probs;

    ChanceInfoset(int n, Player *p, int br) : Infoset(n, p, br), probs(br)
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
    void SetActionProb(int i, const T &value)  { probs[i] = value; }
    const T &GetActionProb(int i) const   { return probs[i]; }
};



#endif   //# INFOSET_H

