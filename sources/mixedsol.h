//
// FILE: mixedsol.h -- Mixed strategy solution classes
//
// $Id$
//

#ifndef MIXEDSOL_H
#define MIXEDSOL_H

#include "gmisc.h"
#include "mixed.h"
#include "gambitio.h"
#include "gtext.h"
#include "gnumber.h"

typedef enum 
{
  NfgAlg_USER,
  NfgAlg_GOBIT,
  NfgAlg_LIAP,
  NfgAlg_LEMKE,
  NfgAlg_ZSUM,
  NfgAlg_ENUM,
  NfgAlg_GOBITGRID,
  NfgAlg_SIMPDIV,
  NfgAlg_PURENASH,
  NfgAlg_SEQFORM,
  NfgAlg_POLENUM
} NfgAlgType;

gText NameNfgAlgType(NfgAlgType i);
void DisplayNfgAlgType(gOutput& o, NfgAlgType i);



class MixedSolution : public MixedProfile<gNumber>  {
protected:
  NfgAlgType _Creator;
  mutable TriState _IsNash;
  mutable TriState _IsPerfect;
  mutable TriState _IsProper;
  gNumber _Epsilon;
  gNumber _GobitLambda;
  gNumber _GobitValue;
  mutable gNumber _LiapValue;
  gArray<gNumber> _Payoff;
  unsigned int _Id;
  
  void EvalEquilibria(void) const;
  
public:
  MixedSolution(const MixedProfile<double> &, NfgAlgType creator = NfgAlg_USER);
  MixedSolution(const MixedProfile<gRational> &, NfgAlgType creator = NfgAlg_USER);
  MixedSolution(const MixedProfile<gNumber> &, NfgAlgType creator = NfgAlg_USER);
  MixedSolution(const MixedSolution &);
  
  virtual ~MixedSolution();

  unsigned int Id(void) const;
  void SetId(unsigned int );
  void SetCreator(NfgAlgType);
  NfgAlgType Creator(void) const; //Who created this object? (algorithm ID or user)
  bool IsComplete(void) const;
  
  void SetIsNash(TriState);
  TriState IsNash(void) const; // Is it Nash? Y/N/DK
  void SetIsPerfect(TriState);
  TriState IsPerfect(void) const; //Is it Perfect? Y/N/DK
  void SetIsProper(TriState);
  TriState IsProper(void) const; //Is it Proper? Y/N/DK
  
  void SetEpsilon(gNumber value);
  gNumber Epsilon(void) const; // epsilon for zero tolerance
  
  void SetGobit(gNumber lambda, gNumber value);
  gNumber GobitLambda(void) const; // lambda from gobit alg
  gNumber GobitValue(void) const; // objective function from gobit alg
  void SetLiap(gNumber value);
  gNumber LiapValue(void) const; // liapunov function value (to test for Nash)
  
  bool Equals(const MixedProfile<double> &s) const;
  bool operator==(const MixedSolution &) const;
  void Dump(gOutput& f) const;
  
  
  void Invalidate();
  
  gNumber& operator[](int);
  const gNumber& operator[](int) const;
  gNumber& operator()(int, int);
  const gNumber& operator()(int, int) const;
  
// for now, we may safely use the predefined assignment operator
// for MixedProfile.  This must be written if dynamically allocated
// members are added!!!!!!
  MixedSolution &operator=(const MixedSolution &);
};


gOutput &operator<<(gOutput &f, const MixedSolution &);

#endif    // MIXEDSOL_H
