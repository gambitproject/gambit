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


cdef extern from "tools/lcp/nfglcp.h":
    cdef cppclass c_NashLcpStrategySolverDouble "NashLcpStrategySolver<double>":
        c_NashLcpStrategySolverDouble(int, int)
        List[c_MixedStrategyProfileDouble] Solve(c_Game)


cdef class LCPSolver(object):
    cdef c_NashLcpStrategySolverDouble *alg

    def __cinit__(self, p_stopAfter=0, p_maxDepth=0):
        self.alg = new c_NashLcpStrategySolverDouble(p_stopAfter, p_maxDepth)
    def __dealloc__(self):
        del self.alg
    def solve(self, Game game, rational=False, use_strategic=False):
        cdef List[c_MixedStrategyProfileDouble] solns
        cdef MixedStrategyProfileDouble p
        solns = self.alg.Solve(game.game)
        ret = [ ]
        for i in xrange(solns.Length()):
            p = MixedStrategyProfileDouble()
            p.profile = CopyElement(solns, i+1)
            ret.append(p)
        return ret
