//
// FILE: mixedsol.h -- Mixed strategy solution classes
//
// $Id$
//

#ifndef MIXEDSOL_H
#define MIXEDSOL_H

#include "base/base.h"
#include "math/gnumber.h"
#include "mixed.h"
#include "nfplayer.h"
#include "nfg.h"
#include "algutils.h"  // needed for gFact

typedef enum {
  algorithmNfg_USER,
  algorithmNfg_ENUMPURE, algorithmNfg_ENUMMIXED,
  algorithmNfg_LCP, algorithmNfg_LP, algorithmNfg_LIAP,
  algorithmNfg_SIMPDIV, algorithmNfg_POLENUM,
  algorithmNfg_QRE, algorithmNfg_QREALL,
  algorithmNfg_POLISH_NASH, algorithmNfg_YAMAMOTO
} NfgAlgType;

gText ToText(NfgAlgType);
gOutput &operator<<(gOutput &, NfgAlgType);

class MixedSolution   {
protected:
  MixedProfile<gNumber> m_profile;
  gPrecision m_precision;
  mutable NFSupport m_support;
  mutable NfgAlgType m_creator;
  mutable gFact<gTriState> m_Nash, m_Perfect, m_Proper;
  mutable gFact<gNumber> m_liapValue;
  mutable gNumber m_epsilon, m_qreLambda, m_qreValue;
  gArray<gNumber> m_payoff;
  gText m_name;
  mutable long m_revision;
  
  gTriState GetNash(void) const;
  gTriState GetPerfect(void) const;
  gTriState GetProper(void) const;

  void LevelPrecision(void);

public:
  // CONSTRUCTORS, DESTRUCTOR, AND CONSTRUCTIVE OPERATORS
  MixedSolution(const MixedProfile<double> &, NfgAlgType = algorithmNfg_USER);
  MixedSolution(const MixedProfile<gRational> &,
		NfgAlgType = algorithmNfg_USER);
  MixedSolution(const MixedProfile<gNumber> &, NfgAlgType = algorithmNfg_USER);
  MixedSolution(const MixedSolution &);
  virtual ~MixedSolution();

  MixedSolution &operator=(const MixedSolution &);

  // OPERATOR OVERLOADING
  bool Equals(const MixedProfile<double> &s) const;
  bool operator==(const MixedSolution &) const;
  bool operator!=(const MixedSolution &S) const { return !(*this == S); } 

  void Set(Strategy *, const gNumber &);
  const gNumber &operator()(Strategy *) const;

  MixedSolution &operator+=(const MixedSolution &);
  MixedSolution &operator-=(const MixedSolution &);
  MixedSolution &operator*=(const gNumber &);

  // GENERAL DATA ACCESS
  Nfg &Game(void) const { return m_profile.Game(); }
  gPrecision Precision(void) const { return m_precision; }
  const MixedProfile<gNumber> *Profile(void) const { return &m_profile; }

  // Do probabilities sum to one (within m_epsilon) for each player?)
  bool IsComplete(void) const;

  const gText &GetName(void) const { return m_name; }
  NfgAlgType Creator(void) const { CheckIsValid(); return m_creator; }
  const NFSupport &Support(void) const { CheckIsValid(); return m_support; }
  const gTriState &IsNash(void) const;
  const gTriState &IsPerfect(void) const;
  const gTriState &IsProper(void) const;
  const gNumber &Epsilon(void) const { CheckIsValid(); return m_epsilon; }
  const gNumber &QreLambda(void) const { CheckIsValid(); return m_qreLambda; }
  const gNumber &QreValue(void) const { CheckIsValid(); return m_qreValue; }
  const gNumber &LiapValue(void) const;

  void SetName(const gText &p_name) { m_name = p_name; }
  void SetEpsilon(const gNumber &p_epsilon) { m_epsilon = p_epsilon; }
  void SetQre(const gNumber &p_qreLambda, const gNumber &p_qreValue)
    { m_qreLambda = p_qreLambda; m_qreValue = p_qreValue; }
  
  // Force the invalidation of cached quantities
  void Invalidate(void) const;
  void CheckIsValid(void) const {if(!IsValid()) Invalidate();}
  bool IsValid(void) const {return (m_revision == Game().RevisionNumber());}
  MixedSolution PolishEq() const;

  // FUNCTIONS FOR COMPATIBILITY WITH GUI
  // these are all obsolescent :)
  gNumber Payoff(int p_player) const { return m_profile.Payoff(p_player); }
  const gArray<int> &Lengths(void) const { return m_profile.Lengths(); }
  
  // PAYOFF COMPUTATION
  gNumber Payoff(NFPlayer *, Strategy *) const;

  // OUTPUT
  void Dump(gOutput &) const;
  void DumpInfo(gOutput &) const;
};


gOutput &operator<<(gOutput &f, const MixedSolution &);


#endif    // MIXEDSOL_H
