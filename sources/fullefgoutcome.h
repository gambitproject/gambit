//
// FILE: fullefgoutcome.h: Definition of Outcome representation
//

#ifndef FULLEFGOUTCOME_H
#define FULLEFGOUTCOME_H

class FullEfg::Outcome   {
friend class FullEfg;
friend class BehavProfile<double>;
friend class BehavProfile<gRational>;
friend class BehavProfile<gNumber>;
protected:
  bool m_deleted; 
  int m_number;
  gText m_name;
  gBlock<gNumber> m_payoffs;
  gBlock<gNumber> m_doublePayoffs;

  Outcome(Efg *p_efg, int p_number)
    : m_deleted(false), m_number(p_number), 
      m_payoffs(p_efg->NumPlayers()), m_doublePayoffs(p_efg->NumPlayers())
    { }
  ~Outcome()  { }
};

#endif  // FULLEFGOUTCOME_H
