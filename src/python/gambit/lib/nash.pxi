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
        List[c_MixedStrategyProfileRational] Solve(c_Game) except +RuntimeError

    cdef cppclass c_NashEnumPureAgentSolver "NashEnumPureAgentSolver":
        c_NashEnumPureAgentSolver()
        List[c_MixedBehaviorProfileRational] Solve(c_Game) except +RuntimeError

cdef class EnumPureStrategySolver(object):
    cdef c_NashEnumPureStrategySolver *alg

    def __cinit__(self):
        self.alg = new c_NashEnumPureStrategySolver()
    def __dealloc__(self):
        del self.alg
    def solve(self, Game game):
        cdef List[c_MixedStrategyProfileRational] solns
        cdef MixedStrategyProfileRational p
        solns = self.alg.Solve(game.game)
        ret = [ ]
        for i in xrange(solns.Length()):
            p = MixedStrategyProfileRational()
            p.profile = CopyElementStrategyRational(solns, i+1)
            ret.append(p)
        return ret

cdef class EnumPureAgentSolver(object):
    cdef c_NashEnumPureAgentSolver *alg

    def __cinit__(self, p_stopAfter=0, p_maxDepth=0):
        self.alg = new c_NashEnumPureAgentSolver()
    def __dealloc__(self):
        del self.alg
    def solve(self, Game game):
        cdef List[c_MixedBehaviorProfileRational] solns
        cdef MixedBehaviorProfileRational p
        solns = self.alg.Solve(game.game)
        ret = [ ]
        for i in xrange(solns.Length()):
            p = MixedBehaviorProfileRational()
            p.profile = CopyElementBehaviorRational(solns, i+1)
            ret.append(p)
        return ret


cdef extern from "tools/lcp/nfglcp.h":
    cdef cppclass c_NashLcpStrategySolverDouble "NashLcpStrategySolver<double>":
        c_NashLcpStrategySolverDouble(int, int)
        List[c_MixedStrategyProfileDouble] Solve(c_Game) except +RuntimeError

    cdef cppclass c_NashLcpStrategySolverRational "NashLcpStrategySolver<Rational>":
        c_NashLcpStrategySolverRational(int, int)
        List[c_MixedStrategyProfileRational] Solve(c_Game) except +RuntimeError

cdef extern from "tools/lcp/efglcp.h":
    cdef cppclass c_NashLcpBehaviorSolverDouble "NashLcpBehaviorSolver<double>":
        c_NashLcpBehaviorSolverDouble(int, int)
        List[c_MixedBehaviorProfileDouble] Solve(c_Game) except +RuntimeError

    cdef cppclass c_NashLcpBehaviorSolverRational "NashLcpBehaviorSolver<Rational>":
        c_NashLcpBehaviorSolverRational(int, int)
        List[c_MixedBehaviorProfileRational] Solve(c_Game) except +RuntimeError


cdef class LCPBehaviorSolverDouble(object):
    cdef c_NashLcpBehaviorSolverDouble *alg

    def __cinit__(self, p_stopAfter=0, p_maxDepth=0):
        self.alg = new c_NashLcpBehaviorSolverDouble(p_stopAfter, p_maxDepth)
    def __dealloc__(self):
        del self.alg
    def solve(self, Game game):
        cdef List[c_MixedBehaviorProfileDouble] solns
        cdef MixedBehaviorProfileDouble p
        solns = self.alg.Solve(game.game)
        ret = [ ]
        for i in xrange(solns.Length()):
            p = MixedBehaviorProfileDouble()
            p.profile = CopyElementBehaviorDouble(solns, i+1)
            ret.append(p)
        return ret

cdef class LCPBehaviorSolverRational(object):
    cdef c_NashLcpBehaviorSolverRational *alg

    def __cinit__(self, p_stopAfter=0, p_maxDepth=0):
        self.alg = new c_NashLcpBehaviorSolverRational(p_stopAfter, p_maxDepth)
    def __dealloc__(self):
        del self.alg
    def solve(self, Game game):
        cdef List[c_MixedBehaviorProfileRational] solns
        cdef MixedBehaviorProfileRational p
        solns = self.alg.Solve(game.game)
        ret = [ ]
        for i in xrange(solns.Length()):
            p = MixedBehaviorProfileRational()
            p.profile = CopyElementBehaviorRational(solns, i+1)
            ret.append(p)
        return ret

cdef class LCPStrategySolverDouble(object): 
    cdef c_NashLcpStrategySolverDouble *alg

    def __cinit__(self, p_stopAfter=0, p_maxDepth=0):
        self.alg = new c_NashLcpStrategySolverDouble(p_stopAfter, p_maxDepth)
    def __dealloc__(self):
        del self.alg
    def solve(self, Game game):
        cdef List[c_MixedStrategyProfileDouble] solns
        cdef MixedStrategyProfileDouble p
        solns = self.alg.Solve(game.game)
        ret = [ ]
        for i in xrange(solns.Length()):
            p = MixedStrategyProfileDouble()
            p.profile = CopyElementStrategyDouble(solns, i+1)
            ret.append(p)
        return ret

cdef class LCPStrategySolverRational(object):
    cdef c_NashLcpStrategySolverRational *alg

    def __cinit__(self, p_stopAfter=0, p_maxDepth=0):
        self.alg = new c_NashLcpStrategySolverRational(p_stopAfter, p_maxDepth)
    def __dealloc__(self):
        del self.alg
    def solve(self, Game game):
        cdef List[c_MixedStrategyProfileRational] solns
        cdef MixedStrategyProfileRational p
        solns = self.alg.Solve(game.game)
        ret = [ ]
        for i in xrange(solns.Length()):
            p = MixedStrategyProfileRational()
            p.profile = CopyElementStrategyRational(solns, i+1)
            ret.append(p)
        return ret

