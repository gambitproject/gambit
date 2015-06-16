#
# This file is part of Gambit
# Copyright (c) 1994-2014, The Gambit Project (http://www.gambit-project.org)
#
# FILE: src/python/gambit/lib/nash.pxi
# Cython wrapper for Nash equilibrium computations
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
#

cdef extern from "tools/enumpure/enumpure.h":
    cdef cppclass c_NashEnumPureStrategySolver "NashEnumPureStrategySolver":
        c_NashEnumPureStrategySolver()
        c_List[c_MixedStrategyProfileRational] Solve(c_Game) except +RuntimeError

    cdef cppclass c_NashEnumPureAgentSolver "NashEnumPureAgentSolver":
        c_NashEnumPureAgentSolver()
        c_List[c_MixedBehaviorProfileRational] Solve(c_Game) except +RuntimeError

cdef class EnumPureStrategySolver(object):
    cdef c_NashEnumPureStrategySolver *alg

    def __cinit__(self):
        self.alg = new c_NashEnumPureStrategySolver()
    def __dealloc__(self):
        del self.alg
    def solve(self, Game game):
        cdef c_List[c_MixedStrategyProfileRational] solns
        cdef MixedStrategyProfileRational p
        solns = self.alg.Solve(game.game)
        ret = [ ]
        for i in xrange(solns.Length()):
            p = MixedStrategyProfileRational()
            p.profile = copyitem_list_mspr(solns, i+1)
            ret.append(p)
        return ret

cdef class EnumPureAgentSolver(object):
    cdef c_NashEnumPureAgentSolver *alg

    def __cinit__(self, p_stopAfter=0, p_maxDepth=0):
        self.alg = new c_NashEnumPureAgentSolver()
    def __dealloc__(self):
        del self.alg
    def solve(self, Game game):
        cdef c_List[c_MixedBehaviorProfileRational] solns
        cdef MixedBehaviorProfileRational p
        solns = self.alg.Solve(game.game)
        ret = [ ]
        for i in xrange(solns.Length()):
            p = MixedBehaviorProfileRational()
            p.profile = copyitem_list_mbpr(solns, i+1)
            ret.append(p)
        return ret


cdef extern from "tools/lcp/nfglcp.h":
    cdef cppclass c_NashLcpStrategySolverDouble "NashLcpStrategySolver<double>":
        c_NashLcpStrategySolverDouble(int, int)
        c_List[c_MixedStrategyProfileDouble] Solve(c_Game) except +RuntimeError

    cdef cppclass c_NashLcpStrategySolverRational "NashLcpStrategySolver<Rational>":
        c_NashLcpStrategySolverRational(int, int)
        c_List[c_MixedStrategyProfileRational] Solve(c_Game) except +RuntimeError

cdef extern from "tools/lcp/efglcp.h":
    cdef cppclass c_NashLcpBehaviorSolverDouble "NashLcpBehaviorSolver<double>":
        c_NashLcpBehaviorSolverDouble(int, int)
        c_List[c_MixedBehaviorProfileDouble] Solve(c_Game) except +RuntimeError

    cdef cppclass c_NashLcpBehaviorSolverRational "NashLcpBehaviorSolver<Rational>":
        c_NashLcpBehaviorSolverRational(int, int)
        c_List[c_MixedBehaviorProfileRational] Solve(c_Game) except +RuntimeError


cdef class LCPBehaviorSolverDouble(object):
    cdef c_NashLcpBehaviorSolverDouble *alg

    def __cinit__(self, p_stopAfter=0, p_maxDepth=0):
        self.alg = new c_NashLcpBehaviorSolverDouble(p_stopAfter, p_maxDepth)
    def __dealloc__(self):
        del self.alg
    def solve(self, Game game):
        cdef c_List[c_MixedBehaviorProfileDouble] solns
        cdef MixedBehaviorProfileDouble p
        solns = self.alg.Solve(game.game)
        ret = [ ]
        for i in xrange(solns.Length()):
            p = MixedBehaviorProfileDouble()
            p.profile = copyitem_list_mbpd(solns, i+1)
            ret.append(p)
        return ret

cdef class LCPBehaviorSolverRational(object):
    cdef c_NashLcpBehaviorSolverRational *alg

    def __cinit__(self, p_stopAfter=0, p_maxDepth=0):
        self.alg = new c_NashLcpBehaviorSolverRational(p_stopAfter, p_maxDepth)
    def __dealloc__(self):
        del self.alg
    def solve(self, Game game):
        cdef c_List[c_MixedBehaviorProfileRational] solns
        cdef MixedBehaviorProfileRational p
        solns = self.alg.Solve(game.game)
        ret = [ ]
        for i in xrange(solns.Length()):
            p = MixedBehaviorProfileRational()
            p.profile = copyitem_list_mbpr(solns, i+1)
            ret.append(p)
        return ret

cdef class LCPStrategySolverDouble(object): 
    cdef c_NashLcpStrategySolverDouble *alg

    def __cinit__(self, p_stopAfter=0, p_maxDepth=0):
        self.alg = new c_NashLcpStrategySolverDouble(p_stopAfter, p_maxDepth)
    def __dealloc__(self):
        del self.alg
    def solve(self, Game game):
        cdef c_List[c_MixedStrategyProfileDouble] solns
        cdef MixedStrategyProfileDouble p
        solns = self.alg.Solve(game.game)
        ret = [ ]
        for i in xrange(solns.Length()):
            p = MixedStrategyProfileDouble()
            p.profile = copyitem_list_mspd(solns, i+1)
            ret.append(p)
        return ret

cdef class LCPStrategySolverRational(object):
    cdef c_NashLcpStrategySolverRational *alg

    def __cinit__(self, p_stopAfter=0, p_maxDepth=0):
        self.alg = new c_NashLcpStrategySolverRational(p_stopAfter, p_maxDepth)
    def __dealloc__(self):
        del self.alg
    def solve(self, Game game):
        cdef c_List[c_MixedStrategyProfileRational] solns
        cdef MixedStrategyProfileRational p
        solns = self.alg.Solve(game.game)
        ret = [ ]
        for i in xrange(solns.Length()):
            p = MixedStrategyProfileRational()
            p.profile = copyitem_list_mspr(solns, i+1)
            ret.append(p)
        return ret


cdef extern from "tools/lp/nfglp.h":
    cdef cppclass c_NashLpStrategySolverDouble "NashLpStrategySolver<double>":
        c_NashLpStrategySolverDouble()
        c_List[c_MixedStrategyProfileDouble] Solve(c_Game) except +RuntimeError

    cdef cppclass c_NashLpStrategySolverRational "NashLpStrategySolver<Rational>":
        c_NashLpStrategySolverRational()
        c_List[c_MixedStrategyProfileRational] Solve(c_Game) except +RuntimeError

cdef extern from "tools/lp/efglp.h":
    cdef cppclass c_NashLpBehavSolverDouble "NashLpBehavSolver<double>":
        c_NashLpBehavSolverDouble()
        c_List[c_MixedBehaviorProfileDouble] Solve(c_Game) except +RuntimeError

    cdef cppclass c_NashLpBehavSolverRational "NashLpBehavSolver<Rational>":
        c_NashLpBehavSolverRational()
        c_List[c_MixedBehaviorProfileRational] Solve(c_Game) except +RuntimeError


cdef class LPBehaviorSolverDouble(object):
    cdef c_NashLpBehavSolverDouble *alg

    def __cinit__(self):
        self.alg = new c_NashLpBehavSolverDouble()
    def __dealloc__(self):
        del self.alg
    def solve(self, Game game):
        cdef c_List[c_MixedBehaviorProfileDouble] solns
        cdef MixedBehaviorProfileDouble p
        solns = self.alg.Solve(game.game)
        ret = [ ]
        for i in xrange(solns.Length()):
            p = MixedBehaviorProfileDouble()
            p.profile = copyitem_list_mbpd(solns, i+1)
            ret.append(p)
        return ret

cdef class LPBehaviorSolverRational(object):
    cdef c_NashLpBehavSolverRational *alg

    def __cinit__(self):
        self.alg = new c_NashLpBehavSolverRational()
    def __dealloc__(self):
        del self.alg
    def solve(self, Game game):
        cdef c_List[c_MixedBehaviorProfileRational] solns
        cdef MixedBehaviorProfileRational p
        solns = self.alg.Solve(game.game)
        ret = [ ]
        for i in xrange(solns.Length()):
            p = MixedBehaviorProfileRational()
            p.profile = copyitem_list_mbpr(solns, i+1)
            ret.append(p)
        return ret

cdef class LPStrategySolverDouble(object): 
    cdef c_NashLpStrategySolverDouble *alg

    def __cinit__(self):
        self.alg = new c_NashLpStrategySolverDouble()
    def __dealloc__(self):
        del self.alg
    def solve(self, Game game):
        cdef c_List[c_MixedStrategyProfileDouble] solns
        cdef MixedStrategyProfileDouble p
        solns = self.alg.Solve(game.game)
        ret = [ ]
        for i in xrange(solns.Length()):
            p = MixedStrategyProfileDouble()
            p.profile = copyitem_list_mspd(solns, i+1)
            ret.append(p)
        return ret

cdef class LPStrategySolverRational(object):
    cdef c_NashLpStrategySolverRational *alg

    def __cinit__(self):
        self.alg = new c_NashLpStrategySolverRational()
    def __dealloc__(self):
        del self.alg
    def solve(self, Game game):
        cdef c_List[c_MixedStrategyProfileRational] solns
        cdef MixedStrategyProfileRational p
        solns = self.alg.Solve(game.game)
        ret = [ ]
        for i in xrange(solns.Length()):
            p = MixedStrategyProfileRational()
            p.profile = copyitem_list_mspr(solns, i+1)
            ret.append(p)
        return ret


cdef extern from "tools/logit/nfglogit.h":
    cdef cppclass c_LogitQREMixedStrategyProfile "LogitQREMixedStrategyProfile":
        c_LogitQREMixedStrategyProfile(c_Game)
        c_LogitQREMixedStrategyProfile(c_LogitQREMixedStrategyProfile)
        c_Game GetGame()
        c_MixedStrategyProfileDouble GetProfile()
        double GetLambda()
        int MixedProfileLength()
        double getitem "operator[]"(int) except +IndexError
	
    cdef cppclass c_StrategicQREEstimator "StrategicQREEstimator":
        c_StrategicQREEstimator()
        c_LogitQREMixedStrategyProfile Estimate(c_LogitQREMixedStrategyProfile,
	                                        c_MixedStrategyProfileDouble,
						double, double, double) except +RuntimeError

cdef extern from "util.h":
    c_LogitQREMixedStrategyProfile *copyitem_list_qrem "copyitem"(c_List[c_LogitQREMixedStrategyProfile], int)        

cdef extern from "nash.h":
    c_LogitQREMixedStrategyProfile *_logit_estimate "logit_estimate"(c_MixedStrategyProfileDouble *)
    c_LogitQREMixedStrategyProfile *_logit_atlambda "logit_atlambda"(c_Game, double)
    c_List[c_LogitQREMixedStrategyProfile] _logit_principal_branch "logit_principal_branch"(c_Game, double)
    
cdef class LogitQREMixedStrategyProfile(object):
    cdef c_LogitQREMixedStrategyProfile *thisptr
    def __init__(self, game=None):
        if game is not None:
            self.thisptr = new c_LogitQREMixedStrategyProfile((<Game>game).game)
    def __dealloc__(self):
        del self.thisptr
    def __repr__(self):
        return "LogitQREMixedStrategyProfile(lam=%f,profile=%s)" % (self.lam, self.profile)

    def __len__(self):
        return self.thisptr.MixedProfileLength()
    def __getitem__(self, int i):
        return self.thisptr.getitem(i+1)

    property game:
        def __get__(self):
            cdef Game g
            g = Game()
            g.game = self.thisptr.GetGame()
            return g

    property lam: 
        def __get__(self):
            return self.thisptr.GetLambda()

    property profile:
        def __get__(self):
            cdef MixedStrategyProfileDouble profile
            profile = MixedStrategyProfileDouble()
            profile.profile = new c_MixedStrategyProfileDouble(deref(self.thisptr).GetProfile())
            return profile

def logit_estimate(MixedStrategyProfileDouble p_profile):
    """Estimate QRE corresponding to mixed strategy profile using
    maximum likelihood along the principal branch.
    """
    cdef LogitQREMixedStrategyProfile ret
    ret = LogitQREMixedStrategyProfile()
    ret.thisptr = _logit_estimate(p_profile.profile)
    return ret

def logit_atlambda(Game p_game, double p_lambda):
    """Compute the first QRE along the principal branch with the given
    lambda parameter.
    """
    cdef LogitQREMixedStrategyProfile ret
    ret = LogitQREMixedStrategyProfile()
    ret.thisptr = _logit_atlambda(p_game.game, p_lambda)
    return ret
   
def logit_principal_branch(Game p_game, double p_maxLambda=100000.0):
    cdef c_List[c_LogitQREMixedStrategyProfile] solns
    cdef LogitQREMixedStrategyProfile p
    solns = _logit_principal_branch(p_game.game, p_maxLambda)
    ret = [ ]
    for i in xrange(solns.Length()):
        p = LogitQREMixedStrategyProfile()
        p.thisptr = copyitem_list_qrem(solns, i+1)
        ret.append(p)
    return ret
