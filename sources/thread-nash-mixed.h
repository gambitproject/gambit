//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Declaration of worker threads for computing mixed strategy equilibria
//
// This file is part of Gambit
// Copyright (c) 2004, The Gambit Project
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
//

#ifndef THREAD_NASH_MIXED_H
#define THREAD_NASH_MIXED_H

class gbtNashPanel;

//!
//! The base thread class for all worker threads that compute
//! Nash equilibria in mixed strategies.
//!
class gbtNashMixedThread : public wxThread {
protected:
  gbtNashPanel *m_parent;
  gbtList<gbtMixedProfile<double> > &m_eqa;

  // Member function for subclasses to do computation
  virtual void Compute(void) = 0;

public:
  gbtNashMixedThread(gbtNashPanel *,
		     gbtList<gbtMixedProfile<double> > &p_eqa);

  // Overriding members to implement thread
  void *Entry(void);
  void OnExit(void);
};  

//==========================================================================
//                    class gbtNashOneMixedThread
//==========================================================================

class gbtNashOneMixedThread : public gbtNashMixedThread {
public:
  gbtNashOneMixedThread(gbtNashPanel *p_parent,
			gbtList<gbtMixedProfile<double> > &p_eqa)
    : gbtNashMixedThread(p_parent, p_eqa) { }

private:
  void Compute(void);
};

//==========================================================================
//                   class gbtNashAllMixedThread
//==========================================================================

class gbtNashAllMixedThread : public gbtNashMixedThread {
public:
  gbtNashAllMixedThread(gbtNashPanel *p_parent,
			gbtList<gbtMixedProfile<double> > &p_eqa)
    : gbtNashMixedThread(p_parent, p_eqa) { }

private:
  void Compute(void);
};

//==========================================================================
//                   class gbtNashEnumMixedThread
//==========================================================================

class gbtNashEnumMixedThread : public gbtNashMixedThread {
private:
  int m_stopAfter;

public:
  gbtNashEnumMixedThread(gbtNashPanel *p_parent,
			 gbtList<gbtMixedProfile<double> > &p_eqa,
			 int p_stopAfter)
    : gbtNashMixedThread(p_parent, p_eqa), m_stopAfter(p_stopAfter) { }

private:
  void Compute(void);
};

//==========================================================================
//                    class gbtNashLcpMixedThread
//==========================================================================

class gbtNashLcpMixedThread : public gbtNashMixedThread {
private:
  int m_stopAfter;

public:
  gbtNashLcpMixedThread(gbtNashPanel *p_parent,
			gbtList<gbtMixedProfile<double> > &p_eqa,
			int p_stopAfter)
    : gbtNashMixedThread(p_parent, p_eqa), m_stopAfter(p_stopAfter) { }

private:
  void Compute(void);
};

//==========================================================================
//                   class gbtNashBfsLcpMixedThread
//==========================================================================

class gbtNashBfsLcpMixedThread : public gbtNashMixedThread {
private:
  int m_stopAfter;

public:
  gbtNashBfsLcpMixedThread(gbtNashPanel *p_parent,
			   gbtList<gbtMixedProfile<double> > &p_eqa,
			   int p_stopAfter)
    : gbtNashMixedThread(p_parent, p_eqa), m_stopAfter(p_stopAfter) { }

private:
  void Compute(void);
};

//==========================================================================
//                    class gbtNashLpMixedThread
//==========================================================================

class gbtNashLpMixedThread : public gbtNashMixedThread {
public:
  gbtNashLpMixedThread(gbtNashPanel *p_parent,
		       gbtList<gbtMixedProfile<double> > &p_eqa)
    : gbtNashMixedThread(p_parent, p_eqa) { }

private:
  void Compute(void);
};

//==========================================================================
//                     class gbtNashLiapMixedThread
//==========================================================================

class gbtNashLiapMixedThread : public gbtNashMixedThread {
private:
  int m_stopAfter;

public:
  gbtNashLiapMixedThread(gbtNashPanel *p_parent,
			 gbtList<gbtMixedProfile<double> > &p_eqa,
			 int p_stopAfter)
    : gbtNashMixedThread(p_parent, p_eqa), m_stopAfter(p_stopAfter) { }

private:
  void Compute(void);
};

//==========================================================================
//                  class gbtNashEnumPolyMixedThread
//==========================================================================

class gbtNashEnumPolyMixedThread : public gbtNashMixedThread {
private:
  int m_stopAfter;

public:
  gbtNashEnumPolyMixedThread(gbtNashPanel *p_parent,
			     gbtList<gbtMixedProfile<double> > &p_eqa,
			     int p_stopAfter)
    : gbtNashMixedThread(p_parent, p_eqa), m_stopAfter(p_stopAfter) { }

private:
  void Compute(void);
};

//==========================================================================
//                    class gbtNashLogitMixedThread
//==========================================================================

class gbtNashLogitMixedThread : public gbtNashMixedThread {
public:
  gbtNashLogitMixedThread(gbtNashPanel *p_parent,
			  gbtList<gbtMixedProfile<double> > &p_eqa)
    : gbtNashMixedThread(p_parent, p_eqa) { }

private:
  void Compute(void);
};

//==========================================================================
//                     class gbtNashPNSMixedThread
//==========================================================================

class gbtNashPNSMixedThread : public gbtNashMixedThread {
private:
  int m_stopAfter;

public:
  gbtNashPNSMixedThread(gbtNashPanel *p_parent,
			gbtList<gbtMixedProfile<double> > &p_eqa,
			int p_stopAfter)
    : gbtNashMixedThread(p_parent, p_eqa) { }

private:
  void Compute(void);
};

//==========================================================================
//                  class gbtNashSimpdivMixedThread
//==========================================================================

class gbtNashSimpdivMixedThread : public gbtNashMixedThread {
public:
  gbtNashSimpdivMixedThread(gbtNashPanel *p_parent,
			    gbtList<gbtMixedProfile<double> > &p_eqa)
    : gbtNashMixedThread(p_parent, p_eqa) { }

private:
  void Compute(void);
};

//==========================================================================
//                 class gbtNashYamamotoMixedThread
//==========================================================================

class gbtNashYamamotoMixedThread : public gbtNashMixedThread {
public:
  gbtNashYamamotoMixedThread(gbtNashPanel *p_parent,
			     gbtList<gbtMixedProfile<double> > &p_eqa)
    : gbtNashMixedThread(p_parent, p_eqa) { }

private:
  void Compute(void);
};

//==========================================================================
//                    class gbtNashGnmMixedThread
//==========================================================================

class gbtNashGnmMixedThread : public gbtNashMixedThread {
public:
  gbtNashGnmMixedThread(gbtNashPanel *p_parent,
			gbtList<gbtMixedProfile<double> > &p_eqa)
    : gbtNashMixedThread(p_parent, p_eqa) { }

private:
  void Compute(void);
};

//==========================================================================
//                    class gbtNashIpaMixedThread
//==========================================================================

class gbtNashIpaMixedThread : public gbtNashMixedThread {
public:
  gbtNashIpaMixedThread(gbtNashPanel *p_parent,
			gbtList<gbtMixedProfile<double> > &p_eqa)
    : gbtNashMixedThread(p_parent, p_eqa) { }

private:
  void Compute(void);
};

#endif  // THREAD_NASH_MIXED_H
