%module mixed
%include typemaps.i

%{
#include "../game/game.h"

%}

%nodefault;
class gbtMixedProfileDouble {
public:
  virtual ~gbtMixedProfileDouble();
};

%extend gbtMixedProfileDouble {
  //--------------------------------------------------------------------
  //             General information about the profile
  //--------------------------------------------------------------------
  gbtGame GetGame(void) const { return (*self)->GetGame(); }
  int StrategyProfileLength(void) const
    { return (*self)->StrategyProfileLength(); }
  bool IsRational(void) const { return false; }  
  gbtBehavProfileDouble GetBehavior(void) const 
    { return (*self)->GetBehavior(); }

  //--------------------------------------------------------------------
  //              Accessing the profile probabilities
  //--------------------------------------------------------------------
  double GetStrategyProb(const gbtGameStrategy &s) const
    { return (*self)->GetStrategyProb(s); }
  void SetStrategyProb(const gbtGameStrategy &s, const double &prob)
    { (*self)->SetStrategyProb(s, prob); }
  void SetCentroid(void) { (*self)->SetCentroid(); }

  //--------------------------------------------------------------------
  //                      Payoff information
  //--------------------------------------------------------------------
  double GetPayoff(const gbtGamePlayer &p) const
    { return (*self)->GetPayoff(p); }
  double GetStrategyValue(const gbtGameStrategy &s) const
    { return (*self)->GetStrategyValue(s); }

  double GetLiapValue(bool penalty) const
    { return (*self)->GetLiapValue(penalty); }

  //--------------------------------------------------------------------
  //              Restricting and unrestricting profiles
  //--------------------------------------------------------------------
  gbtMixedProfileDouble Unrestrict(void) const
    { return (*self)->Unrestrict(); }
};


%nodefault;
class gbtMixedProfileRational {
public:
  virtual ~gbtMixedProfileRational();
};

%extend gbtMixedProfileRational {
  //--------------------------------------------------------------------
  //             General information about the profile
  //--------------------------------------------------------------------
  gbtGame GetGame(void) const { return (*self)->GetGame(); }
  int StrategyProfileLength(void) const
    { return (*self)->StrategyProfileLength(); }
  bool IsRational(void) const { return true; }
  gbtBehavProfileRational GetBehavior(void) const 
    { return (*self)->GetBehavior(); }
  
  //--------------------------------------------------------------------
  //              Accessing the profile probabilities
  //--------------------------------------------------------------------
  gbtRational GetStrategyProb(const gbtGameStrategy &s) const
    { return (*self)->GetStrategyProb(s); }
  void SetStrategyProb(const gbtGameStrategy &s, const gbtRational &prob)
    { (*self)->SetStrategyProb(s, prob); }
  void SetCentroid(void) { (*self)->SetCentroid(); }

  //--------------------------------------------------------------------
  //                    Payoff information
  //--------------------------------------------------------------------
  gbtRational GetPayoff(const gbtGamePlayer &p) const
    { return (*self)->GetPayoff(p); }
  gbtRational GetStrategyValue(const gbtGameStrategy &s) const
    { return (*self)->GetStrategyValue(s); }

  gbtRational GetLiapValue(bool penalty) const
    { return (*self)->GetLiapValue(penalty); }

  //--------------------------------------------------------------------
  //              Restricting and unrestricting profiles
  //--------------------------------------------------------------------
  gbtMixedProfileRational Unrestrict(void) const
    { return (*self)->Unrestrict(); }
}
