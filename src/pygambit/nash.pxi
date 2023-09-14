#
# This file is part of Gambit
# Copyright (c) 1994-2023, The Gambit Project (http://www.gambit-project.org)
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

from libcpp.memory cimport shared_ptr, make_shared


cdef class EnumPureStrategySolver:
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

cdef class EnumPureAgentSolver:
    cdef c_NashEnumPureAgentSolver *alg

    def __cinit__(self, p_stopAfter=0, p_maxDepth=0):
        self.alg = new c_NashEnumPureAgentSolver()
    def __dealloc__(self):
        del self.alg
    def solve(self, Game game):
        cdef c_List[c_MixedBehaviorProfileRational] solns
        cdef MixedBehaviorProfileRational p
        cdef shared_ptr[c_BehaviorSupportProfile] profile
        profile = make_shared[c_BehaviorSupportProfile](game.game)
        solns = self.alg.Solve(deref(profile))
        ret = [ ]
        for i in xrange(solns.Length()):
            p = MixedBehaviorProfileRational()
            p.profile = copyitem_list_mbpr(solns, i+1)
            ret.append(p)
        return ret


cdef class EnumMixedStrategySolverDouble:
    cdef c_NashEnumMixedStrategySolverDouble *alg

    def __cinit__(self):
        self.alg = new c_NashEnumMixedStrategySolverDouble()
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

cdef class EnumMixedStrategySolverRational:
    cdef c_NashEnumMixedStrategySolverRational *alg

    def __cinit__(self):
        self.alg = new c_NashEnumMixedStrategySolverRational()
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

cdef class EnumMixedLrsStrategySolver:
    cdef c_NashEnumMixedLrsStrategySolver *alg

    def __cinit__(self):
        self.alg = new c_NashEnumMixedLrsStrategySolver()
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


cdef class LCPBehaviorSolverDouble:
    cdef c_NashLcpBehaviorSolverDouble *alg

    def __cinit__(self, p_stopAfter=0, p_maxDepth=0):
        self.alg = new c_NashLcpBehaviorSolverDouble(p_stopAfter, p_maxDepth)
    def __dealloc__(self):
        del self.alg
    def solve(self, Game game):
        cdef c_List[c_MixedBehaviorProfileDouble] solns
        cdef MixedBehaviorProfileDouble p
        cdef shared_ptr[c_BehaviorSupportProfile] profile
        profile = make_shared[c_BehaviorSupportProfile](game.game)
        solns = self.alg.Solve(deref(profile))
        ret = [ ]
        for i in xrange(solns.Length()):
            p = MixedBehaviorProfileDouble()
            p.profile = copyitem_list_mbpd(solns, i+1)
            ret.append(p)
        return ret

cdef class LCPBehaviorSolverRational:
    cdef c_NashLcpBehaviorSolverRational *alg

    def __cinit__(self, p_stopAfter=0, p_maxDepth=0):
        self.alg = new c_NashLcpBehaviorSolverRational(p_stopAfter, p_maxDepth)
    def __dealloc__(self):
        del self.alg
    def solve(self, Game game):
        cdef c_List[c_MixedBehaviorProfileRational] solns
        cdef MixedBehaviorProfileRational p
        cdef shared_ptr[c_BehaviorSupportProfile] profile
        profile = make_shared[c_BehaviorSupportProfile](game.game)
        solns = self.alg.Solve(deref(profile))
        ret = [ ]
        for i in xrange(solns.Length()):
            p = MixedBehaviorProfileRational()
            p.profile = copyitem_list_mbpr(solns, i+1)
            ret.append(p)
        return ret

cdef class LCPStrategySolverDouble:
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

cdef class LCPStrategySolverRational:
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




cdef class LPBehaviorSolverDouble:
    cdef c_NashLpBehavSolverDouble *alg

    def __cinit__(self):
        self.alg = new c_NashLpBehavSolverDouble()
    def __dealloc__(self):
        del self.alg
    def solve(self, Game game):
        cdef c_List[c_MixedBehaviorProfileDouble] solns
        cdef MixedBehaviorProfileDouble p
        cdef shared_ptr[c_BehaviorSupportProfile] profile
        profile = make_shared[c_BehaviorSupportProfile](game.game)
        solns = self.alg.Solve(deref(profile))
        ret = [ ]
        for i in xrange(solns.Length()):
            p = MixedBehaviorProfileDouble()
            p.profile = copyitem_list_mbpd(solns, i+1)
            ret.append(p)
        return ret

cdef class LPBehaviorSolverRational:
    cdef c_NashLpBehavSolverRational *alg

    def __cinit__(self):
        self.alg = new c_NashLpBehavSolverRational()
    def __dealloc__(self):
        del self.alg
    def solve(self, Game game):
        cdef c_List[c_MixedBehaviorProfileRational] solns
        cdef MixedBehaviorProfileRational p
        cdef shared_ptr[c_BehaviorSupportProfile] profile
        profile = make_shared[c_BehaviorSupportProfile](game.game)
        solns = self.alg.Solve(deref(profile))
        ret = [ ]
        for i in xrange(solns.Length()):
            p = MixedBehaviorProfileRational()
            p.profile = copyitem_list_mbpr(solns, i+1)
            ret.append(p)
        return ret

cdef class LPStrategySolverDouble:
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

cdef class LPStrategySolverRational:
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


cdef class LiapStrategySolver:
    cdef c_NashLiapStrategySolver *alg

    def __cinit__(self, maxiter=100):
        self.alg = new c_NashLiapStrategySolver(maxiter)
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


cdef class LiapBehaviorSolver:
    cdef c_NashLiapBehaviorSolver *alg

    def __cinit__(self, maxiter=100):
        self.alg = new c_NashLiapBehaviorSolver(maxiter)
    def __dealloc__(self):
        del self.alg
    def solve(self, Game game):
        cdef c_List[c_MixedBehaviorProfileDouble] solns
        cdef MixedBehaviorProfileDouble p
        cdef shared_ptr[c_BehaviorSupportProfile] profile
        profile = make_shared[c_BehaviorSupportProfile](game.game)
        solns = self.alg.Solve(deref(profile))
        ret = [ ]
        for i in xrange(solns.Length()):
            p = MixedBehaviorProfileDouble()
            p.profile = copyitem_list_mbpd(solns, i+1)
            ret.append(p)
        return ret


cdef class SimpdivStrategySolver:
    cdef c_NashSimpdivStrategySolver *alg

    def __cinit__(self):
        self.alg = new c_NashSimpdivStrategySolver()
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

cdef class IPAStrategySolver:
    cdef c_NashIPAStrategySolver *alg

    def __cinit__(self):
        self.alg = new c_NashIPAStrategySolver()
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

cdef class GNMStrategySolver:
    cdef c_NashGNMStrategySolver *alg

    def __cinit__(self):
        self.alg = new c_NashGNMStrategySolver()
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


cdef class LogitQREMixedStrategyProfile:
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

    @property
    def game(self) -> Game:
        """The game on which this mixed strategy profile is defined."""
        cdef Game g
        g = Game()
        g.game = self.thisptr.GetGame()
        return g

    @property
    def lam(self) -> double:
        """The value of the precision parameter."""
        return self.thisptr.GetLambda()

    @property
    def log_like(self) -> double:
        """The log-likelihood of the data."""
        return self.thisptr.GetLogLike()

    @property
    def profile(self) -> MixedStrategyProfileDouble:
        """The mixed strategy profile."""
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
