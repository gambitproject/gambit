//
// FILE: mixedsol.h -- Mixed strategy solution classes
//
// $Id$
//

#ifndef MIXEDSOL_H
#define MIXEDSOL_H

#include "gmisc.h"
#include "mixed.h"
#include "gstream.h"
#include "gtext.h"
#include "gnumber.h"
#include "nfplayer.h"
#include "nfg.h"

typedef enum {
  algorithmNfg_USER,
  algorithmNfg_ENUMPURE, algorithmNfg_ENUMMIXED,
  algorithmNfg_LCP, algorithmNfg_LP, algorithmNfg_LIAP,
  algorithmNfg_SIMPDIV, algorithmNfg_POLENUM, algorithmNfg_ALLNASHSOLVE,
  algorithmNfg_QRE, algorithmNfg_QREALL
} NfgAlgType;

gText NameNfgAlgType(NfgAlgType i);
void DisplayNfgAlgType(gOutput& o, NfgAlgType i);



class MixedSolution   {
protected:
  MixedProfile<gNumber> m_profile;
  gPrecision m_precision;
  mutable NFSupport m_support;
  mutable NfgAlgType m_creator;
  mutable gTriState m_isNash, m_isPerfect, m_isProper;
  mutable bool m_checkedPerfect;
  mutable gNumber m_epsilon, m_qreLambda, m_qreValue, m_liapValue;
  gArray<gNumber> m_payoff;
  unsigned int m_id;
  
  void CheckIsNash(void) const;
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

  // Do probabilities sum to one (within m_epsilon) for each player?)
  bool IsComplete(void) const;

  unsigned int Id(void) const { return m_id; }
  NfgAlgType Creator(void) const { return m_creator; }
  const NFSupport &Support(void) const { return m_support; }
  gTriState IsNash(void) const;
  gTriState IsPerfect(void) const;
  gTriState IsProper(void) const;
  const gNumber &Epsilon(void) const { return m_epsilon; }
  const gNumber &QreLambda(void) const { return m_qreLambda; }
  const gNumber &QreValue(void) const { return m_qreValue; }
  const gNumber &LiapValue(void) const;

  void SetId(unsigned int p_id) { m_id = p_id; }
  void SetEpsilon(const gNumber &p_epsilon) { m_epsilon = p_epsilon; }
  void SetQre(const gNumber &p_qreLambda, const gNumber &p_qreValue)
    { m_qreLambda = p_qreLambda; m_qreValue = p_qreValue; }
  void SetLiap(const gNumber &p_liapValue) { m_liapValue = p_liapValue; }
  
  // Force the invalidation of cached quantities
  void Invalidate(void) const;
  

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
