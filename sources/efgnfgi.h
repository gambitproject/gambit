//********************************************************************
// @(#)efgnfgi.h	1.1 5/1/95
// This header defines an interface between the normal and extensive
// form parts of the Gambit program to facilitate the sharing of the
// solutions between the two modules.  The top-level classes of both
// modules are derived from this class, and override the respective
// virtual functions.  When this class is passed to another instance
// of this class, a two way link is automatically estabilished.  When
// one of the classes dies, the other is notified.  Used in normshow.cc
// and extshow.cc.  To kill the link manually, call InterfaceDied.
// Since one usually needs to get at the actual data in the EF/NF, the
// function InterfaceObject[Efg/Nfg] is provided to access it.
//********************************************************************

#ifndef EFGNFGI_H
#define EFGNFGI_H

#include "behav.h"
#include "mixed.h"
typedef enum {gNONE=0,gNFG,gEFG} InterfaceTypeT;
template <class T> class NormalForm;
template <class T> class ExtForm;
// We need this base class to allow the non-templated functions to take
// EfgNfgInterface as a parameter.  By itself, its useless.
class BaseEfgNfgInterface
{
public:
 virtual int InterfaceOk(void) {return 0;}
};
// The actual interface, note that it is templated
template <class T>
class EfgNfgInterface: public BaseEfgNfgInterface
{
InterfaceTypeT type;
EfgNfgInterface<T> *inter;
public:
// Constructor
	EfgNfgInterface(InterfaceTypeT t,EfgNfgInterface<T> *i=0):
		type(t), inter(i) {if (inter) inter->InterfaceSet(this);}
// Destructor
	virtual ~EfgNfgInterface(void) {InterfaceDied();}
// Data access
	void InterfaceDied(void) {if (inter) inter->InterfaceSet(0);inter=0;}
	InterfaceTypeT InterfaceType(void) const {return type;}
	EfgNfgInterface<T> *Interface(void) {return inter;}
	int InterfaceOk(void) {return (inter) ? 1 : 0;}
	void InterfaceSet(EfgNfgInterface<T> *i) {inter=i;}
// Solution passing. set controls if the solution just sent will be displayed now.
	virtual void SolutionToEfg(const BehavProfile<T> &s,bool set=false)
		{assert(type==gNFG); inter->SolutionToEfg(s);}
	virtual void SolutionToNfg(const MixedProfile<T> &s,bool set=false)
		{assert(type==gEFG); inter->SolutionToNfg(s);}
// Getting at the underlying object
	virtual const ExtForm<T> *InterfaceObjectEfg(void)
		{assert(type==gNFG); return inter->InterfaceObjectEfg();}
	virtual const NormalForm<T> *InterfaceObjectNfg(void)
		{assert(type==gEFG); return inter->InterfaceObjectNfg();}
};

#endif
