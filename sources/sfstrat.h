//
// FILE: sfstrat.h -- Declaration of Sequence Form strategy data types
//
// $Id$
//

#ifndef SFSTRAT_H
#define SFSTRAT_H

#include "gtext.h"
#include "gblock.h"
#include "gstream.h"
#include "glist.h"
#include "efg.h"

/*
class EFPlayer;
class Efg;
class Sfg;
class Action;
class Infoset;
*/

struct Sequence {
friend class Sfg;
friend class Efg;
friend class SFSequenceSet;
private:
  int number;
  gText name;
  const EFPlayer *player;
  Action *action;
  const Sequence *parent;
  
  Sequence(const EFPlayer *pl, Action *a, const Sequence *p, int n) 
    : number(n), player(pl), action(a), parent(p) { }
  ~Sequence() { }
public:
  const gText &GetName(void) const   { return name; }
  void SetName(const gText &s)       { name = s; }
  
  gList<const Action *> History(void) const;
  int GetNumber(void) const        { return number; }
  Action *GetAction(void) const  {return action; }
  const Infoset *GetInfoset(void) const   { if(action) return action->BelongsTo();return 0; }
  const EFPlayer *Player(void) const  { return player; }
  const Sequence *Parent(void) const   { return parent; }
  void Dump(gOutput &) const;
};

class SFSequenceSet {
protected:
  const EFPlayer *efp;
  gBlock <Sequence *> sequences;
  
public:
  SFSequenceSet(const SFSequenceSet &s); 
  SFSequenceSet(const EFPlayer *p);
  
  SFSequenceSet &operator=(const SFSequenceSet &s); 
  bool operator==(const SFSequenceSet &s);

  virtual ~SFSequenceSet();

  // Append a sequence to the SFSequenceSet
  void AddSequence(Sequence *s);

  // Removes a sequence pointer. Returns true if the sequence was successfully
  // removed, false otherwise.
  bool RemoveSequence( Sequence *s ); 
  Sequence * Find(int j);

  // Number of sequences in the SFSequenceSet
  int NumSequences(void) const;

  //  return the entire sequence set in a const gArray
  const gBlock<Sequence *> &GetSFSequenceSet(void) const;
};


class SFSupport {
protected:
  const Sfg *bsfg;
  gArray <SFSequenceSet *> sups;
  
public:
  SFSupport(const Sfg &);
  SFSupport(const SFSupport &s); 
  virtual ~SFSupport();
  SFSupport &operator=(const SFSupport &s);

  bool operator==(const SFSupport &s) const;
  bool operator!=(const SFSupport &s) const;

  const Sfg &Game(void) const   { return *bsfg; }
  
  const gBlock<Sequence *> &Sequences(int pl) const;

  int NumSequences(int pl) const;
  const gArray<int> NumSequences(void) const;
  int TotalNumSequences(void) const;

  void AddSequence(Sequence *);
  bool RemoveSequence(Sequence *);
  
  bool IsSubset(const SFSupport &s) const;

  // returns the index of the sequence in the support if it exists,
  // otherwise returns zero
  int Find(Sequence *) const; 

  void Dump(gOutput &) const;
};

class SequenceProfile   {
  friend class Sfg;
private:
  long index;
  gArray<Sequence *> profile;
  
public:
  SequenceProfile(const Sfg &);
  SequenceProfile(const SequenceProfile &p);

  ~SequenceProfile();
  
  SequenceProfile &operator=(const SequenceProfile &);
  
  bool IsValid(void) const; 
  
  long GetIndex(void) const;
  
  Sequence *const operator[](int p) const;
  Sequence *const Get(int p) const;
  void Set(int p, Sequence  *const s);
};


gOutput &operator<<(gOutput &f, const Sequence &);
gOutput &operator<<(gOutput &f, const SFSupport &);

#endif    // SFSTRAT_H


