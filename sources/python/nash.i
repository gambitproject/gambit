%module nash
%include typemaps.i

typedef gbtList<gbtMixedProfile<double> > gbtMixedProfileDoubleList;
typedef gbtList<gbtMixedProfile<gbtRational> > gbtMixedProfileRationalList;

typedef gbtList<gbtBehavProfile<double> > gbtBehavProfileDoubleList;
typedef gbtList<gbtBehavProfile<gbtRational> > gbtBehavProfileRationalList;


%{
#include "../nash/efgpure.h"
#include "../nash/efglp.h"
#include "../nash/efglcp.h"
#include "../nash/efgliap.h"
#include "../nash/efglogit.h"
#include "../nash/efgpoly.h"

#include "../nash/nfgpure.h"
#include "../nash/nfgmixed.h"
#include "../nash/nfglp.h"
#include "../nash/nfglcp.h"
#include "../nash/nfgliap.h"
#include "../nash/nfglogit.h"
#include "../nash/nfgpoly.h"
#include "../nash/nfgsimpdiv.h"
#include "../nash/nfgyamamoto.h"

typedef gbtList<gbtMixedProfile<double> > gbtMixedProfileDoubleList;
typedef gbtList<gbtMixedProfile<gbtRational> > gbtMixedProfileRationalList;

typedef gbtList<gbtBehavProfile<double> > gbtBehavProfileDoubleList;
typedef gbtList<gbtBehavProfile<gbtRational> > gbtBehavProfileRationalList;


gbtMixedProfileDoubleList EnumMixedNfgDoubleHelper(const gbtGame &p_game,
						   int p_stopAfter)
{
  gbtMixedProfileDoubleList ret;
  Py_BEGIN_ALLOW_THREADS
  ret = gbtNashEnumMixedNfg(p_game, (double) 0.0, p_stopAfter);
  Py_END_ALLOW_THREADS
  return ret;
}

gbtMixedProfileRationalList EnumMixedNfgRationalHelper(const gbtGame &p_game,
						       int p_stopAfter)
{
  gbtMixedProfileRationalList ret;
  Py_BEGIN_ALLOW_THREADS
  ret = gbtNashEnumMixedNfg(p_game, (gbtRational) 0.0, p_stopAfter);
  Py_END_ALLOW_THREADS
  return ret;
}

gbtMixedProfileDoubleList EnumPolyNfgHelper(const gbtGame &p_game)
{
  gbtMixedProfileDoubleList ret;
  Py_BEGIN_ALLOW_THREADS
  ret = gbtNashEnumPolyNfg(p_game);
  Py_END_ALLOW_THREADS
  return ret;
}

gbtMixedProfileRationalList EnumPureNfgHelper(const gbtGame &p_game, 
					      int p_stopAfter)
{
  gbtMixedProfileRationalList ret;
  Py_BEGIN_ALLOW_THREADS
  ret = gbtNashEnumPureNfg(p_game, p_stopAfter);
  Py_END_ALLOW_THREADS
  return ret;
}

gbtMixedProfileDoubleList LcpNfgDoubleHelper(const gbtGame &p_game,
					     int p_stopAfter, int p_maxDepth)
{
  gbtMixedProfileDoubleList ret;
  Py_BEGIN_ALLOW_THREADS
  ret = gbtNashLcpNfg(p_game, (double) 0.0, p_stopAfter, p_maxDepth);
  Py_END_ALLOW_THREADS
  return ret;
}

gbtMixedProfileRationalList LcpNfgRationalHelper(const gbtGame &p_game,
						 int p_stopAfter, 
						 int p_maxDepth)
{
  gbtMixedProfileRationalList ret;
  Py_BEGIN_ALLOW_THREADS
  ret = gbtNashLcpNfg(p_game, (gbtRational) 0.0, p_stopAfter, p_maxDepth);
  Py_END_ALLOW_THREADS
  return ret;
}

gbtMixedProfileDoubleList LiapNfgHelper(const gbtMixedProfileDouble &p_profile,
					int p_maxitsN, double p_tolN,
					int p_maxits1, double p_tol1)
{
  gbtMixedProfileDoubleList ret;
  Py_BEGIN_ALLOW_THREADS
  ret = gbtNashLiapNfg(p_profile, p_maxitsN, p_tolN, p_maxits1, p_tol1);
  Py_END_ALLOW_THREADS
  return ret;
}

gbtMixedProfileDoubleList LpNfgDoubleHelper(const gbtGame &p_game)
{
  gbtMixedProfileDoubleList ret;
  Py_BEGIN_ALLOW_THREADS
  ret = gbtNashLpNfg(p_game, (double) 0.0);
  Py_END_ALLOW_THREADS
  return ret;
}

gbtMixedProfileRationalList LpNfgRationalHelper(const gbtGame &p_game)
{
  gbtMixedProfileRationalList ret;
  Py_BEGIN_ALLOW_THREADS
  ret = gbtNashLpNfg(p_game, (gbtRational) 0.0);
  Py_END_ALLOW_THREADS
  return ret;
}

gbtMixedProfileDoubleList LogitNfgHelper(const gbtGame &p_game)
{
  gbtMixedProfileDoubleList ret;
  Py_BEGIN_ALLOW_THREADS
  ret = gbtNashLogitNfg(p_game);
  Py_END_ALLOW_THREADS
  return ret;
}

gbtMixedProfileDoubleList 
SimpdivNfgDoubleHelper(const gbtMixedProfileDouble &p_start, int p_leash)
{
  gbtMixedProfileDoubleList ret;
  Py_BEGIN_ALLOW_THREADS
  ret = gbtNashSimpdivNfg(p_start, p_leash);
  Py_END_ALLOW_THREADS
  return ret;
}

gbtMixedProfileRationalList 
SimpdivNfgRationalHelper(const gbtMixedProfileRational &p_start, 
        	          int p_leash)
{
  gbtMixedProfileRationalList ret;
  Py_BEGIN_ALLOW_THREADS
  ret = gbtNashSimpdivNfg(p_start, p_leash);
  Py_END_ALLOW_THREADS
  return ret;
}

gbtMixedProfileDoubleList YamamotoNfgHelper(const gbtGame &p_game)
{
  gbtMixedProfileDoubleList ret;
  Py_BEGIN_ALLOW_THREADS
  ret = gbtNashYamamotoNfg(p_game);
  Py_END_ALLOW_THREADS
  return ret;
}

gbtBehavProfileDoubleList EnumPolyEfgHelper(const gbtGame &p_game)
{
  gbtBehavProfileDoubleList ret;
  Py_BEGIN_ALLOW_THREADS
  ret = gbtNashEnumPolyEfg(p_game);
  Py_END_ALLOW_THREADS
  return ret;
}

gbtBehavProfileRationalList EnumPureEfgHelper(const gbtGame &p_game, 
					      int p_stopAfter)
{
  gbtBehavProfileRationalList ret;
  Py_BEGIN_ALLOW_THREADS
  ret = gbtNashEnumPureEfg(p_game, p_stopAfter);
  Py_END_ALLOW_THREADS
  return ret;
}

gbtBehavProfileDoubleList LcpEfgDoubleHelper(const gbtGame &p_game,
					     int p_stopAfter, int p_maxDepth)
{
  gbtBehavProfileDoubleList ret;
  Py_BEGIN_ALLOW_THREADS
  ret = gbtNashLcpEfg(p_game, (double) 0.0, p_stopAfter, p_maxDepth);
  Py_END_ALLOW_THREADS
  return ret;
}

gbtBehavProfileRationalList LcpEfgRationalHelper(const gbtGame &p_game,
						 int p_stopAfter, 
						 int p_maxDepth)
{
  gbtBehavProfileRationalList ret;
  Py_BEGIN_ALLOW_THREADS
  ret = gbtNashLcpEfg(p_game, (gbtRational) 0.0, p_stopAfter, p_maxDepth);
  Py_END_ALLOW_THREADS
  return ret;
}

gbtBehavProfileDoubleList LiapEfgHelper(const gbtBehavProfileDouble &p_profile,
					int p_maxitsN, double p_tolN,
					int p_maxits1, double p_tol1)
{
  gbtBehavProfileDoubleList ret;
  Py_BEGIN_ALLOW_THREADS
  ret = gbtNashLiapEfg(p_profile, p_maxitsN, p_tolN, p_maxits1, p_tol1);
  Py_END_ALLOW_THREADS
  return ret;
}

gbtBehavProfileDoubleList LpEfgDoubleHelper(const gbtGame &p_game)
{
  gbtBehavProfileDoubleList ret;
  Py_BEGIN_ALLOW_THREADS
  ret = gbtNashLpEfg(p_game, (double) 0.0);
  Py_END_ALLOW_THREADS
  return ret;
}

gbtBehavProfileRationalList LpEfgRationalHelper(const gbtGame &p_game)
{
  gbtBehavProfileRationalList ret;
  Py_BEGIN_ALLOW_THREADS
  ret = gbtNashLpEfg(p_game, (gbtRational) 0.0);
  Py_END_ALLOW_THREADS
  return ret;
}

gbtBehavProfileDoubleList LogitEfgHelper(const gbtGame &p_game)
{
  gbtBehavProfileDoubleList ret;
  Py_BEGIN_ALLOW_THREADS
  ret = gbtNashLogitEfg(p_game);
  Py_END_ALLOW_THREADS
  return ret;
}


%}


%nodefault;
class gbtMixedProfileDoubleList {
public:
  gbtMixedProfileDoubleList(const gbtMixedProfileDoubleList &);
  ~gbtMixedProfileDoubleList();
};

%extend gbtMixedProfileDoubleList {
  int __len__(void) { return self->Length(); }
  gbtMixedProfileDouble *__getitem__(int index) 
	{ return new gbtMixedProfileDouble((*self)[index+1]); }
};

%nodefault;
class gbtMixedProfileRationalList {
public:
  gbtMixedProfileRationalList(const gbtMixedProfileRationalList &);
  ~gbtMixedProfileRationalList();
};

%extend gbtMixedProfileRationalList {
  int __len__(void) { return self->Length(); }
  gbtMixedProfileRational *__getitem__(int index) 
	{ return new gbtMixedProfileRational((*self)[index+1]); }
};

gbtMixedProfileDoubleList EnumMixedNfgDoubleHelper(const gbtGame &, int);
gbtMixedProfileRationalList EnumMixedNfgRationalHelper(const gbtGame &, int);
gbtMixedProfileDoubleList EnumPolyNfgHelper(const gbtGame &);
gbtMixedProfileRationalList EnumPureNfgHelper(const gbtGame &, int);
gbtMixedProfileDoubleList LcpNfgDoubleHelper(const gbtGame &, int, int);
gbtMixedProfileRationalList LcpNfgRationalHelper(const gbtGame &, int, int);
gbtMixedProfileDoubleList LiapNfgHelper(const gbtMixedProfileDouble &,
					int, double, int, double);
gbtMixedProfileDoubleList LpNfgDoubleHelper(const gbtGame &);
gbtMixedProfileRationalList LpNfgRationalHelper(const gbtGame &);
gbtMixedProfileDoubleList LogitNfgHelper(const gbtGame &);
gbtMixedProfileDoubleList 
SimpdivNfgDoubleHelper(const gbtMixedProfileDouble &, int);
gbtMixedProfileRationalList 
SimpdivNfgRationalHelper(const gbtMixedProfileRational &, int);
gbtMixedProfileDoubleList YamamotoNfgHelper(const gbtGame &);

%nodefault;
class gbtBehavProfileDoubleList {
public:
  gbtBehavProfileDoubleList(const gbtBehavProfileDoubleList &);
  ~gbtBehavProfileDoubleList();
};

%extend gbtBehavProfileDoubleList {
  int __len__(void) { return self->Length(); }
  gbtBehavProfileDouble *__getitem__(int index) 
	{ return new gbtBehavProfileDouble((*self)[index+1]); }
};

%nodefault;
class gbtBehavProfileRationalList {
public:
  gbtBehavProfileRationalList(const gbtBehavProfileRationalList &);
  ~gbtBehavProfileRationalList();
};

%extend gbtBehavProfileRationalList {
  int __len__(void) { return self->Length(); }
  gbtBehavProfileRational *__getitem__(int index) 
	{ return new gbtBehavProfileRational((*self)[index+1]); }
};

gbtBehavProfileRationalList EnumPureEfgHelper(const gbtGame &, int);
gbtBehavProfileDoubleList LcpEfgDoubleHelper(const gbtGame &, int, int);
gbtBehavProfileRationalList LcpEfgRationalHelper(const gbtGame &, int, int);
gbtBehavProfileDoubleList LiapEfgHelper(const gbtBehavProfileDouble &,
					int, double, int, double);
gbtBehavProfileDoubleList LpEfgDoubleHelper(const gbtGame &);
gbtBehavProfileRationalList LpEfgRationalHelper(const gbtGame &);
gbtBehavProfileDoubleList LogitEfgHelper(const gbtGame &);
gbtBehavProfileDoubleList EnumPolyEfgHelper(const gbtGame &);
